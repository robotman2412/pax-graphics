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

#include "pax_internal.h"
#include "pax_shaders.h"

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// The last error reported.
pax_err_t            pax_last_error   = PAX_OK;
// Whether multi-core rendering is enabled.
// You should not modify this variable.
bool                 pax_do_multicore = false;
#ifdef PAX_COMPILE_MCR
// Whether or not the multicore task is currently busy.
static bool          multicore_busy   = false;
// The task handle for the main core.
static TaskHandle_t  main_handle      = NULL;
// The task handle for the other core.
static TaskHandle_t  multicore_handle = NULL;
// The render queue for the other core.
static QueueHandle_t queue_handle     = NULL;
#endif

static inline uint32_t pax_col2buf(pax_buf_t *buf, pax_col_t color) {
	uint8_t bpp = buf->bpp;
	if (PAX_IS_GREY(buf->type)) {
		// Greyscale.
		uint16_t grey = ((color >> 16) & 0xff) + ((color >> 8) & 0xff) + (color & 0xff);
		grey /= 3;
		return ((uint8_t) grey >> (8 - bpp));
	} else if (buf->type == PAX_BUF_4_1111ARGB) {
		// 4BPP 1111-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                      ARGB
		uint16_t value = ((color >> 28) & 0x8) | ((color >> 21) & 0x4) | ((color >> 14) & 0x2) | ((color >> 7) & 0x1);
		return value;
	} else if (buf->type == PAX_BUF_8_332RGB) {
		// 8BPP 332-RGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                 RrrG ggBb
		uint16_t value = ((color >> 16) & 0xe0) | ((color >> 11) & 0x1c) | ((color >> 6) & 0x03);
		return value;
	} else if (buf->type == PAX_BUF_8_2222ARGB) {
		// 8BPP 2222-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                                 AaRr GgBb
		uint16_t value = ((color >> 24) & 0xc0) | ((color >> 18) & 0x30) | ((color >> 12) & 0x0c) | ((color >> 6) & 0x03);
		return value;
	} else if (buf->type == PAX_BUF_16_4444ARGB) {
		// 16BPP 4444-ARGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                       Aaaa Rrrr Gggg Bbbb
		uint16_t value = ((color >> 16) & 0xf000) | ((color >> 12) & 0x0f00) | ((color >> 8) & 0x00f0) | ((color >> 4) & 0x000f);
		return (value >> 8) | ((value << 8) & 0xff00);
	} else if (buf->type == PAX_BUF_16_565RGB) {
		// 16BPP 565-RGB
		// From: Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		// To:                       Rrrr rGgg gggB bbbb
		uint16_t value = ((color >> 8) & 0xf800) | ((color >> 5) & 0x07e0) | ((color >> 3) & 0x001f);
		return (value >> 8) | ((value << 8) & 0xff00);
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return color;
	}
	PAX_ERROR1("pax_col2buf", PAX_ERR_PARAM, 0);
}

static inline uint32_t pax_buf2col(pax_buf_t *buf, uint32_t value) {
	uint8_t bpp = buf->bpp;
	if (PAX_IS_GREY(buf->type)) {
		// Greyscale.
		//uint8_t grey_bits = (buf->type >> 8) & 0xf;
		uint8_t grey = value << (8 - bpp);
		if      (bpp == 7) grey |= grey >> 7;
		else if (bpp == 6) grey |= grey >> 6;
		else if (bpp == 5) grey |= grey >> 5;
		else if (bpp == 4) grey |= grey >> 4;
		else if (bpp == 3) grey = (value * 0x49) >> 1;
		else if (bpp == 2) grey =  value * 0x55;
		else if (bpp == 1) grey = -value;
		return 0xff000000 | (grey << 16) | (grey << 8) | grey;
	} else if (buf->type == PAX_BUF_4_1111ARGB) {
		// 4BPP 1111-ARGB
		// From:                                    ARGB
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		pax_col_t color = ((value << 28) & 0x80000000) | ((value << 21) & 0x00800000) | ((value << 14) & 0x00008000) | ((value << 7) & 0x00000080);
		color |= color >> 1;
		color |= color >> 2;
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_8_332RGB) {
		// 8BPP 332-RGB
		// From:                               RrrG ggBb
		// To:   .... .... Rrr. .... Ggg. .... .... ....
		// Add:  .... .... ...R rrRr ...Gg gGg .... ....
		// Add:  .... .... .... .... .... .... BbBb BbBb
		pax_col_t color = ((value << 16) & 0x00e00000) | ((value << 11) & 0x0000e000);
		color |= (color >> 3) | ((color >> 6) & 0x000f0f00);
		pax_col_t temp  = (value & 0x03);
		temp |= temp << 2;
		color |= temp | (temp << 4);
		return color | 0xff000000;
	} else if (buf->type == PAX_BUF_8_2222ARGB) {
		// 8BPP 2222-ARGB
		// From:                               AaRr GgBb
		// To:   Aaaa aaaa Rrrr rrrr Gggg gggg Bbbb bbbb
		pax_col_t color = ((value << 24) & 0xc0000000) | ((value << 18) & 0x00c00000) | ((value << 12) & 0x0000c000) | ((value << 6) & 0x000000c0);
		color |= color >> 2;
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_16_4444ARGB) {
		// 16BPP 4444-ARGB
		// From:                     Aaaa Rrrr Gggg Bbbb
		// To:   Aaaa .... Rrrr .... Gggg .... Bbbb ....
		// Add:  .... Aaaa .... Rrrr .... Gggg .... Bbbb
		pax_col_t color = ((value << 16) & 0xf0000000) | ((value << 12) & 0x00f00000) | ((value << 8) & 0x0000f000) | ((value << 4) & 0x000000f0);
		color |= color >> 4;
		return color;
	} else if (buf->type == PAX_BUF_16_565RGB) {
		value = ((value << 8) & 0xff00) | ((value >> 8) & 0x00ff);
		// 16BPP 565-RGB
		// From:                     Rrrr rGgg gggB bbbb
		// To:   .... .... Rrrr r... Gggg gg.. Bbbb b...
		// Add:  .... .... .... .Rrr .... ..Gg .... .Bbb
		// Take the existing information.
		pax_col_t color = ((value << 8) & 0x00f80000) | ((value << 5) & 0x0000fc00) | ((value << 3) & 0x000000f8);
		// Now, fill in some missing bits.
		color |= ((value << 3) & 0x00070000) | ((value >> 1) & 0x00000300) | ((value >> 2) & 0x00000007);
		return color | 0xff000000;
	} else if (buf->type == PAX_BUF_32_8888ARGB) {
		return value;
	} else if (PAX_IS_PALETTE(buf->type)) {
		// Pallette lookup.
		if (value >= buf->pallette_size) return *buf->pallette;
		else return buf->pallette[value];
	}
	PAX_ERROR1("pax_buf2col", PAX_ERR_PARAM, 0);
}

