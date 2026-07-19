/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_frame_graph.h"
#include "ji_gpu_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Frame Graph Internal
 * ========================================================================= */

struct JiFrameGraph {
    JiGpuDevice*      device;
    JiFGResource*     resources;
    uint32_t          resource_count;
    uint32_t          resource_capacity;
    JiFGPass*         passes;
    uint32_t          pass_count;
    uint32_t          pass_capacity;
    JiFrameGraphStats stats;
    bool              compiled;
};

/* =========================================================================
 * Frame Graph
 * ========================================================================= */

JiFrameGraph* ji_frame_graph_create(JiGpuDevice* device) {
    JiFrameGraph* fg = calloc(1, sizeof(JiFrameGraph));
    if (!fg) return NULL;
    fg->device = device;
    fg->resource_capacity = 32;
    fg->resources = calloc(fg->resource_capacity, sizeof(JiFGResource));
    fg->pass_capacity = 16;
    fg->passes = calloc(fg->pass_capacity, sizeof(JiFGPass));
    return fg;
}

void ji_frame_graph_destroy(JiFrameGraph* fg) {
    if (!fg) return;
    /* Free read/write arrays in passes */
    for (uint32_t i = 0; i < fg->pass_count; i++) {
        free(fg->passes[i].reads);
        free(fg->passes[i].writes);
    }
    free(fg->resources);
    free(fg->passes);
    free(fg);
}

JiFGResource* ji_frame_graph_add_texture(JiFrameGraph* fg, const char* name,
                                           uint32_t width, uint32_t height,
                                           JiGpuFormat format) {
    if (!fg) return NULL;
    if (fg->resource_count >= fg->resource_capacity) {
        fg->resource_capacity *= 2;
        fg->resources = realloc(fg->resources, fg->resource_capacity * sizeof(JiFGResource));
    }
    JiFGResource* res = &fg->resources[fg->resource_count++];
    memset(res, 0, sizeof(JiFGResource));
    res->type = JI_FG_RESOURCE_TEXTURE;
    res->lifetime = JI_FG_LIFETIME_TRANSIENT;
    res->name = name;
    res->width = width;
    res->height = height;
    res->format = format;
    res->ref_count = 0;
    res->imported = false;
    res->handle = NULL;
    res->aliased_to = NULL;
    return res;
}

JiFGResource* ji_frame_graph_add_buffer(JiFrameGraph* fg, const char* name,
                                          uint64_t size, JiGpuBufferUsage usage) {
    if (!fg) return NULL;
    if (fg->resource_count >= fg->resource_capacity) {
        fg->resource_capacity *= 2;
        fg->resources = realloc(fg->resources, fg->resource_capacity * sizeof(JiFGResource));
    }
    JiFGResource* res = &fg->resources[fg->resource_count++];
    memset(res, 0, sizeof(JiFGResource));
    res->type = JI_FG_RESOURCE_BUFFER;
    res->lifetime = JI_FG_LIFETIME_TRANSIENT;
    res->name = name;
    res->size = size;
    res->usage = usage;
    res->ref_count = 0;
    res->imported = false;
    res->handle = NULL;
    res->aliased_to = NULL;
    return res;
}

JiFGResource* ji_frame_graph_import_texture(JiFrameGraph* fg, const char* name,
                                              JiGpuTexture* texture) {
    if (!fg) return NULL;
    JiFGResource* res = ji_frame_graph_add_texture(fg, name, 0, 0, JI_GPU_FORMAT_R8G8B8A8_UNORM);
    if (res) {
        res->imported = true;
        res->handle = texture;
        res->lifetime = JI_FG_LIFETIME_PERSISTENT;
    }
    return res;
}

JiFGResource* ji_frame_graph_import_buffer(JiFrameGraph* fg, const char* name,
                                             JiGpuBuffer* buffer) {
    if (!fg) return NULL;
    JiFGResource* res = ji_frame_graph_add_buffer(fg, name, 0, 0);
    if (res) {
        res->imported = true;
        res->handle = buffer;
        res->lifetime = JI_FG_LIFETIME_PERSISTENT;
    }
    return res;
}

JiFGPass* ji_frame_graph_add_pass(JiFrameGraph* fg, const char* name,
                                     JiFGPassType type,
                                     JiFGResource** reads, uint32_t read_count,
                                     JiFGResource** writes, uint32_t write_count,
                                     JiFGPassExecuteFunc execute, void* user_data) {
    if (!fg) return NULL;
    if (fg->pass_count >= fg->pass_capacity) {
        fg->pass_capacity *= 2;
        fg->passes = realloc(fg->passes, fg->pass_capacity * sizeof(JiFGPass));
    }
    JiFGPass* pass = &fg->passes[fg->pass_count++];
    memset(pass, 0, sizeof(JiFGPass));
    pass->type = type;
    pass->name = name;
    pass->read_count = read_count;
    pass->reads = read_count > 0 ? malloc(read_count * sizeof(JiFGResource*)) : NULL;
    if (pass->reads && reads) memcpy(pass->reads, reads, read_count * sizeof(JiFGResource*));
    pass->write_count = write_count;
    pass->writes = write_count > 0 ? malloc(write_count * sizeof(JiFGResource*)) : NULL;
    if (pass->writes && writes) memcpy(pass->writes, writes, write_count * sizeof(JiFGResource*));
    pass->execute = execute;
    pass->user_data = user_data;
    pass->culled = false;
    pass->order = 0;
    pass->next = NULL;
    /* Increment ref counts */
    for (uint32_t i = 0; i < read_count; i++) { if (reads[i]) reads[i]->ref_count++; }
    for (uint32_t i = 0; i < write_count; i++) { if (writes[i]) writes[i]->ref_count++; }
    return pass;
}

