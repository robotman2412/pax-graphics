/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef PAX_GFX_C
#ifndef ARDUINO
#pragma message "This file should not be compiled on it's own."
#endif
#else

#include "pax_internal.h"


/* ===== MULTI-CORE RENDERING ==== */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <esp_timer.h>
#include <esp_pm.h>


// The scheduler for multicore rendering.
void paxmcr_add_task(pax_task_t *task) {
	// Create a copy.
	pax_task_t copy    = *task;
	
	// Of the shape,
	if (copy.shape) {
		copy.shape     = malloc(copy.shape_len * sizeof(float));
		if (!copy.shape) goto abort;
		memcpy(copy.shape, task->shape, copy.shape_len * sizeof(float));
	}
	
	// The shader,
	if (copy.shader) {
		copy.shader    = malloc(sizeof(pax_shader_t));
		if (!copy.shader) {
			if (copy.shape) free(copy.shape);
			goto abort;
		}
		*copy.shader   = *task->shader;
	}
	
	// And the UVs.
	if (copy.type == PAX_TASK_TRI && copy.tri_uvs) {
		copy.tri_uvs   = malloc(sizeof(pax_trif));
		if (!copy.tri_uvs) {
			if (copy.shape) free(copy.shape);
			if (copy.shader) free(copy.shape);
			goto abort;
		}
		*copy.tri_uvs  = *task->tri_uvs;
	} else if (copy.type == PAX_TASK_RECT && copy.quad_uvs) {
		copy.quad_uvs  = malloc(sizeof(pax_quadf));
		if (!copy.quad_uvs) {
			if (copy.shape) free(copy.shape);
			if (copy.shader) free(copy.shape);
			goto abort;
		}
		*copy.quad_uvs = *task->quad_uvs;
	}
	
	// Snedt it.
	esp_err_t res = xQueueSend(queue_handle, &copy, pdMS_TO_TICKS(100));
	if (res != pdTRUE) {
		PAX_LOGE(TAG, "No space in queue after 100ms!");
		PAX_LOGW(TAG, "Reverting to disabling MCR.");
		pax_disable_multicore();
	}
	return;
	
	abort:
	PAX_LOGE(TAG, "Out of memory for MCR operation!");
	PAX_LOGW(TAG, "Reverting to disabling MCR.");
	pax_disable_multicore();
}

// The actual task for multicore rendering.
static void pax_multicore_task_function(void *args) {
	const char *TAG = "pax-mcr-worker";
	PAX_LOGI(TAG, "MCR worker started.");
	
	// Up the frequency.
	esp_pm_lock_handle_t pm_lock;
	esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, NULL, &pm_lock);
	esp_pm_lock_acquire(pm_lock);
	
	multicore_busy = false;
	pax_task_t tsk;
	while (pax_do_multicore) {
		// Wait for a task.
		esp_err_t res = xQueueReceive(queue_handle, &tsk, pdMS_TO_TICKS(100));
		if (res != pdTRUE) continue;
		multicore_busy = true;
		// TODO: Sanity check on tasks?
		if (tsk.type == PAX_TASK_STOP) {
			break;
		}
		
		// Now, we actually DRAW.will end itself
		if (tsk.shader) {
			if (tsk.type == PAX_TASK_RECT) {
				paxmcr_rect_shaded(
					true, tsk.buffer,
					tsk.color, tsk.shader,
					tsk.shape[0], tsk.shape[1],
					tsk.shape[2], tsk.shape[3],
					tsk.quad_uvs->x0, tsk.quad_uvs->y0,
					tsk.quad_uvs->x1, tsk.quad_uvs->y1,
					tsk.quad_uvs->x2, tsk.quad_uvs->y2,
					tsk.quad_uvs->x3, tsk.quad_uvs->y3
				);
			} else if (tsk.type == PAX_TASK_TRI) {
				paxmcr_tri_shaded(
					true, tsk.buffer,
					tsk.color, tsk.shader,
					tsk.shape[0], tsk.shape[1],
					tsk.shape[2], tsk.shape[3],
					tsk.shape[4], tsk.shape[5],
					tsk.tri_uvs->x0, tsk.tri_uvs->y0,
					tsk.tri_uvs->x1, tsk.tri_uvs->y1,
					tsk.tri_uvs->x2, tsk.tri_uvs->y2
				);
			}
		} else {
			if (tsk.type == PAX_TASK_RECT) {
				paxmcr_rect_unshaded(
					true, tsk.buffer,
					tsk.color,
					tsk.shape[0], tsk.shape[1],
					tsk.shape[2], tsk.shape[3]
				);
			} else if (tsk.type == PAX_TASK_TRI) {
				paxmcr_tri_unshaded(
					true, tsk.buffer,
					tsk.color,
					tsk.shape[0], tsk.shape[1],
					tsk.shape[2], tsk.shape[3],
					tsk.shape[4], tsk.shape[5]
				);
			}
		}
		
		// Free up our memories.
		if (tsk.shape)
			free(tsk.shape);
		if (tsk.shader)
			free(tsk.shader);
		if (tsk.tri_uvs)
			free(tsk.tri_uvs);
	}
	
	// Cleaning.
	esp_pm_lock_release(pm_lock);
	esp_pm_lock_delete(pm_lock);
	
	PAX_LOGI(TAG, "MCR worker stopped.");
	vTaskDelete(NULL);
}

