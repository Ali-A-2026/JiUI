/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_3d.c
 * @brief Tests for the 3D UI layer — math, meshes, camera, lights, gizmos.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

#define ASSERT_NEAR(a, b, eps) \
    do { if (fabs((double)(a) - (double)(b)) > (eps)) { printf("  [FAIL] %s:%d: %g != %g\n", \
        __FILE__, __LINE__, (double)(a), (double)(b)); assert(0); } } while(0)

/* =========================================================================
 * Math Tests
 * ========================================================================= */

static void test_vec3_operations(void) {
    JiVec3 a = ji_vec3_new(1, 2, 3);
    JiVec3 b = ji_vec3_new(4, 5, 6);

    JiVec3 add = ji_vec3_add(a, b);
    ASSERT_NEAR(add.x, 5, 1e-6);
    ASSERT_NEAR(add.y, 7, 1e-6);
    ASSERT_NEAR(add.z, 9, 1e-6);

    JiVec3 sub = ji_vec3_sub(a, b);
    ASSERT_NEAR(sub.x, -3, 1e-6);
    ASSERT_NEAR(sub.y, -3, 1e-6);
    ASSERT_NEAR(sub.z, -3, 1e-6);

    float dot = ji_vec3_dot(a, b);
    ASSERT_NEAR(dot, 32, 1e-6);

    JiVec3 cross = ji_vec3_cross(a, b);
    ASSERT_NEAR(cross.x, -3, 1e-6);
    ASSERT_NEAR(cross.y, 6, 1e-6);
    ASSERT_NEAR(cross.z, -3, 1e-6);

    float len = ji_vec3_length(a);
    ASSERT_NEAR(len, sqrtf(14), 1e-6);

    JiVec3 norm = ji_vec3_normalize(a);
    ASSERT_NEAR(ji_vec3_length(norm), 1.0, 1e-5);

    JiVec3 scaled = ji_vec3_scale(a, 2);
    ASSERT_NEAR(scaled.x, 2, 1e-6);
    ASSERT_NEAR(scaled.y, 4, 1e-6);
    ASSERT_NEAR(scaled.z, 6, 1e-6);
}

static void test_mat4_identity(void) {
    JiMat4 m = ji_mat4_identity();
    ASSERT_NEAR(m.m[0], 1, 1e-6);
    ASSERT_NEAR(m.m[5], 1, 1e-6);
    ASSERT_NEAR(m.m[10], 1, 1e-6);
    ASSERT_NEAR(m.m[15], 1, 1e-6);
    ASSERT_NEAR(m.m[1], 0, 1e-6);
    ASSERT_NEAR(m.m[4], 0, 1e-6);
}

static void test_mat4_multiply(void) {
    JiMat4 a = ji_mat4_identity();
    JiMat4 b = ji_mat4_identity();
    JiMat4 c = ji_mat4_multiply(a, b);
    /* Identity * Identity = Identity */
    ASSERT_NEAR(c.m[0], 1, 1e-6);
    ASSERT_NEAR(c.m[5], 1, 1e-6);
    ASSERT_NEAR(c.m[10], 1, 1e-6);
    ASSERT_NEAR(c.m[15], 1, 1e-6);
}

static void test_mat4_perspective(void) {
    JiMat4 p = ji_mat4_perspective(90.0f, 1.0f, 0.1f, 100.0f);
    /* Check some known values for 90 degree FOV, aspect 1 */
    ASSERT_NEAR(p.m[0], 1.0f, 1e-4);
    ASSERT_NEAR(p.m[5], 1.0f, 1e-4);
    ASSERT_NEAR(p.m[11], -1.0f, 1e-4);
}

static void test_quat_identity(void) {
    JiQuat q = ji_quat_identity();
    ASSERT_NEAR(q.w, 1, 1e-6);
    ASSERT_NEAR(q.x, 0, 1e-6);
    ASSERT_NEAR(q.y, 0, 1e-6);
    ASSERT_NEAR(q.z, 0, 1e-6);
}

static void test_quat_axis_angle(void) {
    /* 90 degree rotation around Y axis = PI/2 radians */
    float angle_rad = 90.0f * 3.14159265f / 180.0f;
    JiQuat q = ji_quat_from_axis_angle(0, 1, 0, angle_rad);
    /* half-angle = 45 degrees = PI/4 radians */
    float half_rad = angle_rad * 0.5f;
    ASSERT_NEAR(q.w, cosf(half_rad), 1e-4);
    ASSERT_NEAR(q.y, sinf(half_rad), 1e-4);
    ASSERT_NEAR(q.x, 0, 1e-6);
    ASSERT_NEAR(q.z, 0, 1e-6);
}

