#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_profiler.c
 * @brief Built-in profiler implementation.
 */

#include "jiui/ji_profiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * Time utility — microseconds
 * ========================================================================= */

static double ji_profiler_now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiProfiler* ji_profiler_new(void)
{
    JiProfiler* p = (JiProfiler*)calloc(1, sizeof(JiProfiler));
    if (!p) return NULL;
    p->enabled = true;
    p->overlay_mode = JI_PROFILER_OVERLAY_OFF;
    p->current_fps = 0.0;
    p->last_fps_update = ji_profiler_now_us();
    return p;
}

void ji_profiler_free(JiProfiler* profiler)
{
    free(profiler);
}

void ji_profiler_reset(JiProfiler* profiler)
{
    if (!profiler) return;
    memset(profiler->frames, 0, sizeof(profiler->frames));
    profiler->frame_index = 0;
    profiler->frame_count = 0;
    profiler->current_frame = 0;
    profiler->section_stack_depth = 0;
    profiler->memory_current = 0;
    profiler->memory_peak = 0;
    profiler->memory_total_alloc = 0;
    profiler->memory_total_free = 0;
    profiler->draw_calls_this_frame = 0;
    profiler->triangles_this_frame = 0;
    profiler->texture_binds_this_frame = 0;
    profiler->fps_accumulator = 0.0;
    profiler->fps_frame_count = 0;
    profiler->current_fps = 0.0;
    profiler->last_fps_update = ji_profiler_now_us();
    memset(profiler->section_totals, 0, sizeof(profiler->section_totals));
    memset(profiler->section_counts, 0, sizeof(profiler->section_counts));
    memset(profiler->section_names, 0, sizeof(profiler->section_names));
    profiler->registered_sections = 0;
}

/* =========================================================================
 * Frame Profiling
 * ========================================================================= */

void ji_profiler_begin_frame(JiProfiler* profiler)
{
    if (!profiler || !profiler->enabled) return;

    int idx = profiler->frame_index % JI_PROFILER_MAX_FRAMES;
    profiler->current_frame = idx;
    JiProfilerFrame* f = &profiler->frames[idx];

    memset(f, 0, sizeof(JiProfilerFrame));
    f->frame_start_us = ji_profiler_now_us();

    profiler->draw_calls_this_frame = 0;
    profiler->triangles_this_frame = 0;
    profiler->texture_binds_this_frame = 0;
    profiler->section_stack_depth = 0;
}

void ji_profiler_end_frame(JiProfiler* profiler)
{
    if (!profiler || !profiler->enabled) return;

    JiProfilerFrame* f = &profiler->frames[profiler->current_frame];
    f->frame_end_us = ji_profiler_now_us();
    f->frame_time_us = f->frame_end_us - f->frame_start_us;
    f->draw_calls = profiler->draw_calls_this_frame;
    f->triangles = profiler->triangles_this_frame;
    f->texture_binds = profiler->texture_binds_this_frame;
    f->memory_allocated = profiler->memory_total_alloc;
    f->memory_freed = profiler->memory_total_free;
    f->memory_peak = profiler->memory_peak;

    /* FPS calculation — update every 500ms */
    profiler->fps_accumulator += f->frame_time_us;
    profiler->fps_frame_count++;

    double now = f->frame_end_us;
    double elapsed = now - profiler->last_fps_update;
    if (elapsed >= 500000.0) { /* 500ms */
        if (profiler->fps_frame_count > 0) {
            double avg_frame = profiler->fps_accumulator / profiler->fps_frame_count;
            profiler->current_fps = (avg_frame > 0.0) ? (1000000.0 / avg_frame) : 0.0;
        }
        profiler->fps_accumulator = 0.0;
        profiler->fps_frame_count = 0;
        profiler->last_fps_update = now;
    }

    profiler->frame_index++;
    profiler->frame_count++;
}

/* =========================================================================
 * Section Profiling
 * ========================================================================= */

static int ji_profiler_find_or_register_section(JiProfiler* p, const char* name)
{
    for (int i = 0; i < p->registered_sections; i++) {
        if (strncmp(p->section_names[i], name, JI_PROFILER_SECTION_NAME) == 0)
            return i;
    }
    if (p->registered_sections < JI_PROFILER_MAX_SECTIONS) {
        int idx = p->registered_sections++;
        strncpy(p->section_names[idx], name, JI_PROFILER_SECTION_NAME - 1);
        p->section_names[idx][JI_PROFILER_SECTION_NAME - 1] = '\0';
        return idx;
    }
    return -1;
}

void ji_profiler_begin_section(JiProfiler* profiler, const char* name)
{
    if (!profiler || !profiler->enabled) return;

    JiProfilerFrame* f = &profiler->frames[profiler->current_frame];
    if (f->section_count >= JI_PROFILER_MAX_SECTIONS) return;

    int idx = f->section_count++;
    JiProfilerSection* s = &f->sections[idx];
    strncpy(s->name, name, JI_PROFILER_SECTION_NAME - 1);
    s->name[JI_PROFILER_SECTION_NAME - 1] = '\0';
    s->start_us = ji_profiler_now_us();
    s->elapsed_us = 0.0;
    s->depth = profiler->section_stack_depth;
    s->parent = (profiler->section_stack_depth > 0)
                    ? profiler->section_stack[profiler->section_stack_depth - 1]
                    : -1;
    s->active = true;

    /* Push onto stack */
    if (profiler->section_stack_depth < JI_PROFILER_MAX_SECTIONS)
        profiler->section_stack[profiler->section_stack_depth++] = idx;
}

