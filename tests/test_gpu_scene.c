/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * GPU Backend & Scene Graph Tests
 */

#include "jiui/jiui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { printf("  TEST: %-50s", #name); } while(0)

#define PASS() \
    do { printf("  PASS\n"); tests_passed++; } while(0)

#define FAIL(msg) \
    do { printf("  FAIL: %s\n", msg); tests_failed++; } while(0)

#define ASSERT(cond, msg) \
    do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* ---- GPU Backend Tests ---- */

static void test_gpu_backend_names(void) {
    TEST(gpu_backend_names);
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_VULKAN), "Vulkan") == 0, "Vulkan name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_OPENGL), "OpenGL") == 0, "OpenGL name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_SOFTWARE), "Software") == 0, "Software name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_METAL), "Metal") == 0, "Metal name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_WEBGPU), "WebGPU") == 0, "WebGPU name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_D3D11), "D3D11") == 0, "D3D11 name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_D3D12), "D3D12") == 0, "D3D12 name mismatch");
    ASSERT(strcmp(ji_gpu_backend_name(JI_GPU_BACKEND_COUNT), "Unknown") == 0, "Unknown name mismatch");
    PASS();
}

static void test_gpu_format_block_size(void) {
    TEST(gpu_format_block_size);
    ASSERT(ji_gpu_format_block_size(JI_GPU_FORMAT_R8G8B8A8_UNORM) == 4, "RGBA8 should be 4 bytes");
    ASSERT(ji_gpu_format_block_size(JI_GPU_FORMAT_R32G32B32A32_SFLOAT) == 16, "RGBA32F should be 16 bytes");
    ASSERT(ji_gpu_format_block_size(JI_GPU_FORMAT_R16G16B16A16_SFLOAT) == 8, "RGBA16F should be 8 bytes");
    ASSERT(ji_gpu_format_block_size(JI_GPU_FORMAT_R8_UNORM) == 1, "R8 should be 1 byte");
    ASSERT(ji_gpu_format_block_size(JI_GPU_FORMAT_D32_SFLOAT) == 4, "D32 should be 4 bytes");
    PASS();
}

static void test_gpu_format_depth_stencil(void) {
    TEST(gpu_format_depth_stencil);
    ASSERT(ji_gpu_format_has_depth(JI_GPU_FORMAT_D24_UNORM_S8_UINT), "D24S8 has depth");
    ASSERT(ji_gpu_format_has_depth(JI_GPU_FORMAT_D32_SFLOAT), "D32 has depth");
    ASSERT(!ji_gpu_format_has_depth(JI_GPU_FORMAT_R8G8B8A8_UNORM), "RGBA8 has no depth");
    ASSERT(ji_gpu_format_has_stencil(JI_GPU_FORMAT_D24_UNORM_S8_UINT), "D24S8 has stencil");
    ASSERT(!ji_gpu_format_has_stencil(JI_GPU_FORMAT_D32_SFLOAT), "D32 has no stencil");
    PASS();
}

static void test_gpu_format_compressed(void) {
    TEST(gpu_format_compressed);
    ASSERT(ji_gpu_format_is_compressed(JI_GPU_FORMAT_BC1_RGB_UNORM_BLOCK), "BC1 is compressed");
    ASSERT(ji_gpu_format_is_compressed(JI_GPU_FORMAT_BC7_UNORM_BLOCK), "BC7 is compressed");
    ASSERT(!ji_gpu_format_is_compressed(JI_GPU_FORMAT_R8G8B8A8_UNORM), "RGBA8 is not compressed");
    PASS();
}

static void test_gpu_software_device(void) {
    TEST(gpu_software_device_create_destroy);
    JiGpuDeviceOptions opts = {
        .preferred_backend = JI_GPU_SOFTWARE,
        .enable_validation = false,
        .app_name = "JiUI Test",
        .app_version = 1,
        .engine_name = "JiUI",
        .engine_version = 1,
    };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Software device should be created");
    if (dev) {
        const JiGpuCaps* caps = ji_gpu_device_get_caps(dev);
        ASSERT(caps != NULL, "Caps should not be NULL");
        ASSERT(caps->backend == JI_GPU_SOFTWARE, "Backend should be Software");
        ASSERT(strcmp(caps->device_name, "JiUI Software Renderer") == 0, "Device name mismatch");
        ji_gpu_device_destroy(dev);
    }
    PASS();
}

