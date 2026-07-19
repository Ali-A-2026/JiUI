/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_types.c
 * @brief Implementation of core geometric and primitive types.
 */

#include "jiui/ji_types.h"
#include <math.h>
#include <float.h>
#include <string.h>

/* =========================================================================
 * JiPoint
 * ========================================================================= */

JiPoint ji_point(double x, double y) {
    JiPoint p = { x, y };
    return p;
}

JiPoint ji_point_zero(void) {
    return ji_point(0.0, 0.0);
}

bool ji_point_equals(JiPoint a, JiPoint b) {
    return a.x == b.x && a.y == b.y;
}

JiPoint ji_point_add(JiPoint a, JiPoint b) {
    return ji_point(a.x + b.x, a.y + b.y);
}

JiPoint ji_point_add_vector(JiPoint a, JiVector v) {
    return ji_point(a.x + v.x, a.y + v.y);
}

JiVector ji_point_subtract(JiPoint a, JiPoint b) {
    return ji_vector(a.x - b.x, a.y - b.y);
}

JiVector ji_point_to_vector(JiPoint p) {
    return ji_vector(p.x, p.y);
}

/* =========================================================================
 * JiSize
 * ========================================================================= */

JiSize ji_size(double w, double h) {
    JiSize s = { w < 0 ? 0 : w, h < 0 ? 0 : h };
    return s;
}

JiSize ji_size_zero(void) {
    return ji_size(0.0, 0.0);
}

JiSize ji_size_infinite(void) {
    JiSize s = { DBL_MAX, DBL_MAX };
    return s;
}

bool ji_size_is_empty(JiSize s) {
    return s.width <= 0.0 || s.height <= 0.0;
}

bool ji_size_is_infinite(JiSize s) {
    return s.width >= DBL_MAX || s.height >= DBL_MAX;
}

bool ji_size_equals(JiSize a, JiSize b) {
    return a.width == b.width && a.height == b.height;
}

JiSize ji_size_add(JiSize a, JiSize b) {
    return ji_size(a.width + b.width, a.height + b.height);
}

JiSize ji_size_subtract(JiSize a, JiSize b) {
    return ji_size(a.width - b.width, a.height - b.height);
}

JiSize ji_size_max(JiSize a, JiSize b) {
    return ji_size(fmax(a.width, b.width), fmax(a.height, b.height));
}

JiSize ji_size_min(JiSize a, JiSize b) {
    return ji_size(fmin(a.width, b.width), fmin(a.height, b.height));
}

/* =========================================================================
 * JiRect
 * ========================================================================= */

JiRect ji_rect(double x, double y, double w, double h) {
    JiRect r = { x, y, w < 0 ? 0 : w, h < 0 ? 0 : h };
    return r;
}

JiRect ji_rect_zero(void) {
    return ji_rect(0.0, 0.0, 0.0, 0.0);
}

JiRect ji_rect_from_size(JiSize s) {
    return ji_rect(0.0, 0.0, s.width, s.height);
}

JiRect ji_rect_from_points(JiPoint tl, JiPoint br) {
    double x = fmin(tl.x, br.x);
    double y = fmin(tl.y, br.y);
    double w = fabs(br.x - tl.x);
    double h = fabs(br.y - tl.y);
    return ji_rect(x, y, w, h);
}

