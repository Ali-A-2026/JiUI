/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_parser.c
 * @brief Tests for the .ji file lexer, AST, and parser.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/jiui.h>

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
        g_tests_passed++;                                                 \
    } else {                                                              \
        g_tests_failed++;                                                 \
        fprintf(stderr, "  FAIL [%s:%d]: %s (expected %d, got %d)\n",    \
                __FILE__, __LINE__, msg, (int)(expected), (int)(actual));  \
    }                                                                     \
} while(0)

/* =========================================================================
 * Lexer tests
 *
 * NOTE: ji_lexer_init() primes the first token automatically.
 *       Use ji_lexer_peek() / ji_lexer_token() to read the current token.
 *       Use ji_lexer_next() to advance to the NEXT token.
 * ========================================================================= */
static void test_lexer_identifiers(void) {
    printf("  Lexer: identifiers...\n");
    const char* src = "Window Button Grid";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    /* First token is already primed by init */
    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_IDENTIFIER, "First token should be identifier");
    const JiToken* tok = ji_lexer_token(&lexer);
    TEST_ASSERT(tok->length == 6, "Identifier 'Window' length should be 6");
    TEST_ASSERT(strncmp(tok->start, "Window", 6) == 0, "Identifier text should be 'Window'");

    /* Advance to second token */
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_IDENTIFIER, "Second token should be identifier");
    tok = ji_lexer_token(&lexer);
    TEST_ASSERT(tok->length == 6, "Identifier 'Button' length should be 6");

    /* Advance to third token */
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_IDENTIFIER, "Third token should be identifier");
    tok = ji_lexer_token(&lexer);
    TEST_ASSERT(tok->length == 4, "Identifier 'Grid' length should be 4");

    /* Advance to EOF */
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_EOF, "Should reach EOF");
}

static void test_lexer_string_literal(void) {
    printf("  Lexer: string literals...\n");
    const char* src = "\"Hello World\"";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_STRING, "Should tokenize string");
    const JiToken* tok = ji_lexer_token(&lexer);
    TEST_ASSERT(tok->value.string_val != NULL, "String value should not be NULL");
    TEST_ASSERT(strcmp(tok->value.string_val, "Hello World") == 0,
                 "String value should be 'Hello World'");
}

static void test_lexer_number_literals(void) {
    printf("  Lexer: number literals...\n");
    const char* src = "42 3.14 0";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_INTEGER, "42 should be integer");
    const JiToken* tok = ji_lexer_token(&lexer);
    TEST_ASSERT_EQ(tok->value.int_val, 42, "Integer value should be 42");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_FLOAT, "3.14 should be float");
    tok = ji_lexer_token(&lexer);
    TEST_ASSERT(tok->value.float_val > 3.13 && tok->value.float_val < 3.15,
                "Float value should be ~3.14");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_INTEGER, "0 should be integer");
    tok = ji_lexer_token(&lexer);
    TEST_ASSERT_EQ(tok->value.int_val, 0, "Integer value should be 0");
}

static void test_lexer_boolean_and_null(void) {
    printf("  Lexer: boolean and null...\n");
    const char* src = "true false null";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_BOOLEAN, "true should be boolean");
    TEST_ASSERT(ji_lexer_token(&lexer)->value.bool_val == true, "true value should be true");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_BOOLEAN, "false should be boolean");
    TEST_ASSERT(ji_lexer_token(&lexer)->value.bool_val == false, "false value should be false");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_NULL, "null should be null token");
}

static void test_lexer_color_literal(void) {
    printf("  Lexer: color literals...\n");
    const char* src = "#FF0000 #80FF0000";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_COLOR, "#FF0000 should be color");
    const JiToken* tok = ji_lexer_token(&lexer);
    /* #FF0000 = Opaque Red = 0xFFFF0000 */
    TEST_ASSERT(tok->value.color_val == 0xFFFF0000, "Color #FF0000 should be 0xFFFF0000");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_COLOR, "#80FF0000 should be color");
    tok = ji_lexer_token(&lexer);
    /* #80FF0000 = Semi-transparent Red = 0x80FF0000 */
    TEST_ASSERT(tok->value.color_val == 0x80FF0000, "Color #80FF0000 should be 0x80FF0000");
}

