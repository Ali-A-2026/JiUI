/**
 * JiUI - Animation & Undo/Redo Tests
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

/* ---- Easing Curves ---- */
static void test_easing_linear(void) {
    BEGIN_TEST("easing_linear");
    JiEasingCurve c = ji_easing_curve_new(JI_EASE_LINEAR);
    if (ji_easing_curve_value(&c, 0.0) != 0.0) { FAIL("0.0 should be 0"); return; }
    if (ji_easing_curve_value(&c, 1.0) != 1.0) { FAIL("1.0 should be 1"); return; }
    if (ji_easing_curve_value(&c, 0.5) != 0.5) { FAIL("0.5 should be 0.5"); return; }
    PASS();
}

static void test_easing_out_bounce(void) {
    BEGIN_TEST("easing_out_bounce");
    JiEasingCurve c = ji_easing_curve_new(JI_EASE_OUT_BOUNCE);
    if (ji_easing_curve_value(&c, 0.0) != 0.0) { FAIL("start should be 0"); return; }
    if (ji_easing_curve_value(&c, 1.0) != 1.0) { FAIL("end should be 1"); return; }
    double mid = ji_easing_curve_value(&c, 0.5);
    if (mid < 0.0 || mid > 1.5) { FAIL("mid out of range"); return; }
    PASS();
}

static void test_easing_spring(void) {
    BEGIN_TEST("easing_spring");
    JiEasingCurve c = ji_easing_curve_new(JI_EASE_SPRING);
    c.spring_stiffness = 200;
    c.spring_damping = 20;
    c.spring_mass = 1;
    if (ji_easing_curve_value(&c, 0.0) != 0.0) { FAIL("start should be 0"); return; }
    double end = ji_easing_curve_value(&c, 1.0);
    if (end < 0.9 || end > 1.1) { FAIL("spring should converge to 1"); return; }
    PASS();
}

static void test_easing_bezier(void) {
    BEGIN_TEST("easing_bezier");
    JiEasingCurve c = ji_easing_curve_new(JI_EASE_CUBIC_BEZIER);
    ji_easing_curve_set_bezier(&c, 0.42, 0.0, 0.58, 1.0);
    if (ji_easing_curve_value(&c, 0.0) != 0.0) { FAIL("start"); return; }
    if (ji_easing_curve_value(&c, 1.0) != 1.0) { FAIL("end"); return; }
    PASS();
}

/* ---- Animation ---- */
static void test_animation_basic(void) {
    BEGIN_TEST("animation_basic");
    JiAnimation anim;
    ji_animation_init(&anim);
    ji_animation_set_duration(&anim, 1000);
    if (anim.state != JI_ANIMATION_STOPPED) { FAIL("should start stopped"); return; }
    ji_animation_start(&anim);
    if (anim.state != JI_ANIMATION_RUNNING) { FAIL("should be running"); return; }
    ji_animation_stop(&anim);
    if (anim.state != JI_ANIMATION_STOPPED) { FAIL("should be stopped"); return; }
    PASS();
}

static void test_property_animation(void) {
    BEGIN_TEST("property_animation");
    JiPropertyAnimation* pa = ji_property_animation_new();
    if (!pa) { FAIL("new failed"); return; }
    ji_property_animation_set_range(pa, 0.0, 100.0);
    if (pa->start_value != 0.0 || pa->end_value != 100.0) { FAIL("range not set"); ji_property_animation_destroy(pa); return; }
    ji_animation_set_duration(&pa->base, 500);
    if (pa->base.duration_ms != 500) { FAIL("duration not set"); ji_property_animation_destroy(pa); return; }
    ji_property_animation_destroy(pa);
    PASS();
}

static void test_animation_group(void) {
    BEGIN_TEST("animation_group");
    JiAnimationGroup* g = ji_animation_group_new(JI_ANIMATION_GROUP_PARALLEL);
    if (!g) { FAIL("new failed"); return; }
    if (g->child_count != 0) { FAIL("should start empty"); ji_animation_group_destroy(g); return; }
    JiPropertyAnimation* a1 = ji_property_animation_new();
    JiPropertyAnimation* a2 = ji_property_animation_new();
    ji_animation_group_add(g, &a1->base);
    ji_animation_group_add(g, &a2->base);
    if (g->child_count != 2) { FAIL("should have 2 children"); ji_animation_group_destroy(g); return; }
    ji_animation_group_remove(g, &a1->base);
    if (g->child_count != 1) { FAIL("should have 1 child after remove"); ji_animation_group_destroy(g); return; }
    ji_animation_group_destroy(g);
    ji_property_animation_destroy(a1);
    ji_property_animation_destroy(a2);
    PASS();
}

