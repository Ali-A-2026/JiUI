/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_defines.h
 * @brief Fundamental macros, attributes, and compile-time definitions.
 */

#ifndef JIUI_DEFINES_H
#define JIUI_DEFINES_H

#include <jiui/ji_config.h>

/* --------------------------------------------------------------------------
 * Compiler detection
 * -------------------------------------------------------------------------- */
#if defined(_MSC_VER)
#  define JI_COMPILER_MSVC 1
#elif defined(__GNUC__)
#  define JI_COMPILER_GCC 1
#elif defined(__clang__)
#  define JI_COMPILER_CLANG 1
#endif

/* --------------------------------------------------------------------------
 * Platform detection
 * -------------------------------------------------------------------------- */
#if defined(_WIN32) || defined(_WIN64)
#  define JI_PLATFORM_WINDOWS 1
#  define JI_PLATFORM_NAME "Windows"
#elif defined(__linux__)
#  define JI_PLATFORM_LINUX 1
#  define JI_PLATFORM_NAME "Linux"
#elif defined(__APPLE__)
#  include <TargetConditionals.h>
#  if TARGET_OS_MAC
#    define JI_PLATFORM_MACOS 1
#    define JI_PLATFORM_NAME "macOS"
#  elif TARGET_OS_IPHONE
#    define JI_PLATFORM_IOS 1
#    define JI_PLATFORM_NAME "iOS"
#  endif
#elif defined(__ANDROID__)
#  define JI_PLATFORM_ANDROID 1
#  define JI_PLATFORM_NAME "Android"
#endif

/* --------------------------------------------------------------------------
 * Architecture detection
 * -------------------------------------------------------------------------- */
#if defined(_M_X64) || defined(__x86_64__)
#  define JI_ARCH_X64 1
#  define JI_ARCH_NAME "x86_64"
#elif defined(_M_IX86) || defined(__i386__)
#  define JI_ARCH_X86 1
#  define JI_ARCH_NAME "x86"
#elif defined(_M_ARM64) || defined(__aarch64__)
#  define JI_ARCH_ARM64 1
#  define JI_ARCH_NAME "arm64"
#elif defined(_M_ARM) || defined(__arm__)
#  define JI_ARCH_ARM 1
#  define JI_ARCH_NAME "arm"
#endif

/* --------------------------------------------------------------------------
 * Inline hints
 * -------------------------------------------------------------------------- */
#ifdef JI_COMPILER_MSVC
#  define JI_INLINE __forceinline
#  define JI_NOINLINE __declspec(noinline)
#else
#  define JI_INLINE inline __attribute__((always_inline))
#  define JI_NOINLINE __attribute__((noinline))
#endif

/* --------------------------------------------------------------------------
 * Branch prediction hints
 * -------------------------------------------------------------------------- */
#if defined(JI_COMPILER_GCC) || defined(JI_COMPILER_CLANG)
#  define JI_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define JI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define JI_LIKELY(x)   (x)
#  define JI_UNLIKELY(x) (x)
#endif

/* --------------------------------------------------------------------------
 * Unused variable marker
 * -------------------------------------------------------------------------- */
#define JI_UNUSED(x) ((void)(x))

/* --------------------------------------------------------------------------
 * Array size macro
 * -------------------------------------------------------------------------- */
#ifdef __cplusplus
#include <cstddef>
template <typename T, size_t N>
constexpr size_t ji_array_size(T (&)[N]) { return N; }
#endif

#define JI_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* --------------------------------------------------------------------------
 * Min / Max (type-safe, macro-free when C++)
 * -------------------------------------------------------------------------- */
#define JI_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JI_MAX(a, b) ((a) > (b) ? (a) : (b))
#define JI_CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))

/* --------------------------------------------------------------------------
 * Stringify helpers
 * -------------------------------------------------------------------------- */
#define JI_STRINGIFY_(x) #x
#define JI_STRINGIFY(x)  JI_STRINGIFY_(x)

