/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_multimedia.c
 * @brief Tests for the multimedia UI layer.
 */

#include "jiui/ji_multimedia.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* =========================================================================
 * Test Framework
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    static void name(void); \
    static void name(void)

#define RUN_TEST(name) do { \
    g_tests_run++; \
    printf("  [RUN] %s ... ", #name); \
    name(); \
    g_tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return; \
    } \
} while(0)

/* =========================================================================
 * Video Tests
 * ========================================================================= */

TEST(test_video_create)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);
    ASSERT(ji_video_get_state(v) == JI_VIDEO_STOPPED);
    ASSERT(ji_video_get_position(v) == 0);
    ASSERT(ji_video_get_duration(v) == 0);
    ji_video_free(v);
}

TEST(test_video_play_pause)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);

    ASSERT(ji_video_load(v, "test.mp4") == 0);
    ASSERT(strcmp(v->source, "test.mp4") == 0);

    ASSERT(ji_video_play(v) == 0);
    ASSERT(ji_video_get_state(v) == JI_VIDEO_PLAYING);

    ASSERT(ji_video_pause(v) == 0);
    ASSERT(ji_video_get_state(v) == JI_VIDEO_PAUSED);

    ASSERT(ji_video_stop(v) == 0);
    ASSERT(ji_video_get_state(v) == JI_VIDEO_STOPPED);
    ASSERT(ji_video_get_position(v) == 0);

    ji_video_free(v);
}

TEST(test_video_seek)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);

    v->duration_us = 10000000;  /* 10 seconds */
    ASSERT(ji_video_seek(v, 5000000) == 0);
    ASSERT(ji_video_get_position(v) == 5000000);

    /* Seek beyond duration clamps */
    ASSERT(ji_video_seek(v, 20000000) == 0);
    ASSERT(ji_video_get_position(v) == 10000000);

    /* Seek negative clamps to 0 */
    ASSERT(ji_video_seek(v, -100) == 0);
    ASSERT(ji_video_get_position(v) == 0);

    ji_video_free(v);
}

TEST(test_video_volume)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);

    ji_video_set_volume(v, 0.5f);
    ASSERT(v->volume == 0.5f);

    ji_video_set_volume(v, 2.0f);
    ASSERT(v->volume == 1.0f);

    ji_video_set_volume(v, -1.0f);
    ASSERT(v->volume == 0.0f);

    ji_video_free(v);
}

TEST(test_video_frames)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);

    /* Push a frame */
    JiVideoFrame* f = (JiVideoFrame*)calloc(1, sizeof(JiVideoFrame));
    f->width = 640;
    f->height = 480;
    f->stride = 640 * 4;
    f->data = (uint8_t*)calloc(640 * 480 * 4, 1);
    f->pts = 1000;
    f->keyframe = true;

    ASSERT(ji_video_push_frame(v, f) == 0);
    ASSERT(v->frame_count == 1);

    /* Pop the frame */
    JiVideoFrame* popped = ji_video_pop_frame(v);
    ASSERT(popped != NULL);
    ASSERT(popped->width == 640);
    ASSERT(popped->pts == 1000);
    ASSERT(v->frame_count == 0);

    ji_video_free_frame(popped);

    /* Pop from empty */
    ASSERT(ji_video_pop_frame(v) == NULL);

    ji_video_free(v);
}

TEST(test_video_subtitles)
{
    JiVideoWidget* v = ji_video_new();
    ASSERT(v != NULL);

    ASSERT(ji_video_add_subtitle(v, "Hello World") == 0);
    ASSERT(ji_video_add_subtitle(v, "Goodbye") == 1);
    ASSERT(v->subtitle_count == 2);

    ASSERT(strcmp(ji_video_get_subtitle(v, 0), "Hello World") == 0);
    ASSERT(strcmp(ji_video_get_subtitle(v, 1), "Goodbye") == 0);
    ASSERT(ji_video_get_subtitle(v, 99) == NULL);

    ji_video_free(v);
}

/* =========================================================================
 * Audio Viz Tests
 * ========================================================================= */

TEST(test_audio_viz_create)
{
    JiAudioViz* viz = ji_audio_viz_new(JI_VIZ_SPECTRUM);
    ASSERT(viz != NULL);
    ASSERT(viz->type == JI_VIZ_SPECTRUM);
    ASSERT(viz->sample_rate == 44100);
    ASSERT(viz->channels == 2);
    ASSERT(viz->smooth_factor == 0.7f);
    ji_audio_viz_free(viz);
}

TEST(test_audio_viz_samples)
{
    JiAudioViz* viz = ji_audio_viz_new(JI_VIZ_WAVEFORM);
    ASSERT(viz != NULL);

    float samples[256];
    for (int i = 0; i < 256; i++) {
        samples[i] = sinf((float)i * 0.1f) * 0.5f;
    }

    ASSERT(ji_audio_viz_push_samples(viz, samples, 256) == 256);
    ASSERT(viz->sample_count == 256);

    int count = 0;
    const float* wf = ji_audio_viz_get_waveform(viz, &count);
    ASSERT(wf != NULL);
    ASSERT(count == 256);

    ji_audio_viz_free(viz);
}

