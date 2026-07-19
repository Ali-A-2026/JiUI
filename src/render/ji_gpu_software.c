/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * Software GPU Backend - CPU-based fallback renderer
 */

#include "ji_gpu_internal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Software buffer backing */
typedef struct JiSwBuffer {
    uint8_t* data;
    uint64_t size;
} JiSwBuffer;

typedef struct JiSwTexture {
    uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    JiGpuFormat format;
    uint32_t row_pitch;
} JiSwTexture;

/* ---- Device ---- */
static JiGpuDevice* sw_device_create(const JiGpuDeviceOptions* options) {
    JiGpuDevice* dev = ji_calloc(1, sizeof(JiGpuDevice));
    if (!dev) return NULL;
    dev->backend = JI_GPU_SOFTWARE;
    dev->caps.backend = JI_GPU_SOFTWARE;
    strncpy(dev->caps.device_name, "JiUI Software Renderer", 255);
    strncpy(dev->caps.driver_version, "1.0", 127);
    dev->caps.max_texture_size = 4096;
    dev->caps.max_color_attachments = 1;
    dev->caps.max_vertex_attributes = 16;
    dev->caps.max_uniform_buffer_range = 65536;
    dev->caps.vram_bytes = 0; /* uses system RAM */
    dev->validation_enabled = options ? options->enable_validation : false;
    return dev;
}

static void sw_device_destroy(JiGpuDevice* device) { ji_free(device); }
static void sw_device_wait_idle(JiGpuDevice* device) { (void)device; }

/* ---- Swapchain ---- */
static JiGpuSwapchain* sw_swapchain_create(JiGpuDevice* dev, uint32_t w, uint32_t h) {
    JiGpuSwapchain* sc = ji_calloc(1, sizeof(JiGpuSwapchain));
    if (!sc) return NULL;
    sc->device = dev;
    sc->width = w;
    sc->height = h;
    sc->image_count = 2;
    sc->format = JI_GPU_FORMAT_B8G8R8A8_UNORM;
    sc->images = ji_calloc(sc->image_count, sizeof(JiGpuTexture*));
    for (uint32_t i = 0; i < sc->image_count; i++) {
        sc->images[i] = ji_gpu_texture_create(dev, w, h, sc->format, JI_GPU_TEXTURE_USAGE_COLOR_TARGET);
    }
    return sc;
}

static void sw_swapchain_destroy(JiGpuSwapchain* sc) {
    if (!sc) return;
    if (sc->images) {
        for (uint32_t i = 0; i < sc->image_count; i++)
            ji_gpu_texture_destroy(sc->images[i]);
        ji_free(sc->images);
    }
    ji_free(sc);
}

static bool sw_swapchain_resize(JiGpuSwapchain* sc, uint32_t w, uint32_t h) {
    if (!sc) return false;
    sc->width = w; sc->height = h;
    return true;
}

static uint32_t sw_swapchain_acquire(JiGpuSwapchain* sc) { return sc ? sc->current_image : 0; }
static bool sw_swapchain_present(JiGpuSwapchain* sc, uint32_t idx) { (void)idx; return sc ? true : false; }
static JiGpuTexture* sw_swapchain_get_image(JiGpuSwapchain* sc, uint32_t idx) {
    return (sc && idx < sc->image_count) ? sc->images[idx] : NULL;
}

/* ---- Command Buffer ---- */
static JiGpuCommandBuffer* sw_cmd_create(JiGpuDevice* dev) {
    JiGpuCommandBuffer* cmd = ji_calloc(1, sizeof(JiGpuCommandBuffer));
    if (cmd) cmd->device = dev;
    return cmd;
}
static void sw_cmd_destroy(JiGpuCommandBuffer* cmd) { ji_free(cmd); }
static void sw_cmd_begin(JiGpuCommandBuffer* cmd) { cmd->recording = true; }
static void sw_cmd_end(JiGpuCommandBuffer* cmd) { cmd->recording = false; }
static void sw_cmd_reset(JiGpuCommandBuffer* cmd) { cmd->recording = false; }
static void sw_cmd_submit(JiGpuCommandBuffer* cmd, JiGpuFence* fence) {
    (void)cmd;
    if (fence) fence->signaled = true;
}

