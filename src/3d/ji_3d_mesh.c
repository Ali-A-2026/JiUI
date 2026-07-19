/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_3d_mesh.c
 * @brief 3D mesh management — primitives and OBJ loading.
 */

#include "jiui/ji_api.h"
#include "jiui/ji_3d.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * Mesh Lifecycle
 * ========================================================================= */

JI_API Ji3DMesh* ji_3d_mesh_new(const char* name) {
    Ji3DMesh* mesh = ji_calloc(1, sizeof(Ji3DMesh));
    if (!mesh) return NULL;

    mesh->vertices = NULL;
    mesh->vertex_count = 0;
    mesh->indices = NULL;
    mesh->index_count = 0;
    mesh->owns_vertices = false;
    mesh->owns_indices = false;

    if (name) {
        strncpy(mesh->name, name, sizeof(mesh->name) - 1);
    } else {
        strcpy(mesh->name, "unnamed");
    }

    return mesh;
}

JI_API void ji_3d_mesh_destroy(Ji3DMesh* mesh) {
    if (!mesh) return;
    if (mesh->owns_vertices && mesh->vertices) ji_free(mesh->vertices);
    if (mesh->owns_indices && mesh->indices) ji_free(mesh->indices);
    ji_free(mesh);
}

JI_API bool ji_3d_mesh_set_vertices(Ji3DMesh* mesh, const Ji3DVertex* verts, int count) {
    if (!mesh || !verts || count <= 0) return false;

    if (mesh->owns_vertices && mesh->vertices) ji_free(mesh->vertices);
    mesh->vertices = ji_alloc(count * sizeof(Ji3DVertex));
    if (!mesh->vertices) return false;
    memcpy(mesh->vertices, verts, count * sizeof(Ji3DVertex));
    mesh->vertex_count = count;
    mesh->owns_vertices = true;
    return true;
}

JI_API bool ji_3d_mesh_set_indices(Ji3DMesh* mesh, const uint32_t* indices, int count) {
    if (!mesh || !indices || count <= 0) return false;

    if (mesh->owns_indices && mesh->indices) ji_free(mesh->indices);
    mesh->indices = ji_alloc(count * sizeof(uint32_t));
    if (!mesh->indices) return false;
    memcpy(mesh->indices, indices, count * sizeof(uint32_t));
    mesh->index_count = count;
    mesh->owns_indices = true;
    return true;
}

JI_API int ji_3d_mesh_vertex_count(const Ji3DMesh* mesh) {
    return mesh ? mesh->vertex_count : 0;
}

JI_API int ji_3d_mesh_index_count(const Ji3DMesh* mesh) {
    return mesh ? mesh->index_count : 0;
}

/* =========================================================================
 * Primitive Meshes
 * ========================================================================= */

JI_API Ji3DMesh* ji_3d_mesh_cube(void) {
    Ji3DMesh* mesh = ji_3d_mesh_new("cube");
    if (!mesh) return NULL;

    /* 24 vertices (4 per face × 6 faces) for proper normals */
    static const Ji3DVertex verts[24] = {
        /* Front (+Z) */
        {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {0,0}}, {{ 0.5f,-0.5f, 0.5f}, {0,0,1}, {1,0}},
        {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1,1}}, {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {0,1}},
        /* Back (-Z) */
        {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {0,0}}, {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {1,0}},
        {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {1,1}}, {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {0,1}},
        /* Top (+Y) */
        {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {0,0}}, {{ 0.5f, 0.5f, 0.5f}, {0,1,0}, {1,0}},
        {{ 0.5f, 0.5f,-0.5f}, {0,1,0}, {1,1}}, {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {0,1}},
        /* Bottom (-Y) */
        {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {0,0}}, {{ 0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,0}},
        {{ 0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,1}}, {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {0,1}},
        /* Right (+X) */
        {{ 0.5f,-0.5f, 0.5f}, {1,0,0}, {0,0}}, {{ 0.5f,-0.5f,-0.5f}, {1,0,0}, {1,0}},
        {{ 0.5f, 0.5f,-0.5f}, {1,0,0}, {1,1}}, {{ 0.5f, 0.5f, 0.5f}, {1,0,0}, {0,1}},
        /* Left (-X) */
        {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {0,0}}, {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {1,0}},
        {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {1,1}}, {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {0,1}},
    };

    static const uint32_t indices[36] = {
        0,1,2, 0,2,3,        /* Front */
        4,5,6, 4,6,7,        /* Back */
        8,9,10, 8,10,11,     /* Top */
        12,13,14, 12,14,15,  /* Bottom */
        16,17,18, 16,18,19,  /* Right */
        20,21,22, 20,22,23,  /* Left */
    };

    ji_3d_mesh_set_vertices(mesh, verts, 24);
    ji_3d_mesh_set_indices(mesh, indices, 36);

    return mesh;
}

