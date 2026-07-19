/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_drawing.c
 * @brief Implementation of drawing primitives — brushes, pens, geometries,
 *        and drawing context convenience wrappers.
 */

#include "jiui/ji_drawing.h"
#include "jiui/ji_memory.h"

#include <string.h>

/* =========================================================================
 * JiBrush
 * ========================================================================= */
JiBrush ji_brush_solid(uint32_t argb) {
    JiBrush b;
    memset(&b, 0, sizeof(JiBrush));
    b.kind = JI_BRUSH_SOLID;
    b.v.color = argb;
    return b;
}

JiBrush ji_brush_linear_gradient(JiPoint start, JiPoint end,
                                   const JiGradientStop* stops, int stop_count) {
    JiBrush b;
    memset(&b, 0, sizeof(JiBrush));
    b.kind = JI_BRUSH_LINEAR_GRAD;
    b.v.linear.start = start;
    b.v.linear.end = end;
    b.v.linear.stop_count = stop_count;
    if (stop_count > 0 && stops) {
        b.v.linear.stops = (JiGradientStop*)ji_alloc((size_t)stop_count * sizeof(JiGradientStop));
        memcpy(b.v.linear.stops, stops, (size_t)stop_count * sizeof(JiGradientStop));
    } else {
        b.v.linear.stops = NULL;
        b.v.linear.stop_count = 0;
    }
    return b;
}

JiBrush ji_brush_radial_gradient(JiPoint center, double rx, double ry,
                                    const JiGradientStop* stops, int stop_count) {
    JiBrush b;
    memset(&b, 0, sizeof(JiBrush));
    b.kind = JI_BRUSH_RADIAL_GRAD;
    b.v.radial.center = center;
    b.v.radial.radius_x = rx;
    b.v.radial.radius_y = ry;
    b.v.radial.stop_count = stop_count;
    if (stop_count > 0 && stops) {
        b.v.radial.stops = (JiGradientStop*)ji_alloc((size_t)stop_count * sizeof(JiGradientStop));
        memcpy(b.v.radial.stops, stops, (size_t)stop_count * sizeof(JiGradientStop));
    } else {
        b.v.radial.stops = NULL;
        b.v.radial.stop_count = 0;
    }
    return b;
}

void ji_brush_destroy(JiBrush* brush) {
    if (!brush) return;
    switch (brush->kind) {
        case JI_BRUSH_LINEAR_GRAD:
            ji_free(brush->v.linear.stops);
            brush->v.linear.stops = NULL;
            break;
        case JI_BRUSH_RADIAL_GRAD:
            ji_free(brush->v.radial.stops);
            brush->v.radial.stops = NULL;
            break;
        default:
            break;
    }
    brush->kind = JI_BRUSH_SOLID;
    brush->v.color = 0;
}

/* =========================================================================
 * JiPen
 * ========================================================================= */
JiPen ji_pen_solid(uint32_t argb, double thickness) {
    JiPen p;
    memset(&p, 0, sizeof(JiPen));
    p.brush = ji_brush_solid(argb);
    p.thickness = thickness;
    p.dash_offset = 0.0;
    p.dash_array = NULL;
    p.dash_count = 0;
    return p;
}

JiPen ji_pen_dashed(uint32_t argb, double thickness,
                      const double* dash_array, int dash_count, double offset) {
    JiPen p;
    memset(&p, 0, sizeof(JiPen));
    p.brush = ji_brush_solid(argb);
    p.thickness = thickness;
    p.dash_offset = offset;
    p.dash_count = dash_count;
    if (dash_count > 0 && dash_array) {
        p.dash_array = (double*)ji_alloc((size_t)dash_count * sizeof(double));
        memcpy(p.dash_array, dash_array, (size_t)dash_count * sizeof(double));
    } else {
        p.dash_array = NULL;
        p.dash_count = 0;
    }
    return p;
}

void ji_pen_destroy(JiPen* pen) {
    if (!pen) return;
    ji_brush_destroy(&pen->brush);
    ji_free(pen->dash_array);
    pen->dash_array = NULL;
    pen->dash_count = 0;
}

/* =========================================================================
 * JiGeometry
 * ========================================================================= */
