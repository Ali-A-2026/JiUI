/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_controls.c
 * @brief Tests for the control base class and core widgets.
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---- Minimal test framework ---- */
static int s_tests_run    = 0;
static int s_tests_passed = 0;
static int s_tests_failed = 0;

#define TEST_ASSERT(cond, msg)                                              \
    do {                                                                     \
        s_tests_run++;                                                       \
        if (cond) {                                                          \
            s_tests_passed++;                                                \
        } else {                                                             \
            s_tests_failed++;                                                \
            fprintf(stderr, "  FAIL [%s:%d]: %s\n", __func__, __LINE__, msg); \
        }                                                                    \
    } while (0)

#define TEST_FLOAT_EQ(a, b, eps, msg)                                       \
    do {                                                                     \
        s_tests_run++;                                                       \
        if (fabs((a) - (b)) < (eps)) {                                     \
            s_tests_passed++;                                                \
        } else {                                                             \
            s_tests_failed++;                                                \
            fprintf(stderr, "  FAIL [%s:%d]: %s (got %f, expected %f)\n",   \
                    __func__, __LINE__, msg, (double)(a), (double)(b));       \
        }                                                                    \
    } while (0)

/* =========================================================================
 * Control tests
 * ========================================================================= */
static void test_control_create_destroy(void) {
    JiControl* c = ji_control_new();
    TEST_ASSERT(c != NULL, "Control should be created");
    TEST_ASSERT(c->is_enabled == true, "Default should be enabled");
    TEST_ASSERT(c->is_focusable == true, "Default should be focusable");
    TEST_ASSERT(c->is_focused == false, "Default should not be focused");
    TEST_ASSERT(c->tab_index == 0, "Default tab index should be 0");
    TEST_ASSERT(c->name == NULL, "Default name should be NULL");
    TEST_ASSERT(c->tooltip_text == NULL, "Default tooltip should be NULL");
    ji_control_destroy(c);
}

static void test_control_enabled(void) {
    JiControl* c = ji_control_new();
    ji_control_set_enabled(c, false);
    TEST_ASSERT(ji_control_is_enabled(c) == false, "Should be disabled");
    ji_control_set_enabled(c, true);
    TEST_ASSERT(ji_control_is_enabled(c) == true, "Should be enabled");
    ji_control_destroy(c);
}

static void test_control_focus(void) {
    JiControl* c = ji_control_new();
    ji_control_set_focusable(c, false);
    TEST_ASSERT(ji_control_is_focusable(c) == false, "Should not be focusable");
    ji_control_set_focused(c, true);
    TEST_ASSERT(ji_control_is_focused(c) == true, "Should be focused");
    ji_control_destroy(c);
}

static void test_control_name(void) {
    JiControl* c = ji_control_new();
    ji_control_set_name(c, "myButton");
    TEST_ASSERT(strcmp(ji_control_get_name(c), "myButton") == 0,
                "Name should be 'myButton'");
    ji_control_set_name(c, "newName");
    TEST_ASSERT(strcmp(ji_control_get_name(c), "newName") == 0,
                "Name should be 'newName'");
    ji_control_set_name(c, NULL);
    TEST_ASSERT(ji_control_get_name(c) == NULL, "Name should be NULL after clear");
    ji_control_destroy(c);
}

static void test_control_tooltip(void) {
    JiControl* c = ji_control_new();
    ji_control_set_tooltip(c, "Click me");
    TEST_ASSERT(strcmp(ji_control_get_tooltip(c), "Click me") == 0,
                "Tooltip should be 'Click me'");
    ji_control_destroy(c);
}

static void test_control_type_id(void) {
    JiTypeId tid = ji_control_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiControl type should be registered");
}

/* =========================================================================
 * Button tests
 * ========================================================================= */
static void test_button_create_destroy(void) {
    JiButton* btn = ji_button_new("Hello");
    TEST_ASSERT(btn != NULL, "Button should be created");
    TEST_ASSERT(strcmp(ji_button_get_text(btn), "Hello") == 0,
                "Button text should be 'Hello'");
    TEST_ASSERT(btn->is_hovered == false, "Default should not be hovered");
    TEST_ASSERT(btn->is_pressed == false, "Default should not be pressed");
    ji_button_destroy(btn);
}