JI_API Ji3DMesh* ji_3d_mesh_sphere(int segments) {
    if (segments < 4) segments = 4;

    Ji3DMesh* mesh = ji_3d_mesh_new("sphere");
    if (!mesh) return NULL;

    int rings = segments;
    int sectors = segments * 2;

    int vert_count = (rings + 1) * (sectors + 1);
    Ji3DVertex* verts = ji_alloc(vert_count * sizeof(Ji3DVertex));
    if (!verts) { ji_3d_mesh_destroy(mesh); return NULL; }

    int vi = 0;
    for (int r = 0; r <= rings; r++) {
        float v = (float)r / rings;
        float phi = v * (float)M_PI;
        for (int s = 0; s <= sectors; s++) {
            float u = (float)s / sectors;
            float theta = u * 2.0f * (float)M_PI;

            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);

            verts[vi].position.x = x;
            verts[vi].position.y = y;
            verts[vi].position.z = z;
            verts[vi].normal.x = x;
            verts[vi].normal.y = y;
            verts[vi].normal.z = z;
            verts[vi].texcoord.x = u;
            verts[vi].texcoord.y = v;
            vi++;
        }
    }

    int idx_count = rings * sectors * 6;
    uint32_t* indices = ji_alloc(idx_count * sizeof(uint32_t));
    if (!indices) { ji_free(verts); ji_3d_mesh_destroy(mesh); return NULL; }

    int ii = 0;
    for (int r = 0; r < rings; r++) {
        for (int s = 0; s < sectors; s++) {
            int a = r * (sectors + 1) + s;
            int b = a + sectors + 1;
            indices[ii++] = a;
            indices[ii++] = b;
            indices[ii++] = a + 1;
            indices[ii++] = b;
            indices[ii++] = b + 1;
            indices[ii++] = a + 1;
        }
    }

    ji_3d_mesh_set_vertices(mesh, verts, vert_count);
    ji_3d_mesh_set_indices(mesh, indices, idx_count);

    ji_free(verts);
    ji_free(indices);

    return mesh;
}

JI_API Ji3DMesh* ji_3d_mesh_plane(float width, float depth) {
    Ji3DMesh* mesh = ji_3d_mesh_new("plane");
    if (!mesh) return NULL;

    float hw = width * 0.5f;
    float hd = depth * 0.5f;

    Ji3DVertex verts[4] = {
        {{-hw, 0, -hd}, {0,1,0}, {0,0}},
        {{ hw, 0, -hd}, {0,1,0}, {1,0}},
        {{ hw, 0,  hd}, {0,1,0}, {1,1}},
        {{-hw, 0,  hd}, {0,1,0}, {0,1}},
    };

    uint32_t indices[6] = {0, 1, 2, 0, 2, 3};

    ji_3d_mesh_set_vertices(mesh, verts, 4);
    ji_3d_mesh_set_indices(mesh, indices, 6);

    return mesh;
}

/* =========================================================================
 * OBJ Loading (basic)
 * ========================================================================= */

