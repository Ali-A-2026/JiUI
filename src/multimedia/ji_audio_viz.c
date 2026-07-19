/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_audio_viz.c
 * @brief Audio visualization implementation (waveform + spectrum).
 */

#include "jiui/ji_multimedia.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

JiAudioViz* ji_audio_viz_new(JiVizType type)
{
    JiAudioViz* viz = (JiAudioViz*)calloc(1, sizeof(JiAudioViz));
    if (!viz) return NULL;
    viz->type = type;
    viz->format = JI_AUDIO_F32;
    viz->sample_rate = 44100;
    viz->channels = 2;
    viz->sample_count = 0;
    viz->smooth_factor = 0.7f;
    viz->min_db = -80.0f;
    viz->max_db = 0.0f;
    return viz;
}

void ji_audio_viz_free(JiAudioViz* viz)
{
    free(viz);
}

int ji_audio_viz_push_samples(JiAudioViz* viz, const float* samples, int count)
{
    if (!viz || !samples || count <= 0) return -1;
    int to_copy = count;
    if (to_copy > JI_MM_AUDIO_SAMPLES) to_copy = JI_MM_AUDIO_SAMPLES;
    memcpy(viz->samples, samples, (size_t)to_copy * sizeof(float));
    viz->sample_count = to_copy;
    return to_copy;
}

int ji_audio_viz_compute_waveform(JiAudioViz* viz)
{
    if (!viz) return -1;
    /* Waveform is just the raw samples — nothing to compute */
    return viz->sample_count > 0 ? 0 : -1;
}

int ji_audio_viz_compute_spectrum(JiAudioViz* viz)
{
    if (!viz || viz->sample_count == 0) return -1;

    /* Simple DFT for N=JI_MM_SPECTRUM_BINS bins.
     * This is a simplified O(N*M) approach suitable for testing.
     * A production implementation would use FFT. */
    int N = viz->sample_count;
    int M = JI_MM_SPECTRUM_BINS;

    for (int k = 0; k < M; k++) {
        float real = 0.0f, imag = 0.0f;
        float freq = (float)k / (float)M * (float)viz->sample_rate * 0.5f;
        float omega = 2.0f * 3.14159265358979f * freq / (float)viz->sample_rate;

        for (int n = 0; n < N; n++) {
            float angle = omega * (float)n;
            real += viz->samples[n] * cosf(angle);
            imag -= viz->samples[n] * sinf(angle);
        }

        float magnitude = sqrtf(real * real + imag * imag) / (float)N;
        float db = 20.0f * log10f(magnitude + 1e-10f);
        if (db < viz->min_db) db = viz->min_db;
        if (db > viz->max_db) db = viz->max_db;

        /* Normalize to 0..1 */
        float normalized = (db - viz->min_db) / (viz->max_db - viz->min_db);

        /* Apply smoothing */
        viz->spectrum[k] = viz->spectrum[k] * viz->smooth_factor +
                           normalized * (1.0f - viz->smooth_factor);
    }

    return 0;
}

const float* ji_audio_viz_get_spectrum(const JiAudioViz* viz, int* bin_count)
{
    if (!viz) return NULL;
    if (bin_count) *bin_count = JI_MM_SPECTRUM_BINS;
    return viz->spectrum;
}

const float* ji_audio_viz_get_waveform(const JiAudioViz* viz, int* sample_count)
{
    if (!viz) return NULL;
    if (sample_count) *sample_count = viz->sample_count;
    return viz->samples;
}

void ji_audio_viz_set_smooth(JiAudioViz* viz, float factor)
{
    if (!viz) return;
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;
    viz->smooth_factor = factor;
}

void ji_audio_viz_set_range(JiAudioViz* viz, float min_db, float max_db)
{
    if (!viz) return;
    viz->min_db = min_db;
    viz->max_db = max_db;
}
