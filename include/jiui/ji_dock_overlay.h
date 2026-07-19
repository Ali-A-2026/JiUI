/**
 * JiUI - Dock Overlay header
 */

#ifndef JIUI_DOCK_OVERLAY_H
#define JIUI_DOCK_OVERLAY_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JiDockOverlayArea {
    JI_DOCK_OVERLAY_NONE = 0,
    JI_DOCK_OVERLAY_CENTER = 1,
    JI_DOCK_OVERLAY_LEFT = 2,
    JI_DOCK_OVERLAY_RIGHT = 3,
    JI_DOCK_OVERLAY_TOP = 4,
    JI_DOCK_OVERLAY_BOTTOM = 5
} JiDockOverlayArea;

typedef struct JiDockOverlay {
    bool               is_visible;
    int                target_x;
    int                target_y;
    int                target_width;
    int                target_height;
    JiDockOverlayArea  highlighted_area;
    bool               is_tab_indicator;   /* shows tab indicator on center */
    int                indicator_size;     /* default 40 */
    int                center_rect_x, center_rect_y, center_rect_w, center_rect_h;
    int                left_rect_x, left_rect_y, left_rect_w, left_rect_h;
    int                right_rect_x, right_rect_y, right_rect_w, right_rect_h;
    int                top_rect_x, top_rect_y, top_rect_w, top_rect_h;
    int                bottom_rect_x, bottom_rect_y, bottom_rect_w, bottom_rect_h;
} JiDockOverlay;

JI_API JiDockOverlay* ji_dock_overlay_new(void);
JI_API void ji_dock_overlay_destroy(JiDockOverlay* overlay);
JI_API void ji_dock_overlay_show(JiDockOverlay* overlay, int x, int y, int w, int h);
JI_API void ji_dock_overlay_hide(JiDockOverlay* overlay);
JI_API void ji_dock_overlay_set_highlight(JiDockOverlay* overlay, JiDockOverlayArea area);
JI_API JiDockOverlayArea ji_dock_overlay_get_highlight(const JiDockOverlay* overlay);
JI_API void ji_dock_overlay_update_rects(JiDockOverlay* overlay);
JI_API JiDockOverlayArea ji_dock_overlay_hit_test(const JiDockOverlay* overlay, int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DOCK_OVERLAY_H */