static void test_gpu_software_buffer(void) {
    TEST(gpu_software_buffer_create_map_upload);
    JiGpuDeviceOptions opts = { .preferred_backend = JI_GPU_SOFTWARE };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Device needed for buffer test");
    if (!dev) return;

    JiGpuBuffer* buf = ji_gpu_buffer_create(dev, 1024, JI_GPU_BUFFER_USAGE_VERTEX);
    ASSERT(buf != NULL, "Buffer should be created");
    ASSERT(ji_gpu_buffer_get_size(buf) == 1024, "Buffer size should be 1024");

    if (buf) {
        void* mapped = ji_gpu_buffer_map(buf);
        ASSERT(mapped != NULL, "Buffer should be mappable");
        if (mapped) {
            memset(mapped, 0xAB, 1024);
            ji_gpu_buffer_unmap(buf);
        }

        /* Test upload */
        uint32_t data = 0xDEADBEEF;
        ji_gpu_buffer_upload(buf, &data, sizeof(data), 0);
        ji_gpu_buffer_destroy(buf);
    }
    ji_gpu_device_destroy(dev);
    PASS();
}

static void test_gpu_software_texture(void) {
    TEST(gpu_software_texture_create_upload);
    JiGpuDeviceOptions opts = { .preferred_backend = JI_GPU_SOFTWARE };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Device needed for texture test");
    if (!dev) return;

    JiGpuTexture* tex = ji_gpu_texture_create(dev, 256, 256,
        JI_GPU_FORMAT_R8G8B8A8_UNORM, JI_GPU_TEXTURE_USAGE_SAMPLED);
    ASSERT(tex != NULL, "Texture should be created");
    if (tex) {
        ASSERT(ji_gpu_texture_get_width(tex) == 256, "Width should be 256");
        ASSERT(ji_gpu_texture_get_height(tex) == 256, "Height should be 256");
        ASSERT(ji_gpu_texture_get_format(tex) == JI_GPU_FORMAT_R8G8B8A8_UNORM, "Format mismatch");

        uint32_t pixels[64];
        memset(pixels, 0xFF, sizeof(pixels));
        ji_gpu_texture_upload(tex, pixels, 8, 8, 8 * 4);
        ji_gpu_texture_destroy(tex);
    }
    ji_gpu_device_destroy(dev);
    PASS();
}

static void test_gpu_software_swapchain(void) {
    TEST(gpu_software_swapchain_create_present);
    JiGpuDeviceOptions opts = { .preferred_backend = JI_GPU_SOFTWARE };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Device needed for swapchain test");
    if (!dev) return;

    JiGpuSwapchain* sc = ji_gpu_swapchain_create(dev, 800, 600);
    ASSERT(sc != NULL, "Swapchain should be created");
    if (sc) {
        ASSERT(ji_gpu_swapchain_get_image_count(sc) == 2, "Should have 2 images");
        ASSERT(ji_gpu_swapchain_get_format(sc) == JI_GPU_FORMAT_B8G8R8A8_UNORM, "Format mismatch");

        uint32_t idx = ji_gpu_swapchain_acquire_next_image(sc);
        ASSERT(idx < 2, "Image index should be < 2");

        bool presented = ji_gpu_swapchain_present(sc, idx);
        ASSERT(presented, "Present should succeed");

        ji_gpu_swapchain_destroy(sc);
    }
    ji_gpu_device_destroy(dev);
    PASS();
}

static void test_gpu_software_sync(void) {
    TEST(gpu_software_fence_semaphore);
    JiGpuDeviceOptions opts = { .preferred_backend = JI_GPU_SOFTWARE };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Device needed for sync test");
    if (!dev) return;

    JiGpuFence* fence = ji_gpu_fence_create(dev, false);
    ASSERT(fence != NULL, "Fence should be created");
    if (fence) {
        ASSERT(!ji_gpu_fence_is_signaled(fence), "Fence should not be signaled");
        ji_gpu_fence_reset(fence);
        ji_gpu_fence_wait(fence, 0); /* unsignaled, should return false */
        ji_gpu_fence_destroy(fence);
    }

    JiGpuFence* signaled = ji_gpu_fence_create(dev, true);
    ASSERT(signaled != NULL, "Signaled fence should be created");
    if (signaled) {
        ASSERT(ji_gpu_fence_is_signaled(signaled), "Fence should be signaled");
        ji_gpu_fence_destroy(signaled);
    }

    JiGpuSemaphore* sem = ji_gpu_semaphore_create(dev);
    ASSERT(sem != NULL, "Semaphore should be created");
    if (sem) ji_gpu_semaphore_destroy(sem);

    ji_gpu_device_destroy(dev);
    PASS();
}

/* ---- Scene Graph Tests ---- */