/* =========================================================================
 * Transform Tests
 * ========================================================================= */

static void test_transform_identity(void) {
    Ji3DTransform t = ji_3d_transform_identity();
    ASSERT_NEAR(t.position.x, 0, 1e-6);
    ASSERT_NEAR(t.position.y, 0, 1e-6);
    ASSERT_NEAR(t.position.z, 0, 1e-6);
    ASSERT_NEAR(t.rotation.w, 1, 1e-6);
    ASSERT_NEAR(t.scale.x, 1, 1e-6);
    ASSERT_NEAR(t.scale.y, 1, 1e-6);
    ASSERT_NEAR(t.scale.z, 1, 1e-6);
}

static void test_transform_translate(void) {
    Ji3DTransform t = ji_3d_transform_identity();
    ji_3d_transform_translate(&t, 5, 10, 15);
    ASSERT_NEAR(t.position.x, 5, 1e-6);
    ASSERT_NEAR(t.position.y, 10, 1e-6);
    ASSERT_NEAR(t.position.z, 15, 1e-6);
}

static void test_transform_scale(void) {
    Ji3DTransform t = ji_3d_transform_identity();
    ji_3d_transform_scale(&t, 2, 3, 4);
    ASSERT_NEAR(t.scale.x, 2, 1e-6);
    ASSERT_NEAR(t.scale.y, 3, 1e-6);
    ASSERT_NEAR(t.scale.z, 4, 1e-6);
}

static void test_transform_to_matrix(void) {
    Ji3DTransform t = ji_3d_transform_identity();
    JiMat4 m = ji_3d_transform_to_matrix(&t);
    /* Identity transform should produce identity matrix */
    ASSERT_NEAR(m.m[0], 1, 1e-6);
    ASSERT_NEAR(m.m[5], 1, 1e-6);
    ASSERT_NEAR(m.m[10], 1, 1e-6);
    ASSERT_NEAR(m.m[15], 1, 1e-6);
}

/* =========================================================================
 * Mesh Tests
 * ========================================================================= */

static void test_mesh_create(void) {
    Ji3DMesh* mesh = ji_3d_mesh_new("test");
    ASSERT_TRUE(mesh != NULL);
    ASSERT_TRUE(strcmp(mesh->name, "test") == 0);
    ASSERT_TRUE(ji_3d_mesh_vertex_count(mesh) == 0);
    ASSERT_TRUE(ji_3d_mesh_index_count(mesh) == 0);
    ji_3d_mesh_destroy(mesh);
}

static void test_mesh_cube(void) {
    Ji3DMesh* mesh = ji_3d_mesh_cube();
    ASSERT_TRUE(mesh != NULL);
    ASSERT_TRUE(ji_3d_mesh_vertex_count(mesh) == 24);
    ASSERT_TRUE(ji_3d_mesh_index_count(mesh) == 36);
    ji_3d_mesh_destroy(mesh);
}

static void test_mesh_sphere(void) {
    Ji3DMesh* mesh = ji_3d_mesh_sphere(16);
    ASSERT_TRUE(mesh != NULL);
    ASSERT_TRUE(ji_3d_mesh_vertex_count(mesh) > 0);
    ASSERT_TRUE(ji_3d_mesh_index_count(mesh) > 0);
    ji_3d_mesh_destroy(mesh);
}

static void test_mesh_plane(void) {
    Ji3DMesh* mesh = ji_3d_mesh_plane(10, 20);
    ASSERT_TRUE(mesh != NULL);
    ASSERT_TRUE(ji_3d_mesh_vertex_count(mesh) == 4);
    ASSERT_TRUE(ji_3d_mesh_index_count(mesh) == 6);
    ji_3d_mesh_destroy(mesh);
}