TEST(test_audio_viz_spectrum)
{
    JiAudioViz* viz = ji_audio_viz_new(JI_VIZ_SPECTRUM);
    ASSERT(viz != NULL);

    /* Generate a sine wave at 440Hz */
    float samples[1024];
    for (int i = 0; i < 1024; i++) {
        samples[i] = sinf(2.0f * 3.14159265f * 440.0f * (float)i / 44100.0f);
    }
    ji_audio_viz_push_samples(viz, samples, 1024);

    ASSERT(ji_audio_viz_compute_spectrum(viz) == 0);

    int bins = 0;
    const float* spec = ji_audio_viz_get_spectrum(viz, &bins);
    ASSERT(spec != NULL);
    ASSERT(bins == JI_MM_SPECTRUM_BINS);

    /* At least one bin should have non-zero energy */
    bool has_energy = false;
    for (int i = 0; i < bins; i++) {
        if (spec[i] > 0.01f) {
            has_energy = true;
            break;
        }
    }
    ASSERT(has_energy);

    ji_audio_viz_free(viz);
}

TEST(test_audio_viz_smooth)
{
    JiAudioViz* viz = ji_audio_viz_new(JI_VIZ_SPECTRUM);
    ASSERT(viz != NULL);

    ji_audio_viz_set_smooth(viz, 0.5f);
    ASSERT(viz->smooth_factor == 0.5f);

    ji_audio_viz_set_smooth(viz, 2.0f);
    ASSERT(viz->smooth_factor == 1.0f);

    ji_audio_viz_set_smooth(viz, -1.0f);
    ASSERT(viz->smooth_factor == 0.0f);

    ji_audio_viz_free(viz);
}

/* =========================================================================
 * Timeline Tests
 * ========================================================================= */

TEST(test_timeline_create)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);
    ASSERT(ji_timeline_get_duration(tl) == 0);
    ASSERT(ji_timeline_get_position(tl) == 0);
    ASSERT(ji_timeline_is_playing(tl) == false);
    ASSERT(ji_timeline_is_scrubbing(tl) == false);
    ji_timeline_free(tl);
}

TEST(test_timeline_playback)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);

    ji_timeline_set_duration(tl, 10000000);  /* 10 seconds */
    ASSERT(ji_timeline_get_duration(tl) == 10000000);

    ji_timeline_play(tl);
    ASSERT(ji_timeline_is_playing(tl) == true);

    ji_timeline_advance(tl, 1000000);  /* 1 second */
    ASSERT(ji_timeline_get_position(tl) == 1000000);

    ji_timeline_pause(tl);
    ASSERT(ji_timeline_is_playing(tl) == false);

    ji_timeline_stop(tl);
    ASSERT(ji_timeline_get_position(tl) == 0);

    ji_timeline_free(tl);
}

TEST(test_timeline_rate)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);

    ji_timeline_set_duration(tl, 10000000);
    ji_timeline_play(tl);
    ji_timeline_set_rate(tl, 2.0);
    ASSERT(ji_timeline_get_rate(tl) == 2.0);

    ji_timeline_advance(tl, 1000000);  /* 1 second at 2x = 2 seconds */
    ASSERT(ji_timeline_get_position(tl) == 2000000);

    ji_timeline_free(tl);
}

TEST(test_timeline_scrub)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);

    ji_timeline_set_duration(tl, 10000000);
    ji_timeline_play(tl);

    ji_timeline_begin_scrub(tl);
    ASSERT(ji_timeline_is_scrubbing(tl) == true);
    ASSERT(ji_timeline_is_playing(tl) == false);

    ji_timeline_set_position(tl, 5000000);
    ASSERT(ji_timeline_get_position(tl) == 5000000);

    /* Advance should not move while scrubbing */
    ji_timeline_advance(tl, 1000000);
    ASSERT(ji_timeline_get_position(tl) == 5000000);

    ji_timeline_end_scrub(tl);
    ASSERT(ji_timeline_is_scrubbing(tl) == false);

    ji_timeline_free(tl);
}

TEST(test_timeline_loop)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);

    ji_timeline_set_duration(tl, 10000000);
    ji_timeline_set_position(tl, 3000000);
    ji_timeline_set_loop(tl, 2000000, 5000000);
    ji_timeline_play(tl);

    /* Advance past loop end */
    ji_timeline_advance(tl, 3000000);  /* 3s + 3s = 6s, but loop wraps */
    ASSERT(ji_timeline_get_position(tl) == 2000000);  /* Wrapped to loop start */

    ji_timeline_clear_loop(tl);
    ASSERT(tl->loop_enabled == false);

    ji_timeline_free(tl);
}

TEST(test_timeline_markers)
{
    JiTimeline* tl = ji_timeline_new();
    ASSERT(tl != NULL);

    ASSERT(ji_timeline_add_marker(tl, 1000000) == 0);
    ASSERT(ji_timeline_add_marker(tl, 5000000) == 1);
    ASSERT(ji_timeline_add_marker(tl, 9000000) == 2);
    ASSERT(ji_timeline_get_marker_count(tl) == 3);

    ASSERT(ji_timeline_get_marker(tl, 0) == 1000000);
    ASSERT(ji_timeline_get_marker(tl, 1) == 5000000);
    ASSERT(ji_timeline_get_marker(tl, 2) == 9000000);
    ASSERT(ji_timeline_get_marker(tl, 99) == -1);

    ji_timeline_free(tl);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Multimedia Tests ===\n");
    RUN_TEST(test_video_create);
    RUN_TEST(test_video_play_pause);
    RUN_TEST(test_video_seek);
    RUN_TEST(test_video_volume);
    RUN_TEST(test_video_frames);
    RUN_TEST(test_video_subtitles);
    RUN_TEST(test_audio_viz_create);
    RUN_TEST(test_audio_viz_samples);
    RUN_TEST(test_audio_viz_spectrum);
    RUN_TEST(test_audio_viz_smooth);
    RUN_TEST(test_timeline_create);
    RUN_TEST(test_timeline_playback);
    RUN_TEST(test_timeline_rate);
    RUN_TEST(test_timeline_scrub);
    RUN_TEST(test_timeline_loop);
    RUN_TEST(test_timeline_markers);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
