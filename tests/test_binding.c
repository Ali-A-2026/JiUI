/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_binding.c
 * @brief Tests for the data binding engine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/jiui.h>
#include <jiui/ji_binding.h>

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
        g_tests_passed++;                                                \
    } else {                                                              \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %d, got %d)\n",    \
                __FILE__, __LINE__, msg, (int)(expected), (int)(actual));  \
    }                                                                     \
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

/* =========================================================================
 * Test: binding mode string conversion
 * ========================================================================= */
static void test_binding_mode_str(void) {
    printf("  Binding: mode string conversion...\n");
    TEST_ASSERT_STR(ji_binding_mode_str(JI_BINDING_ONE_WAY), "OneWay", "OneWay str");
    TEST_ASSERT_STR(ji_binding_mode_str(JI_BINDING_TWO_WAY), "TwoWay", "TwoWay str");
    TEST_ASSERT_STR(ji_binding_mode_str(JI_BINDING_ONE_TIME), "OneTime", "OneTime str");
    TEST_ASSERT_STR(ji_binding_mode_str(JI_BINDING_ONE_WAY_TO_SOURCE), "OneWayToSource", "OneWayToSource str");
}

/* =========================================================================
 * Test: binding mode from string
 * ========================================================================= */
static void test_binding_mode_from_str(void) {
    printf("  Binding: mode from string...\n");
    TEST_ASSERT_EQ(ji_binding_mode_from_str("OneWay"), JI_BINDING_ONE_WAY, "OneWay");
    TEST_ASSERT_EQ(ji_binding_mode_from_str("TwoWay"), JI_BINDING_TWO_WAY, "TwoWay");
    TEST_ASSERT_EQ(ji_binding_mode_from_str("OneTime"), JI_BINDING_ONE_TIME, "OneTime");
    TEST_ASSERT_EQ(ji_binding_mode_from_str("OneWayToSource"), JI_BINDING_ONE_WAY_TO_SOURCE, "OneWayToSource");
    TEST_ASSERT_EQ(ji_binding_mode_from_str(NULL), JI_BINDING_ONE_WAY, "NULL defaults to OneWay");
    TEST_ASSERT_EQ(ji_binding_mode_from_str("Unknown"), JI_BINDING_ONE_WAY, "Unknown defaults to OneWay");
}

/* =========================================================================
 * Test: create and destroy binding
 * ========================================================================= */
static void test_create_destroy(void) {
    printf("  Binding: create and destroy...\n");
    JiBinding* binding = ji_binding_new(NULL, JI_PROPERTY_INVALID,
                                          NULL, JI_PROPERTY_INVALID,
                                          JI_BINDING_ONE_WAY);
    TEST_ASSERT(binding != NULL, "Binding should not be NULL");
    TEST_ASSERT(!ji_binding_is_active(binding), "Should not be active initially");
    ji_binding_destroy(binding);
}

/* =========================================================================
 * Test: create binding with path
 * ========================================================================= */
static void test_create_with_path(void) {
    printf("  Binding: create with path...\n");
    JiBinding* binding = ji_binding_new_with_path(NULL, JI_PROPERTY_INVALID,
                                                    "UserName",
                                                    JI_BINDING_ONE_WAY);
    TEST_ASSERT(binding != NULL, "Binding should not be NULL");
    TEST_ASSERT_STR(binding->source_path, "UserName", "Source path");
    ji_binding_destroy(binding);
}

/* =========================================================================
 * Test: binding engine init/destroy
 * ========================================================================= */
static void test_engine_init_destroy(void) {
    printf("  Binding: engine init/destroy...\n");
    JiBindingEngine engine;
    ji_binding_engine_init(&engine);
    TEST_ASSERT_EQ(ji_binding_engine_count(&engine), 0, "Should be empty");
    ji_binding_engine_destroy(&engine);
}

/* =========================================================================
 * Test: binding engine add/remove
 * ========================================================================= */
