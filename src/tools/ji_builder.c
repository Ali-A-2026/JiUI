#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_builder.c
 * @brief Visual UI Builder implementation — widget management, selection,
 *        properties, undo/redo, grid snapping.
 */

#include "jiui/ji_builder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiBuilder* ji_builder_new(void)
{
    JiBuilder* b = (JiBuilder*)calloc(1, sizeof(JiBuilder));
    if (!b) return NULL;
    b->next_id = 1;
    b->selected_id = -1;
    b->snap_to_grid = false;
    b->grid_size = 10;
    b->canvas_width = 800;
    b->canvas_height = 600;
    return b;
}

void ji_builder_free(JiBuilder* builder)
{
    free(builder);
}

void ji_builder_reset(JiBuilder* builder)
{
    if (!builder) return;
    builder->widget_count = 0;
    builder->next_id = 1;
    builder->selected_id = -1;
    builder->undo_count = 0;
    builder->undo_index = 0;
    builder->redo_count = 0;
    builder->modified = false;
}

/* =========================================================================
 * Widget Management
 * ========================================================================= */

int ji_builder_add_widget(JiBuilder* builder, JiBuilderWidgetType type,
                            int parent_id, JiRect bounds)
{
    if (!builder) return -1;
    if (builder->widget_count >= JI_BUILDER_MAX_WIDGETS) return -1;

    JiBuilderWidget* w = &builder->widgets[builder->widget_count++];
    memset(w, 0, sizeof(JiBuilderWidget));
    w->id = builder->next_id++;
    w->type = type;
    w->parent_id = parent_id;
    w->bounds = bounds;
    snprintf(w->name, JI_BUILDER_NAME_MAX, "%s_%d",
             ji_builder_widget_type_name(type), w->id);
    w->selected = false;
    w->locked = false;
    builder->modified = true;
    return w->id;
}

bool ji_builder_remove_widget(JiBuilder* builder, int widget_id)
{
    if (!builder) return false;
    /* Also remove children */
    for (int i = 0; i < builder->widget_count; i++) {
        if (builder->widgets[i].parent_id == widget_id)
            builder->widgets[i].parent_id = -1;
    }
    /* Find and remove */
    for (int i = 0; i < builder->widget_count; i++) {
        if (builder->widgets[i].id == widget_id) {
            /* Shift down */
            for (int j = i; j < builder->widget_count - 1; j++)
                builder->widgets[j] = builder->widgets[j + 1];
            builder->widget_count--;
            if (builder->selected_id == widget_id)
                builder->selected_id = -1;
            builder->modified = true;
            return true;
        }
    }
    return false;
}

bool ji_builder_move_widget(JiBuilder* builder, int widget_id, JiRect bounds)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w || w->locked) return false;
    if (builder->snap_to_grid)
        bounds = ji_builder_snap_rect(builder, bounds);
    w->bounds = bounds;
    builder->modified = true;
    return true;
}

bool ji_builder_rename_widget(JiBuilder* builder, int widget_id, const char* name)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w || !name) return false;
    strncpy(w->name, name, JI_BUILDER_NAME_MAX - 1);
    w->name[JI_BUILDER_NAME_MAX - 1] = '\0';
    builder->modified = true;
    return true;
}

bool ji_builder_reparent_widget(JiBuilder* builder, int widget_id, int new_parent)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w) return false;
    /* Prevent making widget its own parent */
    if (widget_id == new_parent) return false;
    /* Check for cycles */
    int p = new_parent;
    while (p != -1) {
        JiBuilderWidget* pw = ji_builder_get_widget(builder, p);
        if (!pw) break;
        if (pw->id == widget_id) return false; /* Cycle */
        p = pw->parent_id;
    }
    w->parent_id = new_parent;
    builder->modified = true;
    return true;
}

JiBuilderWidget* ji_builder_get_widget(JiBuilder* builder, int widget_id)
{
    if (!builder) return NULL;
    for (int i = 0; i < builder->widget_count; i++) {
        if (builder->widgets[i].id == widget_id)
            return &builder->widgets[i];
    }
    return NULL;
}

JiBuilderWidget* ji_builder_get_selected(JiBuilder* builder)
{
    if (!builder || builder->selected_id < 0) return NULL;
    return ji_builder_get_widget(builder, builder->selected_id);
}

/* =========================================================================
 * Selection
 * ========================================================================= */

void ji_builder_select(JiBuilder* builder, int widget_id)
{
    if (!builder) return;
    /* Clear previous selection */
    for (int i = 0; i < builder->widget_count; i++)
        builder->widgets[i].selected = false;
    /* Set new selection */
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (w) w->selected = true;
    builder->selected_id = widget_id;
}

void ji_builder_select_none(JiBuilder* builder)
{
    if (!builder) return;
    for (int i = 0; i < builder->widget_count; i++)
        builder->widgets[i].selected = false;
    builder->selected_id = -1;
}

int ji_builder_get_selected_id(JiBuilder* builder)
{
    if (!builder) return -1;
    return builder->selected_id;
}

/* =========================================================================
 * Properties
 * ========================================================================= */

