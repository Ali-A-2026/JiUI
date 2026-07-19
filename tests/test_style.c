/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_style.c
 * @brief Tests for the styling & theming system.
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

/* =========================================================================
 * Selector tests
 * ========================================================================= */
static void test_selector_type(void) {
    JiSelector sel = ji_selector_type("JiButton");
    TEST_ASSERT(sel.kind == JI_SELECTOR_TYPE, "Kind should be TYPE");
    TEST_ASSERT(sel.type_name != NULL, "Type name should not be NULL");
    TEST_ASSERT(strcmp(sel.type_name, "JiButton") == 0, "Type name should be JiButton");
    ji_selector_destroy(&sel);
}

static void test_selector_class(void) {
    JiSelector sel = ji_selector_class("primary");
    TEST_ASSERT(sel.kind == JI_SELECTOR_CLASS, "Kind should be CLASS");
    TEST_ASSERT(strcmp(sel.class_name, "primary") == 0, "Class name should be primary");
    ji_selector_destroy(&sel);
}

static void test_selector_name(void) {
    JiSelector sel = ji_selector_name("myButton");
    TEST_ASSERT(sel.kind == JI_SELECTOR_NAME, "Kind should be NAME");
    TEST_ASSERT(strcmp(sel.name, "myButton") == 0, "Name should be myButton");
    ji_selector_destroy(&sel);
}

static void test_selector_pseudo(void) {
    JiSelector sel = ji_selector_pseudo(JI_PSEUDO_FOCUS);
    TEST_ASSERT(sel.kind == JI_SELECTOR_PSEUDO, "Kind should be PSEUDO");
    TEST_ASSERT(sel.pseudo == JI_PSEUDO_FOCUS, "Pseudo should be FOCUS");
    ji_selector_destroy(&sel);
}

static void test_selector_type_class(void) {
    JiSelector sel = ji_selector_type_class("JiButton", "primary");
    TEST_ASSERT(sel.kind == JI_SELECTOR_TYPE_CLASS, "Kind should be TYPE_CLASS");
    TEST_ASSERT(strcmp(sel.type_name, "JiButton") == 0, "Type name should be JiButton");
    TEST_ASSERT(strcmp(sel.class_name, "primary") == 0, "Class name should be primary");
    ji_selector_destroy(&sel);
}

static void test_selector_matches_type(void) {
    JiSelector sel = ji_selector_type("JiButton");
    JiButton* btn = ji_button_new("Test");
    JiControl* c = &btn->control;

    TEST_ASSERT(ji_selector_matches(&sel, c), "Should match JiButton type");

    ji_selector_destroy(&sel);
    ji_button_destroy(btn);
}

static void test_selector_matches_name(void) {
    JiSelector sel = ji_selector_name("myBtn");
    JiButton* btn = ji_button_new("Test");
    ji_control_set_name(&btn->control, "myBtn");

    TEST_ASSERT(ji_selector_matches(&sel, &btn->control), "Should match name myBtn");

    ji_selector_destroy(&sel);
    ji_button_destroy(btn);
}

static void test_selector_no_match(void) {
    JiSelector sel = ji_selector_name("wrongName");
    JiButton* btn = ji_button_new("Test");
    ji_control_set_name(&btn->control, "myBtn");

    TEST_ASSERT(!ji_selector_matches(&sel, &btn->control), "Should not match wrong name");

    ji_selector_destroy(&sel);
    ji_button_destroy(btn);
}

/* =========================================================================
 * Setter tests
 * ========================================================================= */
static void test_setter_double(void) {
    JiSetter s = ji_setter_double("opacity", 0.5);
    TEST_ASSERT(strcmp(s.property_name, "opacity") == 0, "Property name should be opacity");
    TEST_ASSERT(s.value.type == JI_PROP_TYPE_DOUBLE, "Value type should be DOUBLE");
    TEST_ASSERT(fabs(s.value.v.double_val - 0.5) < 1e-9, "Value should be 0.5");
    ji_setter_destroy(&s);
}

static void test_setter_int(void) {
    JiSetter s = ji_setter_int("tab_index", 5);
    TEST_ASSERT(s.value.type == JI_PROP_TYPE_INT, "Value type should be INT");
    TEST_ASSERT(s.value.v.int_val == 5, "Value should be 5");
    ji_setter_destroy(&s);
}

static void test_setter_color(void) {
    JiSetter s = ji_setter_color("background", 0xFFFF0000);
    TEST_ASSERT(s.value.type == JI_PROP_TYPE_COLOR, "Value type should be COLOR");
    TEST_ASSERT(s.value.v.color_val == 0xFFFF0000, "Value should be 0xFFFF0000");
    ji_setter_destroy(&s);
}

static void test_setter_bool(void) {
    JiSetter s = ji_setter_bool("is_enabled", false);
    TEST_ASSERT(s.value.type == JI_PROP_TYPE_BOOL, "Value type should be BOOL");
    TEST_ASSERT(s.value.v.bool_val == false, "Value should be false");
    ji_setter_destroy(&s);
}