static void test_scene_node_create_destroy(void) {
    TEST(scene_node_create_destroy);
    JiSceneNode* node = ji_scene_node_create(JI_SCENE_NODE_WIDGET);
    ASSERT(node != NULL, "Node should be created");
    if (node) {
        ASSERT(node->type == JI_SCENE_NODE_WIDGET, "Type should be WIDGET");
        ASSERT(node->visible == true, "Should be visible by default");
        ASSERT(node->opacity == 1.0f, "Opacity should be 1.0");
        ASSERT(node->dirty == false, "Should not be dirty initially");
        ASSERT(node->child_count == 0, "Should have no children");
        ji_scene_node_destroy(node);
    }
    PASS();
}

static void test_scene_node_hierarchy(void) {
    TEST(scene_node_hierarchy);
    JiSceneNode* root = ji_scene_node_create(JI_SCENE_NODE_ROOT);
    JiSceneNode* child1 = ji_scene_node_create(JI_SCENE_NODE_WIDGET);
    JiSceneNode* child2 = ji_scene_node_create(JI_SCENE_NODE_RENDERABLE);
    JiSceneNode* grandchild = ji_scene_node_create(JI_SCENE_NODE_TEXT);

    ASSERT(root && child1 && child2 && grandchild, "All nodes should be created");

    ji_scene_node_add_child(root, child1);
    ji_scene_node_add_child(root, child2);
    ji_scene_node_add_child(child1, grandchild);

    ASSERT(root->child_count == 2, "Root should have 2 children");
    ASSERT(child1->child_count == 1, "Child1 should have 1 child");
    ASSERT(child1->parent == root, "Child1 parent should be root");
    ASSERT(grandchild->parent == child1, "Grandchild parent should be child1");

    /* Remove child */
    ji_scene_node_remove_child(root, child1);
    ASSERT(root->child_count == 1, "Root should have 1 child after remove");
    ASSERT(child1->parent == NULL, "Removed child should have no parent");

    ji_scene_node_destroy(root);
    ji_scene_node_destroy(child1); /* includes grandchild */
    PASS();
}

static void test_scene_node_dirty_propagation(void) {
    TEST(scene_node_dirty_propagation);
    JiSceneNode* root = ji_scene_node_create(JI_SCENE_NODE_ROOT);
    JiSceneNode* child = ji_scene_node_create(JI_SCENE_NODE_WIDGET);
    JiSceneNode* grandchild = ji_scene_node_create(JI_SCENE_NODE_TEXT);

    ji_scene_node_add_child(root, child);
    ji_scene_node_add_child(child, grandchild);

    /* Mark grandchild dirty - should propagate up */
    ji_scene_node_mark_dirty(grandchild);
    ASSERT(grandchild->dirty == true, "Grandchild should be dirty");
    ASSERT(child->dirty == true, "Child should be dirty (propagated)");
    ASSERT(root->dirty == true, "Root should be dirty (propagated)");

    ji_scene_node_destroy(root);
    PASS();
}

static void test_scene_node_properties(void) {
    TEST(scene_node_properties);
    JiSceneNode* node = ji_scene_node_create(JI_SCENE_NODE_WIDGET);

    ji_scene_node_set_bounds(node, (JiRectF){10, 20, 100, 200});
    ASSERT(node->bounds.x == 10 && node->bounds.y == 20, "Bounds position");
    ASSERT(node->bounds.width == 100 && node->bounds.height == 200, "Bounds size");

    ji_scene_node_set_opacity(node, 0.5f);
    ASSERT(fabs(node->opacity - 0.5f) < 0.001f, "Opacity should be 0.5");

    ji_scene_node_set_opacity(node, -1.0f);
    ASSERT(node->opacity == 0.0f, "Opacity should clamp to 0");

    ji_scene_node_set_opacity(node, 2.0f);
    ASSERT(node->opacity == 1.0f, "Opacity should clamp to 1");

    ji_scene_node_set_visible(node, false);
    ASSERT(node->visible == false, "Should be invisible");

    ji_scene_node_destroy(node);
    PASS();
}

