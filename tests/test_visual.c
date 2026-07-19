/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_visual.c
 * @brief Tests for the visual system and drawing primitives.
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Minimal test framework ---- */
static int s_tests_run    = 0;
static int s_tests_passed = 0;
static int s_tests_failed = 0;

#define TEST_ASSERT(cond, msg)                                              \
    do {                                                                     \
        s_tests_run++;                                                       \
        if (cond) {                                                          \
            s_tests_passed++;                                                \
        } else {                                                             \
            s_tests_failed++;                                                \
            fprintf(stderr, "  FAIL [%s:%d]: %s\n", __func__, __LINE__, msg); \
        }                                                                    \
    } while (0)

#define TEST_FLOAT_EQ(a, b, eps, msg)                                       \
    do {                                                                     \
        s_tests_run++;                                                       \
        if (fabs((a) - (b)) < (eps)) {                                     \
            s_tests_passed++;                                                \
        } else {                                                             \
            s_tests_failed++;                                                \
            fprintf(stderr, "  FAIL [%s:%d]: %s (got %f, expected %f)\n",   \
                    __func__, __LINE__, msg, (double)(a), (double)(b));       \
        }                                                                    \
    } while (0)

/* =========================================================================
 * Visual tests
 * ========================================================================= */
static void test_visual_create_destroy(void) {
    JiVisual* v = ji_visual_new();
    TEST_ASSERT(v != NULL, "Visual should be created");
    TEST_ASSERT(v->opacity == 1.0, "Default opacity should be 1.0");
    TEST_ASSERT(v->visibility == JI_VISIBILITY_VISIBLE, "Default visibility should be VISIBLE");
    TEST_ASSERT(v->is_visible == true, "Default effective visibility should be true");
    TEST_ASSERT(v->is_dirty == true, "New visual should be dirty");
    TEST_ASSERT(v->z_index == 0, "Default z_index should be 0");
    TEST_ASSERT(v->visual_parent == NULL, "New visual should have no parent");
    TEST_ASSERT(v->visual_child_count == 0, "New visual should have no children");
    TEST_ASSERT(v->on_render == NULL, "Default on_render should be NULL");
    ji_visual_destroy(v);
}

static void test_visual_opacity(void) {
    JiVisual* v = ji_visual_new();
    TEST_ASSERT(v != NULL, "Visual should be created");

    ji_visual_set_opacity(v, 0.5);
    TEST_FLOAT_EQ(ji_visual_get_opacity(v), 0.5, 1e-9, "Opacity should be 0.5");

    /* Clamping */
    ji_visual_set_opacity(v, -1.0);
    TEST_FLOAT_EQ(ji_visual_get_opacity(v), 0.0, 1e-9, "Opacity should clamp to 0.0");

    ji_visual_set_opacity(v, 2.0);
    TEST_FLOAT_EQ(ji_visual_get_opacity(v), 1.0, 1e-9, "Opacity should clamp to 1.0");

    ji_visual_destroy(v);
}

static void test_visual_visibility(void) {
    JiVisual* v = ji_visual_new();
    TEST_ASSERT(v != NULL, "Visual should be created");

    ji_visual_set_visibility(v, JI_VISIBILITY_HIDDEN);
    TEST_ASSERT(ji_visual_get_visibility(v) == JI_VISIBILITY_HIDDEN, "Visibility should be HIDDEN");
    TEST_ASSERT(v->is_visible == false, "Hidden visual should not be effectively visible");

    ji_visual_set_visibility(v, JI_VISIBILITY_COLLAPSED);
    TEST_ASSERT(ji_visual_get_visibility(v) == JI_VISIBILITY_COLLAPSED, "Visibility should be COLLAPSED");
    TEST_ASSERT(v->is_visible == false, "Collapsed visual should not be effectively visible");

    ji_visual_set_visibility(v, JI_VISIBILITY_VISIBLE);
    TEST_ASSERT(ji_visual_get_visibility(v) == JI_VISIBILITY_VISIBLE, "Visibility should be VISIBLE");
    TEST_ASSERT(v->is_visible == true, "Visible visual should be effectively visible");

    ji_visual_destroy(v);
}

