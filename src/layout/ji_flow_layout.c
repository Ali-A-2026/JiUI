/**
 * JiUI - FlowLayout implementation
 */

#include <jiui/ji_flow_layout.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

static void grow_items(JiFlowLayout* layout) {
    if (layout->item_count < layout->item_capacity) return;
    int new_cap = layout->item_capacity * 2;
    JiFlowLayoutItem* new_arr = (JiFlowLayoutItem*)ji_alloc(sizeof(JiFlowLayoutItem) * new_cap);
    if (!new_arr) return;
    memcpy(new_arr, layout->items, sizeof(JiFlowLayoutItem) * layout->item_count);
    ji_free(layout->items);
    layout->items = new_arr;
    layout->item_capacity = new_cap;
}

JiFlowLayout* ji_flow_layout_new(void) {
    JiFlowLayout* layout = (JiFlowLayout*)ji_calloc(1, sizeof(JiFlowLayout));
    if (!layout) { JI_ERROR_LOG("ji_flow_layout_new: out of memory"); return NULL; }
    layout->item_capacity = 8;
    layout->items = (JiFlowLayoutItem*)ji_alloc(sizeof(JiFlowLayoutItem) * layout->item_capacity);
    layout->spacing = 6;
    layout->margin_left = 0;
    layout->margin_top = 0;
    layout->margin_right = 0;
    layout->margin_bottom = 0;
    layout->wrap = true;
    return layout;
}

void ji_flow_layout_destroy(JiFlowLayout* layout) { if (layout) { ji_free(layout->items); ji_free(layout); } }

void ji_flow_layout_add(JiFlowLayout* layout, JiControl* control) {
    if (!layout) return;
    grow_items(layout);
    JiFlowLayoutItem* item = &layout->items[layout->item_count++];
    memset(item, 0, sizeof(*item));
    item->control = control;
}

int ji_flow_layout_count(const JiFlowLayout* layout) { return layout ? layout->item_count : 0; }
void ji_flow_layout_set_spacing(JiFlowLayout* layout, int spacing) { if (layout) layout->spacing = spacing; }
void ji_flow_layout_set_margins(JiFlowLayout* layout, int left, int top, int right, int bottom) { if (layout) { layout->margin_left = left; layout->margin_top = top; layout->margin_right = right; layout->margin_bottom = bottom; } }
