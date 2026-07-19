/**
 * JiUI - GPU Backend Abstraction
 * Cross-platform GPU abstraction: Vulkan, Metal, D3D11, D3D12, OpenGL, WebGPU, Software
 */

#ifndef JIUI_GPU_H
#define JIUI_GPU_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * GPU Backend Types
 * ========================================================================= */
typedef enum JiGpuBackend {
    JI_GPU_VULKAN = 0,
    JI_GPU_METAL,
    JI_GPU_D3D11,
    JI_GPU_D3D12,
    JI_GPU_OPENGL,
    JI_GPU_WEBGPU,
    JI_GPU_SOFTWARE,
    JI_GPU_BACKEND_COUNT
} JiGpuBackend;

typedef enum JiGpuFormat {
    JI_GPU_FORMAT_UNDEFINED = 0,
    JI_GPU_FORMAT_R8G8B8A8_UNORM,
    JI_GPU_FORMAT_B8G8R8A8_UNORM,
    JI_GPU_FORMAT_R8G8B8A8_SRGB,
    JI_GPU_FORMAT_B8G8R8A8_SRGB,
    JI_GPU_FORMAT_R32G32B32A32_SFLOAT,
    JI_GPU_FORMAT_R16G16B16A16_SFLOAT,
    JI_GPU_FORMAT_R8_UNORM,
    JI_GPU_FORMAT_D24_UNORM_S8_UINT,
    JI_GPU_FORMAT_D32_SFLOAT,
    JI_GPU_FORMAT_D24_UNORM,
    JI_GPU_FORMAT_BC1_RGB_UNORM_BLOCK,
    JI_GPU_FORMAT_BC2_UNORM_BLOCK,
    JI_GPU_FORMAT_BC3_UNORM_BLOCK,
    JI_GPU_FORMAT_BC4_UNORM_BLOCK,
    JI_GPU_FORMAT_BC5_UNORM_BLOCK,
    JI_GPU_FORMAT_BC7_UNORM_BLOCK,
} JiGpuFormat;

typedef enum JiGpuBufferUsage {
    JI_GPU_BUFFER_USAGE_VERTEX    = 1 << 0,
    JI_GPU_BUFFER_USAGE_INDEX     = 1 << 1,
    JI_GPU_BUFFER_USAGE_UNIFORM   = 1 << 2,
    JI_GPU_BUFFER_USAGE_STORAGE   = 1 << 3,
    JI_GPU_BUFFER_USAGE_TRANSFER_SRC = 1 << 4,
    JI_GPU_BUFFER_USAGE_TRANSFER_DST = 1 << 5,
} JiGpuBufferUsage;

typedef enum JiGpuTextureUsage {
    JI_GPU_TEXTURE_USAGE_SAMPLED       = 1 << 0,
    JI_GPU_TEXTURE_USAGE_COLOR_TARGET  = 1 << 1,
    JI_GPU_TEXTURE_USAGE_DEPTH_TARGET  = 1 << 2,
    JI_GPU_TEXTURE_USAGE_STORAGE       = 1 << 3,
    JI_GPU_TEXTURE_USAGE_TRANSFER_SRC  = 1 << 4,
    JI_GPU_TEXTURE_USAGE_TRANSFER_DST  = 1 << 5,
} JiGpuTextureUsage;

typedef enum JiGpuShaderStage {
    JI_GPU_SHADER_VERTEX   = 1 << 0,
    JI_GPU_SHADER_FRAGMENT = 1 << 1,
    JI_GPU_SHADER_COMPUTE  = 1 << 2,
} JiGpuShaderStage;

typedef enum JiGpuPrimitiveTopology {
    JI_GPU_PRIM_POINT_LIST,
    JI_GPU_PRIM_LINE_LIST,
    JI_GPU_PRIM_LINE_STRIP,
    JI_GPU_PRIM_TRIANGLE_LIST,
    JI_GPU_PRIM_TRIANGLE_STRIP,
    JI_GPU_PRIM_TRIANGLE_FAN,
} JiGpuPrimitiveTopology;

