/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_threads.c
 * @brief Tests for the multi-threaded architecture.
 */

#include "jiui/ji_threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* =========================================================================
 * Test Framework
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    static void name(void); \
    static void name(void)

#define RUN_TEST(name) do { \
    g_tests_run++; \
    printf("  [RUN] %s ... ", #name); \
    name(); \
    g_tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return; \
    } \
} while(0)

/* =========================================================================
 * Queue Tests
 * ========================================================================= */

TEST(test_queue_create)
{
    JiThreadQueue* q = ji_thread_queue_new(JI_THREAD_UI, JI_THREAD_RENDER);
    ASSERT(q != NULL);
    ASSERT(ji_thread_queue_empty(q) == true);
    ASSERT(ji_thread_queue_full(q) == false);
    ASSERT(ji_thread_queue_count(q) == 0);
    ji_thread_queue_free(q);
}

TEST(test_queue_push_pop)
{
    JiThreadQueue* q = ji_thread_queue_new(JI_THREAD_UI, JI_THREAD_RENDER);
    ASSERT(q != NULL);

    JiThreadCmd cmd;
    cmd.type = 42;
    cmd.data = (void*)0xDEADBEEF;
    cmd.data_size = 0;
    cmd.priority = JI_PRIO_NORMAL;

    ASSERT(ji_thread_queue_push(q, &cmd) == 0);
    ASSERT(ji_thread_queue_count(q) == 1);
    ASSERT(ji_thread_queue_empty(q) == false);

    JiThreadCmd out;
    ASSERT(ji_thread_queue_pop(q, &out) == 0);
    ASSERT(out.type == 42);
    ASSERT(out.data == (void*)0xDEADBEEF);
    ASSERT(ji_thread_queue_empty(q) == true);

    ji_thread_queue_free(q);
}

TEST(test_queue_fill)
{
    JiThreadQueue* q = ji_thread_queue_new(JI_THREAD_UI, JI_THREAD_RENDER);
    ASSERT(q != NULL);

    /* Fill the queue */
    JiThreadCmd cmd;
    cmd.type = 1;
    cmd.data = NULL;
    cmd.data_size = 0;
    cmd.priority = JI_PRIO_NORMAL;

    int pushed = 0;
    while (ji_thread_queue_push(q, &cmd) == 0) {
        pushed++;
    }
    ASSERT(pushed == JI_THREAD_QUEUE_CAP - 1);
    ASSERT(ji_thread_queue_full(q) == true);

    /* Pop all */
    JiThreadCmd out;
    int popped = 0;
    while (ji_thread_queue_pop(q, &out) == 0) {
        popped++;
    }
    ASSERT(popped == pushed);
    ASSERT(ji_thread_queue_empty(q) == true);

    ji_thread_queue_free(q);
}

/* =========================================================================
 * Thread Manager Tests
 * ========================================================================= */

static volatile int g_worker_count = 0;

static void worker_func(void* user_data)
{
    volatile bool* running = (volatile bool*)user_data;
    int local_count = 0;
    while (*running) {
        local_count++;
        ji_thread_sleep_ms(1);
        if (local_count > 100) break;
    }
    __atomic_add_fetch(&g_worker_count, 1, __ATOMIC_SEQ_CST);
}

TEST(test_manager_create)
{
    JiThreadManager* mgr = ji_thread_manager_new();
    ASSERT(mgr != NULL);
    ASSERT(ji_thread_manager_is_running(mgr) == false);
    ASSERT(ji_thread_manager_get_frame_count(mgr) == 0);

    for (int i = 0; i < JI_THREAD_COUNT; i++) {
        ASSERT(ji_thread_manager_get_state(mgr, (JiThreadRole)i) == JI_THREAD_STOPPED);
    }

    ji_thread_manager_free(mgr);
}

TEST(test_manager_start_stop)
{
    JiThreadManager* mgr = ji_thread_manager_new();
    ASSERT(mgr != NULL);

    /* Configure a simple worker on the UI thread */
    static volatile bool running = true;
    JiThreadConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.role = JI_THREAD_UI;
    cfg.func = worker_func;
    cfg.user_data = (void*)&running;
    cfg.enabled = true;
    strcpy(cfg.name, "TestWorker");
    ASSERT(ji_thread_manager_configure(mgr, JI_THREAD_UI, &cfg) == 0);

    /* Disable other threads */
    cfg.enabled = false;
    cfg.func = NULL;
    ji_thread_manager_configure(mgr, JI_THREAD_RENDER, &cfg);
    ji_thread_manager_configure(mgr, JI_THREAD_RESOURCE, &cfg);
    ji_thread_manager_configure(mgr, JI_THREAD_ASSET, &cfg);

    g_worker_count = 0;
    ASSERT(ji_thread_manager_start(mgr) == 0);
    ASSERT(ji_thread_manager_is_running(mgr) == true);

    ji_thread_sleep_ms(50);

    running = false;
    ji_thread_manager_stop(mgr);
    ji_thread_manager_join(mgr);

    ASSERT(g_worker_count >= 1);
    ASSERT(ji_thread_manager_get_state(mgr, JI_THREAD_UI) == JI_THREAD_STOPPED);

    ji_thread_manager_free(mgr);
    running = true;  /* Reset for next test */
}

