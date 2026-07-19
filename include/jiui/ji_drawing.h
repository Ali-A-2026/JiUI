/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_drawing.h
 * @brief Drawing primitives — brushes, pens, geometries, and drawing context.
 *
 * Provides the rendering abstraction layer. Platform backends implement
 * JiDrawingContext by filling in the function pointers for actual drawing.
 */

#ifndef JIUI_DRAWING_H
#define JIUI_DRAWING_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiBrush — fill description
 * ========================================================================= */
typedef enum JiBrushKind {
    JI_BRUSH_SOLID      = 0,   /* solid color */
    JI_BRUSH_LINEAR_GRAD = 1,  /* linear gradient */
    JI_BRUSH_RADIAL_GRAD = 2,  /* radial gradient */
    JI_BRUSH_IMAGE       = 3   /* image/tile brush (future) */
} JiBrushKind;

/** A single gradient stop. */
typedef struct JiGradientStop {
    double   offset;   /* 0.0 .. 1.0 */
    uint32_t color;    /* ARGB */
} JiGradientStop;

typedef struct JiBrush {
    JiBrushKind kind;
    union {
        /* JI_BRUSH_SOLID */
        uint32_t color;                    /* ARGB */

        /* JI_BRUSH_LINEAR_GRAD */
        struct {
            JiPoint          start;
            JiPoint          end;
            JiGradientStop*  stops;
            int              stop_count;
        } linear;

        /* JI_BRUSH_RADIAL_GRAD */
        struct {
            JiPoint          center;
            double           radius_x;
            double           radius_y;
            JiGradientStop*  stops;
            int              stop_count;
        } radial;
    } v;
} JiBrush;

/** Create a solid-color brush. */
JI_API JiBrush ji_brush_solid(uint32_t argb);

/** Create a linear gradient brush. Stops are copied. */
JI_API JiBrush ji_brush_linear_gradient(JiPoint start, JiPoint end,
                                         const JiGradientStop* stops, int stop_count);

/** Create a radial gradient brush. Stops are copied. */
JI_API JiBrush ji_brush_radial_gradient(JiPoint center, double rx, double ry,
                                          const JiGradientStop* stops, int stop_count);

/** Destroy a brush (frees gradient stop arrays). */
JI_API void ji_brush_destroy(JiBrush* brush);

/* =========================================================================
 * JiPen — stroke description
 * ========================================================================= */
typedef struct JiPen {
    JiBrush brush;
    double  thickness;
    double  dash_offset;
    double* dash_array;     /* NULL for solid line */
    int     dash_count;
} JiPen;

/** Create a solid pen. */
JI_API JiPen ji_pen_solid(uint32_t argb, double thickness);

/** Create a dashed pen. Dash array is copied. */
JI_API JiPen ji_pen_dashed(uint32_t argb, double thickness,
                            const double* dash_array, int dash_count, double offset);

/** Destroy a pen (frees dash array). */
JI_API void ji_pen_destroy(JiPen* pen);

/* =========================================================================
 * JiGeometry — vector shape for fill/stroke
 * ========================================================================= */
typedef enum JiGeometryKind {
    JI_GEOM_RECT           = 0,
    JI_GEOM_ROUNDED_RECT   = 1,
    JI_GEOM_ELLIPSE        = 2,
    JI_GEOM_LINE           = 3,
    JI_GEOM_PATH           = 4     /* arbitrary path (future) */
} JiGeometryKind;

typedef struct JiGeometry {
    JiGeometryKind kind;
    union {
        /* JI_GEOM_RECT */
        JiRect rect;

        /* JI_GEOM_ROUNDED_RECT */
        JiRoundedRect rounded_rect;

        /* JI_GEOM_ELLIPSE */
        struct {
            JiRect bounds;    /* bounding rect of the ellipse */
        } ellipse;

        /* JI_GEOM_LINE */
        struct {
            JiPoint start;
            JiPoint end;
        } line;
    } v;
} JiGeometry;

/** Create a rectangle geometry. */
JI_API JiGeometry ji_geometry_rect(JiRect rect);

/** Create a rounded-rectangle geometry. */
JI_API JiGeometry ji_geometry_rounded_rect(JiRect rect, JiCornerRadius radius);

/** Create an ellipse geometry. */
JI_API JiGeometry ji_geometry_ellipse(JiRect bounds);

/** Create a line geometry. */
JI_API JiGeometry ji_geometry_line(JiPoint start, JiPoint end);

/* =========================================================================
 * JiDrawingContext — abstract drawing interface
 * ========================================================================= */

/** Forward declaration (full definition below). */
typedef struct JiDrawingContext JiDrawingContext;

/** Push a clipping rectangle. */
typedef void (*JiDrawPushClipFunc)(JiDrawingContext* ctx, JiRect clip);

/** Pop the last clipping rectangle. */
typedef void (*JiDrawPopClipFunc)(JiDrawingContext* ctx);

/** Push a transform. */
typedef void (*JiDrawPushTransformFunc)(JiDrawingContext* ctx, JiMatrix transform);

/** Pop the last transform. */
typedef void (*JiDrawPopTransformFunc)(JiDrawingContext* ctx);

