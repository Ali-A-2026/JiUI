/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_svg.h
 * @brief SVG engine — SVG 1.1 subset parsing, vector path operations,
 *        GPU path rendering, gradient meshes, SVG filters.
 */

#ifndef JIUI_SVG_H
#define JIUI_SVG_H

#include "ji_types.h"
#include "ji_gpu.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * SVG Path Commands
 * ========================================================================= */

typedef enum JiSvgCmd {
    JI_SVG_MOVE_TO,
    JI_SVG_LINE_TO,
    JI_SVG_HLINE_TO,
    JI_SVG_VLINE_TO,
    JI_SVG_CURVE_TO,
    JI_SVG_SMOOTH_CURVE_TO,
    JI_SVG_QUAD_TO,
    JI_SVG_SMOOTH_QUAD_TO,
    JI_SVG_ARC_TO,
    JI_SVG_CLOSE_PATH
} JiSvgCmd;

typedef struct JiSvgPathCmd {
    JiSvgCmd  cmd;
    bool      relative;
    float     x, y;
    float     x1, y1;     /* Control point 1 */
    float     x2, y2;     /* Control point 2 */
    float     rx, ry;     /* Arc radii */
    float     rotation;   /* Arc rotation */
    bool      large_arc;   /* Arc flag */
    bool      sweep;       /* Arc flag */
} JiSvgPathCmd;

/* =========================================================================
 * SVG Path
 * ========================================================================= */

typedef struct JiSvgPath JiSvgPath;

JiSvgPath*    ji_svg_path_create(void);
void          ji_svg_path_destroy(JiSvgPath* path);
JiSvgPath*    ji_svg_path_clone(const JiSvgPath* path);

void          ji_svg_path_move_to(JiSvgPath* path, float x, float y, bool relative);
void          ji_svg_path_line_to(JiSvgPath* path, float x, float y, bool relative);
void          ji_svg_path_curve_to(JiSvgPath* path, float x1, float y1, float x2, float y2, float x, float y, bool relative);
void          ji_svg_path_quad_to(JiSvgPath* path, float x1, float y1, float x, float y, bool relative);
void          ji_svg_path_arc_to(JiSvgPath* path, float rx, float ry, float rotation, bool large_arc, bool sweep, float x, float y, bool relative);
void          ji_svg_path_close(JiSvgPath* path);

uint32_t      ji_svg_path_get_cmd_count(const JiSvgPath* path);
const JiSvgPathCmd* ji_svg_path_get_cmds(const JiSvgPath* path);

/* Flatten to line segments */
typedef struct JiSvgPoint { float x, y; } JiSvgPoint;

JiSvgPoint*   ji_svg_path_flatten(const JiSvgPath* path, float tolerance, uint32_t* point_count);

/* =========================================================================
 * Vector Boolean Operations
 * ========================================================================= */

typedef enum JiVectorOp {
    JI_VECTOR_OP_UNION,
    JI_VECTOR_OP_INTERSECTION,
    JI_VECTOR_OP_DIFFERENCE,
    JI_VECTOR_OP_XOR
} JiVectorOp;

JiSvgPath*    ji_svg_path_boolean(const JiSvgPath* a, const JiSvgPath* b, JiVectorOp op);

/* =========================================================================
 * SVG Document
 * ========================================================================= */

typedef struct JiSvgNode JiSvgNode;

typedef enum JiSvgNodeType {
    JI_SVG_NODE_GROUP,
    JI_SVG_NODE_PATH,
    JI_SVG_NODE_RECT,
    JI_SVG_NODE_CIRCLE,
    JI_SVG_NODE_ELLIPSE,
    JI_SVG_NODE_LINE,
    JI_SVG_NODE_POLYGON,
    JI_SVG_NODE_POLYLINE,
    JI_SVG_NODE_TEXT,
    JI_SVG_NODE_IMAGE
} JiSvgNodeType;

typedef struct JiSvgPaint {
    float   r, g, b, a;
    bool    is_none;
} JiSvgPaint;

typedef struct JiSvgFill {
    JiSvgPaint  paint;
    float       opacity;
    JiSvgPath*  clip_path;
} JiSvgFill;

typedef struct JiSvgStroke {
    JiSvgPaint  paint;
    float       width;
    float       opacity;
    float       dash_offset;
    float*      dash_array;
    uint32_t    dash_count;
    char        line_cap;    /* 'b'utt, 'r'ound, 's'quare */
    char        line_join;   /* 'm'iter, 'r'ound, 'b'evel */
} JiSvgStroke;

struct JiSvgNode {
    JiSvgNodeType  type;
    char*          id;
    JiSvgFill      fill;
    JiSvgStroke    stroke;
    JiSvgPath*     path;
    JiSvgNode**    children;
    uint32_t       child_count;
    float          transform[6]; /* 2D affine: a,b,c,d,e,f */
};

typedef struct JiSvgDocument JiSvgDocument;

JiSvgDocument* ji_svg_document_create(void);
void           ji_svg_document_destroy(JiSvgDocument* doc);

/* Parse SVG from string */
JiSvgDocument* ji_svg_parse(const char* svg_data, uint32_t length);

/* Parse SVG from file */
JiSvgDocument* ji_svg_parse_file(const char* path);

/* Get root node */
JiSvgNode*     ji_svg_document_get_root(const JiSvgDocument* doc);

/* Find node by ID */
JiSvgNode*     ji_svg_document_find_by_id(const JiSvgDocument* doc, const char* id);

/* Get document dimensions */
float          ji_svg_document_get_width(const JiSvgDocument* doc);
float          ji_svg_document_get_height(const JiSvgDocument* doc);

/* Render SVG to GPU texture */
bool           ji_svg_render_to_texture(const JiSvgDocument* doc,
                                          JiGpuDevice* device,
                                          JiGpuTexture* texture,
                                          uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SVG_H */
