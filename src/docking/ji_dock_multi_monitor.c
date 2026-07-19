/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_multi_monitor.c
 * @brief Multi-monitor layout helpers for the professional docking system.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_dock_pro.h"
#include "jiui/ji_memory.h"
#include <string.h>

/* Monitor geometry storage */
static JiRect* s_monitor_geometries = NULL;
static int s_monitor_count = 0;

/* Detect available monitors (platform-specific stub) */
int ji_dock_detect_monitors(JiRect** out_geometries) {
    /* Default: single monitor 1920x1080 */
    int count = 1;
    JiRect* geoms = ji_calloc(count, sizeof(JiRect));
    if (!geoms) return 0;

    geoms[0].x = 0;
    geoms[0].y = 0;
    geoms[0].width = 1920;
    geoms[0].height = 1080;

    if (out_geometries) *out_geometries = geoms;
    else ji_free(geoms);

    return count;
}

/* Find which monitor contains a point */
int ji_dock_find_monitor_for_point(int x, int y) {
    for (int i = 0; i < s_monitor_count; i++) {
        if (x >= s_monitor_geometries[i].x &&
            x < s_monitor_geometries[i].x + s_monitor_geometries[i].width &&
            y >= s_monitor_geometries[i].y &&
            y < s_monitor_geometries[i].y + s_monitor_geometries[i].height) {
            return i;
        }
    }
    return 0; /* Default to first monitor */
}

/* Find which monitor contains a rect (largest overlap) */
int ji_dock_find_monitor_for_rect(JiRect rect) {
    int best = 0;
    int best_area = 0;

    for (int i = 0; i < s_monitor_count; i++) {
        JiRect* m = &s_monitor_geometries[i];
        int ox = (rect.x < m->x) ? m->x : rect.x;
        int oy = (rect.y < m->y) ? m->y : rect.y;
        int ex = (rect.x + rect.width < m->x + m->width) ? (rect.x + rect.width) : (m->x + m->width);
        int ey = (rect.y + rect.height < m->y + m->height) ? (rect.y + rect.height) : (m->y + m->height);
        int w = ex - ox;
        int h = ey - oy;
        if (w > 0 && h > 0) {
            int area = w * h;
            if (area > best_area) {
                best_area = area;
                best = i;
            }
        }
    }
    return best;
}

/* Clamp a rect to a monitor's geometry */
JiRect ji_dock_clamp_to_monitor(JiRect rect, int monitor_index) {
    if (monitor_index < 0 || monitor_index >= s_monitor_count) return rect;
    JiRect* m = &s_monitor_geometries[monitor_index];

    if (rect.x < m->x) rect.x = m->x;
    if (rect.y < m->y) rect.y = m->y;
    if (rect.x + rect.width > m->x + m->width) {
        rect.x = m->x + m->width - rect.width;
        if (rect.x < m->x) rect.x = m->x;
    }
    if (rect.y + rect.height > m->y + m->height) {
        rect.y = m->y + m->height - rect.height;
        if (rect.y < m->y) rect.y = m->y;
    }
    return rect;
}

/* Initialize monitor geometries */
void ji_dock_init_monitors(void) {
    if (s_monitor_geometries) ji_free(s_monitor_geometries);
    s_monitor_geometries = NULL;
    s_monitor_count = ji_dock_detect_monitors(&s_monitor_geometries);
}

/* Cleanup monitor geometries */
void ji_dock_cleanup_monitors(void) {
    if (s_monitor_geometries) {
        ji_free(s_monitor_geometries);
        s_monitor_geometries = NULL;
    }
    s_monitor_count = 0;
}

/* Get monitor count */
int ji_dock_get_monitor_count(void) {
    return s_monitor_count;
}

/* Get monitor geometry by index */
JiRect ji_dock_get_monitor_rect(int index) {
    if (index < 0 || index >= s_monitor_count) {
        JiRect empty = {0, 0, 0, 0};
        return empty;
    }
    return s_monitor_geometries[index];
}
