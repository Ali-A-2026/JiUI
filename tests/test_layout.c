/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_layout.c
 * @brief Tests for the layout system and panel types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jiui/jiui.h>

/* =========================================================================
 * Simple test framework
 * ========================================================================= */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) do {                                      \
    g_tests_run++;                                                       \
    if (cond) {                                                          \
        g_tests_passed++;                                                \
    } else {                                                             \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s\n", __FILE__, __LINE__, msg); \
    }                                                                    \
} while(0)

#define TEST_ASSERT_EQ(actual, expected, msg) do {                       \
    g_tests_run++;                                                        \
    if ((actual) == (expected)) {                                         \
        g_tests_passed++;                                                 \
    } else {                                                              \
        g_tests_failed++;                                                 \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %d, got %d)\n",    \
                __FILE__, __LINE__, msg, (int)(expected), (int)(actual));  \
    }                                                                     \
} while(0)

#define TEST_ASSERT_FEQ(actual, expected, msg) do {                      \
    g_tests_run++;                                                        \
    if (fabs((actual) - (expected)) < 1e-6) {                             \
        g_tests_passed++;                                                 \
    } else {                                                              \
        g_tests_failed++;                                                 \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %f, got %f)\n",    \
                __FILE__, __LINE__, msg, (double)(expected), (double)(actual)); \
    }                                                                     \
} while(0)

/* =========================================================================
 * NaN tests
 * ========================================================================= */
static void test_nan_helpers(void) {
    printf("  NaN helpers...\n");
    double nan_val = ji_nan();
    TEST_ASSERT(ji_is_nan(nan_val), "ji_nan() should be NaN");
    TEST_ASSERT(!ji_is_nan(0.0), "0.0 should not be NaN");
    TEST_ASSERT(!ji_is_nan(1.0), "1.0 should not be NaN");
    TEST_ASSERT(!ji_is_nan(-1.0), "-1.0 should not be NaN");
}

/* =========================================================================
 * Layout Element tests
 * ========================================================================= */
static void test_layout_element_create(void) {
    printf("  LayoutElement: create/destroy...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    TEST_ASSERT(elem != NULL, "Layout element should not be NULL");
    TEST_ASSERT_EQ(elem->horizontal_alignment, JI_ALIGN_H_STRETCH, "Default H alignment should be STRETCH");
    TEST_ASSERT_EQ(elem->vertical_alignment, JI_ALIGN_V_STRETCH, "Default V alignment should be STRETCH");
    TEST_ASSERT(ji_is_nan(elem->width), "Default width should be NaN (auto)");
    TEST_ASSERT(ji_is_nan(elem->height), "Default height should be NaN (auto)");
    TEST_ASSERT_FEQ(elem->min_width, 0.0, "Default min_width should be 0");
    TEST_ASSERT_FEQ(elem->max_width, 1e9, "Default max_width should be 1e9");
    TEST_ASSERT(ji_layout_element_is_measure_dirty(elem), "New element should be measure dirty");
    TEST_ASSERT(ji_layout_element_is_arrange_dirty(elem), "New element should be arrange dirty");
    ji_layout_element_destroy(elem);
}

static void test_layout_element_measure(void) {
    printf("  LayoutElement: measure...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    
    /* Default measure: explicit width/height or 0 */
    ji_layout_element_measure(elem, ji_size(1000, 1000));
    JiSize desired = ji_layout_element_get_desired_size(elem);
    TEST_ASSERT_FEQ(desired.width, 0.0, "Default measure should return 0 width");
    TEST_ASSERT_FEQ(desired.height, 0.0, "Default measure should return 0 height");
    TEST_ASSERT(!ji_layout_element_is_measure_dirty(elem), "After measure, should not be dirty");

    /* Set explicit size */
    elem->width = 200;
    elem->height = 100;
    ji_layout_element_invalidate_measure(elem);
    TEST_ASSERT(ji_layout_element_is_measure_dirty(elem), "After invalidate, should be dirty");
    ji_layout_element_measure(elem, ji_size(1000, 1000));
    desired = ji_layout_element_get_desired_size(elem);
    TEST_ASSERT_FEQ(desired.width, 200.0, "Explicit width should be 200");
    TEST_ASSERT_FEQ(desired.height, 100.0, "Explicit height should be 100");

    ji_layout_element_destroy(elem);
}

static void test_layout_element_arrange(void) {
    printf("  LayoutElement: arrange...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    
    JiRect final_rect = ji_rect(10, 20, 300, 200);
    ji_layout_element_measure(elem, ji_size(300, 200));
    ji_layout_element_arrange(elem, final_rect);
    
    JiRect slot = ji_layout_element_get_layout_slot(elem);
    TEST_ASSERT_FEQ(slot.x, 10.0, "Slot x should be 10");
    TEST_ASSERT_FEQ(slot.y, 20.0, "Slot y should be 20");
    TEST_ASSERT_FEQ(slot.width, 300.0, "Slot width should be 300");
    TEST_ASSERT_FEQ(slot.height, 200.0, "Slot height should be 200");
    TEST_ASSERT(!ji_layout_element_is_arrange_dirty(elem), "After arrange, should not be dirty");

    ji_layout_element_destroy(elem);
}

static void test_layout_element_alignment(void) {
    printf("  LayoutElement: alignment...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    elem->width = 50;
    elem->height = 30;

    /* Center alignment */
    elem->horizontal_alignment = JI_ALIGN_H_CENTER;
    elem->vertical_alignment = JI_ALIGN_V_CENTER;
    ji_layout_element_measure(elem, ji_size(200, 100));
    ji_layout_element_arrange(elem, ji_rect(0, 0, 200, 100));
    
    JiRect slot = ji_layout_element_get_layout_slot(elem);
    TEST_ASSERT_FEQ(slot.x, 75.0, "Center H: x should be 75");
    TEST_ASSERT_FEQ(slot.y, 35.0, "Center V: y should be 35");
    TEST_ASSERT_FEQ(slot.width, 50.0, "Center H: width should be 50");
    TEST_ASSERT_FEQ(slot.height, 30.0, "Center V: height should be 30");

    /* Right/Bottom alignment */
    elem->horizontal_alignment = JI_ALIGN_H_RIGHT;
    elem->vertical_alignment = JI_ALIGN_V_BOTTOM;
    ji_layout_element_invalidate_measure(elem);
    ji_layout_element_measure(elem, ji_size(200, 100));
    ji_layout_element_arrange(elem, ji_rect(0, 0, 200, 100));
    
    slot = ji_layout_element_get_layout_slot(elem);
    TEST_ASSERT_FEQ(slot.x, 150.0, "Right: x should be 150");
    TEST_ASSERT_FEQ(slot.y, 70.0, "Bottom: y should be 70");

    ji_layout_element_destroy(elem);
}

static void test_layout_element_margin(void) {
    printf("  LayoutElement: margin...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    elem->width = 100;
    elem->height = 50;
    elem->margin = ji_thickness_all(10, 5, 10, 5);

    /* Collapse margin */
    JiSize available = ji_size(200, 100);
    JiSize collapsed = ji_layout_element_collapse_margin(elem, available);
    TEST_ASSERT_FEQ(collapsed.width, 180.0, "Collapsed width should be 180");
    TEST_ASSERT_FEQ(collapsed.height, 90.0, "Collapsed height should be 90");

    /* Expand margin */
    JiSize size = ji_size(100, 50);
    JiSize expanded = ji_layout_element_expand_margin(elem, size);
    TEST_ASSERT_FEQ(expanded.width, 120.0, "Expanded width should be 120");
    TEST_ASSERT_FEQ(expanded.height, 60.0, "Expanded height should be 60");

    ji_layout_element_destroy(elem);
}

static void test_layout_element_clamp(void) {
    printf("  LayoutElement: clamp size...\n");
    JiLayoutElement* elem = ji_layout_element_new(ji_layout_element_type_id());
    elem->min_width = 50;
    elem->max_width = 200;
    elem->min_height = 30;
    elem->max_height = 150;

    JiSize clamped = ji_layout_element_clamp_size(elem, ji_size(100, 100));
    TEST_ASSERT_FEQ(clamped.width, 100.0, "Within range: width should be 100");
    TEST_ASSERT_FEQ(clamped.height, 100.0, "Within range: height should be 100");

    clamped = ji_layout_element_clamp_size(elem, ji_size(20, 20));
    TEST_ASSERT_FEQ(clamped.width, 50.0, "Below min: width should be 50");
    TEST_ASSERT_FEQ(clamped.height, 30.0, "Below min: height should be 30");

    clamped = ji_layout_element_clamp_size(elem, ji_size(300, 300));
    TEST_ASSERT_FEQ(clamped.width, 200.0, "Above max: width should be 200");
    TEST_ASSERT_FEQ(clamped.height, 150.0, "Above max: height should be 150");

    ji_layout_element_destroy(elem);
}

/* =========================================================================
 * Panel tests
 * ========================================================================= */
static void test_panel_create(void) {
    printf("  Panel: create/destroy...\n");
    JiPanel* panel = ji_panel_new(ji_panel_type_id());
    TEST_ASSERT(panel != NULL, "Panel should not be NULL");
    TEST_ASSERT_EQ(ji_panel_get_child_count(panel), 0, "New panel should have 0 children");
    ji_panel_destroy(panel);
}

static void test_panel_add_remove_child(void) {
    printf("  Panel: add/remove child...\n");
    JiPanel* panel = ji_panel_new(ji_panel_type_id());
    JiLayoutElement* child1 = ji_layout_element_new(ji_layout_element_type_id());
    JiLayoutElement* child2 = ji_layout_element_new(ji_layout_element_type_id());

    ji_panel_add_child(panel, child1);
    TEST_ASSERT_EQ(ji_panel_get_child_count(panel), 1, "Should have 1 child");
    TEST_ASSERT(ji_panel_get_child(panel, 0) == child1, "First child should be child1");
    TEST_ASSERT(child1->layout_parent == &panel->layout, "Child's layout_parent should be set");

    ji_panel_add_child(panel, child2);
    TEST_ASSERT_EQ(ji_panel_get_child_count(panel), 2, "Should have 2 children");

    bool removed = ji_panel_remove_child(panel, child1);
    TEST_ASSERT(removed, "Remove should succeed");
    TEST_ASSERT_EQ(ji_panel_get_child_count(panel), 1, "Should have 1 child after remove");
    TEST_ASSERT(child1->layout_parent == NULL, "Removed child's parent should be NULL");

    ji_layout_element_destroy(child1);
    ji_panel_destroy(panel);
}

/* =========================================================================
 * StackPanel tests
 * ========================================================================= */
static void test_stack_panel_vertical(void) {
    printf("  StackPanel: vertical layout...\n");
    JiStackPanel* sp = ji_stack_panel_new();
    TEST_ASSERT(sp != NULL, "StackPanel should not be NULL");
    TEST_ASSERT_EQ(sp->orientation, JI_ORIENTATION_VERTICAL, "Default orientation should be vertical");

    /* Add children with explicit sizes */
    JiLayoutElement* c1 = ji_layout_element_new(ji_layout_element_type_id());
    c1->width = 50; c1->height = 30;
    JiLayoutElement* c2 = ji_layout_element_new(ji_layout_element_type_id());
    c2->width = 60; c2->height = 40;
    JiLayoutElement* c3 = ji_layout_element_new(ji_layout_element_type_id());
    c3->width = 40; c3->height = 20;

    ji_panel_add_child(&sp->panel, c1);
    ji_panel_add_child(&sp->panel, c2);
    ji_panel_add_child(&sp->panel, c3);

    /* Measure */
    ji_layout_element_measure(&sp->panel.layout, ji_size(1000, 1000));
    JiSize desired = ji_layout_element_get_desired_size(&sp->panel.layout);
    TEST_ASSERT_FEQ(desired.height, 90.0, "Vertical StackPanel height should be 90 (30+40+20)");
    TEST_ASSERT_FEQ(desired.width, 60.0, "Vertical StackPanel width should be 60 (max)");

    /* Arrange */
    ji_layout_element_arrange(&sp->panel.layout, ji_rect(0, 0, 200, 200));
    
    JiRect slot1 = ji_layout_element_get_layout_slot(c1);
    JiRect slot2 = ji_layout_element_get_layout_slot(c2);
    JiRect slot3 = ji_layout_element_get_layout_slot(c3);

    TEST_ASSERT_FEQ(slot1.y, 0.0, "Child 1 y should be 0");
    TEST_ASSERT_FEQ(slot2.y, 30.0, "Child 2 y should be 30");
    TEST_ASSERT_FEQ(slot3.y, 70.0, "Child 3 y should be 70");

    ji_stack_panel_destroy(sp);
}

static void test_stack_panel_horizontal(void) {
    printf("  StackPanel: horizontal layout...\n");
    JiStackPanel* sp = ji_stack_panel_new();
    ji_stack_panel_set_orientation(sp, JI_ORIENTATION_HORIZONTAL);

    JiLayoutElement* c1 = ji_layout_element_new(ji_layout_element_type_id());
    c1->width = 50; c1->height = 30;
    JiLayoutElement* c2 = ji_layout_element_new(ji_layout_element_type_id());
    c2->width = 60; c2->height = 40;

    ji_panel_add_child(&sp->panel, c1);
    ji_panel_add_child(&sp->panel, c2);

    ji_layout_element_measure(&sp->panel.layout, ji_size(1000, 1000));
    JiSize desired = ji_layout_element_get_desired_size(&sp->panel.layout);
    TEST_ASSERT_FEQ(desired.width, 110.0, "Horizontal StackPanel width should be 110 (50+60)");
    TEST_ASSERT_FEQ(desired.height, 40.0, "Horizontal StackPanel height should be 40 (max)");

    ji_layout_element_arrange(&sp->panel.layout, ji_rect(0, 0, 200, 200));
    
    JiRect slot1 = ji_layout_element_get_layout_slot(c1);
    JiRect slot2 = ji_layout_element_get_layout_slot(c2);
    TEST_ASSERT_FEQ(slot1.x, 0.0, "Child 1 x should be 0");
    TEST_ASSERT_FEQ(slot2.x, 50.0, "Child 2 x should be 50");

    ji_stack_panel_destroy(sp);
}

static void test_stack_panel_spacing(void) {
    printf("  StackPanel: spacing...\n");
    JiStackPanel* sp = ji_stack_panel_new();
    ji_stack_panel_set_spacing(sp, 10.0);

    JiLayoutElement* c1 = ji_layout_element_new(ji_layout_element_type_id());
    c1->width = 50; c1->height = 30;
    JiLayoutElement* c2 = ji_layout_element_new(ji_layout_element_type_id());
    c2->width = 50; c2->height = 30;

    ji_panel_add_child(&sp->panel, c1);
    ji_panel_add_child(&sp->panel, c2);

    ji_layout_element_measure(&sp->panel.layout, ji_size(1000, 1000));
    JiSize desired = ji_layout_element_get_desired_size(&sp->panel.layout);
    TEST_ASSERT_FEQ(desired.height, 70.0, "With spacing: height should be 70 (30+10+30)");

    ji_layout_element_arrange(&sp->panel.layout, ji_rect(0, 0, 200, 200));
    JiRect slot2 = ji_layout_element_get_layout_slot(c2);
    TEST_ASSERT_FEQ(slot2.y, 40.0, "Child 2 y should be 40 (30+10)");

    ji_stack_panel_destroy(sp);
}

/* =========================================================================
 * Grid tests
 * ========================================================================= */
static void test_grid_basic(void) {
    printf("  Grid: basic layout...\n");
    JiGrid* grid = ji_grid_new();
    TEST_ASSERT(grid != NULL, "Grid should not be NULL");

    /* Add 2 columns and 2 rows */
    ji_grid_add_column(grid, ji_grid_length_pixel(100));
    ji_grid_add_column(grid, ji_grid_length_pixel(200));
    ji_grid_add_row(grid, ji_grid_length_pixel(50));
    ji_grid_add_row(grid, ji_grid_length_pixel(80));

    TEST_ASSERT_EQ(ji_grid_get_row_count(grid), 2, "Should have 2 rows");
    TEST_ASSERT_EQ(ji_grid_get_col_count(grid), 2, "Should have 2 columns");

    /* Add children in cells */
    JiLayoutElement* c00 = ji_layout_element_new(ji_layout_element_type_id());
    c00->width = 50; c00->height = 30;
    ji_dock_panel_set_dock(c00, JI_DOCK_LEFT); /* just to set a property */
    ji_panel_add_child(&grid->panel, c00);

    /* Measure */
    ji_layout_element_measure(&grid->panel.layout, ji_size(300, 130));
    JiSize desired = ji_layout_element_get_desired_size(&grid->panel.layout);
    TEST_ASSERT_FEQ(desired.width, 300.0, "Grid width should be 300 (100+200)");
    TEST_ASSERT_FEQ(desired.height, 130.0, "Grid height should be 130 (50+80)");

    /* Arrange */
    ji_layout_element_arrange(&grid->panel.layout, ji_rect(0, 0, 300, 130));
    JiRect slot00 = ji_layout_element_get_layout_slot(c00);
    TEST_ASSERT_FEQ(slot00.x, 0.0, "Cell (0,0) x should be 0");
    TEST_ASSERT_FEQ(slot00.y, 0.0, "Cell (0,0) y should be 0");
    TEST_ASSERT_FEQ(slot00.width, 100.0, "Cell (0,0) width should be 100");
    TEST_ASSERT_FEQ(slot00.height, 50.0, "Cell (0,0) height should be 50");

    ji_grid_destroy(grid);
}

static void test_grid_star(void) {
    printf("  Grid: star sizing...\n");
    JiGrid* grid = ji_grid_new();

    ji_grid_add_column(grid, ji_grid_length_star(1.0));
    ji_grid_add_column(grid, ji_grid_length_star(2.0));
    ji_grid_add_row(grid, ji_grid_length_star(1.0));

    JiLayoutElement* c = ji_layout_element_new(ji_layout_element_type_id());
    c->width = 10; c->height = 10;
    ji_panel_add_child(&grid->panel, c);

    /* Measure with 300px width */
    ji_layout_element_measure(&grid->panel.layout, ji_size(300, 100));
    ji_layout_element_arrange(&grid->panel.layout, ji_rect(0, 0, 300, 100));

    /* Column 0 should be 100px (1/3 of 300), Column 1 should be 200px (2/3 of 300) */
    JiRect slot = ji_layout_element_get_layout_slot(c);
    TEST_ASSERT_FEQ(slot.width, 100.0, "Star column 0 should be 100px");
    TEST_ASSERT_FEQ(slot.height, 100.0, "Star row 0 should be 100px");

    ji_grid_destroy(grid);
}

/* =========================================================================
 * Canvas tests
 * ========================================================================= */
static void test_canvas_basic(void) {
    printf("  Canvas: basic layout...\n");
    JiCanvas* canvas = ji_canvas_new();
    TEST_ASSERT(canvas != NULL, "Canvas should not be NULL");

    JiLayoutElement* c1 = ji_layout_element_new(ji_layout_element_type_id());
    c1->width = 50; c1->height = 30;
    ji_canvas_set_left(c1, 10.0);
    ji_canvas_set_top(c1, 20.0);

    JiLayoutElement* c2 = ji_layout_element_new(ji_layout_element_type_id());
    c2->width = 60; c2->height = 40;
    ji_canvas_set_left(c2, 100.0);
    ji_canvas_set_top(c2, 50.0);

    ji_panel_add_child(&canvas->panel, c1);
    ji_panel_add_child(&canvas->panel, c2);

    /* Measure */
    ji_layout_element_measure(&canvas->panel.layout, ji_size(1000, 1000));
    JiSize desired = ji_layout_element_get_desired_size(&canvas->panel.layout);
    TEST_ASSERT_FEQ(desired.width, 160.0, "Canvas width should be 160 (100+60)");
    TEST_ASSERT_FEQ(desired.height, 90.0, "Canvas height should be 90 (50+40)");

    /* Arrange */
    ji_layout_element_arrange(&canvas->panel.layout, ji_rect(0, 0, 200, 200));
    
    JiRect slot1 = ji_layout_element_get_layout_slot(c1);
    TEST_ASSERT_FEQ(slot1.x, 10.0, "Canvas child 1 x should be 10");
    TEST_ASSERT_FEQ(slot1.y, 20.0, "Canvas child 1 y should be 20");
    TEST_ASSERT_FEQ(slot1.width, 50.0, "Canvas child 1 width should be 50");

    JiRect slot2 = ji_layout_element_get_layout_slot(c2);
    TEST_ASSERT_FEQ(slot2.x, 100.0, "Canvas child 2 x should be 100");
    TEST_ASSERT_FEQ(slot2.y, 50.0, "Canvas child 2 y should be 50");

    ji_canvas_destroy(canvas);
}

/* =========================================================================
 * DockPanel tests
 * ========================================================================= */
static void test_dock_panel_basic(void) {
    printf("  DockPanel: basic layout...\n");
    JiDockPanel* dp = ji_dock_panel_new();
    TEST_ASSERT(dp != NULL, "DockPanel should not be NULL");
    TEST_ASSERT(dp->last_child_fill == true, "Default last_child_fill should be true");

    /* Left dock */
    JiLayoutElement* left = ji_layout_element_new(ji_layout_element_type_id());
    left->width = 100; left->height = 50;
    ji_dock_panel_set_dock(left, JI_DOCK_LEFT);
    ji_panel_add_child(&dp->panel, left);

    /* Top dock */
    JiLayoutElement* top = ji_layout_element_new(ji_layout_element_type_id());
    top->width = 50; top->height = 30;
    ji_dock_panel_set_dock(top, JI_DOCK_TOP);
    ji_panel_add_child(&dp->panel, top);

    /* Last child fills remaining */
    JiLayoutElement* fill = ji_layout_element_new(ji_layout_element_type_id());
    fill->width = 10; fill->height = 10;
    ji_panel_add_child(&dp->panel, fill);

    /* Measure and arrange */
    ji_layout_element_measure(&dp->panel.layout, ji_size(400, 300));
    ji_layout_element_arrange(&dp->panel.layout, ji_rect(0, 0, 400, 300));

    JiRect left_slot = ji_layout_element_get_layout_slot(left);
    TEST_ASSERT_FEQ(left_slot.x, 0.0, "Left dock x should be 0");
    TEST_ASSERT_FEQ(left_slot.width, 100.0, "Left dock width should be 100");
    TEST_ASSERT_FEQ(left_slot.height, 300.0, "Left dock height should be full");

    JiRect top_slot = ji_layout_element_get_layout_slot(top);
    TEST_ASSERT_FEQ(top_slot.x, 100.0, "Top dock x should be 100 (after left)");
    TEST_ASSERT_FEQ(top_slot.y, 0.0, "Top dock y should be 0");
    TEST_ASSERT_FEQ(top_slot.height, 30.0, "Top dock height should be 30");

    JiRect fill_slot = ji_layout_element_get_layout_slot(fill);
    TEST_ASSERT_FEQ(fill_slot.x, 100.0, "Fill x should be 100");
    TEST_ASSERT_FEQ(fill_slot.y, 30.0, "Fill y should be 30");
    TEST_ASSERT_FEQ(fill_slot.width, 300.0, "Fill width should be 300 (400-100)");
    TEST_ASSERT_FEQ(fill_slot.height, 270.0, "Fill height should be 270 (300-30)");

    ji_dock_panel_destroy(dp);
}

/* =========================================================================
 * WrapPanel tests
 * ========================================================================= */
static void test_wrap_panel_basic(void) {
    printf("  WrapPanel: basic layout...\n");
    JiWrapPanel* wp = ji_wrap_panel_new();
    TEST_ASSERT(wp != NULL, "WrapPanel should not be NULL");

    /* Add 3 children that fit on one line */
    JiLayoutElement* c1 = ji_layout_element_new(ji_layout_element_type_id());
    c1->width = 50; c1->height = 30;
    JiLayoutElement* c2 = ji_layout_element_new(ji_layout_element_type_id());
    c2->width = 50; c2->height = 30;
    JiLayoutElement* c3 = ji_layout_element_new(ji_layout_element_type_id());
    c3->width = 50; c3->height = 30;

    ji_panel_add_child(&wp->panel, c1);
    ji_panel_add_child(&wp->panel, c2);
    ji_panel_add_child(&wp->panel, c3);

    /* Measure with enough space for all on one line */
    ji_layout_element_measure(&wp->panel.layout, ji_size(200, 100));
    JiSize desired = ji_layout_element_get_desired_size(&wp->panel.layout);
    TEST_ASSERT_FEQ(desired.width, 150.0, "WrapPanel width should be 150 (3*50)");
    TEST_ASSERT_FEQ(desired.height, 30.0, "WrapPanel height should be 30 (one line)");

    /* Measure with space for only 2 on a line */
    ji_layout_element_invalidate_measure(&wp->panel.layout);
    ji_layout_element_measure(&wp->panel.layout, ji_size(110, 100));
    desired = ji_layout_element_get_desired_size(&wp->panel.layout);
    TEST_ASSERT_FEQ(desired.width, 100.0, "WrapPanel wrapped width should be 100 (2*50)");
    TEST_ASSERT_FEQ(desired.height, 60.0, "WrapPanel wrapped height should be 60 (2 lines)");

    ji_wrap_panel_destroy(wp);
}

/* =========================================================================
 * Type hierarchy tests
 * ========================================================================= */
static void test_type_hierarchy(void) {
    printf("  Type hierarchy...\n");
    JiTypeId layout_type = ji_layout_element_type_id();
    JiTypeId panel_type  = ji_panel_type_id();
    JiTypeId sp_type     = ji_stack_panel_type_id();
    JiTypeId grid_type   = ji_grid_type_id();
    JiTypeId canvas_type  = ji_canvas_type_id();
    JiTypeId wp_type     = ji_wrap_panel_type_id();
    JiTypeId dp_type     = ji_dock_panel_type_id();

    TEST_ASSERT(layout_type != JI_TYPE_INVALID, "LayoutElement type should be registered");
    TEST_ASSERT(panel_type  != JI_TYPE_INVALID, "Panel type should be registered");
    TEST_ASSERT(sp_type     != JI_TYPE_INVALID, "StackPanel type should be registered");
    TEST_ASSERT(grid_type   != JI_TYPE_INVALID, "Grid type should be registered");
    TEST_ASSERT(canvas_type  != JI_TYPE_INVALID, "Canvas type should be registered");
    TEST_ASSERT(wp_type     != JI_TYPE_INVALID, "WrapPanel type should be registered");
    TEST_ASSERT(dp_type     != JI_TYPE_INVALID, "DockPanel type should be registered");

    /* Check hierarchy */
    TEST_ASSERT(ji_type_is_a(panel_type, layout_type), "Panel should be a LayoutElement");
    TEST_ASSERT(ji_type_is_a(sp_type, panel_type), "StackPanel should be a Panel");
    TEST_ASSERT(ji_type_is_a(sp_type, layout_type), "StackPanel should be a LayoutElement");
    TEST_ASSERT(ji_type_is_a(grid_type, panel_type), "Grid should be a Panel");
    TEST_ASSERT(ji_type_is_a(canvas_type, panel_type), "Canvas should be a Panel");
    TEST_ASSERT(ji_type_is_a(wp_type, panel_type), "WrapPanel should be a Panel");
    TEST_ASSERT(ji_type_is_a(dp_type, panel_type), "DockPanel should be a Panel");
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    printf("=== JiUI Layout Test Suite ===\n\n");

    JiResultCode rc = ji_initialize();
    if (rc != JI_OK) {
        fprintf(stderr, "FATAL: ji_initialize() failed with code %d\n", rc);
        return 1;
    }

    printf("--- NaN Helpers ---\n");
    test_nan_helpers();

    printf("\n--- Layout Element ---\n");
    test_layout_element_create();
    test_layout_element_measure();
    test_layout_element_arrange();
    test_layout_element_alignment();
    test_layout_element_margin();
    test_layout_element_clamp();

    printf("\n--- Panel ---\n");
    test_panel_create();
    test_panel_add_remove_child();

    printf("\n--- StackPanel ---\n");
    test_stack_panel_vertical();
    test_stack_panel_horizontal();
    test_stack_panel_spacing();

    printf("\n--- Grid ---\n");
    test_grid_basic();
    test_grid_star();

    printf("\n--- Canvas ---\n");
    test_canvas_basic();

    printf("\n--- DockPanel ---\n");
    test_dock_panel_basic();

    printf("\n--- WrapPanel ---\n");
    test_wrap_panel_basic();

    printf("\n--- Type Hierarchy ---\n");
    test_type_hierarchy();

    printf("\n=== Results ===\n");
    printf("  Total:  %d\n", g_tests_run);
    printf("  Passed: %d\n", g_tests_passed);
    printf("  Failed: %d\n", g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
