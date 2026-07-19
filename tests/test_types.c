/**
 * JiUI - Core Types Unit Tests
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT(expr, msg) do { \
    g_tests_run++; \
    if (expr) { g_tests_passed++; } \
    else { g_tests_failed++; fprintf(stderr, "FAIL: %s (line %d): %s\n", __func__, __LINE__, msg); } \
} while(0)

#define ASSERT_EQ_DBL(a, b, msg) ASSERT(fabs((a) - (b)) < 1e-9, msg)

/* ---- JiPoint ---- */
static void test_point_basic(void) {
    JiPoint p = ji_point(3.0, 4.0);
    ASSERT_EQ_DBL(p.x, 3.0, "point.x");
    ASSERT_EQ_DBL(p.y, 4.0, "point.y");

    JiPoint z = ji_point_zero();
    ASSERT(ji_point_equals(z, ji_point(0.0, 0.0)), "point zero");
}

static void test_point_add_subtract(void) {
    JiPoint a = ji_point(1.0, 2.0);
    JiPoint b = ji_point(3.0, 4.0);
    JiPoint result = ji_point_add(a, b);
    ASSERT(ji_point_equals(result, ji_point(4.0, 6.0)), "point + point");

    JiVector v = ji_vector(3.0, 4.0);
    JiPoint result2 = ji_point_add_vector(a, v);
    ASSERT(ji_point_equals(result2, ji_point(4.0, 6.0)), "point + vector");

    JiPoint c = ji_point(4.0, 6.0);
    JiVector diff = ji_point_subtract(c, a);
    ASSERT(ji_vector_equals(diff, ji_vector(3.0, 4.0)), "point - point => vector");
}

/* ---- JiSize ---- */
static void test_size_basic(void) {
    JiSize s = ji_size(100.0, 200.0);
    ASSERT_EQ_DBL(s.width, 100.0, "size.width");
    ASSERT_EQ_DBL(s.height, 200.0, "size.height");

    ASSERT(!ji_size_is_empty(s), "size not empty");
    ASSERT(!ji_size_is_infinite(s), "size not infinite");

    JiSize z = ji_size_zero();
    ASSERT(ji_size_is_empty(z), "size zero is empty");

    JiSize inf = ji_size_infinite();
    ASSERT(ji_size_is_infinite(inf), "size infinite");
}

static void test_size_operations(void) {
    JiSize a = ji_size(100.0, 200.0);
    JiSize b = ji_size(50.0, 300.0);
    JiSize sum = ji_size_add(a, b);
    ASSERT(ji_size_equals(sum, ji_size(150.0, 500.0)), "size add");

    JiSize mx = ji_size_max(a, b);
    ASSERT(ji_size_equals(mx, ji_size(100.0, 300.0)), "size max");
}

/* ---- JiRect ---- */
static void test_rect_basic(void) {
    JiRect r = ji_rect(10.0, 20.0, 100.0, 200.0);
    ASSERT_EQ_DBL(r.x, 10.0, "rect.x");
    ASSERT_EQ_DBL(r.width, 100.0, "rect.width");
    ASSERT(!ji_rect_is_empty(r), "rect not empty");

    JiRect z = ji_rect_zero();
    ASSERT(ji_rect_is_empty(z), "rect zero is empty");
}

static void test_rect_contains(void) {
    JiRect r = ji_rect(0.0, 0.0, 100.0, 100.0);
    ASSERT(ji_rect_contains_point(r, ji_point(50.0, 50.0)), "contains center");
    ASSERT(!ji_rect_contains_point(r, ji_point(200.0, 200.0)), "not contains outside");
}

static void test_rect_intersect(void) {
    JiRect a = ji_rect(0.0, 0.0, 100.0, 100.0);
    JiRect b = ji_rect(50.0, 50.0, 100.0, 100.0);
    ASSERT(ji_rect_intersects(a, b), "rects intersect");

    JiRect isect = ji_rect_intersect(a, b);
    ASSERT(ji_rect_equals(isect, ji_rect(50.0, 50.0, 50.0, 50.0)), "intersection rect");
}

static void test_rect_inflate_deflate(void) {
    JiRect r = ji_rect(10.0, 10.0, 80.0, 80.0);
    JiThickness t = ji_thickness_all(5.0, 5.0, 5.0, 5.0);
    JiRect inflated = ji_rect_inflate(r, t);
    ASSERT(ji_rect_equals(inflated, ji_rect(5.0, 5.0, 90.0, 90.0)), "inflate");

    JiRect deflated = ji_rect_deflate(r, t);
    ASSERT(ji_rect_equals(deflated, ji_rect(15.0, 15.0, 70.0, 70.0)), "deflate");
}

