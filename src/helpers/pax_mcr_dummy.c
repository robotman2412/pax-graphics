
// SPDX-License-Identifier: MIT

#include "../pax_internal.h"

#if !defined(PAX_COMPILE_MCR) || (!defined(PAX_ESP_IDF) && !defined(PAX_STANDALONE))

/* ===== MULTI-CORE RENDERING ==== */

// If multi-core rendering is enabled, wait for the other core.
void pax_join() {
    // No MCR, no wait.
}

// Enable multi-core rendering.
void pax_enable_multicore(int core) {
    PAX_LOGE(TAG, "Failed to enable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
}

// Disable multi-core rendering.
void pax_disable_multicore() {
    PAX_LOGE(TAG, "No need to disable MCR: MCR not compiled, please define PAX_COMPILE_MCR.");
}

#endif
