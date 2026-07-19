/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_xml_parser.c
 * @brief Tests for the XML parser for .ji files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/ji_xml_parser.h>

/* =========================================================================
 * Simple test framework
 * ========================================================================= */
static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) do {                                      \
    g_tests_run++;                                                       \
    if (cond) {                                                          \
        g_tests_passed++;                                                \
    } else {                                                             \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s\n", __FILE__, __LINE__, msg); \
    }                                                                    \
} while(0)

#define TEST_ASSERT_EQ(actual, expected, msg) do {                       \
    g_tests_run++;                                                        \
    if ((actual) == (expected)) {                                         \
        g_tests_passed++;                                                \
    } else {                                                              \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %d, got %d)\n",    \
                __FILE__, __LINE__, msg, (int)(expected), (int)(actual));  \
    }                                                                     \
} while(0)

#define TEST_ASSERT_STR(actual, expected, msg) do {                       \
    g_tests_run++;                                                        \
    if ((actual) && (expected) && strcmp((actual), (expected)) == 0) {   \
        g_tests_passed++;                                                \
    } else {                                                              \
        g_tests_failed++;                                                \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected \"%s\", got \"%s\")\n", \
                __FILE__, __LINE__, msg, (expected), (actual) ? (actual) : "(null)"); \
    }                                                                     \
} while(0)

/* =========================================================================
 * Test: simple self-closing element
 * ========================================================================= */
static void test_simple_self_closing(void) {
    printf("  XML Parser: simple self-closing element...\n");
    const char* src = "<Button/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->type, JI_AST_OBJECT, "Root should be OBJECT");
    TEST_ASSERT_STR(root->type_name, "Button", "Type name should be Button");
    TEST_ASSERT_EQ(root->attr_count, 0, "Should have no attributes");
    TEST_ASSERT_EQ(root->child_count, 0, "Should have no children");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: element with attributes
 * ========================================================================= */
static void test_element_with_attributes(void) {
    printf("  XML Parser: element with attributes...\n");
    const char* src = "<Button Content=\"Click Me\" Width=\"100\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "Button", "Type name should be Button");
    TEST_ASSERT_EQ(root->attr_count, 2, "Should have 2 attributes");

    /* First attribute: Content="Click Me" */
    TEST_ASSERT_STR(root->attributes[0]->attr_name, "Content", "First attr name");
    TEST_ASSERT(root->attributes[0]->attr_value != NULL, "First attr value should exist");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_STRING_VAL, "Content should be string");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.string_val, "Click Me", "Content value");

    /* Second attribute: Width="100" */
    TEST_ASSERT_STR(root->attributes[1]->attr_name, "Width", "Second attr name");
    TEST_ASSERT(root->attributes[1]->attr_value != NULL, "Second attr value should exist");
    TEST_ASSERT_EQ(root->attributes[1]->attr_value->type, JI_AST_INT_VAL, "Width should be int");
    TEST_ASSERT_EQ(root->attributes[1]->attr_value->v.int_val, 100, "Width value");

    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: element with boolean attribute
 * ========================================================================= */
