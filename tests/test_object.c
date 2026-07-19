/**
 * JiUI - Object & Property System Unit Tests
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <string.h>

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT(expr, msg) do { \
    g_tests_run++; \
    if (expr) { g_tests_passed++; } \
    else { g_tests_failed++; fprintf(stderr, "FAIL: %s (line %d): %s\n", __func__, __LINE__, msg); } \
} while(0)

/* =========================================================================
 * Type system tests
 * ========================================================================= */

static void test_type_base(void) {
    /* Base JiObject type should be id=0 */
    const JiType* t = ji_type_get(JI_TYPE_OBJECT);
    ASSERT(t != NULL, "base type exists");
    ASSERT(strcmp(t->name, "JiObject") == 0, "base type name");
    ASSERT(t->instance_size >= sizeof(JiObject), "base type size");
    ASSERT(t->parent_id == JI_TYPE_INVALID, "base has no parent");
}

static void test_type_register(void) {
    JiTypeId widget_id = ji_type_register("JiWidget", sizeof(JiObject),
                                           JI_TYPE_OBJECT, NULL, NULL);
    ASSERT(widget_id >= 0, "register JiWidget");
    ASSERT(widget_id != JI_TYPE_INVALID, "valid id");

    const JiType* t = ji_type_get(widget_id);
    ASSERT(t != NULL, "get JiWidget type");
    ASSERT(strcmp(t->name, "JiWidget") == 0, "JiWidget name");
    ASSERT(t->parent_id == JI_TYPE_OBJECT, "JiWidget parent is JiObject");
}

static void test_type_is_a(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    ASSERT(ji_type_is_a(widget_id, JI_TYPE_OBJECT), "JiWidget is-a JiObject");
    ASSERT(ji_type_is_a(widget_id, widget_id), "JiWidget is-a JiWidget");
    ASSERT(!ji_type_is_a(JI_TYPE_OBJECT, widget_id), "JiObject is-NOT-a JiWidget");
}

static void test_type_hierarchy(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiTypeId test_btn_id = ji_type_register("TestWidget", sizeof(JiObject),
                                              widget_id, NULL, NULL);
    ASSERT(test_btn_id >= 0, "register TestWidget");
    ASSERT(ji_type_is_a(test_btn_id, widget_id), "TestWidget is-a JiWidget");
    ASSERT(ji_type_is_a(test_btn_id, JI_TYPE_OBJECT), "TestWidget is-a JiObject");
}

static void test_type_from_name(void) {
    JiTypeId id = ji_type_from_name("JiWidget");
    ASSERT(id >= 0, "found JiWidget by name");
    ASSERT(ji_type_from_name("NonExistent") == JI_TYPE_INVALID, "non-existent type");
}

/* =========================================================================
 * Property system tests
 * ========================================================================= */

static void test_property_register(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");

    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.default_value = ji_value_bool(false);

    JiPropertyId visible_prop = ji_property_register_styled(
        "IsVisible", widget_id, JI_PROP_TYPE_BOOL, meta);
    ASSERT(visible_prop >= 0, "register IsVisible");

    JiPropertyId width_prop = ji_property_register_styled(
        "Width", widget_id, JI_PROP_TYPE_DOUBLE, meta);
    ASSERT(width_prop >= 0, "register Width");

    const JiProperty* p = ji_property_get(visible_prop);
    ASSERT(p != NULL, "get property descriptor");
    ASSERT(strcmp(p->name, "IsVisible") == 0, "property name");
    ASSERT(p->kind == JI_PROP_STYLED, "property kind");
    ASSERT(p->value_type == JI_PROP_TYPE_BOOL, "property value type");
}

static void test_property_from_name(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiPropertyId prop = ji_property_from_name("IsVisible", widget_id);
    ASSERT(prop >= 0, "find IsVisible by name");

    JiPropertyId not_found = ji_property_from_name("NonExistent", widget_id);
    ASSERT(not_found == JI_PROPERTY_INVALID, "non-existent property");
}

