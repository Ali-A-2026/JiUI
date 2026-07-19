/**
 * JiUI - Input Widget Tests
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define BEGIN_TEST(name) printf("  TEST: %s ... ", name);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

/* ---- RadioButton ---- */
static void test_radio_button(void) {
    BEGIN_TEST("radio_button_group");
    JiRadioButtonGroup* g = ji_radio_button_group_new();
    if (!g) { FAIL("group new failed"); return; }
    JiRadioButton* rb1 = ji_radio_button_new(g, "Option A");
    JiRadioButton* rb2 = ji_radio_button_new(g, "Option B");
    JiRadioButton* rb3 = ji_radio_button_new(g, "Option C");
    if (ji_radio_button_group_count(g) != 3) { FAIL("count should be 3"); ji_radio_button_group_destroy(g); return; }
    if (!ji_radio_button_is_checked(rb1)) { FAIL("rb1 should be checked initially"); ji_radio_button_group_destroy(g); return; }
    if (ji_radio_button_group_get_selected(g) != 0) { FAIL("selected should be 0"); ji_radio_button_group_destroy(g); return; }

    ji_radio_button_set_checked(rb3, true);
    if (!ji_radio_button_is_checked(rb3)) { FAIL("rb3 should be checked"); ji_radio_button_group_destroy(g); return; }
    if (ji_radio_button_is_checked(rb1)) { FAIL("rb1 should be unchecked"); ji_radio_button_group_destroy(g); return; }
    if (ji_radio_button_group_get_selected(g) != 2) { FAIL("selected should be 2"); ji_radio_button_group_destroy(g); return; }

    ji_radio_button_group_set_selected(g, 1);
    if (!ji_radio_button_is_checked(rb2)) { FAIL("rb2 should be checked after set_selected"); ji_radio_button_group_destroy(g); return; }
    ji_radio_button_group_destroy(g);
    PASS();
}

