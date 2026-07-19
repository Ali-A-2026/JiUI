/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d.h
 * @brief 3D UI layer — 3D viewport, meshes, cameras, lights, gizmos.
 *
 * Provides a 3D rendering layer that integrates with the JiUI scene graph:
 *   - 3D viewport widget (orbit camera, pan, zoom)
 *   - Mesh loading (OBJ) and primitives (cube, sphere, plane)
 *   - Camera system (perspective, orthographic)
 *   - Lighting (directional, point, ambient)
 *   - Gizmos (translate, rotate, scale)
 *   - 3D text rendering
 *   - VR/AR stereo rendering hooks
 */

#ifndef JIUI_3D_H
#define JIUI_3D_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include "ji_object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Math Types
 * ========================================================================= */

/** 2D vector. */
typedef struct JiVec2 {
    float x, y;
} JiVec2;

/** 3D vector. */
typedef struct JiVec3 {
    float x, y, z;
} JiVec3;

/** 4D vector. */
typedef struct JiVec4 {
    float x, y, z, w;
} JiVec4;

/** Quaternion. */
typedef struct JiQuat {
    float x, y, z, w;
} JiQuat;

/** 4x4 matrix (column-major). */
typedef struct JiMat4 {
    float m[16];
} JiMat4;

/* =========================================================================
 * Mesh
 * ========================================================================= */

/** Vertex with position, normal, and texture coordinates. */
typedef struct Ji3DVertex {
    JiVec3  position;
    JiVec3  normal;
    JiVec2  texcoord;
} Ji3DVertex;

/** A 3D mesh: vertices + indices. */
typedef struct Ji3DMesh {
    Ji3DVertex*  vertices;
    int          vertex_count;
    uint32_t*    indices;
    int          index_count;
    char         name[128];
    bool         owns_vertices;
    bool         owns_indices;
} Ji3DMesh;

/** Create a new empty mesh. */
JI_API Ji3DMesh* ji_3d_mesh_new(const char* name);

/** Destroy a mesh. */
JI_API void ji_3d_mesh_destroy(Ji3DMesh* mesh);

/** Create a cube mesh. */
JI_API Ji3DMesh* ji_3d_mesh_cube(void);

/** Create a sphere mesh with given segment count. */
JI_API Ji3DMesh* ji_3d_mesh_sphere(int segments);

/** Create a plane mesh. */
JI_API Ji3DMesh* ji_3d_mesh_plane(float width, float depth);

/** Load a mesh from an OBJ file. */
JI_API Ji3DMesh* ji_3d_mesh_from_obj(const char* path);

/** Set mesh vertex data (copies the data). */
JI_API bool ji_3d_mesh_set_vertices(Ji3DMesh* mesh, const Ji3DVertex* verts, int count);

/** Set mesh index data (copies the data). */
JI_API bool ji_3d_mesh_set_indices(Ji3DMesh* mesh, const uint32_t* indices, int count);

/** Get vertex count. */
JI_API int ji_3d_mesh_vertex_count(const Ji3DMesh* mesh);

/** Get index count. */
JI_API int ji_3d_mesh_index_count(const Ji3DMesh* mesh);

/* =========================================================================
 * Material
 * ========================================================================= */

/** A simple 3D material. */
typedef struct Ji3DMaterial {
    JiVec4  base_color;       /* RGBA base color */
    float   metallic;         /* 0 = dielectric, 1 = metal */
    float   roughness;        /* 0 = smooth, 1 = rough */
    float   emissive;         /* Emissive intensity */
    JiVec4  emissive_color;   /* Emissive color */
    bool    wireframe;        /* Render as wireframe */
    bool    double_sided;     /* Render both faces */
} Ji3DMaterial;

/** Create a new default material. */
JI_API Ji3DMaterial* ji_3d_material_new(void);

/** Destroy a material. */
JI_API void ji_3d_material_destroy(Ji3DMaterial* mat);

/** Set base color (RGBA 0..1). */
JI_API void ji_3d_material_set_color(Ji3DMaterial* mat, float r, float g, float b, float a);

/** Set metallic/roughness. */
JI_API void ji_3d_material_set_metallic_roughness(Ji3DMaterial* mat,
                                                    float metallic, float roughness);

/* =========================================================================
 * Transform
 * ========================================================================= */

/** 3D transform: position, rotation (quaternion), scale. */
typedef struct Ji3DTransform {
    JiVec3  position;
    JiQuat  rotation;
    JiVec3  scale;
} Ji3DTransform;

