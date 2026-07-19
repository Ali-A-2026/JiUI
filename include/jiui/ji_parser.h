/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_parser.h
 * @brief Recursive-descent parser for .ji declarative markup files.
 */

#ifndef JIUI_PARSER_H
#define JIUI_PARSER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_lexer.h"
#include "ji_ast.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Parser state
 * ========================================================================= */
typedef struct JiParser {
    JiLexer  lexer;
    bool     has_error;
    char     error_msg[512];
    int      error_line;
    int      error_column;
} JiParser;

/* =========================================================================
 * Parser API
 * ========================================================================= */

/** Initialize a parser with source text. */
JI_API void ji_parser_init(JiParser* parser, const char* source, size_t source_len);

/** Parse the entire source and return the root AST node. Returns NULL on error. */
JI_API JiAstNode* ji_parser_parse(JiParser* parser);

/** Check if the parser encountered an error. */
JI_API bool ji_parser_has_error(const JiParser* parser);

/** Get the error message (valid only if has_error is true). */
JI_API const char* ji_parser_error_message(const JiParser* parser);

/* =========================================================================
 * Convenience: parse a .ji file
 * ========================================================================= */

/** Parse a .ji file from disk. Returns the root AST node, or NULL on error. */
JI_API JiAstNode* ji_parse_file(const char* filepath);

/** Parse a .ji string in memory. Returns the root AST node, or NULL on error. */
JI_API JiAstNode* ji_parse_string(const char* source);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PARSER_H */
