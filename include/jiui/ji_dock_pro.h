/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_pro.h
 * @brief Professional docking system — infinite nesting, tab groups,
 *        floating panels, workspace save/load, auto-hide, multi-monitor.
 *
 * Extends the base docking system (ji_dock_manager.h) with:
 *   - Infinite docking (unlimited nesting depth)
 *   - Tab groups (vertical + horizontal tab bars)
 *   - Floating panels (detached top-level windows)
 *   - Workspace save/load (JSON serialization)
 *   - Hover-over docking (preview overlay)
 *   - Multi-monitor layouts
 *   - Dock auto-hide (slide in/out)
 *   - Dock pin/unpin
 *   - Tab tear-off
 *   - Dock locking (prevent accidental changes)
 */

#ifndef JIUI_DOCK_PRO_H
#define JIUI_DOCK_PRO_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_dock_manager.h"
#include "ji_dock_area.h"
#include "ji_dock_widget.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Tab Group Orientation
 * ========================================================================= */
typedef enum JiTabGroupOrientation {
    JI_TAB_GROUP_HORIZONTAL = 0,   /* Tabs laid out left-to-right */
    JI_TAB_GROUP_VERTICAL         /* Tabs laid out top-to-bottom */
} JiTabGroupOrientation;

/* =========================================================================
 * Dock Auto-Hide State
 * ========================================================================= */
typedef enum JiDockAutoHideState {
    JI_DOCK_AUTO_HIDE_HIDDEN = 0,  /* Panel is slid out / hidden */
    JI_DOCK_AUTO_HIDE_SHOWN,       /* Panel is slid in / visible */
    JI_DOCK_AUTO_HIDE_ANIMATING    /* Panel is transitioning */
} JiDockAutoHideState;

/* =========================================================================
 * Dock Pin State
 * ========================================================================= */
typedef enum JiDockPinState {
    JI_DOCK_PIN_PINNED = 0,   /* Dock stays visible */
    JI_DOCK_PIN_UNPINNED      /* Dock auto-hides on mouse leave */
} JiDockPinState;

/* =========================================================================
 * Tab Group — a group of dock widgets sharing tab bar
 * ========================================================================= */
typedef struct JiTabGroup {
    JiDockWidget**     widgets;       /* Widgets in this tab group */
    int                widget_count;  /* Number of widgets */
    int                widget_capacity;
    int                active_index;  /* Currently visible tab index */
    JiTabGroupOrientation orientation; /* Tab bar orientation */
    JiDockArea*        area;          /* Owning dock area */
} JiTabGroup;

/* =========================================================================
 * Floating Panel — a detached dock widget in its own window
 * ========================================================================= */
typedef struct JiFloatingPanel {
    JiDockWidget*  widget;           /* The docked widget */
    JiRect         geometry;         /* Window geometry on screen */
    bool           always_on_top;     /* Keep panel above main window */
    bool           resizable;         /* Allow user resizing */
    char           title[256];        /* Panel window title */
    int            monitor_index;     /* Which monitor the panel is on */
} JiFloatingPanel;

/* =========================================================================
 * Auto-Hide Dock — a dock that slides in/out
 * ========================================================================= */
typedef struct JiAutoHideDock {
    JiDockWidget*       widget;       /* The widget to show/hide */
    JiDockArea*         area;          /* Associated dock area */
    JiDockAutoHideState state;         /* Current visibility state */
    JiDockPinState      pin_state;     /* Pinned or auto-hide */
    JiRect              hidden_rect;   /* Geometry when hidden (slim bar) */
    JiRect              shown_rect;    /* Geometry when shown (full panel) */
    float               animation_progress; /* 0.0 = hidden, 1.0 = shown */
    float               target_progress;   /* Target animation value */
    int                 slide_direction; /* 0=left, 1=right, 2=top, 3=bottom */
} JiAutoHideDock;

/* =========================================================================
 * Dock Lock — prevents accidental layout changes
 * ========================================================================= */
typedef struct JiDockLock {
    bool     locked;              /* True if docking is locked */
    bool     allow_tear_off;     /* Allow tab tear-off even when locked */
    bool     allow_resize;       /* Allow splitter resize when locked */
    bool     allow_auto_hide;    /* Allow auto-hide toggle when locked */
} JiDockLock;

