/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_event.h
 * @brief Event handler system for XML-declared event bindings.
 *
 * Supports connecting XML event attributes (e.g. Click="onButtonClick")
 * to C callback functions registered at runtime.
 */

#ifndef JIUI_EVENT_H
#define JIUI_EVENT_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_object.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Event callback type
 * ========================================================================= */

/** Generic event callback. */
typedef void (*JiEventHandler)(JiObject* sender, void* args, void* user_data);

/* =========================================================================
 * Event descriptor — describes a named event on an object type
 * ========================================================================= */
typedef struct JiEventDesc {
    char*           name;       /* Event name (e.g. "Click", "TextChanged") */
    JiTypeId        owner_type; /* Type that declares this event */
} JiEventDesc;

/* =========================================================================
 * Event subscription — connects a handler to an event on an object
 * ========================================================================= */
typedef struct JiEventSubscription {
    JiObject*       target;      /* Object the event is on */
    char*           event_name;  /* Event name */
    JiEventHandler  handler;     /* Callback function */
    void*           user_data;   /* User data passed to callback */
    bool            is_active;   /* Whether the subscription is active */
} JiEventSubscription;

/* =========================================================================
 * Event bus — manages all event subscriptions and dispatching
 * ========================================================================= */
typedef struct JiEventBus {
    JiEventSubscription* subscriptions;
    int                 count;
    int                 capacity;
} JiEventBus;

/* =========================================================================
 * Event API
 * ========================================================================= */

/** Initialize an event bus. */
JI_API void ji_event_bus_init(JiEventBus* bus);

/** Destroy an event bus and free all subscriptions. */
JI_API void ji_event_bus_destroy(JiEventBus* bus);

/**
 * Subscribe a handler to an event on a target object.
 * @return Subscription index (>=0) or -1 on failure.
 */
JI_API int ji_event_bus_subscribe(JiEventBus* bus,
                                     JiObject* target,
                                     const char* event_name,
                                     JiEventHandler handler,
                                     void* user_data);

/**
 * Unsubscribe by index.
 */
JI_API void ji_event_bus_unsubscribe(JiEventBus* bus, int index);

/**
 * Dispatch an event on a target object.
 * Calls all handlers subscribed to the given event name on that object.
 */
JI_API void ji_event_bus_dispatch(JiEventBus* bus,
                                     JiObject* target,
                                     const char* event_name,
                                     void* args);

/** Get the number of active subscriptions. */
JI_API int ji_event_bus_count(const JiEventBus* bus);

/**
 * Register a named handler function that can be looked up by name.
 * This allows XML attributes like Click="onButtonClick" to resolve
 * to the correct C function at load time.
 */
JI_API void ji_event_register_handler(const char* name, JiEventHandler handler);

/**
 * Look up a registered handler by name.
 * @return The handler, or NULL if not found.
 */
JI_API JiEventHandler ji_event_lookup_handler(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_EVENT_H */
