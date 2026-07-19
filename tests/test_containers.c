/**
 * JiUI - Container Widget Tests
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

/* ---- GroupBox ---- */
static void test_group_box_new(void) {
    BEGIN_TEST("group_box_new");
    JiGroupBox* box = ji_group_box_new("Settings");
    if (!box) { FAIL("new returned NULL"); return; }
    if (!box->title || strcmp(box->title, "Settings") != 0) { FAIL("title mismatch"); ji_group_box_destroy(box); return; }
    if (box->is_checkable) { FAIL("should not be checkable"); ji_group_box_destroy(box); return; }
    if (!box->is_checked) { FAIL("should be checked by default"); ji_group_box_destroy(box); return; }
    ji_group_box_destroy(box);
    PASS();
}

static void test_group_box_checkable(void) {
    BEGIN_TEST("group_box_checkable");
    JiGroupBox* box = ji_group_box_new("Test");
    ji_group_box_set_checkable(box, true);
    if (!ji_group_box_is_checkable(box)) { FAIL("should be checkable"); ji_group_box_destroy(box); return; }
    ji_group_box_set_checked(box, false);
    if (ji_group_box_is_checked(box)) { FAIL("should be unchecked"); ji_group_box_destroy(box); return; }
    ji_group_box_destroy(box);
    PASS();
}

/* ---- Frame ---- */
static void test_frame_new(void) {
    BEGIN_TEST("frame_new");
    JiFrame* f = ji_frame_new();
    if (!f) { FAIL("new returned NULL"); return; }
    if (ji_frame_get_shape(f) != JI_FRAME_STYLED_PANEL) { FAIL("default shape should be STYLED_PANEL"); ji_frame_destroy(f); return; }
    if (ji_frame_get_shadow(f) != JI_FRAME_SUNKEN) { FAIL("default shadow should be SUNKEN"); ji_frame_destroy(f); return; }
    if (ji_frame_get_line_width(f) != 1) { FAIL("default line width should be 1"); ji_frame_destroy(f); return; }
    ji_frame_set_shape(f, JI_FRAME_BOX);
    if (ji_frame_get_shape(f) != JI_FRAME_BOX) { FAIL("shape should be BOX"); ji_frame_destroy(f); return; }
    ji_frame_destroy(f);
    PASS();
}

/* ---- ScrollArea ---- */
static void test_scroll_area_new(void) {
    BEGIN_TEST("scroll_area_new");
    JiScrollArea* sa = ji_scroll_area_new();
    if (!sa) { FAIL("new returned NULL"); return; }
    if (sa->h_policy != JI_SCROLL_POLICY_AS_NEEDED) { FAIL("default h_policy should be AS_NEEDED"); ji_scroll_area_destroy(sa); return; }
    if (sa->scrollbar_width != 16) { FAIL("default scrollbar width should be 16"); ji_scroll_area_destroy(sa); return; }
    ji_scroll_area_set_scroll(sa, 50, 100);
    int x, y;
    ji_scroll_area_get_scroll(sa, &x, &y);
    if (x != 50 || y != 100) { FAIL("scroll position mismatch"); ji_scroll_area_destroy(sa); return; }
    ji_scroll_area_destroy(sa);
    PASS();
}

/* ---- TabWidget ---- */
static void test_tab_widget_new(void) {
    BEGIN_TEST("tab_widget_new");
    JiTabWidget* tw = ji_tab_widget_new();
    if (!tw) { FAIL("new returned NULL"); return; }
    if (ji_tab_widget_count(tw) != 0) { FAIL("count should be 0"); ji_tab_widget_destroy(tw); return; }
    ji_tab_widget_destroy(tw);
    PASS();
}

static void test_tab_widget_add_remove(void) {
    BEGIN_TEST("tab_widget_add_remove");
    JiTabWidget* tw = ji_tab_widget_new();
    int idx1 = ji_tab_widget_add_tab(tw, "Tab 1", NULL);
    int idx2 = ji_tab_widget_add_tab(tw, "Tab 2", NULL);
    int idx3 = ji_tab_widget_add_tab(tw, "Tab 3", NULL);
    if (idx1 != 0 || idx2 != 1 || idx3 != 2) { FAIL("indices wrong"); ji_tab_widget_destroy(tw); return; }
    if (ji_tab_widget_count(tw) != 3) { FAIL("count should be 3"); ji_tab_widget_destroy(tw); return; }
    if (strcmp(ji_tab_widget_get_tab_label(tw, 1), "Tab 2") != 0) { FAIL("label mismatch"); ji_tab_widget_destroy(tw); return; }

    ji_tab_widget_set_current_index(tw, 2);
    if (ji_tab_widget_get_current_index(tw) != 2) { FAIL("current should be 2"); ji_tab_widget_destroy(tw); return; }

    ji_tab_widget_remove_tab(tw, 1);
    if (ji_tab_widget_count(tw) != 2) { FAIL("count should be 2 after remove"); ji_tab_widget_destroy(tw); return; }
    if (strcmp(ji_tab_widget_get_tab_label(tw, 1), "Tab 3") != 0) { FAIL("label after remove mismatch"); ji_tab_widget_destroy(tw); return; }

    ji_tab_widget_destroy(tw);
    PASS();
}