/* ---- Buffer ---- */
static JiGpuBuffer* sw_buffer_create(JiGpuDevice* dev, uint64_t size, JiGpuBufferUsage usage) {
    JiGpuBuffer* buf = ji_calloc(1, sizeof(JiGpuBuffer));
    if (!buf) return NULL;
    buf->device = dev;
    buf->size = size;
    buf->usage = usage;
    buf->mapped_ptr = ji_calloc(1, (size_t)size);
    buf->memory = buf->mapped_ptr;
    return buf;
}

static void sw_buffer_destroy(JiGpuBuffer* buf) {
    if (!buf) return;
    ji_free(buf->mapped_ptr);
    ji_free(buf);
}

static void* sw_buffer_map(JiGpuBuffer* buf) { return buf ? buf->mapped_ptr : NULL; }
static void sw_buffer_unmap(JiGpuBuffer* buf) { (void)buf; }

static void sw_buffer_upload(JiGpuBuffer* buf, const void* data, uint64_t size, uint64_t offset) {
    if (!buf || !buf->mapped_ptr || !data) return;
    if (offset + size > buf->size) return;
    memcpy((uint8_t*)buf->mapped_ptr + offset, data, (size_t)size);
}

/* ---- Texture ---- */
static JiGpuTexture* sw_texture_create(JiGpuDevice* dev, uint32_t w, uint32_t h, JiGpuFormat fmt, JiGpuTextureUsage usage) {
    JiGpuTexture* tex = ji_calloc(1, sizeof(JiGpuTexture));
    if (!tex) return NULL;
    tex->device = dev;
    tex->width = w;
    tex->height = h;
    tex->format = fmt;
    tex->usage = usage;
    uint32_t bpp = ji_gpu_format_block_size(fmt);
    if (bpp == 0) bpp = 4;
    tex->memory = ji_calloc(1, (size_t)(w * h * bpp));
    return tex;
}

static void sw_texture_destroy(JiGpuTexture* tex) {
    if (!tex) return;
    ji_free(tex->memory);
    ji_free(tex);
}

static void sw_texture_upload(JiGpuTexture* tex, const void* data, uint32_t w, uint32_t h, uint32_t pitch) {
    if (!tex || !data) return;
    uint32_t bpp = ji_gpu_format_block_size(tex->format);
    if (bpp == 0) bpp = 4;
    uint32_t dst_pitch = w * bpp;
    uint8_t* dst = (uint8_t*)tex->memory;
    const uint8_t* src = (const uint8_t*)data;
    for (uint32_t y = 0; y < h && y < tex->height; y++) {
        uint32_t copy = dst_pitch < pitch ? dst_pitch : pitch;
        if (copy > 0 && y * dst_pitch + copy <= tex->width * tex->height * bpp)
            memcpy(dst + y * dst_pitch, src + y * pitch, copy);
    }
}

/* ---- Texture View ---- */
static JiGpuTextureView* sw_texture_view_create(JiGpuTexture* tex, JiGpuFormat fmt) {
    JiGpuTextureView* view = ji_calloc(1, sizeof(JiGpuTextureView));
    if (view) { view->texture = tex; view->format = fmt; }
    return view;
}
static void sw_texture_view_destroy(JiGpuTextureView* view) { ji_free(view); }

/* ---- Sampler ---- */
static JiGpuSampler* sw_sampler_create(JiGpuDevice* dev, JiGpuFilter mg, JiGpuFilter mn, JiGpuSamplerAddressMode u, JiGpuSamplerAddressMode v, float a) {
    JiGpuSampler* s = ji_calloc(1, sizeof(JiGpuSampler));
    if (s) s->device = dev;
    (void)mg; (void)mn; (void)u; (void)v; (void)a;
    return s;
}
static void sw_sampler_destroy(JiGpuSampler* s) { ji_free(s); }