static void test_visual_tree(void) {
    JiVisual* root = ji_visual_new();
    JiVisual* c1   = ji_visual_new();
    JiVisual* c2   = ji_visual_new();

    TEST_ASSERT(root != NULL, "Root should be created");
    TEST_ASSERT(c1 != NULL, "Child 1 should be created");
    TEST_ASSERT(c2 != NULL, "Child 2 should be created");

    ji_visual_add_child(root, c1);
    ji_visual_add_child(root, c2);

    TEST_ASSERT(ji_visual_get_child_count(root) == 2, "Root should have 2 children");
    TEST_ASSERT(ji_visual_get_child(root, 0) == c1, "First child should be c1");
    TEST_ASSERT(ji_visual_get_child(root, 1) == c2, "Second child should be c2");
    TEST_ASSERT(c1->visual_parent == root, "c1 parent should be root");
    TEST_ASSERT(c2->visual_parent == root, "c2 parent should be root");

    /* Remove child */
    bool removed = ji_visual_remove_child(root, c1);
    TEST_ASSERT(removed, "Remove should succeed");
    TEST_ASSERT(ji_visual_get_child_count(root) == 1, "Root should have 1 child after remove");
    TEST_ASSERT(c1->visual_parent == NULL, "Removed child should have no parent");

    ji_visual_destroy(c1);
    ji_visual_destroy(root); /* destroys c2 too */
}

static void test_visual_effective_visibility(void) {
    JiVisual* root = ji_visual_new();
    JiVisual* child = ji_visual_new();

    ji_visual_add_child(root, child);

    /* Both visible */
    TEST_ASSERT(ji_visual_is_effectively_visible(root), "Root should be effectively visible");
    TEST_ASSERT(ji_visual_is_effectively_visible(child), "Child should be effectively visible");

    /* Hide parent — child should also become not effectively visible */
    ji_visual_set_visibility(root, JI_VISIBILITY_HIDDEN);
    TEST_ASSERT(!ji_visual_is_effectively_visible(root), "Hidden root should not be effectively visible");
    TEST_ASSERT(!ji_visual_is_effectively_visible(child), "Child of hidden root should not be effectively visible");

    /* Show parent again */
    ji_visual_set_visibility(root, JI_VISIBILITY_VISIBLE);
    TEST_ASSERT(ji_visual_is_effectively_visible(root), "Root should be effectively visible again");
    TEST_ASSERT(ji_visual_is_effectively_visible(child), "Child should be effectively visible again");

    ji_visual_destroy(root);
}

static void test_visual_z_index(void) {
    JiVisual* v = ji_visual_new();
    ji_visual_set_z_index(v, 42);
    TEST_ASSERT(ji_visual_get_z_index(v) == 42, "Z-index should be 42");
    ji_visual_destroy(v);
}

static void test_visual_clip(void) {
    JiVisual* v = ji_visual_new();
    JiRect clip = ji_rect(10, 20, 100, 200);
    ji_visual_set_clip(v, clip);
    JiRect got = ji_visual_get_clip(v);
    TEST_ASSERT(got.x == 10 && got.y == 20 && got.width == 100 && got.height == 200,
                "Clip rect should match");
    ji_visual_destroy(v);
}

static void test_visual_render_transform(void) {
    JiVisual* v = ji_visual_new();
    JiMatrix identity = ji_matrix_identity();
    JiMatrix transform = ji_matrix_translation(50.0, 30.0);

    /* Default should be identity */
    JiMatrix def = ji_visual_get_render_transform(v);
    TEST_ASSERT(ji_matrix_is_identity(def), "Default transform should be identity");

    ji_visual_set_render_transform(v, transform);
    JiMatrix got = ji_visual_get_render_transform(v);
    TEST_FLOAT_EQ(got.m31, 50.0, 1e-9, "Transform offset_x should be 50");
    TEST_FLOAT_EQ(got.m32, 30.0, 1e-9, "Transform offset_y should be 30");

    ji_visual_destroy(v);
}

static void test_visual_invalidate(void) {
    JiVisual* v = ji_visual_new();
    v->is_dirty = false;
    ji_visual_invalidate(v);
    TEST_ASSERT(v->is_dirty == true, "Invalidate should mark visual as dirty");
    ji_visual_destroy(v);
}

static void test_visual_type_id(void) {
    JiTypeId tid = ji_visual_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiVisual type should be registered");
}

/* =========================================================================
 * Drawing tests
 * ========================================================================= */
static void test_brush_solid(void) {
    JiBrush b = ji_brush_solid(0xFF0000FF); /* blue */
    TEST_ASSERT(b.kind == JI_BRUSH_SOLID, "Brush kind should be SOLID");
    TEST_ASSERT(b.v.color == 0xFF0000FF, "Brush color should match");
    ji_brush_destroy(&b);
}

