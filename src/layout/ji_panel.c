/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_panel.c
 * @brief Implementation of panel types — StackPanel, Grid, Canvas, WrapPanel, DockPanel.
 */

#include "jiui/ji_panel.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Static type IDs
 * ========================================================================= */
static JiTypeId s_panel_type       = JI_TYPE_INVALID;
static JiTypeId s_stack_panel_type = JI_TYPE_INVALID;
static JiTypeId s_grid_type        = JI_TYPE_INVALID;
static JiTypeId s_canvas_type      = JI_TYPE_INVALID;
static JiTypeId s_wrap_panel_type  = JI_TYPE_INVALID;
static JiTypeId s_dock_panel_type  = JI_TYPE_INVALID;

JiTypeId ji_panel_type_id(void)      { return s_panel_type; }
JiTypeId ji_stack_panel_type_id(void) { return s_stack_panel_type; }
JiTypeId ji_grid_type_id(void)       { return s_grid_type; }
JiTypeId ji_canvas_type_id(void)     { return s_canvas_type; }
JiTypeId ji_wrap_panel_type_id(void) { return s_wrap_panel_type; }
JiTypeId ji_dock_panel_type_id(void) { return s_dock_panel_type; }

/* =========================================================================
 * Attached property IDs for Canvas, Grid, DockPanel
 * ========================================================================= */
static JiPropertyId s_canvas_left_prop   = JI_PROPERTY_INVALID;
static JiPropertyId s_canvas_top_prop    = JI_PROPERTY_INVALID;
static JiPropertyId s_grid_row_prop      = JI_PROPERTY_INVALID;
static JiPropertyId s_grid_col_prop      = JI_PROPERTY_INVALID;
static JiPropertyId s_grid_row_span_prop = JI_PROPERTY_INVALID;
static JiPropertyId s_grid_col_span_prop = JI_PROPERTY_INVALID;
static JiPropertyId s_dock_prop          = JI_PROPERTY_INVALID;

/* =========================================================================
 * Helper: initialize layout fields for a panel-derived struct
 * ========================================================================= */
static void ji_panel_init_layout(JiLayoutElement* layout, JiTypeId type_id) {
    ji_object_init(&layout->base, type_id);
    layout->horizontal_alignment = JI_ALIGN_H_STRETCH;
    layout->vertical_alignment   = JI_ALIGN_V_STRETCH;
    layout->margin  = ji_thickness_zero();
    layout->padding = ji_thickness_zero();
    layout->width   = ji_nan();
    layout->height  = ji_nan();
    layout->min_width  = 0.0;
    layout->max_width  = 1e9;
    layout->min_height = 0.0;
    layout->max_height = 1e9;
    layout->desired_size = ji_size(0, 0);
    layout->layout_slot  = ji_rect(0, 0, 0, 0);
    layout->previous_available_size = ji_size(0, 0);
    layout->layout_flags = JI_LAYOUT_MEASURE_DIRTY | JI_LAYOUT_ARRANGE_DIRTY;
    layout->layout_parent = NULL;
}

/* =========================================================================
 * JiPanel — base panel
 * ========================================================================= */
JiPanel* ji_panel_new(JiTypeId type_id) {
    JiPanel* panel = JI_NEW(JiPanel);
    if (!panel) return NULL;
    memset(panel, 0, sizeof(JiPanel));

    ji_panel_init_layout(&panel->layout, type_id);
    panel->background = 0x00000000; /* transparent */

    return panel;
}

void ji_panel_destroy(JiPanel* panel) {
    if (!panel) return;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_object_destroy(&panel->layout.base);
}

void ji_panel_add_child(JiPanel* panel, JiLayoutElement* child) {
    if (!panel || !child) return;
    if (panel->child_count >= panel->child_capacity) {
        int new_cap = panel->child_capacity == 0 ? 8 : panel->child_capacity * 2;
        panel->children = (JiLayoutElement**)ji_realloc(panel->children,
            (size_t)new_cap * sizeof(JiLayoutElement*));
        panel->child_capacity = new_cap;
    }
    panel->children[panel->child_count++] = child;
    child->layout_parent = &panel->layout;
    ji_layout_element_invalidate_measure(&panel->layout);
}