/* =========================================================================
 * Workspace — serializable layout state
 * ========================================================================= */
typedef struct JiWorkspace {
    char*    name;               /* Workspace name */
    char*    json_data;          /* Serialized layout (JSON) */
    int      monitor_count;      /* Number of monitors used */
    JiRect*  monitor_geometries; /* Geometry of each monitor */
} JiWorkspace;

/* =========================================================================
 * Pro Dock Manager — extends JiDockManager
 * ========================================================================= */
typedef struct JiDockProManager {
    JiDockManager*      base;              /* Base dock manager */
    JiTabGroup**        tab_groups;        /* All tab groups */
    int                 tab_group_count;
    int                 tab_group_capacity;
    JiFloatingPanel**   floating_panels;   /* Detached panels */
    int                 floating_panel_count;
    int                 floating_panel_capacity;
    JiAutoHideDock**    auto_hide_docks;   /* Auto-hide docks */
    int                 auto_hide_count;
    int                 auto_hide_capacity;
    JiDockLock          lock;              /* Dock lock state */
    JiWorkspace**       workspaces;        /* Saved workspaces */
    int                 workspace_count;
    int                 workspace_capacity;
    int                 monitor_count;     /* Number of monitors */
    bool                hover_preview;     /* Show overlay during hover-dock */
    JiRect              preview_rect;      /* Current hover preview rect */
} JiDockProManager;

/* =========================================================================
 * Pro Dock Manager — Lifecycle
 * ========================================================================= */

/** Create a new professional dock manager. */
JI_API JiDockProManager* ji_dock_pro_new(void);

/** Destroy a professional dock manager and all resources. */
JI_API void ji_dock_pro_destroy(JiDockProManager* manager);

/* =========================================================================
 * Tab Groups
 * ========================================================================= */

/** Create a tab group in the given area. */
JI_API JiTabGroup* ji_tab_group_new(JiDockArea* area, JiTabGroupOrientation orient);

/** Destroy a tab group (does not destroy widgets). */
JI_API void ji_tab_group_destroy(JiTabGroup* group);

/** Add a widget to a tab group. */
JI_API bool ji_tab_group_add(JiTabGroup* group, JiDockWidget* widget);

/** Remove a widget from a tab group. */
JI_API bool ji_tab_group_remove(JiTabGroup* group, JiDockWidget* widget);

/** Set the active tab by index. */
JI_API bool ji_tab_group_set_active(JiTabGroup* group, int index);

/** Get the active widget in a tab group. */
JI_API JiDockWidget* ji_tab_group_get_active(const JiTabGroup* group);

/** Get the number of widgets in a tab group. */
JI_API int ji_tab_group_count(const JiTabGroup* group);

/** Set tab group orientation. */
JI_API void ji_tab_group_set_orientation(JiTabGroup* group, JiTabGroupOrientation orient);

/** Tear off a tab into a floating panel. */
JI_API JiFloatingPanel* ji_tab_group_tear_off(JiDockProManager* manager,
                                                 JiTabGroup* group, int index);

/* =========================================================================
 * Floating Panels
 * ========================================================================= */

/** Create a floating panel from a dock widget. */
JI_API JiFloatingPanel* ji_floating_panel_new(JiDockWidget* widget, JiRect geometry);

/** Destroy a floating panel (re-docks widget if re_dock is true). */
JI_API void ji_floating_panel_destroy(JiFloatingPanel* panel, bool re_dock);

/** Set floating panel geometry. */
JI_API void ji_floating_panel_set_geometry(JiFloatingPanel* panel, JiRect geometry);

/** Set always-on-top. */
JI_API void ji_floating_panel_set_always_on_top(JiFloatingPanel* panel, bool on_top);

/** Set panel title. */
JI_API void ji_floating_panel_set_title(JiFloatingPanel* panel, const char* title);

/** Get the number of floating panels. */
JI_API int ji_dock_pro_floating_count(const JiDockProManager* manager);

/* =========================================================================
 * Auto-Hide Docks
 * ========================================================================= */

