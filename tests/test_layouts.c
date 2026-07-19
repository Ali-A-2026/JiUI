/**
 * JiUI - Layout Widget Tests (BoxLayout, FormLayout, FlowLayout)
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

/* ---- BoxLayout ---- */
static void test_box_layout(void) {
    BEGIN_TEST("box_layout");
    JiBoxLayout* layout = ji_box_layout_new(JI_BOX_LEFT_TO_RIGHT);
    if (!layout) { FAIL("new failed"); return; }
    if (ji_box_layout_count(layout) != 0) { FAIL("should start empty"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_add(layout, NULL, 0);
    ji_box_layout_add(layout, NULL, 1);
    ji_box_layout_add(layout, NULL, 2);
    if (ji_box_layout_count(layout) != 3) { FAIL("count should be 3"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_add_spacer(layout, 20);
    if (ji_box_layout_count(layout) != 4) { FAIL("count should be 4 after spacer"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_add_stretch(layout, 1);
    if (ji_box_layout_count(layout) != 5) { FAIL("count should be 5 after stretch"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_set_direction(layout, JI_BOX_TOP_TO_BOTTOM);
    if (layout->direction != JI_BOX_TOP_TO_BOTTOM) { FAIL("direction not set"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_set_spacing(layout, 10);
    if (layout->spacing != 10) { FAIL("spacing not set"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_set_margins(layout, 5, 5, 5, 5);
    if (layout->margin_left != 5) { FAIL("margin not set"); ji_box_layout_destroy(layout); return; }
    ji_box_layout_destroy(layout);
    PASS();
}

/* ---- FormLayout ---- */
static void test_form_layout(void) {
    BEGIN_TEST("form_layout");
    JiFormLayout* layout = ji_form_layout_new();
    if (!layout) { FAIL("new failed"); return; }
    int r1 = ji_form_layout_add_row(layout, "Name", NULL);
    int r2 = ji_form_layout_add_row(layout, "Email", NULL);
    int r3 = ji_form_layout_add_row(layout, "Phone", NULL);
    if (r1 != 0 || r2 != 1 || r3 != 2) { FAIL("row indices wrong"); ji_form_layout_destroy(layout); return; }
    if (ji_form_layout_row_count(layout) != 3) { FAIL("count should be 3"); ji_form_layout_destroy(layout); return; }
    ji_form_layout_insert_row(layout, 1, "Address", NULL);
    if (ji_form_layout_row_count(layout) != 4) { FAIL("count should be 4 after insert"); ji_form_layout_destroy(layout); return; }
    if (strcmp(layout->rows[1].label_text, "Address") != 0) { FAIL("inserted row label wrong"); ji_form_layout_destroy(layout); return; }
    ji_form_layout_remove_row(layout, 1);
    if (ji_form_layout_row_count(layout) != 3) { FAIL("count should be 3 after remove"); ji_form_layout_destroy(layout); return; }
    ji_form_layout_set_spacing(layout, 12);
    if (layout->spacing != 12) { FAIL("spacing not set"); ji_form_layout_destroy(layout); return; }
    ji_form_layout_set_field_growth(layout, JI_FORM_FIELDS_ALL_NON_FIXED);
    if (layout->field_growth != JI_FORM_FIELDS_ALL_NON_FIXED) { FAIL("field growth not set"); ji_form_layout_destroy(layout); return; }
    ji_form_layout_destroy(layout);
    PASS();
}

/* ---- FlowLayout ---- */
static void test_flow_layout(void) {
    BEGIN_TEST("flow_layout");
    JiFlowLayout* layout = ji_flow_layout_new();
    if (!layout) { FAIL("new failed"); return; }
    if (ji_flow_layout_count(layout) != 0) { FAIL("should start empty"); ji_flow_layout_destroy(layout); return; }
    ji_flow_layout_add(layout, NULL);
    ji_flow_layout_add(layout, NULL);
    ji_flow_layout_add(layout, NULL);
    if (ji_flow_layout_count(layout) != 3) { FAIL("count should be 3"); ji_flow_layout_destroy(layout); return; }
    ji_flow_layout_set_spacing(layout, 8);
    if (layout->spacing != 8) { FAIL("spacing not set"); ji_flow_layout_destroy(layout); return; }
    ji_flow_layout_set_margins(layout, 4, 4, 4, 4);
    if (layout->margin_left != 4) { FAIL("margin not set"); ji_flow_layout_destroy(layout); return; }
    if (!layout->wrap) { FAIL("wrap should default to true"); ji_flow_layout_destroy(layout); return; }
    ji_flow_layout_destroy(layout);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Layout Widget Tests ===\n\n");

    test_box_layout();
    test_form_layout();
    test_flow_layout();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
