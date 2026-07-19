/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_accessibility.h
 * @brief Accessibility system — screen reader support, keyboard navigation,
 *        high contrast, magnifier hints, accessible names, role/state system,
 *        announcement API.
 */

#ifndef JIUI_ACCESSIBILITY_H
#define JIUI_ACCESSIBILITY_H

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
 * Accessibility Constants
 * ========================================================================= */

#define JI_A11Y_MAX_WIDGETS    4096
#define JI_A11Y_MAX_NAME_LEN    256
#define JI_A11Y_MAX_DESC_LEN    512
#define JI_A11Y_MAX_ANNOUNCE    256

/* =========================================================================
 * Accessibility Roles (AT-SPI2 / WAI-ARIA compatible)
 * ========================================================================= */

typedef enum JiA11yRole {
    JI_A11Y_ROLE_NONE = 0,
    JI_A11Y_ROLE_BUTTON,
    JI_A11Y_ROLE_CHECKBOX,
    JI_A11Y_ROLE_RADIO_BUTTON,
    JI_A11Y_ROLE_TOGGLE_BUTTON,
    JI_A11Y_ROLE_TEXT,
    JI_A11Y_ROLE_TEXT_BOX,
    JI_A11Y_ROLE_SLIDER,
    JI_A11Y_ROLE_SPIN_BOX,
    JI_A11Y_ROLE_COMBO_BOX,
    JI_A11Y_ROLE_LIST,
    JI_A11Y_ROLE_LIST_ITEM,
    JI_A11Y_ROLE_TREE,
    JI_A11Y_ROLE_TREE_ITEM,
    JI_A11Y_ROLE_TABLE,
    JI_A11Y_ROLE_TABLE_CELL,
    JI_A11Y_ROLE_TAB,
    JI_A11Y_ROLE_TAB_LIST,
    JI_A11Y_ROLE_MENU,
    JI_A11Y_ROLE_MENU_ITEM,
    JI_A11Y_ROLE_TOOLBAR,
    JI_A11Y_ROLE_STATUS_BAR,
    JI_A11Y_ROLE_PROGRESS_BAR,
    JI_A11Y_ROLE_DIALOG,
    JI_A11Y_ROLE_WINDOW,
    JI_A11Y_ROLE_PANEL,
    JI_A11Y_ROLE_SCROLL_BAR,
    JI_A11Y_ROLE_SCROLL_AREA,
    JI_A11Y_ROLE_SEPARATOR,
    JI_A11Y_ROLE_LABEL,
    JI_A11Y_ROLE_IMAGE,
    JI_A11Y_ROLE_LINK,
    JI_A11Y_ROLE_HEADING,
    JI_A11Y_ROLE_GROUP
} JiA11yRole;

/* =========================================================================
 * Accessibility States
 * ========================================================================= */

typedef enum JiA11yState {
    JI_A11Y_STATE_NORMAL     = 0,
    JI_A11Y_STATE_FOCUSED    = 1 << 0,
    JI_A11Y_STATE_DISABLED   = 1 << 1,
    JI_A11Y_STATE_CHECKED    = 1 << 2,
    JI_A11Y_STATE_EXPANDED   = 1 << 3,
    JI_A11Y_STATE_COLLAPSED  = 1 << 4,
    JI_A11Y_STATE_SELECTED   = 1 << 5,
    JI_A11Y_STATE_PRESSED    = 1 << 6,
    JI_A11Y_STATE_READONLY   = 1 << 7,
    JI_A11Y_STATE_HIDDEN     = 1 << 8,
    JI_A11Y_STATE_BUSY       = 1 << 9,
    JI_A11Y_STATE_REQUIRED   = 1 << 10,
    JI_A11Y_STATE_INVALID   = 1 << 11,
    JI_A11Y_STATE_MODAL     = 1 << 12,
    JI_A11Y_STATE_MULTISELECTABLE = 1 << 13,
    JI_A11Y_STATE_EDITABLE  = 1 << 14
} JiA11yState;

/* =========================================================================
 * Accessibility Node
 * ========================================================================= */

typedef struct JiA11yNode {
    uint32_t    id;                         /* Unique ID */
    uint32_t    parent_id;                  /* 0 = root */
    JiA11yRole  role;
    uint32_t    states;                     /* Bitmask of JiA11yState */
    char        name[JI_A11Y_MAX_NAME_LEN];
    char        description[JI_A11Y_MAX_DESC_LEN];
    char        value[256];                 /* Current value (e.g. slider position) */
    int         tab_index;                 /* Tab navigation order (-1 = none) */
    JiRect      bounds;                     /* Screen bounds for hit testing */
    bool        visible;
} JiA11yNode;