static void test_lexer_punctuation(void) {
    printf("  Lexer: punctuation...\n");
    const char* src = "{ } : , . # ( ) * @";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_LBRACE, "{ should be LBRACE");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_RBRACE, "} should be RBRACE");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_COLON, ": should be COLON");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_COMMA, ", should be COMMA");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_DOT, ". should be DOT");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_HASH, "# should be HASH");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_LPAREN, "( should be LPAREN");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_RPAREN, ") should be RPAREN");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_STAR, "* should be STAR");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_AT, "@ should be AT");
    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_EOF, "Should reach EOF");
}

static void test_lexer_peek(void) {
    printf("  Lexer: peek...\n");
    const char* src = "Window";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    JiTokenType t1 = ji_lexer_peek(&lexer);
    JiTokenType t2 = ji_lexer_peek(&lexer);
    TEST_ASSERT_EQ(t1, t2, "Peek should return same token twice");
    TEST_ASSERT_EQ(t1, JI_TOK_IDENTIFIER, "Peek should return identifier");

    /* After peek, next should advance to the next token (EOF) */
    JiTokenType t3 = ji_lexer_next(&lexer);
    TEST_ASSERT_EQ(t3, JI_TOK_EOF, "Next after peek should advance to EOF");
}

static void test_lexer_token_type_name(void) {
    printf("  Lexer: token type names...\n");
    TEST_ASSERT(strcmp(ji_token_type_name(JI_TOK_EOF), "EOF") == 0,
                 "EOF name should be 'EOF'");
    TEST_ASSERT(strcmp(ji_token_type_name(JI_TOK_IDENTIFIER), "IDENTIFIER") == 0,
                 "IDENTIFIER name should be 'IDENTIFIER'");
    TEST_ASSERT(strcmp(ji_token_type_name(JI_TOK_STRING), "STRING") == 0,
                 "STRING name should be 'STRING'");
    TEST_ASSERT(strcmp(ji_token_type_name(JI_TOK_ERROR), "ERROR") == 0,
                 "ERROR name should be 'ERROR'");
}

static void test_lexer_line_tracking(void) {
    printf("  Lexer: line tracking...\n");
    const char* src = "Window\nButton";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_token(&lexer)->line, 1, "First token on line 1");

    ji_lexer_next(&lexer);
    TEST_ASSERT_EQ(ji_lexer_token(&lexer)->line, 2, "Second token on line 2");
}

static void test_lexer_comments(void) {
    printf("  Lexer: comments...\n");
    const char* src = "// This is a comment\nWindow /* block comment */ Button";
    JiLexer lexer;
    ji_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_lexer_peek(&lexer), JI_TOK_IDENTIFIER, "Should skip line comment");
    TEST_ASSERT(strncmp(ji_lexer_token(&lexer)->start, "Window", 6) == 0,
                 "First token after line comment should be 'Window'");

    TEST_ASSERT_EQ(ji_lexer_next(&lexer), JI_TOK_IDENTIFIER, "Should skip block comment");
    TEST_ASSERT(strncmp(ji_lexer_token(&lexer)->start, "Button", 6) == 0,
                 "Token after block comment should be 'Button'");
}

/* =========================================================================
 * AST tests
 * ========================================================================= */
static void test_ast_object_node(void) {
    printf("  AST: object node...\n");
    JiAstNode* obj = ji_ast_object("Window", NULL, 1, 1);
    TEST_ASSERT(obj != NULL, "Object node should not be NULL");
    TEST_ASSERT_EQ(obj->type, JI_AST_OBJECT, "Node type should be JI_AST_OBJECT");
    TEST_ASSERT(strcmp(obj->type_name, "Window") == 0, "Type name should be 'Window'");
    TEST_ASSERT(obj->element_name == NULL, "Element name should be NULL");
    TEST_ASSERT_EQ(obj->child_count, 0, "Initial child count should be 0");

    ji_ast_destroy(obj);
}

