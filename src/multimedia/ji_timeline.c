/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_timeline.c
 * @brief Timeline controls implementation.
 */

#include "jiui/ji_multimedia.h"
#include <stdlib.h>
#include <string.h>

JiTimeline* ji_timeline_new(void)
{
    JiTimeline* tl = (JiTimeline*)calloc(1, sizeof(JiTimeline));
    if (!tl) return NULL;
    tl->duration_us = 0;
    tl->position_us = 0;
    tl->playing = false;
    tl->playback_rate = 1.0;
    tl->scrubbing = false;
    tl->loop_start_us = 0;
    tl->loop_end_us = 0;
    tl->loop_enabled = false;
    tl->marker_count = 0;
    tl->current_marker = -1;
    return tl;
}

void ji_timeline_free(JiTimeline* tl)
{
    free(tl);
}

void ji_timeline_set_duration(JiTimeline* tl, int64_t duration_us)
{
    if (!tl || duration_us < 0) return;
    tl->duration_us = duration_us;
    if (tl->position_us > duration_us) {
        tl->position_us = duration_us;
    }
}

int64_t ji_timeline_get_duration(const JiTimeline* tl)
{
    return tl ? tl->duration_us : 0;
}

int64_t ji_timeline_get_position(const JiTimeline* tl)
{
    return tl ? tl->position_us : 0;
}

void ji_timeline_set_position(JiTimeline* tl, int64_t position_us)
{
    if (!tl) return;
    if (position_us < 0) position_us = 0;
    if (tl->duration_us > 0 && position_us > tl->duration_us) {
        position_us = tl->duration_us;
    }
    tl->position_us = position_us;
}

void ji_timeline_play(JiTimeline* tl)
{
    if (!tl) return;
    tl->playing = true;
}

void ji_timeline_pause(JiTimeline* tl)
{
    if (!tl) return;
    tl->playing = false;
}

void ji_timeline_stop(JiTimeline* tl)
{
    if (!tl) return;
    tl->playing = false;
    tl->position_us = 0;
}

bool ji_timeline_is_playing(const JiTimeline* tl)
{
    return tl ? tl->playing : false;
}

void ji_timeline_set_rate(JiTimeline* tl, double rate)
{
    if (tl && rate > 0.0) tl->playback_rate = rate;
}

double ji_timeline_get_rate(const JiTimeline* tl)
{
    return tl ? tl->playback_rate : 1.0;
}

void ji_timeline_set_loop(JiTimeline* tl, int64_t start_us, int64_t end_us)
{
    if (!tl || start_us >= end_us) return;
    tl->loop_start_us = start_us;
    tl->loop_end_us = end_us;
    tl->loop_enabled = true;
}

void ji_timeline_clear_loop(JiTimeline* tl)
{
    if (!tl) return;
    tl->loop_enabled = false;
    tl->loop_start_us = 0;
    tl->loop_end_us = 0;
}

void ji_timeline_begin_scrub(JiTimeline* tl)
{
    if (!tl) return;
    tl->scrubbing = true;
    tl->playing = false;
}

void ji_timeline_end_scrub(JiTimeline* tl)
{
    if (!tl) return;
    tl->scrubbing = false;
}

bool ji_timeline_is_scrubbing(const JiTimeline* tl)
{
    return tl ? tl->scrubbing : false;
}

int ji_timeline_add_marker(JiTimeline* tl, int64_t position_us)
{
    if (!tl || tl->marker_count >= 32) return -1;
    tl->markers[tl->marker_count] = position_us;
    return tl->marker_count++;
}

int64_t ji_timeline_get_marker(const JiTimeline* tl, int index)
{
    if (!tl || index < 0 || index >= tl->marker_count) return -1;
    return tl->markers[index];
}

int ji_timeline_get_marker_count(const JiTimeline* tl)
{
    return tl ? tl->marker_count : 0;
}

void ji_timeline_advance(JiTimeline* tl, int64_t delta_us)
{
    if (!tl || !tl->playing || tl->scrubbing) return;
    int64_t new_pos = tl->position_us + (int64_t)(delta_us * tl->playback_rate);
    if (tl->loop_enabled && new_pos >= tl->loop_end_us) {
        new_pos = tl->loop_start_us;
    } else if (new_pos >= tl->duration_us) {
        new_pos = tl->duration_us;
        tl->playing = false;
    }
    if (new_pos < 0) new_pos = 0;
    tl->position_us = new_pos;
}