typedef enum JiGpuCompareOp {
    JI_GPU_COMPARE_NEVER,
    JI_GPU_COMPARE_LESS,
    JI_GPU_COMPARE_EQUAL,
    JI_GPU_COMPARE_LESS_EQUAL,
    JI_GPU_COMPARE_GREATER,
    JI_GPU_COMPARE_NOT_EQUAL,
    JI_GPU_COMPARE_GREATER_EQUAL,
    JI_GPU_COMPARE_ALWAYS,
} JiGpuCompareOp;

typedef enum JiGpuBlendFactor {
    JI_GPU_BLEND_ZERO,
    JI_GPU_BLEND_ONE,
    JI_GPU_BLEND_SRC_COLOR,
    JI_GPU_BLEND_ONE_MINUS_SRC_COLOR,
    JI_GPU_BLEND_DST_COLOR,
    JI_GPU_BLEND_ONE_MINUS_DST_COLOR,
    JI_GPU_BLEND_SRC_ALPHA,
    JI_GPU_BLEND_ONE_MINUS_SRC_ALPHA,
    JI_GPU_BLEND_DST_ALPHA,
    JI_GPU_BLEND_ONE_MINUS_DST_ALPHA,
} JiGpuBlendFactor;

typedef enum JiGpuBlendOp {
    JI_GPU_BLEND_OP_ADD,
    JI_GPU_BLEND_OP_SUBTRACT,
    JI_GPU_BLEND_OP_REVERSE_SUBTRACT,
    JI_GPU_BLEND_OP_MIN,
    JI_GPU_BLEND_OP_MAX,
} JiGpuBlendOp;

typedef enum JiGpuLoadOp {
    JI_GPU_LOAD_OP_LOAD,
    JI_GPU_LOAD_OP_CLEAR,
    JI_GPU_LOAD_OP_DONT_CARE,
} JiGpuLoadOp;

typedef enum JiGpuStoreOp {
    JI_GPU_STORE_OP_STORE,
    JI_GPU_STORE_OP_DONT_CARE,
} JiGpuStoreOp;

typedef enum JiGpuFilter {
    JI_GPU_FILTER_NEAREST,
    JI_GPU_FILTER_LINEAR,
} JiGpuFilter;

typedef enum JiGpuSamplerAddressMode {
    JI_GPU_SAMPLER_REPEAT,
    JI_GPU_SAMPLER_MIRRORED_REPEAT,
    JI_GPU_SAMPLER_CLAMP_TO_EDGE,
    JI_GPU_SAMPLER_CLAMP_TO_BORDER,
} JiGpuSamplerAddressMode;

/* =========================================================================
 * GPU Capability Query
 * ========================================================================= */
typedef struct JiGpuCaps {
    JiGpuBackend backend;
    char         device_name[256];
    char         driver_version[128];
    uint32_t     vendor_id;
    uint32_t     device_id;
    uint64_t     vram_bytes;
    uint32_t     max_texture_size;
    uint32_t     max_texture_layers;
    uint32_t     max_color_attachments;
    uint32_t     max_vertex_attributes;
    uint32_t     max_uniform_buffer_range;
    uint32_t     max_storage_buffer_range;
    uint32_t     min_uniform_buffer_offset_alignment;
    uint32_t     min_storage_buffer_offset_alignment;
    uint32_t     max_push_constants_size;
    bool         supports_compute;
    bool         supports_geometry;
    bool         supports_tessellation;
    bool         supports_multiview;
    bool         supports_bc_compression;
    bool         supports_etc_compression;
    bool         supports_astc_compression;
    float        max_anisotropy;
} JiGpuCaps;

/* =========================================================================
 * GPU Objects (opaque handles)
 * ========================================================================= */
typedef struct JiGpuDevice JiGpuDevice;
typedef struct JiGpuSwapchain JiGpuSwapchain;
typedef struct JiGpuCommandBuffer JiGpuCommandBuffer;
typedef struct JiGpuBuffer JiGpuBuffer;
typedef struct JiGpuTexture JiGpuTexture;
typedef struct JiGpuTextureView JiGpuTextureView;
typedef struct JiGpuSampler JiGpuSampler;
typedef struct JiGpuShader JiGpuShader;
typedef struct JiGpuPipeline JiGpuPipeline;
typedef struct JiGpuPipelineLayout JiGpuPipelineLayout;
typedef struct JiGpuDescriptorSet JiGpuDescriptorSet;
typedef struct JiGpuDescriptorSetLayout JiGpuDescriptorSetLayout;
typedef struct JiGpuRenderPass JiGpuRenderPass;
typedef struct JiGpuFramebuffer JiGpuFramebuffer;
typedef struct JiGpuFence JiGpuFence;
typedef struct JiGpuSemaphore JiGpuSemaphore;

