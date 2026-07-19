#include "jiui/ji_hidpi.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
 * HiDPI Tests
 * ========================================================================= */

TEST(test_hidpi_create)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    ASSERT(mgr != NULL);
    ASSERT(mgr->monitor_count == 0);
    ASSERT(mgr->global_scale == 0.0);
    ASSERT(mgr->auto_detect == true);
    ji_hidpi_free(mgr);
}

TEST(test_hidpi_add_monitor)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    int idx = ji_hidpi_add_monitor(mgr, 1, 0, 0, 1920, 1080, 96);
    ASSERT(idx == 0);
    ASSERT(mgr->monitor_count == 1);

    const JiMonitorDpi* m = ji_hidpi_get_monitor(mgr, 0);
    ASSERT(m != NULL);
    ASSERT(m->id == 1);
    ASSERT(m->width == 1920);
    ASSERT(m->height == 1080);
    ASSERT(m->dpi == 96);
    ASSERT(fabs(m->scale - 1.0) < 0.01);

    /* Add a HiDPI monitor */
    idx = ji_hidpi_add_monitor(mgr, 2, 1920, 0, 3840, 2160, 192);
    ASSERT(idx == 1);
    ASSERT(mgr->monitor_count == 2);

    m = ji_hidpi_get_monitor(mgr, 1);
    ASSERT(m->dpi == 192);
    ASSERT(fabs(m->scale - 2.0) < 0.01);

    ji_hidpi_free(mgr);
}

TEST(test_hidpi_get_monitor_at)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    ji_hidpi_add_monitor(mgr, 1, 0, 0, 1920, 1080, 96);
    ji_hidpi_add_monitor(mgr, 2, 1920, 0, 3840, 2160, 192);

    const JiMonitorDpi* m = ji_hidpi_get_monitor_at(mgr, 100, 100);
    ASSERT(m != NULL);
    ASSERT(m->id == 1);

    m = ji_hidpi_get_monitor_at(mgr, 2000, 500);
    ASSERT(m != NULL);
    ASSERT(m->id == 2);

    /* Out of bounds returns first monitor */
    m = ji_hidpi_get_monitor_at(mgr, 99999, 99999);
    ASSERT(m != NULL);

    ji_hidpi_free(mgr);
}

TEST(test_hidpi_scale)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    ji_hidpi_add_monitor(mgr, 1, 0, 0, 1920, 1080, 96);
    ji_hidpi_add_monitor(mgr, 2, 1920, 0, 3840, 2160, 192);

    ASSERT(fabs(ji_hidpi_get_scale(mgr, 1) - 1.0) < 0.01);
    ASSERT(fabs(ji_hidpi_get_scale(mgr, 2) - 2.0) < 0.01);

    ASSERT(fabs(ji_hidpi_get_scale_at(mgr, 100, 100) - 1.0) < 0.01);
    ASSERT(fabs(ji_hidpi_get_scale_at(mgr, 2000, 500) - 2.0) < 0.01);

    /* Global scale override */
    ji_hidpi_set_global_scale(mgr, 1.5);
    ASSERT(fabs(ji_hidpi_get_global_scale(mgr) - 1.5) < 0.01);
    ASSERT(fabs(ji_hidpi_get_scale(mgr, 1) - 1.5) < 0.01);
    ASSERT(fabs(ji_hidpi_get_scale(mgr, 2) - 1.5) < 0.01);

    /* Clamp scale */
    ji_hidpi_set_global_scale(mgr, 0.1);
    ASSERT(fabs(ji_hidpi_get_global_scale(mgr) - JI_HIDPI_SCALE_MIN) < 0.01);

    ji_hidpi_set_global_scale(mgr, 10.0);
    ASSERT(fabs(ji_hidpi_get_global_scale(mgr) - JI_HIDPI_SCALE_MAX) < 0.01);

    ji_hidpi_free(mgr);
}

TEST(test_hidpi_coordinate_conversion)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    ji_hidpi_add_monitor(mgr, 1, 0, 0, 1920, 1080, 192);   /* 2x scale */

    int px, py;
    ji_hidpi_dip_to_pixel(mgr, 1, 100.0, 200.0, &px, &py);
    ASSERT(px == 200);
    ASSERT(py == 400);

    double dx, dy;
    ji_hidpi_pixel_to_dip(mgr, 1, 200, 400, &dx, &dy);
    ASSERT(fabs(dx - 100.0) < 0.01);
    ASSERT(fabs(dy - 200.0) < 0.01);

    ji_hidpi_free(mgr);
}

