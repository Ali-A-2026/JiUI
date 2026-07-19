/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_types.h
 * @brief Core geometric and primitive types for JiUI.
 *
 * Core geometric types: Point, Size, Rect, Vector, Thickness,
 * CornerRadius, PixelPoint, PixelRect, PixelSize, Matrix, etc.
 */

#ifndef JIUI_TYPES_H
#define JIUI_TYPES_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
typedef struct JiPoint      JiPoint;
typedef struct JiSize        JiSize;
typedef struct JiRect        JiRect;
typedef struct JiVector      JiVector;
typedef struct JiVector3D    JiVector3D;
typedef struct JiThickness   JiThickness;
typedef struct JiCornerRadius JiCornerRadius;
typedef struct JiPixelPoint  JiPixelPoint;
typedef struct JiPixelSize   JiPixelSize;
typedef struct JiPixelRect   JiPixelRect;
typedef struct JiPixelVector JiPixelVector;
typedef struct JiMatrix      JiMatrix;
typedef struct JiRelativePoint JiRelativePoint;
typedef struct JiRelativeRect  JiRelativeRect;
typedef struct JiRelativeScalar JiRelativeScalar;
typedef struct JiRoundedRect JiRoundedRect;
typedef struct JiGridLength  JiGridLength;

/* =========================================================================
 * JiPoint — 2D floating-point position
 * ========================================================================= */
struct JiPoint {
    double x;
    double y;
};

JI_API JiPoint ji_point(double x, double y);
JI_API JiPoint ji_point_zero(void);
JI_API bool    ji_point_equals(JiPoint a, JiPoint b);
JI_API JiPoint  ji_point_add(JiPoint a, JiPoint b);
JI_API JiPoint  ji_point_add_vector(JiPoint a, JiVector v); /* point + vector */
JI_API JiVector ji_point_subtract(JiPoint a, JiPoint b);    /* point - point => vector */
JI_API JiVector ji_point_to_vector(JiPoint p);

/* =========================================================================
 * JiSize — 2D floating-point dimensions (non-negative)
 * ========================================================================= */
struct JiSize {
    double width;
    double height;
};

JI_API JiSize  ji_size(double w, double h);
JI_API JiSize  ji_size_zero(void);
JI_API JiSize  ji_size_infinite(void);
JI_API bool    ji_size_is_empty(JiSize s);
JI_API bool    ji_size_is_infinite(JiSize s);
JI_API bool    ji_size_equals(JiSize a, JiSize b);
JI_API JiSize  ji_size_add(JiSize a, JiSize b);
JI_API JiSize  ji_size_subtract(JiSize a, JiSize b);
JI_API JiSize  ji_size_max(JiSize a, JiSize b);
JI_API JiSize  ji_size_min(JiSize a, JiSize b);

/* =========================================================================
 * JiRect — 2D floating-point rectangle
 * ========================================================================= */
struct JiRect {
    double x;
    double y;
    double width;
    double height;
};

JI_API JiRect  ji_rect(double x, double y, double w, double h);
JI_API JiRect  ji_rect_zero(void);
JI_API JiRect  ji_rect_from_size(JiSize s);
JI_API JiRect  ji_rect_from_points(JiPoint tl, JiPoint br);
JI_API bool    ji_rect_equals(JiRect a, JiRect b);
JI_API bool    ji_rect_is_empty(JiRect r);
JI_API bool    ji_rect_contains_point(JiRect r, JiPoint p);
JI_API bool    ji_rect_contains_rect(JiRect outer, JiRect inner);
JI_API bool    ji_rect_intersects(JiRect a, JiRect b);
JI_API JiRect  ji_rect_intersect(JiRect a, JiRect b);
JI_API JiRect  ji_rect_union(JiRect a, JiRect b);
JI_API JiRect  ji_rect_inflate(JiRect r, JiThickness t);
JI_API JiRect  ji_rect_deflate(JiRect r, JiThickness t);
JI_API JiRect  ji_rect_offset(JiRect r, double dx, double dy);
JI_API JiPoint ji_rect_center(JiRect r);
JI_API JiSize  ji_rect_get_size(JiRect r);
JI_API JiPoint ji_rect_get_position(JiRect r);