bool ji_rect_equals(JiRect a, JiRect b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

bool ji_rect_is_empty(JiRect r) {
    return r.width <= 0.0 || r.height <= 0.0;
}

bool ji_rect_contains_point(JiRect r, JiPoint p) {
    return p.x >= r.x && p.x <= r.x + r.width &&
           p.y >= r.y && p.y <= r.y + r.height;
}

bool ji_rect_contains_rect(JiRect outer, JiRect inner) {
    return inner.x >= outer.x &&
           inner.y >= outer.y &&
           inner.x + inner.width  <= outer.x + outer.width &&
           inner.y + inner.height <= outer.y + outer.height;
}

bool ji_rect_intersects(JiRect a, JiRect b) {
    if (a.x < b.x + b.width && a.x + a.width > b.x &&
        a.y < b.y + b.height && a.y + a.height > b.y) {
        return true;
    }
    return false;
}

JiRect ji_rect_intersect(JiRect a, JiRect b) {
    double x1 = fmax(a.x, b.x);
    double y1 = fmax(a.y, b.y);
    double x2 = fmin(a.x + a.width, b.x + b.width);
    double y2 = fmin(a.y + a.height, b.y + b.height);
    if (x2 <= x1 || y2 <= y1) return ji_rect_zero();
    return ji_rect(x1, y1, x2 - x1, y2 - y1);
}

JiRect ji_rect_union(JiRect a, JiRect b) {
    if (ji_rect_is_empty(a)) return b;
    if (ji_rect_is_empty(b)) return a;
    double x1 = fmin(a.x, b.x);
    double y1 = fmin(a.y, b.y);
    double x2 = fmax(a.x + a.width, b.x + b.width);
    double y2 = fmax(a.y + a.height, b.y + b.height);
    return ji_rect(x1, y1, x2 - x1, y2 - y1);
}

JiRect ji_rect_inflate(JiRect r, JiThickness t) {
    return ji_rect(r.x - t.left, r.y - t.top,
                   r.width + t.left + t.right,
                   r.height + t.top + t.bottom);
}

JiRect ji_rect_deflate(JiRect r, JiThickness t) {
    return ji_rect(r.x + t.left, r.y + t.top,
                   r.width - t.left - t.right,
                   r.height - t.top - t.bottom);
}

JiRect ji_rect_offset(JiRect r, double dx, double dy) {
    return ji_rect(r.x + dx, r.y + dy, r.width, r.height);
}

JiPoint ji_rect_center(JiRect r) {
    return ji_point(r.x + r.width * 0.5, r.y + r.height * 0.5);
}

JiSize ji_rect_get_size(JiRect r) {
    return ji_size(r.width, r.height);
}

JiPoint ji_rect_get_position(JiRect r) {
    return ji_point(r.x, r.y);
}

/* =========================================================================
 * JiVector
 * ========================================================================= */

JiVector ji_vector(double x, double y) {
    JiVector v = { x, y };
    return v;
}

JiVector ji_vector_zero(void) {
    return ji_vector(0.0, 0.0);
}

bool ji_vector_equals(JiVector a, JiVector b) {
    return a.x == b.x && a.y == b.y;
}

double ji_vector_length(JiVector v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

double ji_vector_length_squared(JiVector v) {
    return v.x * v.x + v.y * v.y;
}

JiVector ji_vector_normalize(JiVector v) {
    double len = ji_vector_length(v);
    if (len < DBL_EPSILON) return ji_vector_zero();
    return ji_vector(v.x / len, v.y / len);
}

JiVector ji_vector_add(JiVector a, JiVector b) {
    return ji_vector(a.x + b.x, a.y + b.y);
}

JiVector ji_vector_subtract(JiVector a, JiVector b) {
    return ji_vector(a.x - b.x, a.y - b.y);
}

JiVector ji_vector_negate(JiVector v) {
    return ji_vector(-v.x, -v.y);
}

JiVector ji_vector_scale(JiVector v, double factor) {
    return ji_vector(v.x * factor, v.y * factor);
}

double ji_vector_dot(JiVector a, JiVector b) {
    return a.x * b.x + a.y * b.y;
}

double ji_vector_cross(JiVector a, JiVector b) {
    return a.x * b.y - a.y * b.x;
}

/* =========================================================================
 * JiVector3D
 * ========================================================================= */

JiVector3D ji_vector3d(double x, double y, double z) {
    JiVector3D v = { x, y, z };
    return v;
}

JiVector3D ji_vector3d_zero(void) {
    return ji_vector3d(0.0, 0.0, 0.0);
}

bool ji_vector3d_equals(JiVector3D a, JiVector3D b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

/* =========================================================================
 * JiThickness
 * ========================================================================= */

JiThickness ji_thickness(double uniform) {
    JiThickness t = { uniform, uniform, uniform, uniform };
    return t;
}

JiThickness ji_thickness_lr(double left_right, double top_bottom) {
    JiThickness t = { left_right, top_bottom, left_right, top_bottom };
    return t;
}

JiThickness ji_thickness_all(double left, double top, double right, double bottom) {
    JiThickness t = { left, top, right, bottom };
    return t;
}

JiThickness ji_thickness_zero(void) {
    return ji_thickness(0.0);
}

bool ji_thickness_equals(JiThickness a, JiThickness b) {
    return a.left == b.left && a.top == b.top &&
           a.right == b.right && a.bottom == b.bottom;
}

bool ji_thickness_is_uniform(JiThickness t) {
    return t.left == t.top && t.top == t.right && t.right == t.bottom;
}

double ji_thickness_horizontal(JiThickness t) {
    return t.left + t.right;
}

double ji_thickness_vertical(JiThickness t) {
    return t.top + t.bottom;
}

/* =========================================================================
 * JiCornerRadius
 * ========================================================================= */

JiCornerRadius ji_corner_radius(double uniform) {
    JiCornerRadius c = { uniform, uniform, uniform, uniform };
    return c;
}

JiCornerRadius ji_corner_radius_all(double tl, double tr, double br, double bl) {
    JiCornerRadius c = { tl, tr, br, bl };
    return c;
}

JiCornerRadius ji_corner_radius_zero(void) {
    return ji_corner_radius(0.0);
}

bool ji_corner_radius_equals(JiCornerRadius a, JiCornerRadius b) {
    return a.top_left == b.top_left && a.top_right == b.top_right &&
           a.bottom_right == b.bottom_right && a.bottom_left == b.bottom_left;
}

/* =========================================================================
 * JiPixelPoint
 * ========================================================================= */

JiPixelPoint ji_pixel_point(int32_t x, int32_t y) {
    JiPixelPoint p = { x, y };
    return p;
}

bool ji_pixel_point_equals(JiPixelPoint a, JiPixelPoint b) {
    return a.x == b.x && a.y == b.y;
}

/* =========================================================================
 * JiPixelSize
 * ========================================================================= */

JiPixelSize ji_pixel_size(int32_t w, int32_t h) {
    JiPixelSize s = { w < 0 ? 0 : w, h < 0 ? 0 : h };
    return s;
}

bool ji_pixel_size_equals(JiPixelSize a, JiPixelSize b) {
    return a.width == b.width && a.height == b.height;
}

/* =========================================================================
 * JiPixelRect
 * ========================================================================= */

JiPixelRect ji_pixel_rect(int32_t x, int32_t y, int32_t w, int32_t h) {
    JiPixelRect r = { x, y, w < 0 ? 0 : w, h < 0 ? 0 : h };
    return r;
}

bool ji_pixel_rect_equals(JiPixelRect a, JiPixelRect b) {
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

bool ji_pixel_rect_contains(JiPixelRect r, JiPixelPoint p) {
    return p.x >= r.x && p.x <= r.x + r.width &&
           p.y >= r.y && p.y <= r.y + r.height;
}

/* =========================================================================
 * JiPixelVector
 * ========================================================================= */

JiPixelVector ji_pixel_vector(int32_t x, int32_t y) {
    JiPixelVector v = { x, y };
    return v;
}

/* =========================================================================
 * JiMatrix
 * ========================================================================= */

JiMatrix ji_matrix_identity(void) {
    JiMatrix m = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    return m;
}

JiMatrix ji_matrix_translation(double dx, double dy) {
    JiMatrix m = { 1.0, 0.0, 0.0, 1.0, dx, dy };
    return m;
}

JiMatrix ji_matrix_scale(double sx, double sy) {
    JiMatrix m = { sx, 0.0, 0.0, sy, 0.0, 0.0 };
    return m;
}

JiMatrix ji_matrix_rotation(double angle_radians) {
    double c = cos(angle_radians);
    double s = sin(angle_radians);
    JiMatrix m = { c, s, -s, c, 0.0, 0.0 };
    return m;
}

JiMatrix ji_matrix_skew(double angle_x_radians, double angle_y_radians) {
    JiMatrix m = { 1.0, tan(angle_y_radians), tan(angle_x_radians), 1.0, 0.0, 0.0 };
    return m;
}

bool ji_matrix_is_identity(JiMatrix m) {
    return m.m11 == 1.0 && m.m12 == 0.0 &&
           m.m21 == 0.0 && m.m22 == 1.0 &&
           m.m31 == 0.0 && m.m32 == 0.0;
}

JiMatrix ji_matrix_multiply(JiMatrix a, JiMatrix b) {
    JiMatrix r;
    r.m11 = a.m11 * b.m11 + a.m12 * b.m21;
    r.m12 = a.m11 * b.m12 + a.m12 * b.m22;
    r.m21 = a.m21 * b.m11 + a.m22 * b.m21;
    r.m22 = a.m21 * b.m12 + a.m22 * b.m22;
    r.m31 = a.m31 * b.m11 + a.m32 * b.m21 + b.m31;
    r.m32 = a.m31 * b.m12 + a.m32 * b.m22 + b.m32;
    return r;
}

JiPoint ji_matrix_transform_point(JiMatrix m, JiPoint p) {
    return ji_point(
        p.x * m.m11 + p.y * m.m21 + m.m31,
        p.x * m.m12 + p.y * m.m22 + m.m32
    );
}

JiMatrix ji_matrix_invert(JiMatrix m) {
    JiMatrix inv;
    double det = m.m11 * m.m22 - m.m12 * m.m21;
    double inv_det = 1.0 / det;
    inv.m11 =  m.m22 * inv_det;
    inv.m12 = -m.m12 * inv_det;
    inv.m21 = -m.m21 * inv_det;
    inv.m22 =  m.m11 * inv_det;
    inv.m31 = (m.m21 * m.m32 - m.m22 * m.m31) * inv_det;
    inv.m32 = (m.m12 * m.m31 - m.m11 * m.m32) * inv_det;
    return inv;
}

bool ji_matrix_try_invert(JiMatrix m, JiMatrix* out) {
    double det = m.m11 * m.m22 - m.m12 * m.m21;
    if (fabs(det) < DBL_EPSILON) return false;
    *out = ji_matrix_invert(m);
    return true;
}

/* =========================================================================
 * JiRelativePoint
 * ========================================================================= */

JiRelativePoint ji_relative_point(double x, double y, JiRelativeUnit unit) {
    JiRelativePoint rp = { x, y, unit };
    return rp;
}

JiPoint ji_relative_point_resolve(JiRelativePoint rp, JiSize available) {
    if (rp.unit == JI_RELATIVE_UNIT_RELATIVE) {
        return ji_point(rp.x * available.width, rp.y * available.height);
    }
    return ji_point(rp.x, rp.y);
}

/* =========================================================================
 * JiRelativeRect
 * ========================================================================= */

JiRelativeRect ji_relative_rect(JiRelativePoint origin, JiSize size, JiRelativeUnit unit) {
    JiRelativeRect rr = { origin, size, unit };
    return rr;
}

JiRect ji_relative_rect_resolve(JiRelativeRect rr, JiSize available) {
    JiPoint pos = ji_relative_point_resolve(rr.origin, available);
    JiSize sz = rr.size;
    if (rr.size_unit == JI_RELATIVE_UNIT_RELATIVE) {
        sz = ji_size(rr.size.width * available.width,
                     rr.size.height * available.height);
    }
    return ji_rect(pos.x, pos.y, sz.width, sz.height);
}

/* =========================================================================
 * JiRelativeScalar
 * ========================================================================= */

JiRelativeScalar ji_relative_scalar(double value, JiRelativeUnit unit) {
    JiRelativeScalar rs = { value, unit };
    return rs;
}

double ji_relative_scalar_resolve(JiRelativeScalar rs, double available) {
    if (rs.unit == JI_RELATIVE_UNIT_RELATIVE) {
        return rs.value * available;
    }
    return rs.value;
}

/* =========================================================================
 * JiRoundedRect
 * ========================================================================= */

JiRoundedRect ji_rounded_rect(JiRect rect, JiCornerRadius radius) {
    JiRoundedRect rr = { rect, radius };
    return rr;
}

/* =========================================================================
 * JiGridLength
 * ========================================================================= */

JiGridLength ji_grid_length_auto(void) {
    JiGridLength gl = { 0.0, JI_GRID_UNIT_AUTO };
    return gl;
}

JiGridLength ji_grid_length_pixel(double px) {
    JiGridLength gl = { px < 0 ? 0 : px, JI_GRID_UNIT_PIXEL };
    return gl;
}

JiGridLength ji_grid_length_star(double star) {
    JiGridLength gl = { star, JI_GRID_UNIT_STAR };
    return gl;
}

bool ji_grid_length_equals(JiGridLength a, JiGridLength b) {
    return a.unit_type == b.unit_type && a.value == b.value;
}
