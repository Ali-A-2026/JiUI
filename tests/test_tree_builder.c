/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_tree_builder.c
 * @brief Tests for the tree builder — XML AST to JiObject tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/jiui.h>
#include <jiui/ji_tree_builder.h>
#include <jiui/ji_type.h>
#include <jiui/ji_object.h>

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
 * Test: build simple object
 * ========================================================================= */
static void test_simple_object(void) {
    printf("  Tree Builder: simple object...\n");
    const char* src = "<Object/>";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        TEST_ASSERT_EQ(ji_object_type_id(result.root), JI_TYPE_OBJECT, "Should be JiObject type");
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Test: build object with x:Name
 * ========================================================================= */
static void test_object_with_name(void) {
    printf("  Tree Builder: object with name...\n");
    const char* src = "<Object x:Name=\"myObj\"/>";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        TEST_ASSERT_STR(ji_object_name(result.root), "myObj", "Name should be myObj");
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Test: build nested objects
 * ========================================================================= */
static void test_nested_objects(void) {
    printf("  Tree Builder: nested objects...\n");
    const char* src = "<Object><Object/><Object/></Object>";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Test: build from NULL AST
 * ========================================================================= */
static void test_null_ast(void) {
    printf("  Tree Builder: null AST...\n");
    JiTreeBuildResult result = ji_tree_build(NULL);
    TEST_ASSERT(result.has_error, "Should have error for NULL AST");
    TEST_ASSERT(result.root == NULL, "Root should be NULL");
}

/* =========================================================================
 * Test: build from NULL string
 * ========================================================================= */
static void test_null_string(void) {
    printf("  Tree Builder: null string...\n");
    JiTreeBuildResult result = ji_tree_build_from_string(NULL);
    TEST_ASSERT(result.has_error, "Should have error for NULL string");
    TEST_ASSERT(result.root == NULL, "Root should be NULL");
}

/* =========================================================================
 * Test: build from invalid XML
 * ========================================================================= */
static void test_invalid_xml(void) {
    printf("  Tree Builder: invalid XML...\n");
    const char* src = "not xml at all";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(result.has_error, "Should have error for invalid XML");
    TEST_ASSERT(result.root == NULL, "Root should be NULL for invalid XML");
}

/* =========================================================================
 * Test: build deeply nested objects
 * ========================================================================= */
static void test_deep_nesting(void) {
    printf("  Tree Builder: deep nesting...\n");
    const char* src = "<Object><Object><Object/></Object></Object>";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        /* Verify parent chain */
        JiObject* child = ji_object_get_parent(result.root);
        (void)child; /* just verify no crash */
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Test: build object with multiple attributes
 * ========================================================================= */
static void test_object_with_attributes(void) {
    printf("  Tree Builder: object with attributes...\n");
    const char* src = "<Object x:Name=\"test\"/>";
    JiTreeBuildResult result = ji_tree_build_from_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        TEST_ASSERT_STR(ji_object_name(result.root), "test", "Name should be test");
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    /* Initialize the JiUI runtime */
    ji_initialize();

    printf("=== Tree Builder Tests ===\n");

    test_simple_object();
    test_object_with_name();
    test_nested_objects();
    test_null_ast();
    test_null_string();
    test_invalid_xml();
    test_deep_nesting();
    test_object_with_attributes();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
