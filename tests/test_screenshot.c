/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_screenshot.c
 * @brief Tests for the screenshot testing system.
 */

#include "jiui/ji_screenshot_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
 * Image Tests
 * ========================================================================= */

TEST(test_image_create)
{
    JiScreenshotImage* img = ji_ss_image_new(100, 50);
    ASSERT(img != NULL);
    ASSERT(img->width == 100);
    ASSERT(img->height == 50);
    ASSERT(img->stride == 400);
    ASSERT(img->pixels != NULL);

    /* Should be initialized to zero (transparent black) */
    uint8_t r, g, b, a;
    ji_ss_image_get_pixel(img, 10, 10, &r, &g, &b, &a);
    ASSERT(r == 0 && g == 0 && b == 0 && a == 0);

    ji_ss_image_free(img);
}

TEST(test_image_fill)
{
    JiScreenshotImage* img = ji_ss_image_new(10, 10);
    ASSERT(img != NULL);

    ji_ss_image_fill(img, 255, 128, 64, 255);

    uint8_t r, g, b, a;
    ji_ss_image_get_pixel(img, 5, 5, &r, &g, &b, &a);
    ASSERT(r == 255);
    ASSERT(g == 128);
    ASSERT(b == 64);
    ASSERT(a == 255);

    ji_ss_image_free(img);
}

TEST(test_image_set_pixel)
{
    JiScreenshotImage* img = ji_ss_image_new(10, 10);
    ASSERT(img != NULL);

    ji_ss_image_set_pixel(img, 3, 7, 100, 200, 50, 255);

    uint8_t r, g, b, a;
    ji_ss_image_get_pixel(img, 3, 7, &r, &g, &b, &a);
    ASSERT(r == 100);
    ASSERT(g == 200);
    ASSERT(b == 50);
    ASSERT(a == 255);

    /* Other pixels should be untouched */
    ji_ss_image_get_pixel(img, 0, 0, &r, &g, &b, &a);
    ASSERT(r == 0 && g == 0 && b == 0 && a == 0);

    ji_ss_image_free(img);
}

TEST(test_image_from_data)
{
    uint8_t data[4 * 4] = {
        255, 0, 0, 255,   0, 255, 0, 255,
        0, 0, 255, 255,   255, 255, 0, 255
    };
    JiScreenshotImage* img = ji_ss_image_from_data(data, 2, 2);
    ASSERT(img != NULL);
    ASSERT(img->width == 2);
    ASSERT(img->height == 2);

    uint8_t r, g, b, a;
    ji_ss_image_get_pixel(img, 0, 0, &r, &g, &b, &a);
    ASSERT(r == 255 && g == 0 && b == 0 && a == 255);

    ji_ss_image_get_pixel(img, 1, 0, &r, &g, &b, &a);
    ASSERT(r == 0 && g == 255 && b == 0 && a == 255);

    ji_ss_image_free(img);
}

/* =========================================================================
 * Comparison Tests
 * ========================================================================= */

TEST(test_compare_identical)
{
    JiScreenshotImage* a = ji_ss_image_new(10, 10);
    JiScreenshotImage* b = ji_ss_image_new(10, 10);

    ji_ss_image_fill(a, 100, 150, 200, 255);
    ji_ss_image_fill(b, 100, 150, 200, 255);

    JiScreenshotDiff diff;
    int rc = ji_ss_compare(a, b, 0, &diff);
    ASSERT(rc == 0);
    ASSERT(diff.different_pixels == 0);
    ASSERT(diff.diff_percentage == 0.0);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
}

TEST(test_compare_different)
{
    JiScreenshotImage* a = ji_ss_image_new(10, 10);
    JiScreenshotImage* b = ji_ss_image_new(10, 10);

    ji_ss_image_fill(a, 0, 0, 0, 255);
    ji_ss_image_fill(b, 255, 255, 255, 255);

    JiScreenshotDiff diff;
    int rc = ji_ss_compare(a, b, 0, &diff);
    ASSERT(rc == 1);
    ASSERT(diff.different_pixels == 100);
    ASSERT(diff.diff_percentage == 100.0);
    ASSERT(diff.max_diff == 255);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
}

TEST(test_compare_tolerance)
{
    JiScreenshotImage* a = ji_ss_image_new(10, 10);
    JiScreenshotImage* b = ji_ss_image_new(10, 10);

    ji_ss_image_fill(a, 100, 100, 100, 255);
    ji_ss_image_fill(b, 110, 110, 110, 255);

    /* With tolerance 10, should pass */
    JiScreenshotDiff diff;
    int rc = ji_ss_compare(a, b, 10, &diff);
    ASSERT(rc == 0);
    ASSERT(diff.different_pixels == 0);

    /* With tolerance 5, should fail */
    rc = ji_ss_compare(a, b, 5, &diff);
    ASSERT(rc == 1);
    ASSERT(diff.different_pixels == 100);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
}

TEST(test_compare_size_mismatch)
{
    JiScreenshotImage* a = ji_ss_image_new(10, 10);
    JiScreenshotImage* b = ji_ss_image_new(20, 20);

    JiScreenshotDiff diff;
    ASSERT(ji_ss_compare(a, b, 0, &diff) == -1);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
}

TEST(test_images_equal)
{
    JiScreenshotImage* a = ji_ss_image_new(5, 5);
    JiScreenshotImage* b = ji_ss_image_new(5, 5);
    JiScreenshotImage* c = ji_ss_image_new(5, 5);

    ji_ss_image_fill(a, 50, 100, 150, 255);
    ji_ss_image_fill(b, 50, 100, 150, 255);
    ji_ss_image_fill(c, 51, 100, 150, 255);

    ASSERT(ji_ss_images_equal(a, b) == true);
    ASSERT(ji_ss_images_equal(a, c) == false);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
    ji_ss_image_free(c);
}