/* =========================================================================
 * Accessibility Manager
 * ========================================================================= */

typedef struct JiA11yManager {
    JiA11yNode  nodes[JI_A11Y_MAX_WIDGETS];
    int         node_count;
    uint32_t    next_id;
    uint32_t    focused_node;               /* Currently focused node ID */
    bool        high_contrast;              /* High contrast mode */
    bool        screen_reader_active;       /* Screen reader detected */
    int         magnification;              /* Magnification level (1 = normal) */
    /* Announcement queue */
    char        announcements[JI_A11Y_MAX_ANNOUNCE];
    int         announce_count;
} JiA11yManager;

/* =========================================================================
 * Accessibility API — Lifecycle
 * ========================================================================= */

JI_API JiA11yManager* ji_a11y_new(void);
JI_API void           ji_a11y_free(JiA11yManager* mgr);

/* =========================================================================
 * Accessibility API — Node Management
 * ========================================================================= */

JI_API uint32_t ji_a11y_register_node(JiA11yManager* mgr, uint32_t parent_id,
                                        JiA11yRole role, const char* name);
JI_API int      ji_a11y_unregister_node(JiA11yManager* mgr, uint32_t id);
JI_API JiA11yNode* ji_a11y_get_node(JiA11yManager* mgr, uint32_t id);
JI_API int      ji_a11y_set_name(JiA11yManager* mgr, uint32_t id, const char* name);
JI_API int      ji_a11y_set_description(JiA11yManager* mgr, uint32_t id, const char* desc);
JI_API int      ji_a11y_set_value(JiA11yManager* mgr, uint32_t id, const char* value);
JI_API int      ji_a11y_set_role(JiA11yManager* mgr, uint32_t id, JiA11yRole role);
JI_API int      ji_a11y_set_state(JiA11yManager* mgr, uint32_t id, uint32_t states);
JI_API int      ji_a11y_add_state(JiA11yManager* mgr, uint32_t id, JiA11yState state);
JI_API int      ji_a11y_remove_state(JiA11yManager* mgr, uint32_t id, JiA11yState state);
JI_API int      ji_a11y_set_bounds(JiA11yManager* mgr, uint32_t id, JiRect bounds);
JI_API int      ji_a11y_set_tab_index(JiA11yManager* mgr, uint32_t id, int tab_index);

/* =========================================================================
 * Accessibility API — Focus & Navigation
 * ========================================================================= */

JI_API int      ji_a11y_focus(JiA11yManager* mgr, uint32_t id);
JI_API uint32_t ji_a11y_get_focused(const JiA11yManager* mgr);
JI_API uint32_t ji_a11y_focus_next(JiA11yManager* mgr);   /* Tab forward */
JI_API uint32_t ji_a11y_focus_prev(JiA11yManager* mgr);   /* Tab backward */
JI_API uint32_t ji_a11y_focus_first(JiA11yManager* mgr);
JI_API uint32_t ji_a11y_focus_last(JiA11yManager* mgr);

/* =========================================================================
 * Accessibility API — Announcement
 * ========================================================================= */

JI_API int      ji_a11y_announce(JiA11yManager* mgr, const char* message);
JI_API const char* ji_a11y_get_announcement(JiA11yManager* mgr, int index);
JI_API int      ji_a11y_get_announcement_count(const JiA11yManager* mgr);
JI_API void     ji_a11y_clear_announcements(JiA11yManager* mgr);

/* =========================================================================
 * Accessibility API — Settings
 * ========================================================================= */

JI_API void     ji_a11y_set_high_contrast(JiA11yManager* mgr, bool enabled);
JI_API bool     ji_a11y_get_high_contrast(const JiA11yManager* mgr);
JI_API void     ji_a11y_set_screen_reader(JiA11yManager* mgr, bool active);
JI_API bool     ji_a11y_get_screen_reader(const JiA11yManager* mgr);
JI_API void     ji_a11y_set_magnification(JiA11yManager* mgr, int level);
JI_API int      ji_a11y_get_magnification(const JiA11yManager* mgr);

/* =========================================================================
 * Accessibility API — AT-SPI2 Backend (Linux)
 * ========================================================================= */

JI_API int      ji_a11y_atspi_init(void);
JI_API void     ji_a11y_atspi_shutdown(void);
JI_API int      ji_a11y_atspi_emit_event(uint32_t id, const char* event_type,
                                           const char* data);
JI_API bool     ji_a11y_atspi_is_available(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_ACCESSIBILITY_H */