static void test_animation_driver(void) {
    BEGIN_TEST("animation_driver");
    JiAnimationDriver* d = ji_animation_driver_new();
    if (!d) { FAIL("new failed"); return; }
    if (d->animation_count != 0) { FAIL("should start empty"); ji_animation_driver_destroy(d); return; }
    JiPropertyAnimation* a = ji_property_animation_new();
    ji_animation_driver_register(d, &a->base);
    if (d->animation_count != 1) { FAIL("should have 1 animation"); ji_animation_driver_destroy(d); return; }
    ji_animation_driver_unregister(d, &a->base);
    if (d->animation_count != 0) { FAIL("should have 0 after unregister"); ji_animation_driver_destroy(d); return; }
    ji_animation_driver_destroy(d);
    ji_property_animation_destroy(a);
    PASS();
}

/* ---- Undo/Redo ---- */
static int undo_count = 0;
static int redo_count = 0;

static void test_undo_fn(JiUndoCommand* cmd, void* user_data) { (void)cmd; (void)user_data; undo_count++; }
static void test_redo_fn(JiUndoCommand* cmd, void* user_data) { (void)cmd; (void)user_data; redo_count++; }

static void test_undo_stack_basic(void) {
    BEGIN_TEST("undo_stack_basic");
    JiUndoStack* s = ji_undo_stack_new();
    if (!s) { FAIL("new failed"); return; }
    if (ji_undo_stack_can_undo(s)) { FAIL("should not be able to undo initially"); ji_undo_stack_destroy(s); return; }
    if (ji_undo_stack_can_redo(s)) { FAIL("should not be able to redo initially"); ji_undo_stack_destroy(s); return; }
    
    JiUndoCommand* cmd1 = ji_undo_command_new("Insert");
    ji_undo_command_set_undo(cmd1, test_undo_fn);
    ji_undo_command_set_redo(cmd1, test_redo_fn);
    redo_count = 0;
    ji_undo_stack_push(s, cmd1);
    if (redo_count != 1) { FAIL("push should call redo"); ji_undo_stack_destroy(s); return; }
    if (!ji_undo_stack_can_undo(s)) { FAIL("should be able to undo after push"); ji_undo_stack_destroy(s); return; }
    
    undo_count = 0;
    ji_undo_stack_undo(s);
    if (undo_count != 1) { FAIL("undo should call undo_fn"); ji_undo_stack_destroy(s); return; }
    if (!ji_undo_stack_can_redo(s)) { FAIL("should be able to redo after undo"); ji_undo_stack_destroy(s); return; }
    
    redo_count = 0;
    ji_undo_stack_redo(s);
    if (redo_count != 1) { FAIL("redo should call redo_fn"); ji_undo_stack_destroy(s); return; }
    
    ji_undo_stack_destroy(s);
    PASS();
}

static void test_undo_stack_clean(void) {
    BEGIN_TEST("undo_stack_clean");
    JiUndoStack* s = ji_undo_stack_new();
    if (!ji_undo_stack_is_clean(s)) { FAIL("should be clean initially"); ji_undo_stack_destroy(s); return; }
    
    JiUndoCommand* cmd = ji_undo_command_new("Edit");
    ji_undo_command_set_undo(cmd, test_undo_fn);
    ji_undo_command_set_redo(cmd, test_redo_fn);
    ji_undo_stack_push(s, cmd);
    if (ji_undo_stack_is_clean(s)) { FAIL("should be dirty after push"); ji_undo_stack_destroy(s); return; }
    
    ji_undo_stack_set_clean(s);
    if (!ji_undo_stack_is_clean(s)) { FAIL("should be clean after set_clean"); ji_undo_stack_destroy(s); return; }
    
    ji_undo_stack_destroy(s);
    PASS();
}

static void test_undo_stack_text(void) {
    BEGIN_TEST("undo_stack_text");
    JiUndoStack* s = ji_undo_stack_new();
    JiUndoCommand* cmd = ji_undo_command_new("Type A");
    ji_undo_command_set_undo(cmd, test_undo_fn);
    ji_undo_command_set_redo(cmd, test_redo_fn);
    ji_undo_stack_push(s, cmd);
    if (strcmp(ji_undo_stack_undo_text(s), "Type A") != 0) { FAIL("undo text wrong"); ji_undo_stack_destroy(s); return; }
    ji_undo_stack_destroy(s);
    PASS();
}

