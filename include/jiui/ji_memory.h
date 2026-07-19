/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_memory.h
 * @brief Memory management, reference counting, and allocation utilities.
 */

#ifndef JIUI_MEMORY_H
#define JIUI_MEMORY_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Allocation functions
 * ========================================================================= */

/** Allocate `size` bytes. Returns NULL on failure. */
JI_API void* ji_alloc(size_t size);

/** Allocate `count * size` bytes, zero-initialized. Returns NULL on failure. */
JI_API void* ji_calloc(size_t count, size_t size);

/** Reallocate `ptr` to `size` bytes. Returns NULL on failure. */
JI_API void* ji_realloc(void* ptr, size_t size);

/** Free a pointer allocated by ji_alloc/ji_calloc/ji_realloc. */
JI_API void  ji_free(void* ptr);

/** Allocate and zero-fill (convenience for structs). */
#define JI_NEW(Type) ((Type*)ji_calloc(1, sizeof(Type)))

/** Allocate array of count elements. */
#define JI_NEW_ARRAY(Type, count) ((Type*)ji_calloc((count), sizeof(Type)))

/** Free and null the pointer. */
#define JI_FREE(ptr) do { ji_free(ptr); (ptr) = NULL; } while(0)

/* =========================================================================
 * Reference counting
 * ========================================================================= */

/** Atomic reference count stored inside ref-counted objects.
 *  In C, _Atomic ensures atomic access. In C++, the atomicity
 *  is handled by the implementation using compiler builtins. */
typedef struct JiRefCount {
    int count;  /* accessed atomically via __atomic builtins */
} JiRefCount;

/** Initialize a ref count to 1. */
JI_API void ji_ref_init(JiRefCount* rc);

/** Increment ref count. Returns new count. */
JI_API int  ji_ref_acquire(JiRefCount* rc);

/** Decrement ref count. Returns new count (0 = object should be freed). */
JI_API int  ji_ref_release(JiRefCount* rc);

/** Get current ref count. */
JI_API int  ji_ref_count(const JiRefCount* rc);

/* =========================================================================
 * Ref-counted object base
 *
 * All ref-counted JiUI objects embed JiRefCount as the first member.
 * The JiRef<T> smart-pointer pattern manages acquire/release automatically.
 * ========================================================================= */

/** Function pointer type for destroying a ref-counted object. */
typedef void (*JiDestroyFunc)(void* obj);

/** Base header for ref-counted objects. */
typedef struct JiRefObject {
    JiRefCount     ref_count;
    JiDestroyFunc  destroy;
} JiRefObject;

/** Initialize a ref-counted object. */
JI_API void ji_ref_object_init(JiRefObject* obj, JiDestroyFunc destroy);

/** Acquire a reference to a ref-counted object. */
JI_API void* ji_ref_object_acquire(void* obj);

/** Release a reference; calls destroy when count reaches 0. */
JI_API void  ji_ref_object_release(void* obj);

/** Get the current reference count. */
JI_API int   ji_ref_object_count(const void* obj);

/* =========================================================================
 * C++ smart-pointer (RAII) for ref-counted objects
 * ========================================================================= */
#ifdef __cplusplus
} /* close extern "C" — C++ templates cannot be inside extern "C" */

#include <cstddef>

/** RAII wrapper for JiRefObject pointers. */
template <typename T>
class JiRef {
public:
    JiRef() : m_ptr(nullptr) {}

    explicit JiRef(T* ptr) : m_ptr(ptr) {
        if (m_ptr) ji_ref_object_acquire(m_ptr);
    }

    JiRef(const JiRef& other) : m_ptr(other.m_ptr) {
        if (m_ptr) ji_ref_object_acquire(m_ptr);
    }

    JiRef(JiRef&& other) noexcept : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }

    ~JiRef() {
        if (m_ptr) ji_ref_object_release(m_ptr);
    }

    JiRef& operator=(const JiRef& other) {
        if (this != &other) {
            if (m_ptr) ji_ref_object_release(m_ptr);
            m_ptr = other.m_ptr;
            if (m_ptr) ji_ref_object_acquire(m_ptr);
        }
        return *this;
    }

    JiRef& operator=(JiRef&& other) noexcept {
        if (this != &other) {
            if (m_ptr) ji_ref_object_release(m_ptr);
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

    T* get() const { return m_ptr; }
    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    explicit operator bool() const { return m_ptr != nullptr; }

    void reset() {
        if (m_ptr) {
            ji_ref_object_release(m_ptr);
            m_ptr = nullptr;
        }
    }

    T* detach() {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

    bool operator==(const JiRef& other) const { return m_ptr == other.m_ptr; }
    bool operator!=(const JiRef& other) const { return m_ptr != other.m_ptr; }
    bool operator==(std::nullptr_t) const { return m_ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return m_ptr != nullptr; }

private:
    T* m_ptr;
};

extern "C" { /* reopen extern "C" for remaining declarations */
#endif /* __cplusplus */

/* =========================================================================
 * Memory pool (simple arena allocator for UI nodes)
 * ========================================================================= */

typedef struct JiMemoryPool JiMemoryPool;

/** Create a memory pool with the given block size. */
JI_API JiMemoryPool* ji_pool_create(size_t block_size);

/** Allocate from the pool. */
JI_API void* ji_pool_alloc(JiMemoryPool* pool, size_t size);

/** Reset the pool (keeps allocated blocks, marks them reusable). */
JI_API void ji_pool_reset(JiMemoryPool* pool);

/** Destroy the pool and free all blocks. */
JI_API void ji_pool_destroy(JiMemoryPool* pool);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_MEMORY_H */
