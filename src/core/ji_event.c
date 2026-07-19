/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_event.c
 * @brief Implementation of the event handler system.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_event.h"
#include "jiui/ji_memory.h"

#include <string.h>
#include <stdlib.h>

/* =========================================================================
 * Global handler registry — maps name → JiEventHandler
 * ========================================================================= */
typedef struct JiHandlerEntry {
    char*           name;
    JiEventHandler  handler;
} JiHandlerEntry;

static JiHandlerEntry* g_handlers = NULL;
static int            g_handler_count = 0;
static int            g_handler_capacity = 0;

/* =========================================================================
 * Event Bus
 * ========================================================================= */

JI_API void ji_event_bus_init(JiEventBus* bus) {
    if (!bus) return;
    memset(bus, 0, sizeof(JiEventBus));
}

JI_API void ji_event_bus_destroy(JiEventBus* bus) {
    if (!bus) return;
    for (int i = 0; i < bus->count; i++) {
        ji_free(bus->subscriptions[i].event_name);
    }
    ji_free(bus->subscriptions);
    bus->subscriptions = NULL;
    bus->count = 0;
    bus->capacity = 0;
}

JI_API int ji_event_bus_subscribe(JiEventBus* bus,
                                     JiObject* target,
                                     const char* event_name,
                                     JiEventHandler handler,
                                     void* user_data) {
    if (!bus || !target || !event_name || !handler) return -1;

    if (bus->count >= bus->capacity) {
        int new_cap = bus->capacity == 0 ? 8 : bus->capacity * 2;
        bus->subscriptions = (JiEventSubscription*)ji_realloc(bus->subscriptions,
            (size_t)new_cap * sizeof(JiEventSubscription));
        bus->capacity = new_cap;
    }

    JiEventSubscription* sub = &bus->subscriptions[bus->count];
    sub->target = target;
    sub->event_name = strdup(event_name);
    sub->handler = handler;
    sub->user_data = user_data;
    sub->is_active = true;

    return bus->count++;
}

JI_API void ji_event_bus_unsubscribe(JiEventBus* bus, int index) {
    if (!bus || index < 0 || index >= bus->count) return;
    ji_free(bus->subscriptions[index].event_name);
    /* Shift remaining */
    for (int i = index; i < bus->count - 1; i++) {
        bus->subscriptions[i] = bus->subscriptions[i + 1];
    }
    bus->count--;
}

JI_API void ji_event_bus_dispatch(JiEventBus* bus,
                                     JiObject* target,
                                     const char* event_name,
                                     void* args) {
    if (!bus || !target || !event_name) return;

    for (int i = 0; i < bus->count; i++) {
        JiEventSubscription* sub = &bus->subscriptions[i];
        if (sub->is_active && sub->target == target &&
            strcmp(sub->event_name, event_name) == 0) {
            sub->handler(target, args, sub->user_data);
        }
    }
}

JI_API int ji_event_bus_count(const JiEventBus* bus) {
    return bus ? bus->count : 0;
}

/* =========================================================================
 * Global handler registry
 * ========================================================================= */

JI_API void ji_event_register_handler(const char* name, JiEventHandler handler) {
    if (!name || !handler) return;

    /* Update existing */
    for (int i = 0; i < g_handler_count; i++) {
        if (strcmp(g_handlers[i].name, name) == 0) {
            g_handlers[i].handler = handler;
            return;
        }
    }

    /* Add new */
    if (g_handler_count >= g_handler_capacity) {
        int new_cap = g_handler_capacity == 0 ? 16 : g_handler_capacity * 2;
        g_handlers = (JiHandlerEntry*)ji_realloc(g_handlers,
            (size_t)new_cap * sizeof(JiHandlerEntry));
        g_handler_capacity = new_cap;
    }

    g_handlers[g_handler_count].name = strdup(name);
    g_handlers[g_handler_count].handler = handler;
    g_handler_count++;
}

JI_API JiEventHandler ji_event_lookup_handler(const char* name) {
    if (!name) return NULL;
    for (int i = 0; i < g_handler_count; i++) {
        if (strcmp(g_handlers[i].name, name) == 0) {
            return g_handlers[i].handler;
        }
    }
    return NULL;
}