/* ---- Model/View ---- */
static void test_string_list_model(void) {
    BEGIN_TEST("string_list_model");
    JiStringListModel* m = ji_string_list_model_new();
    if (!m) { FAIL("new failed"); return; }
    if (ji_model_row_count(&m->base, NULL) != 0) { FAIL("should start empty"); ji_string_list_model_destroy(m); return; }
    
    ji_string_list_model_append(m, "Hello");
    ji_string_list_model_append(m, "World");
    ji_string_list_model_append(m, "!");
    if (ji_model_row_count(&m->base, NULL) != 3) { FAIL("count should be 3"); ji_string_list_model_destroy(m); return; }
    
    JiModelIndex idx = ji_model_index(&m->base, 0, 0, NULL);
    char* data = ji_model_data(&m->base, &idx, JI_ROLE_DISPLAY);
    if (!data || strcmp(data, "Hello") != 0) { FAIL("data[0] wrong"); ji_string_list_model_destroy(m); return; }
    
    idx = ji_model_index(&m->base, 2, 0, NULL);
    data = ji_model_data(&m->base, &idx, JI_ROLE_DISPLAY);
    if (!data || strcmp(data, "!") != 0) { FAIL("data[2] wrong"); ji_string_list_model_destroy(m); return; }
    
    ji_string_list_model_clear(m);
    if (ji_model_row_count(&m->base, NULL) != 0) { FAIL("should be empty after clear"); ji_string_list_model_destroy(m); return; }
    
    ji_string_list_model_destroy(m);
    PASS();
}

static void test_model_insert_remove(void) {
    BEGIN_TEST("model_insert_remove");
    JiStringListModel* m = ji_string_list_model_new();
    ji_string_list_model_append(m, "A");
    ji_string_list_model_append(m, "C");
    
    ji_model_insert_rows(&m->base, 1, 1, NULL);
    JiModelIndex idx = ji_model_index(&m->base, 1, 0, NULL);
    ji_model_set_data(&m->base, &idx, "B", JI_ROLE_DISPLAY);
    char* data = ji_model_data(&m->base, &idx, JI_ROLE_DISPLAY);
    if (!data || strcmp(data, "B") != 0) { FAIL("inserted data wrong"); ji_string_list_model_destroy(m); return; }
    
    ji_model_remove_rows(&m->base, 1, 1, NULL);
    if (ji_model_row_count(&m->base, NULL) != 2) { FAIL("count should be 2 after remove"); ji_string_list_model_destroy(m); return; }
    
    ji_string_list_model_destroy(m);
    PASS();
}

static void test_list_view(void) {
    BEGIN_TEST("list_view");
    JiListView* v = ji_list_view_new();
    if (!v) { FAIL("new failed"); return; }
    if (v->base.row_height != 24) { FAIL("default row height wrong"); ji_list_view_destroy(v); return; }
    ji_list_view_destroy(v);
    PASS();
}

static void test_tree_view(void) {
    BEGIN_TEST("tree_view");
    JiTreeView* v = ji_tree_view_new();
    if (!v) { FAIL("new failed"); return; }
    if (v->indent != 20) { FAIL("default indent wrong"); ji_tree_view_destroy(v); return; }
    JiModelIndex idx = ji_model_index_new(0, 0, NULL, NULL);
    ji_tree_view_expand(v, &idx);
    if (!ji_tree_view_is_expanded(v, &idx)) { FAIL("should be expanded"); ji_tree_view_destroy(v); return; }
    ji_tree_view_collapse(v, &idx);
    if (ji_tree_view_is_expanded(v, &idx)) { FAIL("should be collapsed"); ji_tree_view_destroy(v); return; }
    ji_tree_view_destroy(v);
    PASS();
}

static void test_table_view(void) {
    BEGIN_TEST("table_view");
    JiTableView* v = ji_table_view_new();
    if (!v) { FAIL("new failed"); return; }
    if (!v->show_grid) { FAIL("grid should default on"); ji_table_view_destroy(v); return; }
    ji_table_view_sort_by_column(v, 0, true);
    if (!v->is_sorting_enabled || v->sort_column != 0) { FAIL("sort not set"); ji_table_view_destroy(v); return; }
    ji_table_view_destroy(v);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Advanced Features Tests ===\n\n");

    test_easing_linear();
    test_easing_out_bounce();
    test_easing_spring();
    test_easing_bezier();
    test_animation_basic();
    test_property_animation();
    test_animation_group();
    test_animation_driver();
    test_undo_stack_basic();
    test_undo_stack_clean();
    test_undo_stack_text();
    test_string_list_model();
    test_model_insert_remove();
    test_list_view();
    test_tree_view();
    test_table_view();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