// Set a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline void pax_set_pixel_u(pax_buf_t *buf, uint32_t color, int x, int y) {
	uint8_t bpp = buf->bpp;
	if (bpp == 1) {
		// 1BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 3];
		uint8_t mask = 0x01 << (x & 7);
		*ptr = (*ptr & ~mask) | (color << (x & 7));
	} else if (bpp == 2) {
		// 2BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 2];
		uint8_t mask = 0x03 << (x & 3) * 2;
		*ptr = (*ptr & ~mask) | (color << ((x & 3) * 2));
	} else if (bpp == 4) {
		// 4BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 1];
		uint8_t mask = (x & 1) ? 0xf0 : 0x0f;
		*ptr = (*ptr & ~mask) | (color << ((x & 1) * 4));
	} else if (bpp == 8) {
		// 8BPP
		buf->buf_8bpp[x + y * buf->width] = color;
	} else if (bpp == 16) {
		// 16BPP
		buf->buf_16bpp[x + y * buf->width] = color;
	} else if (bpp == 32) {
		// 32BPP
		buf->buf_32bpp[x + y * buf->width] = color;
	} else {
		PAX_ERROR("pax_set_pixel_u", PAX_ERR_PARAM);
	}
}

// Get a pixel, unsafe (don't check bounds or buffer, no color conversion).
static inline uint32_t pax_get_pixel_u(pax_buf_t *buf, int x, int y) {
	uint8_t bpp = buf->bpp;
	if (bpp == 1) {
		// 1BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 3];
		uint8_t mask = 0x01 << (x & 7);
		return (*ptr & mask) >> (x & 7);
	} else if (bpp == 2) {
		// 2BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 2];
		uint8_t mask = 0x03 << (x & 3) * 2;
		return (*ptr & mask) >> ((x & 3) * 2);
	} else if (bpp == 4) {
		// 4BPP
		uint8_t *ptr = &buf->buf_8bpp[(x + y * buf->width) >> 1];
		uint8_t mask = (x & 1) ? 0xf0 : 0x0f;
		return (*ptr & mask) >> ((x & 1) * 4);
	} else if (bpp == 8) {
		// 8BPP
		return buf->buf_8bpp[x + y * buf->width];
	} else if (bpp == 16) {
		// 16BPP
		return buf->buf_16bpp[x + y * buf->width];
	} else if (bpp == 32) {
		// 32BPP
		return buf->buf_32bpp[x + y * buf->width];
	} else {
		//PAX_ERROR1("pax_get_pixel_u", PAX_ERR_PARAM, 0);
		return 0;
	}
}

// UV interpolation helper for the circle methods.
static inline float pax_flerp4(float x, float y, float e0, float e1, float e2, float e3) {
	x = x *  0.5 + 0.5;
	y = y * -0.5 + 0.5;
	float a = e0 + (e1 - e0) * x;
	float b = e2 + (e3 - e2) * x;
	return a + (b - a) * y;
}


/* ======= DRAWING HELPERS ======= */

#define PAX_GFX_C

// Single-core rendering.
#include "drawing_helpers/pax_dh_unshaded.c"
#include "drawing_helpers/pax_dh_shaded.c"

// Multi-core rendering.
#ifdef PAX_COMPILE_MCR
#include "drawing_helpers/pax_dh_mcr_unshaded.c"
#include "drawing_helpers/pax_dh_mcr_shaded.c"
#endif

/* ============ DEBUG ============ */

// Describe error.
char *pax_desc_err(pax_err_t error) {
	char *unknown = "Unknown error";
	char *desc[] = {
		"Success",
		"No framebuffer",
		"No memory",
		"Invalid parameters",
		"Infinite parameters",
		"Out of bounds",
		"Matrix stack underflow",
		"Out of data",
		"Image decoding error"
	};
	size_t n_desc = sizeof(desc) / sizeof(char *);
	if (error > 0 || error < -n_desc) return unknown;
	else return desc[-error];
}

// Debug stuff.
void pax_debug(pax_buf_t *buf) {
	pax_col_t c0 = 0xff489be0;
	uint32_t  c1 = pax_col2buf(buf, c0);
	uint32_t  c2 = pax_buf2col(buf, c1);
	ESP_LOGW(TAG, "%06x -> %02x -> %06x", c0, c1, c2);
}



/* ===== MULTI-CORE RENDERING ==== */

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
			while (xQueueReceive(queue_handle, &tsk, 0)) {
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
		} else {
			// There's nothing else to do.
			taskYIELD();
		}
	}
	
	ESP_LOGI(TAG, "MCR worker stopped.");
	multicore_handle = NULL;
	vTaskDelete(NULL);
}
#endif

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



/* ============ BUFFER =========== */

// Create a new buffer.
// If mem is NULL, a new area is allocated.
void pax_buf_init(pax_buf_t *buf, void *mem, int width, int height, pax_buf_type_t type) {
	bool use_alloc = !mem;
	if (use_alloc) {
		// Allocate the right amount of bytes.
		ESP_LOGI(TAG, "Allocating new memory for buffer.");
		mem = malloc((PAX_GET_BPP(type) * width * height + 7) >> 3);
		if (!mem) PAX_ERROR("pax_buf_init", PAX_ERR_NOMEM);
	}
	*buf = (pax_buf_t) {
		.type       = type,
		.buf        = mem,
		.width      = width,
		.height     = height,
		.bpp        = PAX_GET_BPP(type),
		.stack_2d   = {
			.parent = NULL,
			.value  = matrix_2d_identity()
		},
		.do_free    = use_alloc
	};
	pax_mark_clean(buf);
	pax_noclip(buf);
	PAX_SUCCESS();
}

