/**
 * JiUI - GPU Backend Abstraction Implementation
 * Dispatches to backend-specific implementations (Vulkan, OpenGL, Software)
 */

#include "ji_gpu_internal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Global vtable */
JiGpuBackendVtable g_gpu_vtable = {0};

/* Backend init functions (defined in backend-specific files) */
extern bool ji_gpu_software_init(JiGpuBackendVtable* vtable);

/* Stub implementations for backends not yet implemented */
bool ji_gpu_vulkan_init(JiGpuBackendVtable* vtable) { (void)vtable; fprintf(stderr, "[JiUI] Vulkan backend not yet implemented\n"); return false; }
bool ji_gpu_opengl_init(JiGpuBackendVtable* vtable) { (void)vtable; fprintf(stderr, "[JiUI] OpenGL backend not yet implemented\n"); return false; }

static bool ji_gpu_init_backend(JiGpuBackend backend) {
    memset(&g_gpu_vtable, 0, sizeof(g_gpu_vtable));
    switch (backend) {
        case JI_GPU_VULKAN:   return ji_gpu_vulkan_init(&g_gpu_vtable);
        case JI_GPU_OPENGL:   return ji_gpu_opengl_init(&g_gpu_vtable);
        case JI_GPU_SOFTWARE: return ji_gpu_software_init(&g_gpu_vtable);
        default: return false;
    }
}

/* =========================================================================
 * Utility
 * ========================================================================= */
const char* ji_gpu_backend_name(JiGpuBackend backend) {
    static const char* names[] = {
        "Vulkan", "Metal", "D3D11", "D3D12", "OpenGL", "WebGPU", "Software"
    };
    if (backend >= 0 && backend < JI_GPU_BACKEND_COUNT) return names[backend];
    return "Unknown";
}

uint32_t ji_gpu_format_block_size(JiGpuFormat format) {
    switch (format) {
        case JI_GPU_FORMAT_R8G8B8A8_UNORM: case JI_GPU_FORMAT_B8G8R8A8_UNORM:
        case JI_GPU_FORMAT_R8G8B8A8_SRGB: case JI_GPU_FORMAT_B8G8R8A8_SRGB:
        case JI_GPU_FORMAT_D24_UNORM_S8_UINT: return 4;
        case JI_GPU_FORMAT_R32G32B32A32_SFLOAT: return 16;
        case JI_GPU_FORMAT_R16G16B16A16_SFLOAT: return 8;
        case JI_GPU_FORMAT_R8_UNORM: case JI_GPU_FORMAT_D24_UNORM: return 1;
        case JI_GPU_FORMAT_D32_SFLOAT: return 4;
        default: return 0;
    }
}

bool ji_gpu_format_has_depth(JiGpuFormat f) {
    return f == JI_GPU_FORMAT_D24_UNORM_S8_UINT || f == JI_GPU_FORMAT_D32_SFLOAT || f == JI_GPU_FORMAT_D24_UNORM;
}
bool ji_gpu_format_has_stencil(JiGpuFormat f) { return f == JI_GPU_FORMAT_D24_UNORM_S8_UINT; }
bool ji_gpu_format_is_compressed(JiGpuFormat f) {
    return f >= JI_GPU_FORMAT_BC1_RGB_UNORM_BLOCK && f <= JI_GPU_FORMAT_BC7_UNORM_BLOCK;
}

/* =========================================================================
 * Device API
 * ========================================================================= */
JiGpuDevice* ji_gpu_device_create(const JiGpuDeviceOptions* options) {
    if (!options) return NULL;
    JiGpuBackend try[] = { options->preferred_backend, JI_GPU_VULKAN, JI_GPU_OPENGL, JI_GPU_SOFTWARE };
    for (int i = 0; i < 4; i++) {
        if (try[i] < 0 || try[i] >= JI_GPU_BACKEND_COUNT) continue;
        if (ji_gpu_init_backend(try[i]) && g_gpu_vtable.device_create) {
            JiGpuDevice* dev = g_gpu_vtable.device_create(options);
            if (dev) { fprintf(stdout, "[JiUI] GPU backend: %s\n", ji_gpu_backend_name(try[i])); return dev; }
        }
    }
    fprintf(stderr, "[JiUI] Failed to initialize any GPU backend\n");
    return NULL;
}

