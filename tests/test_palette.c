/**
 * JiUI - Palette & Theme Engine Tests
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define BEGIN_TEST(name) printf("  TEST: %s ... ", name);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

/* ---- Palette creation / destruction ---- */
static void test_palette_new_destroy(void) {
    BEGIN_TEST("palette_new_destroy");
    JiPalette* pal = ji_palette_new();
    if (!pal) { FAIL("ji_palette_new returned NULL"); return; }
    ji_palette_destroy(pal);
    PASS();
}

/* ---- Palette set/get color ---- */
static void test_palette_set_get_color(void) {
    BEGIN_TEST("palette_set_get_color");
    JiPalette* pal = ji_palette_new();
    if (!pal) { FAIL("ji_palette_new returned NULL"); return; }

    ji_palette_set_color(pal, JI_COLOR_GROUP_ACTIVE, JI_ROLE_WINDOW, 0xFF1E1E1E);
    uint32_t c = ji_palette_get_color(pal, JI_COLOR_GROUP_ACTIVE, JI_ROLE_WINDOW);
    if (c != 0xFF1E1E1E) { printf("FAIL: color mismatch (got 0x%08X)\n", c); tests_failed++; ji_palette_destroy(pal); return; }

    /* Test fallback to Active group */
    uint32_t c2 = ji_palette_get_color(pal, JI_COLOR_GROUP_INACTIVE, JI_ROLE_WINDOW);
    if (c2 != 0xFF1E1E1E) { FAIL("inactive group should fall back to active"); ji_palette_destroy(pal); return; }

    ji_palette_destroy(pal);
    PASS();
}

/* ---- Palette set_color_all ---- */
static void test_palette_set_color_all(void) {
    BEGIN_TEST("palette_set_color_all");
    JiPalette* pal = ji_palette_new();
    if (!pal) { FAIL("ji_palette_new returned NULL"); return; }

    ji_palette_set_color_all(pal, JI_ROLE_HIGHLIGHT, 0xFF0078D4);
    if (ji_palette_get_color(pal, JI_COLOR_GROUP_ACTIVE, JI_ROLE_HIGHLIGHT) != 0xFF0078D4) { FAIL("active mismatch"); ji_palette_destroy(pal); return; }
    if (ji_palette_get_color(pal, JI_COLOR_GROUP_DISABLED, JI_ROLE_HIGHLIGHT) != 0xFF0078D4) { FAIL("disabled mismatch"); ji_palette_destroy(pal); return; }
    if (ji_palette_get_color(pal, JI_COLOR_GROUP_INACTIVE, JI_ROLE_HIGHLIGHT) != 0xFF0078D4) { FAIL("inactive mismatch"); ji_palette_destroy(pal); return; }

    ji_palette_destroy(pal);
    PASS();
}

/* ---- Palette copy ---- */
static void test_palette_copy(void) {
    BEGIN_TEST("palette_copy");
    JiPalette* pal = ji_palette_new();
    if (!pal) { FAIL("ji_palette_new returned NULL"); return; }

    ji_palette_set_color(pal, JI_COLOR_GROUP_ACTIVE, JI_ROLE_WINDOW, 0xFF1E1E1E);
    JiPalette* copy = ji_palette_copy(pal);
    if (!copy) { FAIL("ji_palette_copy returned NULL"); ji_palette_destroy(pal); return; }

    uint32_t orig = ji_palette_get_color(pal, JI_COLOR_GROUP_ACTIVE, JI_ROLE_WINDOW);
    uint32_t cp = ji_palette_get_color(copy, JI_COLOR_GROUP_ACTIVE, JI_ROLE_WINDOW);
    if (orig != cp) { FAIL("copy color mismatch"); ji_palette_destroy(pal); ji_palette_destroy(copy); return; }

    ji_palette_destroy(pal);
    ji_palette_destroy(copy);
    PASS();
}

