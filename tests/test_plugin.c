#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_plugin.h"
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
 * Dummy plugin functions for testing
 * ========================================================================= */

static void* test_widget_create(void)
{
    static int instance = 42;
    return &instance;
}

static void test_widget_destroy(void* instance)
{
    (void)instance;
}

static void* test_effect_create(void)
{
    static int effect = 99;
    return &effect;
}

static void* test_theme_create(void)
{
    static int theme = 77;
    return &theme;
}

/* =========================================================================
 * Plugin Manager Tests
 * ========================================================================= */

TEST(test_plugin_manager_create)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);
    ASSERT(mgr->plugin_count == 0);
    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_register_widget)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    /* Create a fake plugin */
    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;
    strncpy(p->manifest.name, "TestPlugin", JI_PLUGIN_NAME_MAX);

    bool result = ji_plugin_register_widget(p, "TestWidget",
                                              test_widget_create,
                                              test_widget_destroy);
    ASSERT(result == true);
    ASSERT(p->entry_count == 1);
    ASSERT(strcmp(p->entries[0].name, "TestWidget") == 0);
    ASSERT(p->entries[0].type == JI_PLUGIN_WIDGET);

    free(p);
    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_register_effect)
{
    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;

    bool result = ji_plugin_register_effect(p, "TestEffect", test_effect_create);
    ASSERT(result == true);
    ASSERT(p->entry_count == 1);
    ASSERT(p->entries[0].type == JI_PLUGIN_EFFECT);

    free(p);
}

TEST(test_plugin_register_theme)
{
    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;

    bool result = ji_plugin_register_theme(p, "TestTheme", test_theme_create);
    ASSERT(result == true);
    ASSERT(p->entry_count == 1);
    ASSERT(p->entries[0].type == JI_PLUGIN_THEME);

    free(p);
}

TEST(test_plugin_register_multiple)
{
    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;

    ji_plugin_register_widget(p, "Widget1", test_widget_create, test_widget_destroy);
    ji_plugin_register_effect(p, "Effect1", test_effect_create);
    ji_plugin_register_theme(p, "Theme1", test_theme_create);

    ASSERT(p->entry_count == 3);
    ASSERT(p->entries[0].type == JI_PLUGIN_WIDGET);
    ASSERT(p->entries[1].type == JI_PLUGIN_EFFECT);
    ASSERT(p->entries[2].type == JI_PLUGIN_THEME);

    free(p);
}

TEST(test_plugin_activate)
{
    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;

    ji_plugin_register_widget(p, "TestWidget", test_widget_create, test_widget_destroy);

    bool result = ji_plugin_activate(p);
    ASSERT(result == true);
    ASSERT(p->state == JI_PLUGIN_ACTIVE);
    ASSERT(p->entries[0].active == true);
    ASSERT(p->entries[0].instance != NULL);

    ji_plugin_deactivate(p);
    ASSERT(p->state == JI_PLUGIN_LOADED);
    ASSERT(p->entries[0].active == false);

    free(p);
}

