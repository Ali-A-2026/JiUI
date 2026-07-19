/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_video.c
 * @brief Video widget implementation.
 */

#include "jiui/ji_multimedia.h"
#include <stdlib.h>
#include <string.h>

JiVideoWidget* ji_video_new(void)
{
    JiVideoWidget* v = (JiVideoWidget*)calloc(1, sizeof(JiVideoWidget));
    if (!v) return NULL;
    v->state = JI_VIDEO_STOPPED;
    v->playback_rate = 1.0;
    v->volume = 1.0f;
    v->loop = false;
    v->muted = false;
    v->frame_count = 0;
    v->frame_read_idx = 0;
    v->frame_write_idx = 0;
    v->subtitle_count = 0;
    v->current_subtitle = -1;
    return v;
}

void ji_video_free(JiVideoWidget* video)
{
    if (!video) return;
    for (int i = 0; i < video->frame_count; i++) {
        if (video->frames[i]) {
            ji_video_free_frame(video->frames[i]);
            video->frames[i] = NULL;
        }
    }
    free(video);
}

int ji_video_load(JiVideoWidget* video, const char* source)
{
    if (!video || !source) return -1;
    strncpy(video->source, source, sizeof(video->source) - 1);
    video->source[sizeof(video->source) - 1] = '\0';
    video->state = JI_VIDEO_STOPPED;
    video->position_us = 0;
    return 0;
}

int ji_video_play(JiVideoWidget* video)
{
    if (!video) return -1;
    if (video->state == JI_VIDEO_ENDED) {
        video->position_us = 0;
    }
    video->state = JI_VIDEO_PLAYING;
    return 0;
}

int ji_video_pause(JiVideoWidget* video)
{
    if (!video) return -1;
    if (video->state == JI_VIDEO_PLAYING) {
        video->state = JI_VIDEO_PAUSED;
    }
    return 0;
}

int ji_video_stop(JiVideoWidget* video)
{
    if (!video) return -1;
    video->state = JI_VIDEO_STOPPED;
    video->position_us = 0;
    return 0;
}

int ji_video_seek(JiVideoWidget* video, int64_t position_us)
{
    if (!video || position_us < 0) return -1;
    if (video->duration_us > 0 && position_us > video->duration_us) {
        position_us = video->duration_us;
    }
    video->position_us = position_us;
    return 0;
}

int64_t ji_video_get_position(const JiVideoWidget* video)
{
    return video ? video->position_us : 0;
}

int64_t ji_video_get_duration(const JiVideoWidget* video)
{
    return video ? video->duration_us : 0;
}

JiVideoState ji_video_get_state(const JiVideoWidget* video)
{
    return video ? video->state : JI_VIDEO_STOPPED;
}

void ji_video_set_volume(JiVideoWidget* video, float volume)
{
    if (!video) return;
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    video->volume = volume;
}

void ji_video_set_loop(JiVideoWidget* video, bool loop)
{
    if (video) video->loop = loop;
}

void ji_video_set_rate(JiVideoWidget* video, double rate)
{
    if (video && rate > 0.0) video->playback_rate = rate;
}

int ji_video_push_frame(JiVideoWidget* video, JiVideoFrame* frame)
{
    if (!video || !frame) return -1;
    if (video->frame_count >= JI_MM_MAX_FRAMES) return -1;
    video->frames[video->frame_write_idx] = frame;
    video->frame_write_idx = (video->frame_write_idx + 1) % JI_MM_MAX_FRAMES;
    video->frame_count++;
    return 0;
}

JiVideoFrame* ji_video_pop_frame(JiVideoWidget* video)
{
    if (!video || video->frame_count == 0) return NULL;
    JiVideoFrame* f = video->frames[video->frame_read_idx];
    video->frames[video->frame_read_idx] = NULL;
    video->frame_read_idx = (video->frame_read_idx + 1) % JI_MM_MAX_FRAMES;
    video->frame_count--;
    return f;
}

void ji_video_free_frame(JiVideoFrame* frame)
{
    if (!frame) return;
    if (frame->data) free(frame->data);
    free(frame);
}

int ji_video_add_subtitle(JiVideoWidget* video, const char* text)
{
    if (!video || !text) return -1;
    if (video->subtitle_count >= JI_MM_MAX_SUBTITLES) return -1;
    strncpy(video->subtitles[video->subtitle_count], text, 255);
    video->subtitles[video->subtitle_count][255] = '\0';
    return video->subtitle_count++;
}

const char* ji_video_get_subtitle(const JiVideoWidget* video, int index)
{
    if (!video || index < 0 || index >= video->subtitle_count) return NULL;
    return video->subtitles[index];
}