/* ---- Qt6 Dark Palette ---- */
static void test_qt_palette_dark(void) {
    BEGIN_TEST("qt_palette_dark");
    JiPalette* pal = ji_qt_palette_dark();
    if (!pal) { FAIL("ji_qt_palette_dark returned NULL"); return; }

    uint32_t window = ji_palette_color(pal, JI_ROLE_WINDOW);
    if (window != 0xFF1E1E1E) { printf("FAIL: dark window color mismatch (got 0x%08X)\n", window); tests_failed++; ji_palette_destroy(pal); return; }

    uint32_t base = ji_palette_color(pal, JI_ROLE_BASE);
    if (base != 0xFF2D2D2D) { FAIL("dark base color mismatch"); ji_palette_destroy(pal); return; }

    uint32_t highlight = ji_palette_color(pal, JI_ROLE_HIGHLIGHT);
    if (highlight != 0xFF2D5F8F) { FAIL("dark highlight color mismatch"); ji_palette_destroy(pal); return; }

    /* Verify disabled group is populated */
    uint32_t disabled_text = ji_palette_get_color(pal, JI_COLOR_GROUP_DISABLED, JI_ROLE_WINDOW_TEXT);
    if (disabled_text == 0) { FAIL("disabled window_text should be non-zero"); ji_palette_destroy(pal); return; }

    ji_palette_destroy(pal);
    PASS();
}

/* ---- Qt6 Light Palette ---- */
static void test_qt_palette_light(void) {
    BEGIN_TEST("qt_palette_light");
    JiPalette* pal = ji_qt_palette_light();
    if (!pal) { FAIL("ji_qt_palette_light returned NULL"); return; }

    uint32_t window = ji_palette_color(pal, JI_ROLE_WINDOW);
    if (window != 0xFFF0F0F0) { printf("FAIL: light window color mismatch (got 0x%08X)\n", window); tests_failed++; ji_palette_destroy(pal); return; }

    uint32_t base = ji_palette_color(pal, JI_ROLE_BASE);
    if (base != 0xFFFFFFFF) { FAIL("light base color mismatch"); ji_palette_destroy(pal); return; }

    uint32_t highlight = ji_palette_color(pal, JI_ROLE_HIGHLIGHT);
    if (highlight != 0xFF0078D4) { FAIL("light highlight color mismatch"); ji_palette_destroy(pal); return; }

    ji_palette_destroy(pal);
    PASS();
}

/* ---- Theme Engine ---- */
static void test_theme_engine_lifecycle(void) {
    BEGIN_TEST("theme_engine_lifecycle");
    JiThemeEngine* engine = ji_theme_engine_new();
    if (!engine) { FAIL("ji_theme_engine_new returned NULL"); return; }

    if (ji_theme_engine_get_variant(engine) != JI_THEME_SYSTEM) { FAIL("default variant should be SYSTEM"); ji_theme_engine_destroy(engine); return; }

    const JiPalette* pal = ji_theme_engine_get_palette(engine);
    if (!pal) { FAIL("palette should not be NULL"); ji_theme_engine_destroy(engine); return; }

    ji_theme_engine_destroy(engine);
    PASS();
}

/* ---- Theme Engine variant switching ---- */
static void test_theme_engine_variant_switch(void) {
    BEGIN_TEST("theme_engine_variant_switch");
    JiThemeEngine* engine = ji_theme_engine_new();
    if (!engine) { FAIL("ji_theme_engine_new returned NULL"); return; }

    /* Switch to dark */
    ji_theme_engine_set_variant(engine, JI_THEME_DARK);
    if (ji_theme_engine_get_variant(engine) != JI_THEME_DARK) { FAIL("variant should be DARK"); ji_theme_engine_destroy(engine); return; }
    const JiPalette* dark_pal = ji_theme_engine_get_palette(engine);
    if (!dark_pal) { FAIL("dark palette should not be NULL"); ji_theme_engine_destroy(engine); return; }
    uint32_t dark_window = ji_palette_color(dark_pal, JI_ROLE_WINDOW);
    if (dark_window != 0xFF1E1E1E) { FAIL("dark window color mismatch after switch"); ji_theme_engine_destroy(engine); return; }

    /* Switch to light */
    ji_theme_engine_set_variant(engine, JI_THEME_LIGHT);
    if (ji_theme_engine_get_variant(engine) != JI_THEME_LIGHT) { FAIL("variant should be LIGHT"); ji_theme_engine_destroy(engine); return; }
    const JiPalette* light_pal = ji_theme_engine_get_palette(engine);
    if (!light_pal) { FAIL("light palette should not be NULL"); ji_theme_engine_destroy(engine); return; }
    uint32_t light_window = ji_palette_color(light_pal, JI_ROLE_WINDOW);
    if (light_window != 0xFFF0F0F0) { FAIL("light window color mismatch after switch"); ji_theme_engine_destroy(engine); return; }

    ji_theme_engine_destroy(engine);
    PASS();
}

