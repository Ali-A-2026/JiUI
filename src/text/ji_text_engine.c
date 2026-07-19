/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_text_engine.h"
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Text Layout
 * ========================================================================= */

struct JiTextLayout {
    char*           text;
    uint32_t        text_length;
    JiFont*         font;
    JiFontChain*    font_chain;
    JiTextDirection direction;
    JiTextAlign     alignment;
    JiTextWrap      wrap;
    float           max_width;
    float           line_spacing;
    JiShapedRun*    runs;
    uint32_t        run_count;
    float           width;
    float           height;
    uint32_t        line_count;
    bool            shaped;
};

JiTextLayout* ji_text_layout_create(void) {
    JiTextLayout* layout = calloc(1, sizeof(JiTextLayout));
    if (!layout) return NULL;
    layout->direction = JI_TEXT_DIR_AUTO;
    layout->alignment = JI_TEXT_ALIGN_LEFT;
    layout->wrap = JI_TEXT_WRAP_WORD;
    layout->max_width = -1.0f;
    layout->line_spacing = 1.0f;
    return layout;
}

void ji_text_layout_destroy(JiTextLayout* layout) {
    if (!layout) return;
    free(layout->text);
    for (uint32_t i = 0; i < layout->run_count; i++) {
        free(layout->runs[i].glyphs);
    }
    free(layout->runs);
    free(layout);
}

void ji_text_layout_set_text(JiTextLayout* layout, const char* text, uint32_t length) {
    if (!layout) return;
    free(layout->text);
    layout->text = length > 0 ? strndup(text, length) : NULL;
    layout->text_length = length;
    layout->shaped = false;
}

void ji_text_layout_set_font(JiTextLayout* layout, JiFont* font) {
    if (layout) { layout->font = font; layout->shaped = false; }
}

void ji_text_layout_set_font_chain(JiTextLayout* layout, JiFontChain* chain) {
    if (layout) { layout->font_chain = chain; layout->shaped = false; }
}

void ji_text_layout_set_direction(JiTextLayout* layout, JiTextDirection dir) {
    if (layout) { layout->direction = dir; layout->shaped = false; }
}

void ji_text_layout_set_alignment(JiTextLayout* layout, JiTextAlign align) {
    if (layout) layout->alignment = align;
}

void ji_text_layout_set_wrap(JiTextLayout* layout, JiTextWrap wrap) {
    if (layout) { layout->wrap = wrap; layout->shaped = false; }
}

void ji_text_layout_set_max_width(JiTextLayout* layout, float max_width) {
    if (layout) { layout->max_width = max_width; layout->shaped = false; }
}

void ji_text_layout_set_line_spacing(JiTextLayout* layout, float spacing) {
    if (layout) layout->line_spacing = spacing;
}

bool ji_text_layout_shape(JiTextLayout* layout) {
    if (!layout || !layout->text) return false;
    /* Free old runs */
    for (uint32_t i = 0; i < layout->run_count; i++) {
        free(layout->runs[i].glyphs);
    }
    free(layout->runs);
    layout->runs = NULL;
    layout->run_count = 0;

    /* Simple shaping: one run for the whole text */
    layout->runs = calloc(1, sizeof(JiShapedRun));
    if (!layout->runs) return false;
    layout->run_count = 1;

    JiShapedRun* run = &layout->runs[0];
    run->glyph_count = layout->text_length;
    run->glyphs = calloc(run->glyph_count, sizeof(JiShapedGlyph));
    run->font_face = layout->font ? ji_font_get_face(layout->font) : NULL;
    run->direction = layout->direction == JI_TEXT_DIR_AUTO ? JI_TEXT_DIR_LTR : layout->direction;
    run->start_index = 0;
    run->length = layout->text_length;

    float x_pos = 0;
    float font_size = layout->font ? ji_font_get_size(layout->font) : 12.0f;
    for (uint32_t i = 0; i < run->glyph_count; i++) {
        run->glyphs[i].codepoint = (uint8_t)layout->text[i];
        run->glyphs[i].glyph_index = (uint8_t)layout->text[i];
        run->glyphs[i].x_advance = font_size * 0.6f;
        run->glyphs[i].y_advance = 0;
        run->glyphs[i].x_offset = 0;
        run->glyphs[i].y_offset = 0;
        run->glyphs[i].cluster = i;
        run->glyphs[i].is_rtl = (run->direction == JI_TEXT_DIR_RTL);
        run->glyphs[i].font_face = run->font_face;
        x_pos += run->glyphs[i].x_advance;
    }

    layout->width = x_pos;
    float line_height = layout->font ? ji_font_get_line_height(layout->font) : font_size;
    layout->height = line_height;
    layout->line_count = 1;
    layout->shaped = true;
    return true;
}

