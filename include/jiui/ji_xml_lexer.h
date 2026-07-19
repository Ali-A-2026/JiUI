/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_xml_lexer.h
 * @brief XML lexer (tokenizer) for .ji declarative markup files.
 *
 * Tokenizes XML source into tokens: tag delimiters, element names,
 * attribute names/values, text content, comments, CDATA sections,
 * and processing instructions.
 */

#ifndef JIUI_XML_LEXER_H
#define JIUI_XML_LEXER_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * XML Token types
 * ========================================================================= */
typedef enum JiXmlTokenType {
    JI_XML_TOK_EOF           = 0,
    JI_XML_TOK_TAG_OPEN      = 1,  /* < (start of opening tag) */
    JI_XML_TOK_TAG_CLOSE     = 2,  /* </ (start of closing tag) */
    JI_XML_TOK_TAG_END       = 3,  /* > (end of tag) */
    JI_XML_TOK_TAG_SELF_CLOSE= 4,  /* /> (self-closing tag end) */
    JI_XML_TOK_NAME          = 5,  /* element or attribute name */
    JI_XML_TOK_EQUALS        = 6,  /* = */
    JI_XML_TOK_ATTR_VALUE    = 7,  /* attribute value (quoted string) */
    JI_XML_TOK_TEXT          = 8,  /* character data between tags */
    JI_XML_TOK_COMMENT       = 9,  /* <!-- ... --> */
    JI_XML_TOK_CDATA         = 10, /* <![CDATA[ ... ]]> */
    JI_XML_TOK_PI            = 11, /* <?xml ... ?> processing instruction */
    JI_XML_TOK_ERROR         = 99
} JiXmlTokenType;

/* =========================================================================
 * JiXmlToken — a single XML token with location info
 * ========================================================================= */
typedef struct JiXmlToken {
    JiXmlTokenType type;
    const char*    start;     /* pointer into source text */
    size_t         length;    /* length of the token text */
    int            line;      /* 1-based line number */
    int            column;    /* 1-based column number */

    /* Decoded value (heap-allocated for strings, must be freed with ji_free) */
    char*          value;     /* decoded attribute value, text, comment, etc. */
} JiXmlToken;

/* =========================================================================
 * JiXmlLexer — XML tokenizer state
 * ========================================================================= */
typedef struct JiXmlLexer {
    const char*  source;      /* full source text */
    size_t       source_len;  /* length of source */
    size_t       pos;         /* current byte offset */
    int          line;        /* current line (1-based) */
    int          column;      /* current column (1-based) */
    JiXmlToken   current;     /* current (peeked) token */
    bool         has_error;   /* true if a lex error occurred */
    char         error_msg[256]; /* error description */
    int          _ji_state;   /* JiXmlLexerState: 0=CONTENT, 1=TAG */
} JiXmlLexer;

/* =========================================================================
 * Lexer API
 * ========================================================================= */

/** Initialize an XML lexer with source text. */
JI_API void ji_xml_lexer_init(JiXmlLexer* lexer, const char* source, size_t source_len);

/** Advance to the next token. Returns the token type. */
JI_API JiXmlTokenType ji_xml_lexer_next(JiXmlLexer* lexer);

/** Peek at the current token without advancing. */
JI_API JiXmlTokenType ji_xml_lexer_peek(JiXmlLexer* lexer);

/** Get the current token. */
JI_API const JiXmlToken* ji_xml_lexer_token(const JiXmlLexer* lexer);

/** Get a human-readable name for a token type. */
JI_API const char* ji_xml_token_type_name(JiXmlTokenType type);

/** Free any heap-allocated data in a token. */
JI_API void ji_xml_token_free(JiXmlToken* token);

/** Destroy a lexer and free resources. */
JI_API void ji_xml_lexer_destroy(JiXmlLexer* lexer);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_XML_LEXER_H */
