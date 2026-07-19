/**
 * JiUI - ToolBar implementation
 */

#include <jiui/ji_tool_bar.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiToolBar* tb) {
    if (tb->item_count < tb->item_capacity) return;
    int new_cap = tb->item_capacity * 2;
    JiToolBarItem* new_arr = (JiToolBarItem*)ji_alloc(sizeof(JiToolBarItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, tb->items, sizeof(JiToolBarItem) * tb->item_count);
    ji_free(tb->items);
    tb->items = new_arr;
    tb->item_capacity = new_cap;
}

JiToolBar* ji_tool_bar_new(void) {
    JiToolBar* tb = (JiToolBar*)ji_calloc(1, sizeof(JiToolBar));
    if (!tb) { JI_ERROR_LOG("ji_tool_bar_new: out of memory"); return NULL; }
    tb->item_capacity = 8;
    tb->items = (JiToolBarItem*)ji_alloc(sizeof(JiToolBarItem) * tb->item_capacity);
    tb->area = JI_TOOL_BAR_TOP;
    tb->is_movable = true;
    tb->icon_size = 24;
    tb->button_style = 0;
    return tb;
}

void ji_tool_bar_destroy(JiToolBar* tb) {
    if (!tb) return;
    for (int i = 0; i < tb->item_count; i++) { ji_free(tb->items[i].name); ji_free(tb->items[i].label); }
    ji_free(tb->items);
    ji_free(tb);
}

void ji_tool_bar_add_action(JiToolBar* tb, const char* name, const char* label) {
    if (!tb) return;
    grow_items(tb);
    JiToolBarItem* item = &tb->items[tb->item_count++];
    memset(item, 0, sizeof(*item));
    if (name) { size_t len = strlen(name); item->name = (char*)ji_alloc(len+1); if (item->name) memcpy(item->name, name, len+1); }
    if (label) { size_t len = strlen(label); item->label = (char*)ji_alloc(len+1); if (item->label) memcpy(item->label, label, len+1); }
    item->is_separator = false;
}

void ji_tool_bar_add_widget(JiToolBar* tb, JiControl* widget) {
    if (!tb) return;
    grow_items(tb);
    JiToolBarItem* item = &tb->items[tb->item_count++];
    memset(item, 0, sizeof(*item));
    item->widget = widget;
    item->is_separator = false;
}

void ji_tool_bar_add_separator(JiToolBar* tb) {
    if (!tb) return;
    grow_items(tb);
    JiToolBarItem* item = &tb->items[tb->item_count++];
    memset(item, 0, sizeof(*item));
    item->is_separator = true;
}

void ji_tool_bar_set_area(JiToolBar* tb, JiToolBarArea area) { if (tb) tb->area = area; }
void ji_tool_bar_set_movable(JiToolBar* tb, bool movable) { if (tb) tb->is_movable = movable; }
void ji_tool_bar_set_icon_size(JiToolBar* tb, int size) { if (tb) tb->icon_size = size; }