bool ji_panel_remove_child(JiPanel* panel, JiLayoutElement* child) {
    if (!panel || !child) return false;
    for (int i = 0; i < panel->child_count; i++) {
        if (panel->children[i] == child) {
            panel->children[i] = panel->children[panel->child_count - 1];
            panel->child_count--;
            child->layout_parent = NULL;
            ji_layout_element_invalidate_measure(&panel->layout);
            return true;
        }
    }
    return false;
}

int ji_panel_get_child_count(const JiPanel* panel) {
    return panel ? panel->child_count : 0;
}

JiLayoutElement* ji_panel_get_child(const JiPanel* panel, int index) {
    if (!panel || index < 0 || index >= panel->child_count) return NULL;
    return panel->children[index];
}

/* =========================================================================
 * JiStackPanel
 * ========================================================================= */
static JiSize ji_stack_panel_measure_override(JiLayoutElement* self, JiSize available_size) {
    JiStackPanel* sp = (JiStackPanel*)self;
    JiPanel* panel = &sp->panel;

    double total_w = 0.0, total_h = 0.0;
    double max_w = 0.0, max_h = 0.0;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        JiSize child_available;

        if (sp->orientation == JI_ORIENTATION_VERTICAL) {
            child_available = ji_size(available_size.width, 1e9);
        } else {
            child_available = ji_size(1e9, available_size.height);
        }

        ji_layout_element_measure(child, child_available);
        JiSize desired = ji_layout_element_get_desired_size(child);

        if (sp->orientation == JI_ORIENTATION_VERTICAL) {
            total_h += desired.height;
            if (desired.width > max_w) max_w = desired.width;
        } else {
            total_w += desired.width;
            if (desired.height > max_h) max_h = desired.height;
        }
    }

    if (panel->child_count > 1) {
        if (sp->orientation == JI_ORIENTATION_VERTICAL) {
            total_h += sp->spacing * (panel->child_count - 1);
        } else {
            total_w += sp->spacing * (panel->child_count - 1);
        }
    }

    double w = (sp->orientation == JI_ORIENTATION_VERTICAL) ? max_w : total_w;
    double h = (sp->orientation == JI_ORIENTATION_VERTICAL) ? total_h : max_h;

    return ji_size(w, h);
}

static JiRect ji_stack_panel_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    JiStackPanel* sp = (JiStackPanel*)self;
    JiPanel* panel = &sp->panel;

    double offset = 0.0;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        JiSize desired = ji_layout_element_get_desired_size(child);

        JiRect child_rect;
        if (sp->orientation == JI_ORIENTATION_VERTICAL) {
            child_rect = ji_rect(final_rect.x, final_rect.y + offset,
                                  final_rect.width, desired.height);
        } else {
            child_rect = ji_rect(final_rect.x + offset, final_rect.y,
                                  desired.width, final_rect.height);
        }

        ji_layout_element_arrange(child, child_rect);
        offset += (sp->orientation == JI_ORIENTATION_VERTICAL) ? desired.height : desired.width;
        offset += sp->spacing;
    }

    return final_rect;
}

JiStackPanel* ji_stack_panel_new(void) {
    JiStackPanel* sp = JI_NEW(JiStackPanel);
    if (!sp) return NULL;
    memset(sp, 0, sizeof(JiStackPanel));

    JiLayoutElement* layout = &sp->panel.layout;
    ji_panel_init_layout(layout, s_stack_panel_type);

    sp->orientation = JI_ORIENTATION_VERTICAL;
    sp->spacing = 0.0;

    layout->measure_override = ji_stack_panel_measure_override;
    layout->arrange_override = ji_stack_panel_arrange_override;

    return sp;
}

void ji_stack_panel_destroy(JiStackPanel* sp) {
    if (!sp) return;
    JiPanel* panel = &sp->panel;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_object_destroy(&panel->layout.base);
}

void ji_stack_panel_set_orientation(JiStackPanel* sp, JiOrientation orientation) {
    if (!sp) return;
    sp->orientation = orientation;
    ji_layout_element_invalidate_measure(&sp->panel.layout);
}

void ji_stack_panel_set_spacing(JiStackPanel* sp, double spacing) {
    if (!sp) return;
    sp->spacing = spacing;
    ji_layout_element_invalidate_measure(&sp->panel.layout);
}

/* =========================================================================
 * JiGrid
 * ========================================================================= */
