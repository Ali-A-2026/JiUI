/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_error.c
 * @brief Implementation of error handling, result types, and logging.
 */

/* strdup is POSIX, not C99 — must be defined before any includes */
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/jiui.h"
#include "jiui/ji_error.h"
#include "jiui/ji_memory.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* =========================================================================
 * JiError
 * ========================================================================= */

JiError* ji_error_create(JiResultCode code, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    JiError* err = JI_NEW(JiError);
    if (!err) return NULL;

    err->code = code;
    err->source_file = NULL;
    err->source_line = 0;
    err->function = NULL;
    err->cause = NULL;

    if (fmt) {
        va_list args_copy;
        va_copy(args_copy, args);
        int len = vsnprintf(NULL, 0, fmt, args_copy);
        va_end(args_copy);

        if (len > 0) {
            err->message = (char*)ji_alloc((size_t)len + 1);
            if (err->message) {
                vsnprintf(err->message, (size_t)len + 1, fmt, args);
            }
        } else {
            err->message = NULL;
        }
    } else {
        err->message = NULL;
    }

    va_end(args);
    return err;
}

JiError* ji_error_create_at(JiResultCode code,
                             const char* file, int line,
                             const char* func,
                             const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    JiError* err = JI_NEW(JiError);
    if (!err) {
        va_end(args);
        return NULL;
    }

    err->code = code;
    err->source_line = line;
    err->cause = NULL;

    /* Duplicate file and function strings */
    if (file) {
        err->source_file = strdup(file);
    } else {
        err->source_file = NULL;
    }
    if (func) {
        err->function = strdup(func);
    } else {
        err->function = NULL;
    }

    /* Format message */
    if (fmt) {
        va_list args_copy;
        va_copy(args_copy, args);
        int len = vsnprintf(NULL, 0, fmt, args_copy);
        va_end(args_copy);

        if (len > 0) {
            err->message = (char*)ji_alloc((size_t)len + 1);
            if (err->message) {
                vsnprintf(err->message, (size_t)len + 1, fmt, args);
            }
        } else {
            err->message = NULL;
        }
    } else {
        err->message = NULL;
    }

    va_end(args);
    return err;
}

void ji_error_destroy(JiError* err) {
    if (!err) return;
    if (err->message)      ji_free(err->message);
    if (err->source_file)  ji_free(err->source_file);
    if (err->function)     ji_free(err->function);
    if (err->cause)        ji_error_destroy(err->cause);
    ji_free(err);
}

const char* ji_result_to_string(JiResultCode code) {
    switch (code) {
        case JI_OK:                  return "OK";
        case JI_ERROR_UNKNOWN:       return "Unknown error";
        case JI_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case JI_ERROR_INVALID_ARG:   return "Invalid argument";
        case JI_ERROR_NULL_PTR:     return "Null pointer";
        case JI_ERROR_NOT_FOUND:     return "Not found";
        case JI_ERROR_ALREADY_EXISTS: return "Already exists";
        case JI_ERROR_IO:           return "I/O error";
        case JI_ERROR_PARSE:        return "Parse error";
        case JI_ERROR_TYPE_MISMATCH: return "Type mismatch";
        case JI_ERROR_STATE:        return "Invalid state";
        case JI_ERROR_NOT_SUPPORTED: return "Not supported";
        case JI_ERROR_TIMEOUT:      return "Timeout";
        case JI_ERROR_CANCELLED:    return "Cancelled";
        case JI_ERROR_PERMISSION:   return "Permission denied";
        case JI_ERROR_PLATFORM:     return "Platform error";
        case JI_ERROR_RENDER:       return "Render error";
        case JI_ERROR_LAYOUT:       return "Layout error";
        default:                    return "Unknown result code";
    }
}

/* =========================================================================
 * JiResult
 * ========================================================================= */

JiResult ji_result_ok_ptr(void* ptr) {
    JiResult r;
    r.code = JI_OK;
    r.value.ptr = ptr;
    r.error = NULL;
    return r;
}

JiResult ji_result_ok_int(int64_t val) {
    JiResult r;
    r.code = JI_OK;
    r.value.integer = val;
    r.error = NULL;
    return r;
}

JiResult ji_result_ok_float(double val) {
    JiResult r;
    r.code = JI_OK;
    r.value.floating = val;
    r.error = NULL;
    return r;
}

JiResult ji_result_ok(void) {
    JiResult r;
    r.code = JI_OK;
    r.value.ptr = NULL;
    r.error = NULL;
    return r;
}

JiResult ji_result_fail(JiError* err) {
    JiResult r;
    r.code = err ? err->code : JI_ERROR_UNKNOWN;
    r.value.ptr = NULL;
    r.error = err;
    return r;
}

JiResult ji_result_fail_code(JiResultCode code) {
    JiResult r;
    r.code = code;
    r.value.ptr = NULL;
    r.error = NULL;
    return r;
}

void ji_result_destroy(JiResult* result) {
    if (!result) return;
    if (result->error) {
        ji_error_destroy(result->error);
        result->error = NULL;
    }
}

bool ji_result_is_ok(const JiResult* result) {
    return result && JI_SUCCEEDED(result->code);
}

bool ji_result_is_err(const JiResult* result) {
    return !result || JI_FAILED(result->code);
}

/* =========================================================================
 * Logging
 * ========================================================================= */

static JiLogLevel  g_log_level   = JI_LOG_INFO;
static JiLogCallback g_log_callback = NULL;
static void*        g_log_user_data = NULL;

void ji_log_set_callback(JiLogCallback callback, void* user_data) {
    g_log_callback  = callback;
    g_log_user_data = user_data;
}

void ji_log_set_level(JiLogLevel level) {
    g_log_level = level;
}

void ji_log_write(JiLogLevel level, const char* file, int line,
                  const char* func, const char* fmt, ...) {
    if (level < g_log_level) return;

    /* Format the user message */
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    if (g_log_callback) {
        g_log_callback(level, message, g_log_user_data);
    } else {
        /* Default: write to stderr */
        const char* level_str = "UNKNOWN";
        switch (level) {
            case JI_LOG_TRACE: level_str = "TRACE"; break;
            case JI_LOG_DEBUG: level_str = "DEBUG"; break;
            case JI_LOG_INFO:  level_str = "INFO";  break;
            case JI_LOG_WARN:  level_str = "WARN";  break;
            case JI_LOG_ERROR: level_str = "ERROR"; break;
            case JI_LOG_FATAL: level_str = "FATAL"; break;
        }
        fprintf(stderr, "[JiUI][%s] %s:%d (%s): %s\n",
                level_str, file, line, func, message);
    }
}

/* =========================================================================
 * Library init / shutdown (linked from jiui.h)
 * ========================================================================= */

static bool g_initialized = false;

JiResultCode ji_initialize(void) {
    if (g_initialized) return JI_ERROR_STATE;
    g_initialized = true;
    ji_type_init();
    ji_layout_type_init();
    ji_panel_type_init();
    ji_visual_type_init();
    ji_control_type_init();
    ji_widgets_type_init();
    ji_style_type_init();
    ji_platform_type_init();
    JI_INFO("JiUI v%s initialized", ji_version());
    return JI_OK;
}

void ji_shutdown(void) {
    if (!g_initialized) return;
    g_initialized = false;
    JI_INFO("JiUI shutdown");
}

const char* ji_version(void) {
    return JIUI_VERSION_STRING;
}