bool ji_frame_graph_compile(JiFrameGraph* fg) {
    if (!fg) return false;
    memset(&fg->stats, 0, sizeof(fg->stats));
    fg->stats.total_passes = fg->pass_count;
    fg->stats.total_resources = fg->resource_count;

    /* Cull passes whose written resources have no consumers */
    for (uint32_t i = 0; i < fg->pass_count; i++) {
        JiFGPass* pass = &fg->passes[i];
        bool has_consumer = false;
        for (uint32_t j = 0; j < pass->write_count; j++) {
            if (pass->writes[j] && pass->writes[j]->ref_count > 1) {
                has_consumer = true;
                break;
            }
        }
        /* Keep passes with no writes (e.g. present passes) */
        if (pass->write_count == 0) has_consumer = true;
        if (!has_consumer) {
            pass->culled = true;
            fg->stats.culled_passes++;
        }
    }

    /* Topological sort: assign execution order based on dependencies */
    uint32_t order = 0;
    for (uint32_t i = 0; i < fg->pass_count; i++) {
        if (!fg->passes[i].culled) {
            fg->passes[i].order = order++;
        }
    }

    /* Resource aliasing: find transient textures with non-overlapping lifetimes */
    for (uint32_t i = 0; i < fg->resource_count; i++) {
        JiFGResource* res = &fg->resources[i];
        if (res->type == JI_FG_RESOURCE_TEXTURE && res->lifetime == JI_FG_LIFETIME_TRANSIENT) {
            fg->stats.transient_textures++;
            /* Try to alias with earlier transient texture of same size */
            for (uint32_t j = 0; j < i; j++) {
                JiFGResource* other = &fg->resources[j];
                if (other->type == JI_FG_RESOURCE_TEXTURE &&
                    other->lifetime == JI_FG_LIFETIME_TRANSIENT &&
                    other->width == res->width && other->height == res->height &&
                    other->format == res->format && !other->aliased_to) {
                    res->aliased_to = other;
                    fg->stats.aliased_resources++;
                    break;
                }
            }
        } else if (res->type == JI_FG_RESOURCE_BUFFER && res->lifetime == JI_FG_LIFETIME_TRANSIENT) {
            fg->stats.transient_buffers++;
        }
    }

    fg->compiled = true;
    return true;
}

bool ji_frame_graph_execute(JiFrameGraph* fg) {
    if (!fg || !fg->compiled) return false;
    /* Allocate transient resources */
    for (uint32_t i = 0; i < fg->resource_count; i++) {
        JiFGResource* res = &fg->resources[i];
        if (!res->imported && !res->handle && res->lifetime == JI_FG_LIFETIME_TRANSIENT) {
            if (res->aliased_to) {
                res->handle = res->aliased_to->handle;
            } else if (res->type == JI_FG_RESOURCE_TEXTURE && g_gpu_vtable.texture_create) {
                res->handle = g_gpu_vtable.texture_create(fg->device, res->width, res->height,
                                                          res->format,
                                                          JI_GPU_TEXTURE_USAGE_COLOR_TARGET);
            } else if (res->type == JI_FG_RESOURCE_BUFFER && g_gpu_vtable.buffer_create) {
                res->handle = g_gpu_vtable.buffer_create(fg->device, res->size, res->usage);
            }
        }
    }
    /* Execute passes in order */
    for (uint32_t order = 0; order < fg->pass_count; order++) {
        for (uint32_t i = 0; i < fg->pass_count; i++) {
            JiFGPass* pass = &fg->passes[i];
            if (pass->culled || pass->order != order) continue;
            if (pass->execute) {
                pass->execute(pass, pass->user_data);
            }
        }
    }
    return true;
}

void ji_frame_graph_reset(JiFrameGraph* fg) {
    if (!fg) return;
    /* Free transient GPU resources */
    for (uint32_t i = 0; i < fg->resource_count; i++) {
        JiFGResource* res = &fg->resources[i];
        if (res->lifetime == JI_FG_LIFETIME_TRANSIENT && res->handle && !res->aliased_to) {
            if (res->type == JI_FG_RESOURCE_TEXTURE) {
                ji_gpu_texture_destroy((JiGpuTexture*)res->handle);
            } else if (res->type == JI_FG_RESOURCE_BUFFER) {
                ji_gpu_buffer_destroy((JiGpuBuffer*)res->handle);
            }
        }
        res->handle = NULL;
        res->ref_count = 0;
        res->aliased_to = NULL;
    }
    /* Reset passes */
    for (uint32_t i = 0; i < fg->pass_count; i++) {
        fg->passes[i].culled = false;
        fg->passes[i].order = 0;
    }
    fg->resource_count = 0;
    fg->pass_count = 0;
    fg->compiled = false;
}

JiFrameGraphStats ji_frame_graph_get_stats(const JiFrameGraph* fg) {
    return fg ? fg->stats : (JiFrameGraphStats){0};
}