void ji_profiler_end_section(JiProfiler* profiler)
{
    if (!profiler || !profiler->enabled) return;
    if (profiler->section_stack_depth <= 0) return;

    JiProfilerFrame* f = &profiler->frames[profiler->current_frame];
    int idx = profiler->section_stack[--profiler->section_stack_depth];
    JiProfilerSection* s = &f->sections[idx];
    s->elapsed_us = ji_profiler_now_us() - s->start_us;
    s->active = false;

    /* Accumulate for averages */
    int reg = ji_profiler_find_or_register_section(profiler, s->name);
    if (reg >= 0) {
        profiler->section_totals[reg] += s->elapsed_us;
        profiler->section_counts[reg]++;
    }

    /* Track layout/render special sections */
    if (strcmp(s->name, "layout") == 0)
        f->layout_time_us += s->elapsed_us;
    else if (strcmp(s->name, "render") == 0)
        f->render_time_us += s->elapsed_us;
}

void ji_profiler_begin_layout(JiProfiler* profiler)
{
    ji_profiler_begin_section(profiler, "layout");
}

void ji_profiler_end_layout(JiProfiler* profiler)
{
    ji_profiler_end_section(profiler);
}

void ji_profiler_begin_render(JiProfiler* profiler)
{
    ji_profiler_begin_section(profiler, "render");
}

void ji_profiler_end_render(JiProfiler* profiler)
{
    ji_profiler_end_section(profiler);
}

/* =========================================================================
 * GPU / Draw Call Tracking
 * ========================================================================= */

void ji_profiler_add_draw_call(JiProfiler* profiler, uint32_t triangles)
{
    if (!profiler || !profiler->enabled) return;
    profiler->draw_calls_this_frame++;
    profiler->triangles_this_frame += triangles;
}

void ji_profiler_add_texture_bind(JiProfiler* profiler)
{
    if (!profiler || !profiler->enabled) return;
    profiler->texture_binds_this_frame++;
}

void ji_profiler_set_gpu_time(JiProfiler* profiler, double us)
{
    if (!profiler || !profiler->enabled) return;
    JiProfilerFrame* f = &profiler->frames[profiler->current_frame];
    f->gpu_time_us = us;
}

/* =========================================================================
 * Memory Tracking
 * ========================================================================= */

void ji_profiler_track_alloc(JiProfiler* profiler, size_t bytes)
{
    if (!profiler || !profiler->enabled) return;
    profiler->memory_current += bytes;
    profiler->memory_total_alloc += bytes;
    if (profiler->memory_current > profiler->memory_peak)
        profiler->memory_peak = profiler->memory_current;
}

void ji_profiler_track_free(JiProfiler* profiler, size_t bytes)
{
    if (!profiler || !profiler->enabled) return;
    if (bytes > profiler->memory_current)
        profiler->memory_current = 0;
    else
        profiler->memory_current -= bytes;
    profiler->memory_total_free += bytes;
}

/* =========================================================================
 * Stats Retrieval
 * ========================================================================= */

JiProfilerStats ji_profiler_get_stats(JiProfiler* profiler)
{
    JiProfilerStats stats;
    memset(&stats, 0, sizeof(stats));
    if (!profiler) return stats;

    int n = (profiler->frame_count < JI_PROFILER_MAX_FRAMES)
                ? profiler->frame_count
                : JI_PROFILER_MAX_FRAMES;
    if (n == 0) return stats;

    double sum_frame = 0, sum_gpu = 0, sum_layout = 0, sum_render = 0;
    double min_frame = 1e18, max_frame = 0;
    uint32_t sum_draws = 0, sum_tris = 0;

    for (int i = 0; i < n; i++) {
        JiProfilerFrame* f = &profiler->frames[i];
        sum_frame += f->frame_time_us;
        sum_gpu += f->gpu_time_us;
        sum_layout += f->layout_time_us;
        sum_render += f->render_time_us;
        sum_draws += f->draw_calls;
        sum_tris += f->triangles;
        if (f->frame_time_us < min_frame) min_frame = f->frame_time_us;
        if (f->frame_time_us > max_frame) max_frame = f->frame_time_us;
    }

    stats.avg_frame_time_us = sum_frame / n;
    stats.min_frame_time_us = min_frame;
    stats.max_frame_time_us = max_frame;
    stats.avg_gpu_time_us = sum_gpu / n;
    stats.avg_layout_time_us = sum_layout / n;
    stats.avg_render_time_us = sum_render / n;
    stats.avg_draw_calls = sum_draws / n;
    stats.avg_triangles = sum_tris / n;
    stats.fps = profiler->current_fps;
    stats.current_memory = profiler->memory_current;
    stats.peak_memory = profiler->memory_peak;
    stats.total_frames = profiler->frame_count;
    stats.dropped_frames = 0; /* Not tracked yet */

    return stats;
}

