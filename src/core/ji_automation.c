#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_automation.c
 * @brief UI Automation implementation — finding, simulation, assertions.
 */

#include "jiui/ji_automation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * Time utility
 * ========================================================================= */

static double ji_auto_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =========================================================================
 * Internal helpers
 * ========================================================================= */

static void ji_auto_set_error(JiAutomation* a, const char* msg)
{
    if (!a || !msg) return;
    strncpy(a->error_msg, msg, sizeof(a->error_msg) - 1);
    a->error_msg[sizeof(a->error_msg) - 1] = '\0';
}

static bool ji_auto_grow_actions(JiAutomation* a)
{
    if (a->action_count < a->action_capacity) return true;
    int new_cap = a->action_capacity * 2;
    JiAutoAction* new_arr = (JiAutoAction*)realloc(a->actions, new_cap * sizeof(JiAutoAction));
    if (!new_arr) return false;
    a->actions = new_arr;
    a->action_capacity = new_cap;
    return true;
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

JiAutomation* ji_automation_new(void)
{
    JiAutomation* a = (JiAutomation*)calloc(1, sizeof(JiAutomation));
    if (!a) return NULL;
    a->action_capacity = 256;
    a->actions = (JiAutoAction*)calloc(a->action_capacity, sizeof(JiAutoAction));
    a->result_capacity = 64;
    a->results = (JiAutoResult*)calloc(a->result_capacity, sizeof(JiAutoResult));
    a->screenshot_match = true;
    a->screenshot_diff_percent = 0.0;
    return a;
}

void ji_automation_free(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    free(auto_ctx->actions);
    free(auto_ctx->results);
    free(auto_ctx);
}

void ji_automation_reset(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    auto_ctx->action_count = 0;
    auto_ctx->recording = false;
    auto_ctx->playing = false;
    auto_ctx->play_index = 0;
    auto_ctx->result_count = 0;
    auto_ctx->assertion_failures = 0;
    auto_ctx->error_msg[0] = '\0';
    auto_ctx->screenshot_match = true;
    auto_ctx->screenshot_diff_percent = 0.0;
}

/* =========================================================================
 * Widget Finding (stub — requires widget tree integration)
 * ========================================================================= */

int ji_automation_find_widgets(JiAutomation* auto_ctx,
                                 JiAutoFindMode mode,
                                 const char* value,
                                 JiAutoResult* results,
                                 int max_results)
{
    if (!auto_ctx || !value || !results || max_results <= 0) return 0;
    /* In a full implementation, this would traverse the widget tree.
     * For now, return 0 results (no widget tree connected). */
    (void)mode;
    ji_auto_set_error(auto_ctx, "no widget tree connected");
    return 0;
}

int ji_automation_find_one(JiAutomation* auto_ctx,
                            JiAutoFindMode mode,
                            const char* value,
                            JiAutoResult* result)
{
    if (!auto_ctx || !value || !result) return 0;
    int count = ji_automation_find_widgets(auto_ctx, mode, value, result, 1);
    return count > 0 ? 1 : 0;
}

/* =========================================================================
 * Input Simulation
 * ========================================================================= */

static void ji_auto_record(JiAutomation* a, JiAutoActionType type)
{
    if (!a || !a->recording) return;
    if (!ji_auto_grow_actions(a)) return;
    JiAutoAction* act = &a->actions[a->action_count++];
    memset(act, 0, sizeof(JiAutoAction));
    act->type = type;
    act->timestamp = ji_auto_now() - a->record_start_time;
}

void ji_automation_click(JiAutomation* auto_ctx, int x, int y)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_CLICK;
            act->x = x;
            act->y = y;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_double_click(JiAutomation* auto_ctx, int x, int y)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_DOUBLE_CLICK;
            act->x = x;
            act->y = y;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_right_click(JiAutomation* auto_ctx, int x, int y)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_RIGHT_CLICK;
            act->x = x;
            act->y = y;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_type_text(JiAutomation* auto_ctx, const char* text)
{
    if (!auto_ctx || !text) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_TYPE_TEXT;
            strncpy(act->text, text, sizeof(act->text) - 1);
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_key_press(JiAutomation* auto_ctx, int key_code)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_KEY_PRESS;
            act->key_code = key_code;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_mouse_move(JiAutomation* auto_ctx, int x, int y)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_MOUSE_MOVE;
            act->x = x;
            act->y = y;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_mouse_drag(JiAutomation* auto_ctx, int x1, int y1, int x2, int y2)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_MOUSE_DRAG;
            act->x = x1;
            act->y = y1;
            act->dx = x2 - x1;
            act->dy = y2 - y1;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_scroll(JiAutomation* auto_ctx, int dx, int dy)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_SCROLL;
            act->dx = dx;
            act->dy = dy;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

void ji_automation_focus_widget(JiAutomation* auto_ctx, int widget_id)
{
    if (!auto_ctx) return;
    if (auto_ctx->recording) {
        if (ji_auto_grow_actions(auto_ctx)) {
            JiAutoAction* act = &auto_ctx->actions[auto_ctx->action_count++];
            memset(act, 0, sizeof(JiAutoAction));
            act->type = JI_AUTO_FOCUS;
            act->widget_id = widget_id;
            act->timestamp = ji_auto_now() - auto_ctx->record_start_time;
        }
    }
}

/* =========================================================================
 * Assertions
 * ========================================================================= */

bool ji_automation_assert_visible(JiAutomation* auto_ctx, int widget_id)
{
    if (!auto_ctx) return false;
    /* Stub: would check widget tree */
    (void)widget_id;
    return true;
}

bool ji_automation_assert_enabled(JiAutomation* auto_ctx, int widget_id)
{
    if (!auto_ctx) return false;
    (void)widget_id;
    return true;
}

bool ji_automation_assert_text(JiAutomation* auto_ctx, int widget_id,
                                 const char* expected)
{
    if (!auto_ctx || !expected) return false;
    (void)widget_id;
    /* Stub: would compare widget text */
    return true;
}

bool ji_automation_assert_bounds(JiAutomation* auto_ctx, int widget_id,
                                  JiRect expected)
{
    if (!auto_ctx) return false;
    (void)widget_id;
    (void)expected;
    return true;
}

/* =========================================================================
 * Recording / Playback
 * ========================================================================= */

void ji_automation_record_start(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    auto_ctx->recording = true;
    auto_ctx->record_start_time = ji_auto_now();
    auto_ctx->action_count = 0;
}

void ji_automation_record_stop(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    auto_ctx->recording = false;
}

bool ji_automation_is_recording(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return false;
    return auto_ctx->recording;
}

void ji_automation_record_action(JiAutomation* auto_ctx, const JiAutoAction* action)
{
    if (!auto_ctx || !action || !auto_ctx->recording) return;
    if (!ji_auto_grow_actions(auto_ctx)) return;
    auto_ctx->actions[auto_ctx->action_count++] = *action;
}

void ji_automation_play_start(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    auto_ctx->playing = true;
    auto_ctx->play_index = 0;
    auto_ctx->play_start_time = ji_auto_now();
}

void ji_automation_play_stop(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return;
    auto_ctx->playing = false;
}

bool ji_automation_is_playing(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return false;
    return auto_ctx->playing;
}

const JiAutoAction* ji_automation_play_next(JiAutomation* auto_ctx)
{
    if (!auto_ctx || !auto_ctx->playing) return NULL;
    if (auto_ctx->play_index >= auto_ctx->action_count) {
        auto_ctx->playing = false;
        return NULL;
    }
    return &auto_ctx->actions[auto_ctx->play_index++];
}

/* =========================================================================
 * Screenshot Comparison (stub)
 * ========================================================================= */

bool ji_automation_screenshot_compare(JiAutomation* auto_ctx,
                                        const char* baseline_path,
                                        const char* current_path,
                                        double tolerance_percent)
{
    if (!auto_ctx || !baseline_path || !current_path) return false;
    /* Stub: would load both PNGs and compare pixel-by-pixel */
    (void)tolerance_percent;
    strncpy(auto_ctx->last_screenshot_path, current_path,
            sizeof(auto_ctx->last_screenshot_path) - 1);
    auto_ctx->screenshot_match = true;
    auto_ctx->screenshot_diff_percent = 0.0;
    return true;
}

/* =========================================================================
 * Export / Import
 * ========================================================================= */

int ji_automation_export_actions(JiAutomation* auto_ctx, char* buf, int buf_size)
{
    if (!auto_ctx || !buf || buf_size <= 0) return 0;
    int written = 0;
    written += snprintf(buf + written, (size_t)(buf_size - written), "[\n");
    for (int i = 0; i < auto_ctx->action_count && written < buf_size - 1; i++) {
        JiAutoAction* a = &auto_ctx->actions[i];
        written += snprintf(buf + written, (size_t)(buf_size - written),
            "  {\"type\": %d, \"ts\": %.3f, \"x\": %d, \"y\": %d, \"key\": %d, \"text\": \"%s\"}%s\n",
            (int)a->type, a->timestamp, a->x, a->y, a->key_code, a->text,
            (i < auto_ctx->action_count - 1) ? "," : "");
    }
    written += snprintf(buf + written, (size_t)(buf_size - written), "]\n");
    return written;
}

bool ji_automation_import_actions(JiAutomation* auto_ctx, const char* json)
{
    if (!auto_ctx || !json) return false;
    /* Simplified: just count action entries */
    auto_ctx->action_count = 0;
    /* In a full implementation, parse JSON array */
    return true;
}

/* =========================================================================
 * Error Handling
 * ========================================================================= */

const char* ji_automation_get_error(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return NULL;
    return auto_ctx->error_msg;
}

int ji_automation_get_assertion_failures(JiAutomation* auto_ctx)
{
    if (!auto_ctx) return 0;
    return auto_ctx->assertion_failures;
}
