/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_lexer.c
 * @brief Implementation of the .ji file lexer (tokenizer).
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_lexer.h"
#include "jiui/ji_memory.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* =========================================================================
 * Helpers
 * ========================================================================= */
static char ji_peek_at(const JiLexer* l, size_t offset) {
    size_t idx = l->pos + offset;
    return idx < l->source_len ? l->source[idx] : '\0';
}

static char ji_peek(const JiLexer* l) {
    return ji_peek_at(l, 0);
}

static char ji_advance(JiLexer* l) {
    if (l->pos >= l->source_len) return '\0';
    char c = l->source[l->pos++];
    if (c == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }
    return c;
}

static void ji_skip_whitespace_and_comments(JiLexer* l) {
    while (l->pos < l->source_len) {
        char c = ji_peek(l);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ji_advance(l);
        } else if (c == '/' && ji_peek_at(l, 1) == '/') {
            /* Single-line comment */
            while (l->pos < l->source_len && ji_peek(l) != '\n') {
                ji_advance(l);
            }
        } else if (c == '/' && ji_peek_at(l, 1) == '*') {
            /* Block comment */
            ji_advance(l); ji_advance(l);
            while (l->pos < l->source_len) {
                if (ji_peek(l) == '*' && ji_peek_at(l, 1) == '/') {
                    ji_advance(l); ji_advance(l);
                    break;
                }
                ji_advance(l);
            }
        } else {
            break;
        }
    }
}

static bool is_ident_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static bool is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

/* =========================================================================
 * Read a string literal
 * ========================================================================= */
static JiToken ji_read_string(JiLexer* l, char quote) {
    JiToken tok;
    memset(&tok, 0, sizeof(tok));
    tok.type = JI_TOK_STRING;
    tok.line = l->line;
    tok.column = l->column;

    ji_advance(l); /* skip opening quote */

    /* Build the string value */
    size_t cap = 64;
    size_t len = 0;
    char* buf = (char*)ji_alloc(cap);

    while (l->pos < l->source_len && ji_peek(l) != quote) {
        char c = ji_advance(l);
        if (c == '\\' && l->pos < l->source_len) {
            char next = ji_advance(l);
            switch (next) {
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case 'r':  c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"':  c = '"';  break;
                case '\'': c = '\''; break;
                default:   c = next; break;
            }
        }
        if (len + 1 >= cap) {
            cap *= 2;
            buf = (char*)ji_realloc(buf, cap);
        }
        buf[len++] = c;
    }
    buf[len] = '\0';

    if (l->pos < l->source_len) ji_advance(l); /* skip closing quote */

    tok.value.string_val = buf;
    tok.start = NULL;
    tok.length = 0;
    return tok;
}

/* =========================================================================
 * Read a number (integer or float)
 * ========================================================================= */
static JiToken ji_read_number(JiLexer* l) {
    JiToken tok;
    memset(&tok, 0, sizeof(tok));
    tok.line = l->line;
    tok.column = l->column;

    size_t start = l->pos;
    bool is_float = false;

    while (l->pos < l->source_len && isdigit((unsigned char)ji_peek(l))) {
        ji_advance(l);
    }

    if (l->pos < l->source_len && ji_peek(l) == '.') {
        /* Check if it's a float (digit after dot) or a dot operator */
        if (l->pos + 1 < l->source_len && isdigit((unsigned char)l->source[l->pos + 1])) {
            is_float = true;
            ji_advance(l); /* consume dot */
            while (l->pos < l->source_len && isdigit((unsigned char)ji_peek(l))) {
                ji_advance(l);
            }
        }
    }

    size_t num_len = l->pos - start;
    char* num_str = (char*)ji_alloc(num_len + 1);
    memcpy(num_str, l->source + start, num_len);
    num_str[num_len] = '\0';

    if (is_float) {
        tok.type = JI_TOK_FLOAT;
        tok.value.float_val = strtod(num_str, NULL);
    } else {
        tok.type = JI_TOK_INTEGER;
        tok.value.int_val = strtol(num_str, NULL, 10);
    }
    ji_free(num_str);

    tok.start = l->source + start;
    tok.length = num_len;
    return tok;
}

