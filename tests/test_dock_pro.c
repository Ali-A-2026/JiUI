/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_dock_pro.c
 * @brief Tests for the professional docking system.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

/* =========================================================================
 * Pro Dock Manager Tests
 * ========================================================================= */

static void test_dock_pro_create(void) {
    JiDockProManager* mgr = ji_dock_pro_new();
    ASSERT_TRUE(mgr != NULL);
    ASSERT_TRUE(ji_dock_pro_floating_count(mgr) == 0);
    ASSERT_TRUE(ji_dock_pro_workspace_count(mgr) == 0);
    ASSERT_TRUE(ji_dock_pro_is_locked(mgr) == false);
    ASSERT_TRUE(ji_dock_pro_is_hover_preview(mgr) == true);
    ji_dock_pro_destroy(mgr);
}

static void test_dock_pro_destroy_null(void) {
    ji_dock_pro_destroy(NULL);
}

static void test_dock_pro_lock_unlock(void) {
    JiDockProManager* mgr = ji_dock_pro_new();
    ASSERT_TRUE(mgr != NULL);

    ASSERT_TRUE(ji_dock_pro_is_locked(mgr) == false);
    ji_dock_pro_lock(mgr);
    ASSERT_TRUE(ji_dock_pro_is_locked(mgr) == true);
    ji_dock_pro_unlock(mgr);
    ASSERT_TRUE(ji_dock_pro_is_locked(mgr) == false);

    /* Test lock flags */
    JiDockLock flags;
    memset(&flags, 0, sizeof(flags));
    flags.locked = true;
    flags.allow_tear_off = true;
    ji_dock_pro_set_lock_flags(mgr, flags);
    JiDockLock got = ji_dock_pro_get_lock_flags(mgr);
    ASSERT_TRUE(got.locked == true);
    ASSERT_TRUE(got.allow_tear_off == true);

    ji_dock_pro_destroy(mgr);
}

static void test_tab_group_create(void) {
    JiTabGroup* group = ji_tab_group_new(NULL, JI_TAB_GROUP_HORIZONTAL);
    ASSERT_TRUE(group != NULL);
    ASSERT_TRUE(ji_tab_group_count(group) == 0);
    ASSERT_TRUE(ji_tab_group_get_active(group) == NULL);
    ji_tab_group_destroy(group);
}

static void test_tab_group_add_remove(void) {
    JiTabGroup* group = ji_tab_group_new(NULL, JI_TAB_GROUP_HORIZONTAL);
    ASSERT_TRUE(group != NULL);

    /* Create dock widgets to add */
    JiDockWidget* w1 = ji_dock_widget_new("panel1", "Panel 1");
    JiDockWidget* w2 = ji_dock_widget_new("panel2", "Panel 2");

    ASSERT_TRUE(ji_tab_group_add(group, w1) == true);
    ASSERT_TRUE(ji_tab_group_count(group) == 1);
    ASSERT_TRUE(ji_tab_group_add(group, w2) == true);
    ASSERT_TRUE(ji_tab_group_count(group) == 2);

    /* Set active */
    ASSERT_TRUE(ji_tab_group_set_active(group, 1) == true);
    ASSERT_TRUE(ji_tab_group_get_active(group) == w2);

    /* Remove */
    ASSERT_TRUE(ji_tab_group_remove(group, w1) == true);
    ASSERT_TRUE(ji_tab_group_count(group) == 1);

    /* Remove non-existent */
    ASSERT_TRUE(ji_tab_group_remove(group, w1) == false);

    ji_tab_group_destroy(group);
    ji_dock_widget_destroy(w1);
    ji_dock_widget_destroy(w2);
}

static void test_tab_group_orientation(void) {
    JiTabGroup* group = ji_tab_group_new(NULL, JI_TAB_GROUP_HORIZONTAL);
    ASSERT_TRUE(group != NULL);

    ji_tab_group_set_orientation(group, JI_TAB_GROUP_VERTICAL);
    ASSERT_TRUE(group->orientation == JI_TAB_GROUP_VERTICAL);

    ji_tab_group_destroy(group);
}

static void test_floating_panel_create(void) {
    JiDockWidget* w = ji_dock_widget_new("float_test", "Float Test");
    JiRect geom = {100, 100, 400, 300};
    JiFloatingPanel* panel = ji_floating_panel_new(w, geom);
    ASSERT_TRUE(panel != NULL);
    ASSERT_TRUE(panel->geometry.x == 100);
    ASSERT_TRUE(panel->geometry.width == 400);

    ji_floating_panel_set_title(panel, "My Panel");
    ASSERT_TRUE(strcmp(panel->title, "My Panel") == 0);

    ji_floating_panel_set_always_on_top(panel, true);
    ASSERT_TRUE(panel->always_on_top == true);

    JiRect new_geom = {200, 200, 800, 600};
    ji_floating_panel_set_geometry(panel, new_geom);
    ASSERT_TRUE(panel->geometry.x == 200);
    ASSERT_TRUE(panel->geometry.width == 800);

    ji_floating_panel_destroy(panel, false);
    ji_dock_widget_destroy(w);
}

static void test_auto_hide_dock_create(void) {
    JiDockWidget* w = ji_dock_widget_new("auto_hide", "Auto Hide");
    JiAutoHideDock* dock = ji_auto_hide_dock_new(w, NULL, 0);
    ASSERT_TRUE(dock != NULL);
    ASSERT_TRUE(ji_auto_hide_dock_is_visible(dock) == false);

    ji_auto_hide_dock_show(dock);
    /* Simulate animation completion */
    dock->animation_progress = 1.0f;
    dock->state = JI_DOCK_AUTO_HIDE_SHOWN;
    ASSERT_TRUE(ji_auto_hide_dock_is_visible(dock) == true);

    ji_auto_hide_dock_toggle_pin(dock);
    ASSERT_TRUE(dock->pin_state == JI_DOCK_PIN_PINNED);

    ji_auto_hide_dock_destroy(dock);
    ji_dock_widget_destroy(w);
}

