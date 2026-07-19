/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_threads.h
 * @brief Multi-threaded UI architecture — 4-thread pipeline with lock-free queues.
 *
 * Thread roles:
 *   - UI Thread:      event processing, layout, input
 *   - Render Thread:  GPU commands, scene graph render
 *   - Resource Thread: image decode, font load, SVG parse
 *   - Asset Thread:   file I/O, network, plugin load
 */

#ifndef JIUI_THREADS_H
#define JIUI_THREADS_H

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

#define JI_THREAD_MAX_QUEUES    16
#define JI_THREAD_QUEUE_CAP    256    /* Power-of-2 for ring buffer mask */
#define JI_THREAD_NAME_LEN     32

/* =========================================================================
 * Enums
 * ========================================================================= */

/** Identifies the four pipeline threads. */
typedef enum JiThreadRole {
    JI_THREAD_UI       = 0,
    JI_THREAD_RENDER   = 1,
    JI_THREAD_RESOURCE = 2,
    JI_THREAD_ASSET    = 3,
    JI_THREAD_COUNT    = 4
} JiThreadRole;

/** Priority of a queued command. */
typedef enum JiThreadPriority {
    JI_PRIO_LOW    = 0,
    JI_PRIO_NORMAL = 1,
    JI_PRIO_HIGH   = 2,
    JI_PRIO_URGENT = 3
} JiThreadPriority;

/** State of a thread. */
typedef enum JiThreadState {
    JI_THREAD_STOPPED  = 0,
    JI_THREAD_STARTING  = 1,
    JI_THREAD_RUNNING   = 2,
    JI_THREAD_STOPPING  = 3
} JiThreadState;

/* =========================================================================
 * Thread-Safe Command Queue (lock-free SPSC ring buffer)
 * ========================================================================= */

/** A command sent between threads. */
typedef struct JiThreadCmd {
    uint32_t type;            /**< User-defined command type */
    void*    data;            /**< User-defined payload (ownership transferred) */
    uint32_t data_size;       /**< Size of data in bytes (0 if pointer-only) */
    JiThreadPriority priority;
    uint64_t timestamp;       /**< Monotonic nanoseconds when enqueued */
} JiThreadCmd;

/** Lock-free single-producer / single-consumer ring buffer. */
typedef struct JiThreadQueue {
    JiThreadCmd  ring[JI_THREAD_QUEUE_CAP];
    volatile uint32_t head;   /**< Producer index (atomic) */
    volatile uint32_t tail;   /**< Consumer index (atomic) */
    JiThreadRole producer;
    JiThreadRole consumer;
} JiThreadQueue;

/** Create a queue between two threads. */
JI_API JiThreadQueue* ji_thread_queue_new(JiThreadRole producer, JiThreadRole consumer);

/** Destroy a queue. Does NOT free queued command data. */
JI_API void ji_thread_queue_free(JiThreadQueue* queue);

/** Enqueue a command (non-blocking). Returns 0 on success, -1 if full. */
JI_API int ji_thread_queue_push(JiThreadQueue* queue, const JiThreadCmd* cmd);

/** Dequeue a command (non-blocking). Returns 0 on success, -1 if empty. */
JI_API int ji_thread_queue_pop(JiThreadQueue* queue, JiThreadCmd* out);

/** Returns the number of pending commands. */
JI_API uint32_t ji_thread_queue_count(const JiThreadQueue* queue);

/** Returns true if the queue is empty. */
JI_API bool ji_thread_queue_empty(const JiThreadQueue* queue);

/** Returns true if the queue is full. */
JI_API bool ji_thread_queue_full(const JiThreadQueue* queue);

/* =========================================================================
 * Thread Manager
 * ========================================================================= */

/** Per-thread statistics. */
typedef struct JiThreadStats {
    uint64_t commands_processed;
    uint64_t commands_queued;
    uint64_t idle_cycles;
    uint64_t total_runtime_ns;
} JiThreadStats;

/** Thread entry function signature. */
typedef void (*JiThreadFunc)(void* user_data);

