/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d_camera.c
 * @brief 3D camera system — perspective/orthographic, orbit.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x) * (float)(M_PI / 180.0))

JI_API Ji3DCamera* ji_3d_camera_new(void) {
    Ji3DCamera* cam = ji_calloc(1, sizeof(Ji3DCamera));
    if (!cam) return NULL;

    cam->position.x = 0;
    cam->position.y = 5;
    cam->position.z = 10;
    cam->target.x = 0;
    cam->target.y = 0;
    cam->target.z = 0;
    cam->up.x = 0;
    cam->up.y = 1;
    cam->up.z = 0;
    cam->fov = 60.0f;
    cam->aspect = 16.0f / 9.0f;
    cam->near_plane = 0.1f;
    cam->far_plane = 1000.0f;
    cam->ortho_size = 5.0f;
    cam->mode = JI_3D_CAMERA_PERSPECTIVE;

    return cam;
}

JI_API void ji_3d_camera_destroy(Ji3DCamera* camera) {
    if (camera) ji_free(camera);
}

JI_API void ji_3d_camera_set_position(Ji3DCamera* cam, float x, float y, float z) {
    if (!cam) return;
    cam->position.x = x;
    cam->position.y = y;
    cam->position.z = z;
}

JI_API void ji_3d_camera_set_target(Ji3DCamera* cam, float x, float y, float z) {
    if (!cam) return;
    cam->target.x = x;
    cam->target.y = y;
    cam->target.z = z;
}

JI_API void ji_3d_camera_set_fov(Ji3DCamera* cam, float fov) {
    if (cam) cam->fov = fov;
}

JI_API void ji_3d_camera_set_aspect(Ji3DCamera* cam, float aspect) {
    if (cam) cam->aspect = aspect;
}

JI_API void ji_3d_camera_set_clip(Ji3DCamera* cam, float near_p, float far_p) {
    if (!cam) return;
    cam->near_plane = near_p;
    cam->far_plane = far_p;
}

JI_API void ji_3d_camera_set_mode(Ji3DCamera* cam, Ji3DCameraMode mode) {
    if (cam) cam->mode = mode;
}

JI_API JiMat4 ji_3d_camera_view_matrix(const Ji3DCamera* cam) {
    if (!cam) return ji_mat4_identity();
    return ji_mat4_look_at(cam->position, cam->target, cam->up);
}

JI_API JiMat4 ji_3d_camera_projection_matrix(const Ji3DCamera* cam) {
    if (!cam) return ji_mat4_identity();
    if (cam->mode == JI_3D_CAMERA_PERSPECTIVE) {
        return ji_mat4_perspective(cam->fov, cam->aspect, cam->near_plane, cam->far_plane);
    } else {
        float size = cam->ortho_size;
        float right = size * cam->aspect;
        return ji_mat4_ortho(-right, right, -size, size, cam->near_plane, cam->far_plane);
    }
}

JI_API void ji_3d_camera_orbit(Ji3DCamera* cam, float yaw, float pitch, float distance) {
    if (!cam) return;

    /* Clamp pitch to avoid gimbal lock */
    if (pitch < -89.0f) pitch = -89.0f;
    if (pitch > 89.0f) pitch = 89.0f;

    float yaw_rad = DEG2RAD(yaw);
    float pitch_rad = DEG2RAD(pitch);

    float cp = cosf(pitch_rad);
    cam->position.x = cam->target.x + distance * cp * sinf(yaw_rad);
    cam->position.y = cam->target.y + distance * sinf(pitch_rad);
    cam->position.z = cam->target.z + distance * cp * cosf(yaw_rad);
}
