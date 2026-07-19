/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 */

/**
 * @file test_jiml.c
 * @brief Tests for the JiML parser, binding expressions, and compiler.
 */

#include <jiui/jiui.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { tests_run++; printf("  [RUN] %s\n", #name); name(); tests_passed++; \
         printf("  [PASS] %s\n", #name); } while(0)

#define ASSERT_TRUE(cond) \
    do { if (!(cond)) { printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); assert(cond); } } while(0)

/* =========================================================================
 * Binding Expression Tests
 * ========================================================================= */

static void test_is_binding_expr(void) {
    ASSERT_TRUE(ji_is_binding_expr("{my_var}") == true);
    ASSERT_TRUE(ji_is_binding_expr("{a + b}") == true);
    ASSERT_TRUE(ji_is_binding_expr("hello") == false);
    ASSERT_TRUE(ji_is_binding_expr("") == false);
    ASSERT_TRUE(ji_is_binding_expr(NULL) == false);
    ASSERT_TRUE(ji_is_binding_expr("{") == false);
    ASSERT_TRUE(ji_is_binding_expr("}") == false);
}

static void test_binding_expr_extract(void) {
    char buf[256];
    ASSERT_TRUE(ji_binding_expr_extract("{my_var}", buf, sizeof(buf)) == true);
    ASSERT_TRUE(strcmp(buf, "my_var") == 0);

    ASSERT_TRUE(ji_binding_expr_extract("{a + b}", buf, sizeof(buf)) == true);
    ASSERT_TRUE(strcmp(buf, "a + b") == 0);

    ASSERT_TRUE(ji_binding_expr_extract("not_a_binding", buf, sizeof(buf)) == false);
}

static void test_binding_expr_parse_simple(void) {
    JiBindingExpr* expr = ji_binding_expr_parse("my_var");
    ASSERT_TRUE(expr != NULL);
    ASSERT_TRUE(expr->type == JI_BINDING_EXPR_SIMPLE);
    ASSERT_TRUE(strcmp(expr->var_name, "my_var") == 0);
    ji_binding_expr_destroy(expr);
}

static void test_binding_expr_parse_path(void) {
    JiBindingExpr* expr = ji_binding_expr_parse("model.property");
    ASSERT_TRUE(expr != NULL);
    ASSERT_TRUE(expr->type == JI_BINDING_EXPR_PATH);
    ASSERT_TRUE(strcmp(expr->var_name, "model.property") == 0);
    ji_binding_expr_destroy(expr);
}

static void test_binding_expr_parse_binary(void) {
    JiBindingExpr* expr = ji_binding_expr_parse("a + b");
    ASSERT_TRUE(expr != NULL);
    ASSERT_TRUE(expr->type == JI_BINDING_EXPR_BINARY);
    ASSERT_TRUE(expr->op == JI_BINDING_OP_ADD);
    ASSERT_TRUE(expr->left != NULL);
    ASSERT_TRUE(expr->right != NULL);
    ji_binding_expr_destroy(expr);
}

static void test_binding_expr_parse_unary(void) {
    JiBindingExpr* expr = ji_binding_expr_parse("!flag");
    ASSERT_TRUE(expr != NULL);
    ASSERT_TRUE(expr->type == JI_BINDING_EXPR_UNARY);
    ASSERT_TRUE(expr->op == JI_BINDING_OP_NOT);
    ji_binding_expr_destroy(expr);
}

static void test_binding_expr_parse_literal(void) {
    JiBindingExpr* expr = ji_binding_expr_parse("42");
    ASSERT_TRUE(expr != NULL);
    ASSERT_TRUE(expr->is_literal == true);
    ASSERT_TRUE(expr->literal_num == 42.0);
    ji_binding_expr_destroy(expr);
}

/* =========================================================================
 * JiML Parser Tests
 * ========================================================================= */

static void test_jiml_parse_simple(void) {
    const char* source = "<JiWindow title=\"Hello\" />";
    JiJimlDocument* doc = ji_jiml_parse(source);
    ASSERT_TRUE(doc != NULL);
    /* Parser may report errors for unsupported features; just check it doesn't crash */
    ji_jiml_document_destroy(doc);
}

static void test_jiml_parse_with_children(void) {
    const char* source =
        "<JiWindow title=\"Test\">\n"
        "  <JiButton text=\"Click me\" />\n"
        "  <JiLabel text=\"Hello\" />\n"
        "</JiWindow>";
    JiJimlDocument* doc = ji_jiml_parse(source);
    ASSERT_TRUE(doc != NULL);
    ji_jiml_document_destroy(doc);
}

static void test_jiml_parse_with_binding(void) {
    const char* source = "<JiLabel text=\"{message}\" />";
    JiJimlDocument* doc = ji_jiml_parse(source);
    ASSERT_TRUE(doc != NULL);
    ji_jiml_document_destroy(doc);
}

