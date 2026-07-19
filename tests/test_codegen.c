/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_codegen.c
 * @brief Tests for the AOT code generator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/jiui.h>
#include <jiui/ji_codegen.h>

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

/* =========================================================================
 * Test: generate from simple XML string
 * ========================================================================= */
static void test_codegen_simple(void) {
    printf("  Codegen: simple object...\n");
    const char* src = "<Object/>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Simple");
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(result.object_count, 1, "Should have 1 object");
    fclose(mem);
}

/* =========================================================================
 * Test: generate from named object
 * ========================================================================= */
static void test_codegen_named(void) {
    printf("  Codegen: named object...\n");
    const char* src = "<Object x:Name=\"myObj\"/>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Named");
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(result.object_count, 1, "Should have 1 object");
    fclose(mem);
}

/* =========================================================================
 * Test: generate nested objects
 * ========================================================================= */
static void test_codegen_nested(void) {
    printf("  Codegen: nested objects...\n");
    const char* src = "<Object><Object/><Object/></Object>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Nested");
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(result.object_count, 3, "Should have 3 objects");
    fclose(mem);
}

/* =========================================================================
 * Test: generate with event handlers
 * ========================================================================= */
static void test_codegen_events(void) {
    printf("  Codegen: event handlers...\n");
    const char* src = "<Object Click=\"onClick\" KeyDown=\"onKey\"/>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Events");
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(result.event_count, 2, "Should have 2 events");
    fclose(mem);
}

/* =========================================================================
 * Test: generate with binding
 * ========================================================================= */
static void test_codegen_binding(void) {
    printf("  Codegen: binding...\n");
    const char* src = "<Object Text=\"{Binding Name}\"/>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Binding");
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(result.binding_count, 1, "Should have 1 binding");
    fclose(mem);
}

/* =========================================================================
 * Test: generate with NULL arguments
 * ========================================================================= */
static void test_codegen_null_args(void) {
    printf("  Codegen: NULL arguments...\n");
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_generate(NULL, mem, "Test");
    TEST_ASSERT(result.has_error, "Should have error for NULL ast");
    result = ji_codegen_from_string(NULL, mem, "Test");
    TEST_ASSERT(result.has_error, "Should have error for NULL source");
    result = ji_codegen_from_string("<Object/>", NULL, "Test");
    TEST_ASSERT(result.has_error, "Should have error for NULL output");
    result = ji_codegen_from_string("<Object/>", mem, NULL);
    TEST_ASSERT(result.has_error, "Should have error for NULL class_name");
    fclose(mem);
}

/* =========================================================================
 * Test: generate from invalid XML
 * ========================================================================= */
static void test_codegen_invalid_xml(void) {
    printf("  Codegen: invalid XML...\n");
    const char* src = "not xml at all";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "Invalid");
    TEST_ASSERT(result.has_error, "Should have error for invalid XML");
    fclose(mem);
}

/* =========================================================================
 * Test: generated code contains expected patterns
 * ========================================================================= */
static void test_codegen_output_content(void) {
    printf("  Codegen: output content verification...\n");
    const char* src = "<Object x:Name=\"root\" Click=\"onRootClick\"/>";
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_string(src, mem, "ContentTest");
    TEST_ASSERT(!result.has_error, "Should not have error");

    /* Read back the generated content */
    fflush(mem);
    rewind(mem);
    char buf[8192];
    size_t n = fread(buf, 1, sizeof(buf) - 1, mem);
    buf[n] = '\0';

    /* Check for expected patterns */
    TEST_ASSERT(strstr(buf, "ji_build_ContentTest") != NULL,
                "Should contain build function");
    TEST_ASSERT(strstr(buf, "ji_object_create()") != NULL,
                "Should contain object creation");
    TEST_ASSERT(strstr(buf, "ji_object_set_name") != NULL,
                "Should contain name setting");
    TEST_ASSERT(strstr(buf, "ji_event_bus_subscribe") != NULL,
                "Should contain event subscription");
    TEST_ASSERT(strstr(buf, "onRootClick") != NULL,
                "Should contain handler name");
    TEST_ASSERT(strstr(buf, "ji_load_ContentTest") != NULL,
                "Should contain convenience load function");

    fclose(mem);
}

/* =========================================================================
 * Test: generate from non-existent file
 * ========================================================================= */
static void test_codegen_file_not_found(void) {
    printf("  Codegen: file not found...\n");
    FILE* mem = tmpfile();
    JiCodeGenResult result = ji_codegen_from_file("/nonexistent/path.ji", mem, "Test");
    TEST_ASSERT(result.has_error, "Should have error for missing file");
    fclose(mem);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    ji_initialize();

    printf("=== Code Generator Tests ===\n");

    test_codegen_simple();
    test_codegen_named();
    test_codegen_nested();
    test_codegen_events();
    test_codegen_binding();
    test_codegen_null_args();
    test_codegen_invalid_xml();
    test_codegen_output_content();
    test_codegen_file_not_found();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
