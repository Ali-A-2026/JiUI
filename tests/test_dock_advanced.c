/**
 * JiUI - Dock Advanced Tests (Overlay, TabBar, Serializer)
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

/* ---- DockOverlay ---- */
static void test_dock_overlay(void) {
    BEGIN_TEST("dock_overlay");
    JiDockOverlay* o = ji_dock_overlay_new();
    if (!o) { FAIL("new failed"); return; }
    if (o->is_visible) { FAIL("should start hidden"); ji_dock_overlay_destroy(o); return; }
    ji_dock_overlay_show(o, 0, 0, 400, 300);
    if (!o->is_visible) { FAIL("should be visible after show"); ji_dock_overlay_destroy(o); return; }
    ji_dock_overlay_hide(o);
    if (o->is_visible) { FAIL("should be hidden after hide"); ji_dock_overlay_destroy(o); return; }
    ji_dock_overlay_show(o, 0, 0, 400, 300);
    ji_dock_overlay_set_highlight(o, JI_DOCK_OVERLAY_LEFT);
    if (ji_dock_overlay_get_highlight(o) != JI_DOCK_OVERLAY_LEFT) { FAIL("highlight should be LEFT"); ji_dock_overlay_destroy(o); return; }
    ji_dock_overlay_update_rects(o);
    JiDockOverlayArea hit = ji_dock_overlay_hit_test(o, o->center_rect_x, o->center_rect_y);
    if (hit != JI_DOCK_OVERLAY_CENTER) { FAIL("center hit should be CENTER"); ji_dock_overlay_destroy(o); return; }
    ji_dock_overlay_destroy(o);
    PASS();
}

/* ---- DockTabBar ---- */
static void test_dock_tab_bar(void) {
    BEGIN_TEST("dock_tab_bar");
    JiDockTabBar* bar = ji_dock_tab_bar_new();
    if (!bar) { FAIL("new failed"); return; }
    if (ji_dock_tab_bar_count(bar) != 0) { FAIL("should start empty"); ji_dock_tab_bar_destroy(bar); return; }
    ji_dock_tab_bar_add_tab(bar, "Tab 1", NULL);
    ji_dock_tab_bar_add_tab(bar, "Tab 2", NULL);
    ji_dock_tab_bar_add_tab(bar, "Tab 3", NULL);
    if (ji_dock_tab_bar_count(bar) != 3) { FAIL("count should be 3"); ji_dock_tab_bar_destroy(bar); return; }
    if (ji_dock_tab_bar_get_current(bar) != 0) { FAIL("current should be 0"); ji_dock_tab_bar_destroy(bar); return; }
    ji_dock_tab_bar_set_current(bar, 2);
    if (ji_dock_tab_bar_get_current(bar) != 2) { FAIL("current should be 2"); ji_dock_tab_bar_destroy(bar); return; }
    if (strcmp(ji_dock_tab_bar_get_tab_label(bar, 0), "Tab 1") != 0) { FAIL("label 0 wrong"); ji_dock_tab_bar_destroy(bar); return; }
    ji_dock_tab_bar_set_tab_label(bar, 1, "Modified");
    if (strcmp(ji_dock_tab_bar_get_tab_label(bar, 1), "Modified") != 0) { FAIL("label 1 not updated"); ji_dock_tab_bar_destroy(bar); return; }
    ji_dock_tab_bar_remove_tab(bar, 1);
    if (ji_dock_tab_bar_count(bar) != 2) { FAIL("count should be 2 after remove"); ji_dock_tab_bar_destroy(bar); return; }
    ji_dock_tab_bar_destroy(bar);
    PASS();
}

/* ---- DockSerializer ---- */
static void test_dock_serializer(void) {
    BEGIN_TEST("dock_serializer");
    JiDockSerializer* ser = ji_dock_serializer_new();
    if (!ser) { FAIL("new failed"); return; }
    JiDockManager* mgr = ji_dock_manager_new();
    if (!mgr) { FAIL("manager new failed"); ji_dock_serializer_destroy(ser); return; }
    ji_dock_serializer_save(ser, mgr);
    const char* data = ji_dock_serializer_get_data(ser);
    if (!data || strncmp(data, "dock_manager", 12) != 0) { FAIL("save data should start with dock_manager"); ji_dock_manager_destroy(mgr); ji_dock_serializer_destroy(ser); return; }
    if (!ji_dock_serializer_load(ser, mgr, "dock_manager\n  area_count=0\n")) { FAIL("load should succeed with valid header"); ji_dock_manager_destroy(mgr); ji_dock_serializer_destroy(ser); return; }
    if (ji_dock_serializer_load(ser, mgr, "invalid")) { FAIL("load should fail with invalid header"); ji_dock_manager_destroy(mgr); ji_dock_serializer_destroy(ser); return; }
    ji_dock_manager_destroy(mgr);
    ji_dock_serializer_destroy(ser);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Dock Advanced Tests ===\n\n");

    test_dock_overlay();
    test_dock_tab_bar();
    test_dock_serializer();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
