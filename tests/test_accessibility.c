#include "jiui/ji_accessibility.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Test Framework
 * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) \
    static void name(void); \
    static void name(void)

#define RUN_TEST(name) do { \
    g_tests_run++; \
    printf("  [RUN] %s ... ", #name); \
    name(); \
    g_tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return; \
    } \
} while(0)

/* =========================================================================
 * Accessibility Tests
 * ========================================================================= */

TEST(test_a11y_create)
{
    JiA11yManager* mgr = ji_a11y_new();
    ASSERT(mgr != NULL);
    ASSERT(mgr->node_count == 0);
    ASSERT(mgr->next_id == 1);
    ASSERT(mgr->focused_node == 0);
    ASSERT(mgr->high_contrast == false);
    ASSERT(mgr->magnification == 1);
    ji_a11y_free(mgr);
}

TEST(test_a11y_register_node)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id1 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_WINDOW, "Main Window");
    ASSERT(id1 > 0);
    ASSERT(mgr->node_count == 1);

    uint32_t id2 = ji_a11y_register_node(mgr, id1, JI_A11Y_ROLE_BUTTON, "OK");
    ASSERT(id2 > 0);
    ASSERT(id2 != id1);
    ASSERT(mgr->node_count == 2);

    JiA11yNode* node = ji_a11y_get_node(mgr, id2);
    ASSERT(node != NULL);
    ASSERT(node->role == JI_A11Y_ROLE_BUTTON);
    ASSERT(strcmp(node->name, "OK") == 0);
    ASSERT(node->parent_id == id1);
    ASSERT(node->visible == true);

    ji_a11y_free(mgr);
}

TEST(test_a11y_unregister_node)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id1 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_WINDOW, "Window");
    uint32_t id2 = ji_a11y_register_node(mgr, id1, JI_A11Y_ROLE_BUTTON, "Button");
    uint32_t id3 = ji_a11y_register_node(mgr, id2, JI_A11Y_ROLE_LABEL, "Label");

    ASSERT(mgr->node_count == 3);

    /* Unregister button — label should be re-parented to window */
    ASSERT(ji_a11y_unregister_node(mgr, id2) == 0);
    ASSERT(mgr->node_count == 2);

    JiA11yNode* label = ji_a11y_get_node(mgr, id3);
    ASSERT(label != NULL);
    ASSERT(label->parent_id == id1);

    ji_a11y_free(mgr);
}

TEST(test_a11y_states)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_CHECKBOX, "Check");

    ASSERT(ji_a11y_add_state(mgr, id, JI_A11Y_STATE_FOCUSED) == 0);
    ASSERT(ji_a11y_get_node(mgr, id)->states & JI_A11Y_STATE_FOCUSED);

    ASSERT(ji_a11y_add_state(mgr, id, JI_A11Y_STATE_CHECKED) == 0);
    ASSERT(ji_a11y_get_node(mgr, id)->states & JI_A11Y_STATE_CHECKED);

    ASSERT(ji_a11y_remove_state(mgr, id, JI_A11Y_STATE_FOCUSED) == 0);
    ASSERT(!(ji_a11y_get_node(mgr, id)->states & JI_A11Y_STATE_FOCUSED));
    ASSERT(ji_a11y_get_node(mgr, id)->states & JI_A11Y_STATE_CHECKED);

    ASSERT(ji_a11y_set_state(mgr, id, JI_A11Y_STATE_DISABLED) == 0);
    ASSERT(ji_a11y_get_node(mgr, id)->states == JI_A11Y_STATE_DISABLED);

    ji_a11y_free(mgr);
}

TEST(test_a11y_focus)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id1 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B1");
    uint32_t id2 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B2");
    uint32_t id3 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B3");

    ji_a11y_set_tab_index(mgr, id1, 0);
    ji_a11y_set_tab_index(mgr, id2, 1);
    ji_a11y_set_tab_index(mgr, id3, 2);

    ASSERT(ji_a11y_focus(mgr, id1) == 0);
    ASSERT(ji_a11y_get_focused(mgr) == id1);
    ASSERT(ji_a11y_get_node(mgr, id1)->states & JI_A11Y_STATE_FOCUSED);

    /* Tab forward */
    ASSERT(ji_a11y_focus_next(mgr) == id2);
    ASSERT(ji_a11y_get_focused(mgr) == id2);
    ASSERT(ji_a11y_get_node(mgr, id2)->states & JI_A11Y_STATE_FOCUSED);
    ASSERT(!(ji_a11y_get_node(mgr, id1)->states & JI_A11Y_STATE_FOCUSED));

    /* Tab forward again */
    ASSERT(ji_a11y_focus_next(mgr) == id3);
    ASSERT(ji_a11y_get_focused(mgr) == id3);

    /* Wrap around */
    ASSERT(ji_a11y_focus_next(mgr) == id1);
    ASSERT(ji_a11y_get_focused(mgr) == id1);

    /* Tab backward */
    ASSERT(ji_a11y_focus_prev(mgr) == id3);
    ASSERT(ji_a11y_get_focused(mgr) == id3);

    ji_a11y_free(mgr);
}

