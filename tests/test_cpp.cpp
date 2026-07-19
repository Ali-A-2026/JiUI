/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_cpp.cpp
 * @brief Tests for the C++ RAII wrapper API (jiui.hpp).
 */

#include <jiui/jiui.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

/* =========================================================================
 * Test framework (minimal)
 * ========================================================================= */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST(name) static void test_##name()
#define RUN_TEST(name) do { \
    printf("  %-50s", #name); \
    g_tests_run++; \
    test_##name(); \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("FAIL\n    Line %d: %s\n", __LINE__, #cond); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ_INT(a, b) do { \
    int _a = (int)(a), _b = (int)(b); \
    if (_a != _b) { \
        printf("FAIL\n    Line %d: %d != %d\n", __LINE__, _a, _b); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ_DOUBLE(a, b) do { \
    double _a = (double)(a), _b = (double)(b); \
    if (_a != _b) { \
        printf("FAIL\n    Line %d: %f != %f\n", __LINE__, _a, _b); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define PASS() do { \
    printf("PASS\n"); \
    g_tests_passed++; \
} while(0)

/* =========================================================================
 * Tests
 * ========================================================================= */

TEST(library_raii) {
    /* Library is already initialized by the test main — just test version */
    const char* ver = jiui::Library::version();
    ASSERT_TRUE(ver != nullptr);
    ASSERT_TRUE(strlen(ver) > 0);
    PASS();
}

TEST(smart_visual) {
    auto vis = jiui::make_visual();
    ASSERT_TRUE(vis != nullptr);
    PASS();
}

TEST(smart_visual_destroy) {
    {
        auto vis = jiui::make_visual();
        ASSERT_TRUE(vis != nullptr);
        /* vis goes out of scope — should call ji_visual_destroy */
    }
    /* If we get here without crash, the deleter worked */
    PASS();
}

TEST(solid_brush_helper) {
    JiBrush brush = jiui::solid_brush(0xFF112233);
    ASSERT_TRUE(brush.kind == JI_BRUSH_SOLID);
    ASSERT_TRUE(brush.v.color == 0xFF112233);
    PASS();
}

TEST(solid_pen_helper) {
    JiPen pen = jiui::solid_pen(0xFFAABBCC, 2.5);
    ASSERT_TRUE(pen.brush.kind == JI_BRUSH_SOLID);
    ASSERT_TRUE(pen.thickness == 2.5);
    ji_pen_destroy(&pen);
    PASS();
}

TEST(rect_geometry_helper) {
    JiRect rect = ji_rect(10, 20, 100, 200);
    JiGeometry geom = jiui::rect_geometry(rect);
    ASSERT_TRUE(geom.kind == JI_GEOM_RECT);
    ASSERT_EQ_INT(100, (int)geom.v.rect.width);
    ASSERT_EQ_INT(200, (int)geom.v.rect.height);
    PASS();
}

TEST(ellipse_geometry_helper) {
    JiRect bounds = ji_rect(0, 0, 50, 50);
    JiGeometry geom = jiui::ellipse_geometry(bounds);
    ASSERT_TRUE(geom.kind == JI_GEOM_ELLIPSE);
    PASS();
}

TEST(check_result_ok) {
    /* JI_OK should not throw */
    jiui::check_result(JI_OK);
    PASS();
}

TEST(check_result_error) {
    bool threw = false;
    try {
        jiui::check_result(JI_ERROR_NULL_PTR, "test context");
    } catch (const std::runtime_error& e) {
        threw = true;
    }
    ASSERT_TRUE(threw);
    PASS();
}

TEST(theme_helpers) {
    /* Get current theme — should be NULL initially */
    JiTheme* t = jiui::get_theme();
    ASSERT_TRUE(t == nullptr);
    PASS();
}

TEST(default_backend_null) {
    /* No backend registered in test context — should be NULL */
    JiPlatformBackend* b = jiui::default_backend();
    ASSERT_TRUE(b == nullptr);
    PASS();
}

TEST(window_get_time) {
    double t = ji_window_get_time();
    ASSERT_TRUE(t > 0.0);
    PASS();
}

TEST(window_flags_combo) {
    JiWindowFlags flags = static_cast<JiWindowFlags>(
        JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE | JI_WINDOW_VSYNC);
    ASSERT_TRUE((flags & JI_WINDOW_OPENGL) != 0);
    ASSERT_TRUE((flags & JI_WINDOW_RESIZABLE) != 0);
    ASSERT_TRUE((flags & JI_WINDOW_VSYNC) != 0);
    ASSERT_TRUE((flags & JI_WINDOW_FULLSCREEN) == 0);
    PASS();
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    jiui::Library lib;

    printf("=== C++ API Tests ===\n\n");

    RUN_TEST(library_raii);             PASS();
    RUN_TEST(smart_visual);             PASS();
    RUN_TEST(smart_visual_destroy);      PASS();
    RUN_TEST(solid_brush_helper);        PASS();
    RUN_TEST(solid_pen_helper);          PASS();
    RUN_TEST(rect_geometry_helper);      PASS();
    RUN_TEST(ellipse_geometry_helper);   PASS();
    RUN_TEST(check_result_ok);           PASS();
    RUN_TEST(check_result_error);        PASS();
    RUN_TEST(theme_helpers);             PASS();
    RUN_TEST(default_backend_null);      PASS();
    RUN_TEST(window_get_time);           PASS();
    RUN_TEST(window_flags_combo);        PASS();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
