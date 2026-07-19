/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_builder.h
 * @brief Visual UI Builder — drag-and-drop widget placement, live editing,
 *        code generation (C/C++ and JiML), property inspector, undo/redo.
 */

#ifndef JIUI_BUILDER_H
#define JIUI_BUILDER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Builder Constants
 * ========================================================================= */

#define JI_BUILDER_MAX_WIDGETS   512
#define JI_BUILDER_MAX_PROPS     32
#define JI_BUILDER_NAME_MAX      64
#define JI_BUILDER_PROP_KEY_MAX  32
#define JI_BUILDER_PROP_VAL_MAX  128

/* =========================================================================
 * Builder Widget Types
 * ========================================================================= */

typedef enum JiBuilderWidgetType {
    JI_BW_WINDOW      = 0,
    JI_BW_BUTTON      = 1,
    JI_BW_LABEL       = 2,
    JI_BW_TEXT_EDIT   = 3,
    JI_BW_CHECKBOX    = 4,
    JI_BW_RADIO       = 5,
    JI_BW_COMBO       = 6,
    JI_BW_LIST        = 7,
    JI_BW_TREE        = 8,
    JI_BW_TABLE       = 9,
    JI_BW_TAB_WIDGET  = 10,
    JI_BW_SCROLL_AREA = 11,
    JI_BW_SPLITTER    = 12,
    JI_BW_TOOLBAR     = 13,
    JI_BW_MENUBAR     = 14,
    JI_BW_STATUSBAR   = 15,
    JI_BW_FRAME       = 16,
    JI_BW_DIAL        = 17,
    JI_BW_SLIDER      = 18,
    JI_BW_SPINBOX     = 19,
    JI_BW_CANVAS      = 20,
    JI_BW_VIEWPORT_3D = 21,
    JI_BW_CUSTOM      = 99
} JiBuilderWidgetType;

/** A property key-value pair. */
typedef struct JiBuilderProperty {
    char key[JI_BUILDER_PROP_KEY_MAX];
    char value[JI_BUILDER_PROP_VAL_MAX];
} JiBuilderProperty;

/** A widget in the builder design. */
typedef struct JiBuilderWidget {
    int id;
    JiBuilderWidgetType type;
    char name[JI_BUILDER_NAME_MAX];
    int  parent_id;       /* -1 = root */
    JiRect bounds;
    JiBuilderProperty properties[JI_BUILDER_MAX_PROPS];
    int  prop_count;
    bool selected;
    bool locked;
} JiBuilderWidget;

/** The builder design (collection of widgets). */
typedef struct JiBuilder {
    JiBuilderWidget widgets[JI_BUILDER_MAX_WIDGETS];
    int  widget_count;
    int  next_id;
    int  selected_id;     /* Currently selected widget (-1 = none) */

    /* Undo/redo stacks */
    JiBuilderWidget undo_stack[64];
    int  undo_count;
    int  undo_index;
    JiBuilderWidget redo_stack[64];
    int  redo_count;

    /* Grid/snap settings */
    bool snap_to_grid;
    int  grid_size;

    /* Canvas dimensions */
    int  canvas_width;
    int  canvas_height;

    /* Modified flag */
    bool modified;
} JiBuilder;

/* =========================================================================
 * Builder Lifecycle
 * ========================================================================= */

JI_API JiBuilder* ji_builder_new(void);
JI_API void       ji_builder_free(JiBuilder* builder);
JI_API void       ji_builder_reset(JiBuilder* builder);

/* =========================================================================
 * Widget Management
 * ========================================================================= */

JI_API int  ji_builder_add_widget(JiBuilder* builder, JiBuilderWidgetType type,
                                   int parent_id, JiRect bounds);
JI_API bool ji_builder_remove_widget(JiBuilder* builder, int widget_id);
JI_API bool ji_builder_move_widget(JiBuilder* builder, int widget_id, JiRect bounds);
JI_API bool ji_builder_rename_widget(JiBuilder* builder, int widget_id, const char* name);
JI_API bool ji_builder_reparent_widget(JiBuilder* builder, int widget_id, int new_parent);

JI_API JiBuilderWidget* ji_builder_get_widget(JiBuilder* builder, int widget_id);
JI_API JiBuilderWidget* ji_builder_get_selected(JiBuilder* builder);

/* =========================================================================
 * Selection
 * ========================================================================= */

JI_API void ji_builder_select(JiBuilder* builder, int widget_id);
JI_API void ji_builder_select_none(JiBuilder* builder);
JI_API int  ji_builder_get_selected_id(JiBuilder* builder);

/* =========================================================================
 * Properties
 * ========================================================================= */

JI_API bool ji_builder_set_property(JiBuilder* builder, int widget_id,
                                      const char* key, const char* value);
JI_API const char* ji_builder_get_property(JiBuilder* builder, int widget_id,
                                             const char* key);
JI_API bool ji_builder_remove_property(JiBuilder* builder, int widget_id,
                                         const char* key);

/* =========================================================================
 * Undo / Redo
 * ========================================================================= */

JI_API void ji_builder_push_undo(JiBuilder* builder);
JI_API bool ji_builder_undo(JiBuilder* builder);
JI_API bool ji_builder_redo(JiBuilder* builder);
JI_API bool ji_builder_can_undo(JiBuilder* builder);
JI_API bool ji_builder_can_redo(JiBuilder* builder);

/* =========================================================================
 * Grid / Snap
 * ========================================================================= */

JI_API void ji_builder_set_grid(JiBuilder* builder, int size, bool snap);
JI_API JiRect ji_builder_snap_rect(JiBuilder* builder, JiRect rect);

/* =========================================================================
 * Code Generation
 * ========================================================================= */

/** Generate C code from the builder design. */
JI_API int ji_builder_gen_c(const JiBuilder* builder, char* buf, int buf_size);

/** Generate C++ code from the builder design. */
JI_API int ji_builder_gen_cpp(const JiBuilder* builder, char* buf, int buf_size);

/** Generate JiML markup from the builder design. */
JI_API int ji_builder_gen_jiml(const JiBuilder* builder, char* buf, int buf_size);

/* =========================================================================
 * Widget Palette
 * ========================================================================= */

/** Get the widget type name string. */
JI_API const char* ji_builder_widget_type_name(JiBuilderWidgetType type);

/** Get the number of available widget types in the palette. */
JI_API int ji_builder_palette_count(void);

/** Get widget type at palette index. */
JI_API JiBuilderWidgetType ji_builder_palette_at(int index);

/* =========================================================================
 * Serialization
 * ========================================================================= */

JI_API int  ji_builder_save(const JiBuilder* builder, char* buf, int buf_size);
JI_API bool ji_builder_load(JiBuilder* builder, const char* data);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_BUILDER_H */