static void test_brush_linear_gradient(void) {
    JiGradientStop stops[2] = {
        { 0.0, 0xFFFF0000 },  /* red at 0 */
        { 1.0, 0xFF0000FF }   /* blue at 1 */
    };
    JiBrush b = ji_brush_linear_gradient(
        ji_point(0, 0), ji_point(100, 100), stops, 2);
    TEST_ASSERT(b.kind == JI_BRUSH_LINEAR_GRAD, "Brush kind should be LINEAR_GRAD");
    TEST_ASSERT(b.v.linear.stop_count == 2, "Should have 2 stops");
    TEST_ASSERT(b.v.linear.stops != NULL, "Stops should be allocated");
    TEST_FLOAT_EQ(b.v.linear.stops[0].offset, 0.0, 1e-9, "First stop offset should be 0");
    TEST_FLOAT_EQ(b.v.linear.stops[1].offset, 1.0, 1e-9, "Second stop offset should be 1");
    ji_brush_destroy(&b);
    /* After destroy, stops should be freed */
    TEST_ASSERT(b.v.linear.stops == NULL, "Stops should be NULL after destroy");
}

static void test_brush_radial_gradient(void) {
    JiGradientStop stops[3] = {
        { 0.0, 0xFFFFFFFF },
        { 0.5, 0xFF808080 },
        { 1.0, 0xFF000000 }
    };
    JiBrush b = ji_brush_radial_gradient(
        ji_point(50, 50), 25.0, 25.0, stops, 3);
    TEST_ASSERT(b.kind == JI_BRUSH_RADIAL_GRAD, "Brush kind should be RADIAL_GRAD");
    TEST_ASSERT(b.v.radial.stop_count == 3, "Should have 3 stops");
    TEST_FLOAT_EQ(b.v.radial.radius_x, 25.0, 1e-9, "Radius X should be 25");
    TEST_FLOAT_EQ(b.v.radial.radius_y, 25.0, 1e-9, "Radius Y should be 25");
    ji_brush_destroy(&b);
}

static void test_pen_solid(void) {
    JiPen p = ji_pen_solid(0xFF000000, 2.5);
    TEST_ASSERT(p.brush.kind == JI_BRUSH_SOLID, "Pen brush should be SOLID");
    TEST_FLOAT_EQ(p.thickness, 2.5, 1e-9, "Pen thickness should be 2.5");
    TEST_ASSERT(p.dash_array == NULL, "Solid pen should have no dash array");
    TEST_ASSERT(p.dash_count == 0, "Solid pen should have dash count 0");
    ji_pen_destroy(&p);
}

static void test_pen_dashed(void) {
    double dashes[] = { 5.0, 3.0 };
    JiPen p = ji_pen_dashed(0xFF000000, 1.0, dashes, 2, 2.0);
    TEST_ASSERT(p.dash_count == 2, "Dashed pen should have 2 dashes");
    TEST_ASSERT(p.dash_array != NULL, "Dashed pen should have dash array");
    TEST_FLOAT_EQ(p.dash_offset, 2.0, 1e-9, "Dash offset should be 2.0");
    TEST_FLOAT_EQ(p.dash_array[0], 5.0, 1e-9, "First dash should be 5.0");
    TEST_FLOAT_EQ(p.dash_array[1], 3.0, 1e-9, "Second dash should be 3.0");
    ji_pen_destroy(&p);
    TEST_ASSERT(p.dash_array == NULL, "Dash array should be NULL after destroy");
}

static void test_geometry_rect(void) {
    JiGeometry g = ji_geometry_rect(ji_rect(10, 20, 100, 200));
    TEST_ASSERT(g.kind == JI_GEOM_RECT, "Geometry kind should be RECT");
    TEST_ASSERT(g.v.rect.x == 10 && g.v.rect.y == 20, "Rect position should match");
    TEST_ASSERT(g.v.rect.width == 100 && g.v.rect.height == 200, "Rect size should match");
}

static void test_geometry_rounded_rect(void) {
    JiGeometry g = ji_geometry_rounded_rect(
        ji_rect(0, 0, 50, 50), ji_corner_radius(5));
    TEST_ASSERT(g.kind == JI_GEOM_ROUNDED_RECT, "Geometry kind should be ROUNDED_RECT");
    TEST_ASSERT(g.v.rounded_rect.rect.width == 50, "Rounded rect width should match");
}

static void test_geometry_ellipse(void) {
    JiGeometry g = ji_geometry_ellipse(ji_rect(0, 0, 80, 60));
    TEST_ASSERT(g.kind == JI_GEOM_ELLIPSE, "Geometry kind should be ELLIPSE");
    TEST_ASSERT(g.v.ellipse.bounds.width == 80, "Ellipse bounds width should match");
}

static void test_geometry_line(void) {
    JiGeometry g = ji_geometry_line(ji_point(0, 0), ji_point(100, 100));
    TEST_ASSERT(g.kind == JI_GEOM_LINE, "Geometry kind should be LINE");
    TEST_FLOAT_EQ(g.v.line.start.x, 0.0, 1e-9, "Line start x should be 0");
    TEST_FLOAT_EQ(g.v.line.end.x, 100.0, 1e-9, "Line end x should be 100");
}