/** Draw a filled geometry. */
typedef void (*JiDrawFillGeometryFunc)(JiDrawingContext* ctx,
                                        const JiGeometry* geom,
                                        const JiBrush* brush);

/** Draw a stroked geometry. */
typedef void (*JiDrawStrokeGeometryFunc)(JiDrawingContext* ctx,
                                          const JiGeometry* geom,
                                          const JiPen* pen);

/** Draw a line. */
typedef void (*JiDrawLineFunc)(JiDrawingContext* ctx,
                                JiPoint start, JiPoint end,
                                const JiBrush* brush, double thickness);

/** Draw a filled rectangle. */
typedef void (*JiDrawFillRectFunc)(JiDrawingContext* ctx,
                                    JiRect rect, const JiBrush* brush);

/** Draw a stroked rectangle. */
typedef void (*JiDrawStrokeRectFunc)(JiDrawingContext* ctx,
                                      JiRect rect, const JiPen* pen);

/** Draw text at a position. */
typedef void (*JiDrawTextFunc)(JiDrawingContext* ctx,
                                 JiPoint origin, const char* text,
                                 const JiBrush* brush, const char* font_family,
                                 double font_size);

/** Clear the surface with a color. */
typedef void (*JiDrawClearFunc)(JiDrawingContext* ctx, uint32_t argb);

/** Resize the drawing surface. */
typedef void (*JiDrawResizeFunc)(JiDrawingContext* ctx, int width, int height);

/** Present/flush the drawing surface. */
typedef void (*JiDrawPresentFunc)(JiDrawingContext* ctx);

struct JiDrawingContext {
    /* Function pointers — backend fills these in */
    JiDrawPushClipFunc       push_clip;
    JiDrawPopClipFunc        pop_clip;
    JiDrawPushTransformFunc  push_transform;
    JiDrawPopTransformFunc   pop_transform;
    JiDrawFillGeometryFunc   fill_geometry;
    JiDrawStrokeGeometryFunc stroke_geometry;
    JiDrawLineFunc           draw_line;
    JiDrawFillRectFunc       fill_rect;
    JiDrawStrokeRectFunc     stroke_rect;
    JiDrawTextFunc           draw_text;
    JiDrawClearFunc          clear;
    JiDrawResizeFunc         resize;
    JiDrawPresentFunc        present;

    /* Backend-specific data */
    void* backend_data;

    /* Current surface size */
    int surface_width;
    int surface_height;
};

/** Initialize a drawing context with all function pointers set to NULL. */
JI_API void ji_drawing_context_init(JiDrawingContext* ctx);

/** Check if a drawing context has all required function pointers set. */
JI_API bool ji_drawing_context_is_valid(const JiDrawingContext* ctx);

/* ---- Convenience drawing functions ---- */

/** Push clip rect. No-op if ctx or push_clip is NULL. */
JI_API void ji_draw_push_clip(JiDrawingContext* ctx, JiRect clip);

/** Pop clip rect. No-op if ctx or pop_clip is NULL. */
JI_API void ji_draw_pop_clip(JiDrawingContext* ctx);

/** Push transform. No-op if ctx or push_transform is NULL. */
JI_API void ji_draw_push_transform(JiDrawingContext* ctx, JiMatrix transform);

/** Pop transform. No-op if ctx or pop_transform is NULL. */
JI_API void ji_draw_pop_transform(JiDrawingContext* ctx);

/** Fill a geometry. No-op if ctx or fill_geometry is NULL. */
JI_API void ji_draw_fill_geometry(JiDrawingContext* ctx,
                                   const JiGeometry* geom,
                                   const JiBrush* brush);

/** Stroke a geometry. No-op if ctx or stroke_geometry is NULL. */
JI_API void ji_draw_stroke_geometry(JiDrawingContext* ctx,
                                      const JiGeometry* geom,
                                      const JiPen* pen);

/** Draw a line. No-op if ctx or draw_line is NULL. */
JI_API void ji_draw_line(JiDrawingContext* ctx,
                           JiPoint start, JiPoint end,
                           const JiBrush* brush, double thickness);

/** Fill a rectangle. No-op if ctx or fill_rect is NULL. */
JI_API void ji_draw_fill_rect(JiDrawingContext* ctx,
                                JiRect rect, const JiBrush* brush);

/** Stroke a rectangle. No-op if ctx or stroke_rect is NULL. */
JI_API void ji_draw_stroke_rect(JiDrawingContext* ctx,
                                  JiRect rect, const JiPen* pen);

/** Draw text. No-op if ctx or draw_text is NULL. */
JI_API void ji_draw_text(JiDrawingContext* ctx,
                           JiPoint origin, const char* text,
                           const JiBrush* brush,
                           const char* font_family, double font_size);

/** Clear the surface. No-op if ctx or clear is NULL. */
JI_API void ji_draw_clear(JiDrawingContext* ctx, uint32_t argb);

/** Resize the surface. No-op if ctx or resize is NULL. */
JI_API void ji_draw_resize(JiDrawingContext* ctx, int width, int height);

/** Present/flush. No-op if ctx or present is NULL. */
JI_API void ji_draw_present(JiDrawingContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_DRAWING_H */
