/**
 * JiUI - Menu implementation
 */

#include <jiui/ji_menu.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiMenu* menu) {
    if (menu->item_count < menu->item_capacity) return;
    int new_cap = menu->item_capacity * 2;
    JiMenuItem* new_arr = (JiMenuItem*)ji_alloc(sizeof(JiMenuItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, menu->items, sizeof(JiMenuItem) * menu->item_count);
    ji_free(menu->items);
    menu->items = new_arr;
    menu->item_capacity = new_cap;
}

JiMenu* ji_menu_new(const char* title) {
    JiMenu* menu = (JiMenu*)ji_calloc(1, sizeof(JiMenu));
    if (!menu) { JI_ERROR_LOG("ji_menu_new: out of memory"); return NULL; }
    menu->item_capacity = 8;
    menu->items = (JiMenuItem*)ji_alloc(sizeof(JiMenuItem) * menu->item_capacity);
    if (title) {
        size_t len = strlen(title);
        menu->title = (char*)ji_alloc(len + 1);
        if (menu->title) memcpy(menu->title, title, len + 1);
    }
    menu->is_visible = false;
    menu->hover_index = -1;
    return menu;
}

void ji_menu_destroy(JiMenu* menu) {
    if (!menu) return;
    for (int i = 0; i < menu->item_count; i++) {
        ji_free(menu->items[i].text);
        ji_free(menu->items[i].shortcut);
    }
    ji_free(menu->items);
    ji_free(menu->title);
    ji_free(menu);
}

int ji_menu_add_action(JiMenu* menu, const char* text) {
    if (!menu) return -1;
    grow_items(menu);
    JiMenuItem* item = &menu->items[menu->item_count++];
    memset(item, 0, sizeof(*item));
    item->type = JI_MENU_ITEM_ACTION;
    if (text) {
        size_t len = strlen(text);
        item->text = (char*)ji_alloc(len + 1);
        if (item->text) memcpy(item->text, text, len + 1);
    }
    item->is_enabled = true;
    return menu->item_count - 1;
}

void ji_menu_add_separator(JiMenu* menu) {
    if (!menu) return;
    grow_items(menu);
    JiMenuItem* item = &menu->items[menu->item_count++];
    memset(item, 0, sizeof(*item));
    item->type = JI_MENU_ITEM_SEPARATOR;
}

int ji_menu_add_submenu(JiMenu* menu, struct JiMenu* submenu) {
    if (!menu) return -1;
    grow_items(menu);
    JiMenuItem* item = &menu->items[menu->item_count++];
    memset(item, 0, sizeof(*item));
    item->type = JI_MENU_ITEM_SUBMENU;
    item->submenu = submenu;
    if (submenu && submenu->title) {
        size_t len = strlen(submenu->title);
        item->text = (char*)ji_alloc(len + 1);
        if (item->text) memcpy(item->text, submenu->title, len + 1);
    }
    item->is_enabled = true;
    return menu->item_count - 1;
}

void ji_menu_insert_action(JiMenu* menu, int index, const char* text) {
    if (!menu || index < 0 || index > menu->item_count) return;
    grow_items(menu);
    for (int i = menu->item_count; i > index; i--) menu->items[i] = menu->items[i - 1];
    JiMenuItem* item = &menu->items[index];
    memset(item, 0, sizeof(*item));
    item->type = JI_MENU_ITEM_ACTION;
    if (text) {
        size_t len = strlen(text);
        item->text = (char*)ji_alloc(len + 1);
        if (item->text) memcpy(item->text, text, len + 1);
    }
    item->is_enabled = true;
    menu->item_count++;
}

void ji_menu_remove_item(JiMenu* menu, int index) {
    if (!menu || index < 0 || index >= menu->item_count) return;
    ji_free(menu->items[index].text);
    ji_free(menu->items[index].shortcut);
    for (int i = index; i < menu->item_count - 1; i++) menu->items[i] = menu->items[i + 1];
    menu->item_count--;
}

int ji_menu_item_count(const JiMenu* menu) { return menu ? menu->item_count : 0; }

void ji_menu_set_shortcut(JiMenu* menu, int index, const char* shortcut) {
    if (!menu || index < 0 || index >= menu->item_count) return;
    ji_free(menu->items[index].shortcut);
    if (shortcut) {
        size_t len = strlen(shortcut);
        menu->items[index].shortcut = (char*)ji_alloc(len + 1);
        if (menu->items[index].shortcut) memcpy(menu->items[index].shortcut, shortcut, len + 1);
    } else { menu->items[index].shortcut = NULL; }
}

void ji_menu_set_enabled(JiMenu* menu, int index, bool enabled) {
    if (menu && index >= 0 && index < menu->item_count) menu->items[index].is_enabled = enabled;
}

void ji_menu_set_checkable(JiMenu* menu, int index, bool checkable) {
    if (menu && index >= 0 && index < menu->item_count) menu->items[index].is_checkable = checkable;
}

void ji_menu_set_checked(JiMenu* menu, int index, bool checked) {
    if (menu && index >= 0 && index < menu->item_count) menu->items[index].is_checked = checked;
}

const char* ji_menu_get_title(const JiMenu* menu) { return menu ? menu->title : NULL; }

void ji_menu_set_title(JiMenu* menu, const char* title) {
    if (!menu) return;
    ji_free(menu->title);
    if (title) {
        size_t len = strlen(title);
        menu->title = (char*)ji_alloc(len + 1);
        if (menu->title) memcpy(menu->title, title, len + 1);
    } else { menu->title = NULL; }
}