double ji_profiler_get_fps(JiProfiler* profiler)
{
    if (!profiler) return 0.0;
    return profiler->current_fps;
}

double ji_profiler_get_frame_time_ms(JiProfiler* profiler)
{
    if (!profiler || profiler->frame_count == 0) return 0.0;
    int idx = (profiler->frame_index == 0) ? JI_PROFILER_MAX_FRAMES - 1 : profiler->frame_index - 1;
    return profiler->frames[idx].frame_time_us / 1000.0;
}

double ji_profiler_get_section_time(JiProfiler* profiler, const char* name)
{
    if (!profiler || !name) return 0.0;
    for (int i = 0; i < profiler->registered_sections; i++) {
        if (strncmp(profiler->section_names[i], name, JI_PROFILER_SECTION_NAME) == 0) {
            if (profiler->section_counts[i] == 0) return 0.0;
            return profiler->section_totals[i] / profiler->section_counts[i];
        }
    }
    return 0.0;
}

/* =========================================================================
 * Overlay
 * ========================================================================= */

void ji_profiler_set_overlay(JiProfiler* profiler, JiProfilerOverlay mode)
{
    if (!profiler) return;
    profiler->overlay_mode = mode;
}

JiProfilerOverlay ji_profiler_get_overlay(JiProfiler* profiler)
{
    if (!profiler) return JI_PROFILER_OVERLAY_OFF;
    return profiler->overlay_mode;
}

int ji_profiler_get_overlay_text(JiProfiler* profiler, char* buf, int buf_size)
{
    if (!profiler || !buf || buf_size <= 0) return 0;
    if (profiler->overlay_mode == JI_PROFILER_OVERLAY_OFF) {
        buf[0] = '\0';
        return 0;
    }

    JiProfilerStats s = ji_profiler_get_stats(profiler);

    if (profiler->overlay_mode == JI_PROFILER_OVERLAY_MINIMAL) {
        return snprintf(buf, (size_t)buf_size,
            "FPS: %.1f | Frame: %.2fms",
            s.fps, s.avg_frame_time_us / 1000.0);
    }

    if (profiler->overlay_mode == JI_PROFILER_OVERLAY_GRAPH) {
        /* Simple ASCII bar chart of last 20 frames */
        int written = snprintf(buf, (size_t)buf_size,
            "FPS: %.1f | Frame: %.2fms\nFrame Graph:\n", s.fps, s.avg_frame_time_us / 1000.0);
        int start = profiler->frame_index - 20;
        if (start < 0) start = 0;
        int count = profiler->frame_index - start;
        if (count > 20) count = 20;
        for (int i = 0; i < count; i++) {
            int idx = (start + i) % JI_PROFILER_MAX_FRAMES;
            double ms = profiler->frames[idx].frame_time_us / 1000.0;
            int bars = (int)(ms * 2.0);
            if (bars > 40) bars = 40;
            if (written < buf_size - 1) {
                written += snprintf(buf + written, (size_t)(buf_size - written), "%2dms|", (int)ms);
                for (int b = 0; b < bars && written < buf_size - 1; b++)
                    buf[written++] = '#';
                buf[written++] = '\n';
            }
        }
        buf[written] = '\0';
        return written;
    }

    /* FULL */
    return snprintf(buf, (size_t)buf_size,
        "=== JiUI Profiler ===\n"
        "FPS:           %.1f\n"
        "Frame Time:    %.3f ms (avg)\n"
        "  Min:         %.3f ms\n"
        "  Max:         %.3f ms\n"
        "GPU Time:      %.3f ms\n"
        "Layout Time:   %.3f ms\n"
        "Render Time:   %.3f ms\n"
        "Draw Calls:    %u\n"
        "Triangles:     %u\n"
        "Texture Binds: %u\n"
        "Memory:        %zu KB (peak: %zu KB)\n"
        "Total Frames:  %d\n",
        s.fps,
        s.avg_frame_time_us / 1000.0,
        s.min_frame_time_us / 1000.0,
        s.max_frame_time_us / 1000.0,
        s.avg_gpu_time_us / 1000.0,
        s.avg_layout_time_us / 1000.0,
        s.avg_render_time_us / 1000.0,
        s.avg_draw_calls,
        s.avg_triangles,
        0u, /* texture binds not in stats struct avg */
        s.current_memory / 1024,
        s.peak_memory / 1024,
        s.total_frames);
}

/* =========================================================================
 * Enable / Disable
 * ========================================================================= */

void ji_profiler_set_enabled(JiProfiler* profiler, bool enabled)
{
    if (!profiler) return;
    profiler->enabled = enabled;
}

bool ji_profiler_is_enabled(JiProfiler* profiler)
{
    if (!profiler) return false;
    return profiler->enabled;
}
