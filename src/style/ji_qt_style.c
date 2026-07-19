/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_qt_style.c
 * @brief Qt6 Fusion-style renderer implementation — draws widgets using
 *        palette color roles with pixel-perfect metrics matching Qt6 Fusion.
 */

#include <jiui/ji_qt_style.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <jiui/ji_config.h>

#ifdef JIUI_ENABLE_OPENGL
#  include <GL/gl.h>
#else
/* ---- No-op stubs when OpenGL is disabled ---- */
#  define GL_QUADS          0x0007
#  define GL_LINE_LOOP      0x0002
#  define GL_LINES          0x0001
#  define GL_TRIANGLES      0x0004
#  define GL_TRIANGLE_FAN   0x0006
#  define GL_LINE_STRIP     0x0003
#  define GL_LINE_STIPPLE   0x0B24
#  define GL_TEXTURE_2D     0x0DE1
#  define GL_BLEND          0x0BE2
#  define GL_DEPTH_TEST     0x0B71
#  define GL_LIGHTING       0x0B50
#  define GL_LIGHT0         0x4000
#  define GL_COLOR_MATERIAL 0x0B57
#  define GL_FRONT_AND_BACK 0x0408
#  define GL_AMBIENT_AND_DIFFUSE 0x1602
#  define GL_LESS           0x0201
#  define GL_SRC_ALPHA      0x0302
#  define GL_ONE_MINUS_SRC_ALPHA 0x0303
#  define GL_NEAREST        0x2600
#  define GL_RGBA           0x1908
#  define GL_UNSIGNED_BYTE  0x1401
#  define GL_TEXTURE_MIN_FILTER 0x2801
#  define GL_TEXTURE_MAG_FILTER 0x2800
#  define GL_PROJECTION     0x1701
#  define GL_MODELVIEW      0x1700
#  define GLfloat           float
static inline void glColor4f(float a, float b, float c, float d) { (void)a;(void)b;(void)c;(void)d; }
static inline void glBegin(unsigned int m) { (void)m; }
static inline void glEnd(void) {}
static inline void glVertex2f(float a, float b) { (void)a;(void)b; }
static inline void glLineWidth(float w) { (void)w; }
static inline void glLineStipple(int a, unsigned short b) { (void)a;(void)b; }
static inline void glEnable(unsigned int m) { (void)m; }
static inline void glDisable(unsigned int m) { (void)m; }
#endif

#include <math.h>
#include <string.h>

/* ---- ARGB helpers ---- */
#define ARGB(a, r, g, b) (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | \
                           ((uint32_t)(g) << 8) | (uint32_t)(b))
#define ALPHA(c) ((uint8_t)(((c) >> 24) & 0xFF))
#define RED(c)   ((uint8_t)(((c) >> 16) & 0xFF))
#define GREEN(c) ((uint8_t)(((c) >> 8) & 0xFF))
#define BLUE(c)  ((uint8_t)((c) & 0xFF))

/* ---- Local drawing helpers (OpenGL immediate mode) ---- */
/* These match the draw_* helpers used in the showcase example */