static void test_ast_object_with_name(void) {
    printf("  AST: object with element name...\n");
    JiAstNode* obj = ji_ast_object("Button", "myBtn", 1, 1);
    TEST_ASSERT(obj != NULL, "Object node should not be NULL");
    TEST_ASSERT(strcmp(obj->element_name, "myBtn") == 0, "Element name should be 'myBtn'");

    ji_ast_destroy(obj);
}

static void test_ast_property_node(void) {
    printf("  AST: property node...\n");
    JiAstNode* val = ji_ast_string("Hello", 1, 10);
    JiAstNode* prop = ji_ast_property("Title", NULL, val, 1, 5);
    TEST_ASSERT(prop != NULL, "Property node should not be NULL");
    TEST_ASSERT_EQ(prop->type, JI_AST_PROPERTY, "Node type should be JI_AST_PROPERTY");
    TEST_ASSERT(strcmp(prop->prop_name, "Title") == 0, "Property name should be 'Title'");
    TEST_ASSERT(prop->prop_owner == NULL, "Property owner should be NULL");
    TEST_ASSERT(prop->value != NULL, "Property value should not be NULL");

    ji_ast_destroy(prop);
}

static void test_ast_attached_property(void) {
    printf("  AST: attached property node...\n");
    JiAstNode* val = ji_ast_int(1, 1, 15);
    JiAstNode* prop = ji_ast_property("Row", "Grid", val, 1, 5);
    TEST_ASSERT_EQ(prop->type, JI_AST_ATTACHED_PROP, "Node type should be JI_AST_ATTACHED_PROP");
    TEST_ASSERT(strcmp(prop->prop_name, "Row") == 0, "Property name should be 'Row'");
    TEST_ASSERT(strcmp(prop->prop_owner, "Grid") == 0, "Property owner should be 'Grid'");

    ji_ast_destroy(prop);
}

static void test_ast_value_nodes(void) {
    printf("  AST: value nodes...\n");

    JiAstNode* s = ji_ast_string("test", 1, 1);
    TEST_ASSERT_EQ(s->type, JI_AST_STRING_VAL, "String node type");
    TEST_ASSERT(strcmp(s->v.string_val, "test") == 0, "String value");
    ji_ast_destroy(s);

    JiAstNode* i = ji_ast_int(42, 1, 1);
    TEST_ASSERT_EQ(i->type, JI_AST_INT_VAL, "Int node type");
    TEST_ASSERT_EQ(i->v.int_val, 42, "Int value");
    ji_ast_destroy(i);

    JiAstNode* f = ji_ast_float(3.14, 1, 1);
    TEST_ASSERT_EQ(f->type, JI_AST_FLOAT_VAL, "Float node type");
    TEST_ASSERT(f->v.float_val > 3.13 && f->v.float_val < 3.15, "Float value");
    ji_ast_destroy(f);

    JiAstNode* b = ji_ast_bool(true, 1, 1);
    TEST_ASSERT_EQ(b->type, JI_AST_BOOL_VAL, "Bool node type");
    TEST_ASSERT(b->v.bool_val == true, "Bool value");
    ji_ast_destroy(b);

    JiAstNode* n = ji_ast_null(1, 1);
    TEST_ASSERT_EQ(n->type, JI_AST_NULL_VAL, "Null node type");
    ji_ast_destroy(n);

    JiAstNode* c = ji_ast_color(0xFFFF0000, 1, 1);
    TEST_ASSERT_EQ(c->type, JI_AST_COLOR_VAL, "Color node type");
    TEST_ASSERT_EQ(c->v.color_val, 0xFFFF0000, "Color value");
    ji_ast_destroy(c);

    JiAstNode* e = ji_ast_enum("HorizontalAlignment.Center", 1, 1);
    TEST_ASSERT_EQ(e->type, JI_AST_ENUM_VAL, "Enum node type");
    TEST_ASSERT(strcmp(e->v.string_val, "HorizontalAlignment.Center") == 0, "Enum value");
    ji_ast_destroy(e);

    JiAstNode* r = ji_ast_reference("myButton", 1, 1);
    TEST_ASSERT_EQ(r->type, JI_AST_REFERENCE, "Reference node type");
    TEST_ASSERT(strcmp(r->v.reference_name, "myButton") == 0, "Reference value");
    ji_ast_destroy(r);

    JiAstNode* res = ji_ast_resource("MyBrush", 1, 1);
    TEST_ASSERT_EQ(res->type, JI_AST_RESOURCE, "Resource node type");
    TEST_ASSERT(strcmp(res->v.resource_name, "MyBrush") == 0, "Resource value");
    ji_ast_destroy(res);
}

