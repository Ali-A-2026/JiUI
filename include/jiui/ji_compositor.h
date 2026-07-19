/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file ji_compositor.h
 * @brief GPU Compositor — render target pool, batch rendering, Z-order sorting,
 *        clip region stacking, opacity compositing, and effect chain execution.
 */

#ifndef JIUI_COMPOSITOR_H
#define JIUI_COMPOSITOR_H

#include "ji_gpu.h"
#include "ji_scene.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Render Target Pool
 * ========================================================================= */

typedef struct JiRenderTarget JiRenderTarget;

struct JiRenderTarget {
    JiGpuTexture*   color;
    JiGpuTexture*   depth;
    uint32_t        width;
    uint32_t        height;
    JiGpuFormat     format;
    bool            in_use;
    JiRenderTarget* next;       /* pool linked list */
};

typedef struct JiRenderTargetPool JiRenderTargetPool;

JiRenderTargetPool* ji_render_target_pool_create(JiGpuDevice* device, uint32_t max_targets);
void                ji_render_target_pool_destroy(JiRenderTargetPool* pool);
JiRenderTarget*     ji_render_target_pool_acquire(JiRenderTargetPool* pool,
                                                   uint32_t width, uint32_t height,
                                                   JiGpuFormat format);
void                ji_render_target_pool_release(JiRenderTargetPool* pool, JiRenderTarget* rt);

/* =========================================================================
 * Draw Batch
 * ========================================================================= */

typedef enum JiBatchType {
    JI_BATCH_RECT,
    JI_BATCH_TEXT,
    JI_BATCH_IMAGE,
    JI_BATCH_EFFECT,
    JI_BATCH_CUSTOM
} JiBatchType;

typedef struct JiDrawBatch JiDrawBatch;

struct JiDrawBatch {
    JiBatchType     type;
    JiGpuPipeline*  pipeline;
    JiGpuBuffer*    vertex_buffer;
    JiGpuBuffer*    index_buffer;
    uint32_t        vertex_count;
    uint32_t        index_count;
    uint32_t        instance_count;
    float           z_order;
    JiRectF         clip_rect;
    float           opacity;
    void*           user_data;
    JiDrawBatch*    next;
};

/* =========================================================================
 * Clip Stack
 * ========================================================================= */

typedef struct JiClipStack JiClipStack;

JiClipStack* ji_clip_stack_create(uint32_t max_depth);
void         ji_clip_stack_destroy(JiClipStack* stack);
void         ji_clip_stack_push(JiClipStack* stack, JiRectF clip);
JiRectF      ji_clip_stack_pop(JiClipStack* stack);
JiRectF      ji_clip_stack_peek(const JiClipStack* stack);
JiRectF      ji_clip_stack_intersect(const JiClipStack* stack, JiRectF rect);
uint32_t     ji_clip_stack_depth(const JiClipStack* stack);

/* =========================================================================
 * Compositor
 * ========================================================================= */

typedef struct JiCompositor JiCompositor;

typedef struct JiCompositorStats {
    uint32_t draw_calls;
    uint32_t batch_count;
    uint32_t target_swaps;
    uint32_t clip_changes;
    uint32_t nodes_processed;
} JiCompositorStats;

JiCompositor*      ji_compositor_create(JiGpuDevice* device);
void               ji_compositor_destroy(JiCompositor* comp);

/* Begin a new frame */
void               ji_compositor_begin_frame(JiCompositor* comp, uint32_t width, uint32_t height);

/* Submit a scene node tree for compositing */
void               ji_compositor_submit_tree(JiCompositor* comp, JiSceneNode* root);

/* Sort batches by Z-order (back-to-front) */
void               ji_compositor_sort_batches(JiCompositor* comp);

/* Execute all batches — render to current render target */
void               ji_compositor_execute(JiCompositor* comp, JiGpuDevice* device);

/* End frame and get stats */
JiCompositorStats  ji_compositor_end_frame(JiCompositor* comp);

/* Direct batch submission */
void               ji_compositor_submit_batch(JiCompositor* comp, JiDrawBatch* batch);

/* Render target management */
JiRenderTarget*    ji_compositor_acquire_target(JiCompositor* comp,
                                                 uint32_t width, uint32_t height,
                                                 JiGpuFormat format);
void               ji_compositor_release_target(JiCompositor* comp, JiRenderTarget* rt);

/* Opacity compositing */
void               ji_compositor_set_global_opacity(JiCompositor* comp, float opacity);
float              ji_compositor_get_global_opacity(const JiCompositor* comp);

/* Effect chain */
typedef struct JiEffectChain JiEffectChain;

JiEffectChain*     ji_compositor_effect_chain_create(void);
void               ji_compositor_effect_chain_destroy(JiEffectChain* chain);
void               ji_compositor_effect_add(JiEffectChain* chain, uint32_t effect_id, const void* params);
void               ji_compositor_effect_clear(JiEffectChain* chain);
uint32_t           ji_compositor_effect_count(const JiEffectChain* chain);

void               ji_compositor_set_effect_chain(JiCompositor* comp, JiEffectChain* chain);
JiEffectChain*     ji_compositor_get_effect_chain(const JiCompositor* comp);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_COMPOSITOR_H */