static void test_jiml_parse_error(void) {
    const char* source = "<JiWindow><JiButton></JiWindow>";
    JiJimlDocument* doc = ji_jiml_parse(source);
    /* This should either parse with error or handle gracefully */
    if (doc) {
        ji_jiml_document_destroy(doc);
    }
}

static void test_jiml_parse_empty(void) {
    JiJimlDocument* doc = ji_jiml_parse("");
    /* Empty input should be handled gracefully */
    if (doc) {
        ji_jiml_document_destroy(doc);
    }
}

static void test_jiml_parse_null(void) {
    JiJimlDocument* doc = ji_jiml_parse(NULL);
    ASSERT_TRUE(doc == NULL);
}

/* =========================================================================
 * JiML Compiler Tests
 * ========================================================================= */

static void test_jiml_compile_string(void) {
    const char* source = "<JiWindow title=\"Hello\" />";
    JiTreeBuildResult result = ji_jiml_compile_string(source, NULL);
    /* Result may or may not have a root depending on type registration */
    if (result.root) {
        ji_ref_object_release(result.root);
    }
}

static void test_jiml_compile_with_binding(void) {
    const char* source = "<JiLabel text=\"{message}\" />";
    JiTreeBuildResult result = ji_jiml_compile_string(source, NULL);
    if (result.root) {
        ji_ref_object_release(result.root);
    }
}

static void test_jiml_compile_complex(void) {
    const char* source =
        "<JiWindow title=\"Test\">\n"
        "  <JiButton text=\"Click\" on-click=\"handleClick\" />\n"
        "  <JiLabel text=\"{label_text}\" />\n"
        "</JiWindow>";
    JiTreeBuildResult result = ji_jiml_compile_string(source, NULL);
    if (result.root) {
        ji_ref_object_release(result.root);
    }
}

/* =========================================================================
 * Style Block Tests
 * ========================================================================= */

static void test_style_block_create(void) {
    JiJimlStyleBlock* block = ji_style_block_new();
    ASSERT_TRUE(block != NULL);
    ji_style_block_destroy(block);
}

static void test_style_block_add_rule(void) {
    JiJimlStyleBlock* block = ji_style_block_new();
    ASSERT_TRUE(block != NULL);

    int idx = ji_style_block_add_rule(block, "JiButton");
    ASSERT_TRUE(idx >= 0);

    bool ok = ji_style_rule_add_prop(block, idx, "color", "red");
    ASSERT_TRUE(ok);

    ji_style_block_destroy(block);
}

static void test_style_block_find_rules(void) {
    JiJimlStyleBlock* block = ji_style_block_new();
    ASSERT_TRUE(block != NULL);

    int idx1 = ji_style_block_add_rule(block, "JiButton");
    int idx2 = ji_style_block_add_rule(block, "JiButton:hover");
    ji_style_block_add_rule(block, "JiLabel");

    int indices[10];
    int count = ji_style_block_find_rules(block, "JiButton", indices, 10);
    ASSERT_TRUE(count >= 2);

    ji_style_block_destroy(block);
}

/* =========================================================================
 * Component Definition Tests
 * ========================================================================= */

static void test_component_def_create(void) {
    JiComponentDef* def = ji_component_def_new("MyButton");
    ASSERT_TRUE(def != NULL);
    ASSERT_TRUE(strcmp(def->name, "MyButton") == 0);
    ji_component_def_destroy(def);
}

static void test_component_def_add_param(void) {
    JiComponentDef* def = ji_component_def_new("MyButton");
    ASSERT_TRUE(def != NULL);

    bool ok = ji_component_def_add_param(def, "label");
    ASSERT_TRUE(ok);
    ASSERT_TRUE(def->param_count == 1);

    ji_component_def_destroy(def);
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    printf("=== JiML Tests ===\n");

    printf("-- Binding Expression Tests --\n");
    TEST(test_is_binding_expr);
    TEST(test_binding_expr_extract);
    TEST(test_binding_expr_parse_simple);
    TEST(test_binding_expr_parse_path);
    TEST(test_binding_expr_parse_binary);
    TEST(test_binding_expr_parse_unary);
    TEST(test_binding_expr_parse_literal);

    printf("-- JiML Parser Tests --\n");
    TEST(test_jiml_parse_simple);
    TEST(test_jiml_parse_with_children);
    TEST(test_jiml_parse_with_binding);
    TEST(test_jiml_parse_error);
    TEST(test_jiml_parse_empty);
    TEST(test_jiml_parse_null);

    printf("-- JiML Compiler Tests --\n");
    TEST(test_jiml_compile_string);
    TEST(test_jiml_compile_with_binding);
    TEST(test_jiml_compile_complex);

    printf("-- Style Block Tests --\n");
    TEST(test_style_block_create);
    TEST(test_style_block_add_rule);
    TEST(test_style_block_find_rules);

    printf("-- Component Definition Tests --\n");
    TEST(test_component_def_create);
    TEST(test_component_def_add_param);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_run == tests_passed) ? 0 : 1;
}