static void test_ast_grid_length_node(void) {
    printf("  AST: grid length nodes...\n");

    JiAstNode* auto_gl = ji_ast_grid_length(0, JI_GRID_AST_AUTO, 1, 1);
    TEST_ASSERT_EQ(auto_gl->type, JI_AST_GRID_LENGTH, "Grid length node type");
    TEST_ASSERT_EQ(auto_gl->v.grid.kind, JI_GRID_AST_AUTO, "Grid auto kind");
    ji_ast_destroy(auto_gl);

    JiAstNode* px_gl = ji_ast_grid_length(100, JI_GRID_AST_PIXEL, 1, 1);
    TEST_ASSERT_EQ(px_gl->v.grid.kind, JI_GRID_AST_PIXEL, "Grid pixel kind");
    TEST_ASSERT_EQ(px_gl->v.grid.amount, 100, "Grid pixel amount");
    ji_ast_destroy(px_gl);

    JiAstNode* star_gl = ji_ast_grid_length(1, JI_GRID_AST_STAR, 1, 1);
    TEST_ASSERT_EQ(star_gl->v.grid.kind, JI_GRID_AST_STAR, "Grid star kind");
    ji_ast_destroy(star_gl);
}

static void test_ast_binding_node(void) {
    printf("  AST: binding node...\n");
    JiAstNode* bind = ji_ast_binding("ViewModel.Name", "TwoWay", 1, 1);
    TEST_ASSERT_EQ(bind->type, JI_AST_BINDING, "Binding node type");
    TEST_ASSERT(strcmp(bind->v.binding.path, "ViewModel.Name") == 0, "Binding path");
    TEST_ASSERT(strcmp(bind->v.binding.mode, "TwoWay") == 0, "Binding mode");
    ji_ast_destroy(bind);
}

static void test_ast_add_child(void) {
    printf("  AST: add child...\n");
    JiAstNode* parent = ji_ast_object("StackPanel", NULL, 1, 1);
    JiAstNode* child1 = ji_ast_object("Button", NULL, 2, 3);
    JiAstNode* child2 = ji_ast_object("TextBlock", NULL, 3, 3);

    ji_ast_add_child(parent, child1);
    TEST_ASSERT_EQ(parent->child_count, 1, "Should have 1 child");

    ji_ast_add_child(parent, child2);
    TEST_ASSERT_EQ(parent->child_count, 2, "Should have 2 children");

    TEST_ASSERT(parent->children[0] == child1, "First child should be child1");
    TEST_ASSERT(parent->children[1] == child2, "Second child should be child2");

    ji_ast_destroy(parent);
}

/* =========================================================================
 * Parser tests
 * ========================================================================= */
static void test_parse_simple_object(void) {
    printf("  Parser: simple object...\n");
    const char* src = "Window {}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT_EQ(root->type, JI_AST_OBJECT, "Root should be object");
        TEST_ASSERT(strcmp(root->type_name, "Window") == 0, "Type name should be 'Window'");
        TEST_ASSERT_EQ(root->child_count, 0, "Should have no children");
        ji_ast_destroy(root);
    }
}

