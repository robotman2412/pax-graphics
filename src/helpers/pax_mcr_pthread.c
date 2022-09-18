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

#define PAX_GFX_C
#ifndef PAX_GFX_C
#ifndef ARDUINO
#pragma message "This file should not be compiled on it's own."
#endif
#else

#include "pax_internal.h"


/* ===== MULTI-CORE RENDERING ==== */

#include <ptq.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>


// The scheduler for multicore rendering.
static void paxmcr_add_task(pax_task_t *task) {
	// Create a copy.
	pax_task_t copy    = *task;
	
	// Of the shape,
	if (copy.shape) {
		copy.shape     = malloc(copy.shape_len * sizeof(float));
		memcpy(copy.shape, task->shape, copy.shape_len * sizeof(float));
	}
	
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
	if (!ptq_send_block(queue_handle, &copy, NULL)) {
		PAX_LOGE(TAG, "No space in queue!");
		PAX_LOGW(TAG, "Reverting to disabling MCR.");
		pax_disable_multicore();
	}
}

// The actual task for multicore rendering.
static void *pax_multicore_task_function(void *args) {
	const char *TAG = "pax-mcr-worker";
	
	multicore_busy = false;
	pax_task_t tsk;
	if (!pax_do_multicore) {
		PAX_LOGE(TAG, "Multicore set to disabled before worker started.");
	} else {
		PAX_LOGI(TAG, "MCR worker started.");
	}
	
	while (pax_do_multicore) {
		// Wait for a task.
		if (!ptq_receive_block(queue_handle, &tsk, &multicore_mutex)) {
			PAX_LOGE(TAG, "Error while receiving from queue!");
			PAX_LOGW(TAG, "Reverting to disabling MCR.");
			pax_do_multicore = false;
			break;
		}
		
		// TODO: Sanity check on tasks?
		if (tsk.type == PAX_TASK_STOP) {
			PAX_LOGI(TAG, "Received stop command.");
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
		
		// Release the sync mutex.
		pthread_mutex_unlock(&multicore_mutex);
	}
	
	PAX_LOGI(TAG, "MCR worker stopped.");
	return NULL;
}

// If multi-core rendering is enabled, wait for the other core.
void pax_join() {
	if (pax_do_multicore) {
		// Await queue to be empty.
		ptq_join(queue_handle, &multicore_mutex);
		pthread_mutex_unlock(&multicore_mutex);
	}
}

// Enable multi-core rendering.
void pax_enable_multicore(int core) {
	if (pax_do_multicore) {
		PAX_LOGW(TAG, "No need to enable MCR: MCR was already enabled.");
		return;
	}
	
	// Enable log mutex.
	int res = pthread_mutex_init(&pax_log_mutex, NULL);
	if (res) {
		PAX_LOGE(TAG, "Failed to enable MCR: Log mutex creation error.");
		return;
	}
	pax_log_use_mutex = true;
	
	// Mark MCR as enabled.
	pax_do_multicore  = true;
	
	// Create sync mutex.
	res = pthread_mutex_init(&multicore_mutex, NULL);
	if (res) {
		PAX_LOGE(TAG, "Failed to enable MCR: Sync mutex creation error.");
		return;
	}
	
	// Create queue.
	queue_handle = ptq_create_max(sizeof(pax_task_t), PAX_QUEUE_SIZE);
	if (!queue_handle) {
		PAX_LOGE(TAG, "Failed to enable MCR: Queue creation error.");
		pax_do_multicore = false;
		return;
	}
	
	// Create worker thread.
	res = pthread_create(&multicore_handle, NULL, pax_multicore_task_function, NULL);
	
	if (res) {
		PAX_LOGE(TAG, "Failed to enable MCR: Task creation error %d.", res);
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
	
	// Clean up queue.
	ptq_destroy(queue_handle);
	
	// Disable log mutex.
	pax_log_use_mutex = false;
	pthread_mutex_destroy(&pax_log_mutex);
	
}

#endif
