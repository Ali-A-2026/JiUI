/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d_viewport.c
 * @brief 3D viewport widget — renders a 3D scene with orbit camera.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_object.h"
#include "jiui/ji_type.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x) * (float)(M_PI / 180.0))

/* External type ID for Ji3DViewport (registered at startup) */
static JiTypeId s_3d_viewport_type_id = 0;

/* Get or register the 3D viewport type */
static JiTypeId get_3d_viewport_type(void) {
    if (s_3d_viewport_type_id != 0) return s_3d_viewport_type_id;
    /* Use the base JiObject type as fallback */
    s_3d_viewport_type_id = ji_type_from_name("JiObject");
    return s_3d_viewport_type_id;
}

JI_API Ji3DViewport* ji_3d_viewport_new(void) {
    JiTypeId tid = get_3d_viewport_type();
    Ji3DViewport* vp = (Ji3DViewport*)ji_object_new(tid);
    if (!vp) {
        /* Fallback: allocate directly */
        vp = ji_calloc(1, sizeof(Ji3DViewport));
        if (!vp) return NULL;
    }

    vp->scene = NULL;
    vp->gizmo = NULL;
    vp->orbit_enabled = true;
    vp->pan_enabled = true;
    vp->zoom_enabled = true;
    vp->orbit_speed = 0.3f;
    vp->pan_speed = 0.5f;
    vp->zoom_speed = 0.1f;
    vp->yaw = 0.0f;
    vp->pitch = 30.0f;
    vp->distance = 10.0f;

    return vp;
}

JI_API void ji_3d_viewport_destroy(Ji3DViewport* viewport) {
    if (!viewport) return;
    /* Release via object system if it was created through ji_object_new */
    ji_object_destroy((JiObject*)viewport);
}

JI_API void ji_3d_viewport_set_scene(Ji3DViewport* viewport, Ji3DScene* scene) {
    if (viewport) viewport->scene = scene;
}

JI_API void ji_3d_viewport_set_gizmo(Ji3DViewport* viewport, Ji3DGizmo* gizmo) {
    if (viewport) viewport->gizmo = gizmo;
}

JI_API void ji_3d_viewport_set_orbit(Ji3DViewport* viewport, bool enabled) {
    if (viewport) viewport->orbit_enabled = enabled;
}

JI_API void ji_3d_viewport_set_pan(Ji3DViewport* viewport, bool enabled) {
    if (viewport) viewport->pan_enabled = enabled;
}

JI_API void ji_3d_viewport_set_zoom(Ji3DViewport* viewport, bool enabled) {
    if (viewport) viewport->zoom_enabled = enabled;
}

JI_API void ji_3d_viewport_orbit(Ji3DViewport* viewport, float dx, float dy) {
    if (!viewport || !viewport->orbit_enabled || !viewport->scene) return;

    viewport->yaw -= dx * viewport->orbit_speed;
    viewport->pitch += dy * viewport->orbit_speed;

    /* Clamp pitch */
    if (viewport->pitch < -89.0f) viewport->pitch = -89.0f;
    if (viewport->pitch > 89.0f) viewport->pitch = 89.0f;

    /* Update camera */
    if (viewport->scene->camera) {
        ji_3d_camera_orbit(viewport->scene->camera,
                             viewport->yaw, viewport->pitch, viewport->distance);
    }
}

JI_API void ji_3d_viewport_pan(Ji3DViewport* viewport, float dx, float dy) {
    if (!viewport || !viewport->pan_enabled || !viewport->scene || !viewport->scene->camera) return;

    Ji3DCamera* cam = viewport->scene->camera;

    /* Pan the camera target and position */
    float pan_x = -dx * viewport->pan_speed * viewport->distance * 0.01f;
    float pan_y = dy * viewport->pan_speed * viewport->distance * 0.01f;

    cam->target.x += pan_x;
    cam->target.y += pan_y;
    cam->position.x += pan_x;
    cam->position.y += pan_y;

    /* Re-orbit to keep consistent view */
    ji_3d_camera_orbit(cam, viewport->yaw, viewport->pitch, viewport->distance);
}

JI_API void ji_3d_viewport_zoom(Ji3DViewport* viewport, float delta) {
    if (!viewport || !viewport->zoom_enabled) return;

    viewport->distance *= (1.0f + delta * viewport->zoom_speed);
    if (viewport->distance < 0.1f) viewport->distance = 0.1f;
    if (viewport->distance > 1000.0f) viewport->distance = 1000.0f;

    if (viewport->scene && viewport->scene->camera) {
        ji_3d_camera_orbit(viewport->scene->camera,
                             viewport->yaw, viewport->pitch, viewport->distance);
    }
}
