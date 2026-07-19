/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_screenshot_test.h
 * @brief Screenshot testing system — capture, compare, detect visual regressions.
 */

#ifndef JIUI_SCREENSHOT_TEST_H
#define JIUI_SCREENSHOT_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ji_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Constants
 * ========================================================================= */

#define JI_SS_MAX_BASELINES   128
#define JI_SS_MAX_NAME_LEN    256
#define JI_SS_PNG_HEADER_SIZE 8

/* =========================================================================
 * Image Buffer
 * ========================================================================= */

typedef struct JiScreenshotImage {
    uint8_t* pixels;    /**< RGBA8888 pixel data */
    uint32_t  width;
    uint32_t  height;
    uint32_t  stride;   /**< Bytes per row */
} JiScreenshotImage;

/** Create a blank RGBA image. */
JI_API JiScreenshotImage* ji_ss_image_new(uint32_t width, uint32_t height);

/** Create an image from raw pixel data (copies the data). */
JI_API JiScreenshotImage* ji_ss_image_from_data(const uint8_t* rgba, uint32_t width, uint32_t height);

/** Destroy an image. */
JI_API void ji_ss_image_free(JiScreenshotImage* img);

/** Fill an image with a solid color. */
JI_API void ji_ss_image_fill(JiScreenshotImage* img, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/** Set a single pixel. */
JI_API void ji_ss_image_set_pixel(JiScreenshotImage* img, uint32_t x, uint32_t y,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/** Get a single pixel. */
JI_API void ji_ss_image_get_pixel(const JiScreenshotImage* img, uint32_t x, uint32_t y,
                             uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);

/* =========================================================================
 * PNG I/O (minimal writer)
 * ========================================================================= */

/** Save an image as a PNG file. Returns 0 on success. */
JI_API int ji_ss_image_save_png(const JiScreenshotImage* img, const char* path);

/** Load a PNG file into an image. Returns NULL on failure. */
JI_API JiScreenshotImage* ji_ss_image_load_png(const char* path);

/* =========================================================================
 * Pixel Comparison
 * ========================================================================= */

/** Result of comparing two images. */
typedef struct JiScreenshotDiff {
    uint64_t total_pixels;
    uint64_t different_pixels;
    uint64_t max_diff;          /**< Maximum per-channel difference */
    double   mean_diff;         /**< Average per-pixel difference (0..255) */
    double   diff_percentage;   /**< Percentage of different pixels (0..100) */
} JiScreenshotDiff;

/** Compare two images with per-channel tolerance. Returns 0 if within tolerance. */
JI_API int ji_ss_compare(const JiScreenshotImage* a, const JiScreenshotImage* b,
                    uint8_t tolerance, JiScreenshotDiff* out_diff);

/** Returns true if images are identical (tolerance = 0). */
JI_API bool ji_ss_images_equal(const JiScreenshotImage* a, const JiScreenshotImage* b);

/** Generate a diff image highlighting differences in red. */
JI_API JiScreenshotImage* ji_ss_diff_image(const JiScreenshotImage* a, const JiScreenshotImage* b,
                                      uint8_t tolerance);

/* =========================================================================
 * Baseline Management
 * ========================================================================= */

typedef struct JiScreenshotBaseline {
    char     name[JI_SS_MAX_NAME_LEN];
    JiScreenshotImage* image;
    uint8_t  tolerance;
    bool     valid;
} JiScreenshotBaseline;

typedef struct JiScreenshotTestRunner {
    JiScreenshotBaseline baselines[JI_SS_MAX_BASELINES];
    int     baseline_count;
    int     tests_run;
    int     tests_passed;
    int     tests_failed;
    char    baseline_dir[256];
    char    output_dir[256];
} JiScreenshotTestRunner;

/** Create a test runner. */
JI_API JiScreenshotTestRunner* ji_ss_runner_new(const char* baseline_dir, const char* output_dir);

/** Destroy a test runner. */
JI_API void ji_ss_runner_free(JiScreenshotTestRunner* runner);

/** Register a baseline image from file. */
JI_API int ji_ss_runner_add_baseline(JiScreenshotTestRunner* runner, const char* name,
                                const char* image_path, uint8_t tolerance);

/** Run a screenshot test: compare `actual` against baseline `name`. */
JI_API int ji_ss_runner_test(JiScreenshotTestRunner* runner, const char* name,
                        const JiScreenshotImage* actual);

/** Run a screenshot test and save the actual image if it fails. */
JI_API int ji_ss_runner_test_and_save(JiScreenshotTestRunner* runner, const char* name,
                                 const JiScreenshotImage* actual);

/** Get test results summary. */
JI_API void ji_ss_runner_get_results(const JiScreenshotTestRunner* runner,
                                int* run, int* passed, int* failed);

/** Print test results to stdout. */
JI_API void ji_ss_runner_print_results(const JiScreenshotTestRunner* runner);

/** Save a new baseline image. */
JI_API int ji_ss_runner_save_baseline(JiScreenshotTestRunner* runner, const char* name,
                                 const JiScreenshotImage* image);

/** Update an existing baseline. */
JI_API int ji_ss_runner_update_baseline(JiScreenshotTestRunner* runner, const char* name,
                                   const JiScreenshotImage* image);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SCREENSHOT_TEST_H */
