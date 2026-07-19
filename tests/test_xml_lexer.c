/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file test_xml_lexer.c
 * @brief Tests for the XML lexer for .ji files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jiui/ji_xml_lexer.h>

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
 * Helper: advance and return token type
 * ========================================================================= */
static JiXmlTokenType advance_lexer(JiXmlLexer* l) {
    return ji_xml_lexer_next(l);
}

/* =========================================================================
 * Test: simple self-closing element <Button/>
 * ========================================================================= */
static void test_simple_self_closing(void) {
    printf("  XML Lexer: simple self-closing element...\n");
    const char* src = "<Button/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* First token primed by init: TAG_OPEN */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN,
                   "First token should be TAG_OPEN");
    const JiXmlToken* tok = ji_xml_lexer_token(&lexer);
    TEST_ASSERT(tok->line == 1, "Line should be 1");

    /* NAME: Button */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME,
                   "Second token should be NAME");
    tok = ji_xml_lexer_token(&lexer);
    TEST_ASSERT_STR(tok->value, "Button", "Name should be 'Button'");

    /* TAG_SELF_CLOSE: /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE,
                   "Third token should be TAG_SELF_CLOSE");

    /* EOF */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EOF,
                   "Should be EOF");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: element with attributes
 * ========================================================================= */
static void test_element_with_attributes(void) {
    printf("  XML Lexer: element with attributes...\n");
    const char* src = "<Button Content=\"Click Me\" Width=\"100\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* < */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");

    /* Button */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Button", "Name is Button");

    /* Content */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Content");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Content", "Attr name is Content");

    /* = */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "Click Me" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Click Me", "Attr value is 'Click Me'");

    /* Width */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Width");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Width", "Attr name is Width");

    /* = */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "100" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "100", "Attr value is '100'");

    /* /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    /* EOF */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EOF, "EOF");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: opening and closing tags with text content
 * ========================================================================= */
static void test_open_close_with_text(void) {
    printf("  XML Lexer: open/close tags with text content...\n");
    const char* src = "<Window>Hello</Window>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* < */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");

    /* Window */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Window", "Name is Window");

    /* > */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END");

    /* Hello (text content) */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TEXT, "TEXT");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Hello", "Text is 'Hello'");

    /* </ */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_CLOSE, "TAG_CLOSE");

    /* Window */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window (closing)");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Window", "Closing name is Window");

    /* > */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END (closing)");

    /* EOF */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EOF, "EOF");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: nested elements
 * ========================================================================= */
static void test_nested_elements(void) {
    printf("  XML Lexer: nested elements...\n");
    const char* src = "<StackPanel><Button/></StackPanel>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* < StackPanel > */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN 1");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME StackPanel");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "StackPanel", "StackPanel name");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END 1");

    /* < Button /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN 2");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Button", "Button name");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    /* </ StackPanel > */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_CLOSE, "TAG_CLOSE");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME StackPanel (closing)");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "StackPanel", "StackPanel closing");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END 2");

    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EOF, "EOF");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: XML comment
 * ========================================================================= */
static void test_comment(void) {
    printf("  XML Lexer: comment...\n");
    const char* src = "<!-- This is a comment --><Button/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* Comment */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_COMMENT, "COMMENT");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, " This is a comment ", "Comment text");

    /* <Button/> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN after comment");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: processing instruction
 * ========================================================================= */
static void test_processing_instruction(void) {
    printf("  XML Lexer: processing instruction...\n");
    const char* src = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Window/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* PI */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_PI, "PI");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value,
                    "xml version=\"1.0\" encoding=\"UTF-8\"",
                    "PI content");

    /* <Window/> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN after PI");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: CDATA section
 * ========================================================================= */
static void test_cdata(void) {
    printf("  XML Lexer: CDATA section...\n");
    const char* src = "<Script><![CDATA[x < y && z > 0]]></Script>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <Script> */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Script");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END");

    /* CDATA */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_CDATA, "CDATA");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value,
                    "x < y && z > 0",
                    "CDATA content preserved");

    /* </Script> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_CLOSE, "TAG_CLOSE");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Script (closing)");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END (closing)");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: XML entity decoding in attribute values
 * ========================================================================= */
static void test_entity_decoding(void) {
    printf("  XML Lexer: entity decoding...\n");
    /* Test & decoding */
    const char* src1 = "<Label Text=\"A \x26" "amp; B\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src1, strlen(src1));

    /* <Label */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Label");

    /* Text= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "A & B" (after decoding &) */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value,
                    "A & B",
                    "Decoded & attr value");

    ji_xml_lexer_destroy(&lexer);

    /* Test < decoding */
    const char* src2 = "<Label Text=\"X \x26" "lt; Y\"/>";
    ji_xml_lexer_init(&lexer, src2, strlen(src2));

    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Label");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value,
                    "X < Y",
                    "Decoded < attr value");

    ji_xml_lexer_destroy(&lexer);

    /* Test > decoding */
    const char* src3 = "<Label Text=\"X \x26" "gt; Y\"/>";
    ji_xml_lexer_init(&lexer, src3, strlen(src3));

    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Label");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value,
                    "X > Y",
                    "Decoded > attr value");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: namespaced names (x:Name, xmlns:prefix)
 * ========================================================================= */
static void test_namespaced_names(void) {
    printf("  XML Lexer: namespaced names...\n");
    const char* src = "<Button x:Name=\"myBtn\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* < */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");

    /* Button */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Button", "Button name");

    /* x:Name (single NAME token with colon) */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME x:Name");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "x:Name", "Namespaced attr name");

    /* = */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "myBtn" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "myBtn", "Attr value myBtn");

    /* /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: property element syntax (OwnerType.PropertyName)
 * ========================================================================= */
