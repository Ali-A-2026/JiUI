/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_qt_style.h
 * @brief Qt6 Fusion-style renderer — draws widgets using palette color roles
 *        with pixel-perfect metrics matching Qt6's Fusion theme.
 */

#ifndef JIUI_QT_STYLE_H
#define JIUI_QT_STYLE_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_palette.h"
#include "ji_drawing.h"
#include "ji_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Qt6 Pixel Metrics — matches QStyle::PixelMetric
 * ========================================================================= */
typedef enum JiPixelMetric {
    JI_PM_BUTTON_MARGIN          = 0,   /* 6px horizontal, 4px vertical */
    JI_PM_BUTTON_MARGIN_TOP       = 1,   /* 4px */
    JI_PM_BUTTON_MARGIN_BOTTOM    = 2,   /* 4px */
    JI_PM_BUTTON_MARGIN_LEFT      = 3,   /* 6px */
    JI_PM_BUTTON_MARGIN_RIGHT     = 4,   /* 6px */
    JI_PM_CHECKBOX_SIZE          = 5,   /* 13x13px */
    JI_PM_RADIOBUTTON_SIZE       = 6,   /* 13x13px */
    JI_PM_SLIDER_THICKNESS       = 7,   /* 16px */
    JI_PM_SLIDER_LENGTH          = 8,   /* 11px */
    JI_PM_SLIDER_CONTROL         = 9,   /* 11px thumb */
    JI_PM_PROGRESSBAR_THICKNESS   = 10,  /* 4px */
    JI_PM_SCROLLBAR_EXTENT        = 11,  /* 14px */
    JI_PM_SCROLLBAR_SLIDER       = 12,  /* 12px */
    JI_PM_COMBOBOX_MARGIN         = 13,  /* 2px */
    JI_PM_TEXTBOX_MARGIN          = 14,  /* 2px */
    JI_PM_FOCUS_RECT_WIDTH        = 15,  /* 1px dashed */
    JI_PM_FOCUS_RECT_MARGIN       = 16,  /* 2px inset */
    JI_PM_FRAME_BORDER_WIDTH      = 17,  /* 1px */
    JI_PM_TOOLBAR_MARGIN          = 18,  /* 2px */
    JI_PM_TOOLBAR_SEPARATOR       = 19,  /* 8px */
    JI_PM_TAB_BAR_HEIGHT          = 20,  /* 24px */
    JI_PM_MENU_BAR_HEIGHT         = 21,  /* 24px */
    JI_PM_MENU_ITEM_HEIGHT        = 22,  /* 24px */
    JI_PM_STATUS_BAR_HEIGHT       = 23,  /* 22px */
    JI_PM_GROUP_BOX_BORDER         = 24,  /* 1px */
    JI_PM_DOCK_WIDGET_TITLE_HEIGHT= 25,  /* 20px */
    JI_PM_SPLITTER_WIDTH          = 26,  /* 4px */
} JiPixelMetric;

/* =========================================================================
 * Widget states for rendering
 * ========================================================================= */
typedef enum JiWidgetState {
    JI_STATE_NORMAL    = 0,
    JI_STATE_HOVER     = 1,
    JI_STATE_PRESSED   = 2,
    JI_STATE_FOCUSED   = 4,
    JI_STATE_DISABLED  = 8,
    JI_STATE_CHECKED   = 16,
    JI_STATE_SELECTED  = 32
} JiWidgetState;

/* =========================================================================
 * JiQtStyle — Qt6 Fusion style renderer
 * ========================================================================= */
typedef struct JiQtStyle {
    const JiPalette* palette;
} JiQtStyle;

/** Create a new Qt6 Fusion style renderer. */
JI_API JiQtStyle* ji_qt_style_new(const JiPalette* palette);

/** Destroy a Qt6 style renderer. */
JI_API void ji_qt_style_destroy(JiQtStyle* style);

/** Get a pixel metric value. */
JI_API int ji_qt_style_pixel_metric(JiPixelMetric metric);

/* ---- Widget rendering functions ---- */

/** Draw a push button. */
JI_API void ji_qt_draw_button(JiQtStyle* style, JiDrawingContext* ctx,
                                JiRect rect, const char* text,
                                JiWidgetState state, int screen_w, int screen_h);

