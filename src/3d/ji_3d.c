/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d.c
 * @brief 3D engine core — math utilities, scene management.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD(x) ((x) * (float)(M_PI / 180.0))
#define RAD2DEG(x) ((x) * (float)(180.0 / M_PI))

/* =========================================================================
 * Vec3 Operations
 * ========================================================================= */

JI_API JiVec3 ji_vec3_new(float x, float y, float z) {
    JiVec3 v = {x, y, z};
    return v;
}

JI_API JiVec3 ji_vec3_add(JiVec3 a, JiVec3 b) {
    JiVec3 r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

JI_API JiVec3 ji_vec3_sub(JiVec3 a, JiVec3 b) {
    JiVec3 r = {a.x - b.x, a.y - b.y, a.z - b.z};
    return r;
}

JI_API float ji_vec3_dot(JiVec3 a, JiVec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

JI_API JiVec3 ji_vec3_cross(JiVec3 a, JiVec3 b) {
    JiVec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

JI_API float ji_vec3_length(JiVec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

JI_API JiVec3 ji_vec3_normalize(JiVec3 v) {
    float len = ji_vec3_length(v);
    if (len < 1e-8f) {
        JiVec3 zero = {0, 0, 0};
        return zero;
    }
    float inv = 1.0f / len;
    JiVec3 r = {v.x * inv, v.y * inv, v.z * inv};
    return r;
}

JI_API JiVec3 ji_vec3_scale(JiVec3 v, float s) {
    JiVec3 r = {v.x * s, v.y * s, v.z * s};
    return r;
}

/* =========================================================================
 * Mat4 Operations
 * ========================================================================= */

JI_API JiMat4 ji_mat4_identity(void) {
    JiMat4 m;
    memset(&m, 0, sizeof(m));
    m.m[0] = 1.0f; m.m[5] = 1.0f; m.m[10] = 1.0f; m.m[15] = 1.0f;
    return m;
}

JI_API JiMat4 ji_mat4_multiply(JiMat4 a, JiMat4 b) {
    JiMat4 result;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            result.m[col * 4 + row] = sum;
        }
    }
    return result;
}

JI_API JiMat4 ji_mat4_perspective(float fov_deg, float aspect, float near_p, float far_p) {
    JiMat4 m;
    memset(&m, 0, sizeof(m));

    float f = 1.0f / tanf(DEG2RAD(fov_deg) * 0.5f);
    float nf = 1.0f / (near_p - far_p);

    m.m[0] = f / aspect;
    m.m[5] = f;
    m.m[10] = (far_p + near_p) * nf;
    m.m[11] = -1.0f;
    m.m[14] = 2.0f * far_p * near_p * nf;

    return m;
}

JI_API JiMat4 ji_mat4_ortho(float left, float right, float bottom, float top,
                              float near_p, float far_p) {
    JiMat4 m;
    memset(&m, 0, sizeof(m));

    m.m[0] = 2.0f / (right - left);
    m.m[5] = 2.0f / (top - bottom);
    m.m[10] = -2.0f / (far_p - near_p);
    m.m[12] = -(right + left) / (right - left);
    m.m[13] = -(top + bottom) / (top - bottom);
    m.m[14] = -(far_p + near_p) / (far_p - near_p);
    m.m[15] = 1.0f;

    return m;
}

JI_API JiMat4 ji_mat4_look_at(JiVec3 eye, JiVec3 target, JiVec3 up) {
    JiVec3 f = ji_vec3_normalize(ji_vec3_sub(target, eye));
    JiVec3 s = ji_vec3_normalize(ji_vec3_cross(f, up));
    JiVec3 u = ji_vec3_cross(s, f);

    JiMat4 m;
    memset(&m, 0, sizeof(m));

    m.m[0] = s.x;  m.m[4] = s.y;  m.m[8]  = s.z;
    m.m[1] = u.x;  m.m[5] = u.y;  m.m[9]  = u.z;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z;
    m.m[12] = -ji_vec3_dot(s, eye);
    m.m[13] = -ji_vec3_dot(u, eye);
    m.m[14] = ji_vec3_dot(f, eye);
    m.m[15] = 1.0f;

    return m;
}

/* =========================================================================
 * Quaternion Operations
 * ========================================================================= */

JI_API JiQuat ji_quat_identity(void) {
    JiQuat q = {0.0f, 0.0f, 0.0f, 1.0f};
    return q;
}

JI_API JiQuat ji_quat_from_axis_angle(float ax, float ay, float az, float angle) {
    JiVec3 axis = ji_vec3_normalize(ji_vec3_new(ax, ay, az));
    float half = angle * 0.5f;
    float sin_half = sinf(half);
    JiQuat q;
    q.x = axis.x * sin_half;
    q.y = axis.y * sin_half;
    q.z = axis.z * sin_half;
    q.w = cosf(half);
    return q;
}

JI_API JiQuat ji_quat_multiply(JiQuat a, JiQuat b) {
    JiQuat r;
    r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    return r;
}

JI_API JiQuat ji_quat_normalize(JiQuat q) {
    float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len < 1e-8f) return ji_quat_identity();
    float inv = 1.0f / len;
    JiQuat r = {q.x * inv, q.y * inv, q.z * inv, q.w * inv};
    return r;
}

/* =========================================================================
 * Transform
 * ========================================================================= */

JI_API Ji3DTransform ji_3d_transform_identity(void) {
    Ji3DTransform t;
    t.position = ji_vec3_new(0, 0, 0);
    t.rotation = ji_quat_identity();
    t.scale = ji_vec3_new(1, 1, 1);
    return t;
}

JI_API JiMat4 ji_3d_transform_to_matrix(const Ji3DTransform* t) {
    if (!t) return ji_mat4_identity();

    /* T * R * S */
    JiQuat q = t->rotation;
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    float qx2 = qx * qx, qy2 = qy * qy, qz2 = qz * qz;

    JiMat4 m;
    memset(&m, 0, sizeof(m));

    float sx = t->scale.x, sy = t->scale.y, sz = t->scale.z;

    m.m[0] = (1.0f - 2.0f * (qy2 + qz2)) * sx;
    m.m[1] = (2.0f * (qx * qy + qw * qz)) * sx;
    m.m[2] = (2.0f * (qx * qz - qw * qy)) * sx;

    m.m[4] = (2.0f * (qx * qy - qw * qz)) * sy;
    m.m[5] = (1.0f - 2.0f * (qx2 + qz2)) * sy;
    m.m[6] = (2.0f * (qy * qz + qw * qx)) * sy;

    m.m[8]  = (2.0f * (qx * qz + qw * qy)) * sz;
    m.m[9]  = (2.0f * (qy * qz - qw * qx)) * sz;
    m.m[10] = (1.0f - 2.0f * (qx2 + qy2)) * sz;

    m.m[12] = t->position.x;
    m.m[13] = t->position.y;
    m.m[14] = t->position.z;
    m.m[15] = 1.0f;

    return m;
}

JI_API void ji_3d_transform_translate(Ji3DTransform* t, float x, float y, float z) {
    if (!t) return;
    t->position.x += x;
    t->position.y += y;
    t->position.z += z;
}

JI_API void ji_3d_transform_rotate(Ji3DTransform* t, float ax, float ay, float az, float angle) {
    if (!t) return;
    JiQuat rot = ji_quat_from_axis_angle(ax, ay, az, angle);
    t->rotation = ji_quat_multiply(t->rotation, rot);
    t->rotation = ji_quat_normalize(t->rotation);
}

JI_API void ji_3d_transform_scale(Ji3DTransform* t, float sx, float sy, float sz) {
    if (!t) return;
    t->scale.x *= sx;
    t->scale.y *= sy;
    t->scale.z *= sz;
}

/* =========================================================================
 * Scene
 * ========================================================================= */

JI_API Ji3DScene* ji_3d_scene_new(void) {
    Ji3DScene* scene = ji_calloc(1, sizeof(Ji3DScene));
    if (!scene) return NULL;

    scene->object_capacity = 16;
    scene->objects = ji_calloc(scene->object_capacity, sizeof(Ji3DSceneObject*));

    scene->light_capacity = 8;
    scene->lights = ji_calloc(scene->light_capacity, sizeof(Ji3DLight*));

    scene->camera = ji_3d_camera_new();
    scene->background_color.x = 0.1f;
    scene->background_color.y = 0.1f;
    scene->background_color.z = 0.1f;
    scene->background_color.w = 1.0f;
    scene->show_grid = true;
    scene->show_gizmos = true;

    return scene;
}

JI_API void ji_3d_scene_destroy(Ji3DScene* scene) {
    if (!scene) return;
    ji_free(scene->objects);
    ji_free(scene->lights);
    if (scene->camera) ji_3d_camera_destroy(scene->camera);
    ji_free(scene);
}

JI_API bool ji_3d_scene_add_object(Ji3DScene* scene, Ji3DSceneObject* obj) {
    if (!scene || !obj) return false;
    if (scene->object_count >= scene->object_capacity) {
        int new_cap = scene->object_capacity * 2;
        Ji3DSceneObject** new_arr = ji_realloc(scene->objects, new_cap * sizeof(Ji3DSceneObject*));
        if (!new_arr) return false;
        scene->objects = new_arr;
        scene->object_capacity = new_cap;
    }
    scene->objects[scene->object_count++] = obj;
    return true;
}

JI_API bool ji_3d_scene_remove_object(Ji3DScene* scene, Ji3DSceneObject* obj) {
    if (!scene || !obj) return false;
    for (int i = 0; i < scene->object_count; i++) {
        if (scene->objects[i] == obj) {
            for (int j = i; j < scene->object_count - 1; j++) {
                scene->objects[j] = scene->objects[j + 1];
            }
            scene->object_count--;
            return true;
        }
    }
    return false;
}

JI_API bool ji_3d_scene_add_light(Ji3DScene* scene, Ji3DLight* light) {
    if (!scene || !light) return false;
    if (scene->light_count >= scene->light_capacity) {
        int new_cap = scene->light_capacity * 2;
        Ji3DLight** new_arr = ji_realloc(scene->lights, new_cap * sizeof(Ji3DLight*));
        if (!new_arr) return false;
        scene->lights = new_arr;
        scene->light_capacity = new_cap;
    }
    scene->lights[scene->light_count++] = light;
    return true;
}

JI_API bool ji_3d_scene_remove_light(Ji3DScene* scene, Ji3DLight* light) {
    if (!scene || !light) return false;
    for (int i = 0; i < scene->light_count; i++) {
        if (scene->lights[i] == light) {
            for (int j = i; j < scene->light_count - 1; j++) {
                scene->lights[j] = scene->lights[j + 1];
            }
            scene->light_count--;
            return true;
        }
    }
    return false;
}

JI_API void ji_3d_scene_set_camera(Ji3DScene* scene, Ji3DCamera* camera) {
    if (!scene) return;
    if (scene->camera) ji_3d_camera_destroy(scene->camera);
    scene->camera = camera;
}

JI_API void ji_3d_scene_set_background(Ji3DScene* scene, float r, float g, float b, float a) {
    if (!scene) return;
    scene->background_color.x = r;
    scene->background_color.y = g;
    scene->background_color.z = b;
    scene->background_color.w = a;
}

JI_API int ji_3d_scene_object_count(const Ji3DScene* scene) {
    return scene ? scene->object_count : 0;
}

JI_API int ji_3d_scene_light_count(const Ji3DScene* scene) {
    return scene ? scene->light_count : 0;
}

/* =========================================================================
 * Scene Object
 * ========================================================================= */

JI_API Ji3DSceneObject* ji_3d_scene_object_new(Ji3DMesh* mesh, Ji3DMaterial* mat) {
    Ji3DSceneObject* obj = ji_calloc(1, sizeof(Ji3DSceneObject));
    if (!obj) return NULL;
    obj->mesh = mesh;
    obj->material = mat;
    obj->transform = ji_3d_transform_identity();
    obj->visible = true;
    return obj;
}

JI_API void ji_3d_scene_object_destroy(Ji3DSceneObject* obj) {
    if (!obj) return;
    ji_free(obj);
}

JI_API void ji_3d_scene_object_set_transform(Ji3DSceneObject* obj, const Ji3DTransform* t) {
    if (obj && t) obj->transform = *t;
}

JI_API void ji_3d_scene_object_set_visible(Ji3DSceneObject* obj, bool visible) {
    if (obj) obj->visible = visible;
}
