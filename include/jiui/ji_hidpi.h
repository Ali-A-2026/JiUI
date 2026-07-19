/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_hidpi.h
 * @brief HiDPI support — scale factor detection, per-monitor DPI,
 *        automatic resource scaling (@2x, @3x), fractional scaling,
 *        sharp rendering at any DPI.
 */

#ifndef JIUI_HIDPI_H
#define JIUI_HIDPI_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * HiDPI Constants
 * ========================================================================= */

#define JI_HIDPI_MAX_MONITORS    16
#define JI_HIDPI_SCALE_MIN       0.5
#define JI_HIDPI_SCALE_MAX       5.0

/* =========================================================================
 * HiDPI Data Structures
 * ========================================================================= */

/** Monitor DPI information. */
typedef struct JiMonitorDpi {
    int      id;                 /* Monitor ID */
    int      x, y;               /* Monitor position in virtual desktop */
    int      width;              /* Monitor width in physical pixels */
    int      height;             /* Monitor height in physical pixels */
    int      dpi;                /* Physical DPI (e.g. 96, 144, 192) */
    double   scale;              /* Scale factor (1.0, 1.25, 1.5, 2.0, 3.0) */
    bool     fractional;         /* True if scale is fractional (1.25, 1.5, 1.75) */
} JiMonitorDpi;

/** HiDPI manager state. */
typedef struct JiHidpiManager {
    JiMonitorDpi monitors[JI_HIDPI_MAX_MONITORS];
    int          monitor_count;
    double       global_scale;   /* Global scale override (0 = auto) */
    bool         auto_detect;    /* Auto-detect DPI from system */
} JiHidpiManager;

/* =========================================================================
 * HiDPI API — Lifecycle
 * ========================================================================= */

JI_API JiHidpiManager* ji_hidpi_new(void);
JI_API void             ji_hidpi_free(JiHidpiManager* mgr);

/* =========================================================================
 * HiDPI API — Monitor Management
 * ========================================================================= */

JI_API int  ji_hidpi_add_monitor(JiHidpiManager* mgr, int id, int x, int y,
                                    int width, int height, int dpi);
JI_API int  ji_hidpi_get_monitor_count(const JiHidpiManager* mgr);
JI_API const JiMonitorDpi* ji_hidpi_get_monitor(const JiHidpiManager* mgr, int index);
JI_API const JiMonitorDpi* ji_hidpi_get_monitor_at(const JiHidpiManager* mgr,
                                                     int x, int y);

/* =========================================================================
 * HiDPI API — Scale Factor
 * ========================================================================= */

JI_API double ji_hidpi_get_scale(const JiHidpiManager* mgr, int monitor_id);
JI_API double ji_hidpi_get_scale_at(const JiHidpiManager* mgr, int x, int y);
JI_API void   ji_hidpi_set_global_scale(JiHidpiManager* mgr, double scale);
JI_API double ji_hidpi_get_global_scale(const JiHidpiManager* mgr);
JI_API void   ji_hidpi_set_auto_detect(JiHidpiManager* mgr, bool enabled);

/* =========================================================================
 * HiDPI API — Coordinate Conversion
 * ========================================================================= */

JI_API void ji_hidpi_dip_to_pixel(const JiHidpiManager* mgr, int monitor_id,
                                     double dip_x, double dip_y,
                                     int* pixel_x, int* pixel_y);
JI_API void ji_hidpi_pixel_to_dip(const JiHidpiManager* mgr, int monitor_id,
                                     int pixel_x, int pixel_y,
                                     double* dip_x, double* dip_y);

/* =========================================================================
 * HiDPI API — Resource Scaling
 * ========================================================================= */

/** Returns the appropriate resource suffix for a scale factor.
 *  e.g. 2.0 → "@2x", 3.0 → "@3x", 1.5 → "@1_5x" */
JI_API int  ji_hidpi_get_resource_suffix(double scale, char* out, int out_size);

/** Selects the best available resource scale from a list of available scales. */
JI_API double ji_hidpi_select_best_scale(double target, const double* available,
                                           int count);

/* =========================================================================
 * HiDPI API — DPI Detection
 * ========================================================================= */

JI_API int  ji_hidpi_detect_dpi(void);   /* Returns detected DPI or 96 */
JI_API double ji_hidpi_dpi_to_scale(int dpi);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_HIDPI_H */