TEST(test_diff_image)
{
    JiScreenshotImage* a = ji_ss_image_new(4, 4);
    JiScreenshotImage* b = ji_ss_image_new(4, 4);

    ji_ss_image_fill(a, 0, 0, 0, 255);
    ji_ss_image_fill(b, 255, 255, 255, 255);

    JiScreenshotImage* diff = ji_ss_diff_image(a, b, 0);
    ASSERT(diff != NULL);

    /* All pixels should be red (highlighted) */
    uint8_t r, g, bl, al;
    ji_ss_image_get_pixel(diff, 2, 2, &r, &g, &bl, &al);
    ASSERT(r == 255);
    ASSERT(g == 0);
    ASSERT(bl == 0);

    ji_ss_image_free(a);
    ji_ss_image_free(b);
    ji_ss_image_free(diff);
}

/* =========================================================================
 * PNG I/O Tests
 * ========================================================================= */

TEST(test_png_save_load)
{
    JiScreenshotImage* img = ji_ss_image_new(8, 8);
    ASSERT(img != NULL);

    /* Draw a simple pattern */
    ji_ss_image_fill(img, 0, 0, 0, 255);
    ji_ss_image_set_pixel(img, 0, 0, 255, 0, 0, 255);
    ji_ss_image_set_pixel(img, 7, 7, 0, 255, 0, 255);
    ji_ss_image_set_pixel(img, 3, 4, 0, 0, 255, 255);

    /* Save */
    ASSERT(ji_ss_image_save_png(img, "/tmp/jiui_test_screenshot.png") == 0);

    /* Load */
    JiScreenshotImage* loaded = ji_ss_image_load_png("/tmp/jiui_test_screenshot.png");
    ASSERT(loaded != NULL);
    ASSERT(loaded->width == 8);
    ASSERT(loaded->height == 8);

    /* Verify pixels match */
    ASSERT(ji_ss_images_equal(img, loaded) == true);

    ji_ss_image_free(img);
    ji_ss_image_free(loaded);
}

/* =========================================================================
 * Runner Tests
 * ========================================================================= */

TEST(test_runner_create)
{
    JiScreenshotTestRunner* r = ji_ss_runner_new("/tmp/jiui_baselines", "/tmp/jiui_output");
    ASSERT(r != NULL);
    ASSERT(r->baseline_count == 0);
    ASSERT(r->tests_run == 0);
    ASSERT(r->tests_passed == 0);
    ASSERT(r->tests_failed == 0);
    ji_ss_runner_free(r);
}

TEST(test_runner_save_and_test)
{
    JiScreenshotTestRunner* r = ji_ss_runner_new("/tmp/jiui_baselines", "/tmp/jiui_output");
    ASSERT(r != NULL);

    /* Create and save a baseline */
    JiScreenshotImage* baseline = ji_ss_image_new(10, 10);
    ji_ss_image_fill(baseline, 100, 150, 200, 255);
    ASSERT(ji_ss_runner_save_baseline(r, "test1", baseline) == 0);
    ASSERT(r->baseline_count == 1);

    /* Create an identical image and test */
    JiScreenshotImage* actual = ji_ss_image_new(10, 10);
    ji_ss_image_fill(actual, 100, 150, 200, 255);
    ASSERT(ji_ss_runner_test(r, "test1", actual) == 0);
    ASSERT(r->tests_run == 1);
    ASSERT(r->tests_passed == 1);

    /* Create a different image and test */
    JiScreenshotImage* different = ji_ss_image_new(10, 10);
    ji_ss_image_fill(different, 0, 0, 0, 255);
    ASSERT(ji_ss_runner_test(r, "test1", different) == 1);
    ASSERT(r->tests_run == 2);
    ASSERT(r->tests_passed == 1);
    ASSERT(r->tests_failed == 1);

    ji_ss_image_free(baseline);
    ji_ss_image_free(actual);
    ji_ss_image_free(different);
    ji_ss_runner_free(r);
}

TEST(test_runner_update_baseline)
{
    JiScreenshotTestRunner* r = ji_ss_runner_new("/tmp/jiui_baselines", "/tmp/jiui_output");
    ASSERT(r != NULL);

    JiScreenshotImage* img1 = ji_ss_image_new(5, 5);
    ji_ss_image_fill(img1, 10, 20, 30, 255);
    ji_ss_runner_save_baseline(r, "update_test", img1);

    JiScreenshotImage* img2 = ji_ss_image_new(5, 5);
    ji_ss_image_fill(img2, 40, 50, 60, 255);
    ASSERT(ji_ss_runner_update_baseline(r, "update_test", img2) == 0);

    /* Now testing with img2 should pass */
    ASSERT(ji_ss_runner_test(r, "update_test", img2) == 0);

    ji_ss_image_free(img1);
    ji_ss_image_free(img2);
    ji_ss_runner_free(r);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Screenshot Test Tests ===\n");
    RUN_TEST(test_image_create);
    RUN_TEST(test_image_fill);
    RUN_TEST(test_image_set_pixel);
    RUN_TEST(test_image_from_data);
    RUN_TEST(test_compare_identical);
    RUN_TEST(test_compare_different);
    RUN_TEST(test_compare_tolerance);
    RUN_TEST(test_compare_size_mismatch);
    RUN_TEST(test_images_equal);
    RUN_TEST(test_diff_image);
    RUN_TEST(test_png_save_load);
    RUN_TEST(test_runner_create);
    RUN_TEST(test_runner_save_and_test);
    RUN_TEST(test_runner_update_baseline);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
