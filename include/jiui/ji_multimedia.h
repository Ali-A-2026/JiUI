/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_multimedia.h
 * @brief Multimedia UI layer — video, audio visualization, timeline controls.
 */

#ifndef JIUI_MULTIMEDIA_H
#define JIUI_MULTIMEDIA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ji_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Constants
 * ========================================================================= */

#define JI_MM_MAX_FRAMES        1024
#define JI_MM_AUDIO_SAMPLES     8192
#define JI_MM_SPECTRUM_BINS     256
#define JI_MM_MAX_SUBTITLES     64

/* =========================================================================
 * Enums
 * ========================================================================= */

typedef enum JiVideoState {
    JI_VIDEO_STOPPED  = 0,
    JI_VIDEO_PLAYING  = 1,
    JI_VIDEO_PAUSED   = 2,
    JI_VIDEO_ENDED    = 3,
    JI_VIDEO_ERROR   = 4
} JiVideoState;

typedef enum JiAudioFormat {
    JI_AUDIO_S16 = 0,
    JI_AUDIO_F32 = 1,
    JI_AUDIO_U8  = 2
} JiAudioFormat;

typedef enum JiVizType {
    JI_VIZ_WAVEFORM  = 0,
    JI_VIZ_SPECTRUM  = 1,
    JI_VIZ_BARS      = 2
} JiVizType;

/* =========================================================================
 * Video Widget
 * ========================================================================= */

typedef struct JiVideoFrame {
    uint8_t* data;          /**< Pixel data (RGBA8888) */
    uint32_t  width;
    uint32_t  height;
    uint32_t  stride;
    int64_t   pts;          /**< Presentation timestamp (microseconds) */
    bool      keyframe;
} JiVideoFrame;

typedef struct JiVideoWidget {
    JiVideoState  state;
    char          source[256];
    int64_t       duration_us;     /**< Total duration in microseconds */
    int64_t       position_us;     /**< Current position in microseconds */
    double        playback_rate;   /**< 1.0 = normal, 2.0 = 2x, etc. */
    bool          loop;
    bool          muted;
    float         volume;          /**< 0.0 to 1.0 */
    JiVideoFrame* frames[JI_MM_MAX_FRAMES];
    int           frame_count;
    int           frame_read_idx;
    int           frame_write_idx;
    char          subtitles[JI_MM_MAX_SUBTITLES][256];
    int           subtitle_count;
    int           current_subtitle;
} JiVideoWidget;

JI_API JiVideoWidget* ji_video_new(void);
JI_API void           ji_video_free(JiVideoWidget* video);
JI_API int            ji_video_load(JiVideoWidget* video, const char* source);
JI_API int            ji_video_play(JiVideoWidget* video);
JI_API int            ji_video_pause(JiVideoWidget* video);
JI_API int            ji_video_stop(JiVideoWidget* video);
JI_API int            ji_video_seek(JiVideoWidget* video, int64_t position_us);
JI_API int64_t        ji_video_get_position(const JiVideoWidget* video);
JI_API int64_t        ji_video_get_duration(const JiVideoWidget* video);
JI_API JiVideoState   ji_video_get_state(const JiVideoWidget* video);
JI_API void           ji_video_set_volume(JiVideoWidget* video, float volume);
JI_API void           ji_video_set_loop(JiVideoWidget* video, bool loop);
JI_API void           ji_video_set_rate(JiVideoWidget* video, double rate);
JI_API int            ji_video_push_frame(JiVideoWidget* video, JiVideoFrame* frame);
JI_API JiVideoFrame*  ji_video_pop_frame(JiVideoWidget* video);
JI_API void           ji_video_free_frame(JiVideoFrame* frame);
JI_API int            ji_video_add_subtitle(JiVideoWidget* video, const char* text);
JI_API const char*    ji_video_get_subtitle(const JiVideoWidget* video, int index);

/* =========================================================================
 * Audio Visualization
 * ========================================================================= */

typedef struct JiAudioViz {
    JiVizType    type;
    JiAudioFormat format;
    uint32_t     sample_rate;
    uint8_t      channels;
    float        samples[JI_MM_AUDIO_SAMPLES];
    int          sample_count;
    float        spectrum[JI_MM_SPECTRUM_BINS];
    float        smooth_factor;   /**< 0.0 = no smoothing, 1.0 = max */
    float        min_db;
    float        max_db;
} JiAudioViz;

JI_API JiAudioViz* ji_audio_viz_new(JiVizType type);
JI_API void        ji_audio_viz_free(JiAudioViz* viz);
JI_API int         ji_audio_viz_push_samples(JiAudioViz* viz, const float* samples, int count);
JI_API int         ji_audio_viz_compute_spectrum(JiAudioViz* viz);
JI_API int         ji_audio_viz_compute_waveform(JiAudioViz* viz);
JI_API const float* ji_audio_viz_get_spectrum(const JiAudioViz* viz, int* bin_count);
JI_API const float* ji_audio_viz_get_waveform(const JiAudioViz* viz, int* sample_count);
JI_API void        ji_audio_viz_set_smooth(JiAudioViz* viz, float factor);
JI_API void        ji_audio_viz_set_range(JiAudioViz* viz, float min_db, float max_db);

/* =========================================================================
 * Timeline Controls
 * ========================================================================= */

typedef struct JiTimeline {
    int64_t   duration_us;
    int64_t   position_us;
    bool      playing;
    double    playback_rate;
    bool      scrubbing;
    int64_t   loop_start_us;
    int64_t   loop_end_us;
    bool      loop_enabled;
    int64_t   markers[32];
    int       marker_count;
    int       current_marker;
} JiTimeline;

JI_API JiTimeline* ji_timeline_new(void);
JI_API void        ji_timeline_free(JiTimeline* tl);
JI_API void        ji_timeline_set_duration(JiTimeline* tl, int64_t duration_us);
JI_API int64_t     ji_timeline_get_duration(const JiTimeline* tl);
JI_API int64_t     ji_timeline_get_position(const JiTimeline* tl);
JI_API void        ji_timeline_set_position(JiTimeline* tl, int64_t position_us);
JI_API void        ji_timeline_play(JiTimeline* tl);
JI_API void        ji_timeline_pause(JiTimeline* tl);
JI_API void        ji_timeline_stop(JiTimeline* tl);
JI_API bool        ji_timeline_is_playing(const JiTimeline* tl);
JI_API void        ji_timeline_set_rate(JiTimeline* tl, double rate);
JI_API double      ji_timeline_get_rate(const JiTimeline* tl);
JI_API void        ji_timeline_set_loop(JiTimeline* tl, int64_t start_us, int64_t end_us);
JI_API void        ji_timeline_clear_loop(JiTimeline* tl);
JI_API void        ji_timeline_begin_scrub(JiTimeline* tl);
JI_API void        ji_timeline_end_scrub(JiTimeline* tl);
JI_API bool        ji_timeline_is_scrubbing(const JiTimeline* tl);
JI_API int         ji_timeline_add_marker(JiTimeline* tl, int64_t position_us);
JI_API int64_t     ji_timeline_get_marker(const JiTimeline* tl, int index);
JI_API int         ji_timeline_get_marker_count(const JiTimeline* tl);
JI_API void        ji_timeline_advance(JiTimeline* tl, int64_t delta_us);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_MULTIMEDIA_H */