static void test_drawing_context_init(void) {
    JiDrawingContext ctx;
    ji_drawing_context_init(&ctx);
    TEST_ASSERT(ctx.push_clip == NULL, "push_clip should be NULL after init");
    TEST_ASSERT(ctx.fill_rect == NULL, "fill_rect should be NULL after init");
    TEST_ASSERT(ctx.backend_data == NULL, "backend_data should be NULL after init");
    TEST_ASSERT(ctx.surface_width == 0, "surface_width should be 0 after init");
    TEST_ASSERT(!ji_drawing_context_is_valid(&ctx), "Empty context should not be valid");
}

/* ---- Mock drawing context for render test ---- */
static int s_fill_rect_call_count = 0;
static uint32_t s_last_fill_color = 0;

static void mock_fill_rect(JiDrawingContext* ctx, JiRect rect, const JiBrush* brush) {
    (void)ctx; (void)rect;
    s_fill_rect_call_count++;
    if (brush && brush->kind == JI_BRUSH_SOLID) {
        s_last_fill_color = brush->v.color;
    }
}

static void mock_clear(JiDrawingContext* ctx, uint32_t argb) {
    (void)ctx; (void)argb;
}

static void test_visual_render_with_context(void) {
    /* Create a visual with a custom render callback */
    JiVisual* v = ji_visual_new();
    TEST_ASSERT(v != NULL, "Visual should be created");

    /* Set up a mock drawing context */
    JiDrawingContext ctx;
    ji_drawing_context_init(&ctx);
    ctx.fill_rect = mock_fill_rect;
    ctx.clear = mock_clear;
    ctx.surface_width = 800;
    ctx.surface_height = 600;

    s_fill_rect_call_count = 0;

    /* Render without callback — should not crash */
    ji_visual_render(v, &ctx);
    TEST_ASSERT(s_fill_rect_call_count == 0, "No render callback means no fill_rect calls");

    /* Set a render callback that fills a rect */
    v->on_render = NULL; /* We'll test the render path itself */
    v->is_dirty = true;
    ji_visual_render(v, &ctx);
    TEST_ASSERT(v->is_dirty == false, "Visual should not be dirty after render");

    ji_visual_destroy(v);
}

static void test_visual_render_invisible(void) {
    JiVisual* v = ji_visual_new();
    JiDrawingContext ctx;
    ji_drawing_context_init(&ctx);
    ctx.fill_rect = mock_fill_rect;
    ctx.clear = mock_clear;

    s_fill_rect_call_count = 0;
    ji_visual_set_visibility(v, JI_VISIBILITY_HIDDEN);

    /* Hidden visual should not render */
    ji_visual_render(v, &ctx);
    TEST_ASSERT(s_fill_rect_call_count == 0, "Hidden visual should not render");

    ji_visual_destroy(v);
}

static void test_visual_render_children(void) {
    JiVisual* root = ji_visual_new();
    JiVisual* child = ji_visual_new();
    ji_visual_add_child(root, child);

    JiDrawingContext ctx;
    ji_drawing_context_init(&ctx);
    ctx.fill_rect = mock_fill_rect;
    ctx.clear = mock_clear;

    /* Render root — should also render child */
    ji_visual_render(root, &ctx);
    TEST_ASSERT(root->is_dirty == false, "Root should not be dirty after render");
    TEST_ASSERT(child->is_dirty == false, "Child should not be dirty after render");

    ji_visual_destroy(root);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    JiResultCode rc = ji_initialize();
    if (rc != JI_OK) {
        fprintf(stderr, "Failed to initialize JiUI: %d\n", rc);
        return 1;
    }

    printf("=== Visual System Tests ===\n");

    /* Visual tests */
    printf("--- Visual ---\n");
    test_visual_create_destroy();
    test_visual_opacity();
    test_visual_visibility();
    test_visual_tree();
    test_visual_effective_visibility();
    test_visual_z_index();
    test_visual_clip();
    test_visual_render_transform();
    test_visual_invalidate();
    test_visual_type_id();

    /* Drawing tests */
    printf("--- Drawing ---\n");
    test_brush_solid();
    test_brush_linear_gradient();
    test_brush_radial_gradient();
    test_pen_solid();
    test_pen_dashed();
    test_geometry_rect();
    test_geometry_rounded_rect();
    test_geometry_ellipse();
    test_geometry_line();
    test_drawing_context_init();

    /* Render integration tests */
    printf("--- Render Integration ---\n");
    test_visual_render_with_context();
    test_visual_render_invisible();
    test_visual_render_children();

    printf("\n%d/%d tests passed, %d failed\n",
           s_tests_passed, s_tests_run, s_tests_failed);

    ji_shutdown();
    return s_tests_failed > 0 ? 1 : 0;
}
