/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_loader.c
 * @brief Tests for the runtime loader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/jiui.h>
#include <jiui/ji_loader.h>
#include <jiui/ji_event.h>

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
 * Test: load simple object from string
 * ========================================================================= */
static void test_load_simple(void) {
    printf("  Loader: simple object from string...\n");
    const char* src = "<Object/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    TEST_ASSERT(result.resources != NULL, "Resources should not be NULL");
    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load object with name
 * ========================================================================= */
static void test_load_with_name(void) {
    printf("  Loader: object with name...\n");
    const char* src = "<Object x:Name=\"mainWindow\"/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        TEST_ASSERT_STR(ji_object_name(result.root), "mainWindow", "Name should be mainWindow");
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load nested objects
 * ========================================================================= */
static void test_load_nested(void) {
    printf("  Loader: nested objects...\n");
    const char* src = "<Object><Object/><Object/></Object>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    if (result.root) {
        TEST_ASSERT_EQ(ji_object_get_child_count(result.root), 2, "Root should have 2 children");
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load from NULL string
 * ========================================================================= */
static void test_load_null(void) {
    printf("  Loader: NULL string...\n");
    JiLoadResult result = ji_load_string(NULL);
    TEST_ASSERT(result.has_error, "Should have error for NULL string");
    TEST_ASSERT(result.root == NULL, "Root should be NULL");
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load invalid XML
 * ========================================================================= */
static void test_load_invalid(void) {
    printf("  Loader: invalid XML...\n");
    const char* src = "not xml at all";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(result.has_error, "Should have error for invalid XML");
    TEST_ASSERT(result.root == NULL, "Root should be NULL for invalid XML");
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load from file (non-existent)
 * ========================================================================= */
static void test_load_file_not_found(void) {
    printf("  Loader: file not found...\n");
    JiLoadResult result = ji_load_file("/nonexistent/path/file.ji");
    TEST_ASSERT(result.has_error, "Should have error for missing file");
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: load result destroy with NULL
 * ========================================================================= */
static void test_destroy_null(void) {
    printf("  Loader: destroy NULL result...\n");
    ji_load_result_destroy(NULL);
    /* Should not crash */
    TEST_ASSERT(true, "Should not crash on NULL");
}

/* =========================================================================
 * Test: load with resource dictionary
 * ========================================================================= */
static void test_load_with_resources(void) {
    printf("  Loader: with resource dictionary...\n");
    const char* src =
        "<Object>"
        "  <Object.Resources>"
        "    <ResourceDictionary/>"
        "  </Object.Resources>"
        "</Object>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    TEST_ASSERT(result.resources != NULL, "Resources should not be NULL");
    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: binding engine is initialized
 * ========================================================================= */
static void test_binding_engine_init(void) {
    printf("  Loader: binding engine initialized...\n");
    const char* src = "<Object/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT_EQ(ji_binding_engine_count(&result.bindings), 0, "Should have 0 bindings");
    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: event bus is initialized
 * ========================================================================= */
static void test_event_bus_init(void) {
    printf("  Loader: event bus initialized...\n");
    const char* src = "<Object/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT_EQ(ji_event_bus_count(&result.events), 0, "Should have 0 event subscriptions");
    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: event handler registration and loading
 * ========================================================================= */
static int g_click_count = 0;

static void on_click_handler(JiObject* sender, void* args, void* user_data) {
    (void)sender; (void)args; (void)user_data;
    g_click_count++;
}

static void test_load_with_event(void) {
    printf("  Loader: with event handler...\n");

    /* Register the handler before loading */
    ji_event_register_handler("onButtonClick", on_click_handler);

    const char* src = "<Object Click=\"onButtonClick\"/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(ji_event_bus_count(&result.events), 1, "Should have 1 event subscription");

    /* Dispatch the event and verify handler is called */
    g_click_count = 0;
    ji_event_bus_dispatch(&result.events, result.root, "Click", NULL);
    TEST_ASSERT_EQ(g_click_count, 1, "Click handler should have been called once");

    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: event on child object
 * ========================================================================= */
static int g_loaded_count = 0;

static void on_loaded_handler(JiObject* sender, void* args, void* user_data) {
    (void)sender; (void)args; (void)user_data;
    g_loaded_count++;
}

static void test_load_event_on_child(void) {
    printf("  Loader: event on child object...\n");

    ji_event_register_handler("onChildLoaded", on_loaded_handler);

    const char* src = "<Object><Object Loaded=\"onChildLoaded\"/></Object>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(ji_event_bus_count(&result.events), 1, "Should have 1 event subscription");

    /* The event should be on the child, not the root */
    g_loaded_count = 0;
    JiObject* child = ji_object_get_child(result.root, 0);
    TEST_ASSERT(child != NULL, "Child should not be NULL");

    /* Dispatching on root should NOT trigger the handler */
    ji_event_bus_dispatch(&result.events, result.root, "Loaded", NULL);
    TEST_ASSERT_EQ(g_loaded_count, 0, "Handler should NOT fire on root");

    /* Dispatching on child SHOULD trigger the handler */
    ji_event_bus_dispatch(&result.events, child, "Loaded", NULL);
    TEST_ASSERT_EQ(g_loaded_count, 1, "Handler should fire on child");

    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: unknown handler name is silently ignored
 * ========================================================================= */
static void test_load_event_unknown_handler(void) {
    printf("  Loader: unknown handler name ignored...\n");

    const char* src = "<Object Click=\"nonExistentHandler\"/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT(result.root != NULL, "Root should not be NULL");
    /* No subscription should be created for an unknown handler */
    TEST_ASSERT_EQ(ji_event_bus_count(&result.events), 0, "Should have 0 subscriptions for unknown handler");

    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: multiple events on same object
 * ========================================================================= */
static int g_keydown_count = 0;

static void on_keydown_handler(JiObject* sender, void* args, void* user_data) {
    (void)sender; (void)args; (void)user_data;
    g_keydown_count++;
}

static void test_load_multiple_events(void) {
    printf("  Loader: multiple events on same object...\n");

    ji_event_register_handler("onKeyDown", on_keydown_handler);

    const char* src = "<Object Click=\"onButtonClick\" KeyDown=\"onKeyDown\"/>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");
    TEST_ASSERT_EQ(ji_event_bus_count(&result.events), 2, "Should have 2 event subscriptions");

    /* Dispatch Click */
    g_click_count = 0;
    ji_event_bus_dispatch(&result.events, result.root, "Click", NULL);
    TEST_ASSERT_EQ(g_click_count, 1, "Click handler should fire");

    /* Dispatch KeyDown */
    g_keydown_count = 0;
    ji_event_bus_dispatch(&result.events, result.root, "KeyDown", NULL);
    TEST_ASSERT_EQ(g_keydown_count, 1, "KeyDown handler should fire");

    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Test: find named element recursively
 * ========================================================================= */
static void test_find_named_element(void) {
    printf("  Loader: find named element...\n");

    const char* src =
        "<Object>"
        "  <Object x:Name=\"child1\">"
        "    <Object x:Name=\"grandchild\"/>"
        "  </Object>"
        "</Object>";
    JiLoadResult result = ji_load_string(src);
    TEST_ASSERT(!result.has_error, "Should not have error");

    JiObject* found = ji_load_find_name(result.root, "grandchild");
    TEST_ASSERT(found != NULL, "Should find grandchild");
    if (found) {
        TEST_ASSERT_STR(ji_object_name(found), "grandchild", "Name should be grandchild");
    }

    JiObject* not_found = ji_load_find_name(result.root, "nonexistent");
    TEST_ASSERT(not_found == NULL, "Should not find nonexistent");

    if (result.root) {
        ji_ref_object_release(result.root);
    }
    ji_load_result_destroy(&result);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    ji_initialize();

    printf("=== Runtime Loader Tests ===\n");

    test_load_simple();
    test_load_with_name();
    test_load_nested();
    test_load_null();
    test_load_invalid();
    test_load_file_not_found();
    test_destroy_null();
    test_load_with_resources();
    test_binding_engine_init();
    test_event_bus_init();
    test_load_with_event();
    test_load_event_on_child();
    test_load_event_unknown_handler();
    test_load_multiple_events();
    test_find_named_element();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