/* =========================================================================
 * Descriptor Types
 * ========================================================================= */
typedef enum JiGpuDescriptorType {
    JI_GPU_DESC_SAMPLER,
    JI_GPU_DESC_COMBINED_IMAGE_SAMPLER,
    JI_GPU_DESC_SAMPLED_IMAGE,
    JI_GPU_DESC_STORAGE_IMAGE,
    JI_GPU_DESC_UNIFORM_BUFFER,
    JI_GPU_DESC_STORAGE_BUFFER,
    JI_GPU_DESC_UNIFORM_BUFFER_DYNAMIC,
    JI_GPU_DESC_STORAGE_BUFFER_DYNAMIC,
} JiGpuDescriptorType;

typedef struct JiGpuDescriptorSetLayoutBinding {
    uint32_t             binding;
    JiGpuDescriptorType  descriptor_type;
    JiGpuShaderStage     stage_flags;
    uint32_t             descriptor_count;
} JiGpuDescriptorSetLayoutBinding;

typedef struct JiGpuDescriptorImageInfo {
    JiGpuSampler*     sampler;
    JiGpuTextureView* image_view;
    JiGpuShaderStage  image_layout;
} JiGpuDescriptorImageInfo;

typedef struct JiGpuDescriptorBufferInfo {
    JiGpuBuffer* buffer;
    uint64_t     offset;
    uint64_t     range;
} JiGpuDescriptorBufferInfo;

typedef struct JiGpuWriteDescriptorSet {
    JiGpuDescriptorSet*      dst_set;
    uint32_t                 dst_binding;
    uint32_t                 dst_array_element;
    JiGpuDescriptorType      descriptor_type;
    uint32_t                 descriptor_count;
    JiGpuDescriptorImageInfo*  image_info;
    JiGpuDescriptorBufferInfo* buffer_info;
} JiGpuWriteDescriptorSet;

/* =========================================================================
 * Pipeline State Descriptors
 * ========================================================================= */
typedef struct JiGpuBlendAttachmentState {
    bool             blend_enable;
    JiGpuBlendFactor src_color_blend_factor;
    JiGpuBlendFactor dst_color_blend_factor;
    JiGpuBlendOp     color_blend_op;
    JiGpuBlendFactor src_alpha_blend_factor;
    JiGpuBlendFactor dst_alpha_blend_factor;
    JiGpuBlendOp     alpha_blend_op;
    uint8_t          color_write_mask; /* R=1, G=2, B=4, A=8 */
} JiGpuBlendAttachmentState;

typedef struct JiGpuBlendState {
    bool                          logic_op_enable;
    JiGpuBlendAttachmentState     attachments[8];
    uint32_t                      attachment_count;
} JiGpuBlendState;

typedef struct JiGpuRasterizerState {
    bool     depth_clamp_enable;
    bool     rasterizer_discard_enable;
    float    line_width;
    bool     depth_bias_enable;
    float    depth_bias_constant_factor;
    float    depth_bias_clamp;
    float    depth_bias_slope_factor;
} JiGpuRasterizerState;

typedef struct JiGpuDepthState {
    bool           depth_test_enable;
    bool           depth_write_enable;
    JiGpuCompareOp depth_compare_op;
    bool           depth_bounds_test_enable;
    float          min_depth_bounds;
    float          max_depth_bounds;
    bool           stencil_test_enable;
} JiGpuDepthState;

typedef struct JiGpuVertexAttribute {
    uint32_t     location;
    uint32_t     binding;
    JiGpuFormat  format;
    uint32_t     offset;
} JiGpuVertexAttribute;

typedef struct JiGpuVertexBinding {
    uint32_t binding;
    uint32_t stride;
    bool     per_instance;
} JiGpuVertexBinding;

