/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_hot_reload.c
 * @brief Tests for the file watcher and hot reload engine.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

/* =========================================================================
 * File Watcher Tests
 * ========================================================================= */

static void test_file_watcher_create(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);
    ASSERT_TRUE(ji_file_watcher_count(w) == 0);
    ji_file_watcher_destroy(w);
}

static void test_file_watcher_destroy_null(void) {
    /* Should not crash */
    ji_file_watcher_destroy(NULL);
}

static void test_file_watcher_add_remove(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);

    /* Create a temp file */
    const char* tmpfile = "/tmp/ji_test_watcher.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow title=\"test\" />");
        fclose(f);
    }

    bool ok = ji_file_watcher_add(w, tmpfile);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_file_watcher_count(w) == 1);

    /* Adding same file again should not duplicate */
    ok = ji_file_watcher_add(w, tmpfile);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_file_watcher_count(w) == 1);

    /* Remove */
    ok = ji_file_watcher_remove(w, tmpfile);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_file_watcher_count(w) == 0);

    /* Remove again should fail */
    ok = ji_file_watcher_remove(w, tmpfile);
    ASSERT_TRUE(!ok);

    /* Cleanup */
    unlink(tmpfile);
    ji_file_watcher_destroy(w);
}

static void test_file_watcher_add_null(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);

    ASSERT_TRUE(!ji_file_watcher_add(w, NULL));
    ASSERT_TRUE(!ji_file_watcher_add(NULL, "/tmp/test"));
    ASSERT_TRUE(!ji_file_watcher_remove(w, NULL));
    ASSERT_TRUE(!ji_file_watcher_remove(NULL, "/tmp/test"));

    ji_file_watcher_destroy(w);
}

static int test_poll_callback_count = 0;
static void test_poll_callback(const JiFileNotification* notif, void* user_data) {
    (void)user_data;
    (void)notif;
    test_poll_callback_count++;
}

static void test_file_watcher_poll_no_changes(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);

    const char* tmpfile = "/tmp/ji_test_poll.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow />");
        fclose(f);
    }

    ji_file_watcher_add(w, tmpfile);

    /* First poll establishes baseline, should not report changes */
    test_poll_callback_count = 0;
    int events = ji_file_watcher_poll(w, test_poll_callback, NULL);
    ASSERT_TRUE(events == 0);

    /* Second poll with no changes */
    events = ji_file_watcher_poll(w, test_poll_callback, NULL);
    ASSERT_TRUE(events == 0);

    unlink(tmpfile);
    ji_file_watcher_destroy(w);
}

static void test_file_watcher_poll_modified(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);

    const char* tmpfile = "/tmp/ji_test_modify.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow title=\"old\" />");
        fclose(f);
    }

    ji_file_watcher_add(w, tmpfile);

    /* Establish baseline */
    ji_file_watcher_poll(w, test_poll_callback, NULL);

    /* Sleep 1.1s to ensure mtime changes (stat has 1-second resolution) */
    sleep(1);
    usleep(100000);

    /* Modify the file with different content (different size) */
    f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow title=\"new title here\" />");
        fclose(f);
    }

    /* Poll should detect the change (mtime or size difference) */
    test_poll_callback_count = 0;
    int events = ji_file_watcher_poll(w, test_poll_callback, NULL);
    ASSERT_TRUE(events >= 1);
    ASSERT_TRUE(test_poll_callback_count >= 1);

    unlink(tmpfile);
    ji_file_watcher_destroy(w);
}

static void test_file_watcher_poll_deleted(void) {
    JiFileWatcher* w = ji_file_watcher_new();
    ASSERT_TRUE(w != NULL);

    const char* tmpfile = "/tmp/ji_test_delete.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow />");
        fclose(f);
    }

    ji_file_watcher_add(w, tmpfile);
    ji_file_watcher_poll(w, test_poll_callback, NULL);

    /* Delete the file */
    unlink(tmpfile);

    test_poll_callback_count = 0;
    int events = ji_file_watcher_poll(w, test_poll_callback, NULL);
    ASSERT_TRUE(events >= 1);

    ji_file_watcher_destroy(w);
}

/* =========================================================================
 * Hot Reload Engine Tests
 * ========================================================================= */

static void test_hot_reload_create(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);
    ASSERT_TRUE(ji_hot_reload_is_enabled(engine) == true);
    ASSERT_TRUE(ji_hot_reload_watched_count(engine) == 0);
    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_destroy_null(void) {
    ji_hot_reload_destroy(NULL);
}

static void test_hot_reload_enable_disable(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    ASSERT_TRUE(ji_hot_reload_is_enabled(engine) == true);
    ji_hot_reload_set_enabled(engine, false);
    ASSERT_TRUE(ji_hot_reload_is_enabled(engine) == false);
    ji_hot_reload_set_enabled(engine, true);
    ASSERT_TRUE(ji_hot_reload_is_enabled(engine) == true);

    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_watch_unwatch(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    const char* tmpfile = "/tmp/ji_test_hr_watch.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow title=\"test\" />");
        fclose(f);
    }

    bool ok = ji_hot_reload_watch(engine, tmpfile, NULL);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_hot_reload_watched_count(engine) == 1);

    /* Watching same file again should be ok */
    ok = ji_hot_reload_watch(engine, tmpfile, NULL);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_hot_reload_watched_count(engine) == 1);

    /* Unwatch */
    ok = ji_hot_reload_unwatch(engine, tmpfile);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(ji_hot_reload_watched_count(engine) == 0);

    /* Unwatch again should fail */
    ok = ji_hot_reload_unwatch(engine, tmpfile);
    ASSERT_TRUE(!ok);

    unlink(tmpfile);
    ji_hot_reload_destroy(engine);
}

