/**
 * JiUI - ScrollBar widget header
 */

#ifndef JIUI_SCROLL_BAR_H
#define JIUI_SCROLL_BAR_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiScrollBarOrientation {
    JI_SCROLLBAR_HORIZONTAL = 0,
    JI_SCROLLBAR_VERTICAL   = 1
} JiScrollBarOrientation;

typedef struct JiScrollBar {
    JiControl               control;
    JiScrollBarOrientation  orientation;
    int                     value;
    int                     minimum;
    int                     maximum;
    int                     page_step;
    int                     single_step;
    int                     slider_width;    /* default 16 */
    bool                    is_slider_down;
} JiScrollBar;

JI_API JiScrollBar* ji_scroll_bar_new(JiScrollBarOrientation orientation);
JI_API void ji_scroll_bar_destroy(JiScrollBar* sb);
JI_API void ji_scroll_bar_set_value(JiScrollBar* sb, int value);
JI_API int ji_scroll_bar_get_value(const JiScrollBar* sb);
JI_API void ji_scroll_bar_set_range(JiScrollBar* sb, int minimum, int maximum);
JI_API void ji_scroll_bar_set_page_step(JiScrollBar* sb, int step);
JI_API void ji_scroll_bar_set_single_step(JiScrollBar* sb, int step);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SCROLL_BAR_H */
