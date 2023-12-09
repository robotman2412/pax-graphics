
// SPDX-License-Identifier: MIT

#include "pax_internal.h"

#if PAX_STANDALONE && PAX_COMPILE_MCR

/* ===== MULTI-CORE RENDERING ==== */

#include <pthread.h>
#include <ptq.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>

// Whether or not the multicore task is currently busy.
extern bool            multicore_busy;
// The thread for multicore helper stuff.
extern pthread_t       multicore_handle;
// The mutex used to determine IDLE.
extern pthread_mutex_t multicore_mutex;
// The render queue for the multicore helper.
extern ptq_queue_t     queue_handle;

// The scheduler for multicore rendering.
void paxmcr_add_task(pax_task_t *task) {
    // Snedt it.
    if (!ptq_send_block(queue_handle, task, NULL)) {
        PAX_LOGE(TAG, "No space in queue!");
        PAX_LOGW(TAG, "Reverting to disabling MCR.");
        pax_disable_multicore();
    }
}

// The actual task for multicore rendering.
static void *pax_multicore_task_function(void *args) {
    char const *TAG = "pax-mcr-worker";

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
        if (tsk.use_shader) {
            if (tsk.type == PAX_TASK_RECT) {
                paxmcr_rect_shaded(
                    true,
                    tsk.buffer,
                    tsk.color,
                    &tsk.shader,
                    tsk.shape[0],
                    tsk.shape[1],
                    tsk.shape[2],
                    tsk.shape[3],
                    tsk.quad_uvs.x0,
                    tsk.quad_uvs.y0,
                    tsk.quad_uvs.x1,
                    tsk.quad_uvs.y1,
                    tsk.quad_uvs.x2,
                    tsk.quad_uvs.y2,
                    tsk.quad_uvs.x3,
                    tsk.quad_uvs.y3
                );
            } else if (tsk.type == PAX_TASK_TRI) {
                paxmcr_tri_shaded(
                    true,
                    tsk.buffer,
                    tsk.color,
                    &tsk.shader,
                    tsk.shape[0],
                    tsk.shape[1],
                    tsk.shape[2],
                    tsk.shape[3],
                    tsk.shape[4],
                    tsk.shape[5],
                    tsk.tri_uvs.x0,
                    tsk.tri_uvs.y0,
                    tsk.tri_uvs.x1,
                    tsk.tri_uvs.y1,
                    tsk.tri_uvs.x2,
                    tsk.tri_uvs.y2
                );
            }
        } else {
            if (tsk.type == PAX_TASK_RECT) {
                paxmcr_rect_unshaded(
                    true,
                    tsk.buffer,
                    tsk.color,
                    tsk.shape[0],
                    tsk.shape[1],
                    tsk.shape[2],
                    tsk.shape[3]
                );
            } else if (tsk.type == PAX_TASK_TRI) {
                paxmcr_tri_unshaded(
                    true,
                    tsk.buffer,
                    tsk.color,
                    tsk.shape[0],
                    tsk.shape[1],
                    tsk.shape[2],
                    tsk.shape[3],
                    tsk.shape[4],
                    tsk.shape[5]
                );
            }
        }

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
    pax_do_multicore = true;

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
    pax_do_multicore   = false;
    pax_task_t stopper = {
        .type = PAX_TASK_STOP,
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
