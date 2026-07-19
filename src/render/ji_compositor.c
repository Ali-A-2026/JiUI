/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

#include "jiui/ji_compositor.h"
#include "ji_gpu_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Render Target Pool
 * ========================================================================= */

struct JiRenderTargetPool {
    JiGpuDevice*    device;
    uint32_t        max_targets;
    uint32_t        active_count;
    JiRenderTarget* free_list;
};

JiRenderTargetPool* ji_render_target_pool_create(JiGpuDevice* device, uint32_t max_targets) {
    JiRenderTargetPool* pool = calloc(1, sizeof(JiRenderTargetPool));
    if (!pool) return NULL;
    pool->device = device;
    pool->max_targets = max_targets;
    return pool;
}

void ji_render_target_pool_destroy(JiRenderTargetPool* pool) {
    if (!pool) return;
    JiRenderTarget* rt = pool->free_list;
    while (rt) {
        JiRenderTarget* next = rt->next;
        if (rt->color) ji_gpu_texture_destroy(rt->color);
        if (rt->depth) ji_gpu_texture_destroy(rt->depth);
        free(rt);
        rt = next;
    }
    free(pool);
}

JiRenderTarget* ji_render_target_pool_acquire(JiRenderTargetPool* pool,
                                               uint32_t width, uint32_t height,
                                               JiGpuFormat format) {
    if (!pool) return NULL;
    /* Try to find a matching free target */
    JiRenderTarget* rt = pool->free_list;
    while (rt) {
        if (!rt->in_use && rt->width == width && rt->height == height && rt->format == format) {
            rt->in_use = true;
            pool->active_count++;
            return rt;
        }
        rt = rt->next;
    }
    /* Allocate a new one if under limit */
    if (pool->active_count >= pool->max_targets) {
        fprintf(stderr, "[JiUI] Render target pool exhausted (%u/%u)\n",
                pool->active_count, pool->max_targets);
        return NULL;
    }
    rt = calloc(1, sizeof(JiRenderTarget));
    if (!rt) return NULL;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->in_use = true;
    /* Create GPU textures via vtable */
    if (g_gpu_vtable.texture_create) {
        rt->color = g_gpu_vtable.texture_create(pool->device, width, height, format,
                                                 JI_GPU_TEXTURE_USAGE_COLOR_TARGET);
    }
    /* Add to pool */
    rt->next = pool->free_list;
    pool->free_list = rt;
    pool->active_count++;
    return rt;
}

void ji_render_target_pool_release(JiRenderTargetPool* pool, JiRenderTarget* rt) {
    if (!pool || !rt) return;
    rt->in_use = false;
    if (pool->active_count > 0) pool->active_count--;
}

/* =========================================================================
 * Clip Stack — uses JiRectF for float-precision clip regions
 * ========================================================================= */

struct JiClipStack {
    JiRectF*   stack;
    uint32_t   capacity;
    uint32_t   depth;
};

JiClipStack* ji_clip_stack_create(uint32_t max_depth) {
    JiClipStack* s = calloc(1, sizeof(JiClipStack));
    if (!s) return NULL;
    s->capacity = max_depth;
    s->stack = calloc(max_depth, sizeof(JiRectF));
    return s;
}

void ji_clip_stack_destroy(JiClipStack* stack) {
    if (!stack) return;
    free(stack->stack);
    free(stack);
}

void ji_clip_stack_push(JiClipStack* stack, JiRectF clip) {
    if (!stack || stack->depth >= stack->capacity) return;
    stack->stack[stack->depth++] = clip;
}

JiRectF ji_clip_stack_pop(JiClipStack* stack) {
    if (!stack || stack->depth == 0) return (JiRectF){0, 0, 0, 0};
    return stack->stack[--stack->depth];
}

JiRectF ji_clip_stack_peek(const JiClipStack* stack) {
    if (!stack || stack->depth == 0) return (JiRectF){0, 0, 0, 0};
    return stack->stack[stack->depth - 1];
}

