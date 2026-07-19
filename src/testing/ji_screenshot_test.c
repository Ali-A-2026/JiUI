/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_screenshot_test.c
 * @brief Screenshot testing system implementation.
 */

#include "jiui/ji_screenshot_test.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "ji_zlib_compat.h"
#include <sys/stat.h>
#include <sys/types.h>

/* =========================================================================
 * Image Buffer
 * ========================================================================= */

JiScreenshotImage* ji_ss_image_new(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return NULL;
    JiScreenshotImage* img = (JiScreenshotImage*)calloc(1, sizeof(JiScreenshotImage));
    if (!img) return NULL;
    img->width = width;
    img->height = height;
    img->stride = width * 4;
    img->pixels = (uint8_t*)calloc((size_t)width * height * 4, 1);
    if (!img->pixels) {
        free(img);
        return NULL;
    }
    return img;
}

JiScreenshotImage* ji_ss_image_from_data(const uint8_t* rgba, uint32_t width, uint32_t height)
{
    if (!rgba || width == 0 || height == 0) return NULL;
    JiScreenshotImage* img = ji_ss_image_new(width, height);
    if (!img) return NULL;
    memcpy(img->pixels, rgba, (size_t)width * height * 4);
    return img;
}

void ji_ss_image_free(JiScreenshotImage* img)
{
    if (!img) return;
    if (img->pixels) free(img->pixels);
    free(img);
}

void ji_ss_image_fill(JiScreenshotImage* img, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!img) return;
    for (uint32_t i = 0; i < img->width * img->height; i++) {
        img->pixels[i * 4 + 0] = r;
        img->pixels[i * 4 + 1] = g;
        img->pixels[i * 4 + 2] = b;
        img->pixels[i * 4 + 3] = a;
    }
}

void ji_ss_image_set_pixel(JiScreenshotImage* img, uint32_t x, uint32_t y,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!img || x >= img->width || y >= img->height) return;
    uint32_t idx = (y * img->stride + x * 4);
    img->pixels[idx + 0] = r;
    img->pixels[idx + 1] = g;
    img->pixels[idx + 2] = b;
    img->pixels[idx + 3] = a;
}

void ji_ss_image_get_pixel(const JiScreenshotImage* img, uint32_t x, uint32_t y,
                             uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)
{
    if (!img || x >= img->width || y >= img->height) return;
    uint32_t idx = (y * img->stride + x * 4);
    if (r) *r = img->pixels[idx + 0];
    if (g) *g = img->pixels[idx + 1];
    if (b) *b = img->pixels[idx + 2];
    if (a) *a = img->pixels[idx + 3];
}

/* =========================================================================
 * PNG I/O (minimal writer using zlib)
 * ========================================================================= */

static void write_u32_be(FILE* f, uint32_t val)
{
    fputc((val >> 24) & 0xFF, f);
    fputc((val >> 16) & 0xFF, f);
    fputc((val >>  8) & 0xFF, f);
    fputc(val & 0xFF, f);
}

static uint32_t crc_table[256];
static bool crc_table_init = false;

static void init_crc_table(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) c = 0xEDB88320 ^ (c >> 1);
            else c >>= 1;
        }
        crc_table[i] = c;
    }
    crc_table_init = true;
}