/* ---- Qt Style pixel metrics ---- */
static void test_qt_style_pixel_metrics(void) {
    BEGIN_TEST("qt_style_pixel_metrics");
    if (ji_qt_style_pixel_metric(JI_PM_CHECKBOX_SIZE) != 13) { FAIL("checkbox size should be 13"); return; }
    if (ji_qt_style_pixel_metric(JI_PM_SLIDER_THICKNESS) != 16) { FAIL("slider thickness should be 16"); return; }
    if (ji_qt_style_pixel_metric(JI_PM_PROGRESSBAR_THICKNESS) != 4) { FAIL("progressbar thickness should be 4"); return; }
    if (ji_qt_style_pixel_metric(JI_PM_SPLITTER_WIDTH) != 4) { FAIL("splitter width should be 4"); return; }
    if (ji_qt_style_pixel_metric(JI_PM_TAB_BAR_HEIGHT) != 24) { FAIL("tab bar height should be 24"); return; }
    PASS();
}

/* ---- Qt Style creation ---- */
static void test_qt_style_create(void) {
    BEGIN_TEST("qt_style_create");
    JiPalette* pal = ji_qt_palette_dark();
    if (!pal) { FAIL("palette should not be NULL"); return; }
    JiQtStyle* style = ji_qt_style_new(pal);
    if (!style) { FAIL("ji_qt_style_new returned NULL"); ji_palette_destroy(pal); return; }
    ji_qt_style_destroy(style);
    ji_palette_destroy(pal);
    PASS();
}

/* ---- Color resolve with state ---- */
static void test_qt_style_resolve_color(void) {
    BEGIN_TEST("qt_style_resolve_color");
    JiPalette* pal = ji_qt_palette_dark();
    if (!pal) { FAIL("palette should not be NULL"); return; }
    JiQtStyle* style = ji_qt_style_new(pal);
    if (!style) { FAIL("style should not be NULL"); ji_palette_destroy(pal); return; }

    /* Normal state should return active group color */
    uint32_t normal = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, JI_STATE_NORMAL);
    if (normal != 0xFF1E1E1E) { FAIL("normal window color mismatch"); ji_qt_style_destroy(style); ji_palette_destroy(pal); return; }

    /* Disabled state should return disabled group color */
    uint32_t disabled = ji_qt_style_resolve_color(style, JI_ROLE_WINDOW, JI_STATE_DISABLED);
    if (disabled == 0) { FAIL("disabled color should be non-zero"); ji_qt_style_destroy(style); ji_palette_destroy(pal); return; }
    if (disabled == normal) { FAIL("disabled color should differ from normal"); ji_qt_style_destroy(style); ji_palette_destroy(pal); return; }

    ji_qt_style_destroy(style);
    ji_palette_destroy(pal);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Palette & Theme Engine Tests ===\n\n");

    test_palette_new_destroy();
    test_palette_set_get_color();
    test_palette_set_color_all();
    test_palette_copy();
    test_qt_palette_dark();
    test_qt_palette_light();
    test_theme_engine_lifecycle();
    test_theme_engine_variant_switch();
    test_qt_style_pixel_metrics();
    test_qt_style_create();
    test_qt_style_resolve_color();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