void ji_gpu_device_destroy(JiGpuDevice* d) { if (d && g_gpu_vtable.device_destroy) g_gpu_vtable.device_destroy(d); }
const JiGpuCaps* ji_gpu_device_get_caps(const JiGpuDevice* d) { return d ? &d->caps : NULL; }
void ji_gpu_device_wait_idle(JiGpuDevice* d) { if (d && g_gpu_vtable.device_wait_idle) g_gpu_vtable.device_wait_idle(d); }

/* Swapchain */
JiGpuSwapchain* ji_gpu_swapchain_create(JiGpuDevice* d, uint32_t w, uint32_t h) { return d && g_gpu_vtable.swapchain_create ? g_gpu_vtable.swapchain_create(d, w, h) : NULL; }
void ji_gpu_swapchain_destroy(JiGpuSwapchain* s) { if (s && g_gpu_vtable.swapchain_destroy) g_gpu_vtable.swapchain_destroy(s); }
bool ji_gpu_swapchain_resize(JiGpuSwapchain* s, uint32_t w, uint32_t h) { return s && g_gpu_vtable.swapchain_resize ? g_gpu_vtable.swapchain_resize(s, w, h) : false; }
uint32_t ji_gpu_swapchain_acquire_next_image(JiGpuSwapchain* s) { return s && g_gpu_vtable.swapchain_acquire ? g_gpu_vtable.swapchain_acquire(s) : 0; }
bool ji_gpu_swapchain_present(JiGpuSwapchain* s, uint32_t i) { return s && g_gpu_vtable.swapchain_present ? g_gpu_vtable.swapchain_present(s, i) : false; }
JiGpuTexture* ji_gpu_swapchain_get_image(JiGpuSwapchain* s, uint32_t i) { return s && g_gpu_vtable.swapchain_get_image ? g_gpu_vtable.swapchain_get_image(s, i) : NULL; }
uint32_t ji_gpu_swapchain_get_image_count(const JiGpuSwapchain* s) { return s ? s->image_count : 0; }
JiGpuFormat ji_gpu_swapchain_get_format(const JiGpuSwapchain* s) { return s ? s->format : JI_GPU_FORMAT_UNDEFINED; }

/* Command Buffers */
JiGpuCommandBuffer* ji_gpu_command_buffer_create(JiGpuDevice* d) { return d && g_gpu_vtable.cmd_create ? g_gpu_vtable.cmd_create(d) : NULL; }
void ji_gpu_command_buffer_destroy(JiGpuCommandBuffer* c) { if (c && g_gpu_vtable.cmd_destroy) g_gpu_vtable.cmd_destroy(c); }
void ji_gpu_command_buffer_begin(JiGpuCommandBuffer* c) { if (c && g_gpu_vtable.cmd_begin) g_gpu_vtable.cmd_begin(c); }
void ji_gpu_command_buffer_end(JiGpuCommandBuffer* c) { if (c && g_gpu_vtable.cmd_end) g_gpu_vtable.cmd_end(c); }
void ji_gpu_command_buffer_reset(JiGpuCommandBuffer* c) { if (c && g_gpu_vtable.cmd_reset) g_gpu_vtable.cmd_reset(c); }
void ji_gpu_command_buffer_submit(JiGpuCommandBuffer* c, JiGpuFence* f) { if (c && g_gpu_vtable.cmd_submit) g_gpu_vtable.cmd_submit(c, f); }