JiRectF ji_clip_stack_intersect(const JiClipStack* stack, JiRectF rect) {
    if (!stack || stack->depth == 0) return rect;
    JiRectF clip = stack->stack[stack->depth - 1];
    float x1 = rect.x > clip.x ? rect.x : clip.x;
    float y1 = rect.y > clip.y ? rect.y : clip.y;
    float x2 = (rect.x + rect.width) < (clip.x + clip.width) ? (rect.x + rect.width) : (clip.x + clip.width);
    float y2 = (rect.y + rect.height) < (clip.y + clip.height) ? (rect.y + rect.height) : (clip.y + clip.height);
    return (JiRectF){x1, y1, x2 > x1 ? x2 - x1 : 0, y2 > y1 ? y2 - y1 : 0};
}

uint32_t ji_clip_stack_depth(const JiClipStack* stack) {
    return stack ? stack->depth : 0;
}

/* =========================================================================
 * Effect Chain
 * ========================================================================= */

struct JiEffectChain {
    uint32_t count;
    struct JiEffectChainEntry {
        uint32_t    effect_id;
        void*       params;
    }* entries;
    uint32_t capacity;
};

JiEffectChain* ji_compositor_effect_chain_create(void) {
    JiEffectChain* chain = calloc(1, sizeof(JiEffectChain));
    if (!chain) return NULL;
    chain->capacity = 8;
    chain->entries = calloc(chain->capacity, sizeof(struct JiEffectChainEntry));
    return chain;
}

void ji_compositor_effect_chain_destroy(JiEffectChain* chain) {
    if (!chain) return;
    free(chain->entries);
    free(chain);
}

void ji_compositor_effect_add(JiEffectChain* chain, uint32_t effect_id, const void* params) {
    if (!chain) return;
    if (chain->count >= chain->capacity) {
        chain->capacity *= 2;
        chain->entries = realloc(chain->entries, chain->capacity * sizeof(struct JiEffectChainEntry));
    }
    chain->entries[chain->count].effect_id = effect_id;
    chain->entries[chain->count].params = (void*)params;
    chain->count++;
}

void ji_compositor_effect_clear(JiEffectChain* chain) {
    if (chain) chain->count = 0;
}

uint32_t ji_compositor_effect_count(const JiEffectChain* chain) {
    return chain ? chain->count : 0;
}

/* =========================================================================
 * Compositor
 * ========================================================================= */

struct JiCompositor {
    JiGpuDevice*         device;
    JiRenderTargetPool*  rt_pool;
    JiClipStack*         clip_stack;
    JiDrawBatch*         batch_head;
    JiDrawBatch*         batch_tail;
    uint32_t             batch_count;
    float                global_opacity;
    JiEffectChain*       effect_chain;
    JiCompositorStats    stats;
    uint32_t             frame_width;
    uint32_t             frame_height;
    bool                 in_frame;
};

JiCompositor* ji_compositor_create(JiGpuDevice* device) {
    JiCompositor* comp = calloc(1, sizeof(JiCompositor));
    if (!comp) return NULL;
    comp->device = device;
    comp->rt_pool = ji_render_target_pool_create(device, 16);
    comp->clip_stack = ji_clip_stack_create(32);
    comp->effect_chain = ji_compositor_effect_chain_create();
    comp->global_opacity = 1.0f;
    return comp;
}

void ji_compositor_destroy(JiCompositor* comp) {
    if (!comp) return;
    ji_render_target_pool_destroy(comp->rt_pool);
    ji_clip_stack_destroy(comp->clip_stack);
    ji_compositor_effect_chain_destroy(comp->effect_chain);
    free(comp);
}

void ji_compositor_begin_frame(JiCompositor* comp, uint32_t width, uint32_t height) {
    if (!comp) return;
    comp->frame_width = width;
    comp->frame_height = height;
    comp->in_frame = true;
    comp->batch_head = NULL;
    comp->batch_tail = NULL;
    comp->batch_count = 0;
    memset(&comp->stats, 0, sizeof(comp->stats));
}

