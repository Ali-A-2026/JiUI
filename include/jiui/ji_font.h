/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_font.h
 * @brief Font management — font loading, font fallback chains, variable fonts.
 */

#ifndef JIUI_FONT_H
#define JIUI_FONT_H

#include "ji_types.h"
#include "ji_scene.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Font Types
 * ========================================================================= */

typedef enum JiFontStyle {
    JI_FONT_STYLE_NORMAL,
    JI_FONT_STYLE_ITALIC,
    JI_FONT_STYLE_OBLIQUE
} JiFontStyle;

typedef enum JiFontWeight {
    JI_FONT_WEIGHT_THIN       = 100,
    JI_FONT_WEIGHT_LIGHT      = 300,
    JI_FONT_WEIGHT_NORMAL     = 400,
    JI_FONT_WEIGHT_MEDIUM     = 500,
    JI_FONT_WEIGHT_SEMIBOLD   = 600,
    JI_FONT_WEIGHT_BOLD       = 700,
    JI_FONT_WEIGHT_EXTRABOLD  = 800,
    JI_FONT_WEIGHT_BLACK      = 900
} JiFontWeight;

typedef enum JiFontStretch {
    JI_FONT_STRETCH_NORMAL      = 0,
    JI_FONT_STRETCH_CONDENSED   = -1,
    JI_FONT_STRETCH_EXPANDED    = 1
} JiFontStretch;

typedef struct JiFontMetrics {
    float    ascent;
    float    descent;
    float    line_gap;
    float    units_per_em;
    float    x_height;
    float    cap_height;
    float    underline_position;
    float    underline_thickness;
} JiFontMetrics;

typedef struct JiGlyphInfo {
    uint32_t codepoint;
    uint32_t glyph_index;
    float    advance_x;
    float    advance_y;
    float    x_offset;
    float    y_offset;
    JiRectF  bounds;
    bool     is_color;
} JiGlyphInfo;

/* =========================================================================
 * Font Face
 * ========================================================================= */

typedef struct JiFontFace JiFontFace;

JiFontFace*  ji_font_face_create(const char* path, uint32_t index);
JiFontFace*  ji_font_face_create_from_memory(const void* data, uint32_t size, uint32_t index);
void         ji_font_face_destroy(JiFontFace* face);

const char*  ji_font_face_get_family(const JiFontFace* face);
const char*  ji_font_face_get_style_name(const JiFontFace* face);
JiFontStyle  ji_font_face_get_style(const JiFontFace* face);
JiFontWeight ji_font_face_get_weight(const JiFontFace* face);
JiFontStretch ji_font_face_get_stretch(const JiFontFace* face);
bool         ji_font_face_is_variable(const JiFontFace* face);
JiFontMetrics ji_font_face_get_metrics(const JiFontFace* face, float font_size);

uint32_t     ji_font_face_get_glyph_count(const JiFontFace* face);
uint32_t     ji_font_face_get_glyph_index(const JiFontFace* face, uint32_t codepoint);
JiGlyphInfo  ji_font_face_get_glyph_info(const JiFontFace* face, uint32_t glyph_index, float font_size);

/* =========================================================================
 * Font
 * ========================================================================= */

typedef struct JiFont JiFont;

JiFont*      ji_font_create(JiFontFace* face, float size);
void         ji_font_destroy(JiFont* font);

float        ji_font_get_size(const JiFont* font);
JiFontFace*  ji_font_get_face(const JiFont* font);
JiFontMetrics ji_font_get_metrics(const JiFont* font);

float        ji_font_get_line_height(const JiFont* font);
float        ji_font_get_text_width(const JiFont* font, const char* text, uint32_t length);

/* =========================================================================
 * Font Fallback Chain
 * ========================================================================= */

typedef struct JiFontChain JiFontChain;

JiFontChain* ji_font_chain_create(void);
void         ji_font_chain_destroy(JiFontChain* chain);
void         ji_font_chain_add(JiFontChain* chain, JiFontFace* face);
JiFontFace*  ji_font_chain_find_glyph(const JiFontChain* chain, uint32_t codepoint);
uint32_t     ji_font_chain_count(const JiFontChain* chain);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_FONT_H */
