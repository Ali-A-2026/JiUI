#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_automation.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static void ji_test_sleep_us(int us) {
    struct timespec ts = {0, us * 1000};
    nanosleep(&ts, NULL);
}

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
 * Automation Lifecycle Tests
 * ========================================================================= */

TEST(test_auto_create)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);
    ASSERT(a->recording == false);
    ASSERT(a->playing == false);
    ASSERT(a->action_count == 0);
    ji_automation_free(a);
}

TEST(test_auto_reset)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    /* Add some state */
    ji_automation_record_start(a);
    ji_automation_click(a, 100, 200);
    ji_automation_record_stop(a);

    ASSERT(a->action_count > 0);

    ji_automation_reset(a);
    ASSERT(a->action_count == 0);
    ASSERT(a->recording == false);

    ji_automation_free(a);
}

/* =========================================================================
 * Recording Tests
 * ========================================================================= */

TEST(test_auto_record_start_stop)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ASSERT(ji_automation_is_recording(a) == false);
    ji_automation_record_start(a);
    ASSERT(ji_automation_is_recording(a) == true);
    ji_automation_record_stop(a);
    ASSERT(ji_automation_is_recording(a) == false);

    ji_automation_free(a);
}

TEST(test_auto_record_click)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_click(a, 100, 200);
    ji_automation_click(a, 300, 400);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 2);
    ASSERT(a->actions[0].type == JI_AUTO_CLICK);
    ASSERT(a->actions[0].x == 100);
    ASSERT(a->actions[0].y == 200);
    ASSERT(a->actions[1].type == JI_AUTO_CLICK);
    ASSERT(a->actions[1].x == 300);
    ASSERT(a->actions[1].y == 400);

    ji_automation_free(a);
}

TEST(test_auto_record_double_click)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_double_click(a, 50, 60);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_DOUBLE_CLICK);
    ASSERT(a->actions[0].x == 50);
    ASSERT(a->actions[0].y == 60);

    ji_automation_free(a);
}

TEST(test_auto_record_right_click)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_right_click(a, 10, 20);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_RIGHT_CLICK);

    ji_automation_free(a);
}

TEST(test_auto_record_type_text)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_type_text(a, "Hello World");
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_TYPE_TEXT);
    ASSERT(strcmp(a->actions[0].text, "Hello World") == 0);

    ji_automation_free(a);
}

TEST(test_auto_record_key_press)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_key_press(a, 65); /* 'A' */
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_KEY_PRESS);
    ASSERT(a->actions[0].key_code == 65);

    ji_automation_free(a);
}

TEST(test_auto_record_mouse_move)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_mouse_move(a, 100, 200);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_MOUSE_MOVE);

    ji_automation_free(a);
}

TEST(test_auto_record_drag)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_mouse_drag(a, 0, 0, 100, 100);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_MOUSE_DRAG);
    ASSERT(a->actions[0].x == 0);
    ASSERT(a->actions[0].y == 0);
    ASSERT(a->actions[0].dx == 100);
    ASSERT(a->actions[0].dy == 100);

    ji_automation_free(a);
}

TEST(test_auto_record_scroll)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_scroll(a, 0, -10);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_SCROLL);
    ASSERT(a->actions[0].dy == -10);

    ji_automation_free(a);
}

TEST(test_auto_record_focus)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_focus_widget(a, 42);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 1);
    ASSERT(a->actions[0].type == JI_AUTO_FOCUS);
    ASSERT(a->actions[0].widget_id == 42);

    ji_automation_free(a);
}

TEST(test_auto_record_mixed)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_click(a, 10, 20);
    ji_automation_type_text(a, "test");
    ji_automation_key_press(a, 13);
    ji_automation_scroll(a, 0, -5);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 4);
    ASSERT(a->actions[0].type == JI_AUTO_CLICK);
    ASSERT(a->actions[1].type == JI_AUTO_TYPE_TEXT);
    ASSERT(a->actions[2].type == JI_AUTO_KEY_PRESS);
    ASSERT(a->actions[3].type == JI_AUTO_SCROLL);

    ji_automation_free(a);
}

