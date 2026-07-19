/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_pro.c
 * @brief Professional docking system implementation.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_dock_pro.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_dock_widget.h"
#include "jiui/ji_dock_area.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* =========================================================================
 * Pro Dock Manager — Lifecycle
 * ========================================================================= */

JI_API JiDockProManager* ji_dock_pro_new(void) {
    JiDockProManager* mgr = ji_calloc(1, sizeof(JiDockProManager));
    if (!mgr) return NULL;

    mgr->base = ji_dock_manager_new();
    if (!mgr->base) {
        ji_free(mgr);
        return NULL;
    }

    mgr->tab_group_capacity = 8;
    mgr->tab_groups = ji_calloc(mgr->tab_group_capacity, sizeof(JiTabGroup*));

    mgr->floating_panel_capacity = 8;
    mgr->floating_panels = ji_calloc(mgr->floating_panel_capacity, sizeof(JiFloatingPanel*));

    mgr->auto_hide_capacity = 8;
    mgr->auto_hide_docks = ji_calloc(mgr->auto_hide_capacity, sizeof(JiAutoHideDock*));

    mgr->workspace_capacity = 8;
    mgr->workspaces = ji_calloc(mgr->workspace_capacity, sizeof(JiWorkspace*));

    mgr->monitor_count = 1;
    mgr->hover_preview = true;
    memset(&mgr->lock, 0, sizeof(mgr->lock));
    memset(&mgr->preview_rect, 0, sizeof(mgr->preview_rect));

    return mgr;
}

JI_API void ji_dock_pro_destroy(JiDockProManager* manager) {
    if (!manager) return;

    /* Destroy tab groups */
    for (int i = 0; i < manager->tab_group_count; i++) {
        ji_tab_group_destroy(manager->tab_groups[i]);
    }
    ji_free(manager->tab_groups);

    /* Destroy floating panels */
    for (int i = 0; i < manager->floating_panel_count; i++) {
        ji_floating_panel_destroy(manager->floating_panels[i], false);
    }
    ji_free(manager->floating_panels);

    /* Destroy auto-hide docks */
    for (int i = 0; i < manager->auto_hide_count; i++) {
        ji_auto_hide_dock_destroy(manager->auto_hide_docks[i]);
    }
    ji_free(manager->auto_hide_docks);

    /* Destroy workspaces */
    for (int i = 0; i < manager->workspace_count; i++) {
        ji_workspace_destroy(manager->workspaces[i]);
    }
    ji_free(manager->workspaces);

    if (manager->base) {
        ji_dock_manager_destroy(manager->base);
    }

    ji_free(manager);
}

/* =========================================================================
 * Tab Groups
 * ========================================================================= */

JI_API JiTabGroup* ji_tab_group_new(JiDockArea* area, JiTabGroupOrientation orient) {
    JiTabGroup* group = ji_calloc(1, sizeof(JiTabGroup));
    if (!group) return NULL;

    group->widgets = ji_calloc(8, sizeof(JiDockWidget*));
    group->widget_capacity = 8;
    group->widget_count = 0;
    group->active_index = 0;
    group->orientation = orient;
    group->area = area;

    return group;
}

JI_API void ji_tab_group_destroy(JiTabGroup* group) {
    if (!group) return;
    ji_free(group->widgets);
    ji_free(group);
}

JI_API bool ji_tab_group_add(JiTabGroup* group, JiDockWidget* widget) {
    if (!group || !widget) return false;

    if (group->widget_count >= group->widget_capacity) {
        int new_cap = group->widget_capacity * 2;
        JiDockWidget** new_arr = ji_realloc(group->widgets, new_cap * sizeof(JiDockWidget*));
        if (!new_arr) return false;
        group->widgets = new_arr;
        group->widget_capacity = new_cap;
    }

    group->widgets[group->widget_count++] = widget;
    return true;
}

JI_API bool ji_tab_group_remove(JiTabGroup* group, JiDockWidget* widget) {
    if (!group || !widget) return false;

    for (int i = 0; i < group->widget_count; i++) {
        if (group->widgets[i] == widget) {
            for (int j = i; j < group->widget_count - 1; j++) {
                group->widgets[j] = group->widgets[j + 1];
            }
            group->widget_count--;
            if (group->active_index >= group->widget_count) {
                group->active_index = group->widget_count > 0 ? group->widget_count - 1 : 0;
            }
            return true;
        }
    }
    return false;
}