/* Buffers */
JiGpuBuffer* ji_gpu_buffer_create(JiGpuDevice* d, uint64_t s, JiGpuBufferUsage u) { return d && g_gpu_vtable.buffer_create ? g_gpu_vtable.buffer_create(d, s, u) : NULL; }
void ji_gpu_buffer_destroy(JiGpuBuffer* b) { if (b && g_gpu_vtable.buffer_destroy) g_gpu_vtable.buffer_destroy(b); }
void* ji_gpu_buffer_map(JiGpuBuffer* b) { return b && g_gpu_vtable.buffer_map ? g_gpu_vtable.buffer_map(b) : NULL; }
void ji_gpu_buffer_unmap(JiGpuBuffer* b) { if (b && g_gpu_vtable.buffer_unmap) g_gpu_vtable.buffer_unmap(b); }
void ji_gpu_buffer_upload(JiGpuBuffer* b, const void* d, uint64_t s, uint64_t o) { if (b && g_gpu_vtable.buffer_upload) g_gpu_vtable.buffer_upload(b, d, s, o); }
uint64_t ji_gpu_buffer_get_size(const JiGpuBuffer* b) { return b ? b->size : 0; }

/* Textures */
JiGpuTexture* ji_gpu_texture_create(JiGpuDevice* d, uint32_t w, uint32_t h, JiGpuFormat f, JiGpuTextureUsage u) { return d && g_gpu_vtable.texture_create ? g_gpu_vtable.texture_create(d, w, h, f, u) : NULL; }
void ji_gpu_texture_destroy(JiGpuTexture* t) { if (t && g_gpu_vtable.texture_destroy) g_gpu_vtable.texture_destroy(t); }
void ji_gpu_texture_upload(JiGpuTexture* t, const void* d, uint32_t w, uint32_t h, uint32_t p) { if (t && g_gpu_vtable.texture_upload) g_gpu_vtable.texture_upload(t, d, w, h, p); }
uint32_t ji_gpu_texture_get_width(const JiGpuTexture* t) { return t ? t->width : 0; }
uint32_t ji_gpu_texture_get_height(const JiGpuTexture* t) { return t ? t->height : 0; }
JiGpuFormat ji_gpu_texture_get_format(const JiGpuTexture* t) { return t ? t->format : JI_GPU_FORMAT_UNDEFINED; }

/* Texture Views, Samplers, Shaders */
JiGpuTextureView* ji_gpu_texture_view_create(JiGpuTexture* t, JiGpuFormat f) { return t && g_gpu_vtable.texture_view_create ? g_gpu_vtable.texture_view_create(t, f) : NULL; }
void ji_gpu_texture_view_destroy(JiGpuTextureView* v) { if (v && g_gpu_vtable.texture_view_destroy) g_gpu_vtable.texture_view_destroy(v); }
JiGpuSampler* ji_gpu_sampler_create(JiGpuDevice* d, JiGpuFilter mg, JiGpuFilter mn, JiGpuSamplerAddressMode u, JiGpuSamplerAddressMode v, float a) { return d && g_gpu_vtable.sampler_create ? g_gpu_vtable.sampler_create(d, mg, mn, u, v, a) : NULL; }
void ji_gpu_sampler_destroy(JiGpuSampler* s) { if (s && g_gpu_vtable.sampler_destroy) g_gpu_vtable.sampler_destroy(s); }
JiGpuShader* ji_gpu_shader_create_from_spirv(JiGpuDevice* d, JiGpuShaderStage st, const uint32_t* c, size_t sz) { return d && g_gpu_vtable.shader_create_spirv ? g_gpu_vtable.shader_create_spirv(d, st, c, sz) : NULL; }
JiGpuShader* ji_gpu_shader_create_from_glsl(JiGpuDevice* d, JiGpuShaderStage st, const char* s) { return d && g_gpu_vtable.shader_create_glsl ? g_gpu_vtable.shader_create_glsl(d, st, s) : NULL; }
void ji_gpu_shader_destroy(JiGpuShader* s) { if (s && g_gpu_vtable.shader_destroy) g_gpu_vtable.shader_destroy(s); }