/* ---- Shader (stub) ---- */
static JiGpuShader* sw_shader_create_spirv(JiGpuDevice* dev, JiGpuShaderStage stage, const uint32_t* code, size_t size) {
    JiGpuShader* sh = ji_calloc(1, sizeof(JiGpuShader));
    if (sh) { sh->device = dev; sh->stage = stage; }
    (void)code; (void)size;
    return sh;
}
static JiGpuShader* sw_shader_create_glsl(JiGpuDevice* dev, JiGpuShaderStage stage, const char* src) {
    JiGpuShader* sh = ji_calloc(1, sizeof(JiGpuShader));
    if (sh) { sh->device = dev; sh->stage = stage; }
    (void)src;
    return sh;
}
static void sw_shader_destroy(JiGpuShader* sh) { ji_free(sh); }

/* ---- Descriptor Set (stub) ---- */
static JiGpuDescriptorSetLayout* sw_ds_layout_create(JiGpuDevice* dev, const JiGpuDescriptorSetLayoutBinding* b, uint32_t c) {
    JiGpuDescriptorSetLayout* l = ji_calloc(1, sizeof(JiGpuDescriptorSetLayout));
    if (l) l->device = dev;
    (void)b; (void)c;
    return l;
}
static void sw_ds_layout_destroy(JiGpuDescriptorSetLayout* l) { ji_free(l); }
static JiGpuDescriptorSet* sw_ds_create(JiGpuDevice* dev, JiGpuDescriptorSetLayout* layout) {
    JiGpuDescriptorSet* s = ji_calloc(1, sizeof(JiGpuDescriptorSet));
    if (s) s->device = dev;
    (void)layout;
    return s;
}
static void sw_ds_destroy(JiGpuDescriptorSet* s) { ji_free(s); }
static void sw_ds_update(JiGpuDevice* dev, const JiGpuWriteDescriptorSet* w, uint32_t c) { (void)dev; (void)w; (void)c; }

/* ---- Pipeline Layout (stub) ---- */
static JiGpuPipelineLayout* sw_pipeline_layout_create(JiGpuDevice* dev, JiGpuDescriptorSetLayout** l, uint32_t c, const void* p, uint32_t pc) {
    JiGpuPipelineLayout* pl = ji_calloc(1, sizeof(JiGpuPipelineLayout));
    if (pl) pl->device = dev;
    (void)l; (void)c; (void)p; (void)pc;
    return pl;
}
static void sw_pipeline_layout_destroy(JiGpuPipelineLayout* l) { ji_free(l); }

/* ---- Pipeline (stub) ---- */
static JiGpuPipeline* sw_pipeline_create(JiGpuDevice* dev, const JiGpuPipelineCreateInfo* info) {
    JiGpuPipeline* p = ji_calloc(1, sizeof(JiGpuPipeline));
    if (p) p->device = dev;
    (void)info;
    return p;
}
static void sw_pipeline_destroy(JiGpuPipeline* p) { ji_free(p); }

/* ---- Render Pass (stub) ---- */
static JiGpuRenderPass* sw_render_pass_create(JiGpuDevice* dev, const JiGpuFormat* cf, uint32_t cc, JiGpuFormat df, JiGpuLoadOp l, JiGpuStoreOp s) {
    JiGpuRenderPass* p = ji_calloc(1, sizeof(JiGpuRenderPass));
    if (p) p->device = dev;
    (void)cf; (void)cc; (void)df; (void)l; (void)s;
    return p;
}
static void sw_render_pass_destroy(JiGpuRenderPass* p) { ji_free(p); }

