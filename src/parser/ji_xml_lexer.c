/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_xml_lexer.c
 * @brief Implementation of the XML lexer for .ji files.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_xml_lexer.h"
#include "jiui/ji_memory.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

/* =========================================================================
 * Helper macros
 * ========================================================================= */
#define IS_EOF(l)      ((l)->pos >= (l)->source_len)
#define PEEK_CHAR(l)   (IS_EOF(l) ? '\0' : (l)->source[(l)->pos])
#define PEEK_NEXT(l)   (((l)->pos + 1 >= (l)->source_len) ? '\0' : (l)->source[(l)->pos + 1])

/* =========================================================================
 * Lexer state: are we inside a tag or in content between tags?
 * ========================================================================= */
typedef enum JiXmlLexerState {
    JI_XML_STATE_CONTENT,   /* between tags — text is TEXT token */
    JI_XML_STATE_TAG        /* inside < ... > — names are NAME tokens */
} JiXmlLexerState;

/* =========================================================================
 * Character classification for XML names
 * ========================================================================= */

/** Check if a character is a valid XML NameStartChar. */
static bool is_name_start(char c) {
    return (c == '_') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           ((unsigned char)c >= 0xC0);
}

/** Check if a character is a valid XML NameChar. */
static bool is_name_char(char c) {
    return is_name_start(c) || (c >= '0' && c <= '9') ||
           c == '-' || c == '.' || c == ':';
}

/* =========================================================================
 * Lexer error helper
 * ========================================================================= */
static void xml_lexer_error(JiXmlLexer* l, const char* fmt, ...) {
    l->has_error = true;
    va_list args;
    va_start(args, fmt);
    vsnprintf(l->error_msg, sizeof(l->error_msg), fmt, args);
    va_end(args);
}

/* =========================================================================
 * Advance position tracking line/column
 * ========================================================================= */
static char advance(JiXmlLexer* l) {
    if (IS_EOF(l)) return '\0';
    char c = l->source[l->pos];
    l->pos++;
    if (c == '\n') {
        l->line++;
        l->column = 1;
    } else {
        l->column++;
    }
    return c;
}

/* =========================================================================
 * Skip whitespace
 * ========================================================================= */
static void skip_whitespace(JiXmlLexer* l) {
    while (!IS_EOF(l)) {
        char c = PEEK_CHAR(l);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            advance(l);
        } else {
            break;
        }
    }
}

/* =========================================================================
 * Decode XML entities in a string
 *
 * Handles: & < > " ' &#NNN; &#xHH;
 * Returns a heap-allocated decoded string.
 * ========================================================================= */