static void draw_filled_rect(float x, float y, float w, float h,
                             float r, float g, float b, float a,
                             int screen_w, int screen_h) {
    (void)screen_w; (void)screen_h;
    /* Use OpenGL immediate mode — same pattern as showcase */
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_rect_outline(float x, float y, float w, float h,
                              float r, float g, float b, float a,
                              int screen_w, int screen_h) {
    (void)screen_w; (void)screen_h;
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_text_simple(const char* text, float x, float y,
                             float r, float g, float b, float a,
                             int screen_w, int screen_h) {
    (void)screen_w; (void)screen_h;
    /* Delegate to platform text rendering — simplified stub for now */
    /* The actual text rendering is handled by the showcase's draw_text */
    (void)text; (void)x; (void)y; (void)r; (void)g; (void)b; (void)a;
}

static void draw_triangle(float x1, float y1, float x2, float y2,
                          float x3, float y3,
                          float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

static void draw_dashed_rect(float x, float y, float w, float h,
                             float r, float g, float b, float a) {
    /* Simple dashed focus rectangle */
    glColor4f(r, g, b, a);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xCCCC);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

/* ---- Color normalization ---- */
static float cf(uint32_t argb, int channel) {
    switch (channel) {
        case 0: return (float)RED(argb) / 255.0f;
        case 1: return (float)GREEN(argb) / 255.0f;
        case 2: return (float)BLUE(argb) / 255.0f;
        case 3: return (float)ALPHA(argb) / 255.0f;
        default: return 0.0f;
    }
}

/* ---- Lifecycle ---- */

JiQtStyle* ji_qt_style_new(const JiPalette* palette) {
    JiQtStyle* style = (JiQtStyle*)ji_calloc(1, sizeof(JiQtStyle));
    if (!style) {
        JI_ERROR_LOG("ji_qt_style_new: out of memory");
        return NULL;
    }
    style->palette = palette;
    return style;
}

void ji_qt_style_destroy(JiQtStyle* style) {
    ji_free(style);
}

/* ---- Pixel metrics ---- */

int ji_qt_style_pixel_metric(JiPixelMetric metric) {
    switch (metric) {
        case JI_PM_BUTTON_MARGIN:          return 6;
        case JI_PM_BUTTON_MARGIN_TOP:     return 4;
        case JI_PM_BUTTON_MARGIN_BOTTOM:  return 4;
        case JI_PM_BUTTON_MARGIN_LEFT:    return 6;
        case JI_PM_BUTTON_MARGIN_RIGHT:   return 6;
        case JI_PM_CHECKBOX_SIZE:         return 13;
        case JI_PM_RADIOBUTTON_SIZE:      return 13;
        case JI_PM_SLIDER_THICKNESS:      return 16;
        case JI_PM_SLIDER_LENGTH:         return 11;
        case JI_PM_SLIDER_CONTROL:        return 11;
        case JI_PM_PROGRESSBAR_THICKNESS: return 4;
        case JI_PM_SCROLLBAR_EXTENT:      return 14;
        case JI_PM_SCROLLBAR_SLIDER:      return 12;
        case JI_PM_COMBOBOX_MARGIN:       return 2;
        case JI_PM_TEXTBOX_MARGIN:        return 2;
        case JI_PM_FOCUS_RECT_WIDTH:      return 1;
        case JI_PM_FOCUS_RECT_MARGIN:     return 2;
        case JI_PM_FRAME_BORDER_WIDTH:    return 1;
        case JI_PM_TOOLBAR_MARGIN:        return 2;
        case JI_PM_TOOLBAR_SEPARATOR:     return 8;
        case JI_PM_TAB_BAR_HEIGHT:        return 24;
        case JI_PM_MENU_BAR_HEIGHT:       return 24;
        case JI_PM_MENU_ITEM_HEIGHT:      return 24;
        case JI_PM_STATUS_BAR_HEIGHT:     return 22;
        case JI_PM_GROUP_BOX_BORDER:      return 1;
        case JI_PM_DOCK_WIDGET_TITLE_HEIGHT: return 20;
        case JI_PM_SPLITTER_WIDTH:        return 4;
        default: return 0;
    }
}

/* ---- Color resolution ---- */

uint32_t ji_qt_style_resolve_color(JiQtStyle* style, JiColorRole role,
                                       JiWidgetState state) {
    if (!style || !style->palette) return 0;

    JiColorGroup group = JI_COLOR_GROUP_ACTIVE;
    if (state & JI_STATE_DISABLED)
        group = JI_COLOR_GROUP_DISABLED;
    else if (state & JI_STATE_SELECTED)
        group = JI_COLOR_GROUP_INACTIVE;

    return ji_palette_get_color(style->palette, group, role);
}

/* ---- Button ---- */

void ji_qt_draw_button(JiQtStyle* style, JiDrawingContext* ctx,
                         JiRect rect, const char* text,
                         JiWidgetState state, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    /* Resolve colors */
    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON_TEXT, state);
    uint32_t light = ji_qt_style_resolve_color(style, JI_ROLE_LIGHT, state);

    /* Hover/press adjustments */
    if (state & JI_STATE_PRESSED) {
        bg = ji_qt_style_resolve_color(style, JI_ROLE_MID, state);
    } else if (state & JI_STATE_HOVER) {
        /* Slightly lighten */
        bg = ji_qt_style_resolve_color(style, JI_ROLE_LIGHT, state);
    }

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);

    /* Top highlight (3D effect) */
    draw_filled_rect(x, y, w, 1.0f, cf(light,0), cf(light,1), cf(light,2), 0.5f, sw, sh);

    /* Border (Qt6 Fusion: 1px solid) */
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.8f, sw, sh);

    /* Focus rectangle */
    if (state & JI_STATE_FOCUSED) {
        int fm = ji_qt_style_pixel_metric(JI_PM_FOCUS_RECT_MARGIN);
        draw_dashed_rect(x + fm, y + fm, w - 2*fm, h - 2*fm,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 0.5f);
    }

    /* Text */
    if (text) {
        draw_text_simple(text, x + 6.0f, y + (h - 16.0f) / 2.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Checkbox ---- */

void ji_qt_draw_checkbox(JiQtStyle* style, JiDrawingContext* ctx,
                           JiRect rect, const char* text,
                           JiWidgetState state, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    int sz = ji_qt_style_pixel_metric(JI_PM_CHECKBOX_SIZE);
    float cx = x, cy = y + ((float)rect.height - sz) / 2.0f;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BASE, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);
    uint32_t check_col = ji_qt_style_resolve_color(style, JI_ROLE_TEXT, state);

    /* Checkbox background */
    draw_filled_rect(cx, cy, sz, sz, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    draw_rect_outline(cx, cy, sz, sz, cf(border,0), cf(border,1), cf(border,2), 0.8f, sw, sh);

    /* Checkmark */
    if (state & JI_STATE_CHECKED) {
        /* Draw a checkmark using lines */
        glColor4f(cf(check_col,0), cf(check_col,1), cf(check_col,2), 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(cx + 3, cy + sz/2);
        glVertex2f(cx + sz/2 - 1, cy + sz - 4);
        glVertex2f(cx + sz - 3, cy + 3);
        glEnd();
        glLineWidth(1.0f);
    }

    /* Text */
    if (text) {
        draw_text_simple(text, cx + sz + 6.0f, cy,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Radio Button ---- */

void ji_qt_draw_radio_button(JiQtStyle* style, JiDrawingContext* ctx,
                                JiRect rect, const char* text,
                                JiWidgetState state, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    int sz = ji_qt_style_pixel_metric(JI_PM_RADIOBUTTON_SIZE);
    float cx = x, cy = y + ((float)rect.height - sz) / 2.0f;
    float center_x = cx + sz / 2.0f;
    float center_y = cy + sz / 2.0f;
    float radius = sz / 2.0f;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BASE, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);
    uint32_t dot_col = ji_qt_style_resolve_color(style, JI_ROLE_TEXT, state);

    /* Radio circle background */
    glColor4f(cf(bg,0), cf(bg,1), cf(bg,2), 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 32; i++) {
        float a = (float)i * 2.0f * 3.14159f / 32.0f;
        glVertex2f(center_x + cosf(a) * radius, center_y + sinf(a) * radius);
    }
    glEnd();

    /* Radio circle border */
    glColor4f(cf(border,0), cf(border,1), cf(border,2), 0.8f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 32; i++) {
        float a = (float)i * 2.0f * 3.14159f / 32.0f;
        glVertex2f(center_x + cosf(a) * radius, center_y + sinf(a) * radius);
    }
    glEnd();

    /* Selected dot */
    if (state & JI_STATE_CHECKED) {
        float dot_r = radius * 0.4f;
        glColor4f(cf(dot_col,0), cf(dot_col,1), cf(dot_col,2), 1.0f);
        glBegin(GL_TRIANGLE_FAN);
        for (int i = 0; i <= 24; i++) {
            float a = (float)i * 2.0f * 3.14159f / 24.0f;
            glVertex2f(center_x + cosf(a) * dot_r, center_y + sinf(a) * dot_r);
        }
        glEnd();
    }

    /* Text */
    if (text) {
        draw_text_simple(text, cx + sz + 6.0f, cy,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Slider ---- */

void ji_qt_draw_slider(JiQtStyle* style, JiDrawingContext* ctx,
                         JiRect rect, double value, double min, double max,
                         bool horizontal, JiWidgetState state,
                         int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t track_bg = ji_qt_style_resolve_color(style, JI_ROLE_MID, state);
    uint32_t track_fill = ji_qt_style_resolve_color(style, JI_ROLE_HIGHLIGHT, state);
    uint32_t thumb_col = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);

    double pct = (max > min) ? (value - min) / (max - min) : 0.0;
    if (pct < 0.0) pct = 0.0;
    if (pct > 1.0) pct = 1.0;

    if (horizontal) {
        float track_h = 4.0f;
        float track_y = y + (h - track_h) / 2.0f;
        /* Track background */
        draw_filled_rect(x, track_y, w, track_h,
                         cf(track_bg,0), cf(track_bg,1), cf(track_bg,2), 1.0f, sw, sh);
        /* Fill */
        float fill_w = (float)pct * w;
        if (fill_w > 0)
            draw_filled_rect(x, track_y, fill_w, track_h,
                             cf(track_fill,0), cf(track_fill,1), cf(track_fill,2), 1.0f, sw, sh);
        /* Thumb (diamond shape) */
        float thumb_sz = 11.0f;
        float tcx = x + fill_w;
        float tcy = y + h / 2.0f;
        glColor4f(cf(thumb_col,0), cf(thumb_col,1), cf(thumb_col,2), 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(tcx, tcy - thumb_sz/2);
        glVertex2f(tcx + thumb_sz/2, tcy);
        glVertex2f(tcx, tcy + thumb_sz/2);
        glVertex2f(tcx - thumb_sz/2, tcy);
        glEnd();
        /* Thumb border */
        glColor4f(cf(border,0), cf(border,1), cf(border,2), 0.8f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(tcx, tcy - thumb_sz/2);
        glVertex2f(tcx + thumb_sz/2, tcy);
        glVertex2f(tcx, tcy + thumb_sz/2);
        glVertex2f(tcx - thumb_sz/2, tcy);
        glEnd();
    } else {
        /* Vertical slider */
        float track_w = 4.0f;
        float track_x = x + (w - track_w) / 2.0f;
        draw_filled_rect(track_x, y, track_w, h,
                         cf(track_bg,0), cf(track_bg,1), cf(track_bg,2), 1.0f, sw, sh);
        float fill_h = (float)pct * h;
        if (fill_h > 0)
            draw_filled_rect(track_x, y + h - fill_h, track_w, fill_h,
                             cf(track_fill,0), cf(track_fill,1), cf(track_fill,2), 1.0f, sw, sh);
        /* Thumb */
        float thumb_sz = 11.0f;
        float tcx = x + w / 2.0f;
        float tcy = y + h - fill_h;
        glColor4f(cf(thumb_col,0), cf(thumb_col,1), cf(thumb_col,2), 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(tcx, tcy - thumb_sz/2);
        glVertex2f(tcx + thumb_sz/2, tcy);
        glVertex2f(tcx, tcy + thumb_sz/2);
        glVertex2f(tcx - thumb_sz/2, tcy);
        glEnd();
    }
}

/* ---- Progress Bar ---- */

void ji_qt_draw_progress_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                JiRect rect, double value, double min, double max,
                                bool indeterminate, JiWidgetState state,
                                int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, state);
    uint32_t fill = ji_qt_style_resolve_color(style, JI_ROLE_HIGHLIGHT, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);

    if (indeterminate) {
        /* Animated chunk — caller handles animation position */
        float chunk_w = w * 0.3f;
        draw_filled_rect(x, y, chunk_w, h,
                         cf(fill,0), cf(fill,1), cf(fill,2), 1.0f, sw, sh);
    } else {
        double pct = (max > min) ? (value - min) / (max - min) : 0.0;
        if (pct < 0.0) pct = 0.0;
        if (pct > 1.0) pct = 1.0;
        float fill_w = (float)pct * w;
        if (fill_w > 0)
            draw_filled_rect(x, y, fill_w, h,
                             cf(fill,0), cf(fill,1), cf(fill,2), 1.0f, sw, sh);
    }
}

/* ---- Text Box ---- */

void ji_qt_draw_text_box(JiQtStyle* style, JiDrawingContext* ctx,
                            JiRect rect, const char* text, const char* placeholder,
                            JiWidgetState state, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BASE, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_TEXT, state);
    uint32_t ph_col = ji_qt_style_resolve_color(style, JI_ROLE_PLACEHOLDER_TEXT, state);

    if (state & JI_STATE_FOCUSED)
        border = ji_qt_style_resolve_color(style, JI_ROLE_HIGHLIGHT, state);

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    /* Border */
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.8f, sw, sh);

    /* Text or placeholder */
    const char* display = (text && text[0]) ? text : placeholder;
    uint32_t col = (text && text[0]) ? text_col : ph_col;
    if (display) {
        draw_text_simple(display, x + 4.0f, y + 4.0f,
                         cf(col,0), cf(col,1), cf(col,2), 1.0f, sw, sh);
    }
}

/* ---- Combo Box ---- */

void ji_qt_draw_combo_box(JiQtStyle* style, JiDrawingContext* ctx,
                             JiRect rect, const char* text,
                             JiWidgetState state, bool is_open,
                             int sw, int sh) {
    (void)ctx; (void)is_open;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON_TEXT, state);
    uint32_t arrow_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);

    if (state & JI_STATE_HOVER)
        bg = ji_qt_style_resolve_color(style, JI_ROLE_LIGHT, state);

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    /* Border */
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.8f, sw, sh);

    /* Arrow (down triangle) */
    float ax = x + w - 16.0f;
    float ay = y + h / 2.0f;
    draw_triangle(ax, ay - 3, ax + 8, ay - 3, ax + 4, ay + 3,
                  cf(arrow_col,0), cf(arrow_col,1), cf(arrow_col,2), 0.8f);

    /* Text */
    if (text) {
        draw_text_simple(text, x + 6.0f, y + 4.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Group Box ---- */

void ji_qt_draw_group_box(JiQtStyle* style, JiDrawingContext* ctx,
                             JiRect rect, const char* title,
                             JiWidgetState state, int sw, int sh) {
    (void)ctx; (void)state;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, JI_STATE_NORMAL);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, JI_STATE_NORMAL);

    /* Border (rectangular, no rounding — Qt6 Fusion) */
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);

    /* Title — drawn on top border with background gap */
    if (title) {
        draw_text_simple(title, x + 8.0f, y - 6.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Frame ---- */

void ji_qt_draw_frame(JiQtStyle* style, JiDrawingContext* ctx,
                         JiRect rect, int frame_shape, JiWidgetState state,
                         int sw, int sh) {
    (void)ctx; (void)state; (void)frame_shape;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, JI_STATE_NORMAL);
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);
}

/* ---- Focus Rectangle ---- */

void ji_qt_draw_focus_rect(JiQtStyle* style, JiDrawingContext* ctx,
                              JiRect rect, int sw, int sh) {
    (void)ctx; (void)sw; (void)sh;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t fc = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, JI_STATE_NORMAL);
    draw_dashed_rect(x, y, w, h, cf(fc,0), cf(fc,1), cf(fc,2), 0.5f);
}

/* ---- Scroll Bar ---- */

void ji_qt_draw_scroll_bar(JiQtStyle* style, JiDrawingContext* ctx,
                              JiRect rect, double value, double min, double max,
                              bool horizontal, JiWidgetState state,
                              int sw, int sh) {
    (void)ctx; (void)value; (void)min; (void)max;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, state);
    uint32_t slider = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);

    /* Slider (simplified — full extent for now) */
    if (horizontal) {
        float slider_w = w * 0.3f;
        draw_filled_rect(x + 2, y + 1, slider_w, h - 2,
                         cf(slider,0), cf(slider,1), cf(slider,2), 1.0f, sw, sh);
        draw_rect_outline(x + 2, y + 1, slider_w, h - 2,
                          cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);
    } else {
        float slider_h = h * 0.3f;
        draw_filled_rect(x + 1, y + 2, w - 2, slider_h,
                         cf(slider,0), cf(slider,1), cf(slider,2), 1.0f, sw, sh);
        draw_rect_outline(x + 1, y + 2, w - 2, slider_h,
                          cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);
    }
}

/* ---- Tab ---- */

void ji_qt_draw_tab(JiQtStyle* style, JiDrawingContext* ctx,
                       JiRect rect, const char* text,
                       JiWidgetState state, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, state);
    uint32_t selected_bg = ji_qt_style_resolve_color(style, JI_ROLE_BASE, state);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);

    uint32_t use_bg = (state & JI_STATE_SELECTED) ? selected_bg : bg;

    /* Tab background */
    draw_filled_rect(x, y, w, h, cf(use_bg,0), cf(use_bg,1), cf(use_bg,2), 1.0f, sw, sh);

    /* Borders (top, left, right — no bottom when selected) */
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.6f, sw, sh);

    /* Text */
    if (text) {
        draw_text_simple(text, x + 8.0f, y + 4.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Dock Title Bar ---- */

void ji_qt_draw_dock_title(JiQtStyle* style, JiDrawingContext* ctx,
                              JiRect rect, const char* title,
                              JiWidgetState state, bool is_floating,
                              int sw, int sh) {
    (void)ctx; (void)is_floating;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_BUTTON, state);
    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);

    /* Background */
    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);

    /* Title text */
    if (title) {
        draw_text_simple(title, x + 6.0f, y + 3.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }
}

