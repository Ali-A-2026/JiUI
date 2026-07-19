/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_profiler.h
 * @brief Built-in profiler — frame timing, section profiling, memory tracking,
 *        draw call counting, texture memory, layout cost, FPS, overlay display.
 */

#ifndef JIUI_PROFILER_H
#define JIUI_PROFILER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Profiler Constants
 * ========================================================================= */

#define JI_PROFILER_MAX_SECTIONS   256
#define JI_PROFILER_MAX_FRAMES     120   /* Ring buffer for frame history */
#define JI_PROFILER_SECTION_NAME   64

/* =========================================================================
 * Profiler Data Structures
 * ========================================================================= */

/** A single timed section within a frame. */
typedef struct JiProfilerSection {
    char     name[JI_PROFILER_SECTION_NAME];
    double   start_us;     /* Start time in microseconds */
    double   elapsed_us;   /* Elapsed time in microseconds */
    int      parent;       /* Parent section index (-1 = root) */
    int      depth;
    bool     active;
} JiProfilerSection;

/** Per-frame statistics. */
typedef struct JiProfilerFrame {
    double   frame_start_us;
    double   frame_end_us;
    double   frame_time_us;     /* Total frame time */
    double   gpu_time_us;       /* GPU time estimate */
    double   layout_time_us;    /* Layout phase time */
    double   render_time_us;    /* Render phase time */
    uint32_t draw_calls;
    uint32_t triangles;
    uint32_t texture_binds;
    size_t   memory_allocated;
    size_t   memory_freed;
    size_t   memory_peak;
    int      section_count;
    JiProfilerSection sections[JI_PROFILER_MAX_SECTIONS];
} JiProfilerFrame;

/** Aggregated stats over recent frames. */
typedef struct JiProfilerStats {
    double   avg_frame_time_us;
    double   min_frame_time_us;
    double   max_frame_time_us;
    double   avg_gpu_time_us;
    double   avg_layout_time_us;
    double   avg_render_time_us;
    uint32_t avg_draw_calls;
    uint32_t avg_triangles;
    double   fps;                /* Frames per second */
    size_t   current_memory;
    size_t   peak_memory;
    int      total_frames;
    int      dropped_frames;
} JiProfilerStats;

/** Overlay display mode. */
typedef enum JiProfilerOverlay {
    JI_PROFILER_OVERLAY_OFF = 0,
    JI_PROFILER_OVERLAY_MINIMAL,    /* FPS + frame time */
    JI_PROFILER_OVERLAY_FULL,       /* All stats */
    JI_PROFILER_OVERLAY_GRAPH       /* Frame time graph */
} JiProfilerOverlay;

/** Main profiler handle. */
typedef struct JiProfiler {
    JiProfilerFrame frames[JI_PROFILER_MAX_FRAMES];
    int      frame_index;        /* Ring buffer write index */
    int      frame_count;        /* Total frames recorded */
    int      current_frame;      /* Index of frame being recorded */

    /* Section stack for nested profiling */
    int      section_stack[JI_PROFILER_MAX_SECTIONS];
    int      section_stack_depth;

    /* Memory tracking */
    size_t   memory_current;
    size_t   memory_peak;
    size_t   memory_total_alloc;
    size_t   memory_total_free;

    /* Draw call tracking */
    uint32_t draw_calls_this_frame;
    uint32_t triangles_this_frame;
    uint32_t texture_binds_this_frame;

    /* Timing */
    double   fps_accumulator;
    int      fps_frame_count;
    double   last_fps_update;
    double   current_fps;

    /* Overlay */
    JiProfilerOverlay overlay_mode;
    bool     enabled;

    /* Section timing accumulators (for averages) */
    double   section_totals[JI_PROFILER_MAX_SECTIONS];
    int      section_counts[JI_PROFILER_MAX_SECTIONS];
    char     section_names[JI_PROFILER_MAX_SECTIONS][JI_PROFILER_SECTION_NAME];
    int      registered_sections;
} JiProfiler;

/* =========================================================================
 * Profiler Lifecycle
 * ========================================================================= */

JI_API JiProfiler* ji_profiler_new(void);
JI_API void        ji_profiler_free(JiProfiler* profiler);
JI_API void        ji_profiler_reset(JiProfiler* profiler);

/* =========================================================================
 * Frame Profiling
 * ========================================================================= */

JI_API void ji_profiler_begin_frame(JiProfiler* profiler);
JI_API void ji_profiler_end_frame(JiProfiler* profiler);

/* =========================================================================
 * Section Profiling (nested)
 * ========================================================================= */

JI_API void ji_profiler_begin_section(JiProfiler* profiler, const char* name);
JI_API void ji_profiler_end_section(JiProfiler* profiler);

/* Convenience: begin/end layout section */
JI_API void ji_profiler_begin_layout(JiProfiler* profiler);
JI_API void ji_profiler_end_layout(JiProfiler* profiler);

/* Convenience: begin/end render section */
JI_API void ji_profiler_begin_render(JiProfiler* profiler);
JI_API void ji_profiler_end_render(JiProfiler* profiler);

/* =========================================================================
 * GPU / Draw Call Tracking
 * ========================================================================= */

JI_API void ji_profiler_add_draw_call(JiProfiler* profiler, uint32_t triangles);
JI_API void ji_profiler_add_texture_bind(JiProfiler* profiler);
JI_API void ji_profiler_set_gpu_time(JiProfiler* profiler, double us);

/* =========================================================================
 * Memory Tracking
 * ========================================================================= */

JI_API void ji_profiler_track_alloc(JiProfiler* profiler, size_t bytes);
JI_API void ji_profiler_track_free(JiProfiler* profiler, size_t bytes);

/* =========================================================================
 * Stats Retrieval
 * ========================================================================= */

JI_API JiProfilerStats ji_profiler_get_stats(JiProfiler* profiler);
JI_API double ji_profiler_get_fps(JiProfiler* profiler);
JI_API double ji_profiler_get_frame_time_ms(JiProfiler* profiler);

/* Get section average time by name (returns 0 if not found) */
JI_API double ji_profiler_get_section_time(JiProfiler* profiler, const char* name);

/* =========================================================================
 * Overlay
 * ========================================================================= */

JI_API void ji_profiler_set_overlay(JiProfiler* profiler, JiProfilerOverlay mode);
JI_API JiProfilerOverlay ji_profiler_get_overlay(JiProfiler* profiler);

/* Get overlay text (for rendering) — returns string length, 0 if overlay off */
JI_API int ji_profiler_get_overlay_text(JiProfiler* profiler, char* buf, int buf_size);

/* =========================================================================
 * Enable / Disable
 * ========================================================================= */

JI_API void ji_profiler_set_enabled(JiProfiler* profiler, bool enabled);
JI_API bool ji_profiler_is_enabled(JiProfiler* profiler);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PROFILER_H */
