/**
 * JiUI - BoxLayout widget header
 */

#ifndef JIUI_BOX_LAYOUT_H
#define JIUI_BOX_LAYOUT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiBoxLayoutDirection {
    JI_BOX_LEFT_TO_RIGHT = 0,
    JI_BOX_RIGHT_TO_LEFT = 1,
    JI_BOX_TOP_TO_BOTTOM = 2,
    JI_BOX_BOTTOM_TO_TOP = 3
} JiBoxLayoutDirection;

typedef struct JiBoxLayoutItem {
    JiControl*  control;
    int         stretch;
    bool        is_spacer;
    int         spacer_size;
} JiBoxLayoutItem;

typedef struct JiBoxLayout {
    JiControl            control;
    JiBoxLayoutDirection direction;
    JiBoxLayoutItem*     items;
    int                  item_count;
    int                  item_capacity;
    int                  spacing;       /* spacing between items (default 6) */
    int                  margin_left;
    int                  margin_top;
    int                  margin_right;
    int                  margin_bottom;
} JiBoxLayout;

JI_API JiBoxLayout* ji_box_layout_new(JiBoxLayoutDirection direction);
JI_API void ji_box_layout_destroy(JiBoxLayout* layout);
JI_API void ji_box_layout_add(JiBoxLayout* layout, JiControl* control, int stretch);
JI_API void ji_box_layout_add_spacer(JiBoxLayout* layout, int size);
JI_API void ji_box_layout_add_stretch(JiBoxLayout* layout, int stretch);
JI_API void ji_box_layout_set_direction(JiBoxLayout* layout, JiBoxLayoutDirection direction);
JI_API void ji_box_layout_set_spacing(JiBoxLayout* layout, int spacing);
JI_API void ji_box_layout_set_margins(JiBoxLayout* layout, int left, int top, int right, int bottom);
JI_API int ji_box_layout_count(const JiBoxLayout* layout);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_BOX_LAYOUT_H */