typedef struct JiGpuVertexInputState {
    JiGpuVertexAttribute* attributes;
    uint32_t              attribute_count;
    JiGpuVertexBinding*   bindings;
    uint32_t              binding_count;
} JiGpuVertexInputState;

typedef struct JiGpuViewport {
    float x, y;
    float width, height;
    float min_depth, max_depth;
} JiGpuViewport;

typedef struct JiGpuRect2D {
    int32_t x, y;
    uint32_t width, height;
} JiGpuRect2D;

typedef struct JiGpuScissor {
    JiGpuRect2D rect;
} JiGpuScissor;

typedef struct JiGpuColorClearValue {
    float r, g, b, a;
} JiGpuColorClearValue;

typedef struct JiGpuDepthStencilClearValue {
    float    depth;
    uint32_t stencil;
} JiGpuDepthStencilClearValue;

/* =========================================================================
 * Pipeline Create Info
 * ========================================================================= */
typedef struct JiGpuPipelineCreateInfo {
    JiGpuShader*           vertex_shader;
    JiGpuShader*           fragment_shader;
    JiGpuPipelineLayout*   layout;
    JiGpuRenderPass*       render_pass;
    uint32_t               subpass;
    JiGpuVertexInputState  vertex_input;
    JiGpuPrimitiveTopology  topology;
    JiGpuBlendState         blend_state;
    JiGpuRasterizerState    rasterizer_state;
    JiGpuDepthState         depth_state;
    bool                    dynamic_viewport;
    bool                    dynamic_scissor;
} JiGpuPipelineCreateInfo;

/* =========================================================================
 * Device Creation Options
 * ========================================================================= */
typedef struct JiGpuDeviceOptions {
    JiGpuBackend preferred_backend;
    bool         enable_validation;
    bool         enable_debug_markers;
    bool         prefer_discrete_gpu;
    const char*  app_name;
    uint32_t     app_version;
    const char*  engine_name;
    uint32_t     engine_version;
    void*        window_handle;     /* platform-specific */
    void*        display_handle;    /* platform-specific */
    uint32_t     initial_width;
    uint32_t     initial_height;
} JiGpuDeviceOptions;

/* =========================================================================
 * GPU Device API
 * ========================================================================= */

/** Create a GPU device with the given options */
JI_API JiGpuDevice* ji_gpu_device_create(const JiGpuDeviceOptions* options);

/** Destroy a GPU device and all associated resources */
JI_API void ji_gpu_device_destroy(JiGpuDevice* device);

/** Get device capabilities */
JI_API const JiGpuCaps* ji_gpu_device_get_caps(const JiGpuDevice* device);

/** Wait for all GPU operations to complete */
JI_API void ji_gpu_device_wait_idle(JiGpuDevice* device);

/* ---- Swapchain ---- */
JI_API JiGpuSwapchain* ji_gpu_swapchain_create(JiGpuDevice* device, uint32_t width, uint32_t height);
JI_API void ji_gpu_swapchain_destroy(JiGpuSwapchain* swapchain);
JI_API bool ji_gpu_swapchain_resize(JiGpuSwapchain* swapchain, uint32_t width, uint32_t height);
JI_API uint32_t ji_gpu_swapchain_acquire_next_image(JiGpuSwapchain* swapchain);
JI_API bool ji_gpu_swapchain_present(JiGpuSwapchain* swapchain, uint32_t image_index);
JI_API JiGpuTexture* ji_gpu_swapchain_get_image(JiGpuSwapchain* swapchain, uint32_t index);
JI_API uint32_t ji_gpu_swapchain_get_image_count(const JiGpuSwapchain* swapchain);
JI_API JiGpuFormat ji_gpu_swapchain_get_format(const JiGpuSwapchain* swapchain);

/* ---- Command Buffers ---- */
JI_API JiGpuCommandBuffer* ji_gpu_command_buffer_create(JiGpuDevice* device);
JI_API void ji_gpu_command_buffer_destroy(JiGpuCommandBuffer* cmd);
JI_API void ji_gpu_command_buffer_begin(JiGpuCommandBuffer* cmd);
JI_API void ji_gpu_command_buffer_end(JiGpuCommandBuffer* cmd);
JI_API void ji_gpu_command_buffer_reset(JiGpuCommandBuffer* cmd);
JI_API void ji_gpu_command_buffer_submit(JiGpuCommandBuffer* cmd, JiGpuFence* fence);

