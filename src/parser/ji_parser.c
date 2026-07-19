/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_parser.c
 * @brief Implementation of the recursive-descent parser for .ji files.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_parser.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_error.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* =========================================================================
 * Parser error helper
 * ========================================================================= */
static void ji_parser_error(JiParser* p, const char* fmt, ...) {
    p->has_error = true;
    p->error_line = p->lexer.current.line;
    p->error_column = p->lexer.current.column;
    va_list args;
    va_start(args, fmt);
    vsnprintf(p->error_msg, sizeof(p->error_msg), fmt, args);
    va_end(args);
}

/* =========================================================================
 * Token helpers
 * ========================================================================= */
static JiTokenType ji_p_peek(JiParser* p) {
    return ji_lexer_peek(&p->lexer);
}

static const JiToken* ji_p_token(JiParser* p) {
    return ji_lexer_token(&p->lexer);
}

static JiTokenType ji_p_advance(JiParser* p) {
    return ji_lexer_next(&p->lexer);
}

static bool ji_p_match(JiParser* p, JiTokenType type) {
    if (ji_p_peek(p) == type) {
        ji_p_advance(p);
        return true;
    }
    return false;
}

static bool ji_p_expect(JiParser* p, JiTokenType type, const char* msg) {
    if (ji_p_peek(p) == type) {
        ji_p_advance(p);
        return true;
    }
    ji_parser_error(p, "Expected %s, got %s (line %d, col %d)",
                    msg, ji_token_type_name(ji_p_peek(p)),
                    ji_p_token(p)->line, ji_p_token(p)->column);
    return false;
}

/* Make a null-terminated copy of the token text */
static char* ji_token_text(const JiToken* tok) {
    if (tok->start && tok->length > 0) {
        char* s = (char*)ji_alloc(tok->length + 1);
        memcpy(s, tok->start, tok->length);
        s[tok->length] = '\0';
        return s;
    }
    return NULL;
}

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
static JiAstNode* ji_p_parse_object(JiParser* p);
static JiAstNode* ji_p_parse_member(JiParser* p);
static JiAstNode* ji_p_parse_value(JiParser* p);

/* =========================================================================
 * Parse a value expression
 *
 * value = string | number | boolean | null | color | enum | reference
 *       | binding | resource | grid_length | '(' expr ')'
 * ========================================================================= */