static char* decode_xml_entities(const char* src, size_t len) {
    /* Result is at most len bytes (entities are longer than their decoded form) */
    char* result = (char*)ji_alloc(len + 1);
    if (!result) return NULL;

    size_t ri = 0;
    size_t si = 0;
    while (si < len) {
        if (src[si] == '&') {
            si++; /* skip & */
            if (si >= len) { result[ri++] = '&'; break; }

            /* Numeric character reference: &#NNN; or &#xHH; */
            if (src[si] == '#') {
                si++; /* skip # */
                unsigned long code = 0;
                bool hex = false;
                if (si < len && (src[si] == 'x' || src[si] == 'X')) {
                    hex = true;
                    si++;
                }
                while (si < len && src[si] != ';') {
                    if (hex) {
                        char d = src[si];
                        if (d >= '0' && d <= '9') code = code * 16 + (d - '0');
                        else if (d >= 'a' && d <= 'f') code = code * 16 + (d - 'a' + 10);
                        else if (d >= 'A' && d <= 'F') code = code * 16 + (d - 'A' + 10);
                    } else {
                        if (src[si] >= '0' && src[si] <= '9')
                            code = code * 10 + (src[si] - '0');
                    }
                    si++;
                }
                if (si < len && src[si] == ';') si++; /* skip ; */
                /* Write as UTF-8 (simplified: only handle BMP) */
                if (code < 0x80) {
                    result[ri++] = (char)code;
                } else if (code < 0x800) {
                    result[ri++] = (char)(0xC0 | (code >> 6));
                    result[ri++] = (char)(0x80 | (code & 0x3F));
                } else {
                    result[ri++] = (char)(0xE0 | (code >> 12));
                    result[ri++] = (char)(0x80 | ((code >> 6) & 0x3F));
                    result[ri++] = (char)(0x80 | (code & 0x3F));
                }
            } else {
                /* Named entity */
                const char* ent_start = src + si;
                size_t ent_len = 0;
                while (si < len && src[si] != ';') { si++; ent_len++; }
                /* si now points to ';' or end of string */
                if (si < len && src[si] == ';') si++; /* skip ; */

                if (ent_len == 3 && strncmp(ent_start, "amp", 3) == 0)
                    result[ri++] = '&';
                else if (ent_len == 2 && strncmp(ent_start, "lt", 2) == 0)
                    result[ri++] = '<';
                else if (ent_len == 2 && strncmp(ent_start, "gt", 2) == 0)
                    result[ri++] = '>';
                else if (ent_len == 4 && strncmp(ent_start, "quot", 4) == 0)
                    result[ri++] = '"';
                else if (ent_len == 4 && strncmp(ent_start, "apos", 4) == 0)
                    result[ri++] = '\'';
                else {
                    /* Unknown entity — keep as-is including & and ; */
                    result[ri++] = '&';
                    memcpy(result + ri, ent_start, ent_len);
                    ri += ent_len;
                    result[ri++] = ';';
                }
            }
        } else {
            result[ri++] = src[si++];
        }
    }
    result[ri] = '\0';
    return result;
}

/* =========================================================================
 * Read a quoted attribute value
 * ========================================================================= */
static char* read_attr_value(JiXmlLexer* l) {
    char quote = PEEK_CHAR(l); /* ' or " */
    advance(l); /* consume opening quote */

    size_t start = l->pos;
    while (!IS_EOF(l) && PEEK_CHAR(l) != quote) {
        advance(l);
    }
    size_t val_len = l->pos - start;

    char* raw = (char*)ji_alloc(val_len + 1);
    if (!raw) return NULL;
    memcpy(raw, l->source + start, val_len);
    raw[val_len] = '\0';

    char* decoded = decode_xml_entities(raw, val_len);
    ji_free(raw);

    if (!IS_EOF(l)) advance(l); /* consume closing quote */

    return decoded;
}

/* =========================================================================
 * Read an XML name (element name, attribute name)
 * ========================================================================= */
static char* read_name(JiXmlLexer* l) {
    size_t start = l->pos;
    while (!IS_EOF(l) && is_name_char(PEEK_CHAR(l))) {
        advance(l);
    }
    size_t len = l->pos - start;
    char* name = (char*)ji_alloc(len + 1);
    if (!name) return NULL;
    memcpy(name, l->source + start, len);
    name[len] = '\0';
    return name;
}

/* =========================================================================
 * Set current token
 * ========================================================================= */
static void set_token(JiXmlLexer* l, JiXmlTokenType type,
                       const char* start, size_t length,
                       char* value, int line, int col) {
    /* Free previous token value */
    if (l->current.value) {
        ji_free(l->current.value);
        l->current.value = NULL;
    }
    l->current.type   = type;
    l->current.start  = start;
    l->current.length = length;
    l->current.line   = line;
    l->current.column = col;
    l->current.value  = value;
}

/* =========================================================================
 * Read content between tags as TEXT token
 * ========================================================================= */
