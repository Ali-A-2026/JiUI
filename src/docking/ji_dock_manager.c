/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_manager.c
 * @brief Dock manager implementation.
 */

#include <jiui/ji_dock_manager.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- Lifecycle ---- */

JiDockManager* ji_dock_manager_new(void) {
    JiDockManager* mgr = (JiDockManager*)ji_calloc(1, sizeof(JiDockManager));
    if (!mgr) {
        JI_ERROR_LOG("ji_dock_manager_new: out of memory");
        return NULL;
    }

    mgr->root_area = NULL;
    mgr->floating_count = 0;
    mgr->floating_capacity = 4;
    mgr->floating_widgets = (JiDockWidget**)ji_alloc(sizeof(JiDockWidget*) * mgr->floating_capacity);

    mgr->area_count = 0;
    mgr->area_capacity = 8;
    mgr->areas = (JiDockArea**)ji_alloc(sizeof(JiDockArea*) * mgr->area_capacity);

    mgr->active_widget = NULL;
    mgr->drag_state.is_dragging = false;
    mgr->drag_state.dragged_widget = NULL;
    mgr->drag_state.source_area = NULL;

    return mgr;
}

void ji_dock_manager_destroy(JiDockManager* mgr) {
    if (!mgr) return;

    /* Destroy all areas (which destroys their widgets) */
    for (int i = 0; i < mgr->area_count; i++) {
        if (mgr->areas[i]) ji_dock_area_destroy(mgr->areas[i]);
    }
    ji_free(mgr->areas);

    /* Destroy floating widgets */
    for (int i = 0; i < mgr->floating_count; i++) {
        if (mgr->floating_widgets[i]) ji_dock_widget_destroy(mgr->floating_widgets[i]);
    }
    ji_free(mgr->floating_widgets);

    ji_free(mgr);
}

/* ---- Internal: grow arrays ---- */

static void grow_areas(JiDockManager* mgr) {
    if (mgr->area_count < mgr->area_capacity) return;
    int new_cap = mgr->area_capacity * 2;
    JiDockArea** new_arr = (JiDockArea**)ji_alloc(sizeof(JiDockArea*) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, mgr->areas, sizeof(JiDockArea*) * mgr->area_count);
    ji_free(mgr->areas);
    mgr->areas = new_arr;
    mgr->area_capacity = new_cap;
}

static void grow_floating(JiDockManager* mgr) {
    if (mgr->floating_count < mgr->floating_capacity) return;
    int new_cap = mgr->floating_capacity * 2;
    JiDockWidget** new_arr = (JiDockWidget**)ji_alloc(sizeof(JiDockWidget*) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, mgr->floating_widgets, sizeof(JiDockWidget*) * mgr->floating_count);
    ji_free(mgr->floating_widgets);
    mgr->floating_widgets = new_arr;
    mgr->floating_capacity = new_cap;
}

/* ---- Area management ---- */

void ji_dock_manager_add_area(JiDockManager* mgr, JiDockArea* area) {
    if (!mgr || !area) return;
    grow_areas(mgr);
    mgr->areas[mgr->area_count++] = area;
}

void ji_dock_manager_remove_area(JiDockManager* mgr, JiDockArea* area) {
    if (!mgr || !area) return;
    for (int i = 0; i < mgr->area_count; i++) {
        if (mgr->areas[i] == area) {
            for (int j = i; j < mgr->area_count - 1; j++) {
                mgr->areas[j] = mgr->areas[j + 1];
            }
            mgr->area_count--;
            break;
        }
    }
    if (mgr->root_area == area) mgr->root_area = NULL;
}

JiDockArea* ji_dock_manager_get_root_area(const JiDockManager* mgr) {
    return mgr ? mgr->root_area : NULL;
}

void ji_dock_manager_set_root_area(JiDockManager* mgr, JiDockArea* area) {
    if (!mgr) return;
    mgr->root_area = area;
    /* Ensure it's in the areas list */
    bool found = false;
    for (int i = 0; i < mgr->area_count; i++) {
        if (mgr->areas[i] == area) { found = true; break; }
    }
    if (!found && area) ji_dock_manager_add_area(mgr, area);
}

int ji_dock_manager_area_count(const JiDockManager* mgr) {
    return mgr ? mgr->area_count : 0;
}

JiDockArea* ji_dock_manager_get_area(const JiDockManager* mgr, int index) {
    if (!mgr || index < 0 || index >= mgr->area_count) return NULL;
    return mgr->areas[index];
}

/* ---- Widget management ---- */

void ji_dock_manager_add_widget(JiDockManager* mgr, JiDockWidget* widget, JiDockArea* area) {
    if (!mgr || !widget || !area) return;
    ji_dock_area_add_widget(area, widget);
}