static JiAstNode* ji_p_parse_value(JiParser* p) {
    const JiToken* tok = ji_p_token(p);
    int line = tok->line;
    int col = tok->column;

    switch (ji_p_peek(p)) {
        case JI_TOK_STRING: {
            char* val = tok->value.string_val ? strdup(tok->value.string_val) : NULL;
            ji_p_advance(p);
            return ji_ast_string(val, line, col);
        }
        case JI_TOK_INTEGER: {
            long val = tok->value.int_val;
            ji_p_advance(p);
            /* Check for "px" suffix (grid pixel) or "*" suffix (grid star) */
            if (ji_p_peek(p) == JI_TOK_IDENTIFIER) {
                const JiToken* next = ji_p_token(p);
                if (next->length == 2 && next->start &&
                    strncmp(next->start, "px", 2) == 0) {
                    ji_p_advance(p);
                    return ji_ast_grid_length((double)val, JI_GRID_AST_PIXEL, line, col);
                }
            }
            if (ji_p_peek(p) == JI_TOK_STAR) {
                ji_p_advance(p);
                return ji_ast_grid_length((double)val, JI_GRID_AST_STAR, line, col);
            }
            return ji_ast_int(val, line, col);
        }
        case JI_TOK_FLOAT: {
            double val = tok->value.float_val;
            ji_p_advance(p);
            if (ji_p_peek(p) == JI_TOK_STAR) {
                ji_p_advance(p);
                return ji_ast_grid_length(val, JI_GRID_AST_STAR, line, col);
            }
            return ji_ast_float(val, line, col);
        }
        case JI_TOK_BOOLEAN: {
            bool val = tok->value.bool_val;
            ji_p_advance(p);
            return ji_ast_bool(val, line, col);
        }
        case JI_TOK_NULL: {
            ji_p_advance(p);
            return ji_ast_null(line, col);
        }
        case JI_TOK_COLOR: {
            unsigned long val = tok->value.color_val;
            ji_p_advance(p);
            return ji_ast_color(val, line, col);
        }
        case JI_TOK_AT: {
            ji_p_advance(p); /* consume @ */
            if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                ji_parser_error(p, "Expected identifier after '@'");
                return NULL;
            }
            char* name = ji_token_text(ji_p_token(p));
            ji_p_advance(p); /* consume identifier */
            JiAstNode* n = ji_ast_reference(name, line, col);
            ji_free(name);
            return n;
        }
        case JI_TOK_IDENTIFIER: {
            char* name = ji_token_text(tok);
            ji_p_advance(p);

            /* Check for "auto" (grid) */
            if (name && strcmp(name, "auto") == 0) {
                JiAstNode* n = ji_ast_grid_length(0.0, JI_GRID_AST_AUTO, line, col);
                ji_free(name);
                return n;
            }

            /* Check for enum: Identifier.Identifier */
            if (ji_p_peek(p) == JI_TOK_DOT) {
                ji_p_advance(p); /* consume dot */
                if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                    ji_parser_error(p, "Expected identifier after '.'");
                    ji_free(name);
                    return NULL;
                }
                const JiToken* val_tok = ji_p_token(p);
                char* val_name = ji_token_text(val_tok);
                ji_p_advance(p);

                /* Build "Type.Value" string */
                size_t len = strlen(name) + 1 + strlen(val_name ? val_name : "") + 1;
                char* enum_str = (char*)ji_alloc(len);
                snprintf(enum_str, len, "%s.%s", name, val_name ? val_name : "");

                JiAstNode* n = ji_ast_enum(enum_str, line, col);
                ji_free(name);
                ji_free(val_name);
                ji_free(enum_str);
                return n;
            }

            /* Check for binding: {bind ...} — handled at object level */
            /* Otherwise it's a bare identifier used as a value (e.g. enum without dot) */
            JiAstNode* n = ji_ast_enum(name, line, col);
            ji_free(name);
            return n;
        }
        case JI_TOK_LBRACE: {
            /* Could be {bind ...} or {Resource ...} */
            ji_p_advance(p); /* consume { */

            if (ji_p_peek(p) == JI_TOK_IDENTIFIER) {
                const JiToken* kw = ji_p_token(p);
                char* kw_text = ji_token_text(kw);
                ji_p_advance(p);

                if (kw_text && strcmp(kw_text, "bind") == 0) {
                    /* Binding: {bind Path [, Mode=...] } */
                    if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                        ji_parser_error(p, "Expected binding path after 'bind'");
                        ji_free(kw_text);
                        ji_p_match(p, JI_TOK_RBRACE);
                        return NULL;
                    }
                    char* path = ji_token_text(ji_p_token(p));
                    ji_p_advance(p); /* consume path identifier */

                    /* Check for comma and mode */
                    char* mode = NULL;
                    if (ji_p_peek(p) == JI_TOK_COMMA) {
                        ji_p_advance(p); /* consume comma */
                        /* Mode is like: Mode=TwoWay — read as identifier */
                        if (ji_p_peek(p) == JI_TOK_IDENTIFIER) {
                            /* Read the "Mode=" part */
                            char* mode_key = ji_token_text(ji_p_token(p));
                            ji_p_advance(p);
                            /* Expect '=' — for now, just skip to the value */
                            if (ji_p_peek(p) == JI_TOK_COLON ||
                                (mode_key && strstr(mode_key, "="))) {
                                /* Simplified: just read the next identifier as mode */
                            }
                            ji_free(mode_key);
                            if (ji_p_peek(p) == JI_TOK_IDENTIFIER) {
                                mode = ji_token_text(ji_p_token(p));
                                ji_p_advance(p);
                            }
                        }
                    }

                    ji_p_match(p, JI_TOK_RBRACE);
                    JiAstNode* n = ji_ast_binding(path, mode ? mode : "OneWay", line, col);
                    ji_free(kw_text);
                    ji_free(path);
                    ji_free(mode);
                    return n;
                } else if (kw_text && strcmp(kw_text, "Resource") == 0) {
                    /* Resource: {Resource Name} */
                    if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                        ji_parser_error(p, "Expected resource name after 'Resource'");
                        ji_free(kw_text);
                        ji_p_match(p, JI_TOK_RBRACE);
                        return NULL;
                    }
                    char* res_name = ji_token_text(ji_p_token(p));
                    ji_p_advance(p); /* consume resource name identifier */
                    ji_p_match(p, JI_TOK_RBRACE);
                    JiAstNode* n = ji_ast_resource(res_name, line, col);
                    ji_free(kw_text);
                    ji_free(res_name);
                    return n;
                }
                ji_free(kw_text);
            }

            ji_parser_error(p, "Unexpected '{' in value position");
            ji_p_match(p, JI_TOK_RBRACE);
            return NULL;
        }
        default:
            ji_parser_error(p, "Unexpected token in value: %s",
                            ji_token_type_name(ji_p_peek(p)));
            return NULL;
    }
}