static void test_property_attached(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.default_value = ji_value_int(0);

    JiPropertyId col_prop = ji_property_register_attached(
        "Grid.Column", widget_id, JI_PROP_TYPE_INT, meta);
    ASSERT(col_prop >= 0, "register attached Grid.Column");

    const JiProperty* p = ji_property_get(col_prop);
    ASSERT(p->kind == JI_PROP_ATTACHED, "attached property kind");
}

/* =========================================================================
 * Value helpers tests
 * ========================================================================= */

static void test_value_helpers(void) {
    JiPropertyValue vb = ji_value_bool(true);
    ASSERT(vb.type == JI_PROP_TYPE_BOOL && vb.v.bool_val == true, "value bool");

    JiPropertyValue vi = ji_value_int(42);
    ASSERT(vi.type == JI_PROP_TYPE_INT && vi.v.int_val == 42, "value int");

    JiPropertyValue vd = ji_value_double(3.14);
    ASSERT(vd.type == JI_PROP_TYPE_DOUBLE, "value double type");

    JiPropertyValue vs = ji_value_string("hello");
    ASSERT(vs.type == JI_PROP_TYPE_STRING && vs.v.string_val != NULL, "value string");
    ASSERT(strcmp(vs.v.string_val, "hello") == 0, "value string content");
    ji_value_destroy(&vs);

    JiPropertyValue vc = ji_value_color(0xFF00FF00);
    ASSERT(vc.type == JI_PROP_TYPE_COLOR && vc.v.color_val == 0xFF00FF00, "value color");
}

static void test_value_equals(void) {
    JiPropertyValue a = ji_value_int(10);
    JiPropertyValue b = ji_value_int(10);
    JiPropertyValue c = ji_value_int(20);
    ASSERT(ji_value_equals(&a, &b), "equal ints");
    ASSERT(!ji_value_equals(&a, &c), "unequal ints");

    JiPropertyValue sa = ji_value_string("abc");
    JiPropertyValue sb = ji_value_string("abc");
    JiPropertyValue sc = ji_value_string("xyz");
    ASSERT(ji_value_equals(&sa, &sb), "equal strings");
    ASSERT(!ji_value_equals(&sa, &sc), "unequal strings");
    ji_value_destroy(&sa);
    ji_value_destroy(&sb);
    ji_value_destroy(&sc);
}

static void test_value_copy(void) {
    JiPropertyValue orig = ji_value_string("test");
    JiPropertyValue copy = ji_value_copy(&orig);
    ASSERT(ji_value_equals(&orig, &copy), "copy equals original");
    ASSERT(orig.v.string_val != copy.v.string_val, "copy is deep");
    ji_value_destroy(&orig);
    ji_value_destroy(&copy);
}

/* =========================================================================
 * Object tests
 * ========================================================================= */

static void test_object_create(void) {
    JiObject* obj = ji_object_create();
    ASSERT(obj != NULL, "create object");
    ASSERT(ji_object_type_id(obj) == JI_TYPE_OBJECT, "object type id");
    ASSERT(ji_object_is_a(obj, JI_TYPE_OBJECT), "object is-a JiObject");
    ASSERT(ji_ref_object_count(obj) == 1, "initial ref count");

    ji_ref_object_release(obj);
}

static void test_object_name(void) {
    JiObject* obj = ji_object_create();
    ji_object_set_name(obj, "myObject");
    ASSERT(strcmp(ji_object_name(obj), "myObject") == 0, "object name set/get");

    ji_object_set_name(obj, "renamed");
    ASSERT(strcmp(ji_object_name(obj), "renamed") == 0, "object name rename");

    ji_ref_object_release(obj);
}

static void test_object_subclass(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* widget = ji_object_new(widget_id);
    ASSERT(widget != NULL, "create widget");
    ASSERT(ji_object_is_a(widget, widget_id), "widget is-a JiWidget");
    ASSERT(ji_object_is_a(widget, JI_TYPE_OBJECT), "widget is-a JiObject");

    ji_ref_object_release(widget);
}