static JiXmlTokenType lex_text_content(JiXmlLexer* l, int line, int col) {
    size_t text_start = l->pos;
    while (!IS_EOF(l)) {
        char c = PEEK_CHAR(l);
        if (c == '<') break;  /* start of tag */
        advance(l);
    }
    size_t text_len = l->pos - text_start;
    if (text_len == 0) {
        /* Shouldn't happen, but handle gracefully */
        set_token(l, JI_XML_TOK_EOF, NULL, 0, NULL, line, col);
        return JI_XML_TOK_EOF;
    }

    char* raw = (char*)ji_alloc(text_len + 1);
    if (raw) {
        memcpy(raw, l->source + text_start, text_len);
        raw[text_len] = '\0';
    }
    char* decoded = raw ? decode_xml_entities(raw, text_len) : NULL;
    ji_free(raw);

    set_token(l, JI_XML_TOK_TEXT,
              l->source + text_start, text_len,
              decoded, line, col);
    return JI_XML_TOK_TEXT;
}

/* =========================================================================
 * Main lexer function: produce next token
 *
 * The lexer tracks whether it is inside a tag (between < and >)
 * or in content (between > and <). This determines whether
 * identifiers are NAME tokens or TEXT tokens.
 * ========================================================================= */
JI_API JiXmlTokenType ji_xml_lexer_next(JiXmlLexer* l) {
    if (l->has_error) return JI_XML_TOK_ERROR;

    /* Recover the in_tag state from the _ji_state field */
    JiXmlLexerState state = l->_ji_state;

    if (state == JI_XML_STATE_CONTENT) {
        /* We're between tags. Skip whitespace, then check for < or text. */
        /* Don't skip whitespace here — whitespace is part of text content.
         * But leading whitespace before a tag should be skipped.
         * Actually, in XML, whitespace between tags can be significant
         * (xml:space="preserve"). For .ji files, we'll treat whitespace-only
         * text as insignificant and skip it. */

        /* Peek ahead: if next non-whitespace is <, skip the whitespace */
        size_t saved_pos = l->pos;
        int saved_line = l->line;
        int saved_col = l->column;
        bool only_whitespace = true;
        while (!IS_EOF(l)) {
            char c = PEEK_CHAR(l);
            if (c == '<') break;
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                only_whitespace = false;
                break;
            }
            advance(l);
        }

        if (IS_EOF(l) || PEEK_CHAR(l) == '<') {
            /* Whitespace-only content — skip it, fall through to tag handling */
            /* State remains CONTENT; the < handling below will switch to TAG */
        } else {
            /* There's real text content — restore position and read it */
            l->pos = saved_pos;
            l->line = saved_line;
            l->column = saved_col;
            return lex_text_content(l, l->line, l->column);
        }
    } else {
        /* Inside a tag — skip whitespace between attributes */
        skip_whitespace(l);
    }

    if (IS_EOF(l)) {
        set_token(l, JI_XML_TOK_EOF, NULL, 0, NULL, l->line, l->column);
        l->_ji_state = state;
        return JI_XML_TOK_EOF;
    }

    int line = l->line;
    int col  = l->column;
    char c   = PEEK_CHAR(l);

    /* ---- Tag start: < ---- */
    if (c == '<') {
        advance(l); /* consume < */

        /* Check for closing tag </ */
        if (!IS_EOF(l) && PEEK_CHAR(l) == '/') {
            advance(l); /* consume / */
            l->_ji_state = JI_XML_STATE_TAG;
            set_token(l, JI_XML_TOK_TAG_CLOSE,
                      l->source + l->pos - 2, 2, NULL, line, col);
            return JI_XML_TOK_TAG_CLOSE;
        }

        /* Check for comment <!-- ... --> */
        if (l->pos + 2 < l->source_len &&
            l->source[l->pos]     == '!' &&
            l->source[l->pos + 1] == '-' &&
            l->source[l->pos + 2] == '-') {
            advance(l); advance(l); advance(l); /* consume !-- */

            size_t content_start = l->pos;
            /* Scan for --> */
            while (l->pos + 2 < l->source_len) {
                if (l->source[l->pos]     == '-' &&
                    l->source[l->pos + 1] == '-' &&
                    l->source[l->pos + 2] == '>') {
                    break;
                }
                advance(l);
            }
            size_t content_len = l->pos - content_start;
            char* comment_val = (char*)ji_alloc(content_len + 1);
            if (comment_val) {
                memcpy(comment_val, l->source + content_start, content_len);
                comment_val[content_len] = '\0';
            }
            /* Skip --> */
            if (l->pos + 2 < l->source_len) {
                advance(l); advance(l); advance(l);
            }
            set_token(l, JI_XML_TOK_COMMENT,
                      l->source + content_start, content_len,
                      comment_val, line, col);
            /* Comments don't change state — we stay in CONTENT */
            l->_ji_state = state;
            return JI_XML_TOK_COMMENT;
        }

        /* Check for CDATA <![CDATA[ ... ]]> */
        if (l->pos + 8 < l->source_len &&
            l->source[l->pos]     == '!' &&
            l->source[l->pos + 1] == '[' &&
            strncmp(l->source + l->pos + 2, "CDATA[", 6) == 0) {
            advance(l); advance(l); /* consume ![ */
            l->pos += 6; /* skip CDATA[ */
            l->column += 6;

            size_t content_start = l->pos;
            /* Scan for ]]> */
            while (l->pos + 2 < l->source_len) {
                if (l->source[l->pos]     == ']' &&
                    l->source[l->pos + 1] == ']' &&
                    l->source[l->pos + 2] == '>') {
                    break;
                }
                advance(l);
            }
            size_t content_len = l->pos - content_start;
            char* cdata_val = (char*)ji_alloc(content_len + 1);
            if (cdata_val) {
                memcpy(cdata_val, l->source + content_start, content_len);
                cdata_val[content_len] = '\0';
            }
            /* Skip ]]> */
            if (l->pos + 2 < l->source_len) {
                advance(l); advance(l); advance(l);
            }
            set_token(l, JI_XML_TOK_CDATA,
                      l->source + content_start, content_len,
                      cdata_val, line, col);
            /* CDATA doesn't change state */
            l->_ji_state = state;
            return JI_XML_TOK_CDATA;
        }

        /* Check for processing instruction <? ... ?> */
        if (!IS_EOF(l) && PEEK_CHAR(l) == '?') {
            advance(l); /* consume ? */

            size_t content_start = l->pos;
            /* Scan for ?> */
            while (l->pos + 1 < l->source_len) {
                if (l->source[l->pos] == '?' && l->source[l->pos + 1] == '>') {
                    break;
                }
                advance(l);
            }
            size_t content_len = l->pos - content_start;
            char* pi_val = (char*)ji_alloc(content_len + 1);
            if (pi_val) {
                memcpy(pi_val, l->source + content_start, content_len);
                pi_val[content_len] = '\0';
            }
            /* Skip ?> */
            if (l->pos + 1 < l->source_len) {
                advance(l); advance(l);
            }
            set_token(l, JI_XML_TOK_PI,
                      l->source + content_start, content_len,
                      pi_val, line, col);
            /* PI doesn't change state */
            l->_ji_state = state;
            return JI_XML_TOK_PI;
        }

        /* Regular opening tag < — switch to TAG state */
        l->_ji_state = JI_XML_STATE_TAG;
        set_token(l, JI_XML_TOK_TAG_OPEN,
                  l->source + l->pos - 1, 1, NULL, line, col);
        return JI_XML_TOK_TAG_OPEN;
    }

    /* ---- The remaining tokens only appear inside tags ---- */

    /* ---- Self-closing tag end: /> ---- */
    if (c == '/' && PEEK_NEXT(l) == '>') {
        advance(l); advance(l); /* consume /> */
        l->_ji_state = JI_XML_STATE_CONTENT;
        set_token(l, JI_XML_TOK_TAG_SELF_CLOSE,
                  l->source + l->pos - 2, 2, NULL, line, col);
        return JI_XML_TOK_TAG_SELF_CLOSE;
    }

    /* ---- Tag end: > ---- */
    if (c == '>') {
        advance(l);
        l->_ji_state = JI_XML_STATE_CONTENT;
        set_token(l, JI_XML_TOK_TAG_END,
                  l->source + l->pos - 1, 1, NULL, line, col);
        return JI_XML_TOK_TAG_END;
    }

    /* ---- Equals sign: = ---- */
    if (c == '=') {
        advance(l);
        set_token(l, JI_XML_TOK_EQUALS,
                  l->source + l->pos - 1, 1, NULL, line, col);
        l->_ji_state = state;
        return JI_XML_TOK_EQUALS;
    }

    /* ---- Attribute value (quoted string) ---- */
    if (c == '"' || c == '\'') {
        const char* val_start = l->source + l->pos;
        char* value = read_attr_value(l);
        size_t total_len = l->source + l->pos - val_start;
        set_token(l, JI_XML_TOK_ATTR_VALUE,
                  val_start, total_len, value, line, col);
        l->_ji_state = state;
        return JI_XML_TOK_ATTR_VALUE;
    }

    /* ---- Name (element name, attribute name) ---- */
    if (is_name_start(c)) {
        const char* name_start = l->source + l->pos;
        char* name = read_name(l);
        size_t len = l->pos - (name_start - l->source);
        set_token(l, JI_XML_TOK_NAME,
                  name_start, len, name, line, col);
        l->_ji_state = state;
        return JI_XML_TOK_NAME;
    }

    /* ---- Unknown character ---- */
    xml_lexer_error(l, "Unexpected character '%c' at line %d, col %d", c, line, col);
    advance(l);
    set_token(l, JI_XML_TOK_ERROR,
              l->source + l->pos - 1, 1, NULL, line, col);
    l->_ji_state = state;
    return JI_XML_TOK_ERROR;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

