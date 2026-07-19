/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_platform.c
 * @brief Tests for platform backend system — registry, events, window abstraction.
 */

#include <jiui/jiui.h>
#include <jiui/ji_platform.h>
#include <jiui/ji_window.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Test framework (minimal)
 * ========================================================================= */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST(name) static void test_##name(void)
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

#define ASSERT_EQ_PTR(a, b) do { \
    const void* _a = (const void*)(a); \
    const void* _b = (const void*)(b); \
    if (_a != _b) { \
        printf("FAIL\n    Line %d: %p != %p\n", __LINE__, _a, _b); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NULL(p) ASSERT_EQ_PTR(p, NULL)
#define ASSERT_NOT_NULL(p) ASSERT_TRUE((p) != NULL)

#define PASS() do { \
    printf("PASS\n"); \
    g_tests_passed++; \
} while(0)

/* =========================================================================
 * Mock backend for testing
 * ========================================================================= */

static void* mock_create_window(JiPlatformBackend* backend, const char* title,
                                  int x, int y, int width, int height) {
    (void)backend; (void)title; (void)x; (void)y;
    (void)width; (void)height;
    return (void*)0x12345678;  /* fake handle */
}

static void mock_destroy_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static const char* mock_get_name(JiPlatformBackend* backend) {
    if (backend && backend->backend_data) {
        return (const char*)backend->backend_data;
    }
    return "Mock";
}

static int mock_poll_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    (void)backend; (void)out_events; (void)max_events;
    return 0;
}

static bool mock_should_close(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
    return false;
}

static void mock_destroy_backend(JiPlatformBackend* backend) {
    free(backend);
}

static JiDrawingContext* mock_get_drawing_context(JiPlatformBackend* backend) {
    (void)backend;
    return NULL;
}

static JiGLContext* mock_create_gl_context(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
    return NULL;
}

static void mock_destroy_gl_context(JiPlatformBackend* backend, JiGLContext* ctx) {
    (void)backend; (void)ctx;
}

static bool mock_make_current(JiPlatformBackend* backend, void* native_window, JiGLContext* ctx) {
    (void)backend; (void)native_window; (void)ctx;
    return true;
}

static void mock_swap_buffers(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void mock_set_swap_interval(JiPlatformBackend* backend, int interval) {
    (void)backend; (void)interval;
}

static void mock_show_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void mock_hide_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void mock_set_window_title(JiPlatformBackend* backend, void* native_window, const char* title) {
    (void)backend; (void)native_window; (void)title;
}

static void mock_get_window_size(JiPlatformBackend* backend, void* native_window, int* w, int* h) {
    (void)backend; (void)native_window;
    if (w) *w = 800;
    if (h) *h = 600;
}

static int mock_wait_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    (void)backend; (void)out_events; (void)max_events;
    return 0;
}

static void mock_request_close(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
}

static void* mock_get_native_display(JiPlatformBackend* backend) {
    (void)backend;
    return NULL;
}

static void* mock_get_native_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend;
    return native_window;
}

static JiPlatformBackend* create_mock_backend(const char* name) {
    JiPlatformBackend* backend = (JiPlatformBackend*)calloc(1, sizeof(JiPlatformBackend));
    if (!backend) return NULL;

    backend->create_window      = mock_create_window;
    backend->destroy_window     = mock_destroy_window;
    backend->show_window        = mock_show_window;
    backend->hide_window        = mock_hide_window;
    backend->set_window_title   = mock_set_window_title;
    backend->get_window_size    = mock_get_window_size;
    backend->poll_events        = mock_poll_events;
    backend->wait_events        = mock_wait_events;
    backend->should_close       = mock_should_close;
    backend->request_close      = mock_request_close;
    backend->create_gl_context  = mock_create_gl_context;
    backend->destroy_gl_context = mock_destroy_gl_context;
    backend->make_current       = mock_make_current;
    backend->swap_buffers       = mock_swap_buffers;
    backend->set_swap_interval  = mock_set_swap_interval;
    backend->get_drawing_context= mock_get_drawing_context;
    backend->destroy_backend    = mock_destroy_backend;
    backend->get_name           = mock_get_name;
    backend->get_native_display = mock_get_native_display;
    backend->get_native_window  = mock_get_native_window;
    backend->backend_data       = (void*)name;  /* store name for identification */

    return backend;
}

/* =========================================================================
 * Tests
 * ========================================================================= */

