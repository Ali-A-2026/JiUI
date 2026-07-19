/**
 * JiUI - GPU Internal Header
 * Shared between GPU backend implementations
 */

#ifndef JIUI_GPU_INTERNAL_H
#define JIUI_GPU_INTERNAL_H

#include "jiui/ji_gpu.h"
#include "jiui/ji_memory.h"

struct JiGpuDevice {
    JiGpuBackend backend;
    JiGpuCaps    caps;
    void*        handle;
    void*        physical_device;
    void*        instance;
    void*        queue;
    uint32_t     queue_family;
    void*        allocator;
    bool         validation_enabled;
};

struct JiGpuBuffer {
    JiGpuDevice*     device;
    void*             handle;
    void*             memory;
    uint64_t          size;
    JiGpuBufferUsage  usage;
    bool              mapped;
    void*             mapped_ptr;
};

struct JiGpuTexture {
    JiGpuDevice*      device;
    void*              handle;
    void*              memory;
    void*              view;
    uint32_t           width;
    uint32_t           height;
    JiGpuFormat        format;
    JiGpuTextureUsage  usage;
};

struct JiGpuSwapchain {
    JiGpuDevice* device;
    void*         handle;
    uint32_t      width;
    uint32_t      height;
    uint32_t      image_count;
    JiGpuTexture** images;
    JiGpuFormat   format;
    uint32_t      current_image;
};

struct JiGpuCommandBuffer {
    JiGpuDevice* device;
    void*         handle;
    void*         pool;
    bool          recording;
};

struct JiGpuShader {
    JiGpuDevice*     device;
    JiGpuShaderStage stage;
    void*             handle;
};

struct JiGpuPipeline {
    JiGpuDevice* device;
    void*         handle;
    void*         layout;
};

struct JiGpuPipelineLayout {
    JiGpuDevice* device;
    void*         handle;
};

struct JiGpuDescriptorSetLayout {
    JiGpuDevice* device;
    void*         handle;
};

struct JiGpuDescriptorSet {
    JiGpuDevice* device;
    void*         handle;
};

struct JiGpuRenderPass {
    JiGpuDevice* device;
    void*         handle;
};

struct JiGpuFramebuffer {
    JiGpuDevice* device;
    void*         handle;
    uint32_t      width;
    uint32_t      height;
};

struct JiGpuFence {
    JiGpuDevice* device;
    void*         handle;
    bool          signaled;
};

struct JiGpuSemaphore {
    JiGpuDevice* device;
    void*         handle;
};

struct JiGpuTextureView {
    JiGpuTexture* texture;
    void*          handle;
    JiGpuFormat    format;
};

struct JiGpuSampler {
    JiGpuDevice* device;
    void*         handle;
};

/* Backend vtable */
typedef struct JiGpuBackendVtable JiGpuBackendVtable;

