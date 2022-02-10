
#ifndef PAX_CONFIG_H
#define PAX_CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Tell PAX to immediately report errors instead of just setting pax_last_error.
#define PAX_AUTOREPORT

// Compile in multi-core rendering.
#define PAX_COMPILE_MCR

// Queue size to use for multi-core rendering.
#define PAX_QUEUE_SIZE 30

#endif //PAX_CONFIG_H