/** Draw a checkbox. */
JI_API void ji_qt_draw_checkbox(JiQtStyle* style, JiDrawingContext* ctx,
                                  JiRect rect, const char* text,
                                  JiWidgetState state, int screen_w, int screen_h);

/** Draw a radio button. */
JI_API void ji_qt_draw_radio_button(JiQtStyle* style, JiDrawingContext* ctx,
                                       JiRect rect, const char* text,
                                       JiWidgetState state, int screen_w, int screen_h);

/** Draw a slider track and thumb. */
JI_API void ji_qt_draw_slider(JiQtStyle* style, JiDrawingContext* ctx,
                                JiRect rect, double value, double min, double max,
                                bool horizontal, JiWidgetState state,
                                int screen_w, int screen_h);

/** Draw a progress bar. */
JI_API void ji_qt_draw_progress_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                       JiRect rect, double value, double min, double max,
                                       bool indeterminate, JiWidgetState state,
                                       int screen_w, int screen_h);

/** Draw a text box / line edit. */
JI_API void ji_qt_draw_text_box(JiQtStyle* style, JiDrawingContext* ctx,
                                   JiRect rect, const char* text, const char* placeholder,
                                   JiWidgetState state, int screen_w, int screen_h);

/** Draw a combo box. */
JI_API void ji_qt_draw_combo_box(JiQtStyle* style, JiDrawingContext* ctx,
                                    JiRect rect, const char* text,
                                    JiWidgetState state, bool is_open,
                                    int screen_w, int screen_h);

/** Draw a group box with title. */
JI_API void ji_qt_draw_group_box(JiQtStyle* style, JiDrawingContext* ctx,
                                    JiRect rect, const char* title,
                                    JiWidgetState state, int screen_w, int screen_h);

/** Draw a frame border. */
JI_API void ji_qt_draw_frame(JiQtStyle* style, JiDrawingContext* ctx,
                                JiRect rect, int frame_shape, JiWidgetState state,
                                int screen_w, int screen_h);

/** Draw a focus rectangle (dashed border). */
JI_API void ji_qt_draw_focus_rect(JiQtStyle* style, JiDrawingContext* ctx,
                                     JiRect rect, int screen_w, int screen_h);

/** Draw a scrollbar. */
JI_API void ji_qt_draw_scroll_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                      JiRect rect, double value, double min, double max,
                                      bool horizontal, JiWidgetState state,
                                      int screen_w, int screen_h);

/** Draw a tab bar tab. */
JI_API void ji_qt_draw_tab(JiQtStyle* style, JiDrawingContext* ctx,
                              JiRect rect, const char* text,
                              JiWidgetState state, int screen_w, int screen_h);

/** Draw a dock widget title bar. */
JI_API void ji_qt_draw_dock_title(JiQtStyle* style, JiDrawingContext* ctx,
                                      JiRect rect, const char* title,
                                      JiWidgetState state, bool is_floating,
                                      int screen_w, int screen_h);

/** Draw a toolbar background. */
JI_API void ji_qt_draw_tool_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                   JiRect rect, int screen_w, int screen_h);

/** Draw a status bar background. */
JI_API void ji_qt_draw_status_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                      JiRect rect, int screen_w, int screen_h);

/** Draw a menu bar background. */
JI_API void ji_qt_draw_menu_bar(JiQtStyle* style, JiDrawingContext* ctx,
                                   JiRect rect, int screen_w, int screen_h);

/** Draw a menu item. */
JI_API void ji_qt_draw_menu_item(JiQtStyle* style, JiDrawingContext* ctx,
                                     JiRect rect, const char* text,
                                     JiWidgetState state, bool has_submenu,
                                     int screen_w, int screen_h);

/** Draw a splitter handle. */
JI_API void ji_qt_draw_splitter(JiQtStyle* style, JiDrawingContext* ctx,
                                   JiRect rect, bool horizontal, JiWidgetState state,
                                   int screen_w, int screen_h);

/* ---- Helper: resolve color from palette based on state ---- */
JI_API uint32_t ji_qt_style_resolve_color(JiQtStyle* style, JiColorRole role,
                                             JiWidgetState state);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_QT_STYLE_H */