TEST(platform_register_find) {
    JiPlatformBackend* mock = create_mock_backend("TestBackend1");
    ASSERT_NOT_NULL(mock);

    bool result = ji_platform_register(mock);
    ASSERT_TRUE(result);

    JiPlatformBackend* found = ji_platform_find("TestBackend1");
    ASSERT_EQ_PTR(found, mock);

    ji_platform_unregister(mock);
    mock->destroy_backend(mock);
    PASS();
}

TEST(platform_register_multiple) {
    JiPlatformBackend* m1 = create_mock_backend("Multi1");
    JiPlatformBackend* m2 = create_mock_backend("Multi2");

    ASSERT_TRUE(ji_platform_register(m1));
    ASSERT_TRUE(ji_platform_register(m2));

    ASSERT_EQ_PTR(ji_platform_find("Multi1"), m1);
    ASSERT_EQ_PTR(ji_platform_find("Multi2"), m2);

    ji_platform_unregister(m1);
    ji_platform_unregister(m2);
    m1->destroy_backend(m1);
    m2->destroy_backend(m2);
    PASS();
}

TEST(platform_find_nonexistent) {
    JiPlatformBackend* found = ji_platform_find("NonExistent");
    ASSERT_NULL(found);
    PASS();
}

TEST(platform_get_default) {
    /* Clear any existing backends first */
    JiPlatformBackend* existing = ji_platform_get_default();
    if (existing) {
        /* Don't test if there's already a backend registered (e.g. X11) */
        PASS();
        return;
    }

    JiPlatformBackend* mock = create_mock_backend("DefaultTest");
    ji_platform_register(mock);

    JiPlatformBackend* def = ji_platform_get_default();
    ASSERT_EQ_PTR(def, mock);

    ji_platform_unregister(mock);
    mock->destroy_backend(mock);
    PASS();
}

TEST(platform_unregister) {
    JiPlatformBackend* mock = create_mock_backend("UnregTest");
    ji_platform_register(mock);

    ASSERT_NOT_NULL(ji_platform_find("UnregTest"));

    ji_platform_unregister(mock);
    ASSERT_NULL(ji_platform_find("UnregTest"));

    mock->destroy_backend(mock);
    PASS();
}

TEST(event_key_codes) {
    /* Verify key code enum values are reasonable */
    ASSERT_TRUE(JI_KEY_ESCAPE != JI_KEY_UNKNOWN);
    ASSERT_TRUE(JI_KEY_RETURN != JI_KEY_UNKNOWN);
    ASSERT_TRUE(JI_KEY_SPACE != JI_KEY_UNKNOWN);
    ASSERT_TRUE(JI_KEY_A != JI_KEY_UNKNOWN);
    ASSERT_TRUE(JI_KEY_F1 != JI_KEY_UNKNOWN);
    ASSERT_TRUE(JI_KEY_LEFT != JI_KEY_RIGHT);
    PASS();
}

TEST(event_mouse_buttons) {
    ASSERT_TRUE(JI_MOUSE_LEFT != JI_MOUSE_NONE);
    ASSERT_TRUE(JI_MOUSE_MIDDLE != JI_MOUSE_NONE);
    ASSERT_TRUE(JI_MOUSE_RIGHT != JI_MOUSE_NONE);
    ASSERT_TRUE(JI_MOUSE_LEFT != JI_MOUSE_RIGHT);
    PASS();
}

TEST(event_kinds) {
    ASSERT_TRUE(JI_EVENT_KEY_PRESS != JI_EVENT_KEY_RELEASE);
    ASSERT_TRUE(JI_EVENT_MOUSE_MOVE != JI_EVENT_MOUSE_PRESS);
    ASSERT_TRUE(JI_EVENT_RESIZE != JI_EVENT_CLOSE);
    ASSERT_TRUE(JI_EVENT_FOCUS_IN != JI_EVENT_FOCUS_OUT);
    PASS();
}

TEST(event_modifiers) {
    ASSERT_TRUE((JI_MOD_SHIFT & JI_MOD_SHIFT) != 0);
    ASSERT_TRUE((JI_MOD_CONTROL & JI_MOD_CONTROL) != 0);
    ASSERT_TRUE((JI_MOD_ALT & JI_MOD_ALT) != 0);
    ASSERT_TRUE((JI_MOD_SUPER & JI_MOD_SUPER) != 0);

    /* Modifiers are independent bits */
    uint32_t combined = JI_MOD_SHIFT | JI_MOD_CONTROL;
    ASSERT_TRUE((combined & JI_MOD_SHIFT) != 0);
    ASSERT_TRUE((combined & JI_MOD_CONTROL) != 0);
    ASSERT_TRUE((combined & JI_MOD_ALT) == 0);
    PASS();
}

