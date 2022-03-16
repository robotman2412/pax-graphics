/*
	MIT License

	Copyright (c) 2022 Julian Scheffers

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
#error "This file should not be compiled on it's own."
#endif

#include "pax_internal.h"
#include <esp_timer.h>


/* ===== MULTI-CORE RENDERING ==== */

#define PAX_COMPILE_MCR
#ifdef PAX_COMPILE_MCR

// The scheduler for multicore rendering.
static void paxmcr_add_task(pax_task_t *task) {
	// Create a copy.
	pax_task_t copy    = *task;
	
	// Of the shape,
	copy.shape         = malloc(copy.shape_len * sizeof(float));
	memcpy(copy.shape, task->shape, copy.shape_len * sizeof(float));
	
	// The shader,
	if (copy.shader) {
		copy.shader    = malloc(sizeof(pax_shader_t));
		*copy.shader   = *task->shader;
	}
	
	// And the UVs.
	if (copy.type == PAX_TASK_TRI && copy.tri_uvs) {
		copy.tri_uvs   = malloc(sizeof(pax_tri_t));
		*copy.tri_uvs  = *task->tri_uvs;
	} else if (copy.type == PAX_TASK_RECT && copy.quad_uvs) {
		copy.quad_uvs  = malloc(sizeof(pax_quad_t));
		*copy.quad_uvs = *task->quad_uvs;
	}
	
	// Snedt it.
	if (xQueueSend(queue_handle, &copy, pdMS_TO_TICKS(100)) != pdTRUE) {
		ESP_LOGE(TAG, "No space in queue after 100ms!");
		ESP_LOGW(TAG, "Reverting to disabling MCR.");
		pax_disable_multicore();
	}
}

// The actual task for multicore rendering.
static void pax_multicore_task_function(void *args) {
	const char *TAG = "pax-mcr-worker";
	ESP_LOGI(TAG, "MCR worker started.");
	
	multicore_busy = false;
	pax_task_t tsk;
	while (pax_do_multicore) {
		// Wait for a task.
		if (uxQueueMessagesWaiting(queue_handle)) {
			multicore_busy = true;
			while (uxQueueMessagesWaiting(queue_handle) && xQueueReceive(queue_handle, &tsk, pdMS_TO_TICKS(1))) {
				// TODO: Sanity check on tasks?
				// Now, we actually DRAW.
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
				free(tsk.shape);
				if (tsk.shader)
					free(tsk.shader);
				if (tsk.tri_uvs)
					free(tsk.tri_uvs);
			}
			// We're done with drawing.
			multicore_busy = false;
			// Wake the main task.
			vTaskResume(main_handle);
			vTaskDelay(pdMS_TO_TICKS(5));
		} else {
			// There's nothing else to do.
			vTaskDelay(pdMS_TO_TICKS(5));
		}
	}
	
	ESP_LOGI(TAG, "MCR worker stopped.");
	multicore_handle = NULL;
	vTaskDelete(NULL);
}

#endif //PAX_COMPILE_MCR

// If multi-core rendering is enabled, wait for the other core.
void pax_join() {
	#ifdef PAX_COMPILE_MCR
	while (multicore_handle && (uxQueueMessagesWaiting(queue_handle) || multicore_busy)) {
		// Wait for the other core.
		taskYIELD();
	}
	#endif
}

// Enable multi-core rendering.
void pax_enable_multicore(int core) {
	#ifdef PAX_COMPILE_MCR
	if (pax_do_multicore) {
		ESP_LOGW(TAG, "No need to enable MCR: MCR was already enabled.");
		return;
	}
	
	// Figure out who we are so the worker can wake us up.
	main_handle = xTaskGetCurrentTaskHandle();
	
	// Create a queue for the rendering tasks.
	if (!queue_handle) {
		queue_handle = xQueueCreate(PAX_QUEUE_SIZE, sizeof(pax_task_t));
		if (!queue_handle) {
			ESP_LOGE(TAG, "Failed to enable MCR: Queue creation error.");
		}
	}
	
	// Create a task to do said rendering.
	int result = xTaskCreatePinnedToCore(
		pax_multicore_task_function,
		"pax_mcr_worker", 2048, NULL, 2,
		&multicore_handle, core
	);
	if (result != pdPASS) {
		multicore_handle = NULL;
		ESP_LOGE(TAG, "Failed to enable MCR: Task creation error %s (%x).", esp_err_to_name(result), result);
	} else {
		pax_do_multicore = true;
		ESP_LOGI(TAG, "Successfully enabled MCR.");
	}
	#else
	ESP_LOGE(TAG, "Failed to enable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
	#endif
}

// Disable multi-core rendering.
void pax_disable_multicore() {
	#ifdef PAX_COMPILE_MCR
	if (!pax_do_multicore) {
		ESP_LOGW(TAG, "No need to disable MCR: MCR was not enabled.");
		return;
	}
	pax_do_multicore = false;
	
	// The task, realising multicore is now disabled, will end itself when finished.
	pax_join();
	
	vQueueDelete(queue_handle);
	queue_handle = NULL;
	
	multicore_handle = NULL;
	#else
	ESP_LOGE(TAG, "No need to disable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
	#endif
}