/* ---- SpinBox ---- */
static void test_spin_box(void) {
    BEGIN_TEST("spin_box");
    JiSpinBox* sb = ji_spin_box_new();
    if (!sb) { FAIL("new failed"); return; }
    if (ji_spin_box_get_value(sb) != 0) { FAIL("default value should be 0"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_set_range(sb, 0, 100);
    ji_spin_box_set_value(sb, 50);
    if (ji_spin_box_get_value(sb) != 50) { FAIL("value should be 50"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_set_value(sb, 200);
    if (ji_spin_box_get_value(sb) != 100) { FAIL("value should clamp to 100"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_set_value(sb, 50);
    ji_spin_box_step_up(sb);
    if (ji_spin_box_get_value(sb) != 51) { FAIL("step up should be 51"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_step_down(sb);
    if (ji_spin_box_get_value(sb) != 50) { FAIL("step down should be 50"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_set_wrapping(sb, true);
    ji_spin_box_set_value(sb, 100);
    ji_spin_box_step_up(sb);
    if (ji_spin_box_get_value(sb) != 0) { FAIL("wrapping should go to 0"); ji_spin_box_destroy(sb); return; }
    ji_spin_box_destroy(sb);
    PASS();
}

/* ---- DoubleSpinBox ---- */
static void test_double_spin_box(void) {
    BEGIN_TEST("double_spin_box");
    JiDoubleSpinBox* sb = ji_double_spin_box_new();
    if (!sb) { FAIL("new failed"); return; }
    ji_double_spin_box_set_range(sb, 0.0, 1.0);
    ji_double_spin_box_set_value(sb, 0.5);
    if (ji_double_spin_box_get_value(sb) < 0.499 || ji_double_spin_box_get_value(sb) > 0.501) { FAIL("value should be 0.5"); ji_double_spin_box_destroy(sb); return; }
    ji_double_spin_box_set_single_step(sb, 0.1);
    ji_double_spin_box_step_up(sb);
    if (ji_double_spin_box_get_value(sb) < 0.599) { FAIL("step up failed"); ji_double_spin_box_destroy(sb); return; }
    ji_double_spin_box_set_decimals(sb, 3);
    if (sb->decimals != 3) { FAIL("decimals should be 3"); ji_double_spin_box_destroy(sb); return; }
    ji_double_spin_box_destroy(sb);
    PASS();
}

/* ---- ComboBox ---- */
static void test_combo_box(void) {
    BEGIN_TEST("combo_box");
    JiComboBox* cb = ji_combo_box_new();
    if (!cb) { FAIL("new failed"); return; }
    ji_combo_box_add_item(cb, "Apple");
    ji_combo_box_add_item(cb, "Banana");
    ji_combo_box_add_item(cb, "Cherry");
    if (ji_combo_box_count(cb) != 3) { FAIL("count should be 3"); ji_combo_box_destroy(cb); return; }
    if (ji_combo_box_get_current_index(cb) != 0) { FAIL("current should be 0"); ji_combo_box_destroy(cb); return; }
    if (strcmp(ji_combo_box_get_current_text(cb), "Apple") != 0) { FAIL("current text should be Apple"); ji_combo_box_destroy(cb); return; }

    ji_combo_box_set_current_index(cb, 2);
    if (strcmp(ji_combo_box_get_current_text(cb), "Cherry") != 0) { FAIL("current text should be Cherry"); ji_combo_box_destroy(cb); return; }

    ji_combo_box_remove_item(cb, 1);
    if (ji_combo_box_count(cb) != 2) { FAIL("count should be 2 after remove"); ji_combo_box_destroy(cb); return; }
    if (strcmp(ji_combo_box_get_item_text(cb, 1), "Cherry") != 0) { FAIL("item 1 should be Cherry"); ji_combo_box_destroy(cb); return; }

    ji_combo_box_set_editable(cb, true);
    if (!ji_combo_box_is_editable(cb)) { FAIL("should be editable"); ji_combo_box_destroy(cb); return; }
    ji_combo_box_destroy(cb);
    PASS();
}

/* ---- TextEdit ---- */
static void test_text_edit(void) {
    BEGIN_TEST("text_edit");
    JiTextEdit* te = ji_text_edit_new();
    if (!te) { FAIL("new failed"); return; }
    ji_text_edit_set_text(te, "Hello");
    if (!ji_text_edit_get_text(te) || strcmp(ji_text_edit_get_text(te), "Hello") != 0) { FAIL("text mismatch"); ji_text_edit_destroy(te); return; }
    ji_text_edit_append_text(te, " World");
    if (strcmp(ji_text_edit_get_text(te), "Hello World") != 0) { FAIL("append failed"); ji_text_edit_destroy(te); return; }
    ji_text_edit_set_cursor_position(te, 5);
    if (ji_text_edit_get_cursor_position(te) != 5) { FAIL("cursor should be 5"); ji_text_edit_destroy(te); return; }
    ji_text_edit_insert_text(te, "!");
    if (strcmp(ji_text_edit_get_text(te), "Hello! World") != 0) { FAIL("insert failed"); ji_text_edit_destroy(te); return; }
    ji_text_edit_select_all(te);
    if (te->selection.start != 0 || te->selection.end != te->text_length) { FAIL("select all failed"); ji_text_edit_destroy(te); return; }
    ji_text_edit_set_read_only(te, true);
    if (!ji_text_edit_is_read_only(te)) { FAIL("should be read only"); ji_text_edit_destroy(te); return; }
    ji_text_edit_destroy(te);
    PASS();
}

/* ---- PlainTextEdit ---- */
static void test_plain_text_edit(void) {
    BEGIN_TEST("plain_text_edit");
    JiPlainTextEdit* te = ji_plain_text_edit_new();
    if (!te) { FAIL("new failed"); return; }
    ji_plain_text_edit_set_text(te, "Line 1\nLine 2");
    if (!ji_plain_text_edit_get_text(te) || strcmp(ji_plain_text_edit_get_text(te), "Line 1\nLine 2") != 0) { FAIL("text mismatch"); ji_plain_text_edit_destroy(te); return; }
    ji_plain_text_edit_append_text(te, "\nLine 3");
    if (strcmp(ji_plain_text_edit_get_text(te), "Line 1\nLine 2\nLine 3") != 0) { FAIL("append failed"); ji_plain_text_edit_destroy(te); return; }
    ji_plain_text_edit_destroy(te);
    PASS();
}

/* ---- DateTimeEdit ---- */
static void test_date_time_edit(void) {
    BEGIN_TEST("date_time_edit");
    JiDateTimeEdit* dte = ji_date_time_edit_new();
    if (!dte) { FAIL("new failed"); return; }
    JiDateTime dt = {2026, 7, 15, 14, 30, 0};
    ji_date_time_edit_set_date_time(dte, dt);
    JiDateTime result = ji_date_time_edit_get_date_time(dte);
    if (result.year != 2026 || result.month != 7 || result.day != 15) { FAIL("date mismatch"); ji_date_time_edit_destroy(dte); return; }
    if (result.hour != 14 || result.minute != 30) { FAIL("time mismatch"); ji_date_time_edit_destroy(dte); return; }
    ji_date_time_edit_set_format(dte, JI_DT_FORMAT_DATE_ONLY);
    if (dte->format != JI_DT_FORMAT_DATE_ONLY) { FAIL("format should be DATE_ONLY"); ji_date_time_edit_destroy(dte); return; }
    ji_date_time_edit_destroy(dte);
    PASS();
}

/* ---- Dial ---- */
static void test_dial(void) {
    BEGIN_TEST("dial");
    JiDial* d = ji_dial_new();
    if (!d) { FAIL("new failed"); return; }
    ji_dial_set_range(d, 0, 360);
    ji_dial_set_value(d, 180);
    if (ji_dial_get_value(d) != 180) { FAIL("value should be 180"); ji_dial_destroy(d); return; }
    ji_dial_set_value(d, 500);
    if (ji_dial_get_value(d) != 360) { FAIL("value should clamp to 360"); ji_dial_destroy(d); return; }
    ji_dial_set_wrapping(d, true);
    ji_dial_set_value(d, 361);
    if (ji_dial_get_value(d) != 0) { FAIL("wrapping should go to 0"); ji_dial_destroy(d); return; }
    ji_dial_set_notches_visible(d, false);
    if (d->notches_visible) { FAIL("notches should be hidden"); ji_dial_destroy(d); return; }
    ji_dial_destroy(d);
    PASS();
}

/* ---- ScrollBar ---- */
static void test_scroll_bar(void) {
    BEGIN_TEST("scroll_bar");
    JiScrollBar* sb = ji_scroll_bar_new(JI_SCROLLBAR_VERTICAL);
    if (!sb) { FAIL("new failed"); return; }
    if (sb->orientation != JI_SCROLLBAR_VERTICAL) { FAIL("should be vertical"); ji_scroll_bar_destroy(sb); return; }
    ji_scroll_bar_set_range(sb, 0, 1000);
    ji_scroll_bar_set_value(sb, 500);
    if (ji_scroll_bar_get_value(sb) != 500) { FAIL("value should be 500"); ji_scroll_bar_destroy(sb); return; }
    ji_scroll_bar_set_value(sb, 2000);
    if (ji_scroll_bar_get_value(sb) != 1000) { FAIL("value should clamp to 1000"); ji_scroll_bar_destroy(sb); return; }
    ji_scroll_bar_set_page_step(sb, 50);
    if (sb->page_step != 50) { FAIL("page step should be 50"); ji_scroll_bar_destroy(sb); return; }
    ji_scroll_bar_destroy(sb);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Input Widget Tests ===\n\n");

    test_radio_button();
    test_spin_box();
    test_double_spin_box();
    test_combo_box();
    test_text_edit();
    test_plain_text_edit();
    test_date_time_edit();
    test_dial();
    test_scroll_bar();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
