#include "jiui/ji_i18n.h"
#include "jiui/ji_sandbox.h"
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
 * i18n Tests
 * ========================================================================= */

TEST(test_i18n_create)
{
    JiI18nEngine* engine = ji_i18n_new();
    ASSERT(engine != NULL);
    ASSERT(engine->current_locale == -1);
    ASSERT(engine->locale_count == 0);
    ASSERT(engine->fallback_to_key == true);
    ji_i18n_free(engine);
}

TEST(test_i18n_register_locale)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en;
    ji_i18n_locale_en(&en);
    ASSERT(strcmp(en.language, "en") == 0);
    ASSERT(en.direction == JI_TEXT_DIR_LTR);

    int idx = ji_i18n_register_locale(engine, &en);
    ASSERT(idx == 0);
    ASSERT(engine->locale_count == 1);

    JiLocale ar;
    ji_i18n_locale_ar(&ar);
    ASSERT(strcmp(ar.language, "ar") == 0);
    ASSERT(ar.direction == JI_TEXT_DIR_RTL);

    idx = ji_i18n_register_locale(engine, &ar);
    ASSERT(idx == 1);
    ASSERT(engine->locale_count == 2);
    ASSERT(ji_i18n_get_locale_count(engine) == 2);

    ji_i18n_free(engine);
}

TEST(test_i18n_set_locale)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en, ar;
    ji_i18n_locale_en(&en);
    ji_i18n_locale_ar(&ar);
    ji_i18n_register_locale(engine, &en);
    ji_i18n_register_locale(engine, &ar);

    ASSERT(ji_i18n_set_locale(engine, "en") == 0);
    ASSERT(strcmp(ji_i18n_get_locale(engine), "en") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_LTR);

    ASSERT(ji_i18n_set_locale(engine, "ar") == 0);
    ASSERT(strcmp(ji_i18n_get_locale(engine), "ar") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_RTL);

    /* Invalid locale */
    ASSERT(ji_i18n_set_locale(engine, "xx") == -1);
    ASSERT(strcmp(ji_i18n_get_locale(engine), "ar") == 0);

    ji_i18n_free(engine);
}

TEST(test_i18n_translate)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en, ar;
    ji_i18n_locale_en(&en);
    ji_i18n_locale_ar(&ar);
    ji_i18n_register_locale(engine, &en);
    ji_i18n_register_locale(engine, &ar);

    ji_i18n_set_locale(engine, "en");
    ji_i18n_add_translation(engine, "greeting", "Hello");
    ji_i18n_add_translation(engine, "farewell", "Goodbye");

    ASSERT(strcmp(ji_i18n_translate(engine, "greeting"), "Hello") == 0);
    ASSERT(strcmp(ji_i18n_translate(engine, "farewell"), "Goodbye") == 0);

    /* Switch to Arabic */
    ji_i18n_set_locale(engine, "ar");
    ji_i18n_add_translation(engine, "greeting", "\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7");
    ASSERT(strcmp(ji_i18n_translate(engine, "greeting"), "\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7") == 0);

    /* Missing key falls back to key */
    ASSERT(strcmp(ji_i18n_translate(engine, "nonexistent"), "nonexistent") == 0);

    ji_i18n_free(engine);
}

TEST(test_i18n_plural)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en;
    ji_i18n_locale_en(&en);
    ji_i18n_register_locale(engine, &en);
    ji_i18n_set_locale(engine, "en");

    ji_i18n_add_plural_translation(engine, "items", "item", "items");

    /* English: 1 → singular, 0/2+ → plural */
    ASSERT(strcmp(ji_i18n_translate_plural(engine, "items", 1), "item") == 0);
    ASSERT(strcmp(ji_i18n_translate_plural(engine, "items", 0), "items") == 0);
    ASSERT(strcmp(ji_i18n_translate_plural(engine, "items", 5), "items") == 0);

    /* Plural rules */
    ASSERT(ji_i18n_plural_rule("en", 1) == JI_PLURAL_ONE);
    ASSERT(ji_i18n_plural_rule("en", 2) == JI_PLURAL_OTHER);
    ASSERT(ji_i18n_plural_rule("ar", 0) == JI_PLURAL_ZERO);
    ASSERT(ji_i18n_plural_rule("ar", 1) == JI_PLURAL_ONE);
    ASSERT(ji_i18n_plural_rule("ar", 2) == JI_PLURAL_TWO);
    ASSERT(ji_i18n_plural_rule("ar", 5) == JI_PLURAL_FEW);
    ASSERT(ji_i18n_plural_rule("ar", 15) == JI_PLURAL_MANY);
    ASSERT(ji_i18n_plural_rule("ru", 1) == JI_PLURAL_ONE);
    ASSERT(ji_i18n_plural_rule("ru", 2) == JI_PLURAL_FEW);
    ASSERT(ji_i18n_plural_rule("ru", 5) == JI_PLURAL_MANY);
    ASSERT(ji_i18n_plural_rule("ja", 1) == JI_PLURAL_OTHER);
    ASSERT(ji_i18n_plural_rule("fr", 0) == JI_PLURAL_ONE);

    ji_i18n_free(engine);
}

