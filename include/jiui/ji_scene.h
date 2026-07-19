/**
 * JiUI - Scene Graph
 * Hardware-accelerated scene graph for UI rendering
 */

#ifndef JIUI_SCENE_H
#define JIUI_SCENE_H

#include "ji_gpu.h"
#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Scene Node Types
 * ========================================================================= */
typedef enum JiSceneNodeType {
    JI_SCENE_NODE_ROOT,
    JI_SCENE_NODE_WIDGET,
    JI_SCENE_NODE_RENDERABLE,
    JI_SCENE_NODE_CLIP,
    JI_SCENE_NODE_TRANSFORM,
    JI_SCENE_NODE_OPACITY,
    JI_SCENE_NODE_EFFECT,
    JI_SCENE_NODE_TEXT,
    JI_SCENE_NODE_IMAGE,
} JiSceneNodeType;

/* =========================================================================
 * 2D Transform
 * ========================================================================= */
typedef struct JiTransform2D {
    float m[3][3]; /* row-major: m[row][col] */
} JiTransform2D;

JI_API JiTransform2D ji_transform2d_identity(void);
JI_API JiTransform2D ji_transform2d_translate(float tx, float ty);
JI_API JiTransform2D ji_transform2d_scale(float sx, float sy);
JI_API JiTransform2D ji_transform2d_rotate(float radians);
JI_API JiTransform2D ji_transform2d_multiply(JiTransform2D a, JiTransform2D b);

/* =========================================================================
 * Rect / Color helpers
 * ========================================================================= */
typedef struct JiRectF {
    float x, y, width, height;
} JiRectF;

typedef struct JiColorF {
    float r, g, b, a;
} JiColorF;

/* JiCornerRadius is already defined in ji_types.h */

/* =========================================================================
 * Scene Node
 * ========================================================================= */
typedef struct JiSceneNode JiSceneNode;

typedef void (*JiSceneNodeDestroyFn)(JiSceneNode* node);

struct JiSceneNode {
    JiSceneNodeType     type;
    JiRectF             bounds;
    JiTransform2D       transform;
    float               opacity;
    bool                visible;
    bool                dirty;          /* needs re-layout */
    bool                render_dirty;   /* needs re-render */
    bool                clip_children;
    JiSceneNode*        parent;
    JiSceneNode**       children;
    uint32_t            child_count;
    uint32_t            child_capacity;
    void*               render_data;    /* GPU resources (backend-specific) */
    void*               user_data;
    JiSceneNodeDestroyFn destroy_fn;
};

/* =========================================================================
 * Render Data Types (per-node GPU resources)
 * ========================================================================= */
typedef struct JiRenderDataRect {
    JiGpuBuffer*    vertex_buffer;
    JiGpuBuffer*    index_buffer;
    uint32_t        index_count;
    JiColorF        color;
    JiCornerRadius  corner_radius;
    float           border_width;
    JiColorF        border_color;
} JiRenderDataRect;

typedef struct JiRenderDataText {
    JiGpuBuffer*    vertex_buffer;
    JiGpuBuffer*    index_buffer;
    JiGpuTexture*   glyph_atlas;
    uint32_t        glyph_count;
    JiColorF        color;
} JiRenderDataText;

typedef struct JiRenderDataImage {
    JiGpuBuffer*    vertex_buffer;
    JiGpuBuffer*    index_buffer;
    JiGpuTexture*   texture;
    JiGpuSampler*   sampler;
    JiGpuDescriptorSet* descriptor_set;
    JiRectF         uv_rect;
} JiRenderDataImage;

typedef struct JiRenderDataEffect {
    JiGpuPipeline*  pipeline;
    JiGpuDescriptorSet* descriptor_set;
    JiGpuTexture*   input_texture;
    JiGpuTexture*   output_texture;
    float           intensity;
} JiRenderDataEffect;

/* =========================================================================
 * Scene Graph
 * ========================================================================= */