static void ji_grid_resolve_lengths(JiGridRowColDef* defs, int count,
                                     double available, double spacing) {
    double total_fixed = 0.0;
    double total_star  = 0.0;

    for (int i = 0; i < count; i++) {
        switch (defs[i].length.unit_type) {
            case JI_GRID_UNIT_AUTO:
                defs[i].actual = 0.0;
                break;
            case JI_GRID_UNIT_PIXEL:
                defs[i].actual = defs[i].length.value;
                total_fixed += defs[i].length.value;
                break;
            case JI_GRID_UNIT_STAR:
                total_star += defs[i].length.value;
                defs[i].actual = 0.0;
                break;
        }
    }

    if (count > 1) {
        total_fixed += spacing * (count - 1);
    }

    double remaining = available - total_fixed;
    if (remaining < 0.0) remaining = 0.0;

    for (int i = 0; i < count; i++) {
        if (defs[i].length.unit_type == JI_GRID_UNIT_STAR && total_star > 0.0) {
            defs[i].actual = remaining * (defs[i].length.value / total_star);
        }
    }

    double offset = 0.0;
    for (int i = 0; i < count; i++) {
        defs[i].offset = offset;
        offset += defs[i].actual;
        if (i < count - 1) offset += spacing;
    }
}

static JiSize ji_grid_measure_override(JiLayoutElement* self, JiSize available_size) {
    JiGrid* grid = (JiGrid*)self;
    JiPanel* panel = &grid->panel;

    int row_count = grid->row_count > 0 ? grid->row_count : 1;
    int col_count = grid->col_count > 0 ? grid->col_count : 1;

    JiGridRowColDef default_row = { ji_grid_length_auto(), 0.0, 0.0 };
    JiGridRowColDef default_col = { ji_grid_length_auto(), 0.0, 0.0 };

    JiGridRowColDef* rows = grid->rows ? grid->rows : &default_row;
    JiGridRowColDef* cols = grid->cols ? grid->cols : &default_col;

    ji_grid_resolve_lengths(rows, row_count, available_size.height, grid->row_spacing);
    ji_grid_resolve_lengths(cols, col_count, available_size.width, grid->col_spacing);

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];

        int row = ji_object_get_int(&child->base, s_grid_row_prop);
        int col = ji_object_get_int(&child->base, s_grid_col_prop);

        if (row < 0) row = 0;
        if (row >= row_count) row = row_count - 1;
        if (col < 0) col = 0;
        if (col >= col_count) col = col_count - 1;

        JiSize child_available = ji_size(cols[col].actual, rows[row].actual);
        ji_layout_element_measure(child, child_available);

        JiSize desired = ji_layout_element_get_desired_size(child);
        if (rows[row].length.unit_type == JI_GRID_UNIT_AUTO && desired.height > rows[row].actual) {
            rows[row].actual = desired.height;
        }
        if (cols[col].length.unit_type == JI_GRID_UNIT_AUTO && desired.width > cols[col].actual) {
            cols[col].actual = desired.width;
        }
    }

    double total_w = 0.0, total_h = 0.0;
    for (int c = 0; c < col_count; c++) total_w += cols[c].actual;
    for (int r = 0; r < row_count; r++) total_h += rows[r].actual;
    if (col_count > 1) total_w += grid->col_spacing * (col_count - 1);
    if (row_count > 1) total_h += grid->row_spacing * (row_count - 1);

    return ji_size(total_w, total_h);
}

static JiRect ji_grid_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    JiGrid* grid = (JiGrid*)self;
    JiPanel* panel = &grid->panel;

    int row_count = grid->row_count > 0 ? grid->row_count : 1;
    int col_count = grid->col_count > 0 ? grid->col_count : 1;

    JiGridRowColDef default_row = { ji_grid_length_auto(), 0.0, 0.0 };
    JiGridRowColDef default_col = { ji_grid_length_auto(), 0.0, 0.0 };

    JiGridRowColDef* rows = grid->rows ? grid->rows : &default_row;
    JiGridRowColDef* cols = grid->cols ? grid->cols : &default_col;

    ji_grid_resolve_lengths(rows, row_count, final_rect.height, grid->row_spacing);
    ji_grid_resolve_lengths(cols, col_count, final_rect.width, grid->col_spacing);

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];

        int row = ji_object_get_int(&child->base, s_grid_row_prop);
        int col = ji_object_get_int(&child->base, s_grid_col_prop);

        if (row < 0) row = 0;
        if (row >= row_count) row = row_count - 1;
        if (col < 0) col = 0;
        if (col >= col_count) col = col_count - 1;

        JiRect cell_rect = ji_rect(
            final_rect.x + cols[col].offset,
            final_rect.y + rows[row].offset,
            cols[col].actual,
            rows[row].actual
        );

        ji_layout_element_arrange(child, cell_rect);
    }

    return final_rect;
}

