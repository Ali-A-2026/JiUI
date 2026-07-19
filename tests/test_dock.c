/**
 * JiUI - Docking System Tests
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
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

/* ---- Dock widget lifecycle ---- */
static void test_dock_widget_new(void) {
    BEGIN_TEST("dock_widget_new");
    JiDockWidget* w = ji_dock_widget_new("properties", "Properties");
    if (!w) { FAIL("ji_dock_widget_new returned NULL"); return; }
    if (!w->name || strcmp(w->name, "properties") != 0) { FAIL("name mismatch"); ji_dock_widget_destroy(w); return; }
    if (!w->title || strcmp(w->title, "Properties") != 0) { FAIL("title mismatch"); ji_dock_widget_destroy(w); return; }
    if (w->state != JI_DOCK_DOCKED) { FAIL("initial state should be DOCKED"); ji_dock_widget_destroy(w); return; }
    if (w->features != JI_DOCK_FEATURE_ALL) { FAIL("features should be ALL"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_destroy(w);
    PASS();
}

/* ---- Dock widget set title ---- */
static void test_dock_widget_set_title(void) {
    BEGIN_TEST("dock_widget_set_title");
    JiDockWidget* w = ji_dock_widget_new("test", "Old");
    if (!w) { FAIL("new failed"); return; }
    ji_dock_widget_set_title(w, "New Title");
    if (strcmp(ji_dock_widget_get_title(w), "New Title") != 0) { FAIL("title not updated"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_destroy(w);
    PASS();
}

/* ---- Dock widget floating ---- */
static void test_dock_widget_floating(void) {
    BEGIN_TEST("dock_widget_floating");
    JiDockWidget* w = ji_dock_widget_new("test", "Test");
    if (!w) { FAIL("new failed"); return; }
    ji_dock_widget_set_floating(w, true);
    if (!ji_dock_widget_is_floating(w)) { FAIL("should be floating"); ji_dock_widget_destroy(w); return; }
    if (w->state != JI_DOCK_FLOATING) { FAIL("state should be FLOATING"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_set_floating(w, false);
    if (ji_dock_widget_is_floating(w)) { FAIL("should not be floating"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_destroy(w);
    PASS();
}

/* ---- Dock widget hide/show ---- */
static void test_dock_widget_hide_show(void) {
    BEGIN_TEST("dock_widget_hide_show");
    JiDockWidget* w = ji_dock_widget_new("test", "Test");
    if (!w) { FAIL("new failed"); return; }
    ji_dock_widget_hide(w);
    if (!ji_dock_widget_is_hidden(w)) { FAIL("should be hidden"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_show(w);
    if (ji_dock_widget_is_hidden(w)) { FAIL("should not be hidden"); ji_dock_widget_destroy(w); return; }
    ji_dock_widget_destroy(w);
    PASS();
}

/* ---- Dock area lifecycle ---- */
static void test_dock_area_new(void) {
    BEGIN_TEST("dock_area_new");
    JiDockArea* a = ji_dock_area_new("center");
    if (!a) { FAIL("ji_dock_area_new returned NULL"); return; }
    if (!a->name || strcmp(a->name, "center") != 0) { FAIL("name mismatch"); ji_dock_area_destroy(a); return; }
    if (a->region != JI_DOCK_REGION_CENTER) { FAIL("region should be CENTER"); ji_dock_area_destroy(a); return; }
    if (a->widget_count != 0) { FAIL("widget_count should be 0"); ji_dock_area_destroy(a); return; }
    ji_dock_area_destroy(a);
    PASS();
}

/* ---- Dock area add/remove widgets ---- */
static void test_dock_area_add_remove(void) {
    BEGIN_TEST("dock_area_add_remove");
    JiDockArea* a = ji_dock_area_new("center");
    if (!a) { FAIL("area new failed"); return; }

    JiDockWidget* w1 = ji_dock_widget_new("w1", "Widget 1");
    JiDockWidget* w2 = ji_dock_widget_new("w2", "Widget 2");
    if (!w1 || !w2) { FAIL("widget new failed"); ji_dock_area_destroy(a); return; }

    ji_dock_area_add_widget(a, w1);
    if (a->widget_count != 1) { FAIL("count should be 1"); ji_dock_area_destroy(a); return; }
    if (w1->area != a) { FAIL("widget area not set"); ji_dock_area_destroy(a); return; }

    ji_dock_area_add_widget(a, w2);
    if (a->widget_count != 2) { FAIL("count should be 2"); ji_dock_area_destroy(a); return; }

    /* Find by name */
    JiDockWidget* found = ji_dock_area_find_widget(a, "w2");
    if (found != w2) { FAIL("find_widget failed"); ji_dock_area_destroy(a); return; }

    /* Remove w1 */
    ji_dock_area_remove_widget(a, w1);
    if (a->widget_count != 1) { FAIL("count should be 1 after remove"); ji_dock_area_destroy(a); return; }
    if (w1->area != NULL) { FAIL("removed widget area should be NULL"); ji_dock_area_destroy(a); return; }

    ji_dock_widget_destroy(w1);
    ji_dock_area_destroy(a);
    PASS();
}

/* ---- Dock area active tab ---- */
static void test_dock_area_active_tab(void) {
    BEGIN_TEST("dock_area_active_tab");
    JiDockArea* a = ji_dock_area_new("center");
    if (!a) { FAIL("area new failed"); return; }

    JiDockWidget* w1 = ji_dock_widget_new("w1", "W1");
    JiDockWidget* w2 = ji_dock_widget_new("w2", "W2");
    ji_dock_area_add_widget(a, w1);
    ji_dock_area_add_widget(a, w2);

    /* First widget should be active */
    if (ji_dock_area_get_active_index(a) != 0) { FAIL("active should be 0"); ji_dock_area_destroy(a); return; }
    if (ji_dock_area_get_active_widget(a) != w1) { FAIL("active widget should be w1"); ji_dock_area_destroy(a); return; }

    /* Switch to w2 */
    ji_dock_area_set_active_index(a, 1);
    if (ji_dock_area_get_active_index(a) != 1) { FAIL("active should be 1"); ji_dock_area_destroy(a); return; }
    if (ji_dock_area_get_active_widget(a) != w2) { FAIL("active widget should be w2"); ji_dock_area_destroy(a); return; }
    if (w1->is_active) { FAIL("w1 should not be active"); ji_dock_area_destroy(a); return; }
    if (!w2->is_active) { FAIL("w2 should be active"); ji_dock_area_destroy(a); return; }

    ji_dock_area_destroy(a);
    PASS();
}

/* ---- Dock manager lifecycle ---- */
static void test_dock_manager_new(void) {
    BEGIN_TEST("dock_manager_new");
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("ji_dock_manager_new returned NULL"); return; }
    if (mgr->area_count != 0) { FAIL("area_count should be 0"); ji_dock_manager_destroy(mgr); return; }
    if (mgr->floating_count != 0) { FAIL("floating_count should be 0"); ji_dock_manager_destroy(mgr); return; }
    if (ji_dock_manager_is_dragging(mgr)) { FAIL("should not be dragging"); ji_dock_manager_destroy(mgr); return; }
    ji_dock_manager_destroy(mgr);
    PASS();
}

/* ---- Dock manager add area + widget ---- */
static void test_dock_manager_add_widget(void) {
    BEGIN_TEST("dock_manager_add_widget");
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("mgr new failed"); return; }

    JiDockArea* a = ji_dock_area_new("center");
    ji_dock_manager_set_root_area(mgr, a);
    if (ji_dock_manager_get_root_area(mgr) != a) { FAIL("root area mismatch"); ji_dock_manager_destroy(mgr); return; }
    if (ji_dock_manager_area_count(mgr) != 1) { FAIL("area count should be 1"); ji_dock_manager_destroy(mgr); return; }

    JiDockWidget* w = ji_dock_widget_new("explorer", "Explorer");
    ji_dock_manager_add_widget(mgr, w, a);
    if (ji_dock_area_widget_count(a) != 1) { FAIL("area widget count should be 1"); ji_dock_manager_destroy(mgr); return; }

    /* Find widget */
    JiDockWidget* found = ji_dock_manager_find_widget(mgr, "explorer");
    if (found != w) { FAIL("find_widget failed"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_destroy(mgr);
    PASS();
}

/* ---- Dock manager float/dock ---- */
static void test_dock_manager_float_dock(void) {
    BEGIN_TEST("dock_manager_float_dock");
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("mgr new failed"); return; }

    JiDockArea* a = ji_dock_area_new("center");
    ji_dock_manager_set_root_area(mgr, a);

    JiDockWidget* w = ji_dock_widget_new("output", "Output");
    ji_dock_manager_add_widget(mgr, w, a);
    if (ji_dock_manager_floating_count(mgr) != 0) { FAIL("floating count should be 0"); ji_dock_manager_destroy(mgr); return; }

    /* Float it */
    ji_dock_manager_float_widget(mgr, w);
    if (ji_dock_manager_floating_count(mgr) != 1) { FAIL("floating count should be 1"); ji_dock_manager_destroy(mgr); return; }
    if (!ji_dock_widget_is_floating(w)) { FAIL("widget should be floating"); ji_dock_manager_destroy(mgr); return; }
    if (ji_dock_area_widget_count(a) != 0) { FAIL("area should be empty"); ji_dock_manager_destroy(mgr); return; }

    /* Dock it back */
    ji_dock_manager_dock_widget(mgr, w, a);
    if (ji_dock_manager_floating_count(mgr) != 0) { FAIL("floating count should be 0 after dock"); ji_dock_manager_destroy(mgr); return; }
    if (ji_dock_widget_is_floating(w)) { FAIL("widget should not be floating"); ji_dock_manager_destroy(mgr); return; }
    if (ji_dock_area_widget_count(a) != 1) { FAIL("area should have 1 widget"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_destroy(mgr);
    PASS();
}

/* ---- Dock manager active widget ---- */
static void test_dock_manager_active(void) {
    BEGIN_TEST("dock_manager_active");
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("mgr new failed"); return; }

    JiDockArea* a = ji_dock_area_new("center");
    ji_dock_manager_set_root_area(mgr, a);

    JiDockWidget* w1 = ji_dock_widget_new("w1", "W1");
    JiDockWidget* w2 = ji_dock_widget_new("w2", "W2");
    ji_dock_manager_add_widget(mgr, w1, a);
    ji_dock_manager_add_widget(mgr, w2, a);

    ji_dock_manager_set_active_widget(mgr, w2);
    if (ji_dock_manager_get_active_widget(mgr) != w2) { FAIL("active should be w2"); ji_dock_manager_destroy(mgr); return; }
    if (!w2->is_active) { FAIL("w2 should be active"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_destroy(mgr);
    PASS();
}

/* ---- Dock manager drag state ---- */
static void test_dock_manager_drag(void) {
    BEGIN_TEST("dock_manager_drag");
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("mgr new failed"); return; }

    JiDockArea* a = ji_dock_area_new("center");
    ji_dock_manager_set_root_area(mgr, a);

    JiDockWidget* w = ji_dock_widget_new("w", "W");
    ji_dock_manager_add_widget(mgr, w, a);

    ji_dock_manager_begin_drag(mgr, w, 100, 200);
    if (!ji_dock_manager_is_dragging(mgr)) { FAIL("should be dragging"); ji_dock_manager_destroy(mgr); return; }
    if (mgr->drag_state.dragged_widget != w) { FAIL("dragged widget mismatch"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_update_drag(mgr, 150, 250);
    if (mgr->drag_state.mouse_x != 150) { FAIL("mouse_x not updated"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_end_drag(mgr);
    if (ji_dock_manager_is_dragging(mgr)) { FAIL("should not be dragging"); ji_dock_manager_destroy(mgr); return; }

    ji_dock_manager_destroy(mgr);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Docking System Tests ===\n\n");

    test_dock_widget_new();
    test_dock_widget_set_title();
    test_dock_widget_floating();
    test_dock_widget_hide_show();
    test_dock_area_new();
    test_dock_area_add_remove();
    test_dock_area_active_tab();
    test_dock_manager_new();
    test_dock_manager_add_widget();
    test_dock_manager_float_dock();
    test_dock_manager_active();
    test_dock_manager_drag();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
