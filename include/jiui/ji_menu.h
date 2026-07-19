/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_menu.h
 * @brief Menu — popup menu with actions, separators, submenus.
 *        Also used for MenuBar and ContextMenu.
 */

#ifndef JIUI_MENU_H
#define JIUI_MENU_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiMenuItemType {
    JI_MENU_ITEM_ACTION    = 0,
    JI_MENU_ITEM_SEPARATOR = 1,
    JI_MENU_ITEM_SUBMENU   = 2
} JiMenuItemType;

typedef struct JiMenuItem {
    JiMenuItemType  type;
    char*           text;
    char*           shortcut;    /* e.g. "Ctrl+S" */
    bool            is_enabled;
    bool            is_checked;  /* for checkable actions */
    bool            is_checkable;
    struct JiMenu*  submenu;     /* only for JI_MENU_ITEM_SUBMENU */
} JiMenuItem;

typedef struct JiMenu {
    JiControl      control;        /* must be first */
    JiMenuItem*    items;
    int            item_count;
    int            item_capacity;
    char*          title;          /* for menu bar entries */
    bool           is_visible;
    int            hover_index;    /* currently hovered item */
} JiMenu;

/** Create a new menu. */
JI_API JiMenu* ji_menu_new(const char* title);

/** Destroy a menu. */
JI_API void ji_menu_destroy(JiMenu* menu);

/** Add an action item. Returns the index. */
JI_API int ji_menu_add_action(JiMenu* menu, const char* text);

/** Add a separator. */
JI_API void ji_menu_add_separator(JiMenu* menu);

/** Add a submenu. Returns the index. */
JI_API int ji_menu_add_submenu(JiMenu* menu, struct JiMenu* submenu);

/** Insert an action at the given index. */
JI_API void ji_menu_insert_action(JiMenu* menu, int index, const char* text);

/** Remove an item by index. */
JI_API void ji_menu_remove_item(JiMenu* menu, int index);

/** Get the number of items. */
JI_API int ji_menu_item_count(const JiMenu* menu);

/** Set the shortcut text for an action item. */
JI_API void ji_menu_set_shortcut(JiMenu* menu, int index, const char* shortcut);

/** Set whether an action item is enabled. */
JI_API void ji_menu_set_enabled(JiMenu* menu, int index, bool enabled);

/** Set whether an action item is checkable. */
JI_API void ji_menu_set_checkable(JiMenu* menu, int index, bool checkable);

/** Set the checked state of a checkable action. */
JI_API void ji_menu_set_checked(JiMenu* menu, int index, bool checked);

/** Get the menu title. */
JI_API const char* ji_menu_get_title(const JiMenu* menu);

/** Set the menu title. */
JI_API void ji_menu_set_title(JiMenu* menu, const char* title);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_MENU_H */
