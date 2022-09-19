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

#ifndef PAX_INTERNAL_H
#define PAX_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "pax_gfx.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if (defined(ESP32) || defined(ESP8266) || defined(ESP_PLATFORM)) && !defined(PAX_ESP_IDF)
#define PAX_ESP_IDF
#endif

#ifdef PAX_ESP_IDF
#include <esp_system.h>
#include <sdkconfig.h>
#include <esp_err.h>
#include <esp_log.h>

#define PAX_LOGE(...) ESP_LOGE(__VA_ARGS__)
#define PAX_LOGI(...) ESP_LOGI(__VA_ARGS__)
#define PAX_LOGD(...) ESP_LOGE(__VA_ARGS__)
#define PAX_LOGW(...) ESP_LOGW(__VA_ARGS__)
#else

#include <pthread.h>

extern pthread_mutex_t pax_log_mutex;
extern bool pax_log_use_mutex;

#define PRIVATE_PAX_LOG_HELPER(file, prefix, tag, ...) do {\
		if (pax_log_use_mutex) {\
			pthread_mutex_lock(&pax_log_mutex);\
		}\
		fprintf(file, prefix "%s: ", (tag));\
		fprintf(file, __VA_ARGS__);\
		fputs("\033[0m\r\n", file);\
		if (pax_log_use_mutex) {\
			pthread_mutex_unlock(&pax_log_mutex);\
		}\
	} while(0)

#define PAX_LOGE(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[91mError ", tag, __VA_ARGS__)
#define PAX_LOGI(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[32mInfo  ", tag, __VA_ARGS__)
#define PAX_LOGD(tag, ...) PRIVATE_PAX_LOG_HELPER(stdout, "\033[94mDebug ", tag, __VA_ARGS__)
#define PAX_LOGW(tag, ...) PRIVATE_PAX_LOG_HELPER(stderr, "\033[33mWarn  ", tag, __VA_ARGS__)
#endif

/* =========== HELPERS =========== */

// Whether multi-core rendering is enabled.
extern bool pax_do_multicore;

// Name used in log output.
static const char *TAG   = "pax";

// Helper for setting pixels in drawing routines.
// Used to allow optimising away alpha in colors.
typedef void (*pax_setter_t)(pax_buf_t *buf, pax_col_t color, int x, int y);

// Helper for setting pixels in drawing routines.
// Used to allow optimising away color conversion.
typedef void (*pax_index_setter_t)(pax_buf_t *buf, pax_col_t color, int index);

// Macros for errors.
#ifdef PAX_AUTOREPORT
#define PAX_ERROR(where, errno) { pax_report_error(where, errno); pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { pax_report_error(where, errno); pax_last_error = errno; return retval; }
#else
#define PAX_ERROR(where, errno) { pax_last_error = errno; return; }
#define PAX_ERROR1(where, errno, retval) { pax_last_error = errno; return retval; }
#endif

#define PAX_SUCCESS() { pax_last_error = PAX_OK; }

// Buffer sanity check.
#define PAX_BUF_CHECK(where) { if (!(buf) || !(buf)->buf) PAX_ERROR(where, PAX_ERR_NOBUF); }
// Buffer sanity check.
#define PAX_BUF_CHECK1(where, retval) { if (!(buf) || !(buf)->buf) PAX_ERROR1(where, PAX_ERR_NOBUF, retval); }

// Swap two variables.
#define PAX_SWAP(type, a, b) { type tmp = a; a = b; b = tmp; }
// Swap two points represented by floats.
#define PAX_SWAP_POINTS(x0, y0, x1, y1) { float tmp = x1; x1 = x0; x0 = tmp; tmp = y1; y1 = y0; y0 = tmp; }
// Sort two points represented by floats.
#define PAX_SORT_POINTS(x0, y0, x1, y1) { if (y1 < y0) PAX_SWAP_POINTS(x0, y0, x1, y1) }

// Gets the most efficient index setter for the occasion.
// Also converts the color, if applicable.
// Returns NULL when setting is not required.
pax_index_setter_t pax_get_setter(pax_buf_t *buf, pax_col_t *col, const pax_shader_t *shader);
// Gets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
pax_col_t pax_get_index(pax_buf_t *buf, int index);
// Sets based on index instead of coordinates.
// Does no bounds checking nor color conversion.
void pax_set_index(pax_buf_t *buf, pax_col_t col, int index);
// Sets based on index instead of coordinates.
// Does no bounds checking.
void pax_set_index_conv(pax_buf_t *buf, pax_col_t col, int index);
// Merges based on index instead of coordinates.
// Does no bounds checking.
void pax_merge_index(pax_buf_t *buf, pax_col_t col, int index);

void pax_report_error(const char *where, pax_err_t errno);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //PAX_INTERNAL_H