// Destroy the buffer, freeing its memory.
void pax_buf_destroy(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_buf_destroy");
	
	matrix_stack_2d_t *current = buf->stack_2d.parent;
	while (current) {
		matrix_stack_2d_t *next = current->parent;
		free(current);
		current = next;
	}
	if (buf->do_free) {
		free(buf->buf);
	}
	buf->buf  = NULL;
	buf->type = 0;
	
	PAX_SUCCESS();
}

// Convert the buffer to the given new format.
// If dest is NULL or equal to src, src will be converted.
void pax_buf_convert(pax_buf_t *dst, pax_buf_t *src, pax_buf_type_t type) {
	if (!(src) || !(src)->buf) PAX_ERROR("pax_buf_convert (src)", PAX_ERR_NOBUF);
	if (!(dst) || !(dst)->buf) PAX_ERROR("pax_buf_convert (dst)", PAX_ERR_NOBUF);
	
	// pax_buf_t dummy;
	// bool use_dummy = !dst;
	// if (use_dummy) {
	// 	dummy = *src;
	// 	dst = &dummy;
	// }
	
	// We can't go using realloc on an unknown buffer.
	if (!dst->do_free) PAX_ERROR("pax_buf_convert", PAX_ERR_PARAM);
	// Src and dst must match in size.
	if (src->width != dst->width || src->height != dst->height) {
		ESP_LOGE(TAG, "size mismatch: %dx%d vs %dx%d", src->width, src->height, dst->width, dst->height);
		PAX_ERROR("pax_buf_convert", PAX_ERR_BOUNDS);
	}
	
	dst->bpp = PAX_GET_BPP(type);
	dst->type = type;
	size_t new_pixels = dst->width * dst->height;
	size_t new_size = (new_pixels * dst->bpp + 7) / 8;
	if (dst->bpp > src->bpp) {
		ESP_LOGI(TAG, "Expanding buffer.");
		// Resize the memory for DST beforehand.
		dst->buf = realloc(dst->buf, new_size);
		if (!dst->buf) PAX_ERROR("pax_buf_convert", PAX_ERR_NOMEM);
		// Reverse iterate if the new BPP is larger than the old BPP.
		for (int y = dst->height - 1; y >= 0; y --) {
			for (int x = dst->width - 1; x >= 0; x --) {
				pax_col_t col_src = pax_get_pixel(src, x, y);
				pax_set_pixel(dst, col_src, x, y);
			}
		}
	} else {
		ESP_LOGI(TAG, "Shrinking buffer.");
		// Otherwise, iterate normally.
		for (int y = 0; y < dst->height; y ++) {
			for (int x = 0; x < dst->width; x ++) {
				pax_col_t col_src = pax_get_pixel(src, x, y);
				pax_set_pixel(dst, col_src, x, y);
			}
		}
		// Resize the memory for DST afterwards.
		dst->buf = realloc(dst->buf, new_size);
		if (!dst->buf) PAX_ERROR("pax_buf_convert", PAX_ERR_NOMEM);
	}
	
	// if (use_dummy) {
	// 	*src = dummy;
	// }
}

// Clip the buffer to the desired rectangle.
void pax_clip(pax_buf_t *buf, float x, float y, float width, float height) {
	// Make width and height positive.
	if (width < 0) {
		x += width;
		width = -width;
	}
	if (height < 0) {
		y += height;
		height = -height;
	}
	// Clip the entire rectangle to be at most the buffer's size.
	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > buf->width) {
		width = buf->width - x;
	}
	if (y + height > buf->height) {
		height = buf->height - y;
	}
	// Apply the clip.
	buf->clip = (pax_rect_t) {
		.x = x,
		.y = y,
		.w = width,
		.h = height
	};
}

// Clip the buffer to it's full size.
void pax_noclip(pax_buf_t *buf) {
	buf->clip = (pax_rect_t) {
		.x = 0,
		.y = 0,
		.w = buf->width,
		.h = buf->height
	};
}

// Check whether the buffer is dirty.
bool pax_is_dirty(pax_buf_t *buf) {
	PAX_BUF_CHECK1("pax_is_dirty", 0);
	return buf->dirty_x0 < buf->dirty_x1;
}

// Mark the entire buffer as clean.
void pax_mark_clean(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_mark_clean");
	buf->dirty_x0 = buf->width;
	buf->dirty_y0 = buf->height;
	buf->dirty_x1 = -1;
	buf->dirty_y1 = -1;
	PAX_SUCCESS();
}

// Mark the entire buffer as dirty.
void pax_mark_dirty0(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_mark_dirty0");
	buf->dirty_x0 = 0;
	buf->dirty_y0 = 0;
	buf->dirty_x1 = buf->width;
	buf->dirty_y1 = buf->height;
	PAX_SUCCESS();
}

// Mark a single point as dirty.
void pax_mark_dirty1(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK("pax_mark_dirty1");
	
	if (x < buf->dirty_x0) buf->dirty_x0 = x;
	if (x > buf->dirty_x1) buf->dirty_x1 = x;
	if (y < buf->dirty_y0) buf->dirty_y0 = y;
	if (y > buf->dirty_y1) buf->dirty_y1 = y;
	
	PAX_SUCCESS();
}

// Mark a rectangle as dirty.
void pax_mark_dirty2(pax_buf_t *buf, int x, int y, int width, int height) {
	PAX_BUF_CHECK("pax_mark_dirty2");
	
	if (x              < buf->dirty_x0) buf->dirty_x0 = x;
	if (x + width  - 1 > buf->dirty_x1) buf->dirty_x1 = x + width  - 1;
	if (y              < buf->dirty_x0) buf->dirty_y0 = y;
	if (y + height - 1 > buf->dirty_x1) buf->dirty_y1 = y + height - 1;
	
	PAX_SUCCESS();
}