/* ---- JiVector ---- */
static void test_vector_basic(void) {
    JiVector v = ji_vector(3.0, 4.0);
    ASSERT_EQ_DBL(ji_vector_length(v), 5.0, "vector length 3-4-5");
    ASSERT_EQ_DBL(ji_vector_length_squared(v), 25.0, "vector length squared");

    JiVector n = ji_vector_normalize(v);
    ASSERT_EQ_DBL(ji_vector_length(n), 1.0, "normalized length = 1");
}

static void test_vector_dot_cross(void) {
    JiVector a = ji_vector(1.0, 0.0);
    JiVector b = ji_vector(0.0, 1.0);
    ASSERT_EQ_DBL(ji_vector_dot(a, b), 0.0, "perpendicular dot = 0");
    ASSERT_EQ_DBL(ji_vector_cross(a, b), 1.0, "cross product");
}

/* ---- JiThickness ---- */
static void test_thickness(void) {
    JiThickness t = ji_thickness_all(1.0, 2.0, 3.0, 4.0);
    ASSERT_EQ_DBL(ji_thickness_horizontal(t), 4.0, "horizontal thickness");
    ASSERT_EQ_DBL(ji_thickness_vertical(t), 6.0, "vertical thickness");
    ASSERT(!ji_thickness_is_uniform(t), "not uniform");

    JiThickness u = ji_thickness(5.0);
    ASSERT(ji_thickness_is_uniform(u), "uniform thickness");
}

/* ---- JiMatrix ---- */
static void test_matrix_identity(void) {
    JiMatrix id = ji_matrix_identity();
    ASSERT(ji_matrix_is_identity(id), "identity check");

    JiPoint p = ji_point(5.0, 10.0);
    JiPoint transformed = ji_matrix_transform_point(id, p);
    ASSERT(ji_point_equals(transformed, p), "identity transform");
}

static void test_matrix_translation(void) {
    JiMatrix t = ji_matrix_translation(10.0, 20.0);
    JiPoint p = ji_point(5.0, 5.0);
    JiPoint result = ji_matrix_transform_point(t, p);
    ASSERT(ji_point_equals(result, ji_point(15.0, 25.0)), "translation");
}

static void test_matrix_multiply(void) {
    JiMatrix a = ji_matrix_translation(10.0, 0.0);
    JiMatrix b = ji_matrix_translation(0.0, 20.0);
    JiMatrix combined = ji_matrix_multiply(a, b);
    JiPoint p = ji_point(0.0, 0.0);
    JiPoint result = ji_matrix_transform_point(combined, p);
    ASSERT(ji_point_equals(result, ji_point(10.0, 20.0)), "combined translation");
}

static void test_matrix_invert(void) {
    JiMatrix m = ji_matrix_translation(10.0, 20.0);
    JiMatrix inv;
    ASSERT(ji_matrix_try_invert(m, &inv), "invertible");

    JiPoint p = ji_point(15.0, 25.0);
    JiPoint back = ji_matrix_transform_point(inv, p);
    ASSERT(ji_point_equals(back, ji_point(5.0, 5.0)), "inverse translation");
}

/* ---- JiGridLength ---- */
static void test_grid_length(void) {
    JiGridLength auto_gl = ji_grid_length_auto();
    ASSERT(auto_gl.unit_type == JI_GRID_UNIT_AUTO, "auto type");

    JiGridLength px = ji_grid_length_pixel(100.0);
    ASSERT(px.unit_type == JI_GRID_UNIT_PIXEL && px.value == 100.0, "pixel type");

    JiGridLength star = ji_grid_length_star(2.0);
    ASSERT(star.unit_type == JI_GRID_UNIT_STAR && star.value == 2.0, "star type");
}

/* ---- Main ---- */
int main(void) {
    ji_initialize();

    test_point_basic();
    test_point_add_subtract();
    test_size_basic();
    test_size_operations();
    test_rect_basic();
    test_rect_contains();
    test_rect_intersect();
    test_rect_inflate_deflate();
    test_vector_basic();
    test_vector_dot_cross();
    test_thickness();
    test_matrix_identity();
    test_matrix_translation();
    test_matrix_multiply();
    test_matrix_invert();
    test_grid_length();

    ji_shutdown();

    printf("\n=== Test Results ===\n");
    printf("Total: %d  Passed: %d  Failed: %d\n",
           g_tests_run, g_tests_passed, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
