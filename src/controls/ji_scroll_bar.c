/**
 * JiUI - ScrollBar implementation
 */

#include <jiui/ji_scroll_bar.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>

JiScrollBar* ji_scroll_bar_new(JiScrollBarOrientation orientation) {
    JiScrollBar* sb = (JiScrollBar*)ji_calloc(1, sizeof(JiScrollBar));
    if (!sb) { JI_ERROR_LOG("ji_scroll_bar_new: out of memory"); return NULL; }
    sb->orientation = orientation;
    sb->minimum = 0;
    sb->maximum = 100;
    sb->value = 0;
    sb->page_step = 20;
    sb->single_step = 1;
    sb->slider_width = 16;
    sb->is_slider_down = false;
    return sb;
}

void ji_scroll_bar_destroy(JiScrollBar* sb) { if (sb) ji_free(sb); }

void ji_scroll_bar_set_value(JiScrollBar* sb, int value) {
    if (!sb) return;
    if (value < sb->minimum) value = sb->minimum;
    if (value > sb->maximum) value = sb->maximum;
    sb->value = value;
}

int ji_scroll_bar_get_value(const JiScrollBar* sb) { return sb ? sb->value : 0; }

void ji_scroll_bar_set_range(JiScrollBar* sb, int minimum, int maximum) {
    if (!sb) return;
    sb->minimum = minimum;
    sb->maximum = maximum;
    if (sb->value < minimum) sb->value = minimum;
    if (sb->value > maximum) sb->value = maximum;
}

void ji_scroll_bar_set_page_step(JiScrollBar* sb, int step) { if (sb && step > 0) sb->page_step = step; }
void ji_scroll_bar_set_single_step(JiScrollBar* sb, int step) { if (sb && step > 0) sb->single_step = step; }
