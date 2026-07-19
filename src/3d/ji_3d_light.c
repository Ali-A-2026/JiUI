/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d_light.c
 * @brief 3D lighting — directional, point, spot, ambient lights.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"

JI_API Ji3DLight* ji_3d_light_new(Ji3DLightType type) {
    Ji3DLight* light = ji_calloc(1, sizeof(Ji3DLight));
    if (!light) return NULL;

    light->type = type;
    light->position.x = 0;
    light->position.y = 10;
    light->position.z = 0;
    light->direction.x = 0;
    light->direction.y = -1;
    light->direction.z = 0;
    light->color.x = 1.0f;
    light->color.y = 1.0f;
    light->color.z = 1.0f;
    light->color.w = 1.0f; /* intensity */
    light->range = 50.0f;
    light->inner_angle = 30.0f;
    light->outer_angle = 45.0f;
    light->cast_shadows = false;

    return light;
}

JI_API void ji_3d_light_destroy(Ji3DLight* light) {
    if (light) ji_free(light);
}

JI_API void ji_3d_light_set_color(Ji3DLight* light, float r, float g, float b, float intensity) {
    if (!light) return;
    light->color.x = r;
    light->color.y = g;
    light->color.z = b;
    light->color.w = intensity;
}

JI_API void ji_3d_light_set_position(Ji3DLight* light, float x, float y, float z) {
    if (!light) return;
    light->position.x = x;
    light->position.y = y;
    light->position.z = z;
}

JI_API void ji_3d_light_set_direction(Ji3DLight* light, float x, float y, float z) {
    if (!light) return;
    light->direction.x = x;
    light->direction.y = y;
    light->direction.z = z;
}