static void test_setter_string(void) {
    JiSetter s = ji_setter_string("tooltip", "Hello");
    TEST_ASSERT(s.value.type == JI_PROP_TYPE_STRING, "Value type should be STRING");
    TEST_ASSERT(strcmp(s.value.v.string_val, "Hello") == 0, "Value should be Hello");
    ji_setter_destroy(&s);
}

/* =========================================================================
 * Style tests
 * ========================================================================= */
static void test_style_create_destroy(void) {
    JiStyle* style = ji_style_new(ji_selector_type("JiButton"));
    TEST_ASSERT(style != NULL, "Style should be created");
    TEST_ASSERT(style->setter_count == 0, "New style should have 0 setters");
    ji_style_destroy(style);
}

static void test_style_add_setters(void) {
    JiStyle* style = ji_style_new(ji_selector_type("JiButton"));
    ji_style_add_setter(style, ji_setter_double("opacity", 0.8));
    ji_style_add_setter(style, ji_setter_int("tab_index", 2));
    TEST_ASSERT(ji_style_get_setter_count(style) == 2, "Should have 2 setters");
    ji_style_destroy(style);
}

static void test_style_matches(void) {
    JiStyle* style = ji_style_new(ji_selector_type("JiButton"));
    JiButton* btn = ji_button_new("Test");
    JiLabel* label = ji_label_new("Text");

    TEST_ASSERT(ji_style_matches(style, &btn->control), "Should match JiButton");
    TEST_ASSERT(!ji_style_matches(style, &label->control), "Should not match JiLabel");

    ji_style_destroy(style);
    ji_button_destroy(btn);
    ji_label_destroy(label);
}

static void test_style_apply(void) {
    JiStyle* style = ji_style_new(ji_selector_type("JiButton"));
    ji_style_add_setter(style, ji_setter_double("opacity", 0.5));
    ji_style_add_setter(style, ji_setter_bool("is_enabled", false));
    ji_style_add_setter(style, ji_setter_int("tab_index", 42));

    JiButton* btn = ji_button_new("Test");
    ji_style_apply(style, &btn->control);

    TEST_ASSERT(fabs(ji_visual_get_opacity(&btn->control.visual) - 0.5) < 1e-9,
                "Opacity should be 0.5 after style apply");
    TEST_ASSERT(!ji_control_is_enabled(&btn->control), "Should be disabled after style apply");
    TEST_ASSERT(ji_control_get_tab_index(&btn->control) == 42, "Tab index should be 42");

    ji_style_destroy(style);
    ji_button_destroy(btn);
}

/* =========================================================================
 * ResourceDictionary tests
 * ========================================================================= */
static void test_resource_dict_create_destroy(void) {
    JiResourceDictionary* dict = ji_resource_dict_new();
    TEST_ASSERT(dict != NULL, "Dict should be created");
    TEST_ASSERT(ji_resource_dict_get_count(dict) == 0, "New dict should be empty");
    ji_resource_dict_destroy(dict);
}

static void test_resource_dict_set_get(void) {
    JiResourceDictionary* dict = ji_resource_dict_new();

    ji_resource_dict_set_double(dict, "FontSize", 16.0);
    ji_resource_dict_set_color(dict, "PrimaryColor", 0xFF0066CC);
    ji_resource_dict_set_string(dict, "FontFamily", "Segoe UI");

    TEST_ASSERT(ji_resource_dict_get_count(dict) == 3, "Should have 3 entries");

    JiPropertyValue val;
    TEST_ASSERT(ji_resource_dict_try_get(dict, "FontSize", &val), "Should find FontSize");
    TEST_ASSERT(val.type == JI_PROP_TYPE_DOUBLE, "FontSize should be DOUBLE");
    TEST_ASSERT(fabs(val.v.double_val - 16.0) < 1e-9, "FontSize should be 16.0");

    TEST_ASSERT(ji_resource_dict_try_get(dict, "PrimaryColor", &val), "Should find PrimaryColor");
    TEST_ASSERT(val.type == JI_PROP_TYPE_COLOR, "PrimaryColor should be COLOR");
    TEST_ASSERT(val.v.color_val == 0xFF0066CC, "PrimaryColor should be 0xFF0066CC");

    TEST_ASSERT(ji_resource_dict_try_get(dict, "FontFamily", &val), "Should find FontFamily");
    TEST_ASSERT(val.type == JI_PROP_TYPE_STRING, "FontFamily should be STRING");
    TEST_ASSERT(strcmp(val.v.string_val, "Segoe UI") == 0, "FontFamily should be Segoe UI");

    TEST_ASSERT(!ji_resource_dict_try_get(dict, "NonExistent", &val), "Should not find NonExistent");

    ji_resource_dict_destroy(dict);
}

