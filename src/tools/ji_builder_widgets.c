#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_builder_widgets.c
 * @brief Builder widget palette — type names and palette listing.
 */

#include "jiui/ji_builder.h"
#include <string.h>

/* =========================================================================
 * Widget Type Names
 * ========================================================================= */

static const char* g_type_names[] = {
    "Window",       /* 0 */
    "Button",       /* 1 */
    "Label",        /* 2 */
    "TextEdit",     /* 3 */
    "CheckBox",     /* 4 */
    "RadioButton",  /* 5 */
    "ComboBox",     /* 6 */
    "ListView",     /* 7 */
    "TreeView",     /* 8 */
    "TableView",    /* 9 */
    "TabWidget",    /* 10 */
    "ScrollArea",   /* 11 */
    "Splitter",     /* 12 */
    "ToolBar",      /* 13 */
    "MenuBar",      /* 14 */
    "StatusBar",    /* 15 */
    "Frame",        /* 16 */
    "Dial",         /* 17 */
    "Slider",       /* 18 */
    "SpinBox",      /* 19 */
    "Canvas",       /* 20 */
    "Viewport3D",   /* 21 */
    "Custom"        /* 99 */
};

static const JiBuilderWidgetType g_palette[] = {
    JI_BW_BUTTON,
    JI_BW_LABEL,
    JI_BW_TEXT_EDIT,
    JI_BW_CHECKBOX,
    JI_BW_RADIO,
    JI_BW_COMBO,
    JI_BW_LIST,
    JI_BW_TREE,
    JI_BW_TABLE,
    JI_BW_TAB_WIDGET,
    JI_BW_SCROLL_AREA,
    JI_BW_SPLITTER,
    JI_BW_TOOLBAR,
    JI_BW_MENUBAR,
    JI_BW_STATUSBAR,
    JI_BW_FRAME,
    JI_BW_DIAL,
    JI_BW_SLIDER,
    JI_BW_SPINBOX,
    JI_BW_CANVAS,
    JI_BW_VIEWPORT_3D
};

static const int g_palette_count = (int)(sizeof(g_palette) / sizeof(g_palette[0]));

const char* ji_builder_widget_type_name(JiBuilderWidgetType type)
{
    if (type == JI_BW_CUSTOM) return g_type_names[22];
    if (type < 0 || type > JI_BW_VIEWPORT_3D) return "Unknown";
    return g_type_names[type];
}

int ji_builder_palette_count(void)
{
    return g_palette_count;
}

JiBuilderWidgetType ji_builder_palette_at(int index)
{
    if (index < 0 || index >= g_palette_count) return JI_BW_BUTTON;
    return g_palette[index];
}
