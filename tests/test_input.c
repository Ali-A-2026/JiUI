/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_input.c
 * @brief Tests for the advanced input engine.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

static int event_count = 0;
static JiInputEventType last_event_type = JI_INPUT_EVENT_NONE;

static void input_callback(const JiInputEvent* event, void* user_data) {
    (void)user_data;
    event_count++;
    last_event_type = event->type;
}

static void test_input_manager_create(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ASSERT_TRUE(mgr != NULL);
    ASSERT_TRUE(ji_input_manager_device_count(mgr) == 0);
    ji_input_manager_destroy(mgr);
}

static void test_mouse_move(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    event_count = 0;
    ji_input_manager_mouse_move(mgr, 100.0f, 200.0f, 0);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_MOUSE_MOVE);

    JiMouseState state;
    ji_input_manager_get_mouse_state(mgr, &state);
    ASSERT_TRUE(state.x == 100.0f);
    ASSERT_TRUE(state.y == 200.0f);

    ji_input_manager_destroy(mgr);
}

static void test_mouse_button(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    event_count = 0;
    ji_input_manager_mouse_button(mgr, JI_MOUSE_LEFT, true, 0);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_MOUSE_BUTTON);
    ASSERT_TRUE(ji_input_manager_is_mouse_button_pressed(mgr, JI_MOUSE_LEFT));

    ji_input_manager_mouse_button(mgr, JI_MOUSE_LEFT, false, 10);
    ASSERT_TRUE(!ji_input_manager_is_mouse_button_pressed(mgr, JI_MOUSE_LEFT));

    ji_input_manager_destroy(mgr);
}

static void test_mouse_wheel(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    event_count = 0;
    ji_input_manager_mouse_wheel(mgr, 0.0f, 10.0f, 0);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_MOUSE_WHEEL);

    JiMouseState state;
    ji_input_manager_get_mouse_state(mgr, &state);
    ASSERT_TRUE(state.wheel_delta_y == 10.0f);

    ji_input_manager_destroy(mgr);
}

static void test_key_event(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    event_count = 0;
    ji_input_manager_key_event(mgr, 65, 0, true, 0); /* 'A' */
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_KEY);
    ASSERT_TRUE(ji_input_manager_is_key_pressed(mgr, 65));

    ji_input_manager_key_event(mgr, 65, 0, false, 10);
    ASSERT_TRUE(!ji_input_manager_is_key_pressed(mgr, 65));

    ji_input_manager_destroy(mgr);
}

static void test_device_hotplug(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    ASSERT_TRUE(ji_input_manager_device_count(mgr) == 0);

    JiInputDevice dev = {0};
    dev.id = 1;
    dev.type = JI_INPUT_DEVICE_MOUSE;
    dev.state = JI_INPUT_DEVICE_STATE_CONNECTED;
    snprintf(dev.name, sizeof(dev.name), "Test Mouse");

    event_count = 0;
    ji_input_manager_device_added(mgr, &dev);
    ASSERT_TRUE(ji_input_manager_device_count(mgr) == 1);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_DEVICE_ADDED);

    JiInputDevice retrieved;
    ASSERT_TRUE(ji_input_manager_get_device(mgr, 0, &retrieved));
    ASSERT_TRUE(retrieved.id == 1);
    ASSERT_TRUE(retrieved.type == JI_INPUT_DEVICE_MOUSE);

    ASSERT_TRUE(ji_input_manager_get_device_by_id(mgr, 1, &retrieved));
    ASSERT_TRUE(retrieved.id == 1);

    ji_input_manager_device_removed(mgr, 1);
    ASSERT_TRUE(ji_input_manager_device_count(mgr) == 0);

    ji_input_manager_destroy(mgr);
}

static void test_device_find(void) {
    JiInputManager* mgr = ji_input_manager_new();

    JiInputDevice mouse1 = {0};
    mouse1.id = 1;
    mouse1.type = JI_INPUT_DEVICE_MOUSE;
    mouse1.state = JI_INPUT_DEVICE_STATE_CONNECTED;

    JiInputDevice gamepad1 = {0};
    gamepad1.id = 2;
    gamepad1.type = JI_INPUT_DEVICE_GAMEPAD;
    gamepad1.state = JI_INPUT_DEVICE_STATE_CONNECTED;

    JiInputDevice gamepad2 = {0};
    gamepad2.id = 3;
    gamepad2.type = JI_INPUT_DEVICE_GAMEPAD;
    gamepad2.state = JI_INPUT_DEVICE_STATE_CONNECTED;

    ji_input_manager_device_added(mgr, &mouse1);
    ji_input_manager_device_added(mgr, &gamepad1);
    ji_input_manager_device_added(mgr, &gamepad2);

    JiInputDevice results[4];
    uint32_t count = ji_input_manager_find_devices(mgr, JI_INPUT_DEVICE_GAMEPAD, results, 4);
    ASSERT_TRUE(count == 2);

    count = ji_input_manager_find_devices(mgr, JI_INPUT_DEVICE_MOUSE, results, 4);
    ASSERT_TRUE(count == 1);

    count = ji_input_manager_find_devices(mgr, JI_INPUT_DEVICE_PEN, results, 4);
    ASSERT_TRUE(count == 0);

    ji_input_manager_destroy(mgr);
}

static void test_gesture_manager_integration(void) {
    JiInputManager* mgr = ji_input_manager_new();

    JiGestureManager* gm = ji_input_manager_get_gesture_manager(mgr);
    ASSERT_TRUE(gm != NULL);

    JiGestureRecognizer* tap = ji_gesture_recognizer_new(JI_GESTURE_TAP);
    ji_gesture_manager_add(gm, tap);
    ASSERT_TRUE(ji_gesture_manager_count(gm) == 1);

    ji_input_manager_destroy(mgr);
    ji_gesture_recognizer_destroy(tap);
}

static void test_gamepad_axis(void) {
    JiInputManager* mgr = ji_input_manager_new();
    ji_input_manager_set_callback(mgr, input_callback, NULL);

    event_count = 0;
    ji_input_manager_gamepad_axis(mgr, 0, 0, 0.5f, 0);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_GAMEPAD_AXIS);

    float x, y;
    ji_input_manager_gamepad_left_stick(mgr, 0, &x, &y);
    ASSERT_TRUE(x == 0.5f);

    ji_input_manager_gamepad_axis(mgr, 0, 1, -0.3f, 10);
    ji_input_manager_gamepad_left_stick(mgr, 0, &x, &y);
    ASSERT_TRUE(y == -0.3f);

    ji_input_manager_destroy(mgr);
}

static void test_device_callback(void) {
    JiInputManager* mgr = ji_input_manager_new();

    event_count = 0;
    ji_input_manager_set_device_callback(mgr, JI_INPUT_DEVICE_MOUSE, input_callback, NULL);

    ji_input_manager_mouse_move(mgr, 50.0f, 50.0f, 0);
    ASSERT_TRUE(event_count == 1);
    ASSERT_TRUE(last_event_type == JI_INPUT_EVENT_MOUSE_MOVE);

    ji_input_manager_destroy(mgr);
}

int main(void) {
    printf("=== Input Engine Tests ===\n");
    TEST(test_input_manager_create);
    TEST(test_mouse_move);
    TEST(test_mouse_button);
    TEST(test_mouse_wheel);
    TEST(test_key_event);
    TEST(test_device_hotplug);
    TEST(test_device_find);
    TEST(test_gesture_manager_integration);
    TEST(test_gamepad_axis);
    TEST(test_device_callback);
    printf("=== %d/%d tests passed ===\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