/* ---- Toolbar ---- */

void ji_qt_draw_tool_bar(JiQtStyle* style, JiDrawingContext* ctx,
                            JiRect rect, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, JI_STATE_NORMAL);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, JI_STATE_NORMAL);

    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    draw_rect_outline(x, y, w, h, cf(border,0), cf(border,1), cf(border,2), 0.4f, sw, sh);
}

/* ---- Status Bar ---- */

void ji_qt_draw_status_bar(JiQtStyle* style, JiDrawingContext* ctx,
                              JiRect rect, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, JI_STATE_NORMAL);
    uint32_t border = ji_qt_style_resolve_color(style, JI_ROLE_MID, JI_STATE_NORMAL);

    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
    /* Top border only */
    glColor4f(cf(border,0), cf(border,1), cf(border,2), 0.4f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glEnd();
}

/* ---- Menu Bar ---- */

void ji_qt_draw_menu_bar(JiQtStyle* style, JiDrawingContext* ctx,
                            JiRect rect, int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, JI_STATE_NORMAL);

    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);
}

/* ---- Menu Item ---- */

void ji_qt_draw_menu_item(JiQtStyle* style, JiDrawingContext* ctx,
                             JiRect rect, const char* text,
                             JiWidgetState state, bool has_submenu,
                             int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t text_col = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW_TEXT, state);

    if (state & JI_STATE_SELECTED) {
        uint32_t hl = ji_qt_style_resolve_color(style, JI_ROLE_HIGHLIGHT, state);
        uint32_t hlt = ji_qt_style_resolve_color(style, JI_ROLE_HIGHLIGHTED_TEXT, state);
        draw_filled_rect(x, y, w, h, cf(hl,0), cf(hl,1), cf(hl,2), 1.0f, sw, sh);
        text_col = hlt;
    }

    if (text) {
        draw_text_simple(text, x + 20.0f, y + 4.0f,
                         cf(text_col,0), cf(text_col,1), cf(text_col,2), 1.0f, sw, sh);
    }

    /* Submenu arrow */
    if (has_submenu) {
        float ax = x + w - 12.0f;
        float ay = y + h / 2.0f;
        draw_triangle(ax, ay - 3, ax + 6, ay, ax, ay + 3,
                      cf(text_col,0), cf(text_col,1), cf(text_col,2), 0.8f);
    }
}