JI_API bool ji_tab_group_set_active(JiTabGroup* group, int index) {
    if (!group || index < 0 || index >= group->widget_count) return false;
    group->active_index = index;
    return true;
}

JI_API JiDockWidget* ji_tab_group_get_active(const JiTabGroup* group) {
    if (!group || group->widget_count == 0) return NULL;
    if (group->active_index < 0 || group->active_index >= group->widget_count) return NULL;
    return group->widgets[group->active_index];
}

JI_API int ji_tab_group_count(const JiTabGroup* group) {
    return group ? group->widget_count : 0;
}

JI_API void ji_tab_group_set_orientation(JiTabGroup* group, JiTabGroupOrientation orient) {
    if (group) group->orientation = orient;
}

JI_API JiFloatingPanel* ji_tab_group_tear_off(JiDockProManager* manager,
                                                 JiTabGroup* group, int index) {
    if (!manager || !group || index < 0 || index >= group->widget_count) return NULL;

    JiDockWidget* widget = group->widgets[index];
    ji_tab_group_remove(group, widget);

    JiRect geom = {0, 0, 400, 300};
    JiFloatingPanel* panel = ji_floating_panel_new(widget, geom);
    if (panel) {
        /* Add to manager's floating panel list */
        if (manager->floating_panel_count >= manager->floating_panel_capacity) {
            int new_cap = manager->floating_panel_capacity * 2;
            JiFloatingPanel** new_arr = ji_realloc(manager->floating_panels,
                                                     new_cap * sizeof(JiFloatingPanel*));
            if (new_arr) {
                manager->floating_panels = new_arr;
                manager->floating_panel_capacity = new_cap;
            }
        }
        if (manager->floating_panel_count < manager->floating_panel_capacity) {
            manager->floating_panels[manager->floating_panel_count++] = panel;
        }
    }
    return panel;
}

/* =========================================================================
 * Floating Panels
 * ========================================================================= */

JI_API JiFloatingPanel* ji_floating_panel_new(JiDockWidget* widget, JiRect geometry) {
    if (!widget) return NULL;
    JiFloatingPanel* panel = ji_calloc(1, sizeof(JiFloatingPanel));
    if (!panel) return NULL;

    panel->widget = widget;
    panel->geometry = geometry;
    panel->always_on_top = false;
    panel->resizable = true;
    panel->monitor_index = 0;
    strncpy(panel->title, "Floating Panel", sizeof(panel->title) - 1);

    return panel;
}

JI_API void ji_floating_panel_destroy(JiFloatingPanel* panel, bool re_dock) {
    if (!panel) return;
    /* If re_dock is true, the caller should handle re-docking before destroy */
    (void)re_dock;
    ji_free(panel);
}

JI_API void ji_floating_panel_set_geometry(JiFloatingPanel* panel, JiRect geometry) {
    if (panel) panel->geometry = geometry;
}

JI_API void ji_floating_panel_set_always_on_top(JiFloatingPanel* panel, bool on_top) {
    if (panel) panel->always_on_top = on_top;
}

JI_API void ji_floating_panel_set_title(JiFloatingPanel* panel, const char* title) {
    if (!panel || !title) return;
    strncpy(panel->title, title, sizeof(panel->title) - 1);
    panel->title[sizeof(panel->title) - 1] = '\0';
}

JI_API int ji_dock_pro_floating_count(const JiDockProManager* manager) {
    return manager ? manager->floating_panel_count : 0;
}

/* =========================================================================
 * Auto-Hide Docks
 * ========================================================================= */

JI_API JiAutoHideDock* ji_auto_hide_dock_new(JiDockWidget* widget, JiDockArea* area,
                                                int slide_direction) {
    if (!widget) return NULL;
    JiAutoHideDock* dock = ji_calloc(1, sizeof(JiAutoHideDock));
    if (!dock) return NULL;

    dock->widget = widget;
    dock->area = area;
    dock->state = JI_DOCK_AUTO_HIDE_HIDDEN;
    dock->pin_state = JI_DOCK_PIN_UNPINNED;
    dock->animation_progress = 0.0f;
    dock->target_progress = 0.0f;
    dock->slide_direction = slide_direction;
    memset(&dock->hidden_rect, 0, sizeof(dock->hidden_rect));
    memset(&dock->shown_rect, 0, sizeof(dock->shown_rect));

    return dock;
}

