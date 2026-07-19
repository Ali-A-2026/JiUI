/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * Scene Graph Implementation
 */

#include "jiui/ji_scene.h"
#include "jiui/ji_memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* =========================================================================
 * Transform2D
 * ========================================================================= */
JiTransform2D ji_transform2d_identity(void) {
    JiTransform2D t = {{{1,0,0},{0,1,0},{0,0,1}}};
    return t;
}

JiTransform2D ji_transform2d_translate(float tx, float ty) {
    JiTransform2D t = ji_transform2d_identity();
    t.m[0][2] = tx; t.m[1][2] = ty;
    return t;
}

JiTransform2D ji_transform2d_scale(float sx, float sy) {
    JiTransform2D t = ji_transform2d_identity();
    t.m[0][0] = sx; t.m[1][1] = sy;
    return t;
}

JiTransform2D ji_transform2d_rotate(float radians) {
    JiTransform2D t = ji_transform2d_identity();
    float c = cosf(radians), s = sinf(radians);
    t.m[0][0] = c;  t.m[0][1] = -s;
    t.m[1][0] = s;  t.m[1][1] = c;
    return t;
}

JiTransform2D ji_transform2d_multiply(JiTransform2D a, JiTransform2D b) {
    JiTransform2D r = {{{0,0,0},{0,0,0},{0,0,0}}};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

/* =========================================================================
 * Scene Node
 * ========================================================================= */
JiSceneNode* ji_scene_node_create(JiSceneNodeType type) {
    JiSceneNode* node = ji_calloc(1, sizeof(JiSceneNode));
    if (!node) return NULL;
    node->type = type;
    node->transform = ji_transform2d_identity();
    node->opacity = 1.0f;
    node->visible = true;
    node->child_capacity = 8;
    node->children = ji_calloc(node->child_capacity, sizeof(JiSceneNode*));
    return node;
}

void ji_scene_node_destroy(JiSceneNode* node) {
    if (!node) return;
    if (node->children) {
        for (uint32_t i = 0; i < node->child_count; i++)
            ji_scene_node_destroy(node->children[i]);
        ji_free(node->children);
    }
    if (node->destroy_fn) node->destroy_fn(node);
    if (node->render_data) ji_free(node->render_data);
    ji_free(node);
}

void ji_scene_node_add_child(JiSceneNode* parent, JiSceneNode* child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = ji_realloc(parent->children, parent->child_capacity * sizeof(JiSceneNode*));
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    ji_scene_node_mark_dirty(parent);
}

void ji_scene_node_remove_child(JiSceneNode* parent, JiSceneNode* child) {
    if (!parent || !child) return;
    for (uint32_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[parent->child_count - 1];
            parent->child_count--;
            child->parent = NULL;
            ji_scene_node_mark_dirty(parent);
            return;
        }
    }
}

void ji_scene_node_mark_dirty(JiSceneNode* node) {
    if (!node) return;
    node->dirty = true;
    if (node->parent) ji_scene_node_mark_dirty(node->parent);
}

void ji_scene_node_mark_render_dirty(JiSceneNode* node) {
    if (!node) return;
    node->render_dirty = true;
    if (node->parent) ji_scene_node_mark_render_dirty(node->parent);
}

void ji_scene_node_set_bounds(JiSceneNode* node, JiRectF bounds) {
    if (!node) return;
    node->bounds = bounds;
    ji_scene_node_mark_dirty(node);
}

void ji_scene_node_set_transform(JiSceneNode* node, JiTransform2D transform) {
    if (!node) return;
    node->transform = transform;
    ji_scene_node_mark_render_dirty(node);
}

void ji_scene_node_set_opacity(JiSceneNode* node, float opacity) {
    if (!node) return;
    node->opacity = opacity < 0 ? 0 : (opacity > 1 ? 1 : opacity);
    ji_scene_node_mark_render_dirty(node);
}

void ji_scene_node_set_visible(JiSceneNode* node, bool visible) {
    if (!node) return;
    node->visible = visible;
    ji_scene_node_mark_dirty(node);
}

JiRectF ji_scene_node_get_world_bounds(const JiSceneNode* node) {
    if (!node) return (JiRectF){0, 0, 0, 0};
    JiRectF r = node->bounds;
    /* Walk up to accumulate transforms (simplified - just offset) */
    const JiSceneNode* n = node->parent;
    while (n) {
        r.x += n->bounds.x;
        r.y += n->bounds.y;
        n = n->parent;
    }
    return r;
}

JiSceneNode* ji_scene_node_find(JiSceneNode* root, const void* user_data) {
    if (!root) return NULL;
    if (root->user_data == user_data) return root;
    for (uint32_t i = 0; i < root->child_count; i++) {
        JiSceneNode* found = ji_scene_node_find(root->children[i], user_data);
        if (found) return found;
    }
    return NULL;
}

