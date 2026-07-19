/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_threads.c
 * @brief Multi-threaded UI architecture implementation.
 */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include "jiui/ji_threads.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

/* =========================================================================
 * Utility
 * ========================================================================= */

uint64_t ji_thread_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void ji_thread_sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

const char* ji_thread_role_name(JiThreadRole role)
{
    switch (role) {
        case JI_THREAD_UI:       return "UI";
        case JI_THREAD_RENDER:   return "Render";
        case JI_THREAD_RESOURCE: return "Resource";
        case JI_THREAD_ASSET:    return "Asset";
        default: return "Unknown";
    }
}

const char* ji_thread_state_name(JiThreadState state)
{
    switch (state) {
        case JI_THREAD_STOPPED:  return "Stopped";
        case JI_THREAD_STARTING: return "Starting";
        case JI_THREAD_RUNNING:  return "Running";
        case JI_THREAD_STOPPING: return "Stopping";
        default: return "Unknown";
    }
}

/* =========================================================================
 * Thread-Safe Command Queue (lock-free SPSC ring buffer)
 * ========================================================================= */

JiThreadQueue* ji_thread_queue_new(JiThreadRole producer, JiThreadRole consumer)
{
    JiThreadQueue* q = (JiThreadQueue*)calloc(1, sizeof(JiThreadQueue));
    if (!q) return NULL;
    q->producer = producer;
    q->consumer = consumer;
    q->head = 0;
    q->tail = 0;
    return q;
}

void ji_thread_queue_free(JiThreadQueue* queue)
{
    /* Note: does NOT free queued command data — caller must drain first */
    free(queue);
}

int ji_thread_queue_push(JiThreadQueue* queue, const JiThreadCmd* cmd)
{
    if (!queue || !cmd) return -1;
    uint32_t head = queue->head;
    uint32_t next = (head + 1) & (JI_THREAD_QUEUE_CAP - 1);
    if (next == queue->tail) return -1;  /* Full */
    queue->ring[head] = *cmd;
    queue->ring[head].timestamp = ji_thread_now_ns();
    __atomic_store_n(&queue->head, next, __ATOMIC_RELEASE);
    return 0;
}

int ji_thread_queue_pop(JiThreadQueue* queue, JiThreadCmd* out)
{
    if (!queue || !out) return -1;
    uint32_t tail = queue->tail;
    if (tail == queue->head) return -1;  /* Empty */
    *out = queue->ring[tail];
    __atomic_store_n(&queue->tail, (tail + 1) & (JI_THREAD_QUEUE_CAP - 1), __ATOMIC_RELEASE);
    return 0;
}

uint32_t ji_thread_queue_count(const JiThreadQueue* queue)
{
    if (!queue) return 0;
    uint32_t h = __atomic_load_n(&queue->head, __ATOMIC_ACQUIRE);
    uint32_t t = __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE);
    return (h - t) & (JI_THREAD_QUEUE_CAP - 1);
}

bool ji_thread_queue_empty(const JiThreadQueue* queue)
{
    if (!queue) return true;
    uint32_t h = __atomic_load_n(&queue->head, __ATOMIC_ACQUIRE);
    uint32_t t = __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE);
    return h == t;
}

bool ji_thread_queue_full(const JiThreadQueue* queue)
{
    if (!queue) return false;
    uint32_t h = __atomic_load_n(&queue->head, __ATOMIC_ACQUIRE);
    uint32_t t = __atomic_load_n(&queue->tail, __ATOMIC_ACQUIRE);
    return ((h + 1) & (JI_THREAD_QUEUE_CAP - 1)) == t;
}

/* =========================================================================
 * Thread wrapper
 * ========================================================================= */

typedef struct JiThreadWrapper {
    JiThreadManager* mgr;
    JiThreadRole     role;
    JiThreadFunc     func;
    void*            user_data;
} JiThreadWrapper;

static void* ji_thread_entry(void* arg)
{
    JiThreadWrapper* w = (JiThreadWrapper*)arg;
    JiThreadManager* mgr = w->mgr;
    JiThreadRole role = w->role;

    mgr->states[role] = JI_THREAD_RUNNING;
    mgr->stats[role].total_runtime_ns = ji_thread_now_ns();

    /* Run the user function */
    if (w->func) {
        w->func(w->user_data);
    }

    /* Drain remaining commands for this thread */
    JiThreadCmd cmd;
    for (int p = 0; p < JI_THREAD_COUNT; p++) {
        JiThreadQueue* q = mgr->queues[p][role];
        while (q && ji_thread_queue_pop(q, &cmd) == 0) {
            if (cmd.data && cmd.data_size > 0) {
                free(cmd.data);
            }
            mgr->stats[role].commands_processed++;
        }
    }

    mgr->stats[role].total_runtime_ns = ji_thread_now_ns() - mgr->stats[role].total_runtime_ns;
    mgr->states[role] = JI_THREAD_STOPPED;
    free(w);
    return NULL;
}