TEST(test_i18n_format_number)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en, de;
    ji_i18n_locale_en(&en);
    ji_i18n_locale_de(&de);
    ji_i18n_register_locale(engine, &en);
    ji_i18n_register_locale(engine, &de);

    char buf[64];

    ji_i18n_set_locale(engine, "en");
    ji_i18n_format_number(engine, 1234567.0, buf, sizeof(buf));
    ASSERT(strstr(buf, "1,234,567") != NULL);

    ji_i18n_set_locale(engine, "de");
    ji_i18n_format_number(engine, 1234567.0, buf, sizeof(buf));
    /* German uses . as thousands separator */
    ASSERT(strstr(buf, "1.234.567") != NULL);

    ji_i18n_free(engine);
}

TEST(test_i18n_format_date)
{
    JiI18nEngine* engine = ji_i18n_new();
    JiLocale en, de;
    ji_i18n_locale_en(&en);
    ji_i18n_locale_de(&de);
    ji_i18n_register_locale(engine, &en);
    ji_i18n_register_locale(engine, &de);

    char buf[64];

    ji_i18n_set_locale(engine, "en");
    ji_i18n_format_date(engine, 2026, 7, 18, buf, sizeof(buf));
    ASSERT(strstr(buf, "2026") != NULL);
    ASSERT(strstr(buf, "07") != NULL);
    ASSERT(strstr(buf, "18") != NULL);

    ji_i18n_set_locale(engine, "de");
    ji_i18n_format_date(engine, 2026, 7, 18, buf, sizeof(buf));
    /* German format: 18.07.2026 */
    ASSERT(strstr(buf, "18.07.2026") != NULL);

    ji_i18n_free(engine);
}

TEST(test_i18n_bidi_detect)
{
    ASSERT(ji_i18n_bidi_detect_direction("Hello World") == JI_TEXT_DIR_LTR);
    ASSERT(ji_i18n_bidi_detect_direction("\xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7") == JI_TEXT_DIR_RTL);
    ASSERT(ji_i18n_bidi_detect_direction("12345") == JI_TEXT_DIR_LTR);
    ASSERT(ji_i18n_bidi_detect_direction("") == JI_TEXT_DIR_LTR);
}

TEST(test_i18n_bidi_process)
{
    char out[256];
    /* LTR text with LTR base */
    int len = ji_i18n_bidi_process("Hello World", out, sizeof(out), JI_TEXT_DIR_LTR);
    ASSERT(len > 0);
    ASSERT(strcmp(out, "Hello World") == 0);

    /* Mixed text: "Hello مرحبا World" */
    const char* mixed = "Hello \xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7 World";
    len = ji_i18n_bidi_process(mixed, out, sizeof(out), JI_TEXT_DIR_LTR);
    ASSERT(len > 0);
    /* The Arabic run should be reversed in LTR base */
    ASSERT(strstr(out, "Hello") != NULL);
    ASSERT(strstr(out, "World") != NULL);
}

TEST(test_i18n_multiple_locales)
{
    JiI18nEngine* engine = ji_i18n_new();
    /* Allocate on heap — JiLocale is large due to translation array */
    JiLocale* locales = (JiLocale*)calloc(10, sizeof(JiLocale));
    ASSERT(locales != NULL);
    ji_i18n_locale_en(&locales[0]);
    ji_i18n_locale_ar(&locales[1]);
    ji_i18n_locale_fr(&locales[2]);
    ji_i18n_locale_de(&locales[3]);
    ji_i18n_locale_es(&locales[4]);
    ji_i18n_locale_fa(&locales[5]);
    ji_i18n_locale_he(&locales[6]);
    ji_i18n_locale_ja(&locales[7]);
    ji_i18n_locale_zh(&locales[8]);
    ji_i18n_locale_ru(&locales[9]);

    for (int i = 0; i < 10; i++) {
        ASSERT(ji_i18n_register_locale(engine, &locales[i]) == i);
    }
    ASSERT(ji_i18n_get_locale_count(engine) == 10);

    /* Test RTL locales */
    ASSERT(ji_i18n_set_locale(engine, "ar") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_RTL);
    ASSERT(ji_i18n_set_locale(engine, "fa") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_RTL);
    ASSERT(ji_i18n_set_locale(engine, "he") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_RTL);

    /* Test LTR locales */
    ASSERT(ji_i18n_set_locale(engine, "en") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_LTR);
    ASSERT(ji_i18n_set_locale(engine, "ja") == 0);
    ASSERT(ji_i18n_get_direction(engine) == JI_TEXT_DIR_LTR);

    ji_i18n_free(engine);
    free(locales);
}

