/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_automation.h
 * @brief UI Automation Framework — widget finding, input simulation,
 *        action recording/playback, assertions, screenshot comparison.
 */

#ifndef JIUI_AUTOMATION_H
#define JIUI_AUTOMATION_H

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
 * Automation Constants
 * ========================================================================= */

#define JI_AUTOMATION_MAX_ACTIONS  4096
#define JI_AUTOMATION_MAX_RESULTS  256
#define JI_AUTOMATION_STR_MAX      128

/* =========================================================================
 * Automation Action Types
 * ========================================================================= */

typedef enum JiAutoActionType {
    JI_AUTO_CLICK       = 0,
    JI_AUTO_DOUBLE_CLICK = 1,
    JI_AUTO_RIGHT_CLICK  = 2,
    JI_AUTO_TYPE_TEXT   = 3,
    JI_AUTO_KEY_PRESS   = 4,
    JI_AUTO_KEY_RELEASE = 5,
    JI_AUTO_MOUSE_MOVE  = 6,
    JI_AUTO_MOUSE_DRAG  = 7,
    JI_AUTO_GESTURE     = 8,
    JI_AUTO_SCROLL      = 9,
    JI_AUTO_FOCUS        = 10,
    JI_AUTO_ASSERT       = 11
} JiAutoActionType;

/** A single recorded automation action. */
typedef struct JiAutoAction {
    JiAutoActionType type;
    double timestamp;       /* Seconds since recording start */
    int    widget_id;      /* Target widget ID (-1 = none) */
    int    x, y;           /* Coordinates for mouse actions */
    int    dx, dy;         /* Delta for drag/scroll */
    int    key_code;       /* Key code for key actions */
    char   text[64];       /* Text for type actions */
    char   widget_name[JI_AUTOMATION_STR_MAX]; /* Widget name for find */
    char   property[64];   /* Property name for assertions */
    char   expected[64];   /* Expected value for assertions */
} JiAutoAction;

/* =========================================================================
 * Automation Query Result
 * ========================================================================= */

typedef enum JiAutoFindMode {
    JI_AUTO_FIND_BY_TYPE = 0,
    JI_AUTO_FIND_BY_ID   = 1,
    JI_AUTO_FIND_BY_CLASS = 2,
    JI_AUTO_FIND_BY_NAME  = 3
} JiAutoFindMode;

typedef struct JiAutoResult {
    int    widget_id;
    char   name[JI_AUTOMATION_STR_MAX];
    char   type_name[JI_AUTOMATION_STR_MAX];
    char   class_name[JI_AUTOMATION_STR_MAX];
    JiRect bounds;
    bool   visible;
    bool   enabled;
    bool   focused;
} JiAutoResult;

/* =========================================================================
 * Automation Context
 * ========================================================================= */

typedef struct JiAutomation {
    /* Recording state */
    JiAutoAction* actions;
    int           action_count;
    int           action_capacity;
    bool          recording;
    double        record_start_time;

    /* Playback state */
    bool          playing;
    int           play_index;
    double        play_start_time;

    /* Query results */
    JiAutoResult* results;
    int           result_count;
    int           result_capacity;

    /* Screenshot comparison */
    char          last_screenshot_path[512];
    bool          screenshot_match;
    double        screenshot_diff_percent;

    /* Error state */
    char          error_msg[256];
    int           assertion_failures;
} JiAutomation;

/* =========================================================================
 * Automation Lifecycle
 * ========================================================================= */

JI_API JiAutomation* ji_automation_new(void);
JI_API void          ji_automation_free(JiAutomation* auto_ctx);
JI_API void          ji_automation_reset(JiAutomation* auto_ctx);

/* =========================================================================
 * Widget Finding
 * ========================================================================= */

JI_API int ji_automation_find_widgets(JiAutomation* auto_ctx,
                                       JiAutoFindMode mode,
                                       const char* value,
                                       JiAutoResult* results,
                                       int max_results);

JI_API int ji_automation_find_one(JiAutomation* auto_ctx,
                                  JiAutoFindMode mode,
                                  const char* value,
                                  JiAutoResult* result);

/* =========================================================================
 * Input Simulation
 * ========================================================================= */

JI_API void ji_automation_click(JiAutomation* auto_ctx, int x, int y);
JI_API void ji_automation_double_click(JiAutomation* auto_ctx, int x, int y);
JI_API void ji_automation_right_click(JiAutomation* auto_ctx, int x, int y);
JI_API void ji_automation_type_text(JiAutomation* auto_ctx, const char* text);
JI_API void ji_automation_key_press(JiAutomation* auto_ctx, int key_code);
JI_API void ji_automation_mouse_move(JiAutomation* auto_ctx, int x, int y);
JI_API void ji_automation_mouse_drag(JiAutomation* auto_ctx, int x1, int y1, int x2, int y2);
JI_API void ji_automation_scroll(JiAutomation* auto_ctx, int dx, int dy);
JI_API void ji_automation_focus_widget(JiAutomation* auto_ctx, int widget_id);

/* =========================================================================
 * Assertions
 * ========================================================================= */

JI_API bool ji_automation_assert_visible(JiAutomation* auto_ctx, int widget_id);
JI_API bool ji_automation_assert_enabled(JiAutomation* auto_ctx, int widget_id);
JI_API bool ji_automation_assert_text(JiAutomation* auto_ctx, int widget_id,
                                        const char* expected);
JI_API bool ji_automation_assert_bounds(JiAutomation* auto_ctx, int widget_id,
                                         JiRect expected);

/* =========================================================================
 * Recording / Playback
 * ========================================================================= */

JI_API void ji_automation_record_start(JiAutomation* auto_ctx);
JI_API void ji_automation_record_stop(JiAutomation* auto_ctx);
JI_API bool ji_automation_is_recording(JiAutomation* auto_ctx);

JI_API void ji_automation_record_action(JiAutomation* auto_ctx, const JiAutoAction* action);

JI_API void ji_automation_play_start(JiAutomation* auto_ctx);
JI_API void ji_automation_play_stop(JiAutomation* auto_ctx);
JI_API bool ji_automation_is_playing(JiAutomation* auto_ctx);

/** Get next action to play (returns NULL when playback complete). */
JI_API const JiAutoAction* ji_automation_play_next(JiAutomation* auto_ctx);

/* =========================================================================
 * Screenshot Comparison
 * ========================================================================= */

/** Compare two PNG screenshots. Returns true if within tolerance. */
JI_API bool ji_automation_screenshot_compare(JiAutomation* auto_ctx,
                                              const char* baseline_path,
                                              const char* current_path,
                                              double tolerance_percent);

/* =========================================================================
 * Export / Import
 * ========================================================================= */

JI_API int ji_automation_export_actions(JiAutomation* auto_ctx, char* buf, int buf_size);
JI_API bool ji_automation_import_actions(JiAutomation* auto_ctx, const char* json);

/* =========================================================================
 * Error Handling
 * ========================================================================= */

JI_API const char* ji_automation_get_error(JiAutomation* auto_ctx);
JI_API int ji_automation_get_assertion_failures(JiAutomation* auto_ctx);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_AUTOMATION_H */