struct JiGpuBackendVtable {
    JiGpuDevice*    (*device_create)(const JiGpuDeviceOptions* options);
    void            (*device_destroy)(JiGpuDevice* device);
    void            (*device_wait_idle)(JiGpuDevice* device);
    JiGpuSwapchain* (*swapchain_create)(JiGpuDevice* device, uint32_t w, uint32_t h);
    void            (*swapchain_destroy)(JiGpuSwapchain* sc);
    bool            (*swapchain_resize)(JiGpuSwapchain* sc, uint32_t w, uint32_t h);
    uint32_t        (*swapchain_acquire)(JiGpuSwapchain* sc);
    bool            (*swapchain_present)(JiGpuSwapchain* sc, uint32_t idx);
    JiGpuTexture*   (*swapchain_get_image)(JiGpuSwapchain* sc, uint32_t idx);
    JiGpuCommandBuffer* (*cmd_create)(JiGpuDevice* dev);
    void            (*cmd_destroy)(JiGpuCommandBuffer* cmd);
    void            (*cmd_begin)(JiGpuCommandBuffer* cmd);
    void            (*cmd_end)(JiGpuCommandBuffer* cmd);
    void            (*cmd_reset)(JiGpuCommandBuffer* cmd);
    void            (*cmd_submit)(JiGpuCommandBuffer* cmd, JiGpuFence* fence);
    JiGpuBuffer*   (*buffer_create)(JiGpuDevice* dev, uint64_t size, JiGpuBufferUsage usage);
    void           (*buffer_destroy)(JiGpuBuffer* buf);
    void*          (*buffer_map)(JiGpuBuffer* buf);
    void           (*buffer_unmap)(JiGpuBuffer* buf);
    void           (*buffer_upload)(JiGpuBuffer* buf, const void* data, uint64_t size, uint64_t offset);
    JiGpuTexture*  (*texture_create)(JiGpuDevice* dev, uint32_t w, uint32_t h, JiGpuFormat fmt, JiGpuTextureUsage usage);
    void           (*texture_destroy)(JiGpuTexture* tex);
    void           (*texture_upload)(JiGpuTexture* tex, const void* data, uint32_t w, uint32_t h, uint32_t pitch);
    JiGpuTextureView* (*texture_view_create)(JiGpuTexture* tex, JiGpuFormat fmt);
    void           (*texture_view_destroy)(JiGpuTextureView* view);
    JiGpuSampler*  (*sampler_create)(JiGpuDevice* dev, JiGpuFilter mag, JiGpuFilter min, JiGpuSamplerAddressMode u, JiGpuSamplerAddressMode v, float max_aniso);
    void           (*sampler_destroy)(JiGpuSampler* samp);
    JiGpuShader*   (*shader_create_spirv)(JiGpuDevice* dev, JiGpuShaderStage stage, const uint32_t* code, size_t size);
    JiGpuShader*   (*shader_create_glsl)(JiGpuDevice* dev, JiGpuShaderStage stage, const char* src);
    void           (*shader_destroy)(JiGpuShader* sh);
    JiGpuDescriptorSetLayout* (*ds_layout_create)(JiGpuDevice* dev, const JiGpuDescriptorSetLayoutBinding* bindings, uint32_t count);
    void           (*ds_layout_destroy)(JiGpuDescriptorSetLayout* layout);
    JiGpuDescriptorSet* (*ds_create)(JiGpuDevice* dev, JiGpuDescriptorSetLayout* layout);
    void           (*ds_destroy)(JiGpuDescriptorSet* set);
    void           (*ds_update)(JiGpuDevice* dev, const JiGpuWriteDescriptorSet* writes, uint32_t count);
    JiGpuPipelineLayout* (*pipeline_layout_create)(JiGpuDevice* dev, JiGpuDescriptorSetLayout** layouts, uint32_t count, const void* pc, uint32_t pc_count);
    void           (*pipeline_layout_destroy)(JiGpuPipelineLayout* layout);
    JiGpuPipeline* (*pipeline_create)(JiGpuDevice* dev, const JiGpuPipelineCreateInfo* info);
    void           (*pipeline_destroy)(JiGpuPipeline* pipe);
    JiGpuRenderPass* (*render_pass_create)(JiGpuDevice* dev, const JiGpuFormat* cf, uint32_t cc, JiGpuFormat df, JiGpuLoadOp l, JiGpuStoreOp s);
    void           (*render_pass_destroy)(JiGpuRenderPass* pass);
    JiGpuFramebuffer* (*framebuffer_create)(JiGpuDevice* dev, JiGpuRenderPass* pass, JiGpuTextureView** colors, uint32_t cc, JiGpuTextureView* depth, uint32_t w, uint32_t h);
    void           (*framebuffer_destroy)(JiGpuFramebuffer* fb);
    JiGpuFence*    (*fence_create)(JiGpuDevice* dev, bool signaled);
    void           (*fence_destroy)(JiGpuFence* fence);
    void           (*fence_reset)(JiGpuFence* fence);
    bool           (*fence_wait)(JiGpuFence* fence, uint64_t timeout_ns);
    bool           (*fence_is_signaled)(const JiGpuFence* fence);
    JiGpuSemaphore* (*semaphore_create)(JiGpuDevice* dev);
    void           (*semaphore_destroy)(JiGpuSemaphore* sem);
    void (*cmd_begin_render_pass)(JiGpuCommandBuffer*, JiGpuRenderPass*, JiGpuFramebuffer*, uint32_t, uint32_t, const JiGpuColorClearValue*, uint32_t, const JiGpuDepthStencilClearValue*);
    void (*cmd_end_render_pass)(JiGpuCommandBuffer*);
    void (*cmd_bind_pipeline)(JiGpuCommandBuffer*, JiGpuPipeline*);
    void (*cmd_bind_descriptor_sets)(JiGpuCommandBuffer*, JiGpuPipelineLayout*, uint32_t, JiGpuDescriptorSet**, uint32_t);
    void (*cmd_bind_vertex_buffer)(JiGpuCommandBuffer*, uint32_t, JiGpuBuffer*, uint64_t);
    void (*cmd_bind_index_buffer)(JiGpuCommandBuffer*, JiGpuBuffer*, uint64_t, JiGpuFormat);
    void (*cmd_push_constants)(JiGpuCommandBuffer*, JiGpuPipelineLayout*, JiGpuShaderStage, uint32_t, uint32_t, const void*);
    void (*cmd_set_viewport)(JiGpuCommandBuffer*, const JiGpuViewport*);
    void (*cmd_set_scissor)(JiGpuCommandBuffer*, const JiGpuScissor*);
    void (*cmd_draw)(JiGpuCommandBuffer*, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*cmd_draw_indexed)(JiGpuCommandBuffer*, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);
};

/* Global vtable (defined in ji_gpu.c) */
extern JiGpuBackendVtable g_gpu_vtable;

#endif /* JIUI_GPU_INTERNAL_H */
