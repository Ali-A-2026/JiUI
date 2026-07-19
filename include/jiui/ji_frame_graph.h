/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_frame_graph.h
 * @brief Frame Graph Renderer — automatic GPU resource scheduling,
 *        render pass optimization, resource aliasing, and barrier insertion.
 */

#ifndef JIUI_FRAME_GRAPH_H
#define JIUI_FRAME_GRAPH_H

#include "ji_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Frame Graph Resource
 * ========================================================================= */

typedef enum JiFGResourceType {
    JI_FG_RESOURCE_TEXTURE,
    JI_FG_RESOURCE_BUFFER
} JiFGResourceType;

typedef enum JiFGResourceLifetime {
    JI_FG_LIFETIME_TRANSIENT,   /* Only valid within the frame */
    JI_FG_LIFETIME_PERSISTENT  /* Persists across frames */
} JiFGResourceLifetime;

typedef struct JiFGResource JiFGResource;

struct JiFGResource {
    JiFGResourceType     type;
    JiFGResourceLifetime lifetime;
    const char*          name;
    uint32_t             width;      /* For textures */
    uint32_t             height;     /* For textures */
    JiGpuFormat          format;     /* For textures */
    uint64_t             size;       /* For buffers */
    JiGpuBufferUsage     usage;      /* For buffers */
    uint32_t             ref_count;
    bool                 imported;   /* True if wrapping existing GPU resource */
    void*                handle;     /* Actual GPU resource after allocation */
    JiFGResource*        aliased_to; /* Resource aliasing */
};

/* =========================================================================
 * Frame Graph Pass
 * ========================================================================= */

typedef enum JiFGPassType {
    JI_FG_PASS_RENDER,     /* Graphics render pass */
    JI_FG_PASS_COMPUTE,    /* Compute dispatch pass */
    JI_FG_PASS_TRANSFER    /* Copy/transfer pass */
} JiFGPassType;

typedef struct JiFGPass JiFGPass;

typedef void (*JiFGPassExecuteFunc)(JiFGPass* pass, void* user_data);

struct JiFGPass {
    JiFGPassType         type;
    const char*          name;
    JiFGResource**       reads;
    uint32_t             read_count;
    JiFGResource**       writes;
    uint32_t             write_count;
    JiFGPassExecuteFunc  execute;
    void*                user_data;
    bool                 culled;     /* True if pass was culled (no consumers) */
    uint32_t             order;      /* Execution order after compile */
    JiFGPass*            next;
};

/* =========================================================================
 * Frame Graph
 * ========================================================================= */

typedef struct JiFrameGraph JiFrameGraph;

JiFrameGraph*  ji_frame_graph_create(JiGpuDevice* device);
void           ji_frame_graph_destroy(JiFrameGraph* fg);

/* Resource management */
JiFGResource*  ji_frame_graph_add_texture(JiFrameGraph* fg, const char* name,
                                            uint32_t width, uint32_t height,
                                            JiGpuFormat format);
JiFGResource*  ji_frame_graph_add_buffer(JiFrameGraph* fg, const char* name,
                                           uint64_t size, JiGpuBufferUsage usage);
JiFGResource*  ji_frame_graph_import_texture(JiFrameGraph* fg, const char* name,
                                               JiGpuTexture* texture);
JiFGResource*  ji_frame_graph_import_buffer(JiFrameGraph* fg, const char* name,
                                              JiGpuBuffer* buffer);

/* Pass management */
JiFGPass*     ji_frame_graph_add_pass(JiFrameGraph* fg, const char* name,
                                        JiFGPassType type,
                                        JiFGResource** reads, uint32_t read_count,
                                        JiFGResource** writes, uint32_t write_count,
                                        JiFGPassExecuteFunc execute, void* user_data);

/* Compilation — culls unused passes, inserts barriers, determines order */
bool          ji_frame_graph_compile(JiFrameGraph* fg);

/* Execution — allocates resources, runs passes in order */
bool          ji_frame_graph_execute(JiFrameGraph* fg);

/* Reset for next frame — releases transient resources */
void          ji_frame_graph_reset(JiFrameGraph* fg);

/* Stats */
typedef struct JiFrameGraphStats {
    uint32_t total_passes;
    uint32_t culled_passes;
    uint32_t total_resources;
    uint32_t aliased_resources;
    uint32_t transient_textures;
    uint32_t transient_buffers;
} JiFrameGraphStats;

JiFrameGraphStats ji_frame_graph_get_stats(const JiFrameGraph* fg);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_FRAME_GRAPH_H */
