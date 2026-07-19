/**
 * JiUI - Dock Overlay implementation
 */

#include <jiui/ji_dock_overlay.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>

JiDockOverlay* ji_dock_overlay_new(void) {
    JiDockOverlay* o = (JiDockOverlay*)ji_calloc(1, sizeof(JiDockOverlay));
    if (!o) { JI_ERROR_LOG("ji_dock_overlay_new: out of memory"); return NULL; }
    o->is_visible = false;
    o->highlighted_area = JI_DOCK_OVERLAY_NONE;
    o->is_tab_indicator = true;
    o->indicator_size = 40;
    return o;
}

void ji_dock_overlay_destroy(JiDockOverlay* overlay) { if (overlay) ji_free(overlay); }

void ji_dock_overlay_show(JiDockOverlay* overlay, int x, int y, int w, int h) {
    if (!overlay) return;
    overlay->is_visible = true;
    overlay->target_x = x; overlay->target_y = y;
    overlay->target_width = w; overlay->target_height = h;
    ji_dock_overlay_update_rects(overlay);
}

void ji_dock_overlay_hide(JiDockOverlay* overlay) { if (overlay) overlay->is_visible = false; }
void ji_dock_overlay_set_highlight(JiDockOverlay* overlay, JiDockOverlayArea area) { if (overlay) overlay->highlighted_area = area; }
JiDockOverlayArea ji_dock_overlay_get_highlight(const JiDockOverlay* overlay) { return overlay ? overlay->highlighted_area : JI_DOCK_OVERLAY_NONE; }

void ji_dock_overlay_update_rects(JiDockOverlay* overlay) {
    if (!overlay) return;
    int x = overlay->target_x, y = overlay->target_y;
    int w = overlay->target_width, h = overlay->target_height;
    int s = overlay->indicator_size;
    int half_w = w / 2, half_h = h / 2;
    /* Center */
    overlay->center_rect_x = x + half_w - s/2; overlay->center_rect_y = y + half_h - s/2;
    overlay->center_rect_w = s; overlay->center_rect_h = s;
    /* Left */
    overlay->left_rect_x = x; overlay->left_rect_y = y + half_h - s/2;
    overlay->left_rect_w = half_w - s/2; overlay->left_rect_h = s;
    /* Right */
    overlay->right_rect_x = x + half_w + s/2; overlay->right_rect_y = y + half_h - s/2;
    overlay->right_rect_w = half_w - s/2; overlay->right_rect_h = s;
    /* Top */
    overlay->top_rect_x = x + half_w - s/2; overlay->top_rect_y = y;
    overlay->top_rect_w = s; overlay->top_rect_h = half_h - s/2;
    /* Bottom */
    overlay->bottom_rect_x = x + half_w - s/2; overlay->bottom_rect_y = y + half_h + s/2;
    overlay->bottom_rect_w = s; overlay->bottom_rect_h = half_h - s/2;
}

static bool point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

JiDockOverlayArea ji_dock_overlay_hit_test(const JiDockOverlay* overlay, int x, int y) {
    if (!overlay || !overlay->is_visible) return JI_DOCK_OVERLAY_NONE;
    if (point_in_rect(x, y, overlay->center_rect_x, overlay->center_rect_y, overlay->center_rect_w, overlay->center_rect_h))
        return JI_DOCK_OVERLAY_CENTER;
    if (point_in_rect(x, y, overlay->left_rect_x, overlay->left_rect_y, overlay->left_rect_w, overlay->left_rect_h))
        return JI_DOCK_OVERLAY_LEFT;
    if (point_in_rect(x, y, overlay->right_rect_x, overlay->right_rect_y, overlay->right_rect_w, overlay->right_rect_h))
        return JI_DOCK_OVERLAY_RIGHT;
    if (point_in_rect(x, y, overlay->top_rect_x, overlay->top_rect_y, overlay->top_rect_w, overlay->top_rect_h))
        return JI_DOCK_OVERLAY_TOP;
    if (point_in_rect(x, y, overlay->bottom_rect_x, overlay->bottom_rect_y, overlay->bottom_rect_w, overlay->bottom_rect_h))
        return JI_DOCK_OVERLAY_BOTTOM;
    return JI_DOCK_OVERLAY_NONE;
}
