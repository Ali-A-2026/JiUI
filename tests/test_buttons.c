/**
 * JiUI - Button Widget Tests (ToolButton, CommandLinkButton)
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

/* ---- ToolButton ---- */
static void test_tool_button(void) {
    BEGIN_TEST("tool_button_basic");
    JiToolButton* btn = ji_tool_button_new("Cut");
    if (!btn) { FAIL("new failed"); return; }
    if (strcmp(ji_tool_button_get_text(btn), "Cut") != 0) { FAIL("text mismatch"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_text(btn, "Copy");
    if (strcmp(ji_tool_button_get_text(btn), "Copy") != 0) { FAIL("set text failed"); ji_tool_button_destroy(btn); return; }
    if (btn->button_style != JI_TOOL_BUTTON_TEXT_BESIDE_ICON) { FAIL("default style wrong"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_style(btn, JI_TOOL_BUTTON_ICON_ONLY);
    if (btn->button_style != JI_TOOL_BUTTON_ICON_ONLY) { FAIL("style not set"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_destroy(btn);
    PASS();
}

static void test_tool_button_checkable(void) {
    BEGIN_TEST("tool_button_checkable");
    JiToolButton* btn = ji_tool_button_new("Bold");
    ji_tool_button_set_checkable(btn, true);
    if (btn->is_checkable != true) { FAIL("should be checkable"); ji_tool_button_destroy(btn); return; }
    if (ji_tool_button_is_checked(btn)) { FAIL("should not be checked initially"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_checked(btn, true);
    if (!ji_tool_button_is_checked(btn)) { FAIL("should be checked"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_checkable(btn, false);
    ji_tool_button_set_checked(btn, true);
    if (ji_tool_button_is_checked(btn)) { FAIL("non-checkable should ignore set_checked"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_destroy(btn);
    PASS();
}

static void test_tool_button_icon(void) {
    BEGIN_TEST("tool_button_icon");
    JiToolButton* btn = ji_tool_button_new("Open");
    ji_tool_button_set_icon(btn, "document-open");
    if (!btn->icon_name || strcmp(btn->icon_name, "document-open") != 0) { FAIL("icon not set"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_icon(btn, "document-save");
    if (strcmp(btn->icon_name, "document-save") != 0) { FAIL("icon not updated"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_destroy(btn);
    PASS();
}

static void test_tool_button_popup(void) {
    BEGIN_TEST("tool_button_popup");
    JiToolButton* btn = ji_tool_button_new("Menu");
    ji_tool_button_set_popup_mode(btn, JI_TOOL_BUTTON_INSTANT_POPUP);
    if (btn->popup_mode != JI_TOOL_BUTTON_INSTANT_POPUP) { FAIL("popup mode not set"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_set_menu(btn, (struct JiMenu*)0x1); /* dummy pointer */
    if (!btn->is_arrow_visible) { FAIL("arrow should be visible with menu"); ji_tool_button_destroy(btn); return; }
    ji_tool_button_destroy(btn);
    PASS();
}

/* ---- CommandLinkButton ---- */
static void test_command_link_button(void) {
    BEGIN_TEST("command_link_button");
    JiCommandLinkButton* btn = ji_command_link_button_new("Add Printer", "Add a local or network printer");
    if (!btn) { FAIL("new failed"); return; }
    if (strcmp(ji_command_link_button_get_text(btn), "Add Printer") != 0) { FAIL("text mismatch"); ji_command_link_button_destroy(btn); return; }
    if (strcmp(ji_command_link_button_get_description(btn), "Add a local or network printer") != 0) { FAIL("desc mismatch"); ji_command_link_button_destroy(btn); return; }
    ji_command_link_button_set_text(btn, "Remove Printer");
    ji_command_link_button_set_description(btn, "Remove a printer from the list");
    if (strcmp(ji_command_link_button_get_text(btn), "Remove Printer") != 0) { FAIL("set text failed"); ji_command_link_button_destroy(btn); return; }
    if (strcmp(ji_command_link_button_get_description(btn), "Remove a printer from the list") != 0) { FAIL("set desc failed"); ji_command_link_button_destroy(btn); return; }
    ji_command_link_button_set_default(btn, true);
    if (!ji_command_link_button_is_default(btn)) { FAIL("should be default"); ji_command_link_button_destroy(btn); return; }
    ji_command_link_button_destroy(btn);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Button Widget Tests ===\n\n");

    test_tool_button();
    test_tool_button_checkable();
    test_tool_button_icon();
    test_tool_button_popup();
    test_command_link_button();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