/* =========================================================================
 * JiVector — 2D floating-point direction/magnitude
 * ========================================================================= */
struct JiVector {
    double x;
    double y;
};

JI_API JiVector ji_vector(double x, double y);
JI_API JiVector ji_vector_zero(void);
JI_API bool     ji_vector_equals(JiVector a, JiVector b);
JI_API double   ji_vector_length(JiVector v);
JI_API double   ji_vector_length_squared(JiVector v);
JI_API JiVector ji_vector_normalize(JiVector v);
JI_API JiVector ji_vector_add(JiVector a, JiVector b);
JI_API JiVector ji_vector_subtract(JiVector a, JiVector b);
JI_API JiVector ji_vector_negate(JiVector v);
JI_API JiVector ji_vector_scale(JiVector v, double factor);
JI_API double   ji_vector_dot(JiVector a, JiVector b);
JI_API double   ji_vector_cross(JiVector a, JiVector b);

/* =========================================================================
 * JiVector3D — 3D floating-point vector
 * ========================================================================= */
struct JiVector3D {
    double x;
    double y;
    double z;
};

JI_API JiVector3D ji_vector3d(double x, double y, double z);
JI_API JiVector3D ji_vector3d_zero(void);
JI_API bool       ji_vector3d_equals(JiVector3D a, JiVector3D b);

/* =========================================================================
 * JiThickness — padding/margin descriptor
 * ========================================================================= */
struct JiThickness {
    double left;
    double top;
    double right;
    double bottom;
};

JI_API JiThickness ji_thickness(double uniform);
JI_API JiThickness ji_thickness_lr(double left_right, double top_bottom);
JI_API JiThickness ji_thickness_all(double left, double top, double right, double bottom);
JI_API JiThickness ji_thickness_zero(void);
JI_API bool        ji_thickness_equals(JiThickness a, JiThickness b);
JI_API bool        ji_thickness_is_uniform(JiThickness t);
JI_API double      ji_thickness_horizontal(JiThickness t);
JI_API double      ji_thickness_vertical(JiThickness t);

/* =========================================================================
 * JiCornerRadius — rounded corner radii
 * ========================================================================= */
struct JiCornerRadius {
    double top_left;
    double top_right;
    double bottom_right;
    double bottom_left;
};

JI_API JiCornerRadius ji_corner_radius(double uniform);
JI_API JiCornerRadius ji_corner_radius_all(double tl, double tr, double br, double bl);
JI_API JiCornerRadius ji_corner_radius_zero(void);
JI_API bool           ji_corner_radius_equals(JiCornerRadius a, JiCornerRadius b);

/* =========================================================================
 * JiPixelPoint — integer pixel position
 * ========================================================================= */
struct JiPixelPoint {
    int32_t x;
    int32_t y;
};

JI_API JiPixelPoint ji_pixel_point(int32_t x, int32_t y);
JI_API bool         ji_pixel_point_equals(JiPixelPoint a, JiPixelPoint b);

/* =========================================================================
 * JiPixelSize — integer pixel dimensions
 * ========================================================================= */
struct JiPixelSize {
    int32_t width;
    int32_t height;
};

JI_API JiPixelSize ji_pixel_size(int32_t w, int32_t h);
JI_API bool        ji_pixel_size_equals(JiPixelSize a, JiPixelSize b);

/* =========================================================================
 * JiPixelRect — integer pixel rectangle
 * ========================================================================= */
struct JiPixelRect {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
};

JI_API JiPixelRect ji_pixel_rect(int32_t x, int32_t y, int32_t w, int32_t h);
JI_API bool        ji_pixel_rect_equals(JiPixelRect a, JiPixelRect b);
JI_API bool        ji_pixel_rect_contains(JiPixelRect r, JiPixelPoint p);