const JiShapedRun* ji_text_layout_get_runs(const JiTextLayout* layout, uint32_t* count) {
    if (count) *count = layout ? layout->run_count : 0;
    return layout ? layout->runs : NULL;
}

float ji_text_layout_get_width(const JiTextLayout* layout) { return layout ? layout->width : 0; }
float ji_text_layout_get_height(const JiTextLayout* layout) { return layout ? layout->height : 0; }
uint32_t ji_text_layout_get_line_count(const JiTextLayout* layout) { return layout ? layout->line_count : 0; }

float ji_text_layout_get_line_height(const JiTextLayout* layout, uint32_t line) {
    (void)line;
    return layout && layout->font ? ji_font_get_line_height(layout->font) : 0;
}

uint32_t ji_text_layout_hit_test(const JiTextLayout* layout, float x, float y) {
    if (!layout || !layout->text) return 0;
    float font_size = layout->font ? ji_font_get_size(layout->font) : 12.0f;
    float char_width = font_size * 0.6f;
    if (char_width <= 0) return 0;
    uint32_t col = (uint32_t)(x / char_width);
    uint32_t row = (uint32_t)(y / (font_size * 1.2f));
    uint32_t chars_per_line = layout->max_width > 0 ? (uint32_t)(layout->max_width / char_width) : layout->text_length;
    if (chars_per_line == 0) chars_per_line = 1;
    uint32_t index = row * chars_per_line + col;
    return index < layout->text_length ? index : layout->text_length;
}

void ji_text_layout_get_cursor_pos(const JiTextLayout* layout, uint32_t index,
                                     float* x, float* y, float* height) {
    if (!layout || !layout->text) { *x = *y = *height = 0; return; }
    float font_size = layout->font ? ji_font_get_size(layout->font) : 12.0f;
    float char_width = font_size * 0.6f;
    float line_height = font_size * 1.2f;
    float max_w = layout->max_width > 0 ? layout->max_width : 1e9f;
    uint32_t chars_per_line = (uint32_t)(max_w / char_width);
    if (chars_per_line == 0) chars_per_line = 1;
    uint32_t row = index / chars_per_line;
    uint32_t col = index % chars_per_line;
    *x = col * char_width;
    *y = row * line_height;
    *height = line_height;
}

/* =========================================================================
 * Text Atlas
 * ========================================================================= */

struct JiTextAtlas {
    JiGpuDevice*  device;
    uint32_t      width;
    uint32_t      height;
    JiGpuTexture* texture;
    float         cursor_x;
    float         cursor_y;
    float         row_height;
};

JiTextAtlas* ji_text_atlas_create(JiGpuDevice* device, uint32_t width, uint32_t height) {
    JiTextAtlas* atlas = calloc(1, sizeof(JiTextAtlas));
    if (!atlas) return NULL;
    atlas->device = device;
    atlas->width = width;
    atlas->height = height;
    return atlas;
}

void ji_text_atlas_destroy(JiTextAtlas* atlas) {
    if (!atlas) return;
    if (atlas->texture) ji_gpu_texture_destroy(atlas->texture);
    free(atlas);
}

bool ji_text_atlas_cache_glyph(JiTextAtlas* atlas, JiFontFace* face,
                                 uint32_t glyph_index, float font_size) {
    if (!atlas) return false;
    (void)face; (void)glyph_index; (void)font_size;
    /* Advance cursor in atlas */
    float glyph_w = font_size * 0.7f;
    float glyph_h = font_size * 1.2f;
    atlas->cursor_x += glyph_w;
    if (atlas->cursor_x + glyph_w > atlas->width) {
        atlas->cursor_x = 0;
        atlas->cursor_y += atlas->row_height;
        atlas->row_height = 0;
    }
    if (glyph_h > atlas->row_height) atlas->row_height = glyph_h;
    return true;
}

bool ji_text_atlas_get_glyph_uv(const JiTextAtlas* atlas, JiFontFace* face,
                                   uint32_t glyph_index, float font_size,
                                   float* u0, float* v0, float* u1, float* v1) {
    if (!atlas) return false;
    (void)face; (void)glyph_index; (void)font_size;
    *u0 = 0; *v0 = 0; *u1 = 1; *v1 = 1;
    return true;
}

JiGpuTexture* ji_text_atlas_get_texture(const JiTextAtlas* atlas) {
    return atlas ? atlas->texture : NULL;
}