static int hr_callback_count = 0;
static JiHotReloadResult hr_last_result;
static void hr_test_callback(const JiHotReloadResult* result,
                               JiObject* new_root,
                               void* user_data) {
    (void)new_root;
    (void)user_data;
    hr_callback_count++;
    if (result) {
        hr_last_result = *result;
    }
}

static void test_hot_reload_callback(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    ji_hot_reload_set_callback(engine, hr_test_callback, NULL);

    const char* tmpfile = "/tmp/ji_test_hr_callback.jiml";
    FILE* f = fopen(tmpfile, "w");
    if (f) {
        fprintf(f, "<JiWindow title=\"test\" />");
        fclose(f);
    }

    hr_callback_count = 0;
    ji_hot_reload_watch(engine, tmpfile, NULL);
    /* Initial load may trigger callback or not depending on implementation */

    /* Force a reload */
    hr_callback_count = 0;
    JiHotReloadResult result = ji_hot_reload_reload(engine, tmpfile);
    ASSERT_TRUE(hr_callback_count >= 1);

    unlink(tmpfile);
    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_reload_all(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    const char* tmpfile1 = "/tmp/ji_test_hr_all1.jiml";
    const char* tmpfile2 = "/tmp/ji_test_hr_all2.jiml";

    FILE* f1 = fopen(tmpfile1, "w");
    if (f1) { fprintf(f1, "<JiWindow />"); fclose(f1); }
    FILE* f2 = fopen(tmpfile2, "w");
    if (f2) { fprintf(f2, "<JiFrame />"); fclose(f2); }

    ji_hot_reload_watch(engine, tmpfile1, NULL);
    ji_hot_reload_watch(engine, tmpfile2, NULL);
    ASSERT_TRUE(ji_hot_reload_watched_count(engine) == 2);

    int count = ji_hot_reload_reload_all(engine);
    ASSERT_TRUE(count >= 0);

    unlink(tmpfile1);
    unlink(tmpfile2);
    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_get_root(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    /* Non-existent file */
    JiObject* root = ji_hot_reload_get_root(engine, "/tmp/nonexistent.jiml");
    ASSERT_TRUE(root == NULL);

    /* NULL engine */
    root = ji_hot_reload_get_root(NULL, "/tmp/test.jiml");
    ASSERT_TRUE(root == NULL);

    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_get_last_error(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    JiHotReloadResult err = ji_hot_reload_get_last_error(engine, "/tmp/nonexistent.jiml");
    ASSERT_TRUE(err.level == JI_HOT_RELOAD_OK);

    err = ji_hot_reload_get_last_error(NULL, "/tmp/test.jiml");
    ASSERT_TRUE(err.level == JI_HOT_RELOAD_OK);

    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_poll_disabled(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    ji_hot_reload_set_enabled(engine, false);

    /* Poll when disabled should return 0 */
    int count = ji_hot_reload_poll(engine);
    ASSERT_TRUE(count == 0);

    ji_hot_reload_destroy(engine);
}

static void test_hot_reload_state_preservation(void) {
    JiHotReloadEngine* engine = ji_hot_reload_new();
    ASSERT_TRUE(engine != NULL);

    /* save_state and restore_state with NULL should not crash */
    bool ok = ji_hot_reload_save_state(engine, NULL, "/tmp/test.jiml");
    ASSERT_TRUE(!ok);

    ok = ji_hot_reload_restore_state(engine, NULL, "/tmp/test.jiml");
    ASSERT_TRUE(!ok);

    ji_hot_reload_destroy(engine);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== Hot Reload Tests ===\n");

    printf("-- File Watcher Tests --\n");
    TEST(test_file_watcher_create);
    TEST(test_file_watcher_destroy_null);
    TEST(test_file_watcher_add_remove);
    TEST(test_file_watcher_add_null);
    TEST(test_file_watcher_poll_no_changes);
    TEST(test_file_watcher_poll_modified);
    TEST(test_file_watcher_poll_deleted);

    printf("-- Hot Reload Engine Tests --\n");
    TEST(test_hot_reload_create);
    TEST(test_hot_reload_destroy_null);
    TEST(test_hot_reload_enable_disable);
    TEST(test_hot_reload_watch_unwatch);
    TEST(test_hot_reload_callback);
    TEST(test_hot_reload_reload_all);
    TEST(test_hot_reload_get_root);
    TEST(test_hot_reload_get_last_error);
    TEST(test_hot_reload_poll_disabled);
    TEST(test_hot_reload_state_preservation);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_run == tests_passed) ? 0 : 1;
}
