/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_i18n_bidi.c
 * @brief Unicode Bidirectional Algorithm (UAX #9) — simplified subset.
 *        Detects RTL/LTR runs and reorders for display.
 */

#include "jiui/ji_i18n.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* =========================================================================
 * Unicode property helpers
 * ========================================================================= */

/* Check if a UTF-8 byte sequence starts a strong RTL character.
 * Arabic block: U+0590–U+08FF (Hebrew, Arabic, Syriac, etc.)
 * We check the first code point in a UTF-8 sequence. */
static bool is_rtl_char(uint32_t cp)
{
    /* Hebrew: 0x0590–0x05FF */
    if (cp >= 0x0590 && cp <= 0x05FF) return true;
    /* Arabic: 0x0600–0x06FF */
    if (cp >= 0x0600 && cp <= 0x06FF) return true;
    /* Syriac: 0x0700–0x074F */
    if (cp >= 0x0700 && cp <= 0x074F) return true;
    /* Arabic Supplement: 0x0750–0x077F */
    if (cp >= 0x0750 && cp <= 0x077F) return true;
    /* Thaana: 0x0780–0x07BF */
    if (cp >= 0x0780 && cp <= 0x07BF) return true;
    /* Nko: 0x07C0–0x07FF */
    if (cp >= 0x07C0 && cp <= 0x07FF) return true;
    /* RTL presentation forms: 0xFB1D–0xFEFF */
    if (cp >= 0xFB1D && cp <= 0xFEFF) return true;
    return false;
}

static bool is_ltr_char(uint32_t cp)
{
    /* Basic Latin letters are LTR */
    if ((cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z')) return true;
    /* Latin-1 supplement letters */
    if (cp >= 0x00C0 && cp <= 0x024F) return true;
    /* Greek and Coptic */
    if (cp >= 0x0370 && cp <= 0x03FF) return true;
    /* Cyrillic */
    if (cp >= 0x0400 && cp <= 0x04FF) return true;
    return false;
}

/* Decode one UTF-8 code point at p, store in *cp, return byte length */
static int utf8_decode(const unsigned char* p, uint32_t* cp)
{
    if (!p || !cp) return 0;
    if (p[0] < 0x80) {
        *cp = p[0];
        return 1;
    } else if ((p[0] & 0xE0) == 0xC0 && p[1]) {
        *cp = ((uint32_t)(p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        return 2;
    } else if ((p[0] & 0xF0) == 0xE0 && p[1] && p[2]) {
        *cp = ((uint32_t)(p[0] & 0x0F) << 12) |
              ((uint32_t)(p[1] & 0x3F) << 6) |
              (p[2] & 0x3F);
        return 3;
    } else if ((p[0] & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
        *cp = ((uint32_t)(p[0] & 0x07) << 18) |
              ((uint32_t)(p[1] & 0x3F) << 12) |
              ((uint32_t)(p[2] & 0x3F) << 6) |
              (p[3] & 0x3F);
        return 4;
    }
    *cp = p[0];
    return 1;
}

/* =========================================================================
 * Direction Detection
 * ========================================================================= */

JiTextDirection ji_i18n_bidi_detect_direction(const char* text)
{
    if (!text) return JI_TEXT_DIR_LTR;
    const unsigned char* p = (const unsigned char*)text;
    while (*p) {
        uint32_t cp = 0;
        int len = utf8_decode(p, &cp);
        if (len == 0) break;
        if (is_rtl_char(cp)) return JI_TEXT_DIR_RTL;
        if (is_ltr_char(cp)) return JI_TEXT_DIR_LTR;
        p += len;
    }
    return JI_TEXT_DIR_LTR;
}

/* =========================================================================
 * Bidi Reordering (simplified)
 *
 * This implements a basic run-based reordering:
 * 1. Split text into directional runs (LTR and RTL segments)
 * 2. For RTL base direction, reverse RTL runs and keep LTR runs
 * 3. For LTR base direction, keep LTR runs and reverse RTL runs in place
 * ========================================================================= */

/* A directional run */
typedef struct {
    int start;        /* Byte offset in input */
    int length;       /* Byte length */
    bool is_rtl;
} BidiRun;

int ji_i18n_bidi_process(const char* input, char* out, int out_size,
                           JiTextDirection base_dir)
{
    if (!input || !out || out_size <= 0) return -1;
    int input_len = (int)strlen(input);
    if (input_len == 0) {
        out[0] = '\0';
        return 0;
    }

    /* Auto-detect base direction */
    if (base_dir == JI_TEXT_DIR_AUTO) {
        base_dir = ji_i18n_bidi_detect_direction(input);
    }

    /* Collect runs */
    BidiRun runs[256];
    int run_count = 0;
    int i = 0;
    const unsigned char* p = (const unsigned char*)input;

    while (i < input_len && run_count < 256) {
        int run_start = i;
        bool run_is_rtl = false;
        uint32_t cp = 0;
        int len = utf8_decode(p + i, &cp);
        if (len == 0) break;

        if (is_rtl_char(cp)) {
            run_is_rtl = true;
            /* Extend run while RTL or neutral */
            while (i < input_len) {
                len = utf8_decode(p + i, &cp);
                if (len == 0) break;
                if (is_ltr_char(cp)) break;
                i += len;
            }
        } else {
            run_is_rtl = false;
            /* Extend run while LTR or neutral */
            while (i < input_len) {
                len = utf8_decode(p + i, &cp);
                if (len == 0) break;
                if (is_rtl_char(cp)) break;
                i += len;
            }
        }
        runs[run_count].start = run_start;
        runs[run_count].length = i - run_start;
        runs[run_count].is_rtl = run_is_rtl;
        run_count++;
    }

    /* Build output */
    int oi = 0;
    if (base_dir == JI_TEXT_DIR_RTL) {
        /* RTL base: process runs right-to-left */
        for (int r = run_count - 1; r >= 0; r--) {
            if (oi >= out_size - 1) break;
            if (runs[r].is_rtl) {
                /* Reverse the run character by character (UTF-8 aware) */
                int run_end = runs[r].start + runs[r].length;
                int pos = run_end;
                while (pos > runs[r].start && oi < out_size - 1) {
                    /* Find start of previous character */
                    int char_end = pos;
                    pos--;
                    while (pos > runs[r].start && (p[pos] & 0xC0) == 0x80) pos--;
                    int char_len = char_end - pos;
                    if (oi + char_len < out_size) {
                        memcpy(out + oi, input + pos, char_len);
                        oi += char_len;
                    }
                }
            } else {
                /* LTR run: copy as-is */
                int copy_len = runs[r].length;
                if (oi + copy_len >= out_size) copy_len = out_size - 1 - oi;
                if (copy_len > 0) {
                    memcpy(out + oi, input + runs[r].start, copy_len);
                    oi += copy_len;
                }
            }
        }
    } else {
        /* LTR base: process runs left-to-right, reverse RTL runs in place */
        for (int r = 0; r < run_count; r++) {
            if (oi >= out_size - 1) break;
            if (runs[r].is_rtl) {
                /* Reverse the run */
                int run_end = runs[r].start + runs[r].length;
                int pos = run_end;
                while (pos > runs[r].start && oi < out_size - 1) {
                    int char_end = pos;
                    pos--;
                    while (pos > runs[r].start && (p[pos] & 0xC0) == 0x80) pos--;
                    int char_len = char_end - pos;
                    if (oi + char_len < out_size) {
                        memcpy(out + oi, input + pos, char_len);
                        oi += char_len;
                    }
                }
            } else {
                int copy_len = runs[r].length;
                if (oi + copy_len >= out_size) copy_len = out_size - 1 - oi;
                if (copy_len > 0) {
                    memcpy(out + oi, input + runs[r].start, copy_len);
                    oi += copy_len;
                }
            }
        }
    }

    out[oi] = '\0';
    return oi;
}
