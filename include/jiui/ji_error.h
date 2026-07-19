/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_error.h
 * @brief Error handling infrastructure — result codes, error objects, logging.
 */

#ifndef JIUI_ERROR_H
#define JIUI_ERROR_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Result codes
 * ========================================================================= */
typedef enum JiResultCode {
    JI_OK                  =  0,
    JI_ERROR_UNKNOWN       = -1,
    JI_ERROR_OUT_OF_MEMORY = -2,
    JI_ERROR_INVALID_ARG   = -3,
    JI_ERROR_NULL_PTR      = -4,
    JI_ERROR_NOT_FOUND     = -5,
    JI_ERROR_ALREADY_EXISTS= -6,
    JI_ERROR_IO            = -7,
    JI_ERROR_PARSE         = -8,
    JI_ERROR_TYPE_MISMATCH = -9,
    JI_ERROR_STATE         = -10,
    JI_ERROR_NOT_SUPPORTED = -11,
    JI_ERROR_TIMEOUT       = -12,
    JI_ERROR_CANCELLED     = -13,
    JI_ERROR_PERMISSION    = -14,
    JI_ERROR_PLATFORM      = -15,
    JI_ERROR_RENDER        = -16,
    JI_ERROR_LAYOUT        = -17,
} JiResultCode;

/** Check if a result code indicates success. */
#define JI_SUCCEEDED(r)  ((r) >= 0)
/** Check if a result code indicates failure. */
#define JI_FAILED(r)     ((r) < 0)

/* =========================================================================
 * JiError — detailed error information
 * ========================================================================= */
typedef struct JiError JiError;

struct JiError {
    JiResultCode  code;
    char*         message;     /* heap-allocated, must be freed */
    char*         source_file;
    int           source_line;
    char*         function;
    JiError*      cause;       /* chained inner error */
};

/** Create an error with a formatted message. */
JI_API JiError* ji_error_create(JiResultCode code, const char* fmt, ...);

/** Create an error with source location. */
JI_API JiError* ji_error_create_at(JiResultCode code,
                                    const char* file, int line,
                                    const char* func,
                                    const char* fmt, ...);

/** Destroy an error and its chain. */
JI_API void ji_error_destroy(JiError* err);

/** Get the human-readable string for a result code. */
JI_API const char* ji_result_to_string(JiResultCode code);

/* =========================================================================
 * Convenience macros
 * ========================================================================= */

/** Create an error with automatic source location. */
#define JI_ERROR(code, ...) \
    ji_error_create_at((code), __FILE__, __LINE__, __func__, __VA_ARGS__)

/** Return an error result if ptr is NULL. */
#define JI_RETURN_IF_NULL(ptr) \
    do { if (JI_UNLIKELY(!(ptr))) return JI_ERROR_NULL_PTR; } while(0)

/** Return if a condition is false. */
#define JI_RETURN_IF_FALSE(cond) \
    do { if (JI_UNLIKELY(!(cond))) return JI_ERROR_STATE; } while(0)

/** Return if an expression yields a failure result. */
#define JI_RETURN_IF_FAILED(expr) \
    do { JiResultCode _r = (expr); if (JI_FAILED(_r)) return _r; } while(0)

/* =========================================================================
 * JiResult — tagged union: either a value or an error
 * ========================================================================= */
typedef struct JiResult JiResult;

struct JiResult {
    JiResultCode code;
    union {
        void*       ptr;
        int64_t     integer;
        double      floating;
    } value;
    JiError*      error;  /* non-NULL only on failure */
};

/** Create a successful result with a pointer value. */
JI_API JiResult ji_result_ok_ptr(void* ptr);

/** Create a successful result with an integer value. */
JI_API JiResult ji_result_ok_int(int64_t val);

/** Create a successful result with a float value. */
JI_API JiResult ji_result_ok_float(double val);

/** Create a successful result with no value. */
JI_API JiResult ji_result_ok(void);

/** Create a failed result from an error. Takes ownership of err. */
JI_API JiResult ji_result_fail(JiError* err);

/** Create a failed result from a code (no detailed error). */
JI_API JiResult ji_result_fail_code(JiResultCode code);

/** Destroy a result (frees the error if present). Does NOT free value.ptr. */
JI_API void ji_result_destroy(JiResult* result);

/** Check if a result is successful. */
JI_API bool ji_result_is_ok(const JiResult* result);

/** Check if a result is a failure. */
JI_API bool ji_result_is_err(const JiResult* result);

/* =========================================================================
 * Logging
 * ========================================================================= */
typedef enum JiLogLevel {
    JI_LOG_TRACE = 0,
    JI_LOG_DEBUG = 1,
    JI_LOG_INFO  = 2,
    JI_LOG_WARN  = 3,
    JI_LOG_ERROR = 4,
    JI_LOG_FATAL = 5
} JiLogLevel;

/** Log callback type. */
typedef void (*JiLogCallback)(JiLogLevel level, const char* message, void* user_data);

/** Set the global log callback. */
JI_API void ji_log_set_callback(JiLogCallback callback, void* user_data);

/** Set the minimum log level. */
JI_API void ji_log_set_level(JiLogLevel level);

/** Internal log function. */
JI_API void ji_log_write(JiLogLevel level, const char* file, int line,
                         const char* func, const char* fmt, ...);

/** Convenience logging macros. */
#define JI_TRACE(...) ji_log_write(JI_LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define JI_DEBUG(...) ji_log_write(JI_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define JI_INFO(...)  ji_log_write(JI_LOG_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define JI_WARN(...)  ji_log_write(JI_LOG_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define JI_ERROR_LOG(...) ji_log_write(JI_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define JI_FATAL(...) ji_log_write(JI_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* JIUI_ERROR_H */