JI_API Ji3DMesh* ji_3d_mesh_from_obj(const char* path) {
    if (!path) return NULL;

    FILE* f = fopen(path, "r");
    if (!f) return NULL;

    /* Count positions, normals, texcoords, faces */
    int pos_count = 0, norm_count = 0, tex_count = 0, face_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'v' && line[1] == ' ') pos_count++;
        else if (line[0] == 'v' && line[1] == 'n') norm_count++;
        else if (line[0] == 'v' && line[1] == 't') tex_count++;
        else if (line[0] == 'f' && line[1] == ' ') face_count++;
    }
    rewind(f);

    if (pos_count == 0 || face_count == 0) {
        fclose(f);
        return NULL;
    }

    /* Allocate temp arrays */
    JiVec3* positions = ji_alloc(pos_count * sizeof(JiVec3));
    JiVec3* normals = ji_alloc(norm_count * sizeof(JiVec3));
    JiVec2* texcoords = ji_alloc(tex_count * sizeof(JiVec2));

    int vert_count = face_count * 3;
    Ji3DVertex* verts = ji_alloc(vert_count * sizeof(Ji3DVertex));
    uint32_t* indices = ji_alloc(vert_count * sizeof(uint32_t));

    if (!positions || !verts || !indices) {
        ji_free(positions); ji_free(normals); ji_free(texcoords);
        ji_free(verts); ji_free(indices);
        fclose(f);
        return NULL;
    }

    int pi = 0, ni = 0, ti = 0, vi = 0, ii = 0;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == 'v' && line[1] == ' ') {
            sscanf(line, "v %f %f %f", &positions[pi].x, &positions[pi].y, &positions[pi].z);
            pi++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            sscanf(line, "vn %f %f %f", &normals[ni].x, &normals[ni].y, &normals[ni].z);
            ni++;
        } else if (line[0] == 'v' && line[1] == 't') {
            sscanf(line, "vt %f %f", &texcoords[ti].x, &texcoords[ti].y);
            ti++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            int p0, t0, n0, p1, t1, n1, p2, t2, n2;
            /* Try v/t/n format */
            int matched = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                  &p0, &t0, &n0, &p1, &t1, &n1, &p2, &t2, &n2);
            if (matched == 9) {
                verts[vi].position = positions[p0 - 1];
                verts[vi].normal = (n0 <= norm_count) ? normals[n0 - 1] : (JiVec3){0,1,0};
                verts[vi].texcoord = (t0 <= tex_count) ? texcoords[t0 - 1] : (JiVec2){0,0};
                indices[ii++] = vi++;
                verts[vi].position = positions[p1 - 1];
                verts[vi].normal = (n1 <= norm_count) ? normals[n1 - 1] : (JiVec3){0,1,0};
                verts[vi].texcoord = (t1 <= tex_count) ? texcoords[t1 - 1] : (JiVec2){0,0};
                indices[ii++] = vi++;
                verts[vi].position = positions[p2 - 1];
                verts[vi].normal = (n2 <= norm_count) ? normals[n2 - 1] : (JiVec3){0,1,0};
                verts[vi].texcoord = (t2 <= tex_count) ? texcoords[t2 - 1] : (JiVec2){0,0};
                indices[ii++] = vi++;
            } else {
                /* Try v//n format */
                matched = sscanf(line, "f %d//%d %d//%d %d//%d", &p0, &n0, &p1, &n1, &p2, &n2);
                if (matched == 6) {
                    verts[vi].position = positions[p0 - 1];
                    verts[vi].normal = (n0 <= norm_count) ? normals[n0 - 1] : (JiVec3){0,1,0};
                    verts[vi].texcoord = (JiVec2){0,0};
                    indices[ii++] = vi++;
                    verts[vi].position = positions[p1 - 1];
                    verts[vi].normal = (n1 <= norm_count) ? normals[n1 - 1] : (JiVec3){0,1,0};
                    verts[vi].texcoord = (JiVec2){0,0};
                    indices[ii++] = vi++;
                    verts[vi].position = positions[p2 - 1];
                    verts[vi].normal = (n2 <= norm_count) ? normals[n2 - 1] : (JiVec3){0,1,0};
                    verts[vi].texcoord = (JiVec2){0,0};
                    indices[ii++] = vi++;
                } else {
                    /* Try v format */
                    matched = sscanf(line, "f %d %d %d", &p0, &p1, &p2);
                    if (matched == 3) {
                        verts[vi].position = positions[p0 - 1];
                        verts[vi].normal = (JiVec3){0,1,0};
                        verts[vi].texcoord = (JiVec2){0,0};
                        indices[ii++] = vi++;
                        verts[vi].position = positions[p1 - 1];
                        verts[vi].normal = (JiVec3){0,1,0};
                        verts[vi].texcoord = (JiVec2){0,0};
                        indices[ii++] = vi++;
                        verts[vi].position = positions[p2 - 1];
                        verts[vi].normal = (JiVec3){0,1,0};
                        verts[vi].texcoord = (JiVec2){0,0};
                        indices[ii++] = vi++;
                    }
                }
            }
        }
    }

    fclose(f);

    Ji3DMesh* mesh = ji_3d_mesh_new("obj_mesh");
    if (mesh) {
        ji_3d_mesh_set_vertices(mesh, verts, vi);
        ji_3d_mesh_set_indices(mesh, indices, ii);
    }

    ji_free(positions);
    ji_free(normals);
    ji_free(texcoords);
    ji_free(verts);
    ji_free(indices);

    return mesh;
}

/* =========================================================================
 * Material
 * ========================================================================= */

JI_API Ji3DMaterial* ji_3d_material_new(void) {
    Ji3DMaterial* mat = ji_calloc(1, sizeof(Ji3DMaterial));
    if (!mat) return NULL;
    mat->base_color.x = 0.8f;
    mat->base_color.y = 0.8f;
    mat->base_color.z = 0.8f;
    mat->base_color.w = 1.0f;
    mat->metallic = 0.0f;
    mat->roughness = 0.5f;
    mat->emissive = 0.0f;
    mat->emissive_color.x = 0;
    mat->emissive_color.y = 0;
    mat->emissive_color.z = 0;
    mat->emissive_color.w = 1;
    mat->wireframe = false;
    mat->double_sided = false;
    return mat;
}

JI_API void ji_3d_material_destroy(Ji3DMaterial* mat) {
    if (mat) ji_free(mat);
}

JI_API void ji_3d_material_set_color(Ji3DMaterial* mat, float r, float g, float b, float a) {
    if (!mat) return;
    mat->base_color.x = r;
    mat->base_color.y = g;
    mat->base_color.z = b;
    mat->base_color.w = a;
}

JI_API void ji_3d_material_set_metallic_roughness(Ji3DMaterial* mat,
                                                    float metallic, float roughness) {
    if (!mat) return;
    mat->metallic = metallic;
    mat->roughness = roughness;
}