/* ---- Buffers ---- */
JI_API JiGpuBuffer* ji_gpu_buffer_create(JiGpuDevice* device, uint64_t size, JiGpuBufferUsage usage);
JI_API void ji_gpu_buffer_destroy(JiGpuBuffer* buffer);
JI_API void* ji_gpu_buffer_map(JiGpuBuffer* buffer);
JI_API void ji_gpu_buffer_unmap(JiGpuBuffer* buffer);
JI_API void ji_gpu_buffer_upload(JiGpuBuffer* buffer, const void* data, uint64_t size, uint64_t offset);
JI_API uint64_t ji_gpu_buffer_get_size(const JiGpuBuffer* buffer);

/* ---- Textures ---- */
JI_API JiGpuTexture* ji_gpu_texture_create(JiGpuDevice* device, uint32_t width, uint32_t height,
                                            JiGpuFormat format, JiGpuTextureUsage usage);
JI_API void ji_gpu_texture_destroy(JiGpuTexture* texture);
JI_API void ji_gpu_texture_upload(JiGpuTexture* texture, const void* data,
                                   uint32_t width, uint32_t height, uint32_t row_pitch);
JI_API uint32_t ji_gpu_texture_get_width(const JiGpuTexture* texture);
JI_API uint32_t ji_gpu_texture_get_height(const JiGpuTexture* texture);
JI_API JiGpuFormat ji_gpu_texture_get_format(const JiGpuTexture* texture);

/* ---- Texture Views ---- */
JI_API JiGpuTextureView* ji_gpu_texture_view_create(JiGpuTexture* texture, JiGpuFormat format);
JI_API void ji_gpu_texture_view_destroy(JiGpuTextureView* view);

/* ---- Samplers ---- */
JI_API JiGpuSampler* ji_gpu_sampler_create(JiGpuDevice* device, JiGpuFilter mag_filter,
                                             JiGpuFilter min_filter, JiGpuSamplerAddressMode address_u,
                                             JiGpuSamplerAddressMode address_v, float max_anisotropy);
JI_API void ji_gpu_sampler_destroy(JiGpuSampler* sampler);

/* ---- Shaders ---- */
JI_API JiGpuShader* ji_gpu_shader_create_from_spirv(JiGpuDevice* device, JiGpuShaderStage stage,
                                                      const uint32_t* spirv_code, size_t code_size_bytes);
JI_API JiGpuShader* ji_gpu_shader_create_from_glsl(JiGpuDevice* device, JiGpuShaderStage stage,
                                                     const char* source);
JI_API void ji_gpu_shader_destroy(JiGpuShader* shader);

/* ---- Descriptor Sets ---- */
JI_API JiGpuDescriptorSetLayout* ji_gpu_descriptor_set_layout_create(
    JiGpuDevice* device, const JiGpuDescriptorSetLayoutBinding* bindings, uint32_t binding_count);
JI_API void ji_gpu_descriptor_set_layout_destroy(JiGpuDescriptorSetLayout* layout);
JI_API JiGpuDescriptorSet* ji_gpu_descriptor_set_create(
    JiGpuDevice* device, JiGpuDescriptorSetLayout* layout);
JI_API void ji_gpu_descriptor_set_destroy(JiGpuDescriptorSet* set);
JI_API void ji_gpu_descriptor_set_update(JiGpuDevice* device,
    const JiGpuWriteDescriptorSet* writes, uint32_t write_count);

/* ---- Pipeline Layout ---- */
JI_API JiGpuPipelineLayout* ji_gpu_pipeline_layout_create(
    JiGpuDevice* device, JiGpuDescriptorSetLayout** set_layouts, uint32_t set_layout_count,
    const void* push_constant_ranges, uint32_t push_constant_range_count);
JI_API void ji_gpu_pipeline_layout_destroy(JiGpuPipelineLayout* layout);

/* ---- Pipelines ---- */
JI_API JiGpuPipeline* ji_gpu_pipeline_create(JiGpuDevice* device,
                                                const JiGpuPipelineCreateInfo* info);
JI_API void ji_gpu_pipeline_destroy(JiGpuPipeline* pipeline);