/* Descriptor Sets, Pipeline Layout, Pipeline, Render Pass, Framebuffer */
JiGpuDescriptorSetLayout* ji_gpu_descriptor_set_layout_create(JiGpuDevice* d, const JiGpuDescriptorSetLayoutBinding* b, uint32_t c) { return d && g_gpu_vtable.ds_layout_create ? g_gpu_vtable.ds_layout_create(d, b, c) : NULL; }
void ji_gpu_descriptor_set_layout_destroy(JiGpuDescriptorSetLayout* l) { if (l && g_gpu_vtable.ds_layout_destroy) g_gpu_vtable.ds_layout_destroy(l); }
JiGpuDescriptorSet* ji_gpu_descriptor_set_create(JiGpuDevice* d, JiGpuDescriptorSetLayout* l) { return d && g_gpu_vtable.ds_create ? g_gpu_vtable.ds_create(d, l) : NULL; }
void ji_gpu_descriptor_set_destroy(JiGpuDescriptorSet* s) { if (s && g_gpu_vtable.ds_destroy) g_gpu_vtable.ds_destroy(s); }
void ji_gpu_descriptor_set_update(JiGpuDevice* d, const JiGpuWriteDescriptorSet* w, uint32_t c) { if (d && g_gpu_vtable.ds_update) g_gpu_vtable.ds_update(d, w, c); }
JiGpuPipelineLayout* ji_gpu_pipeline_layout_create(JiGpuDevice* d, JiGpuDescriptorSetLayout** l, uint32_t c, const void* p, uint32_t pc) { return d && g_gpu_vtable.pipeline_layout_create ? g_gpu_vtable.pipeline_layout_create(d, l, c, p, pc) : NULL; }
void ji_gpu_pipeline_layout_destroy(JiGpuPipelineLayout* l) { if (l && g_gpu_vtable.pipeline_layout_destroy) g_gpu_vtable.pipeline_layout_destroy(l); }
JiGpuPipeline* ji_gpu_pipeline_create(JiGpuDevice* d, const JiGpuPipelineCreateInfo* i) { return d && g_gpu_vtable.pipeline_create ? g_gpu_vtable.pipeline_create(d, i) : NULL; }
void ji_gpu_pipeline_destroy(JiGpuPipeline* p) { if (p && g_gpu_vtable.pipeline_destroy) g_gpu_vtable.pipeline_destroy(p); }
JiGpuRenderPass* ji_gpu_render_pass_create(JiGpuDevice* d, const JiGpuFormat* cf, uint32_t cc, JiGpuFormat df, JiGpuLoadOp l, JiGpuStoreOp s) { return d && g_gpu_vtable.render_pass_create ? g_gpu_vtable.render_pass_create(d, cf, cc, df, l, s) : NULL; }
void ji_gpu_render_pass_destroy(JiGpuRenderPass* p) { if (p && g_gpu_vtable.render_pass_destroy) g_gpu_vtable.render_pass_destroy(p); }
JiGpuFramebuffer* ji_gpu_framebuffer_create(JiGpuDevice* d, JiGpuRenderPass* p, JiGpuTextureView** c, uint32_t cc, JiGpuTextureView* dp, uint32_t w, uint32_t h) { return d && g_gpu_vtable.framebuffer_create ? g_gpu_vtable.framebuffer_create(d, p, c, cc, dp, w, h) : NULL; }
void ji_gpu_framebuffer_destroy(JiGpuFramebuffer* f) { if (f && g_gpu_vtable.framebuffer_destroy) g_gpu_vtable.framebuffer_destroy(f); }

/* Sync */
JiGpuFence* ji_gpu_fence_create(JiGpuDevice* d, bool s) { return d && g_gpu_vtable.fence_create ? g_gpu_vtable.fence_create(d, s) : NULL; }
void ji_gpu_fence_destroy(JiGpuFence* f) { if (f && g_gpu_vtable.fence_destroy) g_gpu_vtable.fence_destroy(f); }
void ji_gpu_fence_reset(JiGpuFence* f) { if (f && g_gpu_vtable.fence_reset) g_gpu_vtable.fence_reset(f); }
bool ji_gpu_fence_wait(JiGpuFence* f, uint64_t t) { return f && g_gpu_vtable.fence_wait ? g_gpu_vtable.fence_wait(f, t) : false; }
bool ji_gpu_fence_is_signaled(const JiGpuFence* f) { return f && g_gpu_vtable.fence_is_signaled ? g_gpu_vtable.fence_is_signaled(f) : false; }
JiGpuSemaphore* ji_gpu_semaphore_create(JiGpuDevice* d) { return d && g_gpu_vtable.semaphore_create ? g_gpu_vtable.semaphore_create(d) : NULL; }
void ji_gpu_semaphore_destroy(JiGpuSemaphore* s) { if (s && g_gpu_vtable.semaphore_destroy) g_gpu_vtable.semaphore_destroy(s); }