/* --------------------------------------------------------------------------
 * Concatenation helpers
 * -------------------------------------------------------------------------- */
#define JI_CONCAT_(a, b) a##b
#define JI_CONCAT(a, b)   JI_CONCAT_(a, b)

/* --------------------------------------------------------------------------
 * Bit flag helpers
 * -------------------------------------------------------------------------- */
#define JI_BIT(n)       (1 << (n))
#define JI_FLAG_SET(v, f)   ((v) |= (f))
#define JI_FLAG_CLEAR(v, f) ((v) &= ~(f))
#define JI_FLAG_TOGGLE(v, f) ((v) ^= (f))
#define JI_FLAG_HAS(v, f)   (((v) & (f)) == (f))

/* --------------------------------------------------------------------------
 * Deletion macros (C++ only)
 * -------------------------------------------------------------------------- */
#ifdef __cplusplus
#  define JI_DISABLE_COPY(Class)       \
      Class(const Class&) = delete;     \
      Class& operator=(const Class&) = delete
#  define JI_DISABLE_MOVE(Class)       \
      Class(Class&&) = delete;          \
      Class& operator=(Class&&) = delete
#  define JI_DISABLE_COPY_AND_MOVE(Class) \
      JI_DISABLE_COPY(Class);             \
      JI_DISABLE_MOVE(Class)
#endif

/* --------------------------------------------------------------------------
 * Nullptr compatibility (C)
 * -------------------------------------------------------------------------- */
#if !defined(__cplusplus) && !defined(nullptr)
    /* C23 natively supports nullptr as a keyword */
#   if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
        /* C23 or later - nullptr is a keyword, do not redefine */
#   else
#       define nullptr ((void*)0)
#   endif
#endif

/* --------------------------------------------------------------------------
 * Deprecation macros (Phase 10.4 — Production Stability)
 * -------------------------------------------------------------------------- */
#if defined(JI_COMPILER_GCC) || defined(JI_COMPILER_CLANG)
#  define JI_DEPRECATED(msg) __attribute__((deprecated(msg)))
#  define JI_DEPRECATED_EXPORT(msg) __attribute__((deprecated(msg), visibility("default")))
#  define JI_DEPRECATED_ENUM(msg) __attribute__((deprecated(msg)))
#elif defined(JI_COMPILER_MSVC)
#  define JI_DEPRECATED(msg) __declspec(deprecated(msg))
#  define JI_DEPRECATED_EXPORT(msg) __declspec(deprecated(msg))
#  define JI_DEPRECATED_ENUM(msg)
#else
#  define JI_DEPRECATED(msg)
#  define JI_DEPRECATED_EXPORT(msg)
#  define JI_DEPRECATED_ENUM(msg)
#endif

/* --------------------------------------------------------------------------
 * ABI stability markers (Phase 10.4)
 * -------------------------------------------------------------------------- */
#define JIUI_ABI_VERSION_MAJOR 1
#define JIUI_ABI_VERSION_MINOR 0
#define JIUI_ABI_VERSION_PATCH 0
#define JIUI_ABI_VERSION_STRING "1.0.0"

/* JI_ABI_STABLE: mark a struct/function as having a frozen ABI layout.
 * Future changes to such types must maintain binary compatibility. */
#define JI_ABI_STABLE

/* JI_ABI_UNSTABLE: mark a struct/function whose layout may change between
 * minor versions. Users should not rely on the binary layout. */
#define JI_ABI_UNSTABLE

/* JI_API_EXPORT: visibility macro for public API symbols (alias for JI_API) */
#define JI_API_EXPORT JI_API

/* JI_API_LOCAL: hidden visibility for internal symbols */
#if defined(JI_COMPILER_GCC) || defined(JI_COMPILER_CLANG)
#  define JI_API_LOCAL __attribute__((visibility("hidden")))
#else
#  define JI_API_LOCAL
#endif

#endif /* JIUI_DEFINES_H */