/* ---- Render Pass ---- */
JI_API JiGpuRenderPass* ji_gpu_render_pass_create(JiGpuDevice* device,
    const JiGpuFormat* color_formats, uint32_t color_count,
    JiGpuFormat depth_format, JiGpuLoadOp load_op, JiGpuStoreOp store_op);
JI_API void ji_gpu_render_pass_destroy(JiGpuRenderPass* pass);

/* ---- Framebuffers ---- */
JI_API JiGpuFramebuffer* ji_gpu_framebuffer_create(JiGpuDevice* device,
    JiGpuRenderPass* pass, JiGpuTextureView** color_attachments, uint32_t color_count,
    JiGpuTextureView* depth_attachment, uint32_t width, uint32_t height);
JI_API void ji_gpu_framebuffer_destroy(JiGpuFramebuffer* fb);

/* ---- Synchronization ---- */
JI_API JiGpuFence* ji_gpu_fence_create(JiGpuDevice* device, bool signaled);
JI_API void ji_gpu_fence_destroy(JiGpuFence* fence);
JI_API void ji_gpu_fence_reset(JiGpuFence* fence);
JI_API bool ji_gpu_fence_wait(JiGpuFence* fence, uint64_t timeout_ns);
JI_API bool ji_gpu_fence_is_signaled(const JiGpuFence* fence);

JI_API JiGpuSemaphore* ji_gpu_semaphore_create(JiGpuDevice* device);
JI_API void ji_gpu_semaphore_destroy(JiGpuSemaphore* sem);

/* =========================================================================
 * Command Buffer Recording
 * ========================================================================= */
JI_API void ji_gpu_cmd_begin_render_pass(JiGpuCommandBuffer* cmd, JiGpuRenderPass* pass,
    JiGpuFramebuffer* fb, uint32_t width, uint32_t height,
    const JiGpuColorClearValue* color_clears, uint32_t color_clear_count,
    const JiGpuDepthStencilClearValue* depth_clear);
JI_API void ji_gpu_cmd_end_render_pass(JiGpuCommandBuffer* cmd);

JI_API void ji_gpu_cmd_bind_pipeline(JiGpuCommandBuffer* cmd, JiGpuPipeline* pipeline);
JI_API void ji_gpu_cmd_bind_descriptor_sets(JiGpuCommandBuffer* cmd,
    JiGpuPipelineLayout* layout, uint32_t first_set,
    JiGpuDescriptorSet** sets, uint32_t set_count);
JI_API void ji_gpu_cmd_bind_vertex_buffer(JiGpuCommandBuffer* cmd, uint32_t binding,
    JiGpuBuffer* buffer, uint64_t offset);
JI_API void ji_gpu_cmd_bind_index_buffer(JiGpuCommandBuffer* cmd, JiGpuBuffer* buffer,
    uint64_t offset, JiGpuFormat index_format);
JI_API void ji_gpu_cmd_push_constants(JiGpuCommandBuffer* cmd, JiGpuPipelineLayout* layout,
    JiGpuShaderStage stage_flags, uint32_t offset, uint32_t size, const void* data);

JI_API void ji_gpu_cmd_set_viewport(JiGpuCommandBuffer* cmd, const JiGpuViewport* viewport);
JI_API void ji_gpu_cmd_set_scissor(JiGpuCommandBuffer* cmd, const JiGpuScissor* scissor);

JI_API void ji_gpu_cmd_draw(JiGpuCommandBuffer* cmd, uint32_t vertex_count,
                              uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
JI_API void ji_gpu_cmd_draw_indexed(JiGpuCommandBuffer* cmd, uint32_t index_count,
                                      uint32_t instance_count, uint32_t first_index,
                                      int32_t vertex_offset, uint32_t first_instance);

/* =========================================================================
 * Utility
 * ========================================================================= */
JI_API const char* ji_gpu_backend_name(JiGpuBackend backend);
JI_API uint32_t ji_gpu_format_block_size(JiGpuFormat format);
JI_API bool ji_gpu_format_has_depth(JiGpuFormat format);
JI_API bool ji_gpu_format_has_stencil(JiGpuFormat format);
JI_API bool ji_gpu_format_is_compressed(JiGpuFormat format);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_GPU_H */