/* =========================================================================
 * Read a color literal (#RRGGBB or #AARRGGBB)
 * ========================================================================= */
static JiToken ji_read_color(JiLexer* l) {
    JiToken tok;
    memset(&tok, 0, sizeof(tok));
    tok.type = JI_TOK_COLOR;
    tok.line = l->line;
    tok.column = l->column;

    ji_advance(l); /* skip # */

    size_t start = l->pos;
    while (l->pos < l->source_len && isxdigit((unsigned char)ji_peek(l))) {
        ji_advance(l);
    }

    size_t hex_len = l->pos - start;
    char* hex_str = (char*)ji_alloc(hex_len + 1);
    memcpy(hex_str, l->source + start, hex_len);
    hex_str[hex_len] = '\0';

    tok.value.color_val = strtoul(hex_str, NULL, 16);
    ji_free(hex_str);

    /* If 6 digits (RRGGBB), expand to AARRGGBB with full alpha */
    if (hex_len == 6) {
        tok.value.color_val |= 0xFF000000;
    }

    tok.start = l->source + start;
    tok.length = hex_len;
    return tok;
}

/* =========================================================================
 * Read an identifier or keyword
 * ========================================================================= */
static JiToken ji_read_identifier(JiLexer* l) {
    JiToken tok;
    memset(&tok, 0, sizeof(tok));
    tok.line = l->line;
    tok.column = l->column;

    size_t start = l->pos;
    while (l->pos < l->source_len && is_ident_char(ji_peek(l))) {
        ji_advance(l);
    }

    size_t len = l->pos - start;
    char* word = (char*)ji_alloc(len + 1);
    memcpy(word, l->source + start, len);
    word[len] = '\0';

    /* Check for keywords */
    if (strcmp(word, "true") == 0) {
        tok.type = JI_TOK_BOOLEAN;
        tok.value.bool_val = true;
    } else if (strcmp(word, "false") == 0) {
        tok.type = JI_TOK_BOOLEAN;
        tok.value.bool_val = false;
    } else if (strcmp(word, "null") == 0) {
        tok.type = JI_TOK_NULL;
    } else if (strcmp(word, "auto") == 0) {
        /* "auto" is treated as an identifier — parser handles grid auto */
        tok.type = JI_TOK_IDENTIFIER;
    } else {
        tok.type = JI_TOK_IDENTIFIER;
    }

    tok.start = l->source + start;
    tok.length = len;
    ji_free(word);
    return tok;
}

/* =========================================================================
 * API implementation
 * ========================================================================= */

void ji_lexer_init(JiLexer* lexer, const char* source, size_t source_len) {
    memset(lexer, 0, sizeof(JiLexer));
    lexer->source = source;
    lexer->source_len = source_len;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->has_error = false;

    /* Prime the first token */
    ji_lexer_next(lexer);
}