typedef struct JiSceneGraph JiSceneGraph;

typedef struct JiSceneStats {
    uint32_t total_nodes;
    uint32_t visible_nodes;
    uint32_t dirty_nodes;
    uint32_t draw_calls;
    uint32_t triangle_count;
    uint32_t texture_count;
    uint64_t vertex_buffer_bytes;
    uint64_t index_buffer_bytes;
    float    frame_time_ms;
    float    gpu_time_ms;
} JiSceneStats;

/** Create a scene graph associated with a GPU device */
JI_API JiSceneGraph* ji_scene_graph_create(JiGpuDevice* device);

/** Destroy a scene graph and all nodes */
JI_API void ji_scene_graph_destroy(JiSceneGraph* graph);

/** Get the root node */
JI_API JiSceneNode* ji_scene_graph_get_root(const JiSceneGraph* graph);

/** Sync the widget tree into the scene graph (called once per frame) */
JI_API void ji_scene_graph_update(JiSceneGraph* graph, void* root_widget);

/** Render the entire scene graph */
JI_API void ji_scene_graph_render(JiSceneGraph* graph, JiGpuSwapchain* swapchain);

/** Collect all dirty nodes for incremental update */
JI_API void ji_scene_graph_collect_dirty(JiSceneGraph* graph);

/** Upload GPU resources for all dirty nodes */
JI_API void ji_scene_graph_bake(JiSceneGraph* graph);

/** Get rendering statistics */
JI_API JiSceneStats ji_scene_graph_get_stats(const JiSceneGraph* graph);

/** Reset per-frame statistics */
JI_API void ji_scene_graph_reset_stats(JiSceneGraph* graph);

/* =========================================================================
 * Scene Node Operations
 * ========================================================================= */

/** Create a scene node */
JI_API JiSceneNode* ji_scene_node_create(JiSceneNodeType type);

/** Destroy a scene node and all children */
JI_API void ji_scene_node_destroy(JiSceneNode* node);

/** Add a child node */
JI_API void ji_scene_node_add_child(JiSceneNode* parent, JiSceneNode* child);

/** Remove a child node (does not destroy it) */
JI_API void ji_scene_node_remove_child(JiSceneNode* parent, JiSceneNode* child);

/** Mark a node and all ancestors as dirty */
JI_API void ji_scene_node_mark_dirty(JiSceneNode* node);

/** Mark a node as render-dirty */
JI_API void ji_scene_node_mark_render_dirty(JiSceneNode* node);

/** Set node bounds */
JI_API void ji_scene_node_set_bounds(JiSceneNode* node, JiRectF bounds);

/** Set node transform */
JI_API void ji_scene_node_set_transform(JiSceneNode* node, JiTransform2D transform);

/** Set node opacity */
JI_API void ji_scene_node_set_opacity(JiSceneNode* node, float opacity);

/** Set node visibility */
JI_API void ji_scene_node_set_visible(JiSceneNode* node, bool visible);

/** Get the world-space bounds of a node (accumulated transforms) */
JI_API JiRectF ji_scene_node_get_world_bounds(const JiSceneNode* node);

/** Find a node by user data */
JI_API JiSceneNode* ji_scene_node_find(JiSceneNode* root, const void* user_data);

/** Traverse the scene tree with a callback */
typedef void (*JiSceneTraversalFn)(JiSceneNode* node, void* user_data);
JI_API void ji_scene_node_traverse(JiSceneNode* root, JiSceneTraversalFn fn, void* user_data);

/* =========================================================================
 * Builtin Pipeline Creation
 * ========================================================================= */

/** Create the rect pipeline (rounded rect, border, fill) */
JI_API JiGpuPipeline* ji_scene_create_rect_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass);

/** Create the text pipeline (SDF glyph rendering) */
JI_API JiGpuPipeline* ji_scene_create_text_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass);

/** Create the image pipeline (textured quad) */
JI_API JiGpuPipeline* ji_scene_create_image_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_SCENE_H */
