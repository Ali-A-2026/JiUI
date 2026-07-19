/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_dock_widget.c
 * @brief Dock widget implementation.
 */

#include <jiui/ji_dock_widget.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- Lifecycle ---- */

JiDockWidget* ji_dock_widget_new(const char* name, const char* title) {
    JiDockWidget* widget = (JiDockWidget*)ji_calloc(1, sizeof(JiDockWidget));
    if (!widget) {
        JI_ERROR_LOG("ji_dock_widget_new: out of memory");
        return NULL;
    }

    if (name) {
        size_t len = strlen(name);
        widget->name = (char*)ji_alloc(len + 1);
        if (widget->name) memcpy(widget->name, name, len + 1);
    }

    if (title) {
        size_t len = strlen(title);
        widget->title = (char*)ji_alloc(len + 1);
        if (widget->title) memcpy(widget->title, title, len + 1);
    }

    widget->state = JI_DOCK_DOCKED;
    widget->features = JI_DOCK_FEATURE_ALL;
    widget->title_bar_height = 20;
    widget->is_active = false;
    widget->is_floating = false;

    return widget;
}

void ji_dock_widget_destroy(JiDockWidget* widget) {
    if (!widget) return;
    ji_free(widget->name);
    ji_free(widget->title);
    /* Note: does not destroy content control — caller manages that */
    ji_free(widget);
}

/* ---- Properties ---- */

void ji_dock_widget_set_title(JiDockWidget* widget, const char* title) {
    if (!widget) return;
    ji_free(widget->title);
    if (title) {
        size_t len = strlen(title);
        widget->title = (char*)ji_alloc(len + 1);
        if (widget->title) memcpy(widget->title, title, len + 1);
    } else {
        widget->title = NULL;
    }
}

const char* ji_dock_widget_get_title(const JiDockWidget* widget) {
    return widget ? widget->title : NULL;
}

void ji_dock_widget_set_widget(JiDockWidget* widget, JiControl* content) {
    if (!widget) return;
    widget->content = content;
}

JiControl* ji_dock_widget_get_widget(const JiDockWidget* widget) {
    return widget ? widget->content : NULL;
}

void ji_dock_widget_set_features(JiDockWidget* widget, int features) {
    if (!widget) return;
    widget->features = features;
}

int ji_dock_widget_get_features(const JiDockWidget* widget) {
    return widget ? widget->features : 0;
}

void ji_dock_widget_set_name(JiDockWidget* widget, const char* name) {
    if (!widget) return;
    ji_free(widget->name);
    if (name) {
        size_t len = strlen(name);
        widget->name = (char*)ji_alloc(len + 1);
        if (widget->name) memcpy(widget->name, name, len + 1);
    } else {
        widget->name = NULL;
    }
}

const char* ji_dock_widget_get_name(const JiDockWidget* widget) {
    return widget ? widget->name : NULL;
}

/* ---- State ---- */

void ji_dock_widget_set_floating(JiDockWidget* widget, bool floating) {
    if (!widget) return;
    if (floating) {
        widget->state = JI_DOCK_FLOATING;
        widget->is_floating = true;
    } else {
        widget->state = JI_DOCK_DOCKED;
        widget->is_floating = false;
    }
}

bool ji_dock_widget_is_floating(const JiDockWidget* widget) {
    return widget ? widget->is_floating : false;
}

void ji_dock_widget_hide(JiDockWidget* widget) {
    if (!widget) return;
    widget->state = JI_DOCK_HIDDEN;
}

void ji_dock_widget_show(JiDockWidget* widget) {
    if (!widget) return;
    if (widget->state == JI_DOCK_HIDDEN) {
        widget->state = widget->is_floating ? JI_DOCK_FLOATING : JI_DOCK_DOCKED;
    }
}

bool ji_dock_widget_is_hidden(const JiDockWidget* widget) {
    return widget ? (widget->state == JI_DOCK_HIDDEN) : false;
}
