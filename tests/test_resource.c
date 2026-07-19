/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_resource.c
 * @brief Tests for the resource dictionary system (in ji_style.h).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/ji_style.h>

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

#define TEST_ASSERT_STR(actual, expected, msg) do {                       \
    g_tests_run++;                                                        \
    if ((actual) && (expected) && strcmp((actual), (expected)) == 0) {   \
        g_tests_passed++;                                                \
    } else {                                                              \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected \"%s\", got \"%s\")\n", \
                __FILE__, __LINE__, msg, (expected), (actual) ? (actual) : "(null)"); \
    }                                                                     \
} while(0)

#define TEST_ASSERT_EQ(actual, expected, msg) do {                       \
    g_tests_run++;                                                        \
    if ((actual) == (expected)) {                                         \
        g_tests_passed++;                                                \
    } else {                                                              \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %d, got %d)\n",    \
                __FILE__, __LINE__, msg, (int)(expected), (int)(actual));  \
    }                                                                     \
} while(0)

/* =========================================================================
 * Test: create and destroy
 * ========================================================================= */
static void test_create_destroy(void) {
    printf("  Resource: create and destroy...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    TEST_ASSERT(dict != NULL, "Dict should not be NULL");
    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 0, "Should be empty");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: set and get string
 * ========================================================================= */
static void test_set_get_string(void) {
    printf("  Resource: set/get string...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_string(dict, "Title", "My App");
    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 1, "Should have 1 entry");
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "Title", &val);
    TEST_ASSERT(found, "Should find Title");
    TEST_ASSERT_EQ(val.type, JI_PROP_TYPE_STRING, "Should be string type");
    TEST_ASSERT_STR(val.v.string_val, "My App", "String value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: set and get int
 * ========================================================================= */
static void test_set_get_int(void) {
    printf("  Resource: set/get int...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_int(dict, "Width", 800);
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "Width", &val);
    TEST_ASSERT(found, "Should find Width");
    TEST_ASSERT_EQ(val.type, JI_PROP_TYPE_INT, "Should be int type");
    TEST_ASSERT_EQ(val.v.int_val, 800, "Int value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: set and get double
 * ========================================================================= */
static void test_set_get_double(void) {
    printf("  Resource: set/get double...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_double(dict, "Opacity", 0.75);
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "Opacity", &val);
    TEST_ASSERT(found, "Should find Opacity");
    TEST_ASSERT_EQ(val.type, JI_PROP_TYPE_DOUBLE, "Should be double type");
    TEST_ASSERT(val.v.double_val > 0.74 && val.v.double_val < 0.76, "Double value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: set and get bool
 * ========================================================================= */
static void test_set_get_bool(void) {
    printf("  Resource: set/get bool...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_bool(dict, "IsVisible", true);
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "IsVisible", &val);
    TEST_ASSERT(found, "Should find IsVisible");
    TEST_ASSERT_EQ(val.type, JI_PROP_TYPE_BOOL, "Should be bool type");
    TEST_ASSERT_EQ(val.v.bool_val, true, "Bool value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: set and get color
 * ========================================================================= */
static void test_set_get_color(void) {
    printf("  Resource: set/get color...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_color(dict, "PrimaryColor", 0xFF573399);
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "PrimaryColor", &val);
    TEST_ASSERT(found, "Should find PrimaryColor");
    TEST_ASSERT_EQ(val.type, JI_PROP_TYPE_COLOR, "Should be color type");
    TEST_ASSERT_EQ(val.v.color_val, 0xFF573399u, "Color value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: update existing key
 * ========================================================================= */
static void test_update_key(void) {
    printf("  Resource: update existing key...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_string(dict, "Title", "Old");
    ji_resource_dict_set_string(dict, "Title", "New");
    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 1, "Should still have 1 entry");
    JiPropertyValue val;
    ji_resource_dict_try_get(dict, "Title", &val);
    TEST_ASSERT_STR(val.v.string_val, "New", "Should be updated value");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: get non-existent key
 * ========================================================================= */
static void test_get_nonexistent(void) {
    printf("  Resource: get non-existent key...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    JiPropertyValue val;
    bool found = ji_resource_dict_try_get(dict, "Missing", &val);
    TEST_ASSERT(!found, "Should not find missing key");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: has key
 * ========================================================================= */
static void test_has_key(void) {
    printf("  Resource: has key...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_string(dict, "Title", "My App");
    TEST_ASSERT(ji_resource_dict_has(dict, "Title"), "Should have Title");
    TEST_ASSERT(!ji_resource_dict_has(dict, "Missing"), "Should not have Missing");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: remove key
 * ========================================================================= */
static void test_remove_key(void) {
    printf("  Resource: remove key...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_string(dict, "Title", "My App");
    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 1, "Should have 1 entry");
    bool removed = ji_resource_dict_remove(dict, "Title");
    TEST_ASSERT(removed, "Should return true for removed key");
    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 0, "Should be empty after remove");
    TEST_ASSERT(!ji_resource_dict_has(dict, "Title"), "Should not have Title after remove");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: remove non-existent key
 * ========================================================================= */
static void test_remove_nonexistent(void) {
    printf("  Resource: remove non-existent key...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    bool removed = ji_resource_dict_remove(dict, "Missing");
    TEST_ASSERT(!removed, "Should return false for missing key");
    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Test: merge dictionaries
 * ========================================================================= */
static void test_merge_dicts(void) {
    printf("  Resource: merge dictionaries...\n");
    JiResourceDictionary* base = ji_resource_dict_new();
    JiResourceDictionary* theme = ji_resource_dict_new();

    ji_resource_dict_set_string(base, "Title", "My App");
    ji_resource_dict_set_string(theme, "PrimaryColor", "Blue");
    ji_resource_dict_set_string(theme, "Title", "Theme Override");

    ji_resource_dict_merge(base, theme);

    /* Local entry should take precedence (merge only adds missing keys) */
    JiPropertyValue val;
    ji_resource_dict_try_get(base, "Title", &val);
    TEST_ASSERT_STR(val.v.string_val, "My App", "Local should win over merged");

    /* Merged entry should be found */
    bool found = ji_resource_dict_try_get(base, "PrimaryColor", &val);
    TEST_ASSERT(found, "Should find PrimaryColor from merged");
    TEST_ASSERT_STR(val.v.string_val, "Blue", "Merged value");

    ji_resource_dict_destroy(base);
    ji_resource_dict_destroy(theme);
}

/* =========================================================================
 * Test: multiple types
 * ========================================================================= */
static void test_multiple_types(void) {
    printf("  Resource: multiple types...\n");
    JiResourceDictionary* dict = ji_resource_dict_new();
    ji_resource_dict_set_string(dict, "Title", "My App");
    ji_resource_dict_set_int(dict, "Width", 800);
    ji_resource_dict_set_double(dict, "Opacity", 0.5);
    ji_resource_dict_set_bool(dict, "IsVisible", true);
    ji_resource_dict_set_color(dict, "BgColor", 0xFFFF0000);

    TEST_ASSERT_EQ(ji_resource_dict_get_count(dict), 5, "Should have 5 entries");

    JiPropertyValue s;
    ji_resource_dict_try_get(dict, "Title", &s);
    TEST_ASSERT_EQ(s.type, JI_PROP_TYPE_STRING, "Title type");

    JiPropertyValue i;
    ji_resource_dict_try_get(dict, "Width", &i);
    TEST_ASSERT_EQ(i.type, JI_PROP_TYPE_INT, "Width type");

    JiPropertyValue d;
    ji_resource_dict_try_get(dict, "Opacity", &d);
    TEST_ASSERT_EQ(d.type, JI_PROP_TYPE_DOUBLE, "Opacity type");

    JiPropertyValue b;
    ji_resource_dict_try_get(dict, "IsVisible", &b);
    TEST_ASSERT_EQ(b.type, JI_PROP_TYPE_BOOL, "IsVisible type");

    JiPropertyValue c;
    ji_resource_dict_try_get(dict, "BgColor", &c);
    TEST_ASSERT_EQ(c.type, JI_PROP_TYPE_COLOR, "BgColor type");

    ji_resource_dict_destroy(dict);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    printf("=== Resource Dictionary Tests ===\n");

    test_create_destroy();
    test_set_get_string();
    test_set_get_int();
    test_set_get_double();
    test_set_get_bool();
    test_set_get_color();
    test_update_key();
    test_get_nonexistent();
    test_has_key();
    test_remove_key();
    test_remove_nonexistent();
    test_merge_dicts();
    test_multiple_types();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