/* =========================================================================
 * Thread Manager
 * ========================================================================= */

JiThreadManager* ji_thread_manager_new(void)
{
    JiThreadManager* mgr = (JiThreadManager*)calloc(1, sizeof(JiThreadManager));
    if (!mgr) return NULL;

    /* Default configs */
    mgr->configs[JI_THREAD_UI].role = JI_THREAD_UI;
    strcpy(mgr->configs[JI_THREAD_UI].name, "UI Thread");
    mgr->configs[JI_THREAD_UI].enabled = true;

    mgr->configs[JI_THREAD_RENDER].role = JI_THREAD_RENDER;
    strcpy(mgr->configs[JI_THREAD_RENDER].name, "Render Thread");
    mgr->configs[JI_THREAD_RENDER].enabled = true;

    mgr->configs[JI_THREAD_RESOURCE].role = JI_THREAD_RESOURCE;
    strcpy(mgr->configs[JI_THREAD_RESOURCE].name, "Resource Thread");
    mgr->configs[JI_THREAD_RESOURCE].enabled = true;

    mgr->configs[JI_THREAD_ASSET].role = JI_THREAD_ASSET;
    strcpy(mgr->configs[JI_THREAD_ASSET].name, "Asset Thread");
    mgr->configs[JI_THREAD_ASSET].enabled = true;

    mgr->running = false;
    mgr->frame_count = 0;
    mgr->start_time_ns = 0;

    for (int i = 0; i < JI_THREAD_COUNT; i++) {
        mgr->states[i] = JI_THREAD_STOPPED;
        mgr->threads[i] = NULL;
    }

    return mgr;
}

int ji_thread_manager_configure(JiThreadManager* mgr, JiThreadRole role,
                                  const JiThreadConfig* config)
{
    if (!mgr || role < 0 || role >= JI_THREAD_COUNT || !config) return -1;
    if (mgr->running) return -1;  /* Can't configure while running */
    mgr->configs[role] = *config;
    mgr->configs[role].role = role;  /* Ensure role matches */
    return 0;
}

int ji_thread_manager_create_queue(JiThreadManager* mgr,
                                     JiThreadRole producer,
                                     JiThreadRole consumer)
{
    if (!mgr || producer < 0 || producer >= JI_THREAD_COUNT ||
        consumer < 0 || consumer >= JI_THREAD_COUNT) return -1;
    if (mgr->queues[producer][consumer]) return 0;  /* Already exists */
    mgr->queues[producer][consumer] = ji_thread_queue_new(producer, consumer);
    return mgr->queues[producer][consumer] ? 0 : -1;
}

JiThreadQueue* ji_thread_manager_get_queue(JiThreadManager* mgr,
                                             JiThreadRole producer,
                                             JiThreadRole consumer)
{
    if (!mgr || producer < 0 || producer >= JI_THREAD_COUNT ||
        consumer < 0 || consumer >= JI_THREAD_COUNT) return NULL;
    return mgr->queues[producer][consumer];
}

int ji_thread_manager_start(JiThreadManager* mgr)
{
    if (!mgr || mgr->running) return -1;

    /* Create default queues if none exist: each thread can send to every other */
    for (int p = 0; p < JI_THREAD_COUNT; p++) {
        for (int c = 0; c < JI_THREAD_COUNT; c++) {
            if (p != c && !mgr->queues[p][c]) {
                ji_thread_manager_create_queue(mgr, (JiThreadRole)p, (JiThreadRole)c);
            }
        }
    }

    mgr->running = true;
    mgr->start_time_ns = ji_thread_now_ns();

    /* Start threads */
    for (int i = 0; i < JI_THREAD_COUNT; i++) {
        if (!mgr->configs[i].enabled) continue;

        JiThreadWrapper* w = (JiThreadWrapper*)malloc(sizeof(JiThreadWrapper));
        if (!w) return -1;
        w->mgr = mgr;
        w->role = (JiThreadRole)i;
        w->func = mgr->configs[i].func;
        w->user_data = mgr->configs[i].user_data;

        mgr->states[i] = JI_THREAD_STARTING;

        pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t));
        if (!tid) { free(w); return -1; }

        int rc = pthread_create(tid, NULL, ji_thread_entry, w);
        if (rc != 0) {
            free(w);
            free(tid);
            mgr->states[i] = JI_THREAD_STOPPED;
            return -1;
        }
        mgr->threads[i] = tid;
    }

    return 0;
}