static void test_boolean_attribute(void) {
    printf("  XML Parser: boolean attribute...\n");
    const char* src = "<TextBlock IsVisible=\"true\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_BOOL_VAL, "Should be bool");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->v.bool_val, true, "Should be true");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: element with color attribute
 * ========================================================================= */
static void test_color_attribute(void) {
    printf("  XML Parser: color attribute...\n");
    const char* src = "<Label Foreground=\"#FF5733\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_COLOR_VAL, "Should be color");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: element with x:Name attribute
 * ========================================================================= */
static void test_xname_attribute(void) {
    printf("  XML Parser: x:Name attribute...\n");
    const char* src = "<Button x:Name=\"myBtn\" Content=\"OK\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->element_name, "myBtn", "Element name should be myBtn");
    TEST_ASSERT_EQ(root->attr_count, 2, "Should have 2 attributes (including x:Name)");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: nested elements
 * ========================================================================= */
static void test_nested_elements(void) {
    printf("  XML Parser: nested elements...\n");
    const char* src = "<StackPanel><Button/><TextBlock/></StackPanel>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "StackPanel", "Root type name");
    TEST_ASSERT_EQ(root->child_count, 2, "Should have 2 children");
    TEST_ASSERT_STR(root->children[0]->type_name, "Button", "First child is Button");
    TEST_ASSERT_STR(root->children[1]->type_name, "TextBlock", "Second child is TextBlock");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: element with text content
 * ========================================================================= */
static void test_text_content(void) {
    printf("  XML Parser: text content...\n");
    const char* src = "<Window><Window.Title>My App</Window.Title></Window>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "Window", "Root type name");
    /* Property element — should be converted to a property child */
    TEST_ASSERT_EQ(root->child_count, 1, "Should have 1 child (property)");
    TEST_ASSERT_EQ(root->children[0]->type, JI_AST_ATTACHED_PROP, "Child should be ATTACHED_PROP (has owner)");
    TEST_ASSERT_STR(root->children[0]->prop_name, "Title", "Property name should be Title");
    TEST_ASSERT_STR(root->children[0]->prop_owner, "Window", "Property owner should be Window");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: binding markup extension
 * ========================================================================= */
static void test_binding_extension(void) {
    printf("  XML Parser: binding markup extension...\n");
    const char* src = "<TextBlock Text=\"{Binding UserName}\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_BINDING, "Should be BINDING");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.binding.path, "UserName", "Binding path");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: binding with mode
 * ========================================================================= */
static void test_binding_with_mode(void) {
    printf("  XML Parser: binding with mode...\n");
    const char* src = "<Slider Value=\"{Binding Opacity, Mode=TwoWay}\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_BINDING, "Should be BINDING");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.binding.path, "Opacity", "Binding path");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.binding.mode, "TwoWay", "Binding mode");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: static resource markup extension
 * ========================================================================= */
static void test_static_resource(void) {
    printf("  XML Parser: static resource extension...\n");
    const char* src = "<TextBlock Foreground=\"{StaticResource PrimaryColor}\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_RESOURCE, "Should be RESOURCE");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.resource_name, "PrimaryColor", "Resource key");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: enum attribute value
 * ========================================================================= */
static void test_enum_attribute(void) {
    printf("  XML Parser: enum attribute...\n");
    const char* src = "<StackPanel Orientation=\"Vertical\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_STRING_VAL, "Should be string (no dot)");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.string_val, "Vertical", "Enum value");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: dotted enum attribute value
 * ========================================================================= */
static void test_dotted_enum_attribute(void) {
    printf("  XML Parser: dotted enum attribute...\n");
    const char* src = "<TextBlock Alignment=\"HorizontalAlignment.Center\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_ENUM_VAL, "Should be ENUM");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.string_val, "HorizontalAlignment.Center", "Enum value");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: processing instruction is skipped
 * ========================================================================= */
static void test_pi_skipped(void) {
    printf("  XML Parser: processing instruction skipped...\n");
    const char* src = "<?xml version=\"1.0\"?><Window/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "Window", "Root type name");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: comment is skipped
 * ========================================================================= */
static void test_comment_skipped(void) {
    printf("  XML Parser: comment skipped...\n");
    const char* src = "<!-- comment --><Button/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "Button", "Root type name");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: deeply nested elements
 * ========================================================================= */
static void test_deep_nesting(void) {
    printf("  XML Parser: deep nesting...\n");
    const char* src = "<Window><DockPanel><StackPanel><Button/></StackPanel></DockPanel></Window>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_STR(root->type_name, "Window", "Root type name");
    TEST_ASSERT_EQ(root->child_count, 1, "Window has 1 child");
    TEST_ASSERT_STR(root->children[0]->type_name, "DockPanel", "DockPanel");
    TEST_ASSERT_EQ(root->children[0]->child_count, 1, "DockPanel has 1 child");
    TEST_ASSERT_STR(root->children[0]->children[0]->type_name, "StackPanel", "StackPanel");
    TEST_ASSERT_EQ(root->children[0]->children[0]->child_count, 1, "StackPanel has 1 child");
    TEST_ASSERT_STR(root->children[0]->children[0]->children[0]->type_name, "Button", "Button");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: multiple attributes of different types
 * ========================================================================= */
static void test_mixed_attribute_types(void) {
    printf("  XML Parser: mixed attribute types...\n");
    const char* src = "<Window Title=\"My App\" Width=\"800\" Height=\"600\" IsVisible=\"true\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 4, "Should have 4 attributes");

    /* Title: string */
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_STRING_VAL, "Title is string");
    TEST_ASSERT_STR(root->attributes[0]->attr_value->v.string_val, "My App", "Title value");

    /* Width: int */
    TEST_ASSERT_EQ(root->attributes[1]->attr_value->type, JI_AST_INT_VAL, "Width is int");
    TEST_ASSERT_EQ(root->attributes[1]->attr_value->v.int_val, 800, "Width value");

    /* Height: int */
    TEST_ASSERT_EQ(root->attributes[2]->attr_value->type, JI_AST_INT_VAL, "Height is int");
    TEST_ASSERT_EQ(root->attributes[2]->attr_value->v.int_val, 600, "Height value");

    /* IsVisible: bool */
    TEST_ASSERT_EQ(root->attributes[3]->attr_value->type, JI_AST_BOOL_VAL, "IsVisible is bool");
    TEST_ASSERT_EQ(root->attributes[3]->attr_value->v.bool_val, true, "IsVisible value");

    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: float attribute value
 * ========================================================================= */
static void test_float_attribute(void) {
    printf("  XML Parser: float attribute...\n");
    const char* src = "<Label FontSize=\"14.5\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_FLOAT_VAL, "Should be FLOAT");
    TEST_ASSERT(root->attributes[0]->attr_value->v.float_val > 14.4 && 
                root->attributes[0]->attr_value->v.float_val < 14.6, "Float value");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: null attribute value
 * ========================================================================= */
static void test_null_attribute(void) {
    printf("  XML Parser: null attribute...\n");
    const char* src = "<Label Background=\"null\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_NULL_VAL, "Should be NULL");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Test: x:Null markup extension
 * ========================================================================= */
static void test_xnull_extension(void) {
    printf("  XML Parser: x:Null extension...\n");
    const char* src = "<Label Background=\"{x:Null}\"/>";
    JiAstNode* root = ji_xml_parse_string(src);
    TEST_ASSERT(root != NULL, "Root should not be NULL");
    TEST_ASSERT_EQ(root->attr_count, 1, "Should have 1 attribute");
    TEST_ASSERT_EQ(root->attributes[0]->attr_value->type, JI_AST_NULL_VAL, "Should be NULL");
    ji_ast_destroy(root);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    printf("=== XML Parser Tests ===\n");

    test_simple_self_closing();
    test_element_with_attributes();
    test_boolean_attribute();
    test_color_attribute();
    test_xname_attribute();
    test_nested_elements();
    test_text_content();
    test_binding_extension();
    test_binding_with_mode();
    test_static_resource();
    test_enum_attribute();
    test_dotted_enum_attribute();
    test_pi_skipped();
    test_comment_skipped();
    test_deep_nesting();
    test_mixed_attribute_types();
    test_float_attribute();
    test_null_attribute();
    test_xnull_extension();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