void ji_scene_node_traverse(JiSceneNode* root, JiSceneTraversalFn fn, void* user_data) {
    if (!root || !fn) return;
    fn(root, user_data);
    for (uint32_t i = 0; i < root->child_count; i++)
        ji_scene_node_traverse(root->children[i], fn, user_data);
}

/* =========================================================================
 * Scene Graph
 * ========================================================================= */
struct JiSceneGraph {
    JiGpuDevice*  device;
    JiSceneNode*  root;
    JiSceneStats  stats;
};

JiSceneGraph* ji_scene_graph_create(JiGpuDevice* device) {
    JiSceneGraph* graph = ji_calloc(1, sizeof(JiSceneGraph));
    if (!graph) return NULL;
    graph->device = device;
    graph->root = ji_scene_node_create(JI_SCENE_NODE_ROOT);
    return graph;
}

void ji_scene_graph_destroy(JiSceneGraph* graph) {
    if (!graph) return;
    ji_scene_node_destroy(graph->root);
    ji_free(graph);
}

JiSceneNode* ji_scene_graph_get_root(const JiSceneGraph* graph) {
    return graph ? graph->root : NULL;
}

static void ji_scene_graph_count_nodes(JiSceneNode* node, void* data) {
    JiSceneStats* stats = (JiSceneStats*)data;
    stats->total_nodes++;
    if (node->visible) stats->visible_nodes++;
    if (node->dirty || node->render_dirty) stats->dirty_nodes++;
}

void ji_scene_graph_update(JiSceneGraph* graph, void* root_widget) {
    if (!graph) return;
    /* TODO: sync widget tree → scene graph */
    (void)root_widget;
}

void ji_scene_graph_render(JiSceneGraph* graph, JiGpuSwapchain* swapchain) {
    if (!graph || !swapchain) return;
    /* TODO: full GPU render pass */
    graph->stats.draw_calls = 0;
    graph->stats.triangle_count = 0;
}

void ji_scene_graph_collect_dirty(JiSceneGraph* graph) {
    if (!graph) return;
    graph->stats.total_nodes = 0;
    graph->stats.visible_nodes = 0;
    graph->stats.dirty_nodes = 0;
    ji_scene_node_traverse(graph->root, ji_scene_graph_count_nodes, &graph->stats);
}

void ji_scene_graph_bake(JiSceneGraph* graph) {
    if (!graph) return;
    /* TODO: upload GPU resources for dirty nodes */
}

JiSceneStats ji_scene_graph_get_stats(const JiSceneGraph* graph) {
    return graph ? graph->stats : (JiSceneStats){0};
}

void ji_scene_graph_reset_stats(JiSceneGraph* graph) {
    if (!graph) return;
    memset(&graph->stats, 0, sizeof(JiSceneStats));
}

/* =========================================================================
 * Builtin Pipeline Creation (stubs - real shaders in Phase 2)
 * ========================================================================= */
JiGpuPipeline* ji_scene_create_rect_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass) {
    JiGpuPipelineCreateInfo info = {0};
    info.render_pass = pass;
    info.topology = JI_GPU_PRIM_TRIANGLE_STRIP;
    info.blend_state.attachment_count = 1;
    info.blend_state.attachments[0].blend_enable = true;
    info.blend_state.attachments[0].src_color_blend_factor = JI_GPU_BLEND_SRC_ALPHA;
    info.blend_state.attachments[0].dst_color_blend_factor = JI_GPU_BLEND_ONE_MINUS_SRC_ALPHA;
    return ji_gpu_pipeline_create(device, &info);
}

JiGpuPipeline* ji_scene_create_text_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass) {
    JiGpuPipelineCreateInfo info = {0};
    info.render_pass = pass;
    info.topology = JI_GPU_PRIM_TRIANGLE_STRIP;
    info.blend_state.attachment_count = 1;
    info.blend_state.attachments[0].blend_enable = true;
    info.blend_state.attachments[0].src_color_blend_factor = JI_GPU_BLEND_SRC_ALPHA;
    info.blend_state.attachments[0].dst_color_blend_factor = JI_GPU_BLEND_ONE_MINUS_SRC_ALPHA;
    return ji_gpu_pipeline_create(device, &info);
}

JiGpuPipeline* ji_scene_create_image_pipeline(JiGpuDevice* device, JiGpuRenderPass* pass) {
    JiGpuPipelineCreateInfo info = {0};
    info.render_pass = pass;
    info.topology = JI_GPU_PRIM_TRIANGLE_STRIP;
    info.blend_state.attachment_count = 1;
    info.blend_state.attachments[0].blend_enable = true;
    info.blend_state.attachments[0].src_color_blend_factor = JI_GPU_BLEND_SRC_ALPHA;
    info.blend_state.attachments[0].dst_color_blend_factor = JI_GPU_BLEND_ONE_MINUS_SRC_ALPHA;
    return ji_gpu_pipeline_create(device, &info);
}