static void test_scene_graph_create(void) {
    TEST(scene_graph_create);
    JiGpuDeviceOptions opts = { .preferred_backend = JI_GPU_SOFTWARE };
    JiGpuDevice* dev = ji_gpu_device_create(&opts);
    ASSERT(dev != NULL, "Device needed for scene graph");
    if (!dev) return;

    JiSceneGraph* graph = ji_scene_graph_create(dev);
    ASSERT(graph != NULL, "Scene graph should be created");
    if (graph) {
        JiSceneNode* root = ji_scene_graph_get_root(graph);
        ASSERT(root != NULL, "Root should exist");
        ASSERT(root->type == JI_SCENE_NODE_ROOT, "Root type");

        /* Add some nodes */
        JiSceneNode* n1 = ji_scene_node_create(JI_SCENE_NODE_WIDGET);
        JiSceneNode* n2 = ji_scene_node_create(JI_SCENE_NODE_TEXT);
        JiSceneNode* n3 = ji_scene_node_create(JI_SCENE_NODE_IMAGE);
        ji_scene_node_add_child(root, n1);
        ji_scene_node_add_child(root, n2);
        ji_scene_node_add_child(n1, n3);

        /* Collect dirty */
        ji_scene_graph_collect_dirty(graph);
        JiSceneStats stats = ji_scene_graph_get_stats(graph);
        ASSERT(stats.total_nodes == 4, "Should have 4 nodes total");
        ASSERT(stats.visible_nodes == 4, "All should be visible");

        ji_scene_graph_destroy(graph);
    }
    ji_gpu_device_destroy(dev);
    PASS();
}

static void test_scene_transform2d(void) {
    TEST(scene_transform2d);
    JiTransform2D identity = ji_transform2d_identity();
    ASSERT(identity.m[0][0] == 1.0f && identity.m[1][1] == 1.0f && identity.m[2][2] == 1.0f,
           "Identity diagonal should be 1");
    ASSERT(identity.m[0][1] == 0.0f && identity.m[0][2] == 0.0f,
           "Identity off-diagonal should be 0");

    JiTransform2D translate = ji_transform2d_translate(100, 200);
    ASSERT(translate.m[0][2] == 100.0f && translate.m[1][2] == 200.0f,
           "Translation values");

    JiTransform2D scale = ji_transform2d_scale(2, 3);
    ASSERT(scale.m[0][0] == 2.0f && scale.m[1][1] == 3.0f,
           "Scale values");

    JiTransform2D rot = ji_transform2d_rotate(1.57079632679f); /* pi/2 */
    ASSERT(fabs(rot.m[0][0]) < 0.0001f, "Rotation cos(pi/2) ~ 0");
    ASSERT(fabs(rot.m[0][1] - (-1.0f)) < 0.0001f, "Rotation -sin(pi/2) ~ -1");

    /* Multiply: translate * scale */
    JiTransform2D combined = ji_transform2d_multiply(translate, scale);
    ASSERT(fabs(combined.m[0][0] - 2.0f) < 0.001f, "Combined scale X");
    ASSERT(fabs(combined.m[1][1] - 3.0f) < 0.001f, "Combined scale Y");
    ASSERT(fabs(combined.m[0][2] - 100.0f) < 0.001f, "Combined translate X");

    PASS();
}

static void traverse_counter(JiSceneNode* node, void* user_data) {
    (void)node;
    int* count = (int*)user_data;
    (*count)++;
}

static void test_scene_node_traverse(void) {
    TEST(scene_node_traverse);
    JiSceneNode* root = ji_scene_node_create(JI_SCENE_NODE_ROOT);
    JiSceneNode* c1 = ji_scene_node_create(JI_SCENE_NODE_WIDGET);
    JiSceneNode* c2 = ji_scene_node_create(JI_SCENE_NODE_TEXT);
    ji_scene_node_add_child(root, c1);
    ji_scene_node_add_child(root, c2);

    int count = 0;
    ji_scene_node_traverse(root, traverse_counter, &count);
    ASSERT(count == 3, "Should traverse 3 nodes (root + 2 children)");

    /* Test find with user_data */
    c1->user_data = (void*)0x1234;
    JiSceneNode* found = ji_scene_node_find(root, (void*)0x1234);
    ASSERT(found == c1, "Should find child1 by user_data");

    ji_scene_node_destroy(root);
    PASS();
}

/* ---- Main ---- */

int main(void) {
    printf("\n=== JiUI GPU & Scene Graph Tests ===\n\n");

    /* GPU Backend */
    test_gpu_backend_names();
    test_gpu_format_block_size();
    test_gpu_format_depth_stencil();
    test_gpu_format_compressed();
    test_gpu_software_device();
    test_gpu_software_buffer();
    test_gpu_software_texture();
    test_gpu_software_swapchain();
    test_gpu_software_sync();

    /* Scene Graph */
    test_scene_node_create_destroy();
    test_scene_node_hierarchy();
    test_scene_node_dirty_propagation();
    test_scene_node_properties();
    test_scene_graph_create();
    test_scene_transform2d();
    test_scene_node_traverse();

    printf("\n=== Results: %d passed, %d failed ===\n\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