// If multi-core rendering is enabled, wait for the other core.
PAX_PERF_CRITICAL_ATTR void pax_join() {
	while ((multicore_handle && eTaskGetState(multicore_handle) == eRunning) || (queue_handle && uxQueueMessagesWaiting(queue_handle))) {
		// Wait for the other core.
		taskYIELD();
	}
}

// Enable multi-core rendering.
void pax_enable_multicore(int core) {
	if (pax_do_multicore) {
		PAX_LOGW(TAG, "No need to enable MCR: MCR was already enabled.");
		return;
	}
	
	// Figure out who we are so the worker can wake us up.
	main_handle = xTaskGetCurrentTaskHandle();
	
	// Create a queue for the rendering tasks.
	if (!queue_handle) {
		queue_handle = xQueueCreate(PAX_QUEUE_SIZE, sizeof(pax_task_t));
		if (!queue_handle) {
			PAX_LOGE(TAG, "Failed to enable MCR: Queue creation error.");
			return;
		}
	}
	
	// Create a task to do said rendering.
	pax_do_multicore = true;
	int result = xTaskCreatePinnedToCore(
		pax_multicore_task_function,
		"pax_mcr_worker", 4096, NULL, 2,
		&multicore_handle, core
	);
	if (result != pdPASS) {
		multicore_handle = NULL;
		PAX_LOGE(TAG, "Failed to enable MCR: Task creation error %s (%x).", esp_err_to_name(result), result);
		pax_do_multicore = false;
	} else {
		PAX_LOGI(TAG, "Successfully enabled MCR.");
	}
}

// Disable multi-core rendering.
void pax_disable_multicore() {
	if (!pax_do_multicore) {
		PAX_LOGW(TAG, "No need to disable MCR: MCR was not enabled.");
		return;
	}
	PAX_LOGI(TAG, "Disabling MCR...");
	
	// Notify that multicore is disabled.
	pax_do_multicore = false;
	pax_task_t stopper = {
		.type    = PAX_TASK_STOP,
		.shader  = NULL,
		.shape   = NULL,
		.tri_uvs = NULL,
	};
	paxmcr_add_task(&stopper);
	
	// The task, realising multicore is now disabled, exit when finished.
	pax_join();
	
	vQueueDelete(queue_handle);
	queue_handle = NULL;
	
	PAX_LOGI(TAG, "MCR successfully disabled.");
	multicore_handle = NULL;
}

#endif
