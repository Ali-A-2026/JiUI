/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_memory.c
 * @brief Implementation of memory management, reference counting, and pools.
 */

#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Standard allocation wrappers
 * ========================================================================= */

void* ji_alloc(size_t size) {
    if (JI_UNLIKELY(size == 0)) return NULL;
    void* ptr = malloc(size);
    if (JI_UNLIKELY(!ptr)) {
        JI_FATAL("Out of memory: requested %zu bytes", size);
    }
    return ptr;
}

void* ji_calloc(size_t count, size_t size) {
    if (JI_UNLIKELY(count == 0 || size == 0)) return NULL;
    void* ptr = calloc(count, size);
    if (JI_UNLIKELY(!ptr)) {
        JI_FATAL("Out of memory: requested %zu x %zu bytes", count, size);
    }
    return ptr;
}

void* ji_realloc(void* ptr, size_t size) {
    if (JI_UNLIKELY(size == 0)) {
        free(ptr);
        return NULL;
    }
    void* new_ptr = realloc(ptr, size);
    if (JI_UNLIKELY(!new_ptr)) {
        JI_FATAL("Out of memory: realloc %zu bytes", size);
    }
    return new_ptr;
}

void ji_free(void* ptr) {
    free(ptr);
}

/* =========================================================================
 * Atomic reference counting
 * ========================================================================= */

void ji_ref_init(JiRefCount* rc) {
    rc->count = 1;
}

int ji_ref_acquire(JiRefCount* rc) {
    return __atomic_add_fetch(&rc->count, 1, __ATOMIC_SEQ_CST);
}

int ji_ref_release(JiRefCount* rc) {
    return __atomic_sub_fetch(&rc->count, 1, __ATOMIC_SEQ_CST);
}

int ji_ref_count(const JiRefCount* rc) {
    return __atomic_load_n(&rc->count, __ATOMIC_SEQ_CST);
}

/* =========================================================================
 * Ref-counted object base
 * ========================================================================= */

void ji_ref_object_init(JiRefObject* obj, JiDestroyFunc destroy) {
    ji_ref_init(&obj->ref_count);
    obj->destroy = destroy;
}

void* ji_ref_object_acquire(void* obj) {
    if (!obj) return NULL;
    JiRefObject* ref_obj = (JiRefObject*)obj;
    ji_ref_acquire(&ref_obj->ref_count);
    return obj;
}

void ji_ref_object_release(void* obj) {
    if (!obj) return;
    JiRefObject* ref_obj = (JiRefObject*)obj;
    int count = ji_ref_release(&ref_obj->ref_count);
    if (count <= 0) {
        if (ref_obj->destroy) {
            ref_obj->destroy(obj);
        } else {
            ji_free(obj);
        }
    }
}

int ji_ref_object_count(const void* obj) {
    if (!obj) return 0;
    const JiRefObject* ref_obj = (const JiRefObject*)obj;
    return ji_ref_count(&ref_obj->ref_count);
}

/* =========================================================================
 * Memory pool (arena allocator)
 * ========================================================================= */

#define JI_POOL_DEFAULT_BLOCK_SIZE (64 * 1024) /* 64 KB */

typedef struct JiPoolBlock {
    struct JiPoolBlock* next;
    size_t               capacity;
    size_t               used;
    unsigned char        data[];
} JiPoolBlock;

struct JiMemoryPool {
    JiPoolBlock* first;
    JiPoolBlock* current;
    size_t       block_size;
};

static JiPoolBlock* ji_pool_block_create(size_t size) {
    JiPoolBlock* block = (JiPoolBlock*)ji_alloc(sizeof(JiPoolBlock) + size);
    if (!block) return NULL;
    block->next     = NULL;
    block->capacity = size;
    block->used     = 0;
    return block;
}

JiMemoryPool* ji_pool_create(size_t block_size) {
    if (block_size == 0) block_size = JI_POOL_DEFAULT_BLOCK_SIZE;
    JiMemoryPool* pool = JI_NEW(JiMemoryPool);
    if (!pool) return NULL;
    pool->block_size = block_size;
    pool->first = ji_pool_block_create(block_size);
    if (!pool->first) {
        ji_free(pool);
        return NULL;
    }
    pool->current = pool->first;
    return pool;
}

void* ji_pool_alloc(JiMemoryPool* pool, size_t size) {
    if (!pool || !pool->current) return NULL;

    /* Align to 8 bytes */
    size = (size + 7) & ~(size_t)7;

    JiPoolBlock* block = pool->current;
    while (block) {
        if (block->used + size <= block->capacity) {
            void* ptr = block->data + block->used;
            block->used += size;
            return ptr;
        }
        if (!block->next) {
            size_t new_size = size > pool->block_size ? size : pool->block_size;
            block->next = ji_pool_block_create(new_size);
            if (!block->next) return NULL;
        }
        block = block->next;
    }
    pool->current = block;
    if (block && block->used + size <= block->capacity) {
        void* ptr = block->data + block->used;
        block->used += size;
        return ptr;
    }
    return NULL;
}

void ji_pool_reset(JiMemoryPool* pool) {
    if (!pool) return;
    JiPoolBlock* block = pool->first;
    while (block) {
        block->used = 0;
        block = block->next;
    }
    pool->current = pool->first;
}

void ji_pool_destroy(JiMemoryPool* pool) {
    if (!pool) return;
    JiPoolBlock* block = pool->first;
    while (block) {
        JiPoolBlock* next = block->next;
        ji_free(block);
        block = next;
    }
    ji_free(pool);
}
