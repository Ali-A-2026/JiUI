#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_profiler.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static void ji_test_sleep_us(int us) {
    struct timespec ts = {0, us * 1000};
    nanosleep(&ts, NULL);
}

/* =========================================================================
 * Test Framework
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    static void name(void); \
    static void name(void)

#define RUN_TEST(name) do { \
    g_tests_run++; \
    printf("  [RUN] %s ... ", #name); \
    name(); \
    g_tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return; \
    } \
} while(0)

/* =========================================================================
 * Profiler Tests
 * ========================================================================= */

TEST(test_profiler_create)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);
    ASSERT(ji_profiler_is_enabled(p) == true);
    ji_profiler_free(p);
}

TEST(test_profiler_enable_disable)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_set_enabled(p, false);
    ASSERT(ji_profiler_is_enabled(p) == false);

    ji_profiler_set_enabled(p, true);
    ASSERT(ji_profiler_is_enabled(p) == true);

    ji_profiler_free(p);
}

TEST(test_profiler_frame_timing)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    /* Run a few frames */
    for (int i = 0; i < 5; i++) {
        ji_profiler_begin_frame(p);
        ji_test_sleep_us(1000); /* 1ms */
        ji_profiler_end_frame(p);
    }

    JiProfilerStats stats = ji_profiler_get_stats(p);
    ASSERT(stats.total_frames == 5);
    ASSERT(stats.avg_frame_time_us > 0.0);

    ji_profiler_free(p);
}

TEST(test_profiler_sections)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_begin_frame(p);
    ji_profiler_begin_section(p, "update");
    ji_test_sleep_us(500);
    ji_profiler_end_section(p);
    ji_profiler_begin_section(p, "render");
    ji_test_sleep_us(500);
    ji_profiler_end_section(p);
    ji_profiler_end_frame(p);

    double update_time = ji_profiler_get_section_time(p, "update");
    double render_time = ji_profiler_get_section_time(p, "render");
    ASSERT(update_time > 0.0);
    ASSERT(render_time > 0.0);

    ji_profiler_free(p);
}

TEST(test_profiler_nested_sections)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_begin_frame(p);
    ji_profiler_begin_section(p, "outer");
    ji_profiler_begin_section(p, "inner");
    ji_test_sleep_us(500);
    ji_profiler_end_section(p);
    ji_profiler_end_section(p);
    ji_profiler_end_frame(p);

    double outer = ji_profiler_get_section_time(p, "outer");
    double inner = ji_profiler_get_section_time(p, "inner");
    ASSERT(outer > 0.0);
    ASSERT(inner > 0.0);

    ji_profiler_free(p);
}

TEST(test_profiler_layout_render)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_begin_frame(p);
    ji_profiler_begin_layout(p);
    ji_test_sleep_us(200);
    ji_profiler_end_layout(p);
    ji_profiler_begin_render(p);
    ji_test_sleep_us(300);
    ji_profiler_end_render(p);
    ji_profiler_end_frame(p);

    JiProfilerStats stats = ji_profiler_get_stats(p);
    ASSERT(stats.avg_layout_time_us > 0.0);
    ASSERT(stats.avg_render_time_us > 0.0);

    ji_profiler_free(p);
}

TEST(test_profiler_draw_calls)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_begin_frame(p);
    ji_profiler_add_draw_call(p, 100);
    ji_profiler_add_draw_call(p, 200);
    ji_profiler_add_draw_call(p, 50);
    ji_profiler_add_texture_bind(p);
    ji_profiler_add_texture_bind(p);
    ji_profiler_end_frame(p);

    JiProfilerStats stats = ji_profiler_get_stats(p);
    ASSERT(stats.avg_draw_calls == 3);
    ASSERT(stats.avg_triangles == 350);

    ji_profiler_free(p);
}

TEST(test_profiler_memory_tracking)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_track_alloc(p, 1024);
    ASSERT(p->memory_current == 1024);
    ASSERT(p->memory_peak == 1024);

    ji_profiler_track_alloc(p, 2048);
    ASSERT(p->memory_current == 3072);
    ASSERT(p->memory_peak == 3072);

    ji_profiler_track_free(p, 1024);
    ASSERT(p->memory_current == 2048);
    ASSERT(p->memory_peak == 3072);

    ji_profiler_free(p);
}

TEST(test_profiler_overlay)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_set_overlay(p, JI_PROFILER_OVERLAY_OFF);
    ASSERT(ji_profiler_get_overlay(p) == JI_PROFILER_OVERLAY_OFF);

    ji_profiler_set_overlay(p, JI_PROFILER_OVERLAY_MINIMAL);
    ASSERT(ji_profiler_get_overlay(p) == JI_PROFILER_OVERLAY_MINIMAL);

    ji_profiler_set_overlay(p, JI_PROFILER_OVERLAY_FULL);
    ASSERT(ji_profiler_get_overlay(p) == JI_PROFILER_OVERLAY_FULL);

    ji_profiler_free(p);
}

TEST(test_profiler_overlay_text)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    /* Run a frame */
    ji_profiler_begin_frame(p);
    ji_test_sleep_us(1000);
    ji_profiler_end_frame(p);

    /* Minimal overlay */
    ji_profiler_set_overlay(p, JI_PROFILER_OVERLAY_MINIMAL);
    char buf[1024];
    int len = ji_profiler_get_overlay_text(p, buf, sizeof(buf));
    ASSERT(len > 0);
    ASSERT(strstr(buf, "FPS") != NULL);

    /* Off overlay */
    ji_profiler_set_overlay(p, JI_PROFILER_OVERLAY_OFF);
    len = ji_profiler_get_overlay_text(p, buf, sizeof(buf));
    ASSERT(len == 0);

    ji_profiler_free(p);
}

TEST(test_profiler_reset)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    /* Add some data */
    ji_profiler_begin_frame(p);
    ji_profiler_add_draw_call(p, 100);
    ji_profiler_end_frame(p);
    ji_profiler_track_alloc(p, 512);

    ASSERT(p->frame_count == 1);
    ASSERT(p->memory_current == 512);

    ji_profiler_reset(p);
    ASSERT(p->frame_count == 0);
    ASSERT(p->memory_current == 0);

    ji_profiler_free(p);
}

TEST(test_profiler_gpu_time)
{
    JiProfiler* p = ji_profiler_new();
    ASSERT(p != NULL);

    ji_profiler_begin_frame(p);
    ji_profiler_set_gpu_time(p, 5000.0); /* 5ms */
    ji_profiler_end_frame(p);

    JiProfilerStats stats = ji_profiler_get_stats(p);
    ASSERT(stats.avg_gpu_time_us == 5000.0);

    ji_profiler_free(p);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Profiler Tests ===\n");

    RUN_TEST(test_profiler_create);
    RUN_TEST(test_profiler_enable_disable);
    RUN_TEST(test_profiler_frame_timing);
    RUN_TEST(test_profiler_sections);
    RUN_TEST(test_profiler_nested_sections);
    RUN_TEST(test_profiler_layout_render);
    RUN_TEST(test_profiler_draw_calls);
    RUN_TEST(test_profiler_memory_tracking);
    RUN_TEST(test_profiler_overlay);
    RUN_TEST(test_profiler_overlay_text);
    RUN_TEST(test_profiler_reset);
    RUN_TEST(test_profiler_gpu_time);

    printf("\n%d/%d tests passed\n", g_tests_passed, g_tests_run);
    return (g_tests_run == g_tests_passed) ? 0 : 1;
}
