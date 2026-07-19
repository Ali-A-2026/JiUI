/**
 * JiUI - StatusBar implementation
 */

#include <jiui/ji_status_bar.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiStatusBar* sb) {
    if (sb->item_count < sb->item_capacity) return;
    int new_cap = sb->item_capacity * 2;
    JiStatusBarItem* new_arr = (JiStatusBarItem*)ji_alloc(sizeof(JiStatusBarItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, sb->items, sizeof(JiStatusBarItem) * sb->item_count);
    ji_free(sb->items);
    sb->items = new_arr;
    sb->item_capacity = new_cap;
}

JiStatusBar* ji_status_bar_new(void) {
    JiStatusBar* sb = (JiStatusBar*)ji_calloc(1, sizeof(JiStatusBar));
    if (!sb) { JI_ERROR_LOG("ji_status_bar_new: out of memory"); return NULL; }
    sb->item_capacity = 4;
    sb->items = (JiStatusBarItem*)ji_alloc(sizeof(JiStatusBarItem) * sb->item_capacity);
    sb->height = 22;
    return sb;
}

void ji_status_bar_destroy(JiStatusBar* sb) {
    if (!sb) return;
    for (int i = 0; i < sb->item_count; i++) ji_free(sb->items[i].label);
    ji_free(sb->items);
    ji_free(sb->current_message);
    ji_free(sb);
}

void ji_status_bar_show_message(JiStatusBar* sb, const char* message) {
    if (!sb) return;
    ji_free(sb->current_message);
    if (message) {
        size_t len = strlen(message);
        sb->current_message = (char*)ji_alloc(len + 1);
        if (sb->current_message) memcpy(sb->current_message, message, len + 1);
    } else { sb->current_message = NULL; }
}

void ji_status_bar_clear_message(JiStatusBar* sb) {
    if (!sb) return;
    ji_free(sb->current_message);
    sb->current_message = NULL;
}

void ji_status_bar_add_permanent_widget(JiStatusBar* sb, JiControl* widget) {
    if (!sb) return;
    grow_items(sb);
    JiStatusBarItem* item = &sb->items[sb->item_count++];
    memset(item, 0, sizeof(*item));
    item->widget = widget;
    item->is_permanent = true;
}

void ji_status_bar_add_widget(JiStatusBar* sb, JiControl* widget) {
    if (!sb) return;
    grow_items(sb);
    JiStatusBarItem* item = &sb->items[sb->item_count++];
    memset(item, 0, sizeof(*item));
    item->widget = widget;
    item->is_permanent = false;
}

const char* ji_status_bar_get_current_message(const JiStatusBar* sb) { return sb ? sb->current_message : NULL; }
