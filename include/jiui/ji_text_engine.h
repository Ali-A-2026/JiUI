/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_text_engine.h
 * @brief Text shaping engine — HarfBuzz integration, BiDi, ligatures,
 *        subpixel rendering, GPU text atlas.
 */

#ifndef JIUI_TEXT_ENGINE_H
#define JIUI_TEXT_ENGINE_H

#include "ji_font.h"
#include "ji_gpu.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Text Direction
 * ========================================================================= */

typedef enum JiTextDirection {
    JI_TEXT_DIR_LTR,
    JI_TEXT_DIR_RTL,
    JI_TEXT_DIR_AUTO
} JiTextDirection;

typedef enum JiTextAlign {
    JI_TEXT_ALIGN_LEFT,
    JI_TEXT_ALIGN_CENTER,
    JI_TEXT_ALIGN_RIGHT,
    JI_TEXT_ALIGN_JUSTIFY
} JiTextAlign;

typedef enum JiTextWrap {
    JI_TEXT_WRAP_NONE,
    JI_TEXT_WRAP_WORD,
    JI_TEXT_WRAP_CHARACTER,
    JI_TEXT_WRAP_WORD_CHARACTER
} JiTextWrap;

/* =========================================================================
 * Shaped Glyph
 * ========================================================================= */

typedef struct JiShapedGlyph {
    uint32_t    glyph_index;
    uint32_t    codepoint;
    float       x_offset;
    float       y_offset;
    float       x_advance;
    float       y_advance;
    uint32_t    cluster;
    bool        is_rtl;
    JiFontFace* font_face;
} JiShapedGlyph;

/* =========================================================================
 * Shaped Text Run
 * ========================================================================= */

typedef struct JiShapedRun {
    JiShapedGlyph* glyphs;
    uint32_t       glyph_count;
    JiFontFace*    font_face;
    JiTextDirection direction;
    uint32_t       start_index;
    uint32_t       length;
} JiShapedRun;

/* =========================================================================
 * Text Layout
 * ========================================================================= */

typedef struct JiTextLayout JiTextLayout;

JiTextLayout* ji_text_layout_create(void);
void          ji_text_layout_destroy(JiTextLayout* layout);

void          ji_text_layout_set_text(JiTextLayout* layout, const char* text, uint32_t length);
void          ji_text_layout_set_font(JiTextLayout* layout, JiFont* font);
void          ji_text_layout_set_font_chain(JiTextLayout* layout, JiFontChain* chain);
void          ji_text_layout_set_direction(JiTextLayout* layout, JiTextDirection dir);
void          ji_text_layout_set_alignment(JiTextLayout* layout, JiTextAlign align);
void          ji_text_layout_set_wrap(JiTextLayout* layout, JiTextWrap wrap);
void          ji_text_layout_set_max_width(JiTextLayout* layout, float max_width);
void          ji_text_layout_set_line_spacing(JiTextLayout* layout, float spacing);

/* Shape the text — runs HarfBuzz + BiDi */
bool          ji_text_layout_shape(JiTextLayout* layout);

/* Get shaped runs */
const JiShapedRun* ji_text_layout_get_runs(const JiTextLayout* layout, uint32_t* count);

/* Metrics */
float         ji_text_layout_get_width(const JiTextLayout* layout);
float         ji_text_layout_get_height(const JiTextLayout* layout);
uint32_t      ji_text_layout_get_line_count(const JiTextLayout* layout);
float         ji_text_layout_get_line_height(const JiTextLayout* layout, uint32_t line);

/* Hit testing */
uint32_t      ji_text_layout_hit_test(const JiTextLayout* layout, float x, float y);
void          ji_text_layout_get_cursor_pos(const JiTextLayout* layout, uint32_t index,
                                             float* x, float* y, float* height);

/* =========================================================================
 * Text Atlas (GPU)
 * ========================================================================= */

typedef struct JiTextAtlas JiTextAtlas;

JiTextAtlas*  ji_text_atlas_create(JiGpuDevice* device, uint32_t width, uint32_t height);
void          ji_text_atlas_destroy(JiTextAtlas* atlas);

/* Cache a glyph in the atlas */
bool          ji_text_atlas_cache_glyph(JiTextAtlas* atlas, JiFontFace* face,
                                          uint32_t glyph_index, float font_size);

/* Get UV coords for a cached glyph */
bool          ji_text_atlas_get_glyph_uv(const JiTextAtlas* atlas, JiFontFace* face,
                                            uint32_t glyph_index, float font_size,
                                            float* u0, float* v0, float* u1, float* v1);

/* Get the atlas texture for rendering */
JiGpuTexture* ji_text_atlas_get_texture(const JiTextAtlas* atlas);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TEXT_ENGINE_H */