/* =========================================================================
 * Property get/set on objects
 * ========================================================================= */

static void test_object_property_bool(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);
    JiPropertyId prop = ji_property_from_name("IsVisible", widget_id);

    /* Default should be false */
    bool val = ji_object_get_bool(obj, prop);
    ASSERT(val == false, "default IsVisible = false");

    /* Set to true */
    JiResultCode rc = ji_object_set_bool(obj, prop, true);
    ASSERT(rc == JI_OK, "set IsVisible = true");

    val = ji_object_get_bool(obj, prop);
    ASSERT(val == true, "IsVisible now true");

    ji_ref_object_release(obj);
}

static void test_object_property_double(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);
    JiPropertyId prop = ji_property_from_name("Width", widget_id);

    double val = ji_object_get_double(obj, prop);
    ASSERT(val == 0.0, "default Width = 0.0");

    ji_object_set_double(obj, prop, 800.0);
    val = ji_object_get_double(obj, prop);
    ASSERT(val == 800.0, "Width now 800.0");

    ji_ref_object_release(obj);
}

static void test_object_property_string(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);

    /* Register a string property */
    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.default_value = ji_value_string("default");
    JiPropertyId prop = ji_property_register_styled(
        "Text", widget_id, JI_PROP_TYPE_STRING, meta);

    const char* val = ji_object_get_string(obj, prop);
    ASSERT(val != NULL && strcmp(val, "default") == 0, "default Text");

    ji_object_set_string(obj, prop, "Hello JiUI");
    val = ji_object_get_string(obj, prop);
    ASSERT(val != NULL && strcmp(val, "Hello JiUI") == 0, "Text now Hello JiUI");

    ji_ref_object_release(obj);
}

static void test_object_property_color(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);

    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.default_value = ji_value_color(0xFFFFFFFF);
    JiPropertyId prop = ji_property_register_styled(
        "Background", widget_id, JI_PROP_TYPE_COLOR, meta);

    uint32_t val = ji_object_get_color(obj, prop);
    ASSERT(val == 0xFFFFFFFF, "default Background");

    ji_object_set_color(obj, prop, 0xFF000000);
    val = ji_object_get_color(obj, prop);
    ASSERT(val == 0xFF000000, "Background now black");

    ji_ref_object_release(obj);
}

static void test_object_property_clear(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);
    JiPropertyId prop = ji_property_from_name("IsVisible", widget_id);

    ji_object_set_bool(obj, prop, true);
    ASSERT(ji_object_is_property_set(obj, prop), "property is set");

    ji_object_clear_property(obj, prop);
    ASSERT(!ji_object_is_property_set(obj, prop), "property is cleared");

    bool val = ji_object_get_bool(obj, prop);
    ASSERT(val == false, "cleared property returns default");

    ji_ref_object_release(obj);
}

/* =========================================================================
 * Change notification tests
 * ========================================================================= */

static int g_changed_count = 0;
static JiPropertyId g_changed_prop = JI_PROPERTY_INVALID;
static bool g_changed_old_val = false;
static bool g_changed_new_val = false;

static void on_prop_changed(JiObject* obj, JiPropertyId prop_id,
                             const JiPropertyValue* old_val,
                             const JiPropertyValue* new_val,
                             void* user_data) {
    (void)obj;
    (void)user_data;
    g_changed_count++;
    g_changed_prop = prop_id;
    if (old_val) g_changed_old_val = old_val->v.bool_val;
    if (new_val) g_changed_new_val = new_val->v.bool_val;
}