static void compositor_process_node(JiCompositor* comp, JiSceneNode* node, float parent_opacity) {
    if (!node || !node->visible) return;
    float opacity = parent_opacity * node->opacity;
    if (opacity <= 0.001f) return;

    /* Push clip for this node */
    if (node->bounds.width > 0 && node->bounds.height > 0) {
        ji_clip_stack_push(comp->clip_stack, node->bounds);
    }

    /* Create a draw batch for renderable nodes */
    if (node->type == JI_SCENE_NODE_RENDERABLE || node->type == JI_SCENE_NODE_WIDGET) {
        JiDrawBatch* batch = calloc(1, sizeof(JiDrawBatch));
        if (batch) {
            batch->type = JI_BATCH_RECT;
            batch->z_order = node->bounds.y + node->bounds.height;
            batch->clip_rect = ji_clip_stack_peek(comp->clip_stack);
            batch->opacity = opacity;
            batch->next = NULL;
            if (comp->batch_tail) {
                comp->batch_tail->next = batch;
            } else {
                comp->batch_head = batch;
            }
            comp->batch_tail = batch;
            comp->batch_count++;
            comp->stats.nodes_processed++;
        }
    }

    /* Recurse children */
    for (uint32_t i = 0; i < node->child_count; i++) {
        compositor_process_node(comp, node->children[i], opacity);
    }

    /* Pop clip */
    if (node->bounds.width > 0 && node->bounds.height > 0) {
        ji_clip_stack_pop(comp->clip_stack);
    }
}

void ji_compositor_submit_tree(JiCompositor* comp, JiSceneNode* root) {
    if (!comp || !root) return;
    compositor_process_node(comp, root, comp->global_opacity);
}

static int batch_z_compare(const void* a, const void* b) {
    const JiDrawBatch* ba = *(const JiDrawBatch**)a;
    const JiDrawBatch* bb = *(const JiDrawBatch**)b;
    if (ba->z_order < bb->z_order) return -1;
    if (ba->z_order > bb->z_order) return 1;
    return 0;
}

void ji_compositor_sort_batches(JiCompositor* comp) {
    if (!comp || comp->batch_count == 0) return;
    JiDrawBatch** arr = malloc(comp->batch_count * sizeof(JiDrawBatch*));
    if (!arr) return;
    uint32_t i = 0;
    JiDrawBatch* b = comp->batch_head;
    while (b) { arr[i++] = b; b = b->next; }
    qsort(arr, comp->batch_count, sizeof(JiDrawBatch*), batch_z_compare);
    comp->batch_head = arr[0];
    for (i = 0; i < comp->batch_count - 1; i++) {
        arr[i]->next = arr[i + 1];
    }
    arr[comp->batch_count - 1]->next = NULL;
    comp->batch_tail = arr[comp->batch_count - 1];
    free(arr);
}

void ji_compositor_execute(JiCompositor* comp, JiGpuDevice* device) {
    if (!comp) return;
    JiDrawBatch* batch = comp->batch_head;
    while (batch) {
        comp->stats.draw_calls++;
        comp->stats.batch_count++;
        batch = batch->next;
    }
}

JiCompositorStats ji_compositor_end_frame(JiCompositor* comp) {
    if (!comp) return (JiCompositorStats){0};
    JiDrawBatch* batch = comp->batch_head;
    while (batch) {
        JiDrawBatch* next = batch->next;
        free(batch);
        batch = next;
    }
    comp->batch_head = NULL;
    comp->batch_tail = NULL;
    comp->in_frame = false;
    comp->stats.batch_count = comp->batch_count;
    return comp->stats;
}

void ji_compositor_submit_batch(JiCompositor* comp, JiDrawBatch* batch) {
    if (!comp || !batch) return;
    batch->next = NULL;
    if (comp->batch_tail) {
        comp->batch_tail->next = batch;
    } else {
        comp->batch_head = batch;
    }
    comp->batch_tail = batch;
    comp->batch_count++;
}

JiRenderTarget* ji_compositor_acquire_target(JiCompositor* comp,
                                              uint32_t width, uint32_t height,
                                              JiGpuFormat format) {
    if (!comp) return NULL;
    return ji_render_target_pool_acquire(comp->rt_pool, width, height, format);
}

void ji_compositor_release_target(JiCompositor* comp, JiRenderTarget* rt) {
    if (!comp) return;
    ji_render_target_pool_release(comp->rt_pool, rt);
}

void ji_compositor_set_global_opacity(JiCompositor* comp, float opacity) {
    if (comp) comp->global_opacity = opacity < 0 ? 0 : (opacity > 1 ? 1 : opacity);
}

float ji_compositor_get_global_opacity(const JiCompositor* comp) {
    return comp ? comp->global_opacity : 1.0f;
}

void ji_compositor_set_effect_chain(JiCompositor* comp, JiEffectChain* chain) {
    if (comp) comp->effect_chain = chain;
}

JiEffectChain* ji_compositor_get_effect_chain(const JiCompositor* comp) {
    return comp ? comp->effect_chain : NULL;
}