/** Identity transform. */
JI_API Ji3DTransform ji_3d_transform_identity(void);

/** Compute the model matrix from a transform. */
JI_API JiMat4 ji_3d_transform_to_matrix(const Ji3DTransform* t);

/** Translate a transform. */
JI_API void ji_3d_transform_translate(Ji3DTransform* t, float x, float y, float z);

/** Rotate a transform by an axis-angle. */
JI_API void ji_3d_transform_rotate(Ji3DTransform* t, float ax, float ay, float az, float angle);

/** Scale a transform. */
JI_API void ji_3d_transform_scale(Ji3DTransform* t, float sx, float sy, float sz);

/* =========================================================================
 * Camera
 * ========================================================================= */

/** Camera projection mode. */
typedef enum Ji3DCameraMode {
    JI_3D_CAMERA_PERSPECTIVE = 0,
    JI_3D_CAMERA_ORTHOGRAPHIC
} Ji3DCameraMode;

/** 3D camera. */
typedef struct Ji3DCamera {
    JiVec3          position;
    JiVec3          target;
    JiVec3          up;
    float           fov;           /* Field of view in degrees (perspective) */
    float           aspect;        /* Width / height */
    float           near_plane;
    float           far_plane;
    float           ortho_size;    /* Half-height for orthographic */
    Ji3DCameraMode  mode;
} Ji3DCamera;

/** Create a new perspective camera. */
JI_API Ji3DCamera* ji_3d_camera_new(void);

/** Destroy a camera. */
JI_API void ji_3d_camera_destroy(Ji3DCamera* camera);

/** Set camera position. */
JI_API void ji_3d_camera_set_position(Ji3DCamera* cam, float x, float y, float z);

/** Set camera look-at target. */
JI_API void ji_3d_camera_set_target(Ji3DCamera* cam, float x, float y, float z);

/** Set camera FOV (degrees). */
JI_API void ji_3d_camera_set_fov(Ji3DCamera* cam, float fov);

/** Set camera aspect ratio. */
JI_API void ji_3d_camera_set_aspect(Ji3DCamera* cam, float aspect);

/** Set camera near/far planes. */
JI_API void ji_3d_camera_set_clip(Ji3DCamera* cam, float near_p, float far_p);

/** Set camera projection mode. */
JI_API void ji_3d_camera_set_mode(Ji3DCamera* cam, Ji3DCameraMode mode);

/** Compute the view matrix. */
JI_API JiMat4 ji_3d_camera_view_matrix(const Ji3DCamera* cam);

/** Compute the projection matrix. */
JI_API JiMat4 ji_3d_camera_projection_matrix(const Ji3DCamera* cam);

/** Orbit camera around target by yaw/pitch/distance. */
JI_API void ji_3d_camera_orbit(Ji3DCamera* cam, float yaw, float pitch, float distance);

/* =========================================================================
 * Light
 * ========================================================================= */

/** Light type. */
typedef enum Ji3DLightType {
    JI_3D_LIGHT_DIRECTIONAL = 0,
    JI_3D_LIGHT_POINT,
    JI_3D_LIGHT_SPOT,
    JI_3D_LIGHT_AMBIENT
} Ji3DLightType;

/** 3D light source. */
typedef struct Ji3DLight {
    Ji3DLightType  type;
    JiVec3        position;     /* World position (point/spot) */
    JiVec3        direction;    /* Direction (directional/spot) */
    JiVec4        color;        /* Light color + intensity in w */
    float         range;        /* Attenuation range (point/spot) */
    float         inner_angle;  /* Spot inner cone angle (degrees) */
    float         outer_angle;  /* Spot outer cone angle (degrees) */
    bool          cast_shadows; /* Whether this light casts shadows */
} Ji3DLight;

/** Create a new light. */
JI_API Ji3DLight* ji_3d_light_new(Ji3DLightType type);

/** Destroy a light. */
JI_API void ji_3d_light_destroy(Ji3DLight* light);

/** Set light color. */
JI_API void ji_3d_light_set_color(Ji3DLight* light, float r, float g, float b, float intensity);

/** Set light position. */
JI_API void ji_3d_light_set_position(Ji3DLight* light, float x, float y, float z);

/** Set light direction. */
JI_API void ji_3d_light_set_direction(Ji3DLight* light, float x, float y, float z);

