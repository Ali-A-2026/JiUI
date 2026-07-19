/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_gesture.c
 * @brief Tests for the gesture recognition framework.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

static JiGestureType last_gesture_type = JI_GESTURE_NONE;
static JiGestureState last_gesture_state = JI_GESTURE_STATE_IDLE;
static int gesture_callback_count = 0;

static void gesture_callback(const JiGestureEvent* event, void* user_data) {
    (void)user_data;
    last_gesture_type = event->type;
    last_gesture_state = event->state;
    gesture_callback_count++;
}

static void test_recognizer_create(void) {
    JiGestureRecognizer* rec = ji_gesture_recognizer_new(JI_GESTURE_TAP);
    ASSERT_TRUE(rec != NULL);
    ASSERT_TRUE(ji_gesture_recognizer_get_type(rec) == JI_GESTURE_TAP);
    ASSERT_TRUE(ji_gesture_recognizer_get_state(rec) == JI_GESTURE_STATE_IDLE);

    ji_gesture_recognizer_set_priority(rec, 10);
    ASSERT_TRUE(ji_gesture_recognizer_get_priority(rec) == 10);

    ji_gesture_recognizer_set_simultaneous(rec, true);
    ASSERT_TRUE(ji_gesture_recognizer_get_simultaneous(rec) == true);

    ji_gesture_recognizer_destroy(rec);
}

static void test_tap_gesture(void) {
    JiGestureManager* mgr = ji_gesture_manager_new();
    ASSERT_TRUE(mgr != NULL);

    JiGestureRecognizer* tap = ji_gesture_recognizer_new(JI_GESTURE_TAP);
    ji_gesture_recognizer_set_callback(tap, gesture_callback, NULL);
    ji_gesture_manager_add(mgr, tap);

    gesture_callback_count = 0;
    last_gesture_type = JI_GESTURE_NONE;

    /* Simulate a quick tap */
    JiTouchPoint down = {1, 100.0f, 100.0f, 1.0f, 0};
    ji_gesture_manager_touch_down(mgr, &down);

    JiTouchPoint up = {1, 102.0f, 101.0f, 1.0f, 50};
    ji_gesture_manager_touch_up(mgr, 1, 50);

    ASSERT_TRUE(gesture_callback_count > 0);
    ASSERT_TRUE(last_gesture_type == JI_GESTURE_TAP);
    ASSERT_TRUE(last_gesture_state == JI_GESTURE_STATE_ENDED);

    ji_gesture_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(tap);
}

static void test_swipe_gesture(void) {
    JiGestureManager* mgr = ji_gesture_manager_new();
    JiGestureRecognizer* swipe = ji_gesture_recognizer_new(JI_GESTURE_SWIPE);
    ji_gesture_recognizer_set_callback(swipe, gesture_callback, NULL);
    ji_gesture_swipe_set_min_distance(swipe, 50.0f);
    ji_gesture_manager_add(mgr, swipe);

    gesture_callback_count = 0;
    last_gesture_type = JI_GESTURE_NONE;

    /* Simulate a right swipe */
    JiTouchPoint down = {1, 0.0f, 100.0f, 1.0f, 0};
    ji_gesture_manager_touch_down(mgr, &down);

    JiTouchPoint move = {1, 100.0f, 100.0f, 1.0f, 100};
    ji_gesture_manager_touch_move(mgr, &move);

    ASSERT_TRUE(gesture_callback_count > 0);
    ASSERT_TRUE(last_gesture_type == JI_GESTURE_SWIPE);
    ASSERT_TRUE(last_gesture_state == JI_GESTURE_STATE_ENDED);

    ji_gesture_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(swipe);
}

static void test_drag_gesture(void) {
    JiGestureManager* mgr = ji_gesture_manager_new();
    JiGestureRecognizer* drag = ji_gesture_recognizer_new(JI_GESTURE_DRAG);
    ji_gesture_recognizer_set_callback(drag, gesture_callback, NULL);
    ji_gesture_drag_set_min_distance(drag, 10.0f);
    ji_gesture_manager_add(mgr, drag);

    gesture_callback_count = 0;

    /* Simulate drag */
    JiTouchPoint down = {1, 50.0f, 50.0f, 1.0f, 0};
    ji_gesture_manager_touch_down(mgr, &down);

    JiTouchPoint move1 = {1, 70.0f, 50.0f, 1.0f, 100};
    ji_gesture_manager_touch_move(mgr, &move1);

    ASSERT_TRUE(gesture_callback_count > 0);
    ASSERT_TRUE(last_gesture_state == JI_GESTURE_STATE_BEGAN ||
                last_gesture_state == JI_GESTURE_STATE_CHANGED);

    JiTouchPoint move2 = {1, 90.0f, 50.0f, 1.0f, 200};
    ji_gesture_manager_touch_move(mgr, &move2);

    ji_gesture_manager_touch_up(mgr, 1, 300);
    ASSERT_TRUE(last_gesture_state == JI_GESTURE_STATE_ENDED);

    ji_gesture_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(drag);
}