static void test_engine_add_remove(void) {
    printf("  Binding: engine add/remove...\n");
    JiBindingEngine engine;
    ji_binding_engine_init(&engine);

    JiObject* target = ji_object_create();
    JiObject* source = ji_object_create();

    JiBinding* b = ji_binding_engine_add(&engine, target, JI_PROPERTY_INVALID,
                                            source, JI_PROPERTY_INVALID,
                                            JI_BINDING_ONE_WAY);
    TEST_ASSERT(b != NULL, "Binding should not be NULL");
    TEST_ASSERT_EQ(ji_binding_engine_count(&engine), 1, "Should have 1 binding");

    ji_binding_engine_remove(&engine, b);
    TEST_ASSERT_EQ(ji_binding_engine_count(&engine), 0, "Should be empty after remove");

    ji_binding_engine_destroy(&engine);
    ji_ref_object_release(target);
    ji_ref_object_release(source);
}

/* =========================================================================
 * Test: binding engine add with path
 * ========================================================================= */
static void test_engine_add_path(void) {
    printf("  Binding: engine add with path...\n");
    JiBindingEngine engine;
    ji_binding_engine_init(&engine);

    JiObject* target = ji_object_create();

    JiBinding* b = ji_binding_engine_add_path(&engine, target, JI_PROPERTY_INVALID,
                                                "UserName", JI_BINDING_ONE_WAY);
    TEST_ASSERT(b != NULL, "Binding should not be NULL");
    TEST_ASSERT_EQ(ji_binding_engine_count(&engine), 1, "Should have 1 binding");

    ji_binding_engine_destroy(&engine);
    ji_ref_object_release(target);
}

/* =========================================================================
 * Test: binding apply with NULL source/target
 * ========================================================================= */
static void test_apply_null_objects(void) {
    printf("  Binding: apply with NULL objects...\n");
    JiBinding* binding = ji_binding_new(NULL, JI_PROPERTY_INVALID,
                                          NULL, JI_PROPERTY_INVALID,
                                          JI_BINDING_ONE_WAY);
    ji_binding_apply(binding);
    /* Should not crash */
    TEST_ASSERT(ji_binding_is_active(binding), "Should be active after apply");
    ji_binding_destroy(binding);
}

/* =========================================================================
 * Test: binding detach
 * ========================================================================= */
static void test_detach(void) {
    printf("  Binding: detach...\n");
    JiBinding* binding = ji_binding_new(NULL, JI_PROPERTY_INVALID,
                                          NULL, JI_PROPERTY_INVALID,
                                          JI_BINDING_ONE_WAY);
    ji_binding_apply(binding);
    TEST_ASSERT(ji_binding_is_active(binding), "Should be active");
    ji_binding_detach(binding);
    TEST_ASSERT(!ji_binding_is_active(binding), "Should not be active after detach");
    ji_binding_destroy(binding);
}

/* =========================================================================
 * Test: multiple bindings in engine
 * ========================================================================= */
static void test_engine_multiple(void) {
    printf("  Binding: engine multiple bindings...\n");
    JiBindingEngine engine;
    ji_binding_engine_init(&engine);

    JiObject* obj1 = ji_object_create();
    JiObject* obj2 = ji_object_create();
    JiObject* obj3 = ji_object_create();

    ji_binding_engine_add(&engine, obj1, JI_PROPERTY_INVALID,
                            obj2, JI_PROPERTY_INVALID, JI_BINDING_ONE_WAY);
    ji_binding_engine_add(&engine, obj2, JI_PROPERTY_INVALID,
                            obj3, JI_PROPERTY_INVALID, JI_BINDING_TWO_WAY);
    ji_binding_engine_add_path(&engine, obj3, JI_PROPERTY_INVALID,
                                 "Path", JI_BINDING_ONE_TIME);

    TEST_ASSERT_EQ(ji_binding_engine_count(&engine), 3, "Should have 3 bindings");

    ji_binding_engine_destroy(&engine);
    ji_ref_object_release(obj1);
    ji_ref_object_release(obj2);
    ji_ref_object_release(obj3);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    ji_initialize();

    printf("=== Binding Engine Tests ===\n");

    test_binding_mode_str();
    test_binding_mode_from_str();
    test_create_destroy();
    test_create_with_path();
    test_engine_init_destroy();
    test_engine_add_remove();
    test_engine_add_path();
    test_apply_null_objects();
    test_detach();
    test_engine_multiple();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