static void test_mesh_set_vertices(void) {
    Ji3DMesh* mesh = ji_3d_mesh_new("custom");
    Ji3DVertex verts[3] = {
        {{0,0,0}, {0,1,0}, {0,0}},
        {{1,0,0}, {0,1,0}, {1,0}},
        {{0,0,1}, {0,1,0}, {0,1}},
    };
    ASSERT_TRUE(ji_3d_mesh_set_vertices(mesh, verts, 3) == true);
    ASSERT_TRUE(ji_3d_mesh_vertex_count(mesh) == 3);

    uint32_t indices[3] = {0, 1, 2};
    ASSERT_TRUE(ji_3d_mesh_set_indices(mesh, indices, 3) == true);
    ASSERT_TRUE(ji_3d_mesh_index_count(mesh) == 3);

    ji_3d_mesh_destroy(mesh);
}

/* =========================================================================
 * Material Tests
 * ========================================================================= */

static void test_material_create(void) {
    Ji3DMaterial* mat = ji_3d_material_new();
    ASSERT_TRUE(mat != NULL);
    ASSERT_NEAR(mat->base_color.x, 0.8, 1e-6);
    ASSERT_NEAR(mat->metallic, 0, 1e-6);
    ASSERT_NEAR(mat->roughness, 0.5, 1e-6);
    ji_3d_material_destroy(mat);
}

static void test_material_set_color(void) {
    Ji3DMaterial* mat = ji_3d_material_new();
    ji_3d_material_set_color(mat, 1, 0, 0, 1);
    ASSERT_NEAR(mat->base_color.x, 1, 1e-6);
    ASSERT_NEAR(mat->base_color.y, 0, 1e-6);
    ASSERT_NEAR(mat->base_color.z, 0, 1e-6);

    ji_3d_material_set_metallic_roughness(mat, 0.8, 0.2);
    ASSERT_NEAR(mat->metallic, 0.8, 1e-6);
    ASSERT_NEAR(mat->roughness, 0.2, 1e-6);

    ji_3d_material_destroy(mat);
}

/* =========================================================================
 * Camera Tests
 * ========================================================================= */

static void test_camera_create(void) {
    Ji3DCamera* cam = ji_3d_camera_new();
    ASSERT_TRUE(cam != NULL);
    ASSERT_NEAR(cam->fov, 60, 1e-6);
    ASSERT_NEAR(cam->near_plane, 0.1, 1e-6);
    ASSERT_TRUE(cam->mode == JI_3D_CAMERA_PERSPECTIVE);
    ji_3d_camera_destroy(cam);
}

static void test_camera_set_position(void) {
    Ji3DCamera* cam = ji_3d_camera_new();
    ji_3d_camera_set_position(cam, 10, 20, 30);
    ASSERT_NEAR(cam->position.x, 10, 1e-6);
    ASSERT_NEAR(cam->position.y, 20, 1e-6);
    ASSERT_NEAR(cam->position.z, 30, 1e-6);
    ji_3d_camera_destroy(cam);
}

static void test_camera_view_matrix(void) {
    Ji3DCamera* cam = ji_3d_camera_new();
    JiMat4 view = ji_3d_camera_view_matrix(cam);
    /* View matrix should be valid (not identity for non-origin camera) */
    ASSERT_TRUE(view.m[15] == 1.0f);
    ji_3d_camera_destroy(cam);
}

static void test_camera_projection_matrix(void) {
    Ji3DCamera* cam = ji_3d_camera_new();
    JiMat4 proj = ji_3d_camera_projection_matrix(cam);
    /* Perspective projection should have m[11] = -1 */
    ASSERT_NEAR(proj.m[11], -1.0f, 1e-4);
    ji_3d_camera_destroy(cam);
}

static void test_camera_orbit(void) {
    Ji3DCamera* cam = ji_3d_camera_new();
    ji_3d_camera_orbit(cam, 0, 0, 10);
    /* At yaw=0, pitch=0, distance=10, camera should be at (0,0,10) relative to target */
    ASSERT_NEAR(cam->position.x, 0, 1e-4);
    ASSERT_NEAR(cam->position.z, 10, 1e-4);
    ji_3d_camera_destroy(cam);
}

/* =========================================================================
 * Light Tests
 * ========================================================================= */

static void test_light_create(void) {
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);
    ASSERT_TRUE(light != NULL);
    ASSERT_TRUE(light->type == JI_3D_LIGHT_DIRECTIONAL);
    ASSERT_NEAR(light->color.w, 1.0, 1e-6); /* intensity */
    ji_3d_light_destroy(light);
}

