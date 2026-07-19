#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_automation_record.c
 * @brief Action recording and macro playback for UI automation.
 */

#include "jiui/ji_automation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* =========================================================================
 * Time utility
 * ========================================================================= */

static double ji_rec_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =========================================================================
 * Macro Recording — higher-level recording helpers
 * ========================================================================= */

/* This file provides extended recording utilities that supplement the
 * core automation API in ji_automation.c. The core recording functions
 * (ji_automation_record_start/stop, ji_automation_record_action) are
 * implemented in ji_automation.c. This file adds macro management. */

/** Macro structure for saved recordings. */
typedef struct JiAutoMacro {
    char name[64];
    JiAutoAction* actions;
    int   action_count;
} JiAutoMacro;

#define JI_MAX_MACROS 32
static JiAutoMacro g_macros[JI_MAX_MACROS];
static int g_macro_count = 0;

/** Save current recording as a named macro. */
bool ji_automation_save_macro(JiAutomation* auto_ctx, const char* name)
{
    if (!auto_ctx || !name) return false;
    if (g_macro_count >= JI_MAX_MACROS) return false;

    JiAutoMacro* m = &g_macros[g_macro_count++];
    strncpy(m->name, name, sizeof(m->name) - 1);
    m->action_count = auto_ctx->action_count;
    m->actions = (JiAutoAction*)malloc(m->action_count * sizeof(JiAutoAction));
    if (!m->actions) {
        g_macro_count--;
        return false;
    }
    memcpy(m->actions, auto_ctx->actions, m->action_count * sizeof(JiAutoAction));
    return true;
}

/** Load a named macro into the automation context for playback. */
bool ji_automation_load_macro(JiAutomation* auto_ctx, const char* name)
{
    if (!auto_ctx || !name) return false;
    for (int i = 0; i < g_macro_count; i++) {
        if (strcmp(g_macros[i].name, name) == 0) {
            JiAutoMacro* m = &g_macros[i];
            /* Copy actions into context */
            if (m->action_count > auto_ctx->action_capacity) {
                int new_cap = m->action_count;
                JiAutoAction* new_arr = (JiAutoAction*)realloc(auto_ctx->actions,
                    new_cap * sizeof(JiAutoAction));
                if (!new_arr) return false;
                auto_ctx->actions = new_arr;
                auto_ctx->action_capacity = new_cap;
            }
            memcpy(auto_ctx->actions, m->actions, m->action_count * sizeof(JiAutoAction));
            auto_ctx->action_count = m->action_count;
            return true;
        }
    }
    return false;
}

/** Delete a named macro. */
bool ji_automation_delete_macro(const char* name)
{
    if (!name) return false;
    for (int i = 0; i < g_macro_count; i++) {
        if (strcmp(g_macros[i].name, name) == 0) {
            free(g_macros[i].actions);
            /* Shift remaining macros down */
            for (int j = i; j < g_macro_count - 1; j++)
                g_macros[j] = g_macros[j + 1];
            g_macro_count--;
            return true;
        }
    }
    return false;
}

/** Get list of macro names. */
int ji_automation_list_macros(char names[][64], int max)
{
    int count = (g_macro_count < max) ? g_macro_count : max;
    for (int i = 0; i < count; i++)
        strncpy(names[i], g_macros[i].name, 63);
    return count;
}

/** Free all macros. */
void ji_automation_free_macros(void)
{
    for (int i = 0; i < g_macro_count; i++) {
        free(g_macros[i].actions);
        g_macros[i].actions = NULL;
        g_macros[i].action_count = 0;
    }
    g_macro_count = 0;
}