JiGrid* ji_grid_new(void) {
    JiGrid* grid = JI_NEW(JiGrid);
    if (!grid) return NULL;
    memset(grid, 0, sizeof(JiGrid));

    JiLayoutElement* layout = &grid->panel.layout;
    ji_panel_init_layout(layout, s_grid_type);

    layout->measure_override = ji_grid_measure_override;
    layout->arrange_override = ji_grid_arrange_override;

    grid->row_spacing = 0.0;
    grid->col_spacing = 0.0;
    grid->show_grid_lines = false;

    return grid;
}

void ji_grid_destroy(JiGrid* grid) {
    if (!grid) return;
    JiPanel* panel = &grid->panel;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_free(grid->rows);
    ji_free(grid->cols);
    ji_object_destroy(&panel->layout.base);
}

void ji_grid_add_row(JiGrid* grid, JiGridLength length) {
    if (!grid) return;
    if (grid->row_count >= grid->row_capacity) {
        int new_cap = grid->row_capacity == 0 ? 4 : grid->row_capacity * 2;
        grid->rows = (JiGridRowColDef*)ji_realloc(grid->rows,
            (size_t)new_cap * sizeof(JiGridRowColDef));
        grid->row_capacity = new_cap;
    }
    grid->rows[grid->row_count].length = length;
    grid->rows[grid->row_count].actual = 0.0;
    grid->rows[grid->row_count].offset = 0.0;
    grid->row_count++;
    ji_layout_element_invalidate_measure(&grid->panel.layout);
}

void ji_grid_add_column(JiGrid* grid, JiGridLength length) {
    if (!grid) return;
    if (grid->col_count >= grid->col_capacity) {
        int new_cap = grid->col_capacity == 0 ? 4 : grid->col_capacity * 2;
        grid->cols = (JiGridRowColDef*)ji_realloc(grid->cols,
            (size_t)new_cap * sizeof(JiGridRowColDef));
        grid->col_capacity = new_cap;
    }
    grid->cols[grid->col_count].length = length;
    grid->cols[grid->col_count].actual = 0.0;
    grid->cols[grid->col_count].offset = 0.0;
    grid->col_count++;
    ji_layout_element_invalidate_measure(&grid->panel.layout);
}

void ji_grid_set_row_spacing(JiGrid* grid, double spacing) {
    if (!grid) return;
    grid->row_spacing = spacing;
    ji_layout_element_invalidate_measure(&grid->panel.layout);
}

void ji_grid_set_col_spacing(JiGrid* grid, double spacing) {
    if (!grid) return;
    grid->col_spacing = spacing;
    ji_layout_element_invalidate_measure(&grid->panel.layout);
}

int ji_grid_get_row_count(const JiGrid* grid) { return grid ? grid->row_count : 0; }
int ji_grid_get_col_count(const JiGrid* grid) { return grid ? grid->col_count : 0; }

/* =========================================================================
 * JiCanvas
 * ========================================================================= */
static JiSize ji_canvas_measure_override(JiLayoutElement* self, JiSize available_size) {
    JiCanvas* canvas = (JiCanvas*)self;
    JiPanel* panel = &canvas->panel;
    (void)available_size;

    double max_w = 0.0, max_h = 0.0;
    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        ji_layout_element_measure(child, ji_size(1e9, 1e9));
        JiSize desired = ji_layout_element_get_desired_size(child);

        double left = ji_canvas_get_left(child);
        double top  = ji_canvas_get_top(child);

        double right  = left + desired.width;
        double bottom = top  + desired.height;
        if (right  > max_w) max_w = right;
        if (bottom > max_h) max_h = bottom;
    }

    return ji_size(max_w, max_h);
}

static JiRect ji_canvas_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    JiCanvas* canvas = (JiCanvas*)self;
    JiPanel* panel = &canvas->panel;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        JiSize desired = ji_layout_element_get_desired_size(child);

        double left = ji_canvas_get_left(child);
        double top  = ji_canvas_get_top(child);

        JiRect child_rect = ji_rect(
            final_rect.x + left,
            final_rect.y + top,
            desired.width,
            desired.height
        );
        ji_layout_element_arrange(child, child_rect);
    }

    return final_rect;
}

