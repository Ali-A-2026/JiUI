/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_font.h"
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Font Face — software fallback (no FreeType/HarfBuzz yet)
 * ========================================================================= */

struct JiFontFace {
    char*          family;
    char*          style_name;
    JiFontStyle    style;
    JiFontWeight   weight;
    JiFontStretch  stretch;
    bool           is_variable;
    JiFontMetrics  metrics;
    void*          data;       /* Font file data */
    uint32_t       data_size;
    uint32_t       index;
};

JiFontFace* ji_font_face_create(const char* path, uint32_t index) {
    (void)path;
    JiFontFace* face = calloc(1, sizeof(JiFontFace));
    if (!face) return NULL;
    face->family = strdup("Default");
    face->style_name = strdup("Regular");
    face->style = JI_FONT_STYLE_NORMAL;
    face->weight = JI_FONT_WEIGHT_NORMAL;
    face->stretch = JI_FONT_STRETCH_NORMAL;
    face->is_variable = false;
    face->metrics.ascent = 0.8f;
    face->metrics.descent = 0.2f;
    face->metrics.line_gap = 0.0f;
    face->metrics.units_per_em = 1000.0f;
    face->metrics.x_height = 0.5f;
    face->metrics.cap_height = 0.7f;
    face->metrics.underline_position = -0.1f;
    face->metrics.underline_thickness = 0.05f;
    face->index = index;
    return face;
}

JiFontFace* ji_font_face_create_from_memory(const void* data, uint32_t size, uint32_t index) {
    JiFontFace* face = ji_font_face_create(NULL, index);
    if (face && data && size > 0) {
        face->data = malloc(size);
        if (face->data) {
            memcpy(face->data, data, size);
            face->data_size = size;
        }
    }
    return face;
}

void ji_font_face_destroy(JiFontFace* face) {
    if (!face) return;
    free(face->family);
    free(face->style_name);
    free(face->data);
    free(face);
}

const char*  ji_font_face_get_family(const JiFontFace* face) { return face ? face->family : "Unknown"; }
const char*  ji_font_face_get_style_name(const JiFontFace* face) { return face ? face->style_name : "Regular"; }
JiFontStyle  ji_font_face_get_style(const JiFontFace* face) { return face ? face->style : JI_FONT_STYLE_NORMAL; }
JiFontWeight ji_font_face_get_weight(const JiFontFace* face) { return face ? face->weight : JI_FONT_WEIGHT_NORMAL; }
JiFontStretch ji_font_face_get_stretch(const JiFontFace* face) { return face ? face->stretch : JI_FONT_STRETCH_NORMAL; }
bool         ji_font_face_is_variable(const JiFontFace* face) { return face ? face->is_variable : false; }

JiFontMetrics ji_font_face_get_metrics(const JiFontFace* face, float font_size) {
    if (!face) return (JiFontMetrics){0};
    JiFontMetrics m = face->metrics;
    float scale = font_size / m.units_per_em;
    m.ascent *= scale;
    m.descent *= scale;
    m.line_gap *= scale;
    m.x_height *= scale;
    m.cap_height *= scale;
    m.underline_position *= scale;
    m.underline_thickness *= scale;
    return m;
}

uint32_t ji_font_face_get_glyph_count(const JiFontFace* face) {
    (void)face;
    return 256; /* Placeholder */
}

uint32_t ji_font_face_get_glyph_index(const JiFontFace* face, uint32_t codepoint) {
    (void)face;
    return codepoint < 256 ? codepoint : 0;
}

JiGlyphInfo ji_font_face_get_glyph_info(const JiFontFace* face, uint32_t glyph_index, float font_size) {
    (void)face; (void)glyph_index;
    JiGlyphInfo info = {0};
    info.codepoint = glyph_index;
    info.glyph_index = glyph_index;
    info.advance_x = font_size * 0.6f;
    info.advance_y = 0;
    info.x_offset = 0;
    info.y_offset = 0;
    info.bounds = (JiRectF){0, -font_size * 0.8f, font_size * 0.6f, font_size};
    info.is_color = false;
    return info;
}

/* =========================================================================
 * Font
 * ========================================================================= */

struct JiFont {
    JiFontFace* face;
    float       size;
};

JiFont* ji_font_create(JiFontFace* face, float size) {
    JiFont* font = calloc(1, sizeof(JiFont));
    if (!font) return NULL;
    font->face = face;
    font->size = size > 0 ? size : 12.0f;
    return font;
}

void ji_font_destroy(JiFont* font) { free(font); }

float        ji_font_get_size(const JiFont* font) { return font ? font->size : 0; }
JiFontFace*  ji_font_get_face(const JiFont* font) { return font ? font->face : NULL; }

JiFontMetrics ji_font_get_metrics(const JiFont* font) {
    return font ? ji_font_face_get_metrics(font->face, font->size) : (JiFontMetrics){0};
}

float ji_font_get_line_height(const JiFont* font) {
    if (!font) return 0;
    JiFontMetrics m = ji_font_get_metrics(font);
    return m.ascent + m.descent + m.line_gap;
}

float ji_font_get_text_width(const JiFont* font, const char* text, uint32_t length) {
    if (!font || !text) return 0;
    float width = 0;
    for (uint32_t i = 0; i < length && text[i]; i++) {
        width += font->size * 0.6f;
    }
    return width;
}

/* =========================================================================
 * Font Fallback Chain
 * ========================================================================= */

struct JiFontChain {
    JiFontFace** faces;
    uint32_t     count;
    uint32_t     capacity;
};

JiFontChain* ji_font_chain_create(void) {
    JiFontChain* chain = calloc(1, sizeof(JiFontChain));
    if (!chain) return NULL;
    chain->capacity = 8;
    chain->faces = calloc(chain->capacity, sizeof(JiFontFace*));
    return chain;
}

void ji_font_chain_destroy(JiFontChain* chain) {
    if (!chain) return;
    free(chain->faces);
    free(chain);
}

void ji_font_chain_add(JiFontChain* chain, JiFontFace* face) {
    if (!chain || !face) return;
    if (chain->count >= chain->capacity) {
        chain->capacity *= 2;
        chain->faces = realloc(chain->faces, chain->capacity * sizeof(JiFontFace*));
    }
    chain->faces[chain->count++] = face;
}

JiFontFace* ji_font_chain_find_glyph(const JiFontChain* chain, uint32_t codepoint) {
    if (!chain || chain->count == 0) return NULL;
    for (uint32_t i = 0; i < chain->count; i++) {
        if (ji_font_face_get_glyph_index(chain->faces[i], codepoint) != 0) {
            return chain->faces[i];
        }
    }
    return chain->faces[0]; /* Fallback to first */
}

uint32_t ji_font_chain_count(const JiFontChain* chain) { return chain ? chain->count : 0; }