/* =========================================================================
 * Parse a property or attached property
 *
 * property = [Identifier '.'] Identifier ':' value
 * ========================================================================= */
static JiAstNode* ji_p_parse_member(JiParser* p) {
    const JiToken* tok = ji_p_token(p);
    int line = tok->line;
    int col = tok->column;

    /* Could be a child object or a property */
    if (ji_p_peek(p) == JI_TOK_IDENTIFIER) {
        char* first = ji_token_text(tok);
        ji_p_advance(p);

        /* Check for dot (attached property or enum value) */
        if (ji_p_peek(p) == JI_TOK_DOT) {
            ji_p_advance(p); /* consume dot */
            if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                ji_parser_error(p, "Expected identifier after '.'");
                ji_free(first);
                return NULL;
            }
            const JiToken* second_tok = ji_p_token(p);
            char* second = ji_token_text(second_tok);
            ji_p_advance(p);

            /* Check for colon (attached property) */
            if (ji_p_peek(p) == JI_TOK_COLON) {
                ji_p_advance(p); /* consume colon */
                JiAstNode* value = ji_p_parse_value(p);
                JiAstNode* node = ji_ast_property(second, first, value, line, col);
                ji_free(first);
                ji_free(second);
                return node;
            } else {
                /* No colon — this is an object with a dot in the type name? 
                   Or it could be an enum value. For now, treat as error. */
                ji_parser_error(p, "Expected ':' after attached property name");
                ji_free(first);
                ji_free(second);
                return NULL;
            }
        }

        /* Check for hash (named element) */
        char* element_name = NULL;
        if (ji_p_peek(p) == JI_TOK_HASH) {
            ji_p_advance(p); /* consume # */
            if (ji_p_peek(p) != JI_TOK_IDENTIFIER) {
                ji_parser_error(p, "Expected identifier after '#'");
                ji_free(first);
                return NULL;
            }
            element_name = ji_token_text(ji_p_token(p));
            ji_p_advance(p);
        }

        /* Check for colon (property) or brace (child object) */
        if (ji_p_peek(p) == JI_TOK_COLON) {
            /* Property */
            ji_p_advance(p); /* consume colon */
            JiAstNode* value = ji_p_parse_value(p);
            JiAstNode* node = ji_ast_property(first, NULL, value, line, col);
            ji_free(first);
            ji_free(element_name);
            return node;
        } else if (ji_p_peek(p) == JI_TOK_LBRACE) {
            /* Child object */
            ji_p_advance(p); /* consume { */

            JiAstNode* obj = ji_ast_object(first, element_name, line, col);
            ji_free(first);
            ji_free(element_name);

            /* Parse members until } */
            while (ji_p_peek(p) != JI_TOK_RBRACE && ji_p_peek(p) != JI_TOK_EOF) {
                JiAstNode* member = ji_p_parse_member(p);
                if (member) {
                    ji_ast_add_child(obj, member);
                } else if (p->has_error) {
                    ji_ast_destroy(obj);
                    return NULL;
                }
            }

            if (!ji_p_expect(p, JI_TOK_RBRACE, "}")) {
                ji_ast_destroy(obj);
                return NULL;
            }
            return obj;
        } else {
            ji_parser_error(p, "Expected ':' or '{' after identifier '%s'", first);
            ji_free(first);
            ji_free(element_name);
            return NULL;
        }
    }

    ji_parser_error(p, "Expected identifier");
    return NULL;
}

/* =========================================================================
 * Parse a top-level object
 * ========================================================================= */
static JiAstNode* ji_p_parse_object(JiParser* p) {
    return ji_p_parse_member(p);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

void ji_parser_init(JiParser* parser, const char* source, size_t source_len) {
    memset(parser, 0, sizeof(JiParser));
    ji_lexer_init(&parser->lexer, source, source_len);
}

JiAstNode* ji_parser_parse(JiParser* parser) {
    JiAstNode* root = ji_p_parse_object(parser);
    if (parser->has_error) {
        ji_ast_destroy(root);
        return NULL;
    }
    return root;
}

bool ji_parser_has_error(const JiParser* parser) {
    return parser->has_error;
}

const char* ji_parser_error_message(const JiParser* parser) {
    return parser->error_msg;
}

JiAstNode* ji_parse_file(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)ji_alloc((size_t)size + 1);
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);

    JiAstNode* result = ji_parse_string(buf);
    ji_free(buf);
    return result;
}

JiAstNode* ji_parse_string(const char* source) {
    JiParser parser;
    ji_parser_init(&parser, source, strlen(source));
    return ji_parser_parse(&parser);
}