/** Configuration for a single thread. */
typedef struct JiThreadConfig {
    JiThreadRole  role;
    char          name[JI_THREAD_NAME_LEN];
    JiThreadFunc  func;
    void*         user_data;
    bool          enabled;
} JiThreadConfig;

/** The thread manager orchestrating the 4-thread pipeline. */
typedef struct JiThreadManager {
    JiThreadConfig  configs[JI_THREAD_COUNT];
    JiThreadState   states[JI_THREAD_COUNT];
    JiThreadStats   stats[JI_THREAD_COUNT];
    JiThreadQueue*  queues[JI_THREAD_COUNT][JI_THREAD_COUNT]; /* [producer][consumer] */
    void*           threads[JI_THREAD_COUNT];                 /* Opaque pthread_t* */
    volatile bool   running;
    uint64_t        frame_count;
    uint64_t        start_time_ns;
} JiThreadManager;

/** Create the thread manager with default configs. */
JI_API JiThreadManager* ji_thread_manager_new(void);

/** Configure a thread before starting. */
JI_API int ji_thread_manager_configure(JiThreadManager* mgr, JiThreadRole role,
                                  const JiThreadConfig* config);

/** Start all enabled threads. */
JI_API int ji_thread_manager_start(JiThreadManager* mgr);

/** Signal all threads to stop (non-blocking). */
JI_API void ji_thread_manager_stop(JiThreadManager* mgr);

/** Wait for all threads to finish (blocking). */
JI_API void ji_thread_manager_join(JiThreadManager* mgr);

/** Destroy the manager and all queues. */
JI_API void ji_thread_manager_free(JiThreadManager* mgr);

/** Get the queue from producer → consumer. Returns NULL if not created. */
JI_API JiThreadQueue* ji_thread_manager_get_queue(JiThreadManager* mgr,
                                             JiThreadRole producer,
                                             JiThreadRole consumer);

/** Create a queue between two threads. */
JI_API int ji_thread_manager_create_queue(JiThreadManager* mgr,
                                     JiThreadRole producer,
                                     JiThreadRole consumer);

/** Send a command from one thread to another. */
JI_API int ji_thread_manager_send(JiThreadManager* mgr,
                             JiThreadRole from, JiThreadRole to,
                             uint32_t cmd_type, void* data, uint32_t data_size,
                             JiThreadPriority priority);

/** Poll for commands destined to `role` (non-blocking). Returns 0 if cmd found. */
JI_API int ji_thread_manager_poll(JiThreadManager* mgr, JiThreadRole role, JiThreadCmd* out);

/** Get statistics for a thread. */
JI_API const JiThreadStats* ji_thread_manager_get_stats(const JiThreadManager* mgr, JiThreadRole role);

/** Get the state of a thread. */
JI_API JiThreadState ji_thread_manager_get_state(const JiThreadManager* mgr, JiThreadRole role);

/** Check if the manager is running. */
JI_API bool ji_thread_manager_is_running(const JiThreadManager* mgr);

/** Get the current frame count. */
JI_API uint64_t ji_thread_manager_get_frame_count(const JiThreadManager* mgr);

/* =========================================================================
 * Frame Synchronization
 * ========================================================================= */

/** Frame sync point — called by the UI thread to signal a new frame. */
JI_API int ji_thread_frame_begin(JiThreadManager* mgr);

/** Frame end — called by the render thread when presentation is done. */
JI_API int ji_thread_frame_end(JiThreadManager* mgr);

/** Wait for the render thread to finish the current frame (blocking). */
JI_API int ji_thread_frame_wait(JiThreadManager* mgr, uint32_t timeout_ms);

/* =========================================================================
 * Utility
 * ========================================================================= */

/** Get a human-readable name for a thread role. */
JI_API const char* ji_thread_role_name(JiThreadRole role);

/** Get a human-readable name for a thread state. */
JI_API const char* ji_thread_state_name(JiThreadState state);

/** Get the current monotonic time in nanoseconds. */
JI_API uint64_t ji_thread_now_ns(void);

/** Sleep for the given milliseconds. */
JI_API void ji_thread_sleep_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_THREADS_H */