/* ---- Framebuffer (stub) ---- */
static JiGpuFramebuffer* sw_framebuffer_create(JiGpuDevice* dev, JiGpuRenderPass* pass, JiGpuTextureView** colors, uint32_t cc, JiGpuTextureView* depth, uint32_t w, uint32_t h) {
    JiGpuFramebuffer* fb = ji_calloc(1, sizeof(JiGpuFramebuffer));
    if (fb) { fb->device = dev; fb->width = w; fb->height = h; }
    (void)pass; (void)colors; (void)cc; (void)depth;
    return fb;
}
static void sw_framebuffer_destroy(JiGpuFramebuffer* fb) { ji_free(fb); }

/* ---- Sync (stub) ---- */
static JiGpuFence* sw_fence_create(JiGpuDevice* dev, bool signaled) {
    JiGpuFence* f = ji_calloc(1, sizeof(JiGpuFence));
    if (f) { f->device = dev; f->signaled = signaled; }
    return f;
}
static void sw_fence_destroy(JiGpuFence* f) { ji_free(f); }
static void sw_fence_reset(JiGpuFence* f) { if (f) f->signaled = false; }
static bool sw_fence_wait(JiGpuFence* f, uint64_t t) { (void)t; return f ? f->signaled : false; }
static bool sw_fence_is_signaled(const JiGpuFence* f) { return f ? f->signaled : false; }
static JiGpuSemaphore* sw_semaphore_create(JiGpuDevice* dev) {
    JiGpuSemaphore* s = ji_calloc(1, sizeof(JiGpuSemaphore));
    if (s) s->device = dev;
    return s;
}
static void sw_semaphore_destroy(JiGpuSemaphore* s) { ji_free(s); }

/* ---- Command Recording (stubs) ---- */
static void sw_cmd_begin_render_pass(JiGpuCommandBuffer* c, JiGpuRenderPass* p, JiGpuFramebuffer* fb, uint32_t w, uint32_t h, const JiGpuColorClearValue* cc, uint32_t ccc, const JiGpuDepthStencilClearValue* dc) {
    (void)c; (void)p; (void)fb; (void)w; (void)h; (void)cc; (void)ccc; (void)dc;
}
static void sw_cmd_end_render_pass(JiGpuCommandBuffer* c) { (void)c; }
static void sw_cmd_bind_pipeline(JiGpuCommandBuffer* c, JiGpuPipeline* p) { (void)c; (void)p; }
static void sw_cmd_bind_descriptor_sets(JiGpuCommandBuffer* c, JiGpuPipelineLayout* l, uint32_t fs, JiGpuDescriptorSet** s, uint32_t n) { (void)c; (void)l; (void)fs; (void)s; (void)n; }
static void sw_cmd_bind_vertex_buffer(JiGpuCommandBuffer* c, uint32_t b, JiGpuBuffer* buf, uint64_t o) { (void)c; (void)b; (void)buf; (void)o; }
static void sw_cmd_bind_index_buffer(JiGpuCommandBuffer* c, JiGpuBuffer* buf, uint64_t o, JiGpuFormat f) { (void)c; (void)buf; (void)o; (void)f; }
static void sw_cmd_push_constants(JiGpuCommandBuffer* c, JiGpuPipelineLayout* l, JiGpuShaderStage s, uint32_t o, uint32_t sz, const void* d) { (void)c; (void)l; (void)s; (void)o; (void)sz; (void)d; }
static void sw_cmd_set_viewport(JiGpuCommandBuffer* c, const JiGpuViewport* v) { (void)c; (void)v; }
static void sw_cmd_set_scissor(JiGpuCommandBuffer* c, const JiGpuScissor* s) { (void)c; (void)s; }
static void sw_cmd_draw(JiGpuCommandBuffer* c, uint32_t v, uint32_t i, uint32_t fv, uint32_t fi) { (void)c; (void)v; (void)i; (void)fv; (void)fi; }
static void sw_cmd_draw_indexed(JiGpuCommandBuffer* c, uint32_t ic, uint32_t ins, uint32_t fi, int32_t vo, uint32_t fis) { (void)c; (void)ic; (void)ins; (void)fi; (void)vo; (void)fis; }

/* =========================================================================
 * Backend Registration
 * ========================================================================= */