TEST(test_manager_send_poll)
{
    JiThreadManager* mgr = ji_thread_manager_new();
    ASSERT(mgr != NULL);

    /* Create a queue from UI → Render */
    ASSERT(ji_thread_manager_create_queue(mgr, JI_THREAD_UI, JI_THREAD_RENDER) == 0);

    /* Send a command */
    int payload = 42;
    ASSERT(ji_thread_manager_send(mgr, JI_THREAD_UI, JI_THREAD_RENDER,
                                    100, &payload, sizeof(payload), JI_PRIO_NORMAL) == 0);

    /* Poll from the Render thread's perspective */
    JiThreadCmd cmd;
    ASSERT(ji_thread_manager_poll(mgr, JI_THREAD_RENDER, &cmd) == 0);
    ASSERT(cmd.type == 100);
    ASSERT(cmd.data_size == sizeof(payload));
    ASSERT(*(int*)cmd.data == 42);

    /* Queue should be empty now */
    ASSERT(ji_thread_manager_poll(mgr, JI_THREAD_RENDER, &cmd) == -1);

    ji_thread_manager_free(mgr);
}

TEST(test_manager_stats)
{
    JiThreadManager* mgr = ji_thread_manager_new();
    ASSERT(mgr != NULL);

    ASSERT(ji_thread_manager_create_queue(mgr, JI_THREAD_UI, JI_THREAD_RENDER) == 0);

    /* Send a few commands */
    for (int i = 0; i < 5; i++) {
        ji_thread_manager_send(mgr, JI_THREAD_UI, JI_THREAD_RENDER,
                                 i, NULL, 0, JI_PRIO_NORMAL);
    }

    const JiThreadStats* stats = ji_thread_manager_get_stats(mgr, JI_THREAD_UI);
    ASSERT(stats != NULL);
    ASSERT(stats->commands_queued == 5);

    /* Poll all */
    JiThreadCmd cmd;
    while (ji_thread_manager_poll(mgr, JI_THREAD_RENDER, &cmd) == 0) {
        /* Just drain */
    }

    stats = ji_thread_manager_get_stats(mgr, JI_THREAD_RENDER);
    ASSERT(stats != NULL);
    ASSERT(stats->commands_processed == 5);

    ji_thread_manager_free(mgr);
}

/* =========================================================================
 * Utility Tests
 * ========================================================================= */

TEST(test_util_names)
{
    ASSERT(strcmp(ji_thread_role_name(JI_THREAD_UI), "UI") == 0);
    ASSERT(strcmp(ji_thread_role_name(JI_THREAD_RENDER), "Render") == 0);
    ASSERT(strcmp(ji_thread_role_name(JI_THREAD_RESOURCE), "Resource") == 0);
    ASSERT(strcmp(ji_thread_role_name(JI_THREAD_ASSET), "Asset") == 0);

    ASSERT(strcmp(ji_thread_state_name(JI_THREAD_STOPPED), "Stopped") == 0);
    ASSERT(strcmp(ji_thread_state_name(JI_THREAD_RUNNING), "Running") == 0);
}

TEST(test_util_time)
{
    uint64_t t1 = ji_thread_now_ns();
    ji_thread_sleep_ms(10);
    uint64_t t2 = ji_thread_now_ns();
    ASSERT(t2 > t1);
    ASSERT(t2 - t1 >= 5000000);  /* At least 5ms */
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Thread Architecture Tests ===\n");
    RUN_TEST(test_queue_create);
    RUN_TEST(test_queue_push_pop);
    RUN_TEST(test_queue_fill);
    RUN_TEST(test_manager_create);
    RUN_TEST(test_manager_start_stop);
    RUN_TEST(test_manager_send_poll);
    RUN_TEST(test_manager_stats);
    RUN_TEST(test_util_names);
    RUN_TEST(test_util_time);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