static void test_object_property_changed(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");
    JiObject* obj = ji_object_new(widget_id);
    JiPropertyId prop = ji_property_from_name("IsVisible", widget_id);

    g_changed_count = 0;
    int sub_id = ji_object_add_property_changed(obj, on_prop_changed, NULL);
    ASSERT(sub_id >= 0, "subscription added");

    ji_object_set_bool(obj, prop, true);
    ASSERT(g_changed_count == 1, "change callback fired once");
    ASSERT(g_changed_prop == prop, "correct property id");
    ASSERT(g_changed_old_val == false, "old value was false");
    ASSERT(g_changed_new_val == true, "new value is true");

    /* Setting same value should NOT trigger callback */
    g_changed_count = 0;
    ji_object_set_bool(obj, prop, true);
    ASSERT(g_changed_count == 0, "no callback for same value");

    /* Remove subscription */
    ji_object_remove_property_changed(obj, sub_id);
    g_changed_count = 0;
    ji_object_set_bool(obj, prop, false);
    ASSERT(g_changed_count == 0, "no callback after unsubscribe");

    ji_ref_object_release(obj);
}

/* =========================================================================
 * Coercion test
 * ========================================================================= */

static bool coerce_clamp(JiObject* obj, JiPropertyId prop_id, JiPropertyValue* val) {
    (void)obj;
    (void)prop_id;
    if (val->type == JI_PROP_TYPE_DOUBLE) {
        if (val->v.double_val < 0.0) val->v.double_val = 0.0;
        if (val->v.double_val > 100.0) val->v.double_val = 100.0;
    }
    return true;
}

static void test_property_coercion(void) {
    JiTypeId widget_id = ji_type_from_name("JiWidget");

    JiPropertyMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.default_value = ji_value_double(0.0);
    meta.coerce = coerce_clamp;

    JiPropertyId prop = ji_property_register_direct(
        "Opacity", widget_id, JI_PROP_TYPE_DOUBLE, meta);

    JiObject* obj = ji_object_new(widget_id);

    /* Set value above max — should be clamped to 100 */
    ji_object_set_double(obj, prop, 150.0);
    double val = ji_object_get_double(obj, prop);
    ASSERT(val == 100.0, "coerced to 100.0");

    /* Set value below min — should be clamped to 0 */
    ji_object_set_double(obj, prop, -50.0);
    val = ji_object_get_double(obj, prop);
    ASSERT(val == 0.0, "coerced to 0.0");

    ji_ref_object_release(obj);
}

/* =========================================================================
 * Data context test
 * ========================================================================= */

static void test_data_context(void) {
    JiObject* obj = ji_object_create();
    ASSERT(ji_object_get_data_context(obj) == NULL, "default data context null");

    int data = 42;
    ji_object_set_data_context(obj, &data);
    int* ctx = (int*)ji_object_get_data_context(obj);
    ASSERT(ctx != NULL && *ctx == 42, "data context set/get");

    ji_ref_object_release(obj);
}

/* =========================================================================
 * Logical tree test
 * ========================================================================= */

static void test_logical_tree(void) {
    JiObject* parent = ji_object_create();
    JiObject* child  = ji_object_create();

    ASSERT(ji_object_get_parent(child) == NULL, "no parent initially");

    ji_object_set_parent(child, parent);
    ASSERT(ji_object_get_parent(child) == parent, "parent set");

    ji_ref_object_release(child);
    ji_ref_object_release(parent);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    ji_initialize();

    /* Type system */
    test_type_base();
    test_type_register();
    test_type_is_a();
    test_type_hierarchy();
    test_type_from_name();

    /* Property system */
    test_property_register();
    test_property_from_name();
    test_property_attached();

    /* Value helpers */
    test_value_helpers();
    test_value_equals();
    test_value_copy();

    /* Object basics */
    test_object_create();
    test_object_name();
    test_object_subclass();

    /* Property get/set */
    test_object_property_bool();
    test_object_property_double();
    test_object_property_string();
    test_object_property_color();
    test_object_property_clear();

    /* Change notification */
    test_object_property_changed();

    /* Coercion */
    test_property_coercion();

    /* Data context & tree */
    test_data_context();
    test_logical_tree();

    ji_shutdown();

    printf("\n=== Object/Property Test Results ===\n");
    printf("Total: %d  Passed: %d  Failed: %d\n",
           g_tests_run, g_tests_passed, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