void ji_dock_manager_remove_widget(JiDockManager* mgr, JiDockWidget* widget) {
    if (!mgr || !widget) return;
    if (widget->area) {
        ji_dock_area_remove_widget(widget->area, widget);
    }
    /* Check if it's in floating list */
    for (int i = 0; i < mgr->floating_count; i++) {
        if (mgr->floating_widgets[i] == widget) {
            for (int j = i; j < mgr->floating_count - 1; j++) {
                mgr->floating_widgets[j] = mgr->floating_widgets[j + 1];
            }
            mgr->floating_count--;
            break;
        }
    }
}

void ji_dock_manager_float_widget(JiDockManager* mgr, JiDockWidget* widget) {
    if (!mgr || !widget) return;

    /* Remove from current area */
    if (widget->area) {
        ji_dock_area_remove_widget(widget->area, widget);
    }

    /* Remove from floating list if already there */
    for (int i = 0; i < mgr->floating_count; i++) {
        if (mgr->floating_widgets[i] == widget) return;
    }

    /* Add to floating list */
    grow_floating(mgr);
    ji_dock_widget_set_floating(widget, true);
    mgr->floating_widgets[mgr->floating_count++] = widget;
}

void ji_dock_manager_dock_widget(JiDockManager* mgr, JiDockWidget* widget, JiDockArea* area) {
    if (!mgr || !widget || !area) return;

    /* Remove from floating list */
    for (int i = 0; i < mgr->floating_count; i++) {
        if (mgr->floating_widgets[i] == widget) {
            for (int j = i; j < mgr->floating_count - 1; j++) {
                mgr->floating_widgets[j] = mgr->floating_widgets[j + 1];
            }
            mgr->floating_count--;
            break;
        }
    }

    /* Add to target area */
    ji_dock_widget_set_floating(widget, false);
    ji_dock_area_add_widget(area, widget);
}

JiDockWidget* ji_dock_manager_find_widget(const JiDockManager* mgr, const char* name) {
    if (!mgr || !name) return NULL;

    /* Search all areas */
    for (int i = 0; i < mgr->area_count; i++) {
        JiDockWidget* w = ji_dock_area_find_widget(mgr->areas[i], name);
        if (w) return w;
    }

    /* Search floating widgets */
    for (int i = 0; i < mgr->floating_count; i++) {
        if (mgr->floating_widgets[i] && mgr->floating_widgets[i]->name &&
            strcmp(mgr->floating_widgets[i]->name, name) == 0) {
            return mgr->floating_widgets[i];
        }
    }

    return NULL;
}

/* ---- Floating widget management ---- */

int ji_dock_manager_floating_count(const JiDockManager* mgr) {
    return mgr ? mgr->floating_count : 0;
}

JiDockWidget* ji_dock_manager_get_floating(const JiDockManager* mgr, int index) {
    if (!mgr || index < 0 || index >= mgr->floating_count) return NULL;
    return mgr->floating_widgets[index];
}

/* ---- Active widget ---- */

void ji_dock_manager_set_active_widget(JiDockManager* mgr, JiDockWidget* widget) {
    if (!mgr) return;

    /* Deactivate previous */
    if (mgr->active_widget) {
        mgr->active_widget->is_active = false;
    }

    mgr->active_widget = widget;

    if (widget) {
        widget->is_active = true;
        /* Also set active in its area */
        if (widget->area) {
            for (int i = 0; i < widget->area->widget_count; i++) {
                if (widget->area->widgets[i] == widget) {
                    ji_dock_area_set_active_index(widget->area, i);
                    break;
                }
            }
        }
    }
}

JiDockWidget* ji_dock_manager_get_active_widget(const JiDockManager* mgr) {
    return mgr ? mgr->active_widget : NULL;
}

/* ---- Geometry ---- */

void ji_dock_manager_set_geometry(JiDockManager* mgr, JiRect geometry) {
    if (!mgr) return;
    mgr->geometry = geometry;
}

JiRect ji_dock_manager_get_geometry(const JiDockManager* mgr) {
    return mgr ? mgr->geometry : (JiRect){0, 0, 0, 0};
}

/* ---- Drag state ---- */

void ji_dock_manager_begin_drag(JiDockManager* mgr, JiDockWidget* widget, int mx, int my) {
    if (!mgr || !widget) return;
    mgr->drag_state.dragged_widget = widget;
    mgr->drag_state.source_area = widget->area;
    mgr->drag_state.is_dragging = true;
    mgr->drag_state.mouse_x = mx;
    mgr->drag_state.mouse_y = my;
}

void ji_dock_manager_update_drag(JiDockManager* mgr, int mx, int my) {
    if (!mgr || !mgr->drag_state.is_dragging) return;
    mgr->drag_state.mouse_x = mx;
    mgr->drag_state.mouse_y = my;
}

void ji_dock_manager_end_drag(JiDockManager* mgr) {
    if (!mgr) return;
    mgr->drag_state.is_dragging = false;
    mgr->drag_state.dragged_widget = NULL;
    mgr->drag_state.source_area = NULL;
}

bool ji_dock_manager_is_dragging(const JiDockManager* mgr) {
    return mgr ? mgr->drag_state.is_dragging : false;
}