static void test_parse_object_with_properties(void) {
    printf("  Parser: object with properties...\n");
    const char* src = "Window {\n  Title: \"Hello\"\n  Width: 800\n  Height: 600\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT_EQ(root->child_count, 3, "Should have 3 property children");

        /* First property: Title */
        JiAstNode* prop1 = root->children[0];
        TEST_ASSERT_EQ(prop1->type, JI_AST_PROPERTY, "First child should be property");
        TEST_ASSERT(strcmp(prop1->prop_name, "Title") == 0, "First prop name should be 'Title'");
        TEST_ASSERT(prop1->value != NULL, "Property should have value");
        TEST_ASSERT_EQ(prop1->value->type, JI_AST_STRING_VAL, "Value should be string");
        TEST_ASSERT(strcmp(prop1->value->v.string_val, "Hello") == 0, "String value should be 'Hello'");

        /* Second property: Width */
        JiAstNode* prop2 = root->children[1];
        TEST_ASSERT(strcmp(prop2->prop_name, "Width") == 0, "Second prop name should be 'Width'");
        TEST_ASSERT_EQ(prop2->value->type, JI_AST_INT_VAL, "Width value should be int");
        TEST_ASSERT_EQ(prop2->value->v.int_val, 800, "Width value should be 800");

        /* Third property: Height */
        JiAstNode* prop3 = root->children[2];
        TEST_ASSERT(strcmp(prop3->prop_name, "Height") == 0, "Third prop name should be 'Height'");
        TEST_ASSERT_EQ(prop3->value->type, JI_AST_INT_VAL, "Height value should be int");
        TEST_ASSERT_EQ(prop3->value->v.int_val, 600, "Height value should be 600");

        ji_ast_destroy(root);
    }
}

static void test_parse_nested_objects(void) {
    printf("  Parser: nested objects...\n");
    const char* src = "Window {\n  StackPanel {\n    Button {}\n    TextBlock {}\n  }\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT_EQ(root->child_count, 1, "Window should have 1 child");
        JiAstNode* sp = root->children[0];
        TEST_ASSERT_EQ(sp->type, JI_AST_OBJECT, "Child should be object");
        TEST_ASSERT(strcmp(sp->type_name, "StackPanel") == 0, "Child should be StackPanel");
        TEST_ASSERT_EQ(sp->child_count, 2, "StackPanel should have 2 children");
        TEST_ASSERT(strcmp(sp->children[0]->type_name, "Button") == 0, "First grandchild should be Button");
        TEST_ASSERT(strcmp(sp->children[1]->type_name, "TextBlock") == 0, "Second grandchild should be TextBlock");

        ji_ast_destroy(root);
    }
}

static void test_parse_named_element(void) {
    printf("  Parser: named element (#name)...\n");
    const char* src = "Button #myBtn {\n  Content: \"Click\"\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT(strcmp(root->type_name, "Button") == 0, "Type name should be 'Button'");
        TEST_ASSERT(root->element_name != NULL, "Should have element name");
        if (root->element_name) {
            TEST_ASSERT(strcmp(root->element_name, "myBtn") == 0, "Element name should be 'myBtn'");
        }
        ji_ast_destroy(root);
    }
}

static void test_parse_attached_property(void) {
    printf("  Parser: attached property...\n");
    const char* src = "Button {\n  Grid.Row: 0\n  Grid.Column: 1\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT_EQ(root->child_count, 2, "Should have 2 attached properties");

        JiAstNode* prop1 = root->children[0];
        TEST_ASSERT_EQ(prop1->type, JI_AST_ATTACHED_PROP, "Should be attached property");
        TEST_ASSERT(strcmp(prop1->prop_name, "Row") == 0, "Property name should be 'Row'");
        TEST_ASSERT(prop1->prop_owner != NULL, "Should have owner");
        if (prop1->prop_owner) {
            TEST_ASSERT(strcmp(prop1->prop_owner, "Grid") == 0, "Owner should be 'Grid'");
        }

        ji_ast_destroy(root);
    }
}