static void test_auto_hide_dock_animation(void) {
    JiDockWidget* w = ji_dock_widget_new("anim_dock", "Anim Dock");
    JiAutoHideDock* dock = ji_auto_hide_dock_new(w, NULL, 0);
    ASSERT_TRUE(dock != NULL);

    /* Start hidden */
    dock->animation_progress = 0.0f;
    dock->state = JI_DOCK_AUTO_HIDE_HIDDEN;

    /* Show */
    ji_auto_hide_dock_show(dock);
    ASSERT_TRUE(dock->state == JI_DOCK_AUTO_HIDE_ANIMATING);

    /* Simulate animation frames */
    for (int i = 0; i < 30; i++) {
        ji_auto_hide_dock_update(dock, 0.016f);
    }

    /* Should be shown or close to it */
    ASSERT_TRUE(dock->animation_progress > 0.5f);

    ji_auto_hide_dock_destroy(dock);
    ji_dock_widget_destroy(w);
}

static void test_workspace_save_load(void) {
    JiDockProManager* mgr = ji_dock_pro_new();
    ASSERT_TRUE(mgr != NULL);

    /* Save a workspace */
    ASSERT_TRUE(ji_dock_pro_save_workspace(mgr, "default") == true);
    ASSERT_TRUE(ji_dock_pro_workspace_count(mgr) == 1);

    /* Get workspace by index */
    const JiWorkspace* ws = ji_dock_pro_get_workspace(mgr, 0);
    ASSERT_TRUE(ws != NULL);
    ASSERT_TRUE(strcmp(ws->name, "default") == 0);

    /* Load workspace */
    ASSERT_TRUE(ji_dock_pro_load_workspace(mgr, "default") == true);

    /* Load non-existent */
    ASSERT_TRUE(ji_dock_pro_load_workspace(mgr, "nonexistent") == false);

    /* Save layout to string */
    char* json = ji_dock_pro_save_layout(mgr);
    ASSERT_TRUE(json != NULL);
    ASSERT_TRUE(strlen(json) > 0);
    ji_free(json);

    ji_dock_pro_destroy(mgr);
}

static void test_multi_monitor(void) {
    JiDockProManager* mgr = ji_dock_pro_new();
    ASSERT_TRUE(mgr != NULL);

    /* Default monitor count */
    ASSERT_TRUE(ji_dock_pro_monitor_count(mgr) >= 1);

    /* Set monitors */
    JiRect geoms[2] = {{0,0,1920,1080}, {1920,0,1920,1080}};
    ji_dock_pro_set_monitors(mgr, 2, geoms);
    ASSERT_TRUE(ji_dock_pro_monitor_count(mgr) == 2);

    /* Get monitor geometry */
    JiRect g = ji_dock_pro_monitor_geometry(mgr, 0);
    ASSERT_TRUE(g.width > 0);

    /* Move floating panel to monitor */
    JiDockWidget* w = ji_dock_widget_new("multi_mon", "Multi Monitor");
    JiFloatingPanel* panel = ji_floating_panel_new(w, geoms[0]);
    ASSERT_TRUE(ji_dock_pro_move_to_monitor(mgr, panel, 1) == true);
    ASSERT_TRUE(panel->monitor_index == 1);

    ji_floating_panel_destroy(panel, false);
    ji_dock_widget_destroy(w);
    ji_dock_pro_destroy(mgr);
}

static void test_hover_preview(void) {
    JiDockProManager* mgr = ji_dock_pro_new();
    ASSERT_TRUE(mgr != NULL);

    ASSERT_TRUE(ji_dock_pro_is_hover_preview(mgr) == true);

    ji_dock_pro_set_hover_preview(mgr, false);
    ASSERT_TRUE(ji_dock_pro_is_hover_preview(mgr) == false);

    ji_dock_pro_set_hover_preview(mgr, true);
    ASSERT_TRUE(ji_dock_pro_is_hover_preview(mgr) == true);

    JiRect preview = {50, 50, 200, 200};
    ji_dock_pro_set_preview_rect(mgr, preview);
    JiRect got = ji_dock_pro_get_preview_rect(mgr);
    ASSERT_TRUE(got.x == 50 && got.width == 200);

    ji_dock_pro_destroy(mgr);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== Pro Docking Tests ===\n");

    printf("-- Manager Tests --\n");
    TEST(test_dock_pro_create);
    TEST(test_dock_pro_destroy_null);
    TEST(test_dock_pro_lock_unlock);

    printf("-- Tab Group Tests --\n");
    TEST(test_tab_group_create);
    TEST(test_tab_group_add_remove);
    TEST(test_tab_group_orientation);

    printf("-- Floating Panel Tests --\n");
    TEST(test_floating_panel_create);

    printf("-- Auto-Hide Tests --\n");
    TEST(test_auto_hide_dock_create);
    TEST(test_auto_hide_dock_animation);

    printf("-- Workspace Tests --\n");
    TEST(test_workspace_save_load);

    printf("-- Multi-Monitor Tests --\n");
    TEST(test_multi_monitor);

    printf("-- Hover Preview Tests --\n");
    TEST(test_hover_preview);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_run == tests_passed) ? 0 : 1;
}