TEST(test_a11y_focus_first_last)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id1 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B1");
    uint32_t id2 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B2");
    uint32_t id3 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B3");

    ji_a11y_set_tab_index(mgr, id1, 10);
    ji_a11y_set_tab_index(mgr, id2, 20);
    ji_a11y_set_tab_index(mgr, id3, 30);

    ASSERT(ji_a11y_focus_first(mgr) == id1);
    ASSERT(ji_a11y_get_focused(mgr) == id1);

    ASSERT(ji_a11y_focus_last(mgr) == id3);
    ASSERT(ji_a11y_get_focused(mgr) == id3);

    ji_a11y_free(mgr);
}

TEST(test_a11y_announce)
{
    JiA11yManager* mgr = ji_a11y_new();
    ASSERT(ji_a11y_get_announcement_count(mgr) == 0);

    ASSERT(ji_a11y_announce(mgr, "Button clicked") == 0);
    ASSERT(ji_a11y_get_announcement_count(mgr) == 1);
    ASSERT(strcmp(ji_a11y_get_announcement(mgr, 0), "Button clicked") == 0);

    ji_a11y_clear_announcements(mgr);
    ASSERT(ji_a11y_get_announcement_count(mgr) == 0);

    ji_a11y_free(mgr);
}

TEST(test_a11y_settings)
{
    JiA11yManager* mgr = ji_a11y_new();

    ji_a11y_set_high_contrast(mgr, true);
    ASSERT(ji_a11y_get_high_contrast(mgr) == true);

    ji_a11y_set_high_contrast(mgr, false);
    ASSERT(ji_a11y_get_high_contrast(mgr) == false);

    ji_a11y_set_screen_reader(mgr, true);
    ASSERT(ji_a11y_get_screen_reader(mgr) == true);

    ji_a11y_set_magnification(mgr, 3);
    ASSERT(ji_a11y_get_magnification(mgr) == 3);

    ji_a11y_set_magnification(mgr, 0);  /* Should be clamped to 1 */
    ASSERT(ji_a11y_get_magnification(mgr) == 1);

    ji_a11y_free(mgr);
}

TEST(test_a11y_set_properties)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_SLIDER, "Volume");

    ASSERT(ji_a11y_set_name(mgr, id, "Volume Control") == 0);
    ASSERT(strcmp(ji_a11y_get_node(mgr, id)->name, "Volume Control") == 0);

    ASSERT(ji_a11y_set_description(mgr, id, "Adjusts audio volume") == 0);
    ASSERT(strcmp(ji_a11y_get_node(mgr, id)->description, "Adjusts audio volume") == 0);

    ASSERT(ji_a11y_set_value(mgr, id, "75%") == 0);
    ASSERT(strcmp(ji_a11y_get_node(mgr, id)->value, "75%") == 0);

    ASSERT(ji_a11y_set_role(mgr, id, JI_A11Y_ROLE_PROGRESS_BAR) == 0);
    ASSERT(ji_a11y_get_node(mgr, id)->role == JI_A11Y_ROLE_PROGRESS_BAR);

    JiRect bounds = {10, 20, 300, 40};
    ASSERT(ji_a11y_set_bounds(mgr, id, bounds) == 0);
    ASSERT(ji_a11y_get_node(mgr, id)->bounds.x == 10);
    ASSERT(ji_a11y_get_node(mgr, id)->bounds.width == 300);

    ji_a11y_free(mgr);
}

TEST(test_a11y_atspi)
{
    ASSERT(ji_a11y_atspi_init() == 0);
    ASSERT(ji_a11y_atspi_is_available() == false);  /* Stub mode */
    ASSERT(ji_a11y_atspi_emit_event(1, "focus", "test") == 0);
    ji_a11y_atspi_shutdown();
}

TEST(test_a11y_focus_skip_disabled)
{
    JiA11yManager* mgr = ji_a11y_new();
    uint32_t id1 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B1");
    uint32_t id2 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B2");
    uint32_t id3 = ji_a11y_register_node(mgr, 0, JI_A11Y_ROLE_BUTTON, "B3");

    ji_a11y_set_tab_index(mgr, id1, 0);
    ji_a11y_set_tab_index(mgr, id2, 1);
    ji_a11y_set_tab_index(mgr, id3, 2);

    /* Disable id2 */
    ji_a11y_add_state(mgr, id2, JI_A11Y_STATE_DISABLED);

    /* Focus first */
    ASSERT(ji_a11y_focus_first(mgr) == id1);
    /* Tab next should skip disabled id2 */
    ASSERT(ji_a11y_focus_next(mgr) == id3);

    ji_a11y_free(mgr);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Accessibility Tests ===\n");
    RUN_TEST(test_a11y_create);
    RUN_TEST(test_a11y_register_node);
    RUN_TEST(test_a11y_unregister_node);
    RUN_TEST(test_a11y_states);
    RUN_TEST(test_a11y_focus);
    RUN_TEST(test_a11y_focus_first_last);
    RUN_TEST(test_a11y_announce);
    RUN_TEST(test_a11y_settings);
    RUN_TEST(test_a11y_set_properties);
    RUN_TEST(test_a11y_atspi);
    RUN_TEST(test_a11y_focus_skip_disabled);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