/* ---- StackedWidget ---- */
static void test_stacked_widget(void) {
    BEGIN_TEST("stacked_widget");
    JiStackedWidget* sw = ji_stacked_widget_new();
    if (!sw) { FAIL("new returned NULL"); return; }

    int idx0 = ji_stacked_widget_add_page(sw, (JiControl*)0x100);
    int idx1 = ji_stacked_widget_add_page(sw, (JiControl*)0x200);
    if (idx0 != 0 || idx1 != 1) { FAIL("indices wrong"); ji_stacked_widget_destroy(sw); return; }
    if (ji_stacked_widget_count(sw) != 2) { FAIL("count should be 2"); ji_stacked_widget_destroy(sw); return; }

    ji_stacked_widget_set_current_index(sw, 1);
    if (ji_stacked_widget_get_current_index(sw) != 1) { FAIL("current should be 1"); ji_stacked_widget_destroy(sw); return; }
    if (ji_stacked_widget_get_current_page(sw) != (JiControl*)0x200) { FAIL("current page mismatch"); ji_stacked_widget_destroy(sw); return; }

    if (ji_stacked_widget_index_of(sw, (JiControl*)0x100) != 0) { FAIL("index_of 0x100 should be 0"); ji_stacked_widget_destroy(sw); return; }
    if (ji_stacked_widget_index_of(sw, (JiControl*)0x999) != -1) { FAIL("index_of 0x999 should be -1"); ji_stacked_widget_destroy(sw); return; }

    ji_stacked_widget_destroy(sw);
    PASS();
}

/* ---- ToolBar ---- */
static void test_tool_bar(void) {
    BEGIN_TEST("tool_bar");
    JiToolBar* tb = ji_tool_bar_new();
    if (!tb) { FAIL("new returned NULL"); return; }
    if (tb->area != JI_TOOL_BAR_TOP) { FAIL("default area should be TOP"); ji_tool_bar_destroy(tb); return; }
    if (!tb->is_movable) { FAIL("should be movable by default"); ji_tool_bar_destroy(tb); return; }

    ji_tool_bar_add_action(tb, "new", "New");
    ji_tool_bar_add_separator(tb);
    ji_tool_bar_add_action(tb, "open", "Open");
    if (tb->item_count != 3) { FAIL("item count should be 3"); ji_tool_bar_destroy(tb); return; }
    if (!tb->items[1].is_separator) { FAIL("item 1 should be separator"); ji_tool_bar_destroy(tb); return; }

    ji_tool_bar_destroy(tb);
    PASS();
}

/* ---- StatusBar ---- */
static void test_status_bar(void) {
    BEGIN_TEST("status_bar");
    JiStatusBar* sb = ji_status_bar_new();
    if (!sb) { FAIL("new returned NULL"); return; }
    if (sb->height != 22) { FAIL("default height should be 22"); ji_status_bar_destroy(sb); return; }

    ji_status_bar_show_message(sb, "Ready");
    if (!sb->current_message || strcmp(sb->current_message, "Ready") != 0) { FAIL("message mismatch"); ji_status_bar_destroy(sb); return; }

    ji_status_bar_clear_message(sb);
    if (sb->current_message != NULL) { FAIL("message should be NULL after clear"); ji_status_bar_destroy(sb); return; }

    ji_status_bar_destroy(sb);
    PASS();
}

/* ---- Menu ---- */
static void test_menu(void) {
    BEGIN_TEST("menu");
    JiMenu* m = ji_menu_new("File");
    if (!m) { FAIL("new returned NULL"); return; }
    if (!m->title || strcmp(m->title, "File") != 0) { FAIL("title mismatch"); ji_menu_destroy(m); return; }

    int idx_new = ji_menu_add_action(m, "New");
    ji_menu_set_shortcut(m, idx_new, "Ctrl+N");
    ji_menu_add_separator(m);
    int idx_exit = ji_menu_add_action(m, "Exit");
    ji_menu_set_shortcut(m, idx_exit, "Ctrl+Q");

    if (ji_menu_item_count(m) != 3) { FAIL("item count should be 3"); ji_menu_destroy(m); return; }
    if (m->items[0].type != JI_MENU_ITEM_ACTION) { FAIL("item 0 should be ACTION"); ji_menu_destroy(m); return; }
    if (m->items[1].type != JI_MENU_ITEM_SEPARATOR) { FAIL("item 1 should be SEPARATOR"); ji_menu_destroy(m); return; }
    if (strcmp(m->items[0].shortcut, "Ctrl+N") != 0) { FAIL("shortcut mismatch"); ji_menu_destroy(m); return; }

    /* Checkable */
    ji_menu_set_checkable(m, idx_new, true);
    ji_menu_set_checked(m, idx_new, true);
    if (!m->items[idx_new].is_checkable) { FAIL("should be checkable"); ji_menu_destroy(m); return; }
    if (!m->items[idx_new].is_checked) { FAIL("should be checked"); ji_menu_destroy(m); return; }

    /* Submenu */
    JiMenu* sub = ji_menu_new("Recent");
    ji_menu_add_action(sub, "File 1");
    int sub_idx = ji_menu_add_submenu(m, sub);
    if (m->items[sub_idx].type != JI_MENU_ITEM_SUBMENU) { FAIL("should be SUBMENU"); ji_menu_destroy(m); return; }
    if (m->items[sub_idx].submenu != sub) { FAIL("submenu pointer mismatch"); ji_menu_destroy(m); return; }

    ji_menu_destroy(m);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Container Widget Tests ===\n\n");

    test_group_box_new();
    test_group_box_checkable();
    test_frame_new();
    test_scroll_area_new();
    test_tab_widget_new();
    test_tab_widget_add_remove();
    test_stacked_widget();
    test_tool_bar();
    test_status_bar();
    test_menu();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