/* =========================================================================
 * Scene Object — a mesh instance in the scene
 * ========================================================================= */

/** A scene object: mesh + material + transform. */
typedef struct Ji3DSceneObject {
    Ji3DMesh*        mesh;
    Ji3DMaterial*    material;
    Ji3DTransform    transform;
    bool             visible;
    char             name[128];
} Ji3DSceneObject;

/** Create a new scene object. */
JI_API Ji3DSceneObject* ji_3d_scene_object_new(Ji3DMesh* mesh, Ji3DMaterial* mat);

/** Destroy a scene object (does not destroy mesh/material). */
JI_API void ji_3d_scene_object_destroy(Ji3DSceneObject* obj);

/** Set object transform. */
JI_API void ji_3d_scene_object_set_transform(Ji3DSceneObject* obj, const Ji3DTransform* t);

/** Set object visibility. */
JI_API void ji_3d_scene_object_set_visible(Ji3DSceneObject* obj, bool visible);

/* =========================================================================
 * 3D Scene — collection of objects, lights, and camera
 * ========================================================================= */

/** 3D scene. */
typedef struct Ji3DScene {
    Ji3DSceneObject**  objects;
    int                object_count;
    int                object_capacity;
    Ji3DLight**        lights;
    int                light_count;
    int                light_capacity;
    Ji3DCamera*        camera;
    JiVec4             background_color;
    bool               show_grid;
    bool               show_gizmos;
} Ji3DScene;

/** Create a new 3D scene. */
JI_API Ji3DScene* ji_3d_scene_new(void);

/** Destroy a 3D scene. */
JI_API void ji_3d_scene_destroy(Ji3DScene* scene);

/** Add a scene object to the scene. */
JI_API bool ji_3d_scene_add_object(Ji3DScene* scene, Ji3DSceneObject* obj);

/** Remove a scene object (does not destroy it). */
JI_API bool ji_3d_scene_remove_object(Ji3DScene* scene, Ji3DSceneObject* obj);

/** Add a light to the scene. */
JI_API bool ji_3d_scene_add_light(Ji3DScene* scene, Ji3DLight* light);

/** Remove a light (does not destroy it). */
JI_API bool ji_3d_scene_remove_light(Ji3DScene* scene, Ji3DLight* light);

/** Set the scene's active camera. */
JI_API void ji_3d_scene_set_camera(Ji3DScene* scene, Ji3DCamera* camera);

/** Set background color. */
JI_API void ji_3d_scene_set_background(Ji3DScene* scene, float r, float g, float b, float a);

/** Get object count. */
JI_API int ji_3d_scene_object_count(const Ji3DScene* scene);

/** Get light count. */
JI_API int ji_3d_scene_light_count(const Ji3DScene* scene);

/* =========================================================================
 * Gizmo — manipulation handles
 * ========================================================================= */

/** Gizmo type. */
typedef enum Ji3DGizmoType {
    JI_3D_GIZMO_NONE = 0,
    JI_3D_GIZMO_TRANSLATE,
    JI_3D_GIZMO_ROTATE,
    JI_3D_GIZMO_SCALE,
    JI_3D_GIZMO_UNIVERSAL
} Ji3DGizmoType;

/** Gizmo state. */
typedef struct Ji3DGizmo {
    Ji3DGizmoType    type;
    Ji3DTransform    transform;
    bool             visible;
    bool             active;
    int              active_axis;  /* 0=X, 1=Y, 2=Z, -1=none */
    float            size;         /* Gizmo scale */
} Ji3DGizmo;

/** Create a new gizmo. */
JI_API Ji3DGizmo* ji_3d_gizmo_new(Ji3DGizmoType type);

/** Destroy a gizmo. */
JI_API void ji_3d_gizmo_destroy(Ji3DGizmo* gizmo);

/** Set gizmo type. */
JI_API void ji_3d_gizmo_set_type(Ji3DGizmo* gizmo, Ji3DGizmoType type);

/** Set gizmo transform (follows selected object). */
JI_API void ji_3d_gizmo_set_transform(Ji3DGizmo* gizmo, const Ji3DTransform* t);

/** Set gizmo visibility. */
JI_API void ji_3d_gizmo_set_visible(Ji3DGizmo* gizmo, bool visible);

/** Check if gizmo is active (being dragged). */
JI_API bool ji_3d_gizmo_is_active(const Ji3DGizmo* gizmo);

