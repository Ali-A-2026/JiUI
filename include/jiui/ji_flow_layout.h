/**
 * JiUI - FlowLayout widget header
 */

#ifndef JIUI_FLOW_LAYOUT_H
#define JIUI_FLOW_LAYOUT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JiFlowLayoutItem {
    JiControl*  control;
    int         stretch;
} JiFlowLayoutItem;

typedef struct JiFlowLayout {
    JiControl          control;
    JiFlowLayoutItem*  items;
    int                item_count;
    int                item_capacity;
    int                spacing;       /* horizontal and vertical spacing (default 6) */
    int                margin_left;
    int                margin_top;
    int                margin_right;
    int                margin_bottom;
    bool               wrap;         /* wrap to next line (default true) */
} JiFlowLayout;

JI_API JiFlowLayout* ji_flow_layout_new(void);
JI_API void ji_flow_layout_destroy(JiFlowLayout* layout);
JI_API void ji_flow_layout_add(JiFlowLayout* layout, JiControl* control);
JI_API int ji_flow_layout_count(const JiFlowLayout* layout);
JI_API void ji_flow_layout_set_spacing(JiFlowLayout* layout, int spacing);
JI_API void ji_flow_layout_set_margins(JiFlowLayout* layout, int left, int top, int right, int bottom);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_FLOW_LAYOUT_H */