/* =========================================================================
 * Sandbox Tests (included here for Phase 9 coverage)
 * ========================================================================= */

TEST(test_sandbox_create)
{
    JiSandboxManager* mgr = ji_sandbox_new();
    ASSERT(mgr != NULL);
    ASSERT(mgr->enforce == true);
    ASSERT(mgr->group_count == 0);
    ji_sandbox_free(mgr);
}

TEST(test_sandbox_groups)
{
    JiSandboxManager* mgr = ji_sandbox_new();
    int id1 = ji_sandbox_create_group(mgr, "main", 1024 * 1024);
    ASSERT(id1 > 0);
    ASSERT(mgr->group_count == 1);

    int id2 = ji_sandbox_create_group(mgr, "plugin1", 512 * 1024);
    ASSERT(id2 > 0);
    ASSERT(mgr->group_count == 2);

    JiSandboxGroup* g = ji_sandbox_get_group(mgr, id1);
    ASSERT(g != NULL);
    ASSERT(strcmp(g->name, "main") == 0);
    ASSERT(g->arena.size == 1024 * 1024);

    ji_sandbox_free(mgr);
}

TEST(test_sandbox_permissions)
{
    JiSandboxManager* mgr = ji_sandbox_new();
    int id = ji_sandbox_create_group(mgr, "test", 4096);
    ASSERT(id > 0);

    ASSERT(ji_sandbox_has_permission(mgr, id, JI_PERM_FILE_READ) == false);
    ASSERT(ji_sandbox_grant_permission(mgr, id, JI_PERM_FILE_READ) == 0);
    ASSERT(ji_sandbox_has_permission(mgr, id, JI_PERM_FILE_READ) == true);
    ASSERT(ji_sandbox_check_file_read(mgr, id) == true);

    ASSERT(ji_sandbox_grant_permission(mgr, id, JI_PERM_NETWORK) == 0);
    ASSERT(ji_sandbox_check_network(mgr, id) == true);

    ASSERT(ji_sandbox_revoke_permission(mgr, id, JI_PERM_FILE_READ) == 0);
    ASSERT(ji_sandbox_check_file_read(mgr, id) == false);

    ji_sandbox_free(mgr);
}

TEST(test_sandbox_arena)
{
    JiSandboxManager* mgr = ji_sandbox_new();
    int id = ji_sandbox_create_group(mgr, "test", 4096);
    ASSERT(id > 0);

    void* p1 = ji_sandbox_alloc(mgr, id, 100);
    ASSERT(p1 != NULL);
    ASSERT(ji_sandbox_arena_get_used(mgr, id) >= 100);

    void* p2 = ji_sandbox_alloc(mgr, id, 200);
    ASSERT(p2 != NULL);
    ASSERT(p2 != p1);
    ASSERT(ji_sandbox_arena_get_used(mgr, id) >= 300);

    /* Overflow */
    void* p3 = ji_sandbox_alloc(mgr, id, 4096);
    ASSERT(p3 == NULL);

    /* Reset */
    ji_sandbox_arena_reset(mgr, id);
    ASSERT(ji_sandbox_arena_get_used(mgr, id) == 0);
    ASSERT(ji_sandbox_arena_get_peak(mgr, id) > 0);

    ji_sandbox_free(mgr);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void)
{
    printf("=== i18n & Sandbox Tests ===\n");
    RUN_TEST(test_i18n_create);
    RUN_TEST(test_i18n_register_locale);
    RUN_TEST(test_i18n_set_locale);
    RUN_TEST(test_i18n_translate);
    RUN_TEST(test_i18n_plural);
    RUN_TEST(test_i18n_format_number);
    RUN_TEST(test_i18n_format_date);
    RUN_TEST(test_i18n_bidi_detect);
    RUN_TEST(test_i18n_bidi_process);
    RUN_TEST(test_i18n_multiple_locales);
    RUN_TEST(test_sandbox_create);
    RUN_TEST(test_sandbox_groups);
    RUN_TEST(test_sandbox_permissions);
    RUN_TEST(test_sandbox_arena);
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
    return g_tests_run == g_tests_passed ? 0 : 1;
}