static void test_button_set_text(void) {
    JiButton* btn = ji_button_new("Old");
    ji_button_set_text(btn, "New");
    TEST_ASSERT(strcmp(ji_button_get_text(btn), "New") == 0,
                "Button text should be 'New'");
    ji_button_destroy(btn);
}

static void test_button_type_id(void) {
    JiTypeId tid = ji_button_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiButton type should be registered");
}

/* =========================================================================
 * Label tests
 * ========================================================================= */
static void test_label_create_destroy(void) {
    JiLabel* label = ji_label_new("Hello World");
    TEST_ASSERT(label != NULL, "Label should be created");
    TEST_ASSERT(strcmp(ji_label_get_text(label), "Hello World") == 0,
                "Label text should be 'Hello World'");
    TEST_FLOAT_EQ(ji_label_get_font_size(label), 14.0, 1e-9,
                  "Default font size should be 14");
    ji_label_destroy(label);
}

static void test_label_set_text(void) {
    JiLabel* label = ji_label_new("Old");
    ji_label_set_text(label, "New");
    TEST_ASSERT(strcmp(ji_label_get_text(label), "New") == 0,
                "Label text should be 'New'");
    ji_label_destroy(label);
}

static void test_label_font_size(void) {
    JiLabel* label = ji_label_new("Test");
    ji_label_set_font_size(label, 24.0);
    TEST_FLOAT_EQ(ji_label_get_font_size(label), 24.0, 1e-9,
                  "Font size should be 24");
    ji_label_destroy(label);
}

static void test_label_type_id(void) {
    JiTypeId tid = ji_label_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiLabel type should be registered");
}

/* =========================================================================
 * TextBox tests
 * ========================================================================= */
static void test_text_box_create_destroy(void) {
    JiTextBox* tb = ji_text_box_new();
    TEST_ASSERT(tb != NULL, "TextBox should be created");
    TEST_ASSERT(strcmp(ji_text_box_get_text(tb), "") == 0,
                "Default text should be empty");
    TEST_ASSERT(ji_text_box_is_read_only(tb) == false, "Default should not be read-only");
    TEST_ASSERT(ji_text_box_is_password(tb) == false, "Default should not be password");
    TEST_ASSERT(ji_text_box_get_max_length(tb) == 0, "Default max length should be 0");
    ji_text_box_destroy(tb);
}

static void test_text_box_set_text(void) {
    JiTextBox* tb = ji_text_box_new();
    ji_text_box_set_text(tb, "Hello");
    TEST_ASSERT(strcmp(ji_text_box_get_text(tb), "Hello") == 0,
                "Text should be 'Hello'");
    ji_text_box_set_text(tb, "");
    TEST_ASSERT(strcmp(ji_text_box_get_text(tb), "") == 0,
                "Text should be empty");
    ji_text_box_destroy(tb);
}

static void test_text_box_placeholder(void) {
    JiTextBox* tb = ji_text_box_new();
    ji_text_box_set_placeholder(tb, "Enter text...");
    TEST_ASSERT(strcmp(ji_text_box_get_placeholder(tb), "Enter text...") == 0,
                "Placeholder should be 'Enter text...'");
    ji_text_box_destroy(tb);
}

static void test_text_box_read_only(void) {
    JiTextBox* tb = ji_text_box_new();
    ji_text_box_set_read_only(tb, true);
    TEST_ASSERT(ji_text_box_is_read_only(tb) == true, "Should be read-only");
    ji_text_box_destroy(tb);
}

static void test_text_box_password(void) {
    JiTextBox* tb = ji_text_box_new();
    ji_text_box_set_password(tb, true);
    TEST_ASSERT(ji_text_box_is_password(tb) == true, "Should be password mode");
    ji_text_box_destroy(tb);
}

static void test_text_box_max_length(void) {
    JiTextBox* tb = ji_text_box_new();
    ji_text_box_set_max_length(tb, 100);
    TEST_ASSERT(ji_text_box_get_max_length(tb) == 100, "Max length should be 100");
    ji_text_box_destroy(tb);
}

