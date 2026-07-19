/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_lexer.h
 * @brief Lexer (tokenizer) for .ji declarative markup files.
 */

#ifndef JIUI_LEXER_H
#define JIUI_LEXER_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Token types
 * ========================================================================= */
typedef enum JiTokenType {
    JI_TOK_EOF          = 0,
    JI_TOK_IDENTIFIER   = 1,
    JI_TOK_STRING       = 2,
    JI_TOK_INTEGER      = 3,
    JI_TOK_FLOAT        = 4,
    JI_TOK_BOOLEAN      = 5,
    JI_TOK_NULL         = 6,
    JI_TOK_COLOR        = 7,   /* #RRGGBB or #AARRGGBB */
    JI_TOK_LBRACE       = 8,   /* { */
    JI_TOK_RBRACE       = 9,   /* } */
    JI_TOK_COLON        = 10,  /* : */
    JI_TOK_COMMA        = 11,  /* , */
    JI_TOK_DOT          = 12,  /* . */
    JI_TOK_HASH         = 13,  /* # (for named elements) */
    JI_TOK_LPAREN       = 14,  /* ( */
    JI_TOK_RPAREN       = 15,  /* ) */
    JI_TOK_STAR         = 16,  /* * (for grid star) */
    JI_TOK_AT           = 17,  /* @ (for references) */
    JI_TOK_ERROR        = 99
} JiTokenType;

/* =========================================================================
 * JiToken — a single token with location info
 * ========================================================================= */
typedef struct JiToken {
    JiTokenType  type;
    const char*  start;     /* pointer into source text */
    size_t       length;    /* length of the token text */
    int          line;      /* 1-based line number */
    int          column;    /* 1-based column number */

    /* Decoded values (for literals) */
    union {
        bool        bool_val;
        long        int_val;
        double      float_val;
        char*       string_val;  /* heap-allocated, decoded string (no quotes) */
        unsigned long color_val; /* decoded ARGB color */
    } value;
} JiToken;

/* =========================================================================
 * JiLexer — tokenizer state
 * ========================================================================= */
typedef struct JiLexer {
    const char* source;      /* full source text */
    size_t      source_len;  /* length of source */
    size_t      pos;         /* current byte offset */
    int         line;        /* current line (1-based) */
    int         column;      /* current column (1-based) */
    JiToken     current;     /* current (peeked) token */
    bool        has_error;   /* true if a lex error occurred */
    char        error_msg[256]; /* error description */
} JiLexer;

/* =========================================================================
 * Lexer API
 * ========================================================================= */

/** Initialize a lexer with source text. */
JI_API void ji_lexer_init(JiLexer* lexer, const char* source, size_t source_len);

/** Advance to the next token. Returns the token type. */
JI_API JiTokenType ji_lexer_next(JiLexer* lexer);

/** Peek at the current token without advancing. */
JI_API JiTokenType ji_lexer_peek(JiLexer* lexer);

/** Get the current token. */
JI_API const JiToken* ji_lexer_token(const JiLexer* lexer);

/** Get a human-readable name for a token type. */
JI_API const char* ji_token_type_name(JiTokenType type);

/** Destroy any heap-allocated data in a token. */
JI_API void ji_token_destroy(JiToken* token);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_LEXER_H */
