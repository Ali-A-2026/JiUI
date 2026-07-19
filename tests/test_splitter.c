/**
 * JiUI - Splitter System Tests
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

/* ---- Splitter lifecycle ---- */
static void test_splitter_new(void) {
    BEGIN_TEST("splitter_new");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("ji_splitter_new returned NULL"); return; }
    if (sp->orientation != JI_SPLITTER_HORIZONTAL) { FAIL("orientation mismatch"); ji_splitter_destroy(sp); return; }
    if (sp->panel_count != 0) { FAIL("panel_count should be 0"); ji_splitter_destroy(sp); return; }
    if (sp->handle_width != 4) { FAIL("handle_width should be 4"); ji_splitter_destroy(sp); return; }
    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter add panels ---- */
static void test_splitter_add_panels(void) {
    BEGIN_TEST("splitter_add_panels");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 50, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 50, 1);
    ji_splitter_add_panel(sp, (void*)0x3000, 50, 1);

    if (ji_splitter_panel_count(sp) != 3) { FAIL("panel count should be 3"); ji_splitter_destroy(sp); return; }

    JiSplitterPanel* p0 = ji_splitter_get_panel(sp, 0);
    if (!p0 || p0->content != (void*)0x1000) { FAIL("panel 0 content mismatch"); ji_splitter_destroy(sp); return; }
    if (p0->min_size != 50) { FAIL("panel 0 min_size should be 50"); ji_splitter_destroy(sp); return; }
    if (p0->stretch_factor != 1) { FAIL("panel 0 stretch should be 1"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter geometry recalculation ---- */
static void test_splitter_geometry(void) {
    BEGIN_TEST("splitter_geometry");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 2);
    ji_splitter_add_panel(sp, (void*)0x3000, 32, 1);

    /* Set rect: 1000px wide, 600px tall */
    JiRect rect = {0, 0, 1000, 600};
    ji_splitter_set_rect(sp, rect);

    /* Total available = 1000 - 2*4 = 992 */
    /* Stretch: 1+2+1=4 → panel0=248, panel1=496, panel2=248 */
    int sizes[3];
    ji_splitter_get_sizes(sp, sizes, 3);

    if (sizes[0] + sizes[1] + sizes[2] != 992) {
        printf("FAIL: total size %d != 992\n", sizes[0]+sizes[1]+sizes[2]); tests_failed++; ji_splitter_destroy(sp); return;
    }

    /* Panel 1 should be twice panel 0 */
    if (sizes[1] != sizes[0] * 2) { FAIL("panel 1 should be 2x panel 0"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter panel rects ---- */
static void test_splitter_panel_rects(void) {
    BEGIN_TEST("splitter_panel_rects");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 1);

    JiRect rect = {10, 20, 200, 400};
    ji_splitter_set_rect(sp, rect);

    JiRect r0 = ji_splitter_get_panel_rect(sp, 0);
    JiRect r1 = ji_splitter_get_panel_rect(sp, 1);

    /* Panel 0 should start at x=10 */
    if (r0.x != 10) { FAIL("panel 0 x should be 10"); ji_splitter_destroy(sp); return; }
    if (r0.y != 20) { FAIL("panel 0 y should be 20"); ji_splitter_destroy(sp); return; }
    if (r0.height != 400) { FAIL("panel 0 height should be 400"); ji_splitter_destroy(sp); return; }

    /* Panel 1 should start after panel 0 + handle */
    if (r1.x != r0.x + r0.width + 4) { FAIL("panel 1 x offset wrong"); ji_splitter_destroy(sp); return; }

    /* Handle rect */
    JiRect h0 = ji_splitter_get_handle_rect(sp, 0);
    if (h0.x != r0.x + r0.width) { FAIL("handle x wrong"); ji_splitter_destroy(sp); return; }
    if (h0.width != 4) { FAIL("handle width should be 4"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter drag ---- */
static void test_splitter_drag(void) {
    BEGIN_TEST("splitter_drag");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 1);

    JiRect rect = {0, 0, 400, 300};
    ji_splitter_set_rect(sp, rect);

    int sizes[2];
    ji_splitter_get_sizes(sp, sizes, 2);
    int initial0 = sizes[0];
    int initial1 = sizes[1];

    /* Handle is at x = initial0, y=0, width=4, height=300 */
    int handle_x = initial0;
    /* Begin drag on handle */
    if (!ji_splitter_begin_drag(sp, handle_x + 2, 150)) { FAIL("begin_drag failed"); ji_splitter_destroy(sp); return; }
    if (!ji_splitter_is_dragging(sp)) { FAIL("should be dragging"); ji_splitter_destroy(sp); return; }

    /* Drag right by 50px */
    ji_splitter_update_drag(sp, handle_x + 52, 150);

    ji_splitter_get_sizes(sp, sizes, 2);
    if (sizes[0] <= initial0) { FAIL("panel 0 should have grown"); ji_splitter_destroy(sp); return; }
    if (sizes[0] + sizes[1] != initial0 + initial1) { FAIL("total should be preserved"); ji_splitter_destroy(sp); return; }

    ji_splitter_end_drag(sp);
    if (ji_splitter_is_dragging(sp)) { FAIL("should not be dragging"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter hit test ---- */
static void test_splitter_hit_test(void) {
    BEGIN_TEST("splitter_hit_test");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 1);

    JiRect rect = {0, 0, 400, 300};
    ji_splitter_set_rect(sp, rect);

    int sizes[2];
    ji_splitter_get_sizes(sp, sizes, 2);
    int handle_x = sizes[0];

    /* Hit the handle */
    int idx = ji_splitter_hit_test_handle(sp, handle_x + 2, 150);
    if (idx != 0) { FAIL("should hit handle 0"); ji_splitter_destroy(sp); return; }

    /* Miss (click in panel area) */
    idx = ji_splitter_hit_test_handle(sp, 10, 150);
    if (idx != -1) { FAIL("should not hit any handle"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter collapse ---- */
static void test_splitter_collapse(void) {
    BEGIN_TEST("splitter_collapse");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 1);

    ji_splitter_set_collapsible(sp, 0, true);
    if (ji_splitter_is_collapsed(sp, 0)) { FAIL("panel 0 should not be collapsed initially"); ji_splitter_destroy(sp); return; }

    ji_splitter_toggle_collapse(sp, 0);
    if (!ji_splitter_is_collapsed(sp, 0)) { FAIL("panel 0 should be collapsed"); ji_splitter_destroy(sp); return; }

    JiRect rect = {0, 0, 400, 300};
    ji_splitter_set_rect(sp, rect);

    int sizes[2];
    ji_splitter_get_sizes(sp, sizes, 2);
    if (sizes[0] != 0) { FAIL("collapsed panel size should be 0"); ji_splitter_destroy(sp); return; }
    if (sizes[1] == 0) { FAIL("non-collapsed panel should have size"); ji_splitter_destroy(sp); return; }

    /* Uncollapse */
    ji_splitter_toggle_collapse(sp, 0);
    if (ji_splitter_is_collapsed(sp, 0)) { FAIL("panel 0 should not be collapsed"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Splitter vertical ---- */
static void test_splitter_vertical(void) {
    BEGIN_TEST("splitter_vertical");
    JiSplitter* sp = ji_splitter_new(JI_SPLITTER_VERTICAL);
    if (!sp) { FAIL("new failed"); return; }

    ji_splitter_add_panel(sp, (void*)0x1000, 32, 1);
    ji_splitter_add_panel(sp, (void*)0x2000, 32, 1);

    JiRect rect = {0, 0, 300, 600};
    ji_splitter_set_rect(sp, rect);

    if (ji_splitter_get_orientation(sp) != JI_SPLITTER_VERTICAL) { FAIL("orientation should be VERTICAL"); ji_splitter_destroy(sp); return; }

    int sizes[2];
    ji_splitter_get_sizes(sp, sizes, 2);
    if (sizes[0] + sizes[1] != 600 - 4) { FAIL("total vertical size mismatch"); ji_splitter_destroy(sp); return; }

    JiRect r0 = ji_splitter_get_panel_rect(sp, 0);
    if (r0.height != sizes[0]) { FAIL("panel 0 height should match size"); ji_splitter_destroy(sp); return; }
    if (r0.width != 300) { FAIL("panel 0 width should be 300"); ji_splitter_destroy(sp); return; }

    ji_splitter_destroy(sp);
    PASS();
}

/* ---- Main ---- */
int main(void) {
    printf("=== JiUI Splitter System Tests ===\n\n");

    test_splitter_new();
    test_splitter_add_panels();
    test_splitter_geometry();
    test_splitter_panel_rects();
    test_splitter_drag();
    test_splitter_hit_test();
    test_splitter_collapse();
    test_splitter_vertical();

    printf("\n=== Results: %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