static void test_parse_color_value(void) {
    printf("  Parser: color value...\n");
    const char* src = "Border {\n  Background: #FF0000\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT_EQ(root->child_count, 1, "Should have 1 property");
        JiAstNode* prop = root->children[0];
        TEST_ASSERT(prop->value != NULL, "Property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_COLOR_VAL, "Value should be color");
            TEST_ASSERT_EQ(prop->value->v.color_val, 0xFFFF0000, "Color should be 0xFFFF0000");
        }
        ji_ast_destroy(root);
    }
}

static void test_parse_boolean_values(void) {
    printf("  Parser: boolean values...\n");
    const char* src = "CheckBox {\n  IsChecked: true\n  IsEnabled: false\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop1 = root->children[0];
        TEST_ASSERT_EQ(prop1->value->type, JI_AST_BOOL_VAL, "First value should be bool");
        TEST_ASSERT(prop1->value->v.bool_val == true, "First bool should be true");

        JiAstNode* prop2 = root->children[1];
        TEST_ASSERT_EQ(prop2->value->type, JI_AST_BOOL_VAL, "Second value should be bool");
        TEST_ASSERT(prop2->value->v.bool_val == false, "Second bool should be false");

        ji_ast_destroy(root);
    }
}

static void test_parse_null_value(void) {
    printf("  Parser: null value...\n");
    const char* src = "Border {\n  Child: null\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop = root->children[0];
        TEST_ASSERT_EQ(prop->value->type, JI_AST_NULL_VAL, "Value should be null");
        ji_ast_destroy(root);
    }
}

static void test_parse_float_value(void) {
    printf("  Parser: float value...\n");
    const char* src = "Slider {\n  Value: 0.5\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop = root->children[0];
        TEST_ASSERT_EQ(prop->value->type, JI_AST_FLOAT_VAL, "Value should be float");
        TEST_ASSERT(prop->value->v.float_val > 0.49 && prop->value->v.float_val < 0.51,
                     "Float value should be ~0.5");
        ji_ast_destroy(root);
    }
}

static void test_parse_enum_value(void) {
    printf("  Parser: enum value...\n");
    const char* src = "Button {\n  HorizontalAlignment: HorizontalAlignment.Center\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop = root->children[0];
        TEST_ASSERT(prop->value != NULL, "Property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_ENUM_VAL, "Value should be enum");
            TEST_ASSERT(strcmp(prop->value->v.string_val, "HorizontalAlignment.Center") == 0,
                         "Enum value should be 'HorizontalAlignment.Center'");
        }
        ji_ast_destroy(root);
    }
}

static void test_parse_reference_value(void) {
    printf("  Parser: reference value (@name)...\n");
    const char* src = "Button {\n  Target: @myBtn\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop = root->children[0];
        TEST_ASSERT(prop->value != NULL, "Property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_REFERENCE, "Value should be reference");
            TEST_ASSERT(strcmp(prop->value->v.reference_name, "myBtn") == 0,
                         "Reference name should be 'myBtn'");
        }
        ji_ast_destroy(root);
    }
}