/* Command Recording */
void ji_gpu_cmd_begin_render_pass(JiGpuCommandBuffer* c, JiGpuRenderPass* p, JiGpuFramebuffer* f, uint32_t w, uint32_t h, const JiGpuColorClearValue* cc, uint32_t ccc, const JiGpuDepthStencilClearValue* dc) { if (c && g_gpu_vtable.cmd_begin_render_pass) g_gpu_vtable.cmd_begin_render_pass(c, p, f, w, h, cc, ccc, dc); }
void ji_gpu_cmd_end_render_pass(JiGpuCommandBuffer* c) { if (c && g_gpu_vtable.cmd_end_render_pass) g_gpu_vtable.cmd_end_render_pass(c); }
void ji_gpu_cmd_bind_pipeline(JiGpuCommandBuffer* c, JiGpuPipeline* p) { if (c && g_gpu_vtable.cmd_bind_pipeline) g_gpu_vtable.cmd_bind_pipeline(c, p); }
void ji_gpu_cmd_bind_descriptor_sets(JiGpuCommandBuffer* c, JiGpuPipelineLayout* l, uint32_t fs, JiGpuDescriptorSet** s, uint32_t n) { if (c && g_gpu_vtable.cmd_bind_descriptor_sets) g_gpu_vtable.cmd_bind_descriptor_sets(c, l, fs, s, n); }
void ji_gpu_cmd_bind_vertex_buffer(JiGpuCommandBuffer* c, uint32_t b, JiGpuBuffer* buf, uint64_t o) { if (c && g_gpu_vtable.cmd_bind_vertex_buffer) g_gpu_vtable.cmd_bind_vertex_buffer(c, b, buf, o); }
void ji_gpu_cmd_bind_index_buffer(JiGpuCommandBuffer* c, JiGpuBuffer* buf, uint64_t o, JiGpuFormat f) { if (c && g_gpu_vtable.cmd_bind_index_buffer) g_gpu_vtable.cmd_bind_index_buffer(c, buf, o, f); }
void ji_gpu_cmd_push_constants(JiGpuCommandBuffer* c, JiGpuPipelineLayout* l, JiGpuShaderStage s, uint32_t o, uint32_t sz, const void* d) { if (c && g_gpu_vtable.cmd_push_constants) g_gpu_vtable.cmd_push_constants(c, l, s, o, sz, d); }
void ji_gpu_cmd_set_viewport(JiGpuCommandBuffer* c, const JiGpuViewport* v) { if (c && g_gpu_vtable.cmd_set_viewport) g_gpu_vtable.cmd_set_viewport(c, v); }
void ji_gpu_cmd_set_scissor(JiGpuCommandBuffer* c, const JiGpuScissor* s) { if (c && g_gpu_vtable.cmd_set_scissor) g_gpu_vtable.cmd_set_scissor(c, s); }
void ji_gpu_cmd_draw(JiGpuCommandBuffer* c, uint32_t v, uint32_t i, uint32_t fv, uint32_t fi) { if (c && g_gpu_vtable.cmd_draw) g_gpu_vtable.cmd_draw(c, v, i, fv, fi); }
void ji_gpu_cmd_draw_indexed(JiGpuCommandBuffer* c, uint32_t ic, uint32_t ins, uint32_t fi, int32_t vo, uint32_t fis) { if (c && g_gpu_vtable.cmd_draw_indexed) g_gpu_vtable.cmd_draw_indexed(c, ic, ins, fi, vo, fis); }