static void test_property_element_name(void) {
    printf("  XML Lexer: property element name...\n");
    const char* src = "<Window.Title>My App</Window.Title>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* < */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");

    /* Window.Title */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window.Title");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Window.Title",
                    "Property element name");

    /* > */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END");

    /* My App (text) */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TEXT, "TEXT");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "My App", "Text content");

    /* </Window.Title> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_CLOSE, "TAG_CLOSE");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window.Title (closing)");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Window.Title",
                    "Property element closing name");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END (closing)");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: single-quoted attribute values
 * ========================================================================= */
static void test_single_quoted_attrs(void) {
    printf("  XML Lexer: single-quoted attribute values...\n");
    const char* src = "<Button Content='Click'/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <Button */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");

    /* Content= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Content");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* 'Click' */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Click", "Single-quoted value");

    /* /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: numeric character reference decoding
 * ========================================================================= */
static void test_numeric_entity(void) {
    printf("  XML Lexer: numeric character reference...\n");
    const char* src = "<Label Text=\"A&#66;C\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <Label */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Label");

    /* Text= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "A&#66;C" → "ABC" (&#66; = 'B') */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "ABC",
                    "Numeric entity decoded value");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: token type name function
 * ========================================================================= */
static void test_token_type_names(void) {
    printf("  XML Lexer: token type names...\n");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_EOF), "EOF", "EOF name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_TAG_OPEN), "TAG_OPEN(<)", "TAG_OPEN name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_TAG_CLOSE), "TAG_CLOSE(</)", "TAG_CLOSE name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_TAG_END), "TAG_END(>)", "TAG_END name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_TAG_SELF_CLOSE), "TAG_SELF_CLOSE(/>)", "TAG_SELF_CLOSE name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_NAME), "NAME", "NAME name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_EQUALS), "EQUALS(=)", "EQUALS name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_ATTR_VALUE), "ATTR_VALUE", "ATTR_VALUE name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_TEXT), "TEXT", "TEXT name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_COMMENT), "COMMENT", "COMMENT name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_CDATA), "CDATA", "CDATA name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_PI), "PI", "PI name");
    TEST_ASSERT_STR(ji_xml_token_type_name(JI_XML_TOK_ERROR), "ERROR", "ERROR name");
}

/* =========================================================================
 * Test: multiline XML with whitespace
 * ========================================================================= */
static void test_multiline_xml(void) {
    printf("  XML Lexer: multiline XML...\n");
    const char* src =
        "<Window\n"
        "    Title=\"My App\"\n"
        "    Width=\"800\">\n"
        "  <Button Content=\"OK\"/>\n"
        "</Window>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <Window */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window");
    TEST_ASSERT(ji_xml_lexer_token(&lexer)->line == 1, "Window on line 1");

    /* Title= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Title");
    TEST_ASSERT(ji_xml_lexer_token(&lexer)->line == 2, "Title on line 2");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "My App" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "My App", "Title value");

    /* Width= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Width");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "800" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "800", "Width value");

    /* > */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END");

    /* <Button */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN Button");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Button");

    /* Content= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Content");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "OK" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "OK", "Content value");

    /* /> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    /* </Window> */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_CLOSE, "TAG_CLOSE");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Window (closing)");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_END, "TAG_END (closing)");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: empty element with no attributes
 * ========================================================================= */
static void test_empty_element(void) {
    printf("  XML Lexer: empty element...\n");
    const char* src = "<Separator/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Separator");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "Separator", "Separator name");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_SELF_CLOSE, "TAG_SELF_CLOSE");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: markup extension in attribute value
 * ========================================================================= */
static void test_markup_extension_value(void) {
    printf("  XML Lexer: markup extension in attribute value...\n");
    const char* src = "<TextBlock Text=\"{Binding UserName}\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <TextBlock */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME TextBlock");

    /* Text= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "{Binding UserName}" — lexer returns as-is, parser interprets */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "{Binding UserName}",
                    "Markup extension preserved as-is");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Test: hex numeric entity
 * ========================================================================= */
static void test_hex_entity(void) {
    printf("  XML Lexer: hex numeric entity...\n");
    const char* src = "<Label Text=\"&#x41;&#x42;\"/>";
    JiXmlLexer lexer;
    ji_xml_lexer_init(&lexer, src, strlen(src));

    /* <Label */
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_TAG_OPEN, "TAG_OPEN");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Label");

    /* Text= */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_NAME, "NAME Text");
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_EQUALS, "EQUALS");

    /* "&#x41;&#x42;" → "AB" */
    advance_lexer(&lexer);
    TEST_ASSERT_EQ(ji_xml_lexer_peek(&lexer), JI_XML_TOK_ATTR_VALUE, "ATTR_VALUE");
    TEST_ASSERT_STR(ji_xml_lexer_token(&lexer)->value, "AB",
                    "Hex entity decoded value");

    ji_xml_lexer_destroy(&lexer);
}

/* =========================================================================
 * Main
 * ========================================================================= */
int main(void) {
    printf("=== XML Lexer Tests ===\n");

    test_simple_self_closing();
    test_element_with_attributes();
    test_open_close_with_text();
    test_nested_elements();
    test_comment();
    test_processing_instruction();
    test_cdata();
    test_entity_decoding();
    test_namespaced_names();
    test_property_element_name();
    test_single_quoted_attrs();
    test_numeric_entity();
    test_token_type_names();
    test_multiline_xml();
    test_empty_element();
    test_markup_extension_value();
    test_hex_entity();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           g_tests_passed, g_tests_run, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
