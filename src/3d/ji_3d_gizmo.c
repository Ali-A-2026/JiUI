/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d_gizmo.c
 * @brief 3D gizmos — translate, rotate, scale manipulation handles.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"

JI_API Ji3DGizmo* ji_3d_gizmo_new(Ji3DGizmoType type) {
    Ji3DGizmo* gizmo = ji_calloc(1, sizeof(Ji3DGizmo));
    if (!gizmo) return NULL;

    gizmo->type = type;
    gizmo->transform = ji_3d_transform_identity();
    gizmo->visible = true;
    gizmo->active = false;
    gizmo->active_axis = -1;
    gizmo->size = 1.0f;

    return gizmo;
}

JI_API void ji_3d_gizmo_destroy(Ji3DGizmo* gizmo) {
    if (gizmo) ji_free(gizmo);
}

JI_API void ji_3d_gizmo_set_type(Ji3DGizmo* gizmo, Ji3DGizmoType type) {
    if (gizmo) gizmo->type = type;
}

JI_API void ji_3d_gizmo_set_transform(Ji3DGizmo* gizmo, const Ji3DTransform* t) {
    if (gizmo && t) gizmo->transform = *t;
}

JI_API void ji_3d_gizmo_set_visible(Ji3DGizmo* gizmo, bool visible) {
    if (gizmo) gizmo->visible = visible;
}

JI_API bool ji_3d_gizmo_is_active(const Ji3DGizmo* gizmo) {
    return gizmo ? gizmo->active : false;
}

/* Set the active axis for gizmo dragging (0=X, 1=Y, 2=Z, -1=none) */
void ji_3d_gizmo_set_active_axis(Ji3DGizmo* gizmo, int axis) {
    if (!gizmo) return;
    gizmo->active_axis = axis;
    gizmo->active = (axis >= 0);
}

/* Get the active axis */
int ji_3d_gizmo_get_active_axis(const Ji3DGizmo* gizmo) {
    return gizmo ? gizmo->active_axis : -1;
}

/* Set gizmo size (scale factor) */
void ji_3d_gizmo_set_size(Ji3DGizmo* gizmo, float size) {
    if (gizmo) gizmo->size = size;
}

/* Get gizmo size */
float ji_3d_gizmo_get_size(const Ji3DGizmo* gizmo) {
    return gizmo ? gizmo->size : 1.0f;
}

/* Apply a translation delta to the gizmo's transform along the active axis */
void ji_3d_gizmo_apply_translation(Ji3DGizmo* gizmo, float dx, float dy, float dz) {
    if (!gizmo || gizmo->active_axis < 0) return;
    switch (gizmo->active_axis) {
        case 0: ji_3d_transform_translate(&gizmo->transform, dx, 0, 0); break;
        case 1: ji_3d_transform_translate(&gizmo->transform, 0, dy, 0); break;
        case 2: ji_3d_transform_translate(&gizmo->transform, 0, 0, dz); break;
    }
}

/* Apply a rotation delta to the gizmo's transform around the active axis */
void ji_3d_gizmo_apply_rotation(Ji3DGizmo* gizmo, float delta_angle) {
    if (!gizmo || gizmo->active_axis < 0) return;
    switch (gizmo->active_axis) {
        case 0: ji_3d_transform_rotate(&gizmo->transform, 1, 0, 0, delta_angle); break;
        case 1: ji_3d_transform_rotate(&gizmo->transform, 0, 1, 0, delta_angle); break;
        case 2: ji_3d_transform_rotate(&gizmo->transform, 0, 0, 1, delta_angle); break;
    }
}

/* Apply a scale delta to the gizmo's transform along the active axis */
void ji_3d_gizmo_apply_scale(Ji3DGizmo* gizmo, float factor) {
    if (!gizmo || gizmo->active_axis < 0) return;
    switch (gizmo->active_axis) {
        case 0: ji_3d_transform_scale(&gizmo->transform, factor, 1, 1); break;
        case 1: ji_3d_transform_scale(&gizmo->transform, 1, factor, 1); break;
        case 2: ji_3d_transform_scale(&gizmo->transform, 1, 1, factor); break;
    }
}
