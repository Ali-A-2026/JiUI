/**
 * JiUI - ScrollArea implementation
 */

#include <jiui/ji_scroll_area.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>

JiScrollArea* ji_scroll_area_new(void) {
    JiScrollArea* area = (JiScrollArea*)ji_calloc(1, sizeof(JiScrollArea));
    if (!area) { JI_ERROR_LOG("ji_scroll_area_new: out of memory"); return NULL; }
    area->h_policy = JI_SCROLL_POLICY_AS_NEEDED;
    area->v_policy = JI_SCROLL_POLICY_AS_NEEDED;
    area->scrollbar_width = 16;
    area->widget_resizable = false;
    return area;
}

void ji_scroll_area_destroy(JiScrollArea* area) { if (area) ji_free(area); }
void ji_scroll_area_set_content(JiScrollArea* area, JiControl* content) { if (area) area->content = content; }
JiControl* ji_scroll_area_get_content(const JiScrollArea* area) { return area ? area->content : NULL; }
void ji_scroll_area_set_policy(JiScrollArea* area, JiScrollPolicy h, JiScrollPolicy v) { if (area) { area->h_policy = h; area->v_policy = v; } }
void ji_scroll_area_set_scroll(JiScrollArea* area, int x, int y) { if (area) { area->scroll_x = x; area->scroll_y = y; } }
void ji_scroll_area_get_scroll(const JiScrollArea* area, int* x, int* y) { if (area) { if (x) *x = area->scroll_x; if (y) *y = area->scroll_y; } }
void ji_scroll_area_set_content_size(JiScrollArea* area, int width, int height) { if (area) { area->content_width = width; area->content_height = height; } }
void ji_scroll_area_set_widget_resizable(JiScrollArea* area, bool resizable) { if (area) area->widget_resizable = resizable; }
bool ji_scroll_area_is_widget_resizable(const JiScrollArea* area) { return area ? area->widget_resizable : false; }
