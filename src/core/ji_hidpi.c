/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_hidpi.c
 * @brief HiDPI support implementation.
 */

#include "jiui/ji_hidpi.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiHidpiManager* ji_hidpi_new(void)
{
    JiHidpiManager* mgr = (JiHidpiManager*)calloc(1, sizeof(JiHidpiManager));
    if (!mgr) return NULL;
    mgr->global_scale = 0.0;   /* Auto */
    mgr->auto_detect = true;
    return mgr;
}

void ji_hidpi_free(JiHidpiManager* mgr)
{
    if (mgr) free(mgr);
}

/* =========================================================================
 * Monitor Management
 * ========================================================================= */

int ji_hidpi_add_monitor(JiHidpiManager* mgr, int id, int x, int y,
                            int width, int height, int dpi)
{
    if (!mgr || mgr->monitor_count >= JI_HIDPI_MAX_MONITORS) return -1;
    JiMonitorDpi* m = &mgr->monitors[mgr->monitor_count++];
    memset(m, 0, sizeof(*m));
    m->id = id;
    m->x = x;
    m->y = y;
    m->width = width;
    m->height = height;
    m->dpi = dpi;
    m->scale = ji_hidpi_dpi_to_scale(dpi);
    m->fractional = (m->scale != (double)(int)m->scale);
    return mgr->monitor_count - 1;
}

int ji_hidpi_get_monitor_count(const JiHidpiManager* mgr)
{
    return mgr ? mgr->monitor_count : 0;
}

const JiMonitorDpi* ji_hidpi_get_monitor(const JiHidpiManager* mgr, int index)
{
    if (!mgr || index < 0 || index >= mgr->monitor_count) return NULL;
    return &mgr->monitors[index];
}

const JiMonitorDpi* ji_hidpi_get_monitor_at(const JiHidpiManager* mgr, int x, int y)
{
    if (!mgr) return NULL;
    for (int i = 0; i < mgr->monitor_count; i++) {
        const JiMonitorDpi* m = &mgr->monitors[i];
        if (x >= m->x && x < m->x + m->width &&
            y >= m->y && y < m->y + m->height) {
            return m;
        }
    }
    return mgr->monitor_count > 0 ? &mgr->monitors[0] : NULL;
}

/* =========================================================================
 * Scale Factor
 * ========================================================================= */

double ji_hidpi_get_scale(const JiHidpiManager* mgr, int monitor_id)
{
    if (!mgr) return 1.0;
    if (mgr->global_scale > 0.0) return mgr->global_scale;
    for (int i = 0; i < mgr->monitor_count; i++) {
        if (mgr->monitors[i].id == monitor_id)
            return mgr->monitors[i].scale;
    }
    return 1.0;
}

double ji_hidpi_get_scale_at(const JiHidpiManager* mgr, int x, int y)
{
    if (!mgr) return 1.0;
    if (mgr->global_scale > 0.0) return mgr->global_scale;
    const JiMonitorDpi* m = ji_hidpi_get_monitor_at(mgr, x, y);
    return m ? m->scale : 1.0;
}

void ji_hidpi_set_global_scale(JiHidpiManager* mgr, double scale)
{
    if (!mgr) return;
    if (scale < JI_HIDPI_SCALE_MIN) scale = JI_HIDPI_SCALE_MIN;
    if (scale > JI_HIDPI_SCALE_MAX) scale = JI_HIDPI_SCALE_MAX;
    mgr->global_scale = scale;
}

double ji_hidpi_get_global_scale(const JiHidpiManager* mgr)
{
    return mgr ? mgr->global_scale : 0.0;
}

void ji_hidpi_set_auto_detect(JiHidpiManager* mgr, bool enabled)
{
    if (mgr) mgr->auto_detect = enabled;
}

/* =========================================================================
 * Coordinate Conversion
 * ========================================================================= */

void ji_hidpi_dip_to_pixel(const JiHidpiManager* mgr, int monitor_id,
                              double dip_x, double dip_y,
                              int* pixel_x, int* pixel_y)
{
    double scale = ji_hidpi_get_scale(mgr, monitor_id);
    if (pixel_x) *pixel_x = (int)(dip_x * scale + 0.5);
    if (pixel_y) *pixel_y = (int)(dip_y * scale + 0.5);
}

void ji_hidpi_pixel_to_dip(const JiHidpiManager* mgr, int monitor_id,
                              int pixel_x, int pixel_y,
                              double* dip_x, double* dip_y)
{
    double scale = ji_hidpi_get_scale(mgr, monitor_id);
    if (scale <= 0.0) scale = 1.0;
    if (dip_x) *dip_x = (double)pixel_x / scale;
    if (dip_y) *dip_y = (double)pixel_y / scale;
}

/* =========================================================================
 * Resource Scaling
 * ========================================================================= */

int ji_hidpi_get_resource_suffix(double scale, char* out, int out_size)
{
    if (!out || out_size <= 0) return -1;
    int int_scale = (int)(scale + 0.5);
    /* Integer scales: @2x, @3x */
    if (scale == (double)int_scale && int_scale >= 2) {
        return snprintf(out, out_size, "@%dx", int_scale);
    }
    /* Fractional: @1_5x, @1_25x */
    int tenths = (int)(scale * 10.0 + 0.5);
    int whole = tenths / 10;
    int frac = tenths % 10;
    if (frac == 0) {
        return snprintf(out, out_size, "@%dx", whole);
    }
    return snprintf(out, out_size, "@%d_%dx", whole, frac);
}

double ji_hidpi_select_best_scale(double target, const double* available, int count)
{
    if (!available || count <= 0) return target;
    double best = available[0];
    double best_diff = fabs(available[0] - target);
    for (int i = 1; i < count; i++) {
        double diff = fabs(available[i] - target);
        if (diff < best_diff) {
            best = available[i];
            best_diff = diff;
        }
    }
    return best;
}

/* =========================================================================
 * DPI Detection
 * ========================================================================= */

int ji_hidpi_detect_dpi(void)
{
    /* On Linux, we could check X11 resources or Wayland output metrics.
     * For now, return the standard 96 DPI as a safe default.
     * Real detection would query the display server. */
    return 96;
}

double ji_hidpi_dpi_to_scale(int dpi)
{
    if (dpi <= 0) return 1.0;
    double scale = (double)dpi / 96.0;
    /* Round to nearest 0.25 */
    scale = round(scale * 4.0) / 4.0;
    if (scale < JI_HIDPI_SCALE_MIN) scale = JI_HIDPI_SCALE_MIN;
    if (scale > JI_HIDPI_SCALE_MAX) scale = JI_HIDPI_SCALE_MAX;
    return scale;
}