bool ji_gpu_software_init(JiGpuBackendVtable* vt) {
    if (!vt) return false;
    vt->device_create       = sw_device_create;
    vt->device_destroy      = sw_device_destroy;
    vt->device_wait_idle    = sw_device_wait_idle;
    vt->swapchain_create    = sw_swapchain_create;
    vt->swapchain_destroy   = sw_swapchain_destroy;
    vt->swapchain_resize    = sw_swapchain_resize;
    vt->swapchain_acquire   = sw_swapchain_acquire;
    vt->swapchain_present   = sw_swapchain_present;
    vt->swapchain_get_image = sw_swapchain_get_image;
    vt->cmd_create          = sw_cmd_create;
    vt->cmd_destroy         = sw_cmd_destroy;
    vt->cmd_begin           = sw_cmd_begin;
    vt->cmd_end             = sw_cmd_end;
    vt->cmd_reset           = sw_cmd_reset;
    vt->cmd_submit          = sw_cmd_submit;
    vt->buffer_create       = sw_buffer_create;
    vt->buffer_destroy      = sw_buffer_destroy;
    vt->buffer_map          = sw_buffer_map;
    vt->buffer_unmap        = sw_buffer_unmap;
    vt->buffer_upload       = sw_buffer_upload;
    vt->texture_create      = sw_texture_create;
    vt->texture_destroy     = sw_texture_destroy;
    vt->texture_upload      = sw_texture_upload;
    vt->texture_view_create = sw_texture_view_create;
    vt->texture_view_destroy = sw_texture_view_destroy;
    vt->sampler_create      = sw_sampler_create;
    vt->sampler_destroy     = sw_sampler_destroy;
    vt->shader_create_spirv = sw_shader_create_spirv;
    vt->shader_create_glsl = sw_shader_create_glsl;
    vt->shader_destroy      = sw_shader_destroy;
    vt->ds_layout_create    = sw_ds_layout_create;
    vt->ds_layout_destroy   = sw_ds_layout_destroy;
    vt->ds_create           = sw_ds_create;
    vt->ds_destroy          = sw_ds_destroy;
    vt->ds_update           = sw_ds_update;
    vt->pipeline_layout_create  = sw_pipeline_layout_create;
    vt->pipeline_layout_destroy = sw_pipeline_layout_destroy;
    vt->pipeline_create     = sw_pipeline_create;
    vt->pipeline_destroy    = sw_pipeline_destroy;
    vt->render_pass_create  = sw_render_pass_create;
    vt->render_pass_destroy = sw_render_pass_destroy;
    vt->framebuffer_create  = sw_framebuffer_create;
    vt->framebuffer_destroy = sw_framebuffer_destroy;
    vt->fence_create        = sw_fence_create;
    vt->fence_destroy       = sw_fence_destroy;
    vt->fence_reset         = sw_fence_reset;
    vt->fence_wait          = sw_fence_wait;
    vt->fence_is_signaled   = sw_fence_is_signaled;
    vt->semaphore_create    = sw_semaphore_create;
    vt->semaphore_destroy   = sw_semaphore_destroy;
    vt->cmd_begin_render_pass    = sw_cmd_begin_render_pass;
    vt->cmd_end_render_pass      = sw_cmd_end_render_pass;
    vt->cmd_bind_pipeline        = sw_cmd_bind_pipeline;
    vt->cmd_bind_descriptor_sets  = sw_cmd_bind_descriptor_sets;
    vt->cmd_bind_vertex_buffer   = sw_cmd_bind_vertex_buffer;
    vt->cmd_bind_index_buffer    = sw_cmd_bind_index_buffer;
    vt->cmd_push_constants       = sw_cmd_push_constants;
    vt->cmd_set_viewport         = sw_cmd_set_viewport;
    vt->cmd_set_scissor          = sw_cmd_set_scissor;
    vt->cmd_draw                  = sw_cmd_draw;
    vt->cmd_draw_indexed          = sw_cmd_draw_indexed;
    return true;
}