/* ============ COLORS =========== */

// A linear interpolation based only on ints.
static inline uint8_t pax_lerp(uint8_t part, uint8_t from, uint8_t to) {
	return from + (( (to - from) * (part + (part >> 7)) ) >> 8);
}

// Converts HSV to ARGB.
pax_col_t pax_col_hsv(uint8_t h, uint8_t s, uint8_t v) {
	return pax_col_ahsv(255, h, s, v);
}

// Converts AHSV to ARGB.
pax_col_t pax_col_ahsv(uint8_t a, uint8_t _h, uint8_t s, uint8_t v) {
	uint16_t h     = _h * 6;
	uint16_t phase = h >> 8;
	// Parts of HSV.
	uint8_t up, down, other;
	other  = ~s;
	if (h & 0x100) {
		// Down goes away.
		up     = 0xff;
		down   = pax_lerp(s, 0xff, ~h & 0xff);
	} else {
		// Up comes in.
		up     = pax_lerp(s, 0xff,  h & 0xff);
		down   = 0xff;
	}
	// Apply brightness.
	up    = pax_lerp(v, 0, up);
	down  = pax_lerp(v, 0, down);
	other = pax_lerp(v, 0, other);
	// Apply to RGB.
	uint8_t r, g, b;
	switch (phase >> 1) {
		case 0:
			// From R to G.
			r = down; g = up; b = other;
			break;
		case 1:
			// From G to B.
			r = other; g = down; b = up;
			break;
		case 2:
			// From B to R.
			r = up; g = other; b = down;
			break;
		default:
			// Shut up, compiler.
			return 0;
	}
	// Merge.
	return (a << 24) | (r << 16) | (g << 8) | b;
}

// Linearly interpolates between from and to, including alpha.
pax_col_t pax_col_lerp(uint8_t part, pax_col_t from, pax_col_t to) {
	return (pax_lerp(part, from >> 24, to >> 24) << 24)
		 | (pax_lerp(part, from >> 16, to >> 16) << 16)
		 | (pax_lerp(part, from >>  8, to >>  8) <<  8)
		 |  pax_lerp(part, from,       to);
}

// Merges the two colors, based on alpha.
pax_col_t pax_col_merge(pax_col_t base, pax_col_t top) {
	if (!(top >> 24)) return base;
	if ((top >> 24) == 255) return top;
	uint8_t part = top >> 24;
	return (pax_lerp(part, base >> 24, 255)       << 24)
		 | (pax_lerp(part, base >> 16, top >> 16) << 16)
		 | (pax_lerp(part, base >>  8, top >>  8) <<  8)
		 |  pax_lerp(part, base,       top);
}

// Tints the color, commonly used for textures.
pax_col_t pax_col_tint(pax_col_t col, pax_col_t tint) {
	if (!tint) return 0;
	if (tint == -1) return col;
	return (pax_lerp(tint >> 24, 0, col >> 24) << 24)
		 | (pax_lerp(tint >> 16, 0, col >> 16) << 16)
		 | (pax_lerp(tint >>  8, 0, col >>  8) <<  8)
		 |  pax_lerp(tint,       0, col);
}


/* ============ MATRIX =========== */

// 2D rotation matrix: represents a 2D shearing.
matrix_2d_t matrix_2d_rotate(float angle) {
	float _cos = cosf(-angle);
	float _sin = sinf(-angle);
	return (matrix_2d_t) { .arr = {
		_cos, -_sin, 0,
		_sin,  _cos, 0
	}};
}

// 2D matrix: applies the transformation that b represents on to a.
matrix_2d_t matrix_2d_multiply(matrix_2d_t a, matrix_2d_t b) {
	// [a b c] [p q r] [ap+bs aq+bt ar+bu+c]
	// [d e f]*[s t u]=[dp+es dq+et dr+eu+f]
	// [0 0 1] [0 0 1] [0     0     1      ]
	return (matrix_2d_t) { .arr = {
		a.a0*b.a0 + a.a1*b.b0,   a.a0*b.a1 + a.a1*b.b1,  a.a0*b.a2 + a.a1*b.b2 + a.a2,
		a.b0*b.a0 + a.b1*b.b0,   a.b0*b.a1 + a.b1*b.b1,  a.b0*b.a2 + a.b1*b.b2 + a.b2
	}};
}

// 2D matrix: applies the transformation that a represents on to a point.
void matrix_2d_transform(matrix_2d_t a, float *x, float *y) {
	// [a b c] [x]  [a]  [b] [c] [ax+by+c]
	// [d e f]*[y]=x[d]+y[e]+[f]=[dx+ey+f]
	// [0 0 1] [1]  [0]  [0] [1] [1      ]
	float _x = *x, _y = *y;
	*x = a.a0*_x + a.a1*_y + a.a2;
	*y = a.b0*_x + a.b1*_y + a.b2;
}

// Apply the given matrix to the stack.
void pax_apply_2d(pax_buf_t *buf, matrix_2d_t a) {
	PAX_BUF_CHECK("pax_apply_2d");
	buf->stack_2d.value = matrix_2d_multiply(buf->stack_2d.value, a);
	PAX_SUCCESS();
}

// Push the current matrix up the stack.
void pax_push_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_push_2d");
	matrix_stack_2d_t *parent = malloc(sizeof(matrix_stack_2d_t));
	if (!parent) PAX_ERROR("pax_push_2d", PAX_ERR_NOMEM);
	*parent = buf->stack_2d;
	buf->stack_2d.parent = parent;
	PAX_SUCCESS();
}

// Pop the top matrix off the stack.
void pax_pop_2d(pax_buf_t *buf) {
	PAX_BUF_CHECK("pax_pop_2d");
	matrix_stack_2d_t *parent = buf->stack_2d.parent;
	if (!parent) PAX_ERROR("pax_pop_2d", PAX_ERR_UNDERFLOW);
	buf->stack_2d = *parent;
	free(parent);
	PAX_SUCCESS();
}