static void test_resource_dict_update(void) {
    JiResourceDictionary* dict = ji_resource_dict_new();

    ji_resource_dict_set_double(dict, "FontSize", 16.0);
    TEST_ASSERT(ji_resource_dict_get_count(dict) == 1, "Should have 1 entry");

    /* Update existing key */
    ji_resource_dict_set_double(dict, "FontSize", 24.0);
    TEST_ASSERT(ji_resource_dict_get_count(dict) == 1, "Should still have 1 entry after update");

    JiPropertyValue val;
    ji_resource_dict_try_get(dict, "FontSize", &val);
    TEST_ASSERT(fabs(val.v.double_val - 24.0) < 1e-9, "FontSize should be updated to 24.0");

    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Theme tests
 * ========================================================================= */
static void test_theme_create_destroy(void) {
    JiTheme* theme = ji_theme_new("DarkTheme");
    TEST_ASSERT(theme != NULL, "Theme should be created");
    TEST_ASSERT(strcmp(theme->name, "DarkTheme") == 0, "Theme name should be DarkTheme");
    TEST_ASSERT(ji_theme_get_style_count(theme) == 0, "New theme should have 0 styles");
    TEST_ASSERT(ji_theme_get_resources(theme) != NULL, "Theme should have a resource dict");
    ji_theme_destroy(theme);
}

static void test_theme_add_styles(void) {
    JiTheme* theme = ji_theme_new("TestTheme");

    JiStyle* s1 = ji_style_new(ji_selector_type("JiButton"));
    ji_style_add_setter(s1, ji_setter_double("opacity", 0.9));
    ji_theme_add_style(theme, s1);

    JiStyle* s2 = ji_style_new(ji_selector_type("JiLabel"));
    ji_style_add_setter(s2, ji_setter_int("tab_index", 1));
    ji_theme_add_style(theme, s2);

    TEST_ASSERT(ji_theme_get_style_count(theme) == 2, "Should have 2 styles");
    TEST_ASSERT(ji_theme_get_style(theme, 0) == s1, "First style should be s1");
    TEST_ASSERT(ji_theme_get_style(theme, 1) == s2, "Second style should be s2");

    ji_theme_destroy(theme);
}

static void test_theme_resources(void) {
    JiTheme* theme = ji_theme_new("DarkTheme");
    JiResourceDictionary* res = ji_theme_get_resources(theme);

    ji_resource_dict_set_color(res, "BackgroundColor", 0xFF1E1E1E);
    ji_resource_dict_set_color(res, "ForegroundColor", 0xFFD4D4D4);
    ji_resource_dict_set_double(res, "FontSize", 14.0);

    TEST_ASSERT(ji_resource_dict_get_count(res) == 3, "Should have 3 resources");

    JiPropertyValue val;
    ji_resource_dict_try_get(res, "BackgroundColor", &val);
    TEST_ASSERT(val.v.color_val == 0xFF1E1E1E, "Background should be dark");

    ji_theme_destroy(theme);
}

/* =========================================================================
 * Theme manager tests
 * ========================================================================= */
static void test_theme_manager(void) {
    TEST_ASSERT(ji_get_current_theme() == NULL, "Default theme should be NULL");

    JiTheme* theme = ji_theme_new("TestTheme");
    JiStyle* style = ji_style_new(ji_selector_type("JiButton"));
    ji_style_add_setter(style, ji_setter_double("opacity", 0.7));
    ji_theme_add_style(theme, style);

    ji_set_current_theme(theme);
    TEST_ASSERT(ji_get_current_theme() == theme, "Current theme should be set");

    /* Apply theme to a button */
    JiButton* btn = ji_button_new("Test");
    ji_apply_theme_to_control(&btn->control);
    TEST_ASSERT(fabs(ji_visual_get_opacity(&btn->control.visual) - 0.7) < 1e-9,
                "Button opacity should be 0.7 after theme apply");

    ji_button_destroy(btn);
    /* Setting a new theme destroys the old one */
    ji_set_current_theme(NULL);
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

    printf("=== Styling & Theming Tests ===\n");

    /* Selectors */
    printf("--- Selectors ---\n");
    test_selector_type();
    test_selector_class();
    test_selector_name();
    test_selector_pseudo();
    test_selector_type_class();
    test_selector_matches_type();
    test_selector_matches_name();
    test_selector_no_match();

    /* Setters */
    printf("--- Setters ---\n");
    test_setter_double();
    test_setter_int();
    test_setter_color();
    test_setter_bool();
    test_setter_string();

    /* Styles */
    printf("--- Styles ---\n");
    test_style_create_destroy();
    test_style_add_setters();
    test_style_matches();
    test_style_apply();

    /* ResourceDictionary */
    printf("--- ResourceDictionary ---\n");
    test_resource_dict_create_destroy();
    test_resource_dict_set_get();
    test_resource_dict_update();

    /* Themes */
    printf("--- Themes ---\n");
    test_theme_create_destroy();
    test_theme_add_styles();
    test_theme_resources();
    test_theme_manager();

    printf("\n%d/%d tests passed, %d failed\n",
           s_tests_passed, s_tests_run, s_tests_failed);

    ji_shutdown();
    return s_tests_failed > 0 ? 1 : 0;
}