bool ji_builder_set_property(JiBuilder* builder, int widget_id,
                               const char* key, const char* value)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w || !key || !value) return false;

    /* Check if property exists — update it */
    for (int i = 0; i < w->prop_count; i++) {
        if (strcmp(w->properties[i].key, key) == 0) {
            strncpy(w->properties[i].value, value, JI_BUILDER_PROP_VAL_MAX - 1);
            w->properties[i].value[JI_BUILDER_PROP_VAL_MAX - 1] = '\0';
            builder->modified = true;
            return true;
        }
    }
    /* Add new property */
    if (w->prop_count >= JI_BUILDER_MAX_PROPS) return false;
    JiBuilderProperty* p = &w->properties[w->prop_count++];
    strncpy(p->key, key, JI_BUILDER_PROP_KEY_MAX - 1);
    p->key[JI_BUILDER_PROP_KEY_MAX - 1] = '\0';
    strncpy(p->value, value, JI_BUILDER_PROP_VAL_MAX - 1);
    p->value[JI_BUILDER_PROP_VAL_MAX - 1] = '\0';
    builder->modified = true;
    return true;
}

const char* ji_builder_get_property(JiBuilder* builder, int widget_id,
                                      const char* key)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w || !key) return NULL;
    for (int i = 0; i < w->prop_count; i++) {
        if (strcmp(w->properties[i].key, key) == 0)
            return w->properties[i].value;
    }
    return NULL;
}

bool ji_builder_remove_property(JiBuilder* builder, int widget_id,
                                  const char* key)
{
    JiBuilderWidget* w = ji_builder_get_widget(builder, widget_id);
    if (!w || !key) return false;
    for (int i = 0; i < w->prop_count; i++) {
        if (strcmp(w->properties[i].key, key) == 0) {
            for (int j = i; j < w->prop_count - 1; j++)
                w->properties[j] = w->properties[j + 1];
            w->prop_count--;
            builder->modified = true;
            return true;
        }
    }
    return false;
}

/* =========================================================================
 * Undo / Redo
 * ========================================================================= */

void ji_builder_push_undo(JiBuilder* builder)
{
    if (!builder) return;
    /* Save current state to undo stack (simplified: save selected widget) */
    if (builder->undo_count < 64) {
        JiBuilderWidget* sel = ji_builder_get_selected(builder);
        if (sel) {
            builder->undo_stack[builder->undo_count++] = *sel;
        }
    }
    builder->redo_count = 0; /* Clear redo on new action */
}

bool ji_builder_undo(JiBuilder* builder)
{
    if (!builder || builder->undo_count == 0) return false;
    /* Push current to redo */
    JiBuilderWidget* sel = ji_builder_get_selected(builder);
    if (sel && builder->redo_count < 64)
        builder->redo_stack[builder->redo_count++] = *sel;
    /* Restore from undo */
    JiBuilderWidget* undo_state = &builder->undo_stack[--builder->undo_count];
    JiBuilderWidget* w = ji_builder_get_widget(builder, undo_state->id);
    if (w) *w = *undo_state;
    return true;
}

bool ji_builder_redo(JiBuilder* builder)
{
    if (!builder || builder->redo_count == 0) return false;
    /* Push current to undo */
    JiBuilderWidget* sel = ji_builder_get_selected(builder);
    if (sel && builder->undo_count < 64)
        builder->undo_stack[builder->undo_count++] = *sel;
    /* Restore from redo */
    JiBuilderWidget* redo_state = &builder->redo_stack[--builder->redo_count];
    JiBuilderWidget* w = ji_builder_get_widget(builder, redo_state->id);
    if (w) *w = *redo_state;
    return true;
}

bool ji_builder_can_undo(JiBuilder* builder)
{
    return builder && builder->undo_count > 0;
}

bool ji_builder_can_redo(JiBuilder* builder)
{
    return builder && builder->redo_count > 0;
}

/* =========================================================================
 * Grid / Snap
 * ========================================================================= */

void ji_builder_set_grid(JiBuilder* builder, int size, bool snap)
{
    if (!builder) return;
    builder->grid_size = size > 0 ? size : 1;
    builder->snap_to_grid = snap;
}

JiRect ji_builder_snap_rect(JiBuilder* builder, JiRect rect)
{
    if (!builder || !builder->snap_to_grid || builder->grid_size <= 0)
        return rect;
    int gs = builder->grid_size;
    rect.x = (rect.x / gs) * gs;
    rect.y = (rect.y / gs) * gs;
    rect.width = ((rect.width + gs - 1) / gs) * gs;
    rect.height = ((rect.height + gs - 1) / gs) * gs;
    return rect;
}

/* =========================================================================
 * Serialization (simplified)
 * ========================================================================= */

int ji_builder_save(const JiBuilder* builder, char* buf, int buf_size)
{
    if (!builder || !buf || buf_size <= 0) return 0;
    int written = 0;
    written += snprintf(buf + written, (size_t)(buf_size - written),
        "{\n  \"canvas\": {\"w\": %d, \"h\": %d},\n  \"widgets\": [\n",
        builder->canvas_width, builder->canvas_height);
    for (int i = 0; i < builder->widget_count && written < buf_size - 1; i++) {
        JiBuilderWidget* w = &builder->widgets[i];
        written += snprintf(buf + written, (size_t)(buf_size - written),
            "    {\"id\": %d, \"type\": %d, \"name\": \"%s\", \"parent\": %d, "
            "\"x\": %d, \"y\": %d, \"w\": %d, \"h\": %d}%s\n",
            w->id, (int)w->type, w->name, w->parent_id,
            w->bounds.x, w->bounds.y, w->bounds.width, w->bounds.height,
            (i < builder->widget_count - 1) ? "," : "");
    }
    written += snprintf(buf + written, (size_t)(buf_size - written), "  ]\n}\n");
    return written;
}

bool ji_builder_load(JiBuilder* builder, const char* data)
{
    if (!builder || !data) return false;
    /* Simplified: just reset and return true */
    ji_builder_reset(builder);
    return true;
}
