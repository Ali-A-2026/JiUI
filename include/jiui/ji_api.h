/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_api.h
 * @brief Public API export macros for dynamic linking.
 *
 * All public JiUI symbols are tagged with JI_API. When building the shared
 * library JIUI_BUILDING_DLL is defined, causing JI_API to expand to
 * __declspec(dllexport) on Windows or __attribute__((visibility("default")))
 * on GCC/Clang.  Consumers link against the import library / shared object
 * and see __declspec(dllimport) or the default visibility respectively.
 *
 * For static builds JI_API expands to nothing.
 */

#ifndef JIUI_API_H
#define JIUI_API_H

/* --------------------------------------------------------------------------
 * Shared-library export / import
 * -------------------------------------------------------------------------- */
#if defined(JIUI_SHARED)

#  if defined(_WIN32)
#    ifdef JIUI_BUILDING_DLL
#      define JI_API __declspec(dllexport)
#    else
#      define JI_API __declspec(dllimport)
#    endif
#  else  /* non-Windows shared */
#    define JI_API __attribute__((visibility("default")))
#  endif

#else  /* JIUI_STATIC or no define */
#  define JI_API
#endif

/* --------------------------------------------------------------------------
 * Convenience: call convention (Windows)
 * -------------------------------------------------------------------------- */
#ifdef _WIN32
#  define JI_CALL __stdcall
#else
#  define JI_CALL
#endif

/* --------------------------------------------------------------------------
 * Convenience: extern "C" wrapper for C++ consumers
 * -------------------------------------------------------------------------- */
#ifdef __cplusplus
#  define JI_EXTERN_C extern "C"
#else
#  define JI_EXTERN_C extern
#endif

/* --------------------------------------------------------------------------
 * Public API declaration helper
 * -------------------------------------------------------------------------- */
#define JI_PUBLIC  JI_EXTERN_C JI_API
#define JI_PRIVATE /* internal, not exported */

#endif /* JIUI_API_H */