JiCanvas* ji_canvas_new(void) {
    JiCanvas* canvas = JI_NEW(JiCanvas);
    if (!canvas) return NULL;
    memset(canvas, 0, sizeof(JiCanvas));

    JiLayoutElement* layout = &canvas->panel.layout;
    ji_panel_init_layout(layout, s_canvas_type);

    layout->measure_override = ji_canvas_measure_override;
    layout->arrange_override = ji_canvas_arrange_override;

    return canvas;
}

void ji_canvas_destroy(JiCanvas* canvas) {
    if (!canvas) return;
    JiPanel* panel = &canvas->panel;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_object_destroy(&panel->layout.base);
}

void ji_canvas_set_left(JiLayoutElement* child, double left) {
    if (!child) return;
    ji_object_set_double(&child->base, s_canvas_left_prop, left);
}

void ji_canvas_set_top(JiLayoutElement* child, double top) {
    if (!child) return;
    ji_object_set_double(&child->base, s_canvas_top_prop, top);
}

double ji_canvas_get_left(const JiLayoutElement* child) {
    if (!child) return 0.0;
    return ji_object_get_double((JiObject*)&child->base, s_canvas_left_prop);
}

double ji_canvas_get_top(const JiLayoutElement* child) {
    if (!child) return 0.0;
    return ji_object_get_double((JiObject*)&child->base, s_canvas_top_prop);
}

/* =========================================================================
 * JiWrapPanel
 * ========================================================================= */
static JiSize ji_wrap_panel_measure_override(JiLayoutElement* self, JiSize available_size) {
    JiWrapPanel* wp = (JiWrapPanel*)self;
    JiPanel* panel = &wp->panel;

    double cur_w = 0.0, cur_h = 0.0;
    double max_line_w = 0.0, total_h = 0.0;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        ji_layout_element_measure(child, available_size);
        JiSize desired = ji_layout_element_get_desired_size(child);

        double item_w = wp->item_width > 0 ? wp->item_width : desired.width;
        double item_h = wp->item_height > 0 ? wp->item_height : desired.height;

        if (wp->orientation == JI_ORIENTATION_HORIZONTAL) {
            if (cur_w + item_w > available_size.width && cur_w > 0) {
                total_h += cur_h;
                if (cur_w > max_line_w) max_line_w = cur_w;
                cur_w = 0.0;
                cur_h = 0.0;
            }
            cur_w += item_w;
            if (item_h > cur_h) cur_h = item_h;
        } else {
            if (cur_h + item_h > available_size.height && cur_h > 0) {
                total_h += cur_h;
                if (cur_h > max_line_w) max_line_w = cur_h;
                cur_w = 0.0;
                cur_h = 0.0;
            }
            cur_h += item_h;
            if (item_w > cur_w) cur_w = item_w;
        }
    }

    total_h += cur_h;
    if (cur_w > max_line_w) max_line_w = cur_w;

    return ji_size(max_line_w, total_h);
}

static JiRect ji_wrap_panel_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    JiWrapPanel* wp = (JiWrapPanel*)self;
    JiPanel* panel = &wp->panel;

    double cur_x = final_rect.x;
    double cur_y = final_rect.y;
    double line_h = 0.0;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        JiSize desired = ji_layout_element_get_desired_size(child);

        double item_w = wp->item_width > 0 ? wp->item_width : desired.width;
        double item_h = wp->item_height > 0 ? wp->item_height : desired.height;

        if (wp->orientation == JI_ORIENTATION_HORIZONTAL) {
            if (cur_x + item_w > final_rect.x + final_rect.width && cur_x > final_rect.x) {
                cur_x = final_rect.x;
                cur_y += line_h;
                line_h = 0.0;
            }
            JiRect child_rect = ji_rect(cur_x, cur_y, item_w, item_h);
            ji_layout_element_arrange(child, child_rect);
            cur_x += item_w;
            if (item_h > line_h) line_h = item_h;
        } else {
            if (cur_y + item_h > final_rect.y + final_rect.height && cur_y > final_rect.y) {
                cur_y = final_rect.y;
                cur_x += line_h;
                line_h = 0.0;
            }
            JiRect child_rect = ji_rect(cur_x, cur_y, item_w, item_h);
            ji_layout_element_arrange(child, child_rect);
            cur_y += item_h;
            if (item_w > line_h) line_h = item_w;
        }
    }

    return final_rect;
}