static void test_parse_grid_length_values(void) {
    printf("  Parser: grid length values...\n");

    /* Test auto grid length */
    const char* src1 = "Grid {\n  Width: auto\n}";
    JiParser parser1;
    ji_parser_init(&parser1, src1, strlen(src1));
    JiAstNode* root1 = ji_parser_parse(&parser1);
    TEST_ASSERT(root1 != NULL, "Parse auto should return root node");
    if (root1) {
        JiAstNode* prop = root1->children[0];
        TEST_ASSERT(prop->value != NULL, "Auto property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_GRID_LENGTH,
                            "Auto value should be grid length");
            TEST_ASSERT_EQ(prop->value->v.grid.kind, JI_GRID_AST_AUTO,
                            "Grid length should be auto");
        }
        ji_ast_destroy(root1);
    }

    /* Test pixel grid length */
    const char* src2 = "Grid {\n  Width: 100px\n}";
    JiParser parser2;
    ji_parser_init(&parser2, src2, strlen(src2));
    JiAstNode* root2 = ji_parser_parse(&parser2);
    TEST_ASSERT(root2 != NULL, "Parse px should return root node");
    if (root2) {
        JiAstNode* prop = root2->children[0];
        TEST_ASSERT(prop->value != NULL, "Pixel property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_GRID_LENGTH,
                            "Pixel value should be grid length");
            TEST_ASSERT_EQ(prop->value->v.grid.kind, JI_GRID_AST_PIXEL,
                            "Grid length should be pixel");
            TEST_ASSERT_EQ(prop->value->v.grid.amount, 100,
                            "Pixel amount should be 100");
        }
        ji_ast_destroy(root2);
    }

    /* Test star grid length */
    const char* src3 = "Grid {\n  Width: 1*\n}";
    JiParser parser3;
    ji_parser_init(&parser3, src3, strlen(src3));
    JiAstNode* root3 = ji_parser_parse(&parser3);
    TEST_ASSERT(root3 != NULL, "Parse star should return root node");
    if (root3) {
        JiAstNode* prop = root3->children[0];
        TEST_ASSERT(prop->value != NULL, "Star property should have value");
        if (prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_GRID_LENGTH,
                            "Star value should be grid length");
            TEST_ASSERT_EQ(prop->value->v.grid.kind, JI_GRID_AST_STAR,
                            "Grid length should be star");
        }
        ji_ast_destroy(root3);
    }
}

static void test_parse_complex_document(void) {
    printf("  Parser: complex document...\n");
    const char* src =
        "Window #mainWindow {\n"
        "  Title: \"My App\"\n"
        "  Width: 1024\n"
        "  Height: 768\n"
        "  StackPanel {\n"
        "    Button #clickBtn {\n"
        "      Content: \"Click Me\"\n"
        "      IsEnabled: true\n"
        "    }\n"
        "    TextBlock {\n"
        "      Text: \"Hello World\"\n"
        "      Foreground: #0000FF\n"
        "    }\n"
        "  }\n"
        "}";

    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        TEST_ASSERT(strcmp(root->type_name, "Window") == 0, "Root type should be Window");
        TEST_ASSERT(root->element_name != NULL, "Root should have element name");
        if (root->element_name) {
            TEST_ASSERT(strcmp(root->element_name, "mainWindow") == 0, "Element name should be 'mainWindow'");
        }

        /* Window should have 4 children: Title, Width, Height, StackPanel */
        TEST_ASSERT_EQ(root->child_count, 4, "Window should have 4 children");

        /* StackPanel is the 4th child */
        JiAstNode* sp = root->children[3];
        TEST_ASSERT_EQ(sp->type, JI_AST_OBJECT, "4th child should be object");
        TEST_ASSERT(strcmp(sp->type_name, "StackPanel") == 0, "4th child should be StackPanel");
        TEST_ASSERT_EQ(sp->child_count, 2, "StackPanel should have 2 children");

        /* Button inside StackPanel */
        JiAstNode* btn = sp->children[0];
        TEST_ASSERT(strcmp(btn->type_name, "Button") == 0, "First child should be Button");
        TEST_ASSERT(btn->element_name != NULL, "Button should have element name");
        if (btn->element_name) {
            TEST_ASSERT(strcmp(btn->element_name, "clickBtn") == 0, "Button element name should be 'clickBtn'");
        }

        ji_ast_destroy(root);
    }
}

