/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_area.c
 * @brief Dock area implementation.
 */

#include <jiui/ji_dock_area.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- Lifecycle ---- */

JiDockArea* ji_dock_area_new(const char* name) {
    JiDockArea* area = (JiDockArea*)ji_calloc(1, sizeof(JiDockArea));
    if (!area) {
        JI_ERROR_LOG("ji_dock_area_new: out of memory");
        return NULL;
    }

    if (name) {
        size_t len = strlen(name);
        area->name = (char*)ji_alloc(len + 1);
        if (area->name) memcpy(area->name, name, len + 1);
    }

    area->region = JI_DOCK_REGION_CENTER;
    area->widget_count = 0;
    area->widget_capacity = 4;
    area->widgets = (JiDockWidget**)ji_alloc(sizeof(JiDockWidget*) * area->widget_capacity);
    area->active_index = 0;
    area->tab_bar_height = 24;
    area->splitter = NULL;

    return area;
}

void ji_dock_area_destroy(JiDockArea* area) {
    if (!area) return;

    /* Destroy all contained dock widgets */
    for (int i = 0; i < area->widget_count; i++) {
        if (area->widgets[i]) {
            area->widgets[i]->area = NULL;
            ji_dock_widget_destroy(area->widgets[i]);
        }
    }
    ji_free(area->widgets);
    ji_free(area->name);
    /* Note: splitter is managed separately if set */
    ji_free(area);
}

/* ---- Widget management ---- */

void ji_dock_area_add_widget(JiDockArea* area, JiDockWidget* widget) {
    if (!area || !widget) return;

    /* Grow array if needed */
    if (area->widget_count >= area->widget_capacity) {
        int new_cap = area->widget_capacity * 2;
        JiDockWidget** new_arr = (JiDockWidget**)ji_alloc(sizeof(JiDockWidget*) * new_cap);
        if (!new_arr) return;
        memcpy(new_arr, area->widgets, sizeof(JiDockWidget*) * area->widget_count);
        ji_free(area->widgets);
        area->widgets = new_arr;
        area->widget_capacity = new_cap;
    }

    widget->area = area;
    widget->state = JI_DOCK_DOCKED;
    widget->is_floating = false;
    area->widgets[area->widget_count++] = widget;

    /* If this is the first widget, make it active */
    if (area->widget_count == 1) {
        area->active_index = 0;
        widget->is_active = true;
    }
}

void ji_dock_area_remove_widget(JiDockArea* area, JiDockWidget* widget) {
    if (!area || !widget) return;

    int found_index = -1;
    for (int i = 0; i < area->widget_count; i++) {
        if (area->widgets[i] == widget) {
            found_index = i;
            break;
        }
    }

    if (found_index < 0) return;

    /* Shift remaining widgets down */
    for (int i = found_index; i < area->widget_count - 1; i++) {
        area->widgets[i] = area->widgets[i + 1];
    }
    area->widget_count--;

    widget->area = NULL;
    widget->is_active = false;

    /* Adjust active index */
    if (area->active_index >= area->widget_count) {
        area->active_index = area->widget_count > 0 ? area->widget_count - 1 : 0;
    }
    if (area->widget_count > 0) {
        area->widgets[area->active_index]->is_active = true;
    }
}

int ji_dock_area_widget_count(const JiDockArea* area) {
    return area ? area->widget_count : 0;
}

JiDockWidget* ji_dock_area_get_widget(const JiDockArea* area, int index) {
    if (!area || index < 0 || index >= area->widget_count) return NULL;
    return area->widgets[index];
}

JiDockWidget* ji_dock_area_find_widget(const JiDockArea* area, const char* name) {
    if (!area || !name) return NULL;
    for (int i = 0; i < area->widget_count; i++) {
        if (area->widgets[i] && area->widgets[i]->name &&
            strcmp(area->widgets[i]->name, name) == 0) {
            return area->widgets[i];
        }
    }
    return NULL;
}

/* ---- Tab management ---- */

void ji_dock_area_set_active_index(JiDockArea* area, int index) {
    if (!area || index < 0 || index >= area->widget_count) return;

    /* Deactivate previous */
    if (area->widget_count > 0 && area->active_index < area->widget_count) {
        area->widgets[area->active_index]->is_active = false;
    }

    area->active_index = index;
    area->widgets[index]->is_active = true;
}

int ji_dock_area_get_active_index(const JiDockArea* area) {
    return area ? area->active_index : 0;
}

JiDockWidget* ji_dock_area_get_active_widget(const JiDockArea* area) {
    if (!area || area->widget_count == 0) return NULL;
    if (area->active_index < 0 || area->active_index >= area->widget_count) return NULL;
    return area->widgets[area->active_index];
}

/* ---- Geometry ---- */

void ji_dock_area_set_rect(JiDockArea* area, JiRect rect) {
    if (!area) return;
    area->rect = rect;
}

JiRect ji_dock_area_get_rect(const JiDockArea* area) {
    return area ? area->rect : (JiRect){0, 0, 0, 0};
}

/* ---- Properties ---- */

void ji_dock_area_set_name(JiDockArea* area, const char* name) {
    if (!area) return;
    ji_free(area->name);
    if (name) {
        size_t len = strlen(name);
        area->name = (char*)ji_alloc(len + 1);
        if (area->name) memcpy(area->name, name, len + 1);
    } else {
        area->name = NULL;
    }
}

const char* ji_dock_area_get_name(const JiDockArea* area) {
    return area ? area->name : NULL;
}

void ji_dock_area_set_region(JiDockArea* area, JiDockRegion region) {
    if (!area) return;
    area->region = region;
}

JiDockRegion ji_dock_area_get_region(const JiDockArea* area) {
    return area ? area->region : JI_DOCK_REGION_CENTER;
}
