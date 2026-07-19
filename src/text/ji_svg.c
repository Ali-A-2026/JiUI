/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =========================================================================
 * SVG Path
 * ========================================================================= */

struct JiSvgPath {
    JiSvgPathCmd* cmds;
    uint32_t      count;
    uint32_t      capacity;
};

JiSvgPath* ji_svg_path_create(void) {
    JiSvgPath* path = calloc(1, sizeof(JiSvgPath));
    if (!path) return NULL;
    path->capacity = 16;
    path->cmds = calloc(path->capacity, sizeof(JiSvgPathCmd));
    return path;
}

void ji_svg_path_destroy(JiSvgPath* path) {
    if (!path) return;
    free(path->cmds);
    free(path);
}

JiSvgPath* ji_svg_path_clone(const JiSvgPath* path) {
    if (!path) return NULL;
    JiSvgPath* clone = ji_svg_path_create();
    if (!clone) return NULL;
    clone->count = path->count;
    clone->capacity = path->capacity;
    clone->cmds = realloc(clone->cmds, clone->capacity * sizeof(JiSvgPathCmd));
    memcpy(clone->cmds, path->cmds, path->count * sizeof(JiSvgPathCmd));
    return clone;
}

static void ji_svg_path_ensure_capacity(JiSvgPath* path) {
    if (path->count >= path->capacity) {
        path->capacity *= 2;
        path->cmds = realloc(path->cmds, path->capacity * sizeof(JiSvgPathCmd));
    }
}

void ji_svg_path_move_to(JiSvgPath* path, float x, float y, bool relative) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_MOVE_TO;
    cmd->relative = relative;
    cmd->x = x; cmd->y = y;
}

void ji_svg_path_line_to(JiSvgPath* path, float x, float y, bool relative) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_LINE_TO;
    cmd->relative = relative;
    cmd->x = x; cmd->y = y;
}

void ji_svg_path_curve_to(JiSvgPath* path, float x1, float y1, float x2, float y2, float x, float y, bool relative) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_CURVE_TO;
    cmd->relative = relative;
    cmd->x1 = x1; cmd->y1 = y1;
    cmd->x2 = x2; cmd->y2 = y2;
    cmd->x = x; cmd->y = y;
}

void ji_svg_path_quad_to(JiSvgPath* path, float x1, float y1, float x, float y, bool relative) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_QUAD_TO;
    cmd->relative = relative;
    cmd->x1 = x1; cmd->y1 = y1;
    cmd->x = x; cmd->y = y;
}

void ji_svg_path_arc_to(JiSvgPath* path, float rx, float ry, float rotation, bool large_arc, bool sweep, float x, float y, bool relative) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_ARC_TO;
    cmd->relative = relative;
    cmd->rx = rx; cmd->ry = ry;
    cmd->rotation = rotation;
    cmd->large_arc = large_arc;
    cmd->sweep = sweep;
    cmd->x = x; cmd->y = y;
}

void ji_svg_path_close(JiSvgPath* path) {
    if (!path) return;
    ji_svg_path_ensure_capacity(path);
    JiSvgPathCmd* cmd = &path->cmds[path->count++];
    memset(cmd, 0, sizeof(JiSvgPathCmd));
    cmd->cmd = JI_SVG_CLOSE_PATH;
}

uint32_t ji_svg_path_get_cmd_count(const JiSvgPath* path) { return path ? path->count : 0; }
const JiSvgPathCmd* ji_svg_path_get_cmds(const JiSvgPath* path) { return path ? path->cmds : NULL; }

JiSvgPoint* ji_svg_path_flatten(const JiSvgPath* path, float tolerance, uint32_t* point_count) {
    if (!path || !point_count) return NULL;
    (void)tolerance;
    /* Simple: just convert move_to/line_to to points */
    uint32_t max_pts = path->count * 2;
    JiSvgPoint* pts = malloc(max_pts * sizeof(JiSvgPoint));
    if (!pts) return NULL;
    uint32_t count = 0;
    float cx = 0, cy = 0;
    for (uint32_t i = 0; i < path->count; i++) {
        const JiSvgPathCmd* cmd = &path->cmds[i];
        float x = cmd->relative ? cx + cmd->x : cmd->x;
        float y = cmd->relative ? cy + cmd->y : cmd->y;
        if (cmd->cmd == JI_SVG_MOVE_TO || cmd->cmd == JI_SVG_LINE_TO) {
            if (count < max_pts) { pts[count].x = x; pts[count].y = y; count++; }
        }
        cx = x; cy = y;
    }
    *point_count = count;
    return pts;
}

JiSvgPath* ji_svg_path_boolean(const JiSvgPath* a, const JiSvgPath* b, JiVectorOp op) {
    (void)a; (void)b; (void)op;
    /* Placeholder: return empty path */
    return ji_svg_path_create();
}

/* =========================================================================
 * SVG Document
 * ========================================================================= */

struct JiSvgDocument {
    JiSvgNode* root;
    float      width;
    float      height;
};

JiSvgDocument* ji_svg_document_create(void) {
    JiSvgDocument* doc = calloc(1, sizeof(JiSvgDocument));
    if (!doc) return NULL;
    doc->width = 100;
    doc->height = 100;
    return doc;
}

void ji_svg_document_destroy(JiSvgDocument* doc) {
    if (!doc) return;
    free(doc);
}

JiSvgDocument* ji_svg_parse(const char* svg_data, uint32_t length) {
    if (!svg_data || length == 0) return NULL;
    JiSvgDocument* doc = ji_svg_document_create();
    if (!doc) return NULL;
    /* Simple parser: extract width/height from <svg> tag */
    const char* p = strstr(svg_data, "width=\"");
    if (p) doc->width = (float)atof(p + 7);
    p = strstr(svg_data, "height=\"");
    if (p) doc->height = (float)atof(p + 8);
    return doc;
}

JiSvgDocument* ji_svg_parse_file(const char* path) {
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(size + 1);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, size, f);
    data[size] = 0;
    fclose(f);
    JiSvgDocument* doc = ji_svg_parse(data, (uint32_t)size);
    free(data);
    return doc;
}

JiSvgNode* ji_svg_document_get_root(const JiSvgDocument* doc) {
    return doc ? doc->root : NULL;
}

JiSvgNode* ji_svg_document_find_by_id(const JiSvgDocument* doc, const char* id) {
    (void)doc; (void)id;
    return NULL;
}

float ji_svg_document_get_width(const JiSvgDocument* doc) { return doc ? doc->width : 0; }
float ji_svg_document_get_height(const JiSvgDocument* doc) { return doc ? doc->height : 0; }

bool ji_svg_render_to_texture(const JiSvgDocument* doc, JiGpuDevice* device, JiGpuTexture* texture, uint32_t width, uint32_t height) {
    (void)doc; (void)device; (void)texture; (void)width; (void)height;
    return true;
}
