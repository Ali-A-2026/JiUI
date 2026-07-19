/**
 * JiUI - BoxLayout implementation
 */

#include <jiui/ji_box_layout.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiBoxLayout* layout) {
    if (layout->item_count < layout->item_capacity) return;
    int new_cap = layout->item_capacity * 2;
    JiBoxLayoutItem* new_arr = (JiBoxLayoutItem*)ji_alloc(sizeof(JiBoxLayoutItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, layout->items, sizeof(JiBoxLayoutItem) * layout->item_count);
    ji_free(layout->items);
    layout->items = new_arr;
    layout->item_capacity = new_cap;
}

JiBoxLayout* ji_box_layout_new(JiBoxLayoutDirection direction) {
    JiBoxLayout* layout = (JiBoxLayout*)ji_calloc(1, sizeof(JiBoxLayout));
    if (!layout) { JI_ERROR_LOG("ji_box_layout_new: out of memory"); return NULL; }
    layout->direction = direction;
    layout->item_capacity = 8;
    layout->items = (JiBoxLayoutItem*)ji_alloc(sizeof(JiBoxLayoutItem) * layout->item_capacity);
    layout->spacing = 6;
    layout->margin_left = 0;
    layout->margin_top = 0;
    layout->margin_right = 0;
    layout->margin_bottom = 0;
    return layout;
}

void ji_box_layout_destroy(JiBoxLayout* layout) { if (layout) { ji_free(layout->items); ji_free(layout); } }

void ji_box_layout_add(JiBoxLayout* layout, JiControl* control, int stretch) {
    if (!layout) return;
    grow_items(layout);
    JiBoxLayoutItem* item = &layout->items[layout->item_count++];
    memset(item, 0, sizeof(*item));
    item->control = control;
    item->stretch = stretch;
    item->is_spacer = false;
}

void ji_box_layout_add_spacer(JiBoxLayout* layout, int size) {
    if (!layout) return;
    grow_items(layout);
    JiBoxLayoutItem* item = &layout->items[layout->item_count++];
    memset(item, 0, sizeof(*item));
    item->is_spacer = true;
    item->spacer_size = size;
}

void ji_box_layout_add_stretch(JiBoxLayout* layout, int stretch) {
    if (!layout) return;
    grow_items(layout);
    JiBoxLayoutItem* item = &layout->items[layout->item_count++];
    memset(item, 0, sizeof(*item));
    item->is_spacer = true;
    item->spacer_size = 0;
    item->stretch = stretch;
}

void ji_box_layout_set_direction(JiBoxLayout* layout, JiBoxLayoutDirection direction) { if (layout) layout->direction = direction; }
void ji_box_layout_set_spacing(JiBoxLayout* layout, int spacing) { if (layout) layout->spacing = spacing; }
void ji_box_layout_set_margins(JiBoxLayout* layout, int left, int top, int right, int bottom) { if (layout) { layout->margin_left = left; layout->margin_top = top; layout->margin_right = right; layout->margin_bottom = bottom; } }
int ji_box_layout_count(const JiBoxLayout* layout) { return layout ? layout->item_count : 0; }