JiWrapPanel* ji_wrap_panel_new(void) {
    JiWrapPanel* wp = JI_NEW(JiWrapPanel);
    if (!wp) return NULL;
    memset(wp, 0, sizeof(JiWrapPanel));

    JiLayoutElement* layout = &wp->panel.layout;
    ji_panel_init_layout(layout, s_wrap_panel_type);

    layout->measure_override = ji_wrap_panel_measure_override;
    layout->arrange_override = ji_wrap_panel_arrange_override;

    wp->orientation = JI_ORIENTATION_HORIZONTAL;
    wp->item_width  = 0.0;
    wp->item_height = 0.0;

    return wp;
}

void ji_wrap_panel_destroy(JiWrapPanel* wp) {
    if (!wp) return;
    JiPanel* panel = &wp->panel;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_object_destroy(&panel->layout.base);
}

void ji_wrap_panel_set_orientation(JiWrapPanel* wp, JiOrientation orientation) {
    if (!wp) return;
    wp->orientation = orientation;
    ji_layout_element_invalidate_measure(&wp->panel.layout);
}

void ji_wrap_panel_set_item_width(JiWrapPanel* wp, double width) {
    if (!wp) return;
    wp->item_width = width;
    ji_layout_element_invalidate_measure(&wp->panel.layout);
}

void ji_wrap_panel_set_item_height(JiWrapPanel* wp, double height) {
    if (!wp) return;
    wp->item_height = height;
    ji_layout_element_invalidate_measure(&wp->panel.layout);
}

/* =========================================================================
 * JiDockPanel
 * ========================================================================= */
static JiSize ji_dock_panel_measure_override(JiLayoutElement* self, JiSize available_size) {
    JiDockPanel* dp = (JiDockPanel*)self;
    JiPanel* panel = &dp->panel;

    double remaining_w = available_size.width;
    double remaining_h = available_size.height;
    double max_w = 0.0, max_h = 0.0;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        ji_layout_element_measure(child, ji_size(remaining_w, remaining_h));
        JiSize desired = ji_layout_element_get_desired_size(child);

        JiDock dock = ji_dock_panel_get_dock(child);
        bool is_last = (i == panel->child_count - 1);

        if (is_last && dp->last_child_fill) {
            /* Last child fills remaining space */
        } else {
            switch (dock) {
                case JI_DOCK_LEFT:
                case JI_DOCK_RIGHT:
                    remaining_w -= desired.width;
                    if (desired.height > max_h) max_h = desired.height;
                    break;
                case JI_DOCK_TOP:
                case JI_DOCK_BOTTOM:
                    remaining_h -= desired.height;
                    if (desired.width > max_w) max_w = desired.width;
                    break;
            }
        }

        if (remaining_w < 0.0) remaining_w = 0.0;
        if (remaining_h < 0.0) remaining_h = 0.0;
    }

    return ji_size(available_size.width - remaining_w + max_w,
                    available_size.height - remaining_h + max_h);
}

static JiRect ji_dock_panel_arrange_override(JiLayoutElement* self, JiRect final_rect) {
    JiDockPanel* dp = (JiDockPanel*)self;
    JiPanel* panel = &dp->panel;

    double x = final_rect.x;
    double y = final_rect.y;
    double w = final_rect.width;
    double h = final_rect.height;

    for (int i = 0; i < panel->child_count; i++) {
        JiLayoutElement* child = panel->children[i];
        JiSize desired = ji_layout_element_get_desired_size(child);
        JiDock dock = ji_dock_panel_get_dock(child);
        bool is_last = (i == panel->child_count - 1);

        JiRect child_rect;

        if (is_last && dp->last_child_fill) {
            child_rect = ji_rect(x, y, w, h);
        } else {
            switch (dock) {
                case JI_DOCK_LEFT:
                    child_rect = ji_rect(x, y, desired.width, h);
                    x += desired.width;
                    w -= desired.width;
                    break;
                case JI_DOCK_RIGHT:
                    child_rect = ji_rect(x + w - desired.width, y, desired.width, h);
                    w -= desired.width;
                    break;
                case JI_DOCK_TOP:
                    child_rect = ji_rect(x, y, w, desired.height);
                    y += desired.height;
                    h -= desired.height;
                    break;
                case JI_DOCK_BOTTOM:
                    child_rect = ji_rect(x, y + h - desired.height, w, desired.height);
                    h -= desired.height;
                    break;
                default:
                    child_rect = ji_rect(x, y, w, h);
                    break;
            }
        }

        ji_layout_element_arrange(child, child_rect);
    }

    return final_rect;
}