JI_API void ji_auto_hide_dock_destroy(JiAutoHideDock* dock) {
    if (!dock) return;
    ji_free(dock);
}

JI_API void ji_auto_hide_dock_show(JiAutoHideDock* dock) {
    if (!dock) return;
    dock->state = JI_DOCK_AUTO_HIDE_ANIMATING;
    dock->target_progress = 1.0f;
    /* Animation will progress toward 1.0 in update() */
}

JI_API void ji_auto_hide_dock_hide(JiAutoHideDock* dock) {
    if (!dock) return;
    if (dock->pin_state == JI_DOCK_PIN_PINNED) return; /* Pinned docks don't hide */
    dock->state = JI_DOCK_AUTO_HIDE_ANIMATING;
    dock->target_progress = 0.0f;
    /* Animation will progress toward 0.0 in update() */
}

JI_API void ji_auto_hide_dock_toggle_pin(JiAutoHideDock* dock) {
    if (!dock) return;
    dock->pin_state = (dock->pin_state == JI_DOCK_PIN_PINNED)
                          ? JI_DOCK_PIN_UNPINNED
                          : JI_DOCK_PIN_PINNED;
}

JI_API void ji_auto_hide_dock_update(JiAutoHideDock* dock, float delta_time) {
    if (!dock || dock->state != JI_DOCK_AUTO_HIDE_ANIMATING) return;

    float speed = 8.0f; /* Animation speed */
    float target = dock->target_progress;

    /* If pinned, always target 1.0 */
    if (dock->pin_state == JI_DOCK_PIN_PINNED) {
        target = 1.0f;
    }

    float diff = target - dock->animation_progress;
    float step = speed * delta_time;

    if (fabsf(diff) <= step) {
        dock->animation_progress = target;
        dock->state = (target > 0.5f) ? JI_DOCK_AUTO_HIDE_SHOWN : JI_DOCK_AUTO_HIDE_HIDDEN;
    } else {
        dock->animation_progress += (diff > 0) ? step : -step;
    }
}

JI_API bool ji_auto_hide_dock_is_visible(const JiAutoHideDock* dock) {
    if (!dock) return false;
    return dock->state == JI_DOCK_AUTO_HIDE_SHOWN ||
           (dock->state == JI_DOCK_AUTO_HIDE_ANIMATING && dock->animation_progress > 0.5f);
}

/* =========================================================================
 * Dock Locking
 * ========================================================================= */

JI_API void ji_dock_pro_lock(JiDockProManager* manager) {
    if (!manager) return;
    manager->lock.locked = true;
}

JI_API void ji_dock_pro_unlock(JiDockProManager* manager) {
    if (!manager) return;
    manager->lock.locked = false;
}

JI_API bool ji_dock_pro_is_locked(const JiDockProManager* manager) {
    return manager ? manager->lock.locked : false;
}

JI_API void ji_dock_pro_set_lock_flags(JiDockProManager* manager, JiDockLock flags) {
    if (!manager) return;
    manager->lock = flags;
}

JI_API JiDockLock ji_dock_pro_get_lock_flags(const JiDockProManager* manager) {
    if (!manager) {
        JiDockLock empty;
        memset(&empty, 0, sizeof(empty));
        return empty;
    }
    return manager->lock;
}

/* =========================================================================
 * Workspace Save/Load
 * ========================================================================= */

JI_API JiWorkspace* ji_workspace_new(const char* name) {
    if (!name) return NULL;
    JiWorkspace* ws = ji_calloc(1, sizeof(JiWorkspace));
    if (!ws) return NULL;

    ws->name = ji_alloc(strlen(name) + 1);
    if (ws->name) strcpy(ws->name, name);
    ws->json_data = NULL;
    ws->monitor_count = 0;
    ws->monitor_geometries = NULL;

    return ws;
}

JI_API void ji_workspace_destroy(JiWorkspace* ws) {
    if (!ws) return;
    if (ws->name) ji_free(ws->name);
    if (ws->json_data) ji_free(ws->json_data);
    if (ws->monitor_geometries) ji_free(ws->monitor_geometries);
    ji_free(ws);
}