/** Create an auto-hide dock. */
JI_API JiAutoHideDock* ji_auto_hide_dock_new(JiDockWidget* widget, JiDockArea* area,
                                                int slide_direction);

/** Destroy an auto-hide dock. */
JI_API void ji_auto_hide_dock_destroy(JiAutoHideDock* dock);

/** Show an auto-hide dock (slide in). */
JI_API void ji_auto_hide_dock_show(JiAutoHideDock* dock);

/** Hide an auto-hide dock (slide out). */
JI_API void ji_auto_hide_dock_hide(JiAutoHideDock* dock);

/** Toggle pin state. */
JI_API void ji_auto_hide_dock_toggle_pin(JiAutoHideDock* dock);

/** Update auto-hide animation. Call each frame. */
JI_API void ji_auto_hide_dock_update(JiAutoHideDock* dock, float delta_time);

/** Check if dock is currently visible. */
JI_API bool ji_auto_hide_dock_is_visible(const JiAutoHideDock* dock);

/* =========================================================================
 * Dock Locking
 * ========================================================================= */

/** Lock the docking system (prevent layout changes). */
JI_API void ji_dock_pro_lock(JiDockProManager* manager);

/** Unlock the docking system. */
JI_API void ji_dock_pro_unlock(JiDockProManager* manager);

/** Check if docking is locked. */
JI_API bool ji_dock_pro_is_locked(const JiDockProManager* manager);

/** Set granular lock permissions. */
JI_API void ji_dock_pro_set_lock_flags(JiDockProManager* manager, JiDockLock flags);

/** Get current lock flags. */
JI_API JiDockLock ji_dock_pro_get_lock_flags(const JiDockProManager* manager);

/* =========================================================================
 * Workspace Save/Load
 * ========================================================================= */

/** Save the current layout as a workspace. */
JI_API JiWorkspace* ji_workspace_new(const char* name);

/** Destroy a workspace. */
JI_API void ji_workspace_destroy(JiWorkspace* ws);

/** Serialize the current layout to JSON. Caller must free the string. */
JI_API char* ji_dock_pro_save_layout(const JiDockProManager* manager);

/** Load a layout from JSON string. */
JI_API bool ji_dock_pro_load_layout(JiDockProManager* manager, const char* json);

/** Save workspace to the manager's workspace list. */
JI_API bool ji_dock_pro_save_workspace(JiDockProManager* manager, const char* name);

/** Load a workspace by name from the manager's list. */
JI_API bool ji_dock_pro_load_workspace(JiDockProManager* manager, const char* name);

/** Get the number of saved workspaces. */
JI_API int ji_dock_pro_workspace_count(const JiDockProManager* manager);

/** Get a workspace by index. */
JI_API const JiWorkspace* ji_dock_pro_get_workspace(const JiDockProManager* manager, int index);

/* =========================================================================
 * Multi-Monitor Support
 * ========================================================================= */

/** Set the number of monitors and their geometries. */
JI_API void ji_dock_pro_set_monitors(JiDockProManager* manager,
                                       int count, const JiRect* geometries);

/** Get the number of monitors. */
JI_API int ji_dock_pro_monitor_count(const JiDockProManager* manager);

/** Get a monitor's geometry by index. */
JI_API JiRect ji_dock_pro_monitor_geometry(const JiDockProManager* manager, int index);

/** Move a floating panel to a specific monitor. */
JI_API bool ji_dock_pro_move_to_monitor(JiDockProManager* manager,
                                          JiFloatingPanel* panel, int monitor_index);

/* =========================================================================
 * Hover Preview
 * ========================================================================= */

/** Enable/disable hover-dock preview overlay. */
JI_API void ji_dock_pro_set_hover_preview(JiDockProManager* manager, bool enabled);

/** Set the current hover preview rectangle. */
JI_API void ji_dock_pro_set_preview_rect(JiDockProManager* manager, JiRect rect);

/** Get the current hover preview rectangle. */
JI_API JiRect ji_dock_pro_get_preview_rect(const JiDockProManager* manager);

/** Check if hover preview is active. */
JI_API bool ji_dock_pro_is_hover_preview(const JiDockProManager* manager);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_PRO_H */