TEST(test_plugin_find_entry)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;
    strncpy(p->manifest.name, "TestPlugin", JI_PLUGIN_NAME_MAX);
    ji_plugin_register_widget(p, "MyWidget", test_widget_create, test_widget_destroy);

    /* Add to manager manually */
    mgr->plugins[mgr->plugin_count++] = p;

    JiPluginEntry* entry = ji_plugin_find_entry(mgr, "MyWidget");
    ASSERT(entry != NULL);
    ASSERT(strcmp(entry->name, "MyWidget") == 0);

    JiPluginEntry* not_found = ji_plugin_find_entry(mgr, "NonExistent");
    ASSERT(not_found == NULL);

    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_find)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    JiPlugin* p = (JiPlugin*)calloc(1, sizeof(JiPlugin));
    p->state = JI_PLUGIN_LOADED;
    strncpy(p->manifest.name, "MyPlugin", JI_PLUGIN_NAME_MAX);

    mgr->plugins[mgr->plugin_count++] = p;

    JiPlugin* found = ji_plugin_find(mgr, "MyPlugin");
    ASSERT(found != NULL);
    ASSERT(found == p);

    JiPlugin* not_found = ji_plugin_find(mgr, "OtherPlugin");
    ASSERT(not_found == NULL);

    /* Don't free p via manager since we added it manually */
    mgr->plugin_count = 0; /* Prevent double free */
    free(p);
    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_manifest_parse)
{
    const char* json = "{\"name\": \"TestPlugin\", \"version\": \"1.0.0\", "
                       "\"description\": \"A test plugin\", \"author\": \"Test\", "
                       "\"type\": 0, \"min_api\": 1, \"max_api\": 100}";
    JiPluginManifest manifest;
    bool result = ji_plugin_manifest_parse(json, &manifest);
    ASSERT(result == true);
    ASSERT(strcmp(manifest.name, "TestPlugin") == 0);
    ASSERT(strcmp(manifest.version, "1.0.0") == 0);
    ASSERT(manifest.type == JI_PLUGIN_WIDGET);
    ASSERT(manifest.min_api_version == 1);
    ASSERT(manifest.max_api_version == 100);
}

TEST(test_plugin_manifest_to_json)
{
    JiPluginManifest manifest;
    memset(&manifest, 0, sizeof(manifest));
    strncpy(manifest.name, "TestPlugin", JI_PLUGIN_NAME_MAX);
    strncpy(manifest.version, "2.0.0", JI_PLUGIN_VERSION_MAX);
    manifest.type = JI_PLUGIN_EFFECT;

    char buf[1024];
    int len = ji_plugin_manifest_to_json(&manifest, buf, sizeof(buf));
    ASSERT(len > 0);
    ASSERT(strstr(buf, "TestPlugin") != NULL);
    ASSERT(strstr(buf, "2.0.0") != NULL);
    ASSERT(strstr(buf, "\"type\": 1") != NULL);
}

TEST(test_plugin_sandbox)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    ASSERT(ji_plugin_is_sandboxed(mgr) == false);
    ji_plugin_set_sandbox(mgr, true);
    ASSERT(ji_plugin_is_sandboxed(mgr) == true);
    ji_plugin_set_sandbox(mgr, false);
    ASSERT(ji_plugin_is_sandboxed(mgr) == false);

    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_load_nonexistent)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    JiPlugin* p = ji_plugin_load(mgr, "/nonexistent/path/to/plugin.so");
    ASSERT(p == NULL);
    ASSERT(mgr->plugin_count == 0);

    ji_plugin_manager_free(mgr);
}

TEST(test_plugin_get_error)
{
    JiPluginManager* mgr = ji_plugin_manager_new();
    ASSERT(mgr != NULL);

    /* Trigger an error */
    ji_plugin_load(mgr, "/nonexistent/path.so");
    const char* err = ji_plugin_get_error(mgr);
    ASSERT(err != NULL);
    ASSERT(strlen(err) > 0);

    ji_plugin_manager_free(mgr);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== Plugin Tests ===\n");

    RUN_TEST(test_plugin_manager_create);
    RUN_TEST(test_plugin_register_widget);
    RUN_TEST(test_plugin_register_effect);
    RUN_TEST(test_plugin_register_theme);
    RUN_TEST(test_plugin_register_multiple);
    RUN_TEST(test_plugin_activate);
    RUN_TEST(test_plugin_find_entry);
    RUN_TEST(test_plugin_find);
    RUN_TEST(test_plugin_manifest_parse);
    RUN_TEST(test_plugin_manifest_to_json);
    RUN_TEST(test_plugin_sandbox);
    RUN_TEST(test_plugin_load_nonexistent);
    RUN_TEST(test_plugin_get_error);

    printf("\n%d/%d tests passed\n", g_tests_passed, g_tests_run);
    return (g_tests_run == g_tests_passed) ? 0 : 1;
}