static void test_parse_error_handling(void) {
    printf("  Parser: error handling...\n");
    /* Missing closing brace */
    const char* src = "Window {";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    /* Parser may return partial tree or NULL, but should not crash */
    if (root) {
        ji_ast_destroy(root);
    }
    TEST_ASSERT(true, "Parser should not crash on incomplete input");
}

static void test_parse_empty_input(void) {
    printf("  Parser: empty input...\n");
    const char* src = "";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    /* Empty input should return NULL or an empty node */
    if (root) {
        ji_ast_destroy(root);
    }
    TEST_ASSERT(true, "Parser should not crash on empty input");
}

static void test_parse_convenience_functions(void) {
    printf("  Parser: convenience functions...\n");

    /* ji_parse_string */
    const char* src = "Button { Content: \"OK\" }";
    JiAstNode* root = ji_parse_string(src);
    TEST_ASSERT(root != NULL, "ji_parse_string should return root node");
    if (root) {
        TEST_ASSERT(strcmp(root->type_name, "Button") == 0, "Type should be Button");
        ji_ast_destroy(root);
    }
}

static void test_parse_multiple_roots(void) {
    printf("  Parser: multiple root objects...\n");
    const char* src = "Window {}\nButton {}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    /* Parser should handle this - either as error or by returning first object */
    if (root) {
        ji_ast_destroy(root);
    }
    TEST_ASSERT(true, "Parser should not crash on multiple roots");
}

static void test_parse_resource_value(void) {
    printf("  Parser: resource value...\n");
    const char* src = "Border {\n  Background: {Resource AccentBrush}\n}";
    JiParser parser;
    ji_parser_init(&parser, src, strlen(src));

    JiAstNode* root = ji_parser_parse(&parser);
    TEST_ASSERT(root != NULL, "Parse should return root node");
    if (root) {
        JiAstNode* prop = root->children[0];
        if (prop && prop->value) {
            TEST_ASSERT_EQ(prop->value->type, JI_AST_RESOURCE, "Value should be resource");
            TEST_ASSERT(strcmp(prop->value->v.resource_name, "AccentBrush") == 0,
                         "Resource name should be 'AccentBrush'");
        }
        ji_ast_destroy(root);
    }
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    printf("=== JiUI Parser Test Suite ===\n\n");

    /* Initialize the library */
    JiResultCode rc = ji_initialize();
    if (rc != JI_OK) {
        fprintf(stderr, "FATAL: ji_initialize() failed with code %d\n", rc);
        return 1;
    }

    printf("--- Lexer Tests ---\n");
    test_lexer_identifiers();
    test_lexer_string_literal();
    test_lexer_number_literals();
    test_lexer_boolean_and_null();
    test_lexer_color_literal();
    test_lexer_punctuation();
    test_lexer_peek();
    test_lexer_token_type_name();
    test_lexer_line_tracking();
    test_lexer_comments();

    printf("\n--- AST Tests ---\n");
    test_ast_object_node();
    test_ast_object_with_name();
    test_ast_property_node();
    test_ast_attached_property();
    test_ast_value_nodes();
    test_ast_grid_length_node();
    test_ast_binding_node();
    test_ast_add_child();

    printf("\n--- Parser Tests ---\n");
    test_parse_simple_object();
    test_parse_object_with_properties();
    test_parse_nested_objects();
    test_parse_named_element();
    test_parse_attached_property();
    test_parse_color_value();
    test_parse_boolean_values();
    test_parse_null_value();
    test_parse_float_value();
    test_parse_enum_value();
    test_parse_reference_value();
    test_parse_grid_length_values();
    test_parse_complex_document();
    test_parse_error_handling();
    test_parse_empty_input();
    test_parse_convenience_functions();
    test_parse_multiple_roots();
    test_parse_resource_value();

    printf("\n=== Results ===\n");
    printf("  Total:  %d\n", g_tests_run);
    printf("  Passed: %d\n", g_tests_passed);
    printf("  Failed: %d\n", g_tests_failed);

    ji_shutdown();

    return g_tests_failed > 0 ? 1 : 0;
}