TEST(test_hidpi_resource_suffix)
{
    char buf[32];

    ji_hidpi_get_resource_suffix(1.0, buf, sizeof(buf));
    ASSERT(buf[0] == '\0' || strcmp(buf, "@1x") == 0);

    ji_hidpi_get_resource_suffix(2.0, buf, sizeof(buf));
    ASSERT(strcmp(buf, "@2x") == 0);

    ji_hidpi_get_resource_suffix(3.0, buf, sizeof(buf));
    ASSERT(strcmp(buf, "@3x") == 0);

    ji_hidpi_get_resource_suffix(1.5, buf, sizeof(buf));
    ASSERT(strstr(buf, "1") != NULL);
    ASSERT(strstr(buf, "5") != NULL);

    ji_hidpi_get_resource_suffix(1.25, buf, sizeof(buf));
    ASSERT(strstr(buf, "1") != NULL);
    ASSERT(strstr(buf, "25") != NULL);
}

TEST(test_hidpi_select_best_scale)
{
    double available[] = {1.0, 2.0, 3.0};

    ASSERT(fabs(ji_hidpi_select_best_scale(1.0, available, 3) - 1.0) < 0.01);
    ASSERT(fabs(ji_hidpi_select_best_scale(1.8, available, 3) - 2.0) < 0.01);
    ASSERT(fabs(ji_hidpi_select_best_scale(2.5, available, 3) - 2.0) < 0.01);
    ASSERT(fabs(ji_hidpi_select_best_scale(2.8, available, 3) - 3.0) < 0.01);

    double available2[] = {1.0, 1.5, 2.0, 3.0};
    ASSERT(fabs(ji_hidpi_select_best_scale(1.4, available2, 4) - 1.5) < 0.01);
    ASSERT(fabs(ji_hidpi_select_best_scale(1.6, available2, 4) - 1.5) < 0.01);
    ASSERT(fabs(ji_hidpi_select_best_scale(1.8, available2, 4) - 2.0) < 0.01);
}

TEST(test_hidpi_dpi_to_scale)
{
    ASSERT(fabs(ji_hidpi_dpi_to_scale(96) - 1.0) < 0.01);
    ASSERT(fabs(ji_hidpi_dpi_to_scale(192) - 2.0) < 0.01);
    ASSERT(fabs(ji_hidpi_dpi_to_scale(288) - 3.0) < 0.01);
    ASSERT(fabs(ji_hidpi_dpi_to_scale(120) - 1.25) < 0.01);
    ASSERT(fabs(ji_hidpi_dpi_to_scale(144) - 1.5) < 0.01);
    ASSERT(fabs(ji_hidpi_dpi_to_scale(0) - 1.0) < 0.01);
}

TEST(test_hidpi_auto_detect)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    ASSERT(mgr->auto_detect == true);

    ji_hidpi_set_auto_detect(mgr, false);
    ASSERT(mgr->auto_detect == false);

    ji_hidpi_set_auto_detect(mgr, true);
    ASSERT(mgr->auto_detect == true);

    /* detect_dpi should return a positive value */
    int dpi = ji_hidpi_detect_dpi();
    ASSERT(dpi > 0);

    ji_hidpi_free(mgr);
}

TEST(test_hidpi_fractional)
{
    JiHidpiManager* mgr = ji_hidpi_new();
    /* 120 DPI → 1.25 scale (fractional) */
    ji_hidpi_add_monitor(mgr, 1, 0, 0, 1920, 1080, 120);

    const JiMonitorDpi* m = ji_hidpi_get_monitor(mgr, 0);
    ASSERT(m->fractional == true);
    ASSERT(fabs(m->scale - 1.25) < 0.01);

    /* 96 DPI → 1.0 scale (not fractional) */
    ji_hidpi_add_monitor(mgr, 2, 1920, 0, 1920, 1080, 96);
    m = ji_hidpi_get_monitor(mgr, 1);
    ASSERT(m->fractional == false);

    ji_hidpi_free(mgr);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== HiDPI Tests ===\n");
    RUN_TEST(test_hidpi_create);
    RUN_TEST(test_hidpi_add_monitor);
    RUN_TEST(test_hidpi_get_monitor_at);
    RUN_TEST(test_hidpi_scale);
    RUN_TEST(test_hidpi_coordinate_conversion);
    RUN_TEST(test_hidpi_resource_suffix);
    RUN_TEST(test_hidpi_select_best_scale);
    RUN_TEST(test_hidpi_dpi_to_scale);
    RUN_TEST(test_hidpi_auto_detect);
    RUN_TEST(test_hidpi_fractional);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