// Reset the matrix stack.
// If full is true, the entire stack gets cleared.
// Else, only the top element gets cleared.
void pax_reset_2d(pax_buf_t *buf, bool full) {
	if (full) {
		matrix_stack_2d_t *current = buf->stack_2d.parent;
		while (current) {
			matrix_stack_2d_t *next = current->parent;
			free(current);
			current = next;
		}
		buf->stack_2d.parent = NULL;
	}
	buf->stack_2d.value = matrix_2d_identity();
}



/* ======== DRAWING: PIXEL ======= */

// Set a pixel, merging with alpha.
void pax_merge_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK("pax_merge_pixel");
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	if (PAX_IS_PALETTE(buf->type)) {
		if (color & 0xff000000)
			pax_set_pixel_u(buf, color, x, y);
	} else if (color >= 0xff000000) {
		pax_set_pixel_u(buf, pax_col2buf(buf, color), x, y);
	} else {
		pax_col_t base = pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
		pax_set_pixel_u(buf, pax_col2buf(buf, pax_col_merge(base, color)), x, y);
	}
}

// Set a pixel.
void pax_set_pixel(pax_buf_t *buf, pax_col_t color, int x, int y) {
	PAX_BUF_CHECK("pax_set_pixel");
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return;
	}
	PAX_SUCCESS();
	if (PAX_IS_PALETTE(buf->type)) {
		pax_set_pixel_u(buf, color, x, y);
	} else {
		pax_set_pixel_u(buf, pax_col2buf(buf, color), x, y);
	}
}

// Get a pixel.
pax_col_t pax_get_pixel(pax_buf_t *buf, int x, int y) {
	PAX_BUF_CHECK1("pax_get_pixel", 0);
	if (x < 0 || x >= buf->width || y < 0 || y >= buf->height) {
		// This won't do.
		pax_last_error = PAX_ERR_BOUNDS;
		return 0;
	}
	PAX_SUCCESS();
	return pax_buf2col(buf, pax_get_pixel_u(buf, x, y));
}



/* ========= DRAWING: 2D ========= */

// Draw a rectangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_rect(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x, float y, float width, float height) {
	if (!shader) {
		// If shader is NULL, simplify this.
		pax_draw_rect(buf, color, x, y, width, height);
		return;
	}
	
	if (!uvs) {
		uvs = &(pax_quad_t) {
			.x0 = 0, .y0 = 0,
			.x1 = 1, .y1 = 0,
			.x2 = 1, .y2 = 1,
			.x3 = 0, .y3 = 1
		};
	}
	
	pax_tri_t uv0 = {
		.x0 = uvs->x0, .y0 = uvs->y0,
		.x1 = uvs->x1, .y1 = uvs->y1,
		.x2 = uvs->x2, .y2 = uvs->y2
	};
	pax_tri_t uv1 = {
		.x0 = uvs->x0, .y0 = uvs->y0,
		.x1 = uvs->x3, .y1 = uvs->y3,
		.x2 = uvs->x2, .y2 = uvs->y2
	};
	
	if (matrix_2d_is_identity2(buf->stack_2d.value)) {
		// Simplify this.
		matrix_2d_transform(buf->stack_2d.value, &x, &y);
		width  *= buf->stack_2d.value.a0;
		height *= buf->stack_2d.value.b1;
		pax_mark_dirty2(buf, x - 0.5, y - 0.5, width + 1, height + 1);
		#ifdef PAX_COMPILE_MCR
		if (pax_do_multicore) {
			float shape[4] = {
				x, y, width, height
			};
			pax_task_t task = {
				.buffer    = buf,
				.type      = PAX_TASK_RECT,
				.color     = color,
				.shader    = shader,
				.quad_uvs  = uvs,
				.shape     = shape,
				.shape_len = 4
			};
			paxmcr_add_task(&task);
			paxmcr_rect_shaded(
				false,
				buf, color, shader, x, y, width, height,
				uvs->x0, uvs->y0, uvs->x1, uvs->y1,
				uvs->x2, uvs->y2, uvs->x3, uvs->y3
			);
		} else
		#endif
		{
			pax_rect_shaded(
				buf, color, shader, x, y, width, height,
				uvs->x0, uvs->y0, uvs->x1, uvs->y1,
				uvs->x2, uvs->y2, uvs->x3, uvs->y3
			);
		}
	} else {
		// We still need triangles.
		pax_shade_tri(buf, color, shader, &uv0, x, y, x + width, y, x + width, y + height);
		pax_shade_tri(buf, color, shader, &uv1, x, y, x, y + height, x + width, y + height);
	}
}