/* =========================================================================
 * JiPixelVector — integer pixel direction
 * ========================================================================= */
struct JiPixelVector {
    int32_t x;
    int32_t y;
};

JI_API JiPixelVector ji_pixel_vector(int32_t x, int32_t y);

/* =========================================================================
 * JiMatrix — 3x2 affine transform matrix
 *
 *  | M11  M12  0 |
 *  | M21  M22  0 |
 *  | M31  M32  1 |
 *
 * Stored in row-major: m11, m12, m21, m22, m31(offset_x), m32(offset_y)
 * ========================================================================= */
struct JiMatrix {
    double m11;
    double m12;
    double m21;
    double m22;
    double m31; /* offset_x */
    double m32; /* offset_y */
};

JI_API JiMatrix ji_matrix_identity(void);
JI_API JiMatrix ji_matrix_translation(double dx, double dy);
JI_API JiMatrix ji_matrix_scale(double sx, double sy);
JI_API JiMatrix ji_matrix_rotation(double angle_radians);
JI_API JiMatrix ji_matrix_skew(double angle_x_radians, double angle_y_radians);
JI_API bool     ji_matrix_is_identity(JiMatrix m);
JI_API JiMatrix ji_matrix_multiply(JiMatrix a, JiMatrix b);
JI_API JiPoint  ji_matrix_transform_point(JiMatrix m, JiPoint p);
JI_API JiMatrix ji_matrix_invert(JiMatrix m);
JI_API bool     ji_matrix_try_invert(JiMatrix m, JiMatrix* out);

/* =========================================================================
 * JiRelativePoint — point with relative/absolute units
 * ========================================================================= */
typedef enum JiRelativeUnit {
    JI_RELATIVE_UNIT_ABSOLUTE = 0,
    JI_RELATIVE_UNIT_RELATIVE = 1
} JiRelativeUnit;

struct JiRelativePoint {
    double x;
    double y;
    JiRelativeUnit unit;
};

JI_API JiRelativePoint ji_relative_point(double x, double y, JiRelativeUnit unit);
JI_API JiPoint         ji_relative_point_resolve(JiRelativePoint rp, JiSize available);

/* =========================================================================
 * JiRelativeRect — rectangle with relative/absolute units
 * ========================================================================= */
struct JiRelativeRect {
    JiRelativePoint origin;
    JiSize          size;
    JiRelativeUnit  size_unit;
};

JI_API JiRelativeRect ji_relative_rect(JiRelativePoint origin, JiSize size, JiRelativeUnit unit);
JI_API JiRect         ji_relative_rect_resolve(JiRelativeRect rr, JiSize available);

/* =========================================================================
 * JiRelativeScalar — scalar with relative/absolute units
 * ========================================================================= */
struct JiRelativeScalar {
    double value;
    JiRelativeUnit unit;
};

JI_API JiRelativeScalar ji_relative_scalar(double value, JiRelativeUnit unit);
JI_API double           ji_relative_scalar_resolve(JiRelativeScalar rs, double available);

/* =========================================================================
 * JiRoundedRect — rectangle with rounded corners
 * ========================================================================= */
struct JiRoundedRect {
    JiRect rect;
    JiCornerRadius corner_radius;
};

JI_API JiRoundedRect ji_rounded_rect(JiRect rect, JiCornerRadius radius);

/* =========================================================================
 * JiGridLength — grid column/row sizing
 * ========================================================================= */
typedef enum JiGridUnitType {
    JI_GRID_UNIT_AUTO   = 0,
    JI_GRID_UNIT_PIXEL  = 1,
    JI_GRID_UNIT_STAR   = 2
} JiGridUnitType;

struct JiGridLength {
    double value;
    JiGridUnitType unit_type;
};

JI_API JiGridLength ji_grid_length_auto(void);
JI_API JiGridLength ji_grid_length_pixel(double px);
JI_API JiGridLength ji_grid_length_star(double star);
JI_API bool         ji_grid_length_equals(JiGridLength a, JiGridLength b);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TYPES_H */