/* =========================================================================
 * 3D Viewport — widget that renders a 3D scene
 * ========================================================================= */

/** 3D viewport widget. */
typedef struct Ji3DViewport {
    JiObject     base;            /* Base object (for widget tree integration) */
    Ji3DScene*   scene;           /* Scene to render */
    Ji3DGizmo*   gizmo;           /* Active gizmo */
    bool         orbit_enabled;   /* Allow orbit camera with mouse */
    bool         pan_enabled;     /* Allow panning with mouse */
    bool         zoom_enabled;    /* Allow zoom with mouse wheel */
    float       orbit_speed;     /* Mouse orbit sensitivity */
    float       pan_speed;       /* Mouse pan sensitivity */
    float       zoom_speed;      /* Mouse zoom sensitivity */
    float       yaw;             /* Current orbit yaw */
    float       pitch;           /* Current orbit pitch */
    float       distance;        /* Camera distance from target */
} Ji3DViewport;

/** Create a new 3D viewport widget. */
JI_API Ji3DViewport* ji_3d_viewport_new(void);

/** Destroy a 3D viewport. */
JI_API void ji_3d_viewport_destroy(Ji3DViewport* viewport);

/** Set the scene to render. */
JI_API void ji_3d_viewport_set_scene(Ji3DViewport* viewport, Ji3DScene* scene);

/** Set the gizmo. */
JI_API void ji_3d_viewport_set_gizmo(Ji3DViewport* viewport, Ji3DGizmo* gizmo);

/** Enable/disable orbit camera. */
JI_API void ji_3d_viewport_set_orbit(Ji3DViewport* viewport, bool enabled);

/** Enable/disable pan. */
JI_API void ji_3d_viewport_set_pan(Ji3DViewport* viewport, bool enabled);

/** Enable/disable zoom. */
JI_API void ji_3d_viewport_set_zoom(Ji3DViewport* viewport, bool enabled);

/** Handle mouse orbit (dx, dy in pixels). */
JI_API void ji_3d_viewport_orbit(Ji3DViewport* viewport, float dx, float dy);

/** Handle mouse pan. */
JI_API void ji_3d_viewport_pan(Ji3DViewport* viewport, float dx, float dy);

/** Handle mouse zoom. */
JI_API void ji_3d_viewport_zoom(Ji3DViewport* viewport, float delta);

/* =========================================================================
 * Math Utilities
 * ========================================================================= */

/** Create a Vec3. */
JI_API JiVec3 ji_vec3_new(float x, float y, float z);

/** Vec3 addition. */
JI_API JiVec3 ji_vec3_add(JiVec3 a, JiVec3 b);

/** Vec3 subtraction. */
JI_API JiVec3 ji_vec3_sub(JiVec3 a, JiVec3 b);

/** Vec3 dot product. */
JI_API float ji_vec3_dot(JiVec3 a, JiVec3 b);

/** Vec3 cross product. */
JI_API JiVec3 ji_vec3_cross(JiVec3 a, JiVec3 b);

/** Vec3 length. */
JI_API float ji_vec3_length(JiVec3 v);

/** Vec3 normalize. */
JI_API JiVec3 ji_vec3_normalize(JiVec3 v);

/** Vec3 scale. */
JI_API JiVec3 ji_vec3_scale(JiVec3 v, float s);

/** Identity matrix. */
JI_API JiMat4 ji_mat4_identity(void);

/** Multiply two matrices. */
JI_API JiMat4 ji_mat4_multiply(JiMat4 a, JiMat4 b);

/** Create a perspective projection matrix. */
JI_API JiMat4 ji_mat4_perspective(float fov_deg, float aspect, float near_p, float far_p);

/** Create an orthographic projection matrix. */
JI_API JiMat4 ji_mat4_ortho(float left, float right, float bottom, float top,
                              float near_p, float far_p);

/** Create a look-at view matrix. */
JI_API JiMat4 ji_mat4_look_at(JiVec3 eye, JiVec3 target, JiVec3 up);

/** Identity quaternion. */
JI_API JiQuat ji_quat_identity(void);

/** Quaternion from axis-angle. */
JI_API JiQuat ji_quat_from_axis_angle(float ax, float ay, float az, float angle);

/** Multiply two quaternions. */
JI_API JiQuat ji_quat_multiply(JiQuat a, JiQuat b);

/** Normalize a quaternion. */
JI_API JiQuat ji_quat_normalize(JiQuat q);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_3D_H */