JiTokenType ji_lexer_next(JiLexer* lexer) {
    /* Destroy previous token's heap data */
    ji_token_destroy(&lexer->current);

    ji_skip_whitespace_and_comments(lexer);

    if (lexer->pos >= lexer->source_len) {
        memset(&lexer->current, 0, sizeof(JiToken));
        lexer->current.type = JI_TOK_EOF;
        lexer->current.line = lexer->line;
        lexer->current.column = lexer->column;
        return JI_TOK_EOF;
    }

    char c = ji_peek(lexer);

    JiToken tok;
    memset(&tok, 0, sizeof(JiToken));

    switch (c) {
        case '{': ji_advance(lexer); tok.type = JI_TOK_LBRACE; tok.line = lexer->line; tok.column = lexer->column; break;
        case '}': ji_advance(lexer); tok.type = JI_TOK_RBRACE; tok.line = lexer->line; tok.column = lexer->column; break;
        case ':': ji_advance(lexer); tok.type = JI_TOK_COLON; tok.line = lexer->line; tok.column = lexer->column; break;
        case ',': ji_advance(lexer); tok.type = JI_TOK_COMMA; tok.line = lexer->line; tok.column = lexer->column; break;
        case '.': ji_advance(lexer); tok.type = JI_TOK_DOT; tok.line = lexer->line; tok.column = lexer->column; break;
        case '(': ji_advance(lexer); tok.type = JI_TOK_LPAREN; tok.line = lexer->line; tok.column = lexer->column; break;
        case ')': ji_advance(lexer); tok.type = JI_TOK_RPAREN; tok.line = lexer->line; tok.column = lexer->column; break;
        case '*': ji_advance(lexer); tok.type = JI_TOK_STAR; tok.line = lexer->line; tok.column = lexer->column; break;
        case '@': ji_advance(lexer); tok.type = JI_TOK_AT; tok.line = lexer->line; tok.column = lexer->column; break;
        case '#': {
            /* Could be a color (#RRGGBB or #AARRGGBB) or a named element marker (#name)
             * A color must have exactly 6 or 8 hex digits after the #.
             * Scan ahead to count hex digits. */
            size_t hex_count = 0;
            size_t scan_pos = lexer->pos + 1;
            while (scan_pos < lexer->source_len &&
                   isxdigit((unsigned char)lexer->source[scan_pos])) {
                hex_count++;
                scan_pos++;
            }
            if (hex_count == 6 || hex_count == 8) {
                /* Verify the character after the hex digits is NOT an identifier char
                 * (to avoid matching #ABCdef as color when it's #ABCdefGhij) */
                if (scan_pos >= lexer->source_len ||
                    !is_ident_char(lexer->source[scan_pos])) {
                    tok = ji_read_color(lexer);
                } else {
                    ji_advance(lexer);
                    tok.type = JI_TOK_HASH;
                    tok.line = lexer->line;
                    tok.column = lexer->column;
                }
            } else {
                ji_advance(lexer);
                tok.type = JI_TOK_HASH;
                tok.line = lexer->line;
                tok.column = lexer->column;
            }
            break;
        }
        case '"':
        case '\'':
            tok = ji_read_string(lexer, c);
            break;
        default:
            if (isdigit((unsigned char)c)) {
                tok = ji_read_number(lexer);
            } else if (is_ident_start(c)) {
                tok = ji_read_identifier(lexer);
            } else {
                snprintf(lexer->error_msg, sizeof(lexer->error_msg),
                         "Unexpected character '%c' at line %d, column %d",
                         c, lexer->line, lexer->column);
                lexer->has_error = true;
                memset(&lexer->current, 0, sizeof(JiToken));
                lexer->current.type = JI_TOK_ERROR;
                return JI_TOK_ERROR;
            }
            break;
    }

    lexer->current = tok;
    return lexer->current.type;
}

JiTokenType ji_lexer_peek(JiLexer* lexer) {
    return lexer->current.type;
}

const JiToken* ji_lexer_token(const JiLexer* lexer) {
    return &lexer->current;
}

const char* ji_token_type_name(JiTokenType type) {
    switch (type) {
        case JI_TOK_EOF:        return "EOF";
        case JI_TOK_IDENTIFIER: return "IDENTIFIER";
        case JI_TOK_STRING:     return "STRING";
        case JI_TOK_INTEGER:    return "INTEGER";
        case JI_TOK_FLOAT:      return "FLOAT";
        case JI_TOK_BOOLEAN:    return "BOOLEAN";
        case JI_TOK_NULL:       return "NULL";
        case JI_TOK_COLOR:      return "COLOR";
        case JI_TOK_LBRACE:     return "LBRACE";
        case JI_TOK_RBRACE:     return "RBRACE";
        case JI_TOK_COLON:      return "COLON";
        case JI_TOK_COMMA:      return "COMMA";
        case JI_TOK_DOT:        return "DOT";
        case JI_TOK_HASH:       return "HASH";
        case JI_TOK_LPAREN:     return "LPAREN";
        case JI_TOK_RPAREN:     return "RPAREN";
        case JI_TOK_STAR:       return "STAR";
        case JI_TOK_AT:         return "AT";
        case JI_TOK_ERROR:      return "ERROR";
        default:               return "UNKNOWN";
    }
}

void ji_token_destroy(JiToken* token) {
    if (!token) return;
    if (token->type == JI_TOK_STRING && token->value.string_val) {
        ji_free(token->value.string_val);
        token->value.string_val = NULL;
    }
}