void ji_thread_manager_stop(JiThreadManager* mgr)
{
    if (!mgr) return;
    mgr->running = false;
    for (int i = 0; i < JI_THREAD_COUNT; i++) {
        if (mgr->states[i] == JI_THREAD_RUNNING) {
            mgr->states[i] = JI_THREAD_STOPPING;
        }
    }
}

void ji_thread_manager_join(JiThreadManager* mgr)
{
    if (!mgr) return;
    for (int i = 0; i < JI_THREAD_COUNT; i++) {
        if (mgr->threads[i]) {
            pthread_join(*(pthread_t*)mgr->threads[i], NULL);
            free(mgr->threads[i]);
            mgr->threads[i] = NULL;
        }
    }
}

void ji_thread_manager_free(JiThreadManager* mgr)
{
    if (!mgr) return;
    /* Ensure threads are stopped and joined */
    if (mgr->running) {
        ji_thread_manager_stop(mgr);
        ji_thread_manager_join(mgr);
    }
    /* Free all queues */
    for (int p = 0; p < JI_THREAD_COUNT; p++) {
        for (int c = 0; c < JI_THREAD_COUNT; c++) {
            if (mgr->queues[p][c]) {
                ji_thread_queue_free(mgr->queues[p][c]);
                mgr->queues[p][c] = NULL;
            }
        }
    }
    free(mgr);
}

int ji_thread_manager_send(JiThreadManager* mgr,
                             JiThreadRole from, JiThreadRole to,
                             uint32_t cmd_type, void* data, uint32_t data_size,
                             JiThreadPriority priority)
{
    if (!mgr) return -1;
    JiThreadQueue* q = ji_thread_manager_get_queue(mgr, from, to);
    if (!q) return -1;
    JiThreadCmd cmd;
    cmd.type = cmd_type;
    cmd.data = data;
    cmd.data_size = data_size;
    cmd.priority = priority;
    cmd.timestamp = 0;  /* Set by push */
    int rc = ji_thread_queue_push(q, &cmd);
    if (rc == 0) {
        mgr->stats[from].commands_queued++;
    }
    return rc;
}

int ji_thread_manager_poll(JiThreadManager* mgr, JiThreadRole role, JiThreadCmd* out)
{
    if (!mgr || !out) return -1;
    /* Check all queues where `role` is the consumer */
    for (int p = 0; p < JI_THREAD_COUNT; p++) {
        JiThreadQueue* q = mgr->queues[p][role];
        if (q && ji_thread_queue_pop(q, out) == 0) {
            mgr->stats[role].commands_processed++;
            return 0;
        }
    }
    mgr->stats[role].idle_cycles++;
    return -1;
}

const JiThreadStats* ji_thread_manager_get_stats(const JiThreadManager* mgr, JiThreadRole role)
{
    if (!mgr || role < 0 || role >= JI_THREAD_COUNT) return NULL;
    return &mgr->stats[role];
}

JiThreadState ji_thread_manager_get_state(const JiThreadManager* mgr, JiThreadRole role)
{
    if (!mgr || role < 0 || role >= JI_THREAD_COUNT) return JI_THREAD_STOPPED;
    return mgr->states[role];
}

bool ji_thread_manager_is_running(const JiThreadManager* mgr)
{
    return mgr ? mgr->running : false;
}

uint64_t ji_thread_manager_get_frame_count(const JiThreadManager* mgr)
{
    return mgr ? mgr->frame_count : 0;
}

/* =========================================================================
 * Frame Synchronization
 * ========================================================================= */

static pthread_mutex_t g_frame_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   g_frame_cond  = PTHREAD_COND_INITIALIZER;
static bool             g_frame_ready  = false;

int ji_thread_frame_begin(JiThreadManager* mgr)
{
    if (!mgr) return -1;
    mgr->frame_count++;
    pthread_mutex_lock(&g_frame_mutex);
    g_frame_ready = true;
    pthread_cond_signal(&g_frame_cond);
    pthread_mutex_unlock(&g_frame_mutex);
    return 0;
}

int ji_thread_frame_end(JiThreadManager* mgr)
{
    if (!mgr) return -1;
    /* Signal that rendering is complete */
    pthread_mutex_lock(&g_frame_mutex);
    g_frame_ready = false;
    pthread_cond_signal(&g_frame_cond);
    pthread_mutex_unlock(&g_frame_mutex);
    return 0;
}

int ji_thread_frame_wait(JiThreadManager* mgr, uint32_t timeout_ms)
{
    if (!mgr) return -1;
    pthread_mutex_lock(&g_frame_mutex);
    int rc = 0;
    if (g_frame_ready && timeout_ms > 0) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec  += timeout_ms / 1000;
        ts.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000L;
        }
        rc = pthread_cond_timedwait(&g_frame_cond, &g_frame_mutex, &ts);
    }
    pthread_mutex_unlock(&g_frame_mutex);
    return rc == 0 ? 0 : -1;
}