/* ---- Splitter ---- */

void ji_qt_draw_splitter(JiQtStyle* style, JiDrawingContext* ctx,
                            JiRect rect, bool horizontal, JiWidgetState state,
                            int sw, int sh) {
    (void)ctx;
    float x = (float)rect.x, y = (float)rect.y;
    float w = (float)rect.width, h = (float)rect.height;

    uint32_t bg = ji_qt_style_resolve_color(style, JI_ROLE_MID, state);
    uint32_t handle = ji_qt_style_resolve_color(style, JI_ROLE_DARK, state);

    draw_filled_rect(x, y, w, h, cf(bg,0), cf(bg,1), cf(bg,2), 1.0f, sw, sh);

    /* Handle grip lines */
    if (horizontal) {
        float cx = x + w / 2.0f;
        float cy = y + h / 2.0f;
        for (int i = -1; i <= 1; i++) {
            glColor4f(cf(handle,0), cf(handle,1), cf(handle,2), 0.5f);
            glBegin(GL_LINES);
            glVertex2f(cx - 3, cy + i * 4);
            glVertex2f(cx + 3, cy + i * 4);
            glEnd();
        }
    } else {
        float cx = x + w / 2.0f;
        float cy = y + h / 2.0f;
        for (int i = -1; i <= 1; i++) {
            glColor4f(cf(handle,0), cf(handle,1), cf(handle,2), 0.5f);
            glBegin(GL_LINES);
            glVertex2f(cx + i * 4, cy - 3);
            glVertex2f(cx + i * 4, cy + 3);
            glEnd();
        }
    }
}