static void test_text_box_type_id(void) {
    JiTypeId tid = ji_text_box_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiTextBox type should be registered");
}

/* =========================================================================
 * CheckBox tests
 * ========================================================================= */
static void test_check_box_create_destroy(void) {
    JiCheckBox* cb = ji_check_box_new("Accept");
    TEST_ASSERT(cb != NULL, "CheckBox should be created");
    TEST_ASSERT(strcmp(ji_check_box_get_text(cb), "Accept") == 0,
                "CheckBox text should be 'Accept'");
    TEST_ASSERT(ji_check_box_is_checked(cb) == false, "Default should be unchecked");
    ji_check_box_destroy(cb);
}

static void test_check_box_checked(void) {
    JiCheckBox* cb = ji_check_box_new("Test");
    ji_check_box_set_checked(cb, true);
    TEST_ASSERT(ji_check_box_is_checked(cb) == true, "Should be checked");
    ji_check_box_set_checked(cb, false);
    TEST_ASSERT(ji_check_box_is_checked(cb) == false, "Should be unchecked");
    ji_check_box_destroy(cb);
}

static void test_check_box_toggle(void) {
    JiCheckBox* cb = ji_check_box_new("Test");
    ji_check_box_toggle(cb);
    TEST_ASSERT(ji_check_box_is_checked(cb) == true, "After toggle should be checked");
    ji_check_box_toggle(cb);
    TEST_ASSERT(ji_check_box_is_checked(cb) == false, "After second toggle should be unchecked");
    ji_check_box_destroy(cb);
}

static void test_check_box_type_id(void) {
    JiTypeId tid = ji_check_box_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiCheckBox type should be registered");
}

/* =========================================================================
 * Slider tests
 * ========================================================================= */
static void test_slider_create_destroy(void) {
    JiSlider* s = ji_slider_new();
    TEST_ASSERT(s != NULL, "Slider should be created");
    TEST_FLOAT_EQ(ji_slider_get_minimum(s), 0.0, 1e-9, "Default min should be 0");
    TEST_FLOAT_EQ(ji_slider_get_maximum(s), 100.0, 1e-9, "Default max should be 100");
    TEST_FLOAT_EQ(ji_slider_get_value(s), 0.0, 1e-9, "Default value should be 0");
    TEST_ASSERT(ji_slider_is_horizontal(s) == true, "Default should be horizontal");
    ji_slider_destroy(s);
}

static void test_slider_value_clamped(void) {
    JiSlider* s = ji_slider_new();
    ji_slider_set_value(s, 50.0);
    TEST_FLOAT_EQ(ji_slider_get_value(s), 50.0, 1e-9, "Value should be 50");
    ji_slider_set_value(s, -10.0);
    TEST_FLOAT_EQ(ji_slider_get_value(s), 0.0, 1e-9, "Value should clamp to min");
    ji_slider_set_value(s, 200.0);
    TEST_FLOAT_EQ(ji_slider_get_value(s), 100.0, 1e-9, "Value should clamp to max");
    ji_slider_destroy(s);
}

static void test_slider_orientation(void) {
    JiSlider* s = ji_slider_new();
    ji_slider_set_horizontal(s, false);
    TEST_ASSERT(ji_slider_is_horizontal(s) == false, "Should be vertical");
    ji_slider_destroy(s);
}

static void test_slider_type_id(void) {
    JiTypeId tid = ji_slider_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiSlider type should be registered");
}

/* =========================================================================
 * ProgressBar tests
 * ========================================================================= */
static void test_progress_bar_create_destroy(void) {
    JiProgressBar* pb = ji_progress_bar_new();
    TEST_ASSERT(pb != NULL, "ProgressBar should be created");
    TEST_FLOAT_EQ(ji_progress_bar_get_minimum(pb), 0.0, 1e-9, "Default min should be 0");
    TEST_FLOAT_EQ(ji_progress_bar_get_maximum(pb), 100.0, 1e-9, "Default max should be 100");
    TEST_FLOAT_EQ(ji_progress_bar_get_value(pb), 0.0, 1e-9, "Default value should be 0");
    TEST_ASSERT(ji_progress_bar_is_indeterminate(pb) == false, "Default should not be indeterminate");
    ji_progress_bar_destroy(pb);
}

