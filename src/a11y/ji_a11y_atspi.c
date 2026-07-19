/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_a11y_atspi.c
 * @brief AT-SPI2 backend for Linux accessibility.
 *        On systems without libatspi, provides stub implementations.
 */

#include "jiui/ji_accessibility.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* =========================================================================
 * AT-SPI2 Backend
 *
 * On Linux with libatspi2 installed, this would connect to the AT-SPI bus
 * and emit accessibility events. For portability, we provide stubs that
 * can be linked against when libatspi2 is not available.
 * ========================================================================= */

static bool s_atspi_available = false;
static bool s_atspi_initialized = false;

int ji_a11y_atspi_init(void)
{
    /* In a real implementation, this would call atspi_init() and register
     * the application with the AT-SPI bus. */
    s_atspi_initialized = true;
    s_atspi_available = false;   /* Not connected in stub mode */
    return 0;
}

void ji_a11y_atspi_shutdown(void)
{
    s_atspi_initialized = false;
    s_atspi_available = false;
}

int ji_a11y_atspi_emit_event(uint32_t id, const char* event_type, const char* data)
{
    if (!s_atspi_initialized) return -1;
    /* In a real implementation, this would emit a D-Bus signal on the
     * AT-SPI bus. In stub mode, we just return success. */
    (void)id;
    (void)event_type;
    (void)data;
    return 0;
}

bool ji_a11y_atspi_is_available(void)
{
    return s_atspi_available;
}