// Draw a triangle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 0,1).
void pax_shade_tri(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_tri_t *uvs, float x0, float y0, float x1, float y1, float x2, float y2) {
	if (!shader) {
		// If shader is NULL, simplify this.
		pax_draw_tri(buf, color, x0, y0, x1, y1, x2, y2);
		return;
	}
	
	PAX_BUF_CHECK("pax_shade_tri");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	if (!uvs) {
		uvs = &(pax_tri_t) {
			.x0 = 0, .y0 = 0,
			.x1 = 1, .y1 = 0,
			.x2 = 0, .y2 = 1
		};
	}
	
	// Sort points by height.
	if (y1 < y0) {
		PAX_SWAP_POINTS(x0, y0, x1, y1);
		PAX_SWAP_POINTS(uvs->x0, uvs->y0, uvs->x1, uvs->y1);
	}
	if (y2 < y0) {
		PAX_SWAP_POINTS(x0, y0, x2, y2);
		PAX_SWAP_POINTS(uvs->x0, uvs->y0, uvs->x2, uvs->y2);
	}
	if (y2 < y1) {
		PAX_SWAP_POINTS(x1, y1, x2, y2);
		PAX_SWAP_POINTS(uvs->x1, uvs->y1, uvs->x2, uvs->y2);
	}
	
	if (y2 == y0 || (x2 == x0 && x1 == x0)) {
		// We can't draw a flat triangle.
		PAX_SUCCESS();
		return;
	}
	
	pax_mark_dirty1(buf, x0 - 0.5, y0 - 0.5);
	pax_mark_dirty1(buf, x1 - 0.5, y1 - 0.5);
	pax_mark_dirty1(buf, x2 - 0.5, y2 - 0.5);
	pax_mark_dirty1(buf, x0 + 0.5, y0 + 0.5);
	pax_mark_dirty1(buf, x1 + 0.5, y1 + 0.5);
	pax_mark_dirty1(buf, x2 + 0.5, y2 + 0.5);
	
	#ifdef PAX_COMPILE_MCR
	if (pax_do_multicore) {
		float shape[6] = {
			x0, y0, x1, y1, x2, y2
		};
		pax_task_t task = {
			.buffer    = buf,
			.type      = PAX_TASK_TRI,
			.color     = color,
			.shader    = shader,
			.tri_uvs   = uvs,
			.shape     = shape,
			.shape_len = 6
		};
		paxmcr_add_task(&task);
		paxmcr_tri_shaded(
			false, buf, color, shader,
			x0, y0, x1, y1, x2, y2,
			uvs->x0, uvs->y0, uvs->x1, uvs->y1, uvs->x2, uvs->y2
		);
	} else
	#endif
	{
		pax_tri_shaded(
			buf, color, shader,
			x0, y0, x1, y1, x2, y2,
			uvs->x0, uvs->y0, uvs->x1, uvs->y1, uvs->x2, uvs->y2
		);
	}
	
	PAX_SUCCESS();
}

// Draw an arc with a shader, angles in radians.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_arc(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x,  float y,  float r,  float a0, float a1) {
	if (!shader) {
		// If shader is NULL, simplify this.
		pax_draw_arc(buf, color, x, y, r, a0, a1);
		return;
	}
	
	PAX_BUF_CHECK("pax_draw_arc");
	if (!uvs) {
		uvs = &(pax_quad_t) {
			.x0 = 0, .y0 = 0,
			.x1 = 1, .y1 = 0,
			.x2 = 1, .y2 = 1,
			.x3 = 0, .y3 = 1
		};
	}
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	matrix_2d_t matrix = buf->stack_2d.value;
	float _r = r * sqrtf(matrix.a0*matrix.a0 + matrix.b0*matrix.b0) * sqrtf(matrix.a1*matrix.a1 + matrix.b1*matrix.b1);
	if (_r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	else n_div = (a1 - a0) / M_PI * 16 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	pax_tri_t tri_uvs;
	tri_uvs.x0 = (uvs->x0 + uvs->x1 + uvs->x2 + uvs->x3) * 0.25;
	tri_uvs.y0 = (uvs->y0 + uvs->y1 + uvs->y2 + uvs->y3) * 0.25;
	
	tri_uvs.x1 = pax_flerp4(x0, y0, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
	tri_uvs.y1 = pax_flerp4(x0, y0, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// And UV interpolation.
		tri_uvs.x2 = pax_flerp4(x1, y1, uvs->x0, uvs->x1, uvs->x3, uvs->x2);
		tri_uvs.y2 = pax_flerp4(x1, y1, uvs->y0, uvs->y1, uvs->y3, uvs->y2);
		// We subtract y0 and y1 from y because our up is -y.
		pax_shade_tri(buf, color, shader, &tri_uvs, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
		tri_uvs.x1 = tri_uvs.x2;
		tri_uvs.y1 = tri_uvs.y2;
	}
	
	PAX_SUCCESS();
}

// Draw a circle with a shader.
// If uvs is NULL, a default will be used (0,0; 1,0; 1,1; 0,1).
void pax_shade_circle(pax_buf_t *buf, pax_col_t color, pax_shader_t *shader,
		pax_quad_t *uvs, float x,  float y,  float r) {
	pax_shade_arc(buf, color, shader, uvs, x, y, r, 0, 2*M_PI);
}

// Draw a rectangle.
void pax_draw_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK("pax_draw_rect");
	if (matrix_2d_is_identity2(buf->stack_2d.value)) {
		// This can be simplified significantly.
		matrix_2d_transform(buf->stack_2d.value, &x, &y);
		width  *= buf->stack_2d.value.a0;
		height *= buf->stack_2d.value.b1;
		pax_simple_rect(buf, color, x, y, width, height);
	} else {
		// We need to go full quad.
		float x0 = x,         y0 = y;
		float x1 = x + width, y1 = y;
		float x2 = x + width, y2 = y + height;
		float x3 = x,         y3 = y + height;
		matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
		matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
		matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
		matrix_2d_transform(buf->stack_2d.value, &x3, &y3);
		pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
		pax_simple_tri(buf, color, x0, y0, x3, y3, x2, y2);
	}
}

// Draw a line.
void pax_draw_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK("pax_draw_line");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	pax_simple_line(buf, color, x0, y0, x1, y1);
}

// Draw a triangle.
void pax_draw_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK("pax_draw_tri");
	matrix_2d_transform(buf->stack_2d.value, &x0, &y0);
	matrix_2d_transform(buf->stack_2d.value, &x1, &y1);
	matrix_2d_transform(buf->stack_2d.value, &x2, &y2);
	pax_simple_tri(buf, color, x0, y0, x1, y1, x2, y2);
}