TEST(window_flags) {
    /* Verify window flags are independent bits */
    ASSERT_TRUE((JI_WINDOW_RESIZABLE & JI_WINDOW_RESIZABLE) != 0);
    ASSERT_TRUE((JI_WINDOW_OPENGL & JI_WINDOW_OPENGL) != 0);
    ASSERT_TRUE((JI_WINDOW_VSYNC & JI_WINDOW_VSYNC) != 0);

    JiWindowFlags combined = JI_WINDOW_RESIZABLE | JI_WINDOW_OPENGL | JI_WINDOW_VSYNC;
    ASSERT_TRUE((combined & JI_WINDOW_RESIZABLE) != 0);
    ASSERT_TRUE((combined & JI_WINDOW_OPENGL) != 0);
    ASSERT_TRUE((combined & JI_WINDOW_VSYNC) != 0);
    ASSERT_TRUE((combined & JI_WINDOW_FULLSCREEN) == 0);
    PASS();
}

TEST(window_get_time) {
    double t1 = ji_window_get_time();
    ASSERT_TRUE(t1 > 0.0);

    double t2 = ji_window_get_time();
    ASSERT_TRUE(t2 >= t1);
    PASS();
}

TEST(mock_backend_create_window) {
    JiPlatformBackend* mock = create_mock_backend("WindowTest");
    ji_platform_register(mock);

    void* win = mock->create_window(mock, "Test", 0, 0, 800, 600);
    ASSERT_NOT_NULL(win);

    mock->destroy_window(mock, win);

    ji_platform_unregister(mock);
    mock->destroy_backend(mock);
    PASS();
}

TEST(mock_backend_window_size) {
    JiPlatformBackend* mock = create_mock_backend("SizeTest");
    ji_platform_register(mock);

    int w = 0, h = 0;
    mock->get_window_size(mock, (void*)0x1, &w, &h);
    ASSERT_EQ_INT(w, 800);
    ASSERT_EQ_INT(h, 600);

    ji_platform_unregister(mock);
    mock->destroy_backend(mock);
    PASS();
}

TEST(mock_backend_name) {
    JiPlatformBackend* mock = create_mock_backend("NameTest");
    const char* name = mock->get_name(mock);
    ASSERT_TRUE(name != NULL);
    ASSERT_TRUE(strcmp(name, "NameTest") == 0);

    /* Don't register — just test name directly */
    mock->destroy_backend(mock);
    PASS();
}

TEST(drawing_context_init_platform) {
    /* Verify drawing context init works (from Phase 5) */
    JiDrawingContext ctx;
    ji_drawing_context_init(&ctx);

    ASSERT_NULL(ctx.push_clip);
    ASSERT_NULL(ctx.pop_clip);
    ASSERT_NULL(ctx.fill_rect);
    ASSERT_NULL(ctx.clear);
    ASSERT_NULL(ctx.backend_data);
    ASSERT_FALSE(ji_drawing_context_is_valid(&ctx));
    PASS();
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    ji_initialize();

    printf("=== Platform Backend Tests ===\n\n");

    RUN_TEST(platform_register_find);           PASS();
    RUN_TEST(platform_register_multiple);        PASS();
    RUN_TEST(platform_find_nonexistent);          PASS();
    RUN_TEST(platform_get_default);               PASS();
    RUN_TEST(platform_unregister);                PASS();
    RUN_TEST(event_key_codes);                    PASS();
    RUN_TEST(event_mouse_buttons);                PASS();
    RUN_TEST(event_kinds);                        PASS();
    RUN_TEST(event_modifiers);                    PASS();
    RUN_TEST(window_flags);                       PASS();
    RUN_TEST(window_get_time);                    PASS();
    RUN_TEST(mock_backend_create_window);         PASS();
    RUN_TEST(mock_backend_window_size);           PASS();
    RUN_TEST(mock_backend_name);                  PASS();
    RUN_TEST(drawing_context_init_platform);       PASS();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    ji_shutdown();
    return g_tests_failed > 0 ? 1 : 0;
}