JI_API void ji_xml_lexer_init(JiXmlLexer* lexer, const char* source, size_t source_len) {
    memset(lexer, 0, sizeof(JiXmlLexer));
    lexer->source     = source;
    lexer->source_len = source_len;
    lexer->pos        = 0;
    lexer->line       = 1;
    lexer->column     = 1;
    lexer->has_error  = false;
    lexer->error_msg[0] = '\0';
    lexer->current.value = NULL;
    lexer->_ji_state  = JI_XML_STATE_CONTENT;  /* start in content mode */

    /* Prime the first token */
    ji_xml_lexer_next(lexer);
}

JI_API JiXmlTokenType ji_xml_lexer_peek(JiXmlLexer* lexer) {
    return lexer->current.type;
}

JI_API const JiXmlToken* ji_xml_lexer_token(const JiXmlLexer* lexer) {
    return &lexer->current;
}

JI_API const char* ji_xml_token_type_name(JiXmlTokenType type) {
    switch (type) {
        case JI_XML_TOK_EOF:           return "EOF";
        case JI_XML_TOK_TAG_OPEN:      return "TAG_OPEN(<)";
        case JI_XML_TOK_TAG_CLOSE:     return "TAG_CLOSE(</)";
        case JI_XML_TOK_TAG_END:       return "TAG_END(>)";
        case JI_XML_TOK_TAG_SELF_CLOSE: return "TAG_SELF_CLOSE(/>)";
        case JI_XML_TOK_NAME:          return "NAME";
        case JI_XML_TOK_EQUALS:        return "EQUALS(=)";
        case JI_XML_TOK_ATTR_VALUE:    return "ATTR_VALUE";
        case JI_XML_TOK_TEXT:          return "TEXT";
        case JI_XML_TOK_COMMENT:       return "COMMENT";
        case JI_XML_TOK_CDATA:         return "CDATA";
        case JI_XML_TOK_PI:            return "PI";
        case JI_XML_TOK_ERROR:         return "ERROR";
        default:                       return "UNKNOWN";
    }
}

JI_API void ji_xml_token_free(JiXmlToken* token) {
    if (token && token->value) {
        ji_free(token->value);
        token->value = NULL;
    }
}

JI_API void ji_xml_lexer_destroy(JiXmlLexer* lexer) {
    if (lexer) {
        ji_xml_token_free(&lexer->current);
    }
}