static void test_pinch_gesture(void) {
    JiGestureManager* mgr = ji_gesture_manager_new();
    JiGestureRecognizer* pinch = ji_gesture_recognizer_new(JI_GESTURE_PINCH);
    ji_gesture_recognizer_set_callback(pinch, gesture_callback, NULL);
    ji_gesture_pinch_set_min_scale(pinch, 0.1f);
    ji_gesture_manager_add(mgr, pinch);

    gesture_callback_count = 0;

    /* Two touches moving apart */
    JiTouchPoint t1_down = {1, 100.0f, 100.0f, 1.0f, 0};
    ji_gesture_manager_touch_down(mgr, &t1_down);

    JiTouchPoint t2_down = {2, 110.0f, 100.0f, 1.0f, 10};
    ji_gesture_manager_touch_down(mgr, &t2_down);

    JiTouchPoint t1_move = {1, 50.0f, 100.0f, 1.0f, 100};
    ji_gesture_manager_touch_move(mgr, &t1_move);

    JiTouchPoint t2_move = {2, 160.0f, 100.0f, 1.0f, 110};
    ji_gesture_manager_touch_move(mgr, &t2_move);

    ASSERT_TRUE(gesture_callback_count > 0);
    ASSERT_TRUE(last_gesture_type == JI_GESTURE_PINCH);

    ji_gesture_manager_touch_up(mgr, 1, 200);
    ji_gesture_manager_touch_up(mgr, 2, 210);

    ji_gesture_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(pinch);
}

static void test_manager_count(void) {
    JiGestureManager* mgr = ji_gesture_manager_new();
    ASSERT_TRUE(ji_gesture_manager_count(mgr) == 0);

    JiGestureRecognizer* r1 = ji_gesture_recognizer_new(JI_GESTURE_TAP);
    JiGestureRecognizer* r2 = ji_gesture_recognizer_new(JI_GESTURE_SWIPE);
    ji_gesture_manager_add(mgr, r1);
    ji_gesture_manager_add(mgr, r2);
    ASSERT_TRUE(ji_gesture_manager_count(mgr) == 2);

    ji_gesture_manager_remove(mgr, r1);
    ASSERT_TRUE(ji_gesture_manager_count(mgr) == 1);

    ji_gesture_manager_clear(mgr);
    ASSERT_TRUE(ji_gesture_manager_count(mgr) == 0);

    ji_gesture_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(r1);
    ji_gesture_recognizer_destroy(r2);
}

static void test_gesture_template(void) {
    JiGestureTemplate* tmpl = ji_gesture_template_new("circle");
    ASSERT_TRUE(tmpl != NULL);
    ASSERT_TRUE(ji_gesture_template_point_count(tmpl) == 0);

    /* Record a simple path */
    ji_gesture_template_add_point(tmpl, 0.0f, 0.0f, 0);
    ji_gesture_template_add_point(tmpl, 10.0f, 0.0f, 10);
    ji_gesture_template_add_point(tmpl, 20.0f, 0.0f, 20);
    ji_gesture_template_add_point(tmpl, 30.0f, 0.0f, 30);

    ASSERT_TRUE(ji_gesture_template_point_count(tmpl) == 4);

    /* Match with a similar path */
    JiTouchPoint points[4];
    points[0] = (JiTouchPoint){0, 0.0f, 0.0f, 1.0f, 0};
    points[1] = (JiTouchPoint){0, 10.0f, 0.0f, 1.0f, 10};
    points[2] = (JiTouchPoint){0, 20.0f, 0.0f, 1.0f, 20};
    points[3] = (JiTouchPoint){0, 30.0f, 0.0f, 1.0f, 30};

    float confidence = ji_gesture_template_match(tmpl, points, 4);
    ASSERT_TRUE(confidence > 0.0f);

    ji_gesture_template_destroy(tmpl);
}

static void test_config_setters(void) {
    JiGestureRecognizer* rec = ji_gesture_recognizer_new(JI_GESTURE_TAP);
    ji_gesture_tap_set_max_duration(rec, 500);
    ji_gesture_tap_set_max_movement(rec, 15.0f);

    JiGestureRecognizer* lp = ji_gesture_recognizer_new(JI_GESTURE_LONG_PRESS);
    ji_gesture_long_press_set_min_duration(lp, 1000);
    ji_gesture_long_press_set_max_movement(lp, 20.0f);

    JiGestureRecognizer* sw = ji_gesture_recognizer_new(JI_GESTURE_SWIPE);
    ji_gesture_swipe_set_directions(sw, JI_SWIPE_LEFT | JI_SWIPE_RIGHT);
    ji_gesture_swipe_set_min_distance(sw, 100.0f);
    ji_gesture_swipe_set_min_velocity(sw, 200.0f);

    JiGestureRecognizer* dg = ji_gesture_recognizer_new(JI_GESTURE_DRAG);
    ji_gesture_drag_set_min_distance(dg, 5.0f);
    ji_gesture_drag_set_inertia(dg, true);
    ji_gesture_drag_set_friction(dg, 0.9f);

    ji_gesture_recognizer_destroy(rec);
    ji_gesture_recognizer_destroy(lp);
    ji_gesture_recognizer_destroy(sw);
    ji_gesture_recognizer_destroy(dg);
}

int main(void) {
    printf("=== Gesture Recognition Tests ===\n");
    TEST(test_recognizer_create);
    TEST(test_tap_gesture);
    TEST(test_swipe_gesture);
    TEST(test_drag_gesture);
    TEST(test_pinch_gesture);
    TEST(test_manager_count);
    TEST(test_gesture_template);
    TEST(test_config_setters);
    printf("=== %d/%d tests passed ===\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
