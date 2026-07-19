/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_neural.h
 * @brief Neural UI rendering — layout optimization, interaction prediction,
 *        smart animation timing, and automatic accessibility improvements.
 *
 * Uses lightweight statistical models (no external ML dependencies) to:
 *   - Auto-optimize layouts based on measured render time
 *   - Predict likely user interactions (pre-render next frames)
 *   - Learn ease curves from user behavior
 *   - Auto-adjust font size and contrast for accessibility
 */

#ifndef JIUI_NEURAL_H
#define JIUI_NEURAL_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Neural Model — lightweight statistical predictor
 * ========================================================================= */

/** A simple exponential moving average predictor for render times. */
typedef struct JiNeuralModel {
    /* Layout optimization data */
    float*  render_times;       /* Ring buffer of render times (ms) */
    int     render_time_count;  /* Number of samples */
    int     render_time_capacity;
    int     render_time_index;  /* Ring buffer write index */
    float   avg_render_time;    /* EMA of render time */
    float   render_time_std;     /* Standard deviation estimate */

    /* Interaction prediction */
    int*    interaction_history; /* Ring buffer of interaction IDs */
    int     interaction_count;
    int     interaction_capacity;
    int     interaction_index;
    int     transition_matrix[16][16]; /* Markov chain: next interaction probs */

    /* Animation timing learning */
    float   preferred_duration;  /* Learned preferred animation duration */
    float   duration_adjustment; /* Adjustment factor based on user behavior */

    /* Accessibility */
    float   auto_font_scale;     /* Auto font scale (1.0 = default) */
    float   auto_contrast;       /* Auto contrast boost (0 = none, 1 = max) */
    float   user_reading_speed;  /* Estimated reading speed (chars/sec) */

    /* State */
    float   learning_rate;       /* EMA decay rate (0..1) */
    bool    enabled;
} JiNeuralModel;

/* =========================================================================
 * Neural Model — Lifecycle
 * ========================================================================= */

/** Create a new neural model. */
JI_API JiNeuralModel* ji_neural_new(void);

/** Destroy a neural model. */
JI_API void ji_neural_destroy(JiNeuralModel* model);

/** Enable/disable neural features. */
JI_API void ji_neural_set_enabled(JiNeuralModel* model, bool enabled);

/** Set learning rate (0 = no learning, 1 = instant adaptation). */
JI_API void ji_neural_set_learning_rate(JiNeuralModel* model, float rate);

/* =========================================================================
 * Layout Optimization
 * ========================================================================= */

/** Record a render time measurement (in milliseconds). */
JI_API void ji_neural_record_render_time(JiNeuralModel* model, float time_ms);

/** Get the average render time (EMA). */
JI_API float ji_neural_avg_render_time(const JiNeuralModel* model);

/** Get the render time standard deviation. */
JI_API float ji_neural_render_time_std(const JiNeuralModel* model);

/**
 * Suggest a layout optimization based on render time history.
 * Returns a complexity reduction factor (0..1) — 1 = no reduction needed,
 * 0 = maximum simplification.
 */
JI_API float ji_neural_suggest_layout_complexity(const JiNeuralModel* model,
                                                    float target_fps);

/* =========================================================================
 * Interaction Prediction
 * ========================================================================= */

/** Record a user interaction (by ID 0..15). */
JI_API void ji_neural_record_interaction(JiNeuralModel* model, int interaction_id);

/**
 * Predict the next interaction given the current one.
 * Returns the most likely next interaction ID, or -1 if no data.
 */
JI_API int ji_neural_predict_next_interaction(const JiNeuralModel* model,
                                                  int current_interaction_id);

/**
 * Get the probability of a specific next interaction.
 * Returns 0..1.
 */
JI_API float ji_neural_interaction_probability(const JiNeuralModel* model,
                                                    int from_id, int to_id);

/* =========================================================================
 * Smart Animation Timing
 * ========================================================================= */

/** Record an animation duration that the user experienced. */
JI_API void ji_neural_record_animation(JiNeuralModel* model, float duration);

/** Get the suggested animation duration based on learned preferences. */
JI_API float ji_neural_suggested_duration(const JiNeuralModel* model);

/* =========================================================================
 * Accessibility
 * ========================================================================= */

/** Record user reading speed (chars/sec). */
JI_API void ji_neural_record_reading_speed(JiNeuralModel* model, float chars_per_sec);

/** Get auto-adjusted font scale. */
JI_API float ji_neural_auto_font_scale(const JiNeuralModel* model);

/** Get auto-adjusted contrast boost. */
JI_API float ji_neural_auto_contrast(const JiNeuralModel* model);

/**
 * Evaluate whether the current contrast ratio meets accessibility standards.
 * @param fg_r,fg_g,fg_b  Foreground color (0..1).
 * @param bg_r,bg_g,bg_b  Background color (0..1).
 * @return Contrast ratio (1..21). WCAG AA requires >= 4.5, AAA >= 7.
 */
JI_API float ji_neural_contrast_ratio(float fg_r, float fg_g, float fg_b,
                                         float bg_r, float bg_g, float bg_b);

/**
 * Suggest an accessible foreground color for a given background.
 * Returns the suggested color via output parameters.
 */
JI_API void ji_neural_suggest_fg_color(float bg_r, float bg_g, float bg_b,
                                         float* out_r, float* out_g, float* out_b);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_NEURAL_H */