static void test_progress_bar_value(void) {
    JiProgressBar* pb = ji_progress_bar_new();
    ji_progress_bar_set_value(pb, 75.0);
    TEST_FLOAT_EQ(ji_progress_bar_get_value(pb), 75.0, 1e-9, "Value should be 75");
    ji_progress_bar_set_value(pb, -5.0);
    TEST_FLOAT_EQ(ji_progress_bar_get_value(pb), 0.0, 1e-9, "Value should clamp to min");
    ji_progress_bar_set_value(pb, 150.0);
    TEST_FLOAT_EQ(ji_progress_bar_get_value(pb), 100.0, 1e-9, "Value should clamp to max");
    ji_progress_bar_destroy(pb);
}

static void test_progress_bar_indeterminate(void) {
    JiProgressBar* pb = ji_progress_bar_new();
    ji_progress_bar_set_indeterminate(pb, true);
    TEST_ASSERT(ji_progress_bar_is_indeterminate(pb) == true, "Should be indeterminate");
    ji_progress_bar_destroy(pb);
}

static void test_progress_bar_type_id(void) {
    JiTypeId tid = ji_progress_bar_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiProgressBar type should be registered");
}

/* =========================================================================
 * Image tests
 * ========================================================================= */
static void test_image_create_destroy(void) {
    JiImage* img = ji_image_new("/path/to/image.png");
    TEST_ASSERT(img != NULL, "Image should be created");
    TEST_ASSERT(strcmp(ji_image_get_source(img), "/path/to/image.png") == 0,
                "Source should match");
    ji_image_destroy(img);
}

static void test_image_set_source(void) {
    JiImage* img = ji_image_new("old.png");
    ji_image_set_source(img, "new.png");
    TEST_ASSERT(strcmp(ji_image_get_source(img), "new.png") == 0,
                "Source should be 'new.png'");
    ji_image_destroy(img);
}

static void test_image_type_id(void) {
    JiTypeId tid = ji_image_type_id();
    TEST_ASSERT(tid != JI_TYPE_INVALID, "JiImage type should be registered");
}

/* =========================================================================
 * Type hierarchy tests
 * ========================================================================= */
static void test_type_hierarchy(void) {
    JiTypeId control_tid = ji_control_type_id();
    JiTypeId visual_tid   = ji_visual_type_id();
    JiTypeId layout_tid   = ji_layout_element_type_id();
    JiTypeId object_tid   = ji_type_from_name("JiObject");

    TEST_ASSERT(control_tid != JI_TYPE_INVALID, "Control type should be registered");
    TEST_ASSERT(visual_tid != JI_TYPE_INVALID, "Visual type should be registered");

    /* JiControl extends JiVisual */
    TEST_ASSERT(ji_type_is_a(control_tid, visual_tid), "Control should be a Visual");

    /* JiButton extends JiControl */
    JiTypeId btn_tid = ji_button_type_id();
    TEST_ASSERT(ji_type_is_a(btn_tid, control_tid), "Button should be a Control");

    /* JiLabel extends JiControl */
    JiTypeId label_tid = ji_label_type_id();
    TEST_ASSERT(ji_type_is_a(label_tid, control_tid), "Label should be a Control");

    /* JiTextBox extends JiControl */
    JiTypeId tb_tid = ji_text_box_type_id();
    TEST_ASSERT(ji_type_is_a(tb_tid, control_tid), "TextBox should be a Control");

    /* JiCheckBox extends JiControl */
    JiTypeId cb_tid = ji_check_box_type_id();
    TEST_ASSERT(ji_type_is_a(cb_tid, control_tid), "CheckBox should be a Control");

    /* JiSlider extends JiControl */
    JiTypeId slider_tid = ji_slider_type_id();
    TEST_ASSERT(ji_type_is_a(slider_tid, control_tid), "Slider should be a Control");

    /* JiProgressBar extends JiControl */
    JiTypeId pb_tid = ji_progress_bar_type_id();
    TEST_ASSERT(ji_type_is_a(pb_tid, control_tid), "ProgressBar should be a Control");

    /* JiImage extends JiControl */
    JiTypeId img_tid = ji_image_type_id();
    TEST_ASSERT(ji_type_is_a(img_tid, control_tid), "Image should be a Control");
}