static void test_light_set_color(void) {
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_POINT);
    ji_3d_light_set_color(light, 1, 0.5, 0, 2.0);
    ASSERT_NEAR(light->color.x, 1, 1e-6);
    ASSERT_NEAR(light->color.y, 0.5, 1e-6);
    ASSERT_NEAR(light->color.z, 0, 1e-6);
    ASSERT_NEAR(light->color.w, 2.0, 1e-6);
    ji_3d_light_destroy(light);
}

static void test_light_set_position(void) {
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_POINT);
    ji_3d_light_set_position(light, 5, 10, 15);
    ASSERT_NEAR(light->position.x, 5, 1e-6);
    ASSERT_NEAR(light->position.y, 10, 1e-6);
    ASSERT_NEAR(light->position.z, 15, 1e-6);
    ji_3d_light_destroy(light);
}

/* =========================================================================
 * Scene Tests
 * ========================================================================= */

static void test_scene_create(void) {
    Ji3DScene* scene = ji_3d_scene_new();
    ASSERT_TRUE(scene != NULL);
    ASSERT_TRUE(ji_3d_scene_object_count(scene) == 0);
    ASSERT_TRUE(ji_3d_scene_light_count(scene) == 0);
    ASSERT_TRUE(scene->camera != NULL);
    ASSERT_TRUE(scene->show_grid == true);
    ji_3d_scene_destroy(scene);
}

static void test_scene_add_object(void) {
    Ji3DScene* scene = ji_3d_scene_new();
    Ji3DMesh* mesh = ji_3d_mesh_cube();
    Ji3DMaterial* mat = ji_3d_material_new();
    Ji3DSceneObject* obj = ji_3d_scene_object_new(mesh, mat);

    ASSERT_TRUE(ji_3d_scene_add_object(scene, obj) == true);
    ASSERT_TRUE(ji_3d_scene_object_count(scene) == 1);

    ASSERT_TRUE(ji_3d_scene_remove_object(scene, obj) == true);
    ASSERT_TRUE(ji_3d_scene_object_count(scene) == 0);

    ji_3d_scene_object_destroy(obj);
    ji_3d_material_destroy(mat);
    ji_3d_mesh_destroy(mesh);
    ji_3d_scene_destroy(scene);
}

static void test_scene_add_light(void) {
    Ji3DScene* scene = ji_3d_scene_new();
    Ji3DLight* light = ji_3d_light_new(JI_3D_LIGHT_DIRECTIONAL);

    ASSERT_TRUE(ji_3d_scene_add_light(scene, light) == true);
    ASSERT_TRUE(ji_3d_scene_light_count(scene) == 1);

    ASSERT_TRUE(ji_3d_scene_remove_light(scene, light) == true);
    ASSERT_TRUE(ji_3d_scene_light_count(scene) == 0);

    ji_3d_light_destroy(light);
    ji_3d_scene_destroy(scene);
}

static void test_scene_background(void) {
    Ji3DScene* scene = ji_3d_scene_new();
    ji_3d_scene_set_background(scene, 0.5, 0.3, 0.1, 1.0);
    ASSERT_NEAR(scene->background_color.x, 0.5, 1e-6);
    ASSERT_NEAR(scene->background_color.y, 0.3, 1e-6);
    ASSERT_NEAR(scene->background_color.z, 0.1, 1e-6);
    ji_3d_scene_destroy(scene);
}

/* =========================================================================
 * Gizmo Tests
 * ========================================================================= */

static void test_gizmo_create(void) {
    Ji3DGizmo* gizmo = ji_3d_gizmo_new(JI_3D_GIZMO_TRANSLATE);
    ASSERT_TRUE(gizmo != NULL);
    ASSERT_TRUE(gizmo->type == JI_3D_GIZMO_TRANSLATE);
    ASSERT_TRUE(gizmo->visible == true);
    ASSERT_TRUE(ji_3d_gizmo_is_active(gizmo) == false);
    ji_3d_gizmo_destroy(gizmo);
}

static void test_gizmo_set_type(void) {
    Ji3DGizmo* gizmo = ji_3d_gizmo_new(JI_3D_GIZMO_TRANSLATE);
    ji_3d_gizmo_set_type(gizmo, JI_3D_GIZMO_ROTATE);
    ASSERT_TRUE(gizmo->type == JI_3D_GIZMO_ROTATE);
    ji_3d_gizmo_destroy(gizmo);
}