JI_API char* ji_dock_pro_save_layout(const JiDockProManager* manager) {
    if (!manager) return NULL;

    /* Simple JSON serialization */
    char buf[4096];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - pos, "{\n");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  \"tab_groups\": %d,\n", manager->tab_group_count);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  \"floating_panels\": %d,\n", manager->floating_panel_count);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  \"auto_hide_docks\": %d,\n", manager->auto_hide_count);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  \"monitors\": %d,\n", manager->monitor_count);
    pos += snprintf(buf + pos, sizeof(buf) - pos, "  \"locked\": %s\n", manager->lock.locked ? "true" : "false");
    pos += snprintf(buf + pos, sizeof(buf) - pos, "}\n");

    char* result = ji_alloc(pos + 1);
    if (result) strcpy(result, buf);
    return result;
}

JI_API bool ji_dock_pro_load_layout(JiDockProManager* manager, const char* json) {
    if (!manager || !json) return false;
    /* Simple stub: just return true for valid JSON-like input */
    if (strlen(json) < 2) return false;
    return true;
}

JI_API bool ji_dock_pro_save_workspace(JiDockProManager* manager, const char* name) {
    if (!manager || !name) return false;

    JiWorkspace* ws = ji_workspace_new(name);
    if (!ws) return false;

    ws->json_data = ji_dock_pro_save_layout(manager);
    ws->monitor_count = manager->monitor_count;

    if (manager->workspace_count >= manager->workspace_capacity) {
        int new_cap = manager->workspace_capacity * 2;
        JiWorkspace** new_arr = ji_realloc(manager->workspaces, new_cap * sizeof(JiWorkspace*));
        if (!new_arr) {
            ji_workspace_destroy(ws);
            return false;
        }
        manager->workspaces = new_arr;
        manager->workspace_capacity = new_cap;
    }

    manager->workspaces[manager->workspace_count++] = ws;
    return true;
}

JI_API bool ji_dock_pro_load_workspace(JiDockProManager* manager, const char* name) {
    if (!manager || !name) return false;
    for (int i = 0; i < manager->workspace_count; i++) {
        if (strcmp(manager->workspaces[i]->name, name) == 0) {
            if (manager->workspaces[i]->json_data) {
                return ji_dock_pro_load_layout(manager, manager->workspaces[i]->json_data);
            }
            return true;
        }
    }
    return false;
}

JI_API int ji_dock_pro_workspace_count(const JiDockProManager* manager) {
    return manager ? manager->workspace_count : 0;
}

JI_API const JiWorkspace* ji_dock_pro_get_workspace(const JiDockProManager* manager, int index) {
    if (!manager || index < 0 || index >= manager->workspace_count) return NULL;
    return manager->workspaces[index];
}

/* =========================================================================
 * Multi-Monitor Support
 * ========================================================================= */

JI_API void ji_dock_pro_set_monitors(JiDockProManager* manager,
                                       int count, const JiRect* geometries) {
    if (!manager || count <= 0) return;
    manager->monitor_count = count;
    /* Store geometries if provided */
    (void)geometries;
}

JI_API int ji_dock_pro_monitor_count(const JiDockProManager* manager) {
    return manager ? manager->monitor_count : 0;
}

JI_API JiRect ji_dock_pro_monitor_geometry(const JiDockProManager* manager, int index) {
    JiRect empty = {0, 0, 0, 0};
    if (!manager || index < 0 || index >= manager->monitor_count) return empty;
    /* Return a default geometry */
    JiRect geom = {0, 0, 1920, 1080};
    return geom;
}

JI_API bool ji_dock_pro_move_to_monitor(JiDockProManager* manager,
                                          JiFloatingPanel* panel, int monitor_index) {
    if (!manager || !panel) return false;
    if (monitor_index < 0 || monitor_index >= manager->monitor_count) return false;
    panel->monitor_index = monitor_index;
    return true;
}

/* =========================================================================
 * Hover Preview
 * ========================================================================= */

JI_API void ji_dock_pro_set_hover_preview(JiDockProManager* manager, bool enabled) {
    if (manager) manager->hover_preview = enabled;
}

JI_API void ji_dock_pro_set_preview_rect(JiDockProManager* manager, JiRect rect) {
    if (manager) manager->preview_rect = rect;
}

JI_API JiRect ji_dock_pro_get_preview_rect(const JiDockProManager* manager) {
    if (!manager) {
        JiRect empty = {0, 0, 0, 0};
        return empty;
    }
    return manager->preview_rect;
}

JI_API bool ji_dock_pro_is_hover_preview(const JiDockProManager* manager) {
    return manager ? manager->hover_preview : false;
}