/* =========================================================================
 * Callback tests
 * ========================================================================= */
static int s_clicked_count = 0;
static int s_changed_count = 0;

static void on_clicked(JiControl* sender, void* user_data) {
    (void)sender;
    s_clicked_count += (int)(intptr_t)user_data;
}

static void on_value_changed(JiControl* sender, void* user_data) {
    (void)sender;
    s_changed_count += (int)(intptr_t)user_data;
}

static void test_control_callbacks(void) {
    JiButton* btn = ji_button_new("Click");
    s_clicked_count = 0;
    s_changed_count = 0;

    ji_control_set_on_clicked(&btn->control, on_clicked, (void*)1);
    ji_control_set_on_value_changed(&btn->control, on_value_changed, (void*)10);

    /* Simulate click */
    if (btn->control.on_clicked) {
        btn->control.on_clicked(&btn->control, btn->control.on_clicked_data);
    }
    TEST_ASSERT(s_clicked_count == 1, "Clicked callback should have been called");

    ji_button_destroy(btn);
}

static void test_check_box_callback(void) {
    JiCheckBox* cb = ji_check_box_new("Test");
    s_changed_count = 0;

    ji_control_set_on_value_changed(&cb->control, on_value_changed, (void*)5);

    /* Toggle triggers value changed */
    ji_check_box_toggle(cb);
    TEST_ASSERT(s_changed_count == 5, "Value changed callback should have been called");

    ji_check_box_destroy(cb);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    JiResultCode rc = ji_initialize();
    if (rc != JI_OK) {
        fprintf(stderr, "Failed to initialize JiUI: %d\n", rc);
        return 1;
    }

    printf("=== Controls & Widgets Tests ===\n");

    /* Control base */
    printf("--- Control ---\n");
    test_control_create_destroy();
    test_control_enabled();
    test_control_focus();
    test_control_name();
    test_control_tooltip();
    test_control_type_id();

    /* Button */
    printf("--- Button ---\n");
    test_button_create_destroy();
    test_button_set_text();
    test_button_type_id();

    /* Label */
    printf("--- Label ---\n");
    test_label_create_destroy();
    test_label_set_text();
    test_label_font_size();
    test_label_type_id();

    /* TextBox */
    printf("--- TextBox ---\n");
    test_text_box_create_destroy();
    test_text_box_set_text();
    test_text_box_placeholder();
    test_text_box_read_only();
    test_text_box_password();
    test_text_box_max_length();
    test_text_box_type_id();

    /* CheckBox */
    printf("--- CheckBox ---\n");
    test_check_box_create_destroy();
    test_check_box_checked();
    test_check_box_toggle();
    test_check_box_type_id();

    /* Slider */
    printf("--- Slider ---\n");
    test_slider_create_destroy();
    test_slider_value_clamped();
    test_slider_orientation();
    test_slider_type_id();

    /* ProgressBar */
    printf("--- ProgressBar ---\n");
    test_progress_bar_create_destroy();
    test_progress_bar_value();
    test_progress_bar_indeterminate();
    test_progress_bar_type_id();

    /* Image */
    printf("--- Image ---\n");
    test_image_create_destroy();
    test_image_set_source();
    test_image_type_id();

    /* Type hierarchy */
    printf("--- Type Hierarchy ---\n");
    test_type_hierarchy();

    /* Callbacks */
    printf("--- Callbacks ---\n");
    test_control_callbacks();
    test_check_box_callback();

    printf("\n%d/%d tests passed, %d failed\n",
           s_tests_passed, s_tests_run, s_tests_failed);

    ji_shutdown();
    return s_tests_failed > 0 ? 1 : 0;
}