static void test_gizmo_set_transform(void) {
    Ji3DGizmo* gizmo = ji_3d_gizmo_new(JI_3D_GIZMO_SCALE);
    Ji3DTransform t = ji_3d_transform_identity();
    ji_3d_transform_translate(&t, 5, 5, 5);
    ji_3d_gizmo_set_transform(gizmo, &t);
    ASSERT_NEAR(gizmo->transform.position.x, 5, 1e-6);
    ji_3d_gizmo_destroy(gizmo);
}

/* =========================================================================
 * Viewport Tests
 * ========================================================================= */

static void test_viewport_create(void) {
    Ji3DViewport* vp = ji_3d_viewport_new();
    ASSERT_TRUE(vp != NULL);
    ASSERT_TRUE(vp->orbit_enabled == true);
    ASSERT_TRUE(vp->pan_enabled == true);
    ASSERT_TRUE(vp->zoom_enabled == true);
    ASSERT_NEAR(vp->distance, 10.0, 1e-6);
    ji_3d_viewport_destroy(vp);
}

static void test_viewport_set_scene(void) {
    Ji3DViewport* vp = ji_3d_viewport_new();
    Ji3DScene* scene = ji_3d_scene_new();
    ji_3d_viewport_set_scene(vp, scene);
    ASSERT_TRUE(vp->scene == scene);
    ji_3d_viewport_destroy(vp);
    ji_3d_scene_destroy(scene);
}

static void test_viewport_orbit(void) {
    Ji3DViewport* vp = ji_3d_viewport_new();
    Ji3DScene* scene = ji_3d_scene_new();
    ji_3d_viewport_set_scene(vp, scene);

    ji_3d_viewport_orbit(vp, 10, 5);
    ASSERT_NEAR(vp->yaw, -3.0, 1e-4); /* -10 * 0.3 */
    ASSERT_NEAR(vp->pitch, 31.5, 1e-4); /* 30 + 5 * 0.3 */

    ji_3d_viewport_destroy(vp);
    ji_3d_scene_destroy(scene);
}

static void test_viewport_zoom(void) {
    Ji3DViewport* vp = ji_3d_viewport_new();
    Ji3DScene* scene = ji_3d_scene_new();
    ji_3d_viewport_set_scene(vp, scene);

    float initial = vp->distance;
    ji_3d_viewport_zoom(vp, 1.0);
    ASSERT_TRUE(vp->distance > initial);

    ji_3d_viewport_destroy(vp);
    ji_3d_scene_destroy(scene);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== 3D Engine Tests ===\n");

    printf("-- Math Tests --\n");
    TEST(test_vec3_operations);
    TEST(test_mat4_identity);
    TEST(test_mat4_multiply);
    TEST(test_mat4_perspective);
    TEST(test_quat_identity);
    TEST(test_quat_axis_angle);

    printf("-- Transform Tests --\n");
    TEST(test_transform_identity);
    TEST(test_transform_translate);
    TEST(test_transform_scale);
    TEST(test_transform_to_matrix);

    printf("-- Mesh Tests --\n");
    TEST(test_mesh_create);
    TEST(test_mesh_cube);
    TEST(test_mesh_sphere);
    TEST(test_mesh_plane);
    TEST(test_mesh_set_vertices);

    printf("-- Material Tests --\n");
    TEST(test_material_create);
    TEST(test_material_set_color);

    printf("-- Camera Tests --\n");
    TEST(test_camera_create);
    TEST(test_camera_set_position);
    TEST(test_camera_view_matrix);
    TEST(test_camera_projection_matrix);
    TEST(test_camera_orbit);

    printf("-- Light Tests --\n");
    TEST(test_light_create);
    TEST(test_light_set_color);
    TEST(test_light_set_position);

    printf("-- Scene Tests --\n");
    TEST(test_scene_create);
    TEST(test_scene_add_object);
    TEST(test_scene_add_light);
    TEST(test_scene_background);

    printf("-- Gizmo Tests --\n");
    TEST(test_gizmo_create);
    TEST(test_gizmo_set_type);
    TEST(test_gizmo_set_transform);

    printf("-- Viewport Tests --\n");
    TEST(test_viewport_create);
    TEST(test_viewport_set_scene);
    TEST(test_viewport_orbit);
    TEST(test_viewport_zoom);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_run == tests_passed) ? 0 : 1;
}