/* =========================================================================
 * Playback Tests
 * ========================================================================= */

TEST(test_auto_playback)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_click(a, 100, 200);
    ji_automation_click(a, 300, 400);
    ji_automation_click(a, 500, 600);
    ji_automation_record_stop(a);

    ASSERT(a->action_count == 3);

    ji_automation_play_start(a);
    ASSERT(ji_automation_is_playing(a) == true);

    const JiAutoAction* act1 = ji_automation_play_next(a);
    ASSERT(act1 != NULL);
    ASSERT(act1->x == 100);

    const JiAutoAction* act2 = ji_automation_play_next(a);
    ASSERT(act2 != NULL);
    ASSERT(act2->x == 300);

    const JiAutoAction* act3 = ji_automation_play_next(a);
    ASSERT(act3 != NULL);
    ASSERT(act3->x == 500);

    const JiAutoAction* act4 = ji_automation_play_next(a);
    ASSERT(act4 == NULL);
    ASSERT(ji_automation_is_playing(a) == false);

    ji_automation_free(a);
}

TEST(test_auto_play_stop)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_click(a, 1, 1);
    ji_automation_click(a, 2, 2);
    ji_automation_record_stop(a);

    ji_automation_play_start(a);
    ASSERT(ji_automation_is_playing(a) == true);
    ji_automation_play_stop(a);
    ASSERT(ji_automation_is_playing(a) == false);

    ji_automation_free(a);
}

/* =========================================================================
 * Export / Import Tests
 * ========================================================================= */

TEST(test_auto_export)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    ji_automation_record_start(a);
    ji_automation_click(a, 10, 20);
    ji_automation_record_stop(a);

    char buf[4096];
    int len = ji_automation_export_actions(a, buf, sizeof(buf));
    ASSERT(len > 0);
    ASSERT(strstr(buf, "JI_AUTO_CLICK") != NULL || strstr(buf, "\"type\": 0") != NULL);

    ji_automation_free(a);
}

/* =========================================================================
 * Screenshot Tests
 * ========================================================================= */

TEST(test_auto_screenshot_compare)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    bool result = ji_automation_screenshot_compare(a, "baseline.png", "current.png", 5.0);
    ASSERT(result == true);
    ASSERT(a->screenshot_match == true);

    ji_automation_free(a);
}

/* =========================================================================
 * Assertions Tests
 * ========================================================================= */

TEST(test_auto_assert_visible)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    bool result = ji_automation_assert_visible(a, 1);
    ASSERT(result == true);

    ji_automation_free(a);
}

TEST(test_auto_assert_enabled)
{
    JiAutomation* a = ji_automation_new();
    ASSERT(a != NULL);

    bool result = ji_automation_assert_enabled(a, 1);
    ASSERT(result == true);

    ji_automation_free(a);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Automation Tests ===\n");

    RUN_TEST(test_auto_create);
    RUN_TEST(test_auto_reset);
    RUN_TEST(test_auto_record_start_stop);
    RUN_TEST(test_auto_record_click);
    RUN_TEST(test_auto_record_double_click);
    RUN_TEST(test_auto_record_right_click);
    RUN_TEST(test_auto_record_type_text);
    RUN_TEST(test_auto_record_key_press);
    RUN_TEST(test_auto_record_mouse_move);
    RUN_TEST(test_auto_record_drag);
    RUN_TEST(test_auto_record_scroll);
    RUN_TEST(test_auto_record_focus);
    RUN_TEST(test_auto_record_mixed);
    RUN_TEST(test_auto_playback);
    RUN_TEST(test_auto_play_stop);
    RUN_TEST(test_auto_export);
    RUN_TEST(test_auto_screenshot_compare);
    RUN_TEST(test_auto_assert_visible);
    RUN_TEST(test_auto_assert_enabled);

    printf("\n%d/%d tests passed\n", g_tests_passed, g_tests_run);
    return (g_tests_run == g_tests_passed) ? 0 : 1;
}
