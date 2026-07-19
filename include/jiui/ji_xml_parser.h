/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_xml_parser.h
 * @brief Recursive-descent XML parser for .ji declarative markup files.
 *
 * Parses XML token stream from ji_xml_lexer into an AST (ji_ast.h).
 * Supports:
 *   - Object elements: <Button Content="Click"/>
 *   - Property elements: <Window.Title>My App</Window.Title>
 *   - Attached properties: Grid.Column="0"
 *   - Named elements: x:Name="myButton"
 *   - Markup extensions: {Binding Path}, {StaticResource Key}
 *   - Comments, CDATA, processing instructions (skipped)
 */

#ifndef JIUI_XML_PARSER_H
#define JIUI_XML_PARSER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_xml_lexer.h"
#include "ji_ast.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * XML Namespace entry
 * ========================================================================= */
typedef struct JiXmlNamespace {
    char* prefix;   /* "x", "local", or "" for default */
    char* uri;       /* namespace URI */
} JiXmlNamespace;

/* =========================================================================
 * XML Namespace map
 * ========================================================================= */
typedef struct JiXmlNamespaceMap {
    JiXmlNamespace* entries;
    int             count;
    int             capacity;
} JiXmlNamespaceMap;

/* =========================================================================
 * Parser state
 * ========================================================================= */
typedef struct JiXmlParser {
    JiXmlLexer         lexer;
    JiXmlNamespaceMap  namespaces;
    bool               has_error;
    char               error_msg[512];
    int                error_line;
    int                error_column;
} JiXmlParser;

/* =========================================================================
 * Parser API
 * ========================================================================= */

/** Initialize an XML parser with source text. */
JI_API void ji_xml_parser_init(JiXmlParser* parser, const char* source, size_t source_len);

/** Parse the entire source and return the root AST node. Returns NULL on error. */
JI_API JiAstNode* ji_xml_parser_parse(JiXmlParser* parser);

/** Check if the parser encountered an error. */
JI_API bool ji_xml_parser_has_error(const JiXmlParser* parser);

/** Get the error message (valid only if has_error is true). */
JI_API const char* ji_xml_parser_error_message(const JiXmlParser* parser);

/** Destroy a parser and free resources. */
JI_API void ji_xml_parser_destroy(JiXmlParser* parser);

/* =========================================================================
 * Convenience: parse a .ji file
 * ========================================================================= */

/** Parse a .ji file from disk. Returns the root AST node, or NULL on error. */
JI_API JiAstNode* ji_xml_parse_file(const char* filepath);

/** Parse a .ji string in memory. Returns the root AST node, or NULL on error. */
JI_API JiAstNode* ji_xml_parse_string(const char* source);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_XML_PARSER_H */