// Draw na arc, angles in radians.
void pax_draw_arc(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r,  float a0, float a1) {
	PAX_BUF_CHECK("pax_draw_arc");
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	matrix_2d_t matrix = buf->stack_2d.value;
	float _r = r * sqrtf(matrix.a0*matrix.a0 + matrix.b0*matrix.b0) * sqrtf(matrix.a1*matrix.a1 + matrix.b1*matrix.b1);
	if (_r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	else n_div = (a1 - a0) / M_PI * 16 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_draw_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle.
void pax_draw_circle(pax_buf_t *buf, pax_col_t color, float x,  float y,  float r) {
	pax_draw_arc(buf, color, x, y, r, 0, M_PI * 2);
}



/* ======= DRAWING: TEXT ======= */

// Determine whether a character is visible.
// Includes space.
static inline bool pax_is_visible_char(char c) {
	return c > 0x1f && c < 0x7f;
}

// Draw a string with the given font.
// If font is NULL, the default font (7x9) will be used.
void pax_draw_text(pax_buf_t *buf, pax_col_t color, pax_font_t *font, float font_size, float _x, float _y, char *text) {
	PAX_BUF_CHECK("pax_draw_text");
	
	if (!text || !*text) return;
	if (color < 0x01000000) return;
	if (!font) font = PAX_FONT_DEFAULT;
	
	if (font_size == 0) font_size = font->glyphs_uni_h;
	float size_mul = font_size / font->glyphs_uni_h;
	float w = size_mul * font->glyphs_uni_w;
	float h = size_mul * font->glyphs_uni_h;
	
	size_t len = strlen(text);
	
	float x = _x, y = _y;
	
	pax_shader_font_bitmap_uni_args_t args = {
		.font          = font
	};
	pax_shader_t shader = {
		.callback          = pax_shader_font_bitmap_uni,
		.callback_args     = &args,
		.alpha_promise_0   = true,
		.alpha_promise_255 = false
	};
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			x = _x;
			y += h;
			if (c == '\r' && next == '\n') i++;
		} else {
			pax_join();
			args.glyph = pax_is_visible_char(c) ? c : 1;
			pax_shade_rect(buf, color, &shader, NULL, x, y, w, h);
			x += w;
		}
	}
	pax_join();
	
	PAX_SUCCESS();
}

// Calculate the size of the string with the given font.
// Size is before matrix transformation.
// If font is NULL, the default font (7x9) will be used.
pax_vec1_t pax_text_size(pax_font_t *font, float font_size, char *text) {
	if (!font) font = PAX_FONT_DEFAULT;
	
	if (font_size == 0) font_size = font->glyphs_uni_h;
	float size_mul = font_size / font->glyphs_uni_h;
	float w = size_mul * font->glyphs_uni_w;
	float h = size_mul * font->glyphs_uni_h;
	
	float text_w = 0;
	float text_h = h;
	
	size_t len = strlen(text);
	
	float x = 0, y = 0;
	
	for (size_t i = 0; i < len; i ++) {
		char c = text[i], next = text[i + 1];
		if (c == '\r' || c == '\n') {
			x = 0;
			y += h;
			text_h = y + h;
			if (c == '\r' && next == '\n') i++;
		} else {
			x += w;
			if (x > text_w) text_w = x;
		}
	}
	
	return (pax_vec1_t) { .x = text_w, .y = text_h };
}



/* ======= DRAWING: SIMPLE ======= */

// Fill the background.
void pax_background(pax_buf_t *buf, pax_col_t color) {
	PAX_BUF_CHECK("pax_background");
	
	#ifdef PAX_COMPILE_MCR
	pax_join();
	#endif
	
	uint32_t value;
	if (PAX_IS_PALETTE(buf->type)) {
		if (color > buf->pallette_size) value = 0;
		else value = color;
	} else {
		value = pax_col2buf(buf, color);
	}
	
	if (buf->bpp == 16) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_16bpp[i] = value;
		}
	} else if (buf->bpp == 32) {
		for (size_t i = 0; i < buf->width * buf->height; i++) {
			buf->buf_32bpp[i] = value;
		}
	} else {
		if      (buf->bpp == 1) value = -value;
		else if (buf->bpp == 2) value = value * 0x55;
		else if (buf->bpp == 4) value = value * 0x11;
		size_t limit = (7 + buf->width * buf->height * buf->bpp) / 8;
		for (size_t i = 0; i < limit; i++) {
			buf->buf_8bpp[i] = value;
		}
	}
	
	PAX_SUCCESS();
}

// Draw a rectangle, ignoring matrix transform.
void pax_simple_rect(pax_buf_t *buf, pax_col_t color, float x, float y, float width, float height) {
	PAX_BUF_CHECK("pax_simple_rect");
	if (color < 0x01000000) {
		PAX_SUCCESS();
		return;
	}
	
	// Fix rect dimensions.
	if (width < 0) {
		width = -width;
		x -= width;
	}
	if (height < 0) {
		height = -height;
		y -= height;
	}
	
	// Clip rect in inside of buffer.
	if (x < buf->clip.x) {
		width += buf->clip.x - x;
		x = buf->clip.x;
	}
	if (y < buf->clip.y) {
		height += buf->clip.y - y;
		y = buf->clip.y;
	}
	if (x + width > buf->clip.x + buf->clip.w) {
		width = buf->clip.x + buf->clip.w - x;
	}
	if (y + height > buf->clip.y + buf->clip.h) {
		height = buf->clip.y + buf->clip.h - y;
	}
	
	pax_mark_dirty2(buf, x - 0.5, y - 0.5, width + 1, height + 1);
	#ifdef PAX_COMPILE_MCR
	if (pax_do_multicore) {
		float shape[4] = {
			x, y, width, height
		};
		pax_task_t task = {
			.buffer    = buf,
			.type      = PAX_TASK_RECT,
			.color     = color,
			.shader    = NULL,
			.shape     = shape,
			.shape_len = 4
		};
		paxmcr_add_task(&task);
		paxmcr_rect_unshaded(false, buf, color, x, y, width, height);
	} else
	#endif
	{
		pax_rect_unshaded(buf, color, x, y, width, height);
	}
	
	PAX_SUCCESS();
}