JiGeometry ji_geometry_rect(JiRect rect) {
    JiGeometry g;
    memset(&g, 0, sizeof(JiGeometry));
    g.kind = JI_GEOM_RECT;
    g.v.rect = rect;
    return g;
}

JiGeometry ji_geometry_rounded_rect(JiRect rect, JiCornerRadius radius) {
    JiGeometry g;
    memset(&g, 0, sizeof(JiGeometry));
    g.kind = JI_GEOM_ROUNDED_RECT;
    g.v.rounded_rect = ji_rounded_rect(rect, radius);
    return g;
}

JiGeometry ji_geometry_ellipse(JiRect bounds) {
    JiGeometry g;
    memset(&g, 0, sizeof(JiGeometry));
    g.kind = JI_GEOM_ELLIPSE;
    g.v.ellipse.bounds = bounds;
    return g;
}

JiGeometry ji_geometry_line(JiPoint start, JiPoint end) {
    JiGeometry g;
    memset(&g, 0, sizeof(JiGeometry));
    g.kind = JI_GEOM_LINE;
    g.v.line.start = start;
    g.v.line.end = end;
    return g;
}

/* =========================================================================
 * JiDrawingContext
 * ========================================================================= */
void ji_drawing_context_init(JiDrawingContext* ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(JiDrawingContext));
}

bool ji_drawing_context_is_valid(const JiDrawingContext* ctx) {
    if (!ctx) return false;
    /* At minimum, fill_rect and clear must be set */
    return ctx->fill_rect != NULL && ctx->clear != NULL;
}

/* ---- Convenience wrappers ---- */

void ji_draw_push_clip(JiDrawingContext* ctx, JiRect clip) {
    if (!ctx || !ctx->push_clip) return;
    ctx->push_clip(ctx, clip);
}

void ji_draw_pop_clip(JiDrawingContext* ctx) {
    if (!ctx || !ctx->pop_clip) return;
    ctx->pop_clip(ctx);
}

void ji_draw_push_transform(JiDrawingContext* ctx, JiMatrix transform) {
    if (!ctx || !ctx->push_transform) return;
    ctx->push_transform(ctx, transform);
}

void ji_draw_pop_transform(JiDrawingContext* ctx) {
    if (!ctx || !ctx->pop_transform) return;
    ctx->pop_transform(ctx);
}

void ji_draw_fill_geometry(JiDrawingContext* ctx,
                            const JiGeometry* geom,
                            const JiBrush* brush) {
    if (!ctx || !ctx->fill_geometry) return;
    ctx->fill_geometry(ctx, geom, brush);
}

void ji_draw_stroke_geometry(JiDrawingContext* ctx,
                               const JiGeometry* geom,
                               const JiPen* pen) {
    if (!ctx || !ctx->stroke_geometry) return;
    ctx->stroke_geometry(ctx, geom, pen);
}

void ji_draw_line(JiDrawingContext* ctx,
                    JiPoint start, JiPoint end,
                    const JiBrush* brush, double thickness) {
    if (!ctx || !ctx->draw_line) return;
    ctx->draw_line(ctx, start, end, brush, thickness);
}

void ji_draw_fill_rect(JiDrawingContext* ctx,
                         JiRect rect, const JiBrush* brush) {
    if (!ctx || !ctx->fill_rect) return;
    ctx->fill_rect(ctx, rect, brush);
}

void ji_draw_stroke_rect(JiDrawingContext* ctx,
                            JiRect rect, const JiPen* pen) {
    if (!ctx || !ctx->stroke_rect) return;
    ctx->stroke_rect(ctx, rect, pen);
}

void ji_draw_text(JiDrawingContext* ctx,
                    JiPoint origin, const char* text,
                    const JiBrush* brush,
                    const char* font_family, double font_size) {
    if (!ctx || !ctx->draw_text) return;
    ctx->draw_text(ctx, origin, text, brush, font_family, font_size);
}

void ji_draw_clear(JiDrawingContext* ctx, uint32_t argb) {
    if (!ctx || !ctx->clear) return;
    ctx->clear(ctx, argb);
}

void ji_draw_resize(JiDrawingContext* ctx, int width, int height) {
    if (!ctx || !ctx->resize) return;
    ctx->resize(ctx, width, height);
}

void ji_draw_present(JiDrawingContext* ctx) {
    if (!ctx || !ctx->present) return;
    ctx->present(ctx);
}