static uint32_t crc32_calc(const uint8_t* data, size_t len)
{
    if (!crc_table_init) init_crc_table();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

int ji_ss_image_save_png(const JiScreenshotImage* img, const char* path)
{
    if (!img || !path) return -1;

    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    /* PNG signature */
    static const uint8_t png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    fwrite(png_sig, 1, 8, f);

    /* IHDR chunk */
    uint8_t ihdr[13];
    ihdr[0] = (img->width >> 24) & 0xFF;
    ihdr[1] = (img->width >> 16) & 0xFF;
    ihdr[2] = (img->width >>  8) & 0xFF;
    ihdr[3] = img->width & 0xFF;
    ihdr[4] = (img->height >> 24) & 0xFF;
    ihdr[5] = (img->height >> 16) & 0xFF;
    ihdr[6] = (img->height >>  8) & 0xFF;
    ihdr[7] = img->height & 0xFF;
    ihdr[8] = 8;   /* bit depth */
    ihdr[9] = 6;   /* color type: RGBA */
    ihdr[10] = 0;  /* compression */
    ihdr[11] = 0;  /* filter */
    ihdr[12] = 0;  /* interlace */

    write_u32_be(f, 13);  /* IHDR length */
    fwrite("IHDR", 1, 4, f);
    fwrite(ihdr, 1, 13, f);
    /* CRC over "IHDR" + ihdr data */
    {
        uint8_t ihdr_crc_buf[17];
        memcpy(ihdr_crc_buf, "IHDR", 4);
        memcpy(ihdr_crc_buf + 4, ihdr, 13);
        write_u32_be(f, crc32_calc(ihdr_crc_buf, 17));
    }

    /* Build raw image data with filter byte per row */
    size_t raw_len = (size_t)(img->width * 4 + 1) * img->height;
    uint8_t* raw = (uint8_t*)malloc(raw_len);
    if (!raw) { fclose(f); return -1; }

    for (uint32_t y = 0; y < img->height; y++) {
        raw[y * (img->width * 4 + 1)] = 0;  /* filter: none */
        memcpy(raw + y * (img->width * 4 + 1) + 1,
               img->pixels + y * img->stride, img->width * 4);
    }

    /* Compress with zlib */
    uLong compressed_len = compressBound(raw_len);
    uint8_t* compressed = (uint8_t*)malloc(compressed_len);
    if (!compressed) { free(raw); fclose(f); return -1; }

    if (compress(compressed, &compressed_len, raw, raw_len) != Z_OK) {
        free(raw); free(compressed); fclose(f); return -1;
    }

    /* IDAT chunk */
    write_u32_be(f, (uint32_t)compressed_len);
    fwrite("IDAT", 1, 4, f);
    fwrite(compressed, 1, compressed_len, f);

    /* CRC over "IDAT" + data */
    uint8_t* crc_buf = (uint8_t*)malloc(compressed_len + 4);
    memcpy(crc_buf, "IDAT", 4);
    memcpy(crc_buf + 4, compressed, compressed_len);
    write_u32_be(f, crc32_calc(crc_buf, compressed_len + 4));
    free(crc_buf);

    /* IEND chunk */
    write_u32_be(f, 0);
    fwrite("IEND", 1, 4, f);
    write_u32_be(f, crc32_calc((const uint8_t*)"IEND", 4));

    free(raw);
    free(compressed);
    fclose(f);
    return 0;
}

JiScreenshotImage* ji_ss_image_load_png(const char* path)
{
    /* Minimal PNG reader — only supports RGBA8, no interlace */
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    uint8_t sig[8];
    if (fread(sig, 1, 8, f) != 8) { fclose(f); return NULL; }
    static const uint8_t expected[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (memcmp(sig, expected, 8) != 0) { fclose(f); return NULL; }

    uint32_t width = 0, height = 0;
    uint8_t bit_depth = 0, color_type = 0;
    uint8_t* idat_data = NULL;
    size_t idat_len = 0;

    while (1) {
        uint8_t len_bytes[4];
        if (fread(len_bytes, 1, 4, f) != 4) break;
        uint32_t chunk_len = ((uint32_t)len_bytes[0] << 24) | ((uint32_t)len_bytes[1] << 16) |
                             ((uint32_t)len_bytes[2] << 8) | len_bytes[3];

        char type[5] = {0};
        if (fread(type, 1, 4, f) != 4) break;

        if (strcmp(type, "IHDR") == 0) {
            uint8_t ihdr[13];
            if (fread(ihdr, 1, 13, f) != 13) break;
            width = ((uint32_t)ihdr[0] << 24) | ((uint32_t)ihdr[1] << 16) |
                    ((uint32_t)ihdr[2] << 8) | ihdr[3];
            height = ((uint32_t)ihdr[4] << 24) | ((uint32_t)ihdr[5] << 16) |
                     ((uint32_t)ihdr[6] << 8) | ihdr[7];
            bit_depth = ihdr[8];
            color_type = ihdr[9];
        } else if (strcmp(type, "IDAT") == 0) {
            uint8_t* new_data = (uint8_t*)realloc(idat_data, idat_len + chunk_len);
            if (!new_data) break;
            idat_data = new_data;
            if (fread(idat_data + idat_len, 1, chunk_len, f) != chunk_len) break;
            idat_len += chunk_len;
        } else if (strcmp(type, "IEND") == 0) {
            break;
        } else {
            /* Skip unknown chunk */
            fseek(f, chunk_len, SEEK_CUR);
        }
        /* Skip CRC */
        fseek(f, 4, SEEK_CUR);
    }
    fclose(f);

    if (!idat_data || width == 0 || height == 0) {
        free(idat_data);
        return NULL;
    }

    /* Decompress */
    uLong raw_len = (width * 4 + 1) * height;
    uint8_t* raw = (uint8_t*)malloc(raw_len);
    if (!raw) { free(idat_data); return NULL; }

    if (uncompress(raw, &raw_len, idat_data, idat_len) != Z_OK) {
        free(raw); free(idat_data); return NULL;
    }
    free(idat_data);

    JiScreenshotImage* img = ji_ss_image_new(width, height);
    if (!img) { free(raw); return NULL; }

    /* Strip filter bytes */
    for (uint32_t y = 0; y < height; y++) {
        memcpy(img->pixels + y * img->stride,
               raw + y * (width * 4 + 1) + 1, width * 4);
    }

    free(raw);
    return img;
}

/* =========================================================================
 * Pixel Comparison
 * ========================================================================= */

int ji_ss_compare(const JiScreenshotImage* a, const JiScreenshotImage* b,
                    uint8_t tolerance, JiScreenshotDiff* out_diff)
{
    if (!a || !b) return -1;
    if (a->width != b->width || a->height != b->height) return -1;

    JiScreenshotDiff diff;
    diff.total_pixels = (uint64_t)a->width * a->height;
    diff.different_pixels = 0;
    diff.max_diff = 0;
    diff.mean_diff = 0.0;
    diff.diff_percentage = 0.0;

    uint64_t total_diff = 0;

    for (uint32_t y = 0; y < a->height; y++) {
        for (uint32_t x = 0; x < a->width; x++) {
            uint32_t idx = y * a->stride + x * 4;
            uint32_t pixel_diff = 0;
            for (int c = 0; c < 4; c++) {
                int d = (int)a->pixels[idx + c] - (int)b->pixels[idx + c];
                if (d < 0) d = -d;
                if ((uint32_t)d > diff.max_diff) diff.max_diff = d;
                pixel_diff += (uint32_t)d;
            }
            pixel_diff /= 4;
            total_diff += pixel_diff;
            if (pixel_diff > tolerance) {
                diff.different_pixels++;
            }
        }
    }

    if (diff.total_pixels > 0) {
        diff.mean_diff = (double)total_diff / (double)diff.total_pixels;
        diff.diff_percentage = (double)diff.different_pixels * 100.0 / (double)diff.total_pixels;
    }

    if (out_diff) *out_diff = diff;

    return diff.different_pixels > 0 ? 1 : 0;
}

bool ji_ss_images_equal(const JiScreenshotImage* a, const JiScreenshotImage* b)
{
    if (!a || !b) return false;
    if (a->width != b->width || a->height != b->height) return false;
    return memcmp(a->pixels, b->pixels, (size_t)a->width * a->height * 4) == 0;
}

JiScreenshotImage* ji_ss_diff_image(const JiScreenshotImage* a, const JiScreenshotImage* b,
                                      uint8_t tolerance)
{
    if (!a || !b) return NULL;
    if (a->width != b->width || a->height != b->height) return NULL;

    JiScreenshotImage* diff = ji_ss_image_new(a->width, a->height);
    if (!diff) return NULL;

    for (uint32_t y = 0; y < a->height; y++) {
        for (uint32_t x = 0; x < a->width; x++) {
            uint32_t idx = y * a->stride + x * 4;
            uint32_t pixel_diff = 0;
            for (int c = 0; c < 4; c++) {
                int d = (int)a->pixels[idx + c] - (int)b->pixels[idx + c];
                if (d < 0) d = -d;
                pixel_diff += (uint32_t)d;
            }
            pixel_diff /= 4;
            if (pixel_diff > tolerance) {
                /* Highlight in red */
                ji_ss_image_set_pixel(diff, x, y, 255, 0, 0, 255);
            } else {
                /* Copy original */
                ji_ss_image_set_pixel(diff, x, y,
                    a->pixels[idx], a->pixels[idx+1], a->pixels[idx+2], a->pixels[idx+3]);
            }
        }
    }
    return diff;
}

/* =========================================================================
 * Baseline Management
 * ========================================================================= */

static void ensure_dir(const char* path)
{
    if (!path) return;
    mkdir(path, 0755);
}

JiScreenshotTestRunner* ji_ss_runner_new(const char* baseline_dir, const char* output_dir)
{
    JiScreenshotTestRunner* r = (JiScreenshotTestRunner*)calloc(1, sizeof(JiScreenshotTestRunner));
    if (!r) return NULL;
    if (baseline_dir) {
        strncpy(r->baseline_dir, baseline_dir, sizeof(r->baseline_dir) - 1);
        ensure_dir(baseline_dir);
    }
    if (output_dir) {
        strncpy(r->output_dir, output_dir, sizeof(r->output_dir) - 1);
        ensure_dir(output_dir);
    }
    return r;
}

void ji_ss_runner_free(JiScreenshotTestRunner* runner)
{
    if (!runner) return;
    for (int i = 0; i < runner->baseline_count; i++) {
        if (runner->baselines[i].image) {
            ji_ss_image_free(runner->baselines[i].image);
        }
    }
    free(runner);
}

int ji_ss_runner_add_baseline(JiScreenshotTestRunner* runner, const char* name,
                                const char* image_path, uint8_t tolerance)
{
    if (!runner || !name || !image_path) return -1;
    if (runner->baseline_count >= JI_SS_MAX_BASELINES) return -1;

    JiScreenshotImage* img = ji_ss_image_load_png(image_path);
    if (!img) return -1;

    JiScreenshotBaseline* bl = &runner->baselines[runner->baseline_count];
    strncpy(bl->name, name, JI_SS_MAX_NAME_LEN - 1);
    bl->name[JI_SS_MAX_NAME_LEN - 1] = '\0';
    bl->image = img;
    bl->tolerance = tolerance;
    bl->valid = true;
    return runner->baseline_count++;
}

static JiScreenshotBaseline* find_baseline(JiScreenshotTestRunner* runner, const char* name)
{
    for (int i = 0; i < runner->baseline_count; i++) {
        if (strcmp(runner->baselines[i].name, name) == 0) {
            return &runner->baselines[i];
        }
    }
    return NULL;
}

int ji_ss_runner_test(JiScreenshotTestRunner* runner, const char* name,
                        const JiScreenshotImage* actual)
{
    if (!runner || !name || !actual) return -1;

    runner->tests_run++;

    JiScreenshotBaseline* bl = find_baseline(runner, name);
    if (!bl || !bl->image) {
        runner->tests_failed++;
        return -1;
    }

    JiScreenshotDiff diff;
    int rc = ji_ss_compare(actual, bl->image, bl->tolerance, &diff);
    if (rc == 0) {
        runner->tests_passed++;
        return 0;
    } else {
        runner->tests_failed++;
        return 1;
    }
}

int ji_ss_runner_test_and_save(JiScreenshotTestRunner* runner, const char* name,
                                 const JiScreenshotImage* actual)
{
    int rc = ji_ss_runner_test(runner, name, actual);
    if (rc != 0 && runner->output_dir[0]) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s_fail.png", runner->output_dir, name);
        ji_ss_image_save_png(actual, path);
    }
    return rc;
}

void ji_ss_runner_get_results(const JiScreenshotTestRunner* runner,
                                int* run, int* passed, int* failed)
{
    if (!runner) return;
    if (run) *run = runner->tests_run;
    if (passed) *passed = runner->tests_passed;
    if (failed) *failed = runner->tests_failed;
}

void ji_ss_runner_print_results(const JiScreenshotTestRunner* runner)
{
    if (!runner) return;
    printf("=== Screenshot Test Results ===\n");
    printf("  Run:    %d\n", runner->tests_run);
    printf("  Passed: %d\n", runner->tests_passed);
    printf("  Failed: %d\n", runner->tests_failed);
    if (runner->tests_run > 0) {
        printf("  Rate:   %.1f%%\n",
               (double)runner->tests_passed * 100.0 / (double)runner->tests_run);
    }
}

int ji_ss_runner_save_baseline(JiScreenshotTestRunner* runner, const char* name,
                                 const JiScreenshotImage* image)
{
    if (!runner || !name || !image) return -1;
    if (runner->baseline_count >= JI_SS_MAX_BASELINES) return -1;

    /* Save to file */
    if (runner->baseline_dir[0]) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.png", runner->baseline_dir, name);
        ji_ss_image_save_png(image, path);
    }

    /* Store in memory */
    JiScreenshotBaseline* bl = &runner->baselines[runner->baseline_count];
    strncpy(bl->name, name, JI_SS_MAX_NAME_LEN - 1);
    bl->name[JI_SS_MAX_NAME_LEN - 1] = '\0';
    bl->image = ji_ss_image_from_data(image->pixels, image->width, image->height);
    bl->tolerance = 0;
    bl->valid = true;
    return runner->baseline_count++;
}

int ji_ss_runner_update_baseline(JiScreenshotTestRunner* runner, const char* name,
                                   const JiScreenshotImage* image)
{
    if (!runner || !name || !image) return -1;
    JiScreenshotBaseline* bl = find_baseline(runner, name);
    if (!bl) return -1;

    if (bl->image) ji_ss_image_free(bl->image);
    bl->image = ji_ss_image_from_data(image->pixels, image->width, image->height);

    if (runner->baseline_dir[0]) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.png", runner->baseline_dir, name);
        ji_ss_image_save_png(image, path);
    }
    return 0;
}