// Draw a line, ignoring matrix transform.
void pax_simple_line(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1) {
	PAX_BUF_CHECK("pax_simple_line");
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	if (y0 > y1) PAX_SWAP_POINTS(x0, y0, x1, y1);
	
	// Determine whether the line might fall within the clip rect.
	if (!buf->clip.w || !buf->clip.h) goto noneed;
	if (y1 < buf->clip.y || y0 > buf->clip.y + buf->clip.h - 1) goto noneed;
	if (x0 == x1 && (x0 < buf->clip.x || x0 > buf->clip.x + buf->clip.w - 1)) goto noneed;
	if (x0 < buf->clip.x && x1 < buf->clip.x) goto noneed;
	if (x0 > buf->clip.x + buf->clip.w - 1 && x1 > buf->clip.x + buf->clip.w - 1) goto noneed;
	
	// Clip top.
	if (y0 < buf->clip.y) {
		x0 = x0 + (x1 - x0) * (buf->clip.y - y0) / (y1 - y0);
		y0 = buf->clip.y;
	}
	// Clip bottom.
	if (y1 > buf->clip.y + buf->clip.h - 1) {
		x1 = x0 + (x1 - x0) * (buf->clip.y + buf->clip.h - 1 - y0) / (y1 - y0);
		y1 = buf->clip.y + buf->clip.h - 1;
	}
	// Clip left.
	if (x1 < buf->clip.x) {
		y1 = y0 + (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x1 = buf->clip.x;
	} else if (x0 < buf->clip.x) {
		y0 = y0 + (y1 - y0) * (buf->clip.x - x0) / (x1 - x0);
		x0 = buf->clip.x;
	}
	// Clip right.
	if (x1 > buf->clip.x + buf->clip.w - 1) {
		y1 = y0 + (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x1 = buf->clip.x + buf->clip.w - 1;
	} else if (x0 > buf->clip.x + buf->clip.w - 1) {
		y0 = y0 + (y1 - y0) * (buf->clip.x + buf->clip.w - 1 - x0) / (x1 - x0);
		x0 = buf->clip.x + buf->clip.w - 1;
	}
	
	// If any point is outside clip now, we don't draw a line.
	if (y0 < buf->clip.y || y1 > buf->clip.y + buf->clip.h - 1) goto noneed;
	
	pax_mark_dirty1(buf, x0, y0);
	pax_mark_dirty1(buf, x1, y1);
	#ifdef PAX_COMPILE_MCR
	pax_join();
	#endif
	pax_line_unshaded(buf, color, x0, y0, x1, y1);
	
	// This label is used if there's no need to try to draw a line.
	noneed:
	PAX_SUCCESS();
}

// Draw a triangle, ignoring matrix transform.
void pax_simple_tri(pax_buf_t *buf, pax_col_t color, float x0, float y0, float x1, float y1, float x2, float y2) {
	PAX_BUF_CHECK("pax_simple_tri");
	
	if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) || !isfinite(x2) || !isfinite(y2)) {
		// We can't draw to infinity.
		pax_last_error = PAX_ERR_INF;
		return;
	}
	
	// Sort points by height.
	PAX_SORT_POINTS(x0, y0, x1, y1);
	PAX_SORT_POINTS(x0, y0, x2, y2);
	PAX_SORT_POINTS(x1, y1, x2, y2);
	
	if (y2 == y0 || (x2 == x0 && x1 == x0)) {
		// We can't draw a flat triangle.
		PAX_SUCCESS();
		return;
	}
	
	pax_mark_dirty1(buf, x0 - 0.5, y0 - 0.5);
	pax_mark_dirty1(buf, x1 - 0.5, y1 - 0.5);
	pax_mark_dirty1(buf, x2 - 0.5, y2 - 0.5);
	pax_mark_dirty1(buf, x0 + 0.5, y0 + 0.5);
	pax_mark_dirty1(buf, x1 + 0.5, y1 + 0.5);
	pax_mark_dirty1(buf, x2 + 0.5, y2 + 0.5);
	#ifdef PAX_COMPILE_MCR
	if (pax_do_multicore) {
		float shape[6] = {
			x0, y0, x1, y1, x2, y2
		};
		pax_task_t task = {
			.buffer    = buf,
			.type      = PAX_TASK_TRI,
			.color     = color,
			.shader    = NULL,
			.shape     = shape,
			.shape_len = 6
		};
		paxmcr_add_task(&task);
		paxmcr_tri_unshaded(false, buf, color, x0, y0, x1, y1, x2, y2);
	} else
	#endif
	{
		pax_tri_unshaded(buf, color, x0, y0, x1, y1, x2, y2);
	}
	
	PAX_SUCCESS();
}

// Draw a arc, ignoring matrix transform.
// Angles in radians.
void pax_simple_arc(pax_buf_t *buf, pax_col_t color, float x, float y, float r, float a0, float a1) {
	PAX_BUF_CHECK("pax_simple_arc");
	
	// Simplify the angles slightly.
	float a2 = fmodf(a0, M_PI * 2);
	a1 += a2 - a0;
	a0 = a2;
	if (a1 < a0) PAX_SWAP(float, a0, a1);
	if (a1 - a0 > M_PI * 2) {
		a1 = M_PI * 2;
		a0 = 0;
	}
	
	// Pick an appropriate number of divisions.
	int n_div;
	if (r > 30) n_div = (a1 - a0) / M_PI * 32 + 1;
	if (r > 20) n_div = (a1 - a0) / M_PI * 16 + 1;
	else n_div = (a1 - a0) / M_PI * 8 + 1;
	
	// Get the sine and cosine of one division, used for rotation in the loop.
	float div_angle = (a1 - a0) / n_div;
	float _sin = sinf(div_angle);
	float _cos = cosf(div_angle);
	
	// Start with a unit vector matrix according to a0.
	float x0 = cosf(a0);
	float y0 = sinf(a0);
	
	// Draw it as a series of triangles, rotating with what is essentially matrix multiplication.
	for (int i = 0; i < n_div; i++) {
		// Perform the rotation.
		float x1 = x0 * _cos - y0 * _sin;
		float y1 = x0 * _sin + y0 * _cos;
		// We subtract y0 and y1 from y because our up is -y.
		pax_simple_tri(buf, color, x, y, x + x0 * r, y - y0 * r, x + x1 * r, y - y1 * r);
		// Assign them yes.
		x0 = x1;
		y0 = y1;
	}
	
	PAX_SUCCESS();
}

// Draw a circle, ignoring matrix transform.
void pax_simple_circle(pax_buf_t *buf, pax_col_t color, float x, float y, float r) {
	pax_simple_arc(buf, color, x, y, r, 0, M_PI * 2);
}