JiDockPanel* ji_dock_panel_new(void) {
    JiDockPanel* dp = JI_NEW(JiDockPanel);
    if (!dp) return NULL;
    memset(dp, 0, sizeof(JiDockPanel));

    JiLayoutElement* layout = &dp->panel.layout;
    ji_panel_init_layout(layout, s_dock_panel_type);

    layout->measure_override = ji_dock_panel_measure_override;
    layout->arrange_override = ji_dock_panel_arrange_override;

    dp->last_child_fill = true;

    return dp;
}

void ji_dock_panel_destroy(JiDockPanel* dp) {
    if (!dp) return;
    JiPanel* panel = &dp->panel;
    for (int i = 0; i < panel->child_count; i++) {
        ji_layout_element_destroy(panel->children[i]);
    }
    ji_free(panel->children);
    ji_object_destroy(&panel->layout.base);
}

void ji_dock_panel_set_last_child_fill(JiDockPanel* dp, bool fill) {
    if (!dp) return;
    dp->last_child_fill = fill;
    ji_layout_element_invalidate_measure(&dp->panel.layout);
}

void ji_dock_panel_set_dock(JiLayoutElement* child, JiDock dock) {
    if (!child) return;
    ji_object_set_int(&child->base, s_dock_prop, (int)dock);
}

JiDock ji_dock_panel_get_dock(const JiLayoutElement* child) {
    if (!child) return JI_DOCK_LEFT;
    return (JiDock)ji_object_get_int((JiObject*)&child->base, s_dock_prop);
}

/* =========================================================================
 * Panel type registration
 * ========================================================================= */
void ji_panel_type_init(void) {
    /* Register panel types in the type hierarchy */
    s_panel_type       = ji_type_register("JiPanel", sizeof(JiPanel),
                                          ji_layout_element_type_id(), NULL, NULL);
    s_stack_panel_type = ji_type_register("JiStackPanel", sizeof(JiStackPanel),
                                          s_panel_type, NULL, NULL);
    s_grid_type        = ji_type_register("JiGrid", sizeof(JiGrid),
                                          s_panel_type, NULL, NULL);
    s_canvas_type      = ji_type_register("JiCanvas", sizeof(JiCanvas),
                                          s_panel_type, NULL, NULL);
    s_wrap_panel_type  = ji_type_register("JiWrapPanel", sizeof(JiWrapPanel),
                                          s_panel_type, NULL, NULL);
    s_dock_panel_type  = ji_type_register("JiDockPanel", sizeof(JiDockPanel),
                                          s_panel_type, NULL, NULL);

    /* Register attached properties */
    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(JiPropertyMetadata));
    meta.default_value = ji_value_double(0.0);

    s_canvas_left_prop = ji_property_register_attached("Canvas.Left",
        s_canvas_type, JI_PROP_TYPE_DOUBLE, meta);
    s_canvas_top_prop  = ji_property_register_attached("Canvas.Top",
        s_canvas_type, JI_PROP_TYPE_DOUBLE, meta);

    meta.default_value = ji_value_int(0);
    s_grid_row_prop    = ji_property_register_attached("Grid.Row",
        s_grid_type, JI_PROP_TYPE_INT, meta);
    s_grid_col_prop    = ji_property_register_attached("Grid.Column",
        s_grid_type, JI_PROP_TYPE_INT, meta);
    s_grid_row_span_prop = ji_property_register_attached("Grid.RowSpan",
        s_grid_type, JI_PROP_TYPE_INT, meta);
    s_grid_col_span_prop = ji_property_register_attached("Grid.ColumnSpan",
        s_grid_type, JI_PROP_TYPE_INT, meta);
    s_dock_prop        = ji_property_register_attached("DockPanel.Dock",
        s_dock_panel_type, JI_PROP_TYPE_INT, meta);
}
