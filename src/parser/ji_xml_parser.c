/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_xml_parser.c
 * @brief Implementation of the recursive-descent XML parser for .ji files.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_xml_parser.h"
#include "jiui/ji_memory.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

/* =========================================================================
 * Namespace map helpers
 * ========================================================================= */
static void ns_map_init(JiXmlNamespaceMap* map) {
    map->entries = NULL;
    map->count = 0;
    map->capacity = 0;
}

static void ns_map_add(JiXmlNamespaceMap* map, const char* prefix, const char* uri) {
    if (map->count >= map->capacity) {
        int new_cap = map->capacity == 0 ? 4 : map->capacity * 2;
        map->entries = (JiXmlNamespace*)ji_realloc(map->entries,
            (size_t)new_cap * sizeof(JiXmlNamespace));
        map->capacity = new_cap;
    }
    map->entries[map->count].prefix = prefix ? strdup(prefix) : NULL;
    map->entries[map->count].uri = uri ? strdup(uri) : NULL;
    map->count++;
}

static void ns_map_destroy(JiXmlNamespaceMap* map) {
    for (int i = 0; i < map->count; i++) {
        ji_free(map->entries[i].prefix);
        ji_free(map->entries[i].uri);
    }
    ji_free(map->entries);
}

/* =========================================================================
 * Parser error helper
 * ========================================================================= */
static void xml_parser_error(JiXmlParser* p, const char* fmt, ...) {
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
static JiXmlTokenType xp_peek(JiXmlParser* p) {
    return ji_xml_lexer_peek(&p->lexer);
}

static const JiXmlToken* xp_token(JiXmlParser* p) {
    return ji_xml_lexer_token(&p->lexer);
}

static JiXmlTokenType xp_advance(JiXmlParser* p) {
    return ji_xml_lexer_next(&p->lexer);
}

static bool xp_match(JiXmlParser* p, JiXmlTokenType type) {
    if (ji_xml_lexer_peek(&p->lexer) == type) {
        ji_xml_lexer_next(&p->lexer);
        return true;
    }
    return false;
}

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
static JiAstNode* xp_parse_element(JiXmlParser* p);
static JiAstNode* xp_parse_attr_value(JiXmlParser* p, const char* attr_name);

/* =========================================================================
 * Parse a markup extension in an attribute value
 *
 * Handles: {Binding Path}, {StaticResource Key}, {DynamicResource Key},
 *          {x:Null}, {x:Reference Name}
 * ========================================================================= */
static JiAstNode* xp_parse_markup_ext(JiXmlParser* p, const char* text, int line, int col) {
    /* text is like "{Binding Path, Mode=TwoWay}" */
    /* Strip leading { and trailing } */
    size_t len = strlen(text);
    if (len < 2 || text[0] != '{' || text[len-1] != '}') {
        /* Not a markup extension — treat as plain string */
        return ji_ast_string(text, line, col);
    }

    /* Extract inner content */
    char* inner = (char*)ji_alloc(len - 1);
    if (!inner) return ji_ast_string(text, line, col);
    memcpy(inner, text + 1, len - 2);
    inner[len - 2] = '\0';

    /* Trim leading whitespace */
    char* ptr = inner;
    while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;

    /* Read the extension type */
    char ext_type[64] = {0};
    int ei = 0;
    while (*ptr && *ptr != ' ' && *ptr != ',' && ei < 63) {
        ext_type[ei++] = *ptr++;
    }
    ext_type[ei] = '\0';

    /* Skip whitespace after type */
    while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;

    JiAstNode* result = NULL;

    if (strcmp(ext_type, "Binding") == 0) {
        /* Parse: Path [, Mode=...] */
        char path[256] = {0};
        char mode[64] = "OneWay";
        int pi = 0;

        /* Read path (up to comma or end) */
        while (*ptr && *ptr != ',' && pi < 255) {
            path[pi++] = *ptr++;
        }
        path[pi] = '\0';

        /* Trim trailing space from path */
        while (pi > 0 && (path[pi-1] == ' ' || path[pi-1] == '\t')) {
            path[--pi] = '\0';
        }

        /* Check for Mode= */
        if (*ptr == ',') {
            ptr++;
            while (*ptr && (*ptr == ' ' || *ptr == '\t')) ptr++;
            /* Read "Mode=" */
            char key[64] = {0};
            int ki = 0;
            while (*ptr && *ptr != '=' && ki < 63) {
                key[ki++] = *ptr++;
            }
            key[ki] = '\0';
            if (*ptr == '=') {
                ptr++;
                /* Read mode value */
                int mi = 0;
                while (*ptr && *ptr != ' ' && *ptr != ',' && mi < 63) {
                    mode[mi++] = *ptr++;
                }
                mode[mi] = '\0';
            }
        }

        result = ji_ast_binding(path, mode, line, col);
    } else if (strcmp(ext_type, "StaticResource") == 0 || strcmp(ext_type, "DynamicResource") == 0) {
        /* Parse: Key */
        char key[256] = {0};
        int ki = 0;
        while (*ptr && *ptr != ' ' && *ptr != ',' && ki < 255) {
            key[ki++] = *ptr++;
        }
        key[ki] = '\0';
        result = ji_ast_resource(key, line, col);
    } else if (strcmp(ext_type, "x:Null") == 0) {
        result = ji_ast_null(line, col);
    } else if (strcmp(ext_type, "x:Reference") == 0) {
        char name[256] = {0};
        int ni = 0;
        while (*ptr && *ptr != ' ' && *ptr != ',' && ni < 255) {
            name[ni++] = *ptr++;
        }
        name[ni] = '\0';
        result = ji_ast_reference(name, line, col);
    } else {
        /* Unknown markup extension — keep as string */
        result = ji_ast_string(text, line, col);
    }

    ji_free(inner);
    return result;
}

/* =========================================================================
 * Parse an attribute value string into an AST value node
 *
 * Converts the string to the appropriate typed AST node:
 *   - "true"/"false" → bool
 *   - "#RRGGBB" or "#AARRGGBB" → color
 *   - Integer string → int
 *   - Float string → float
 *   - "null" → null
 *   - EnumType.Value → enum
 *   - {Binding ...} → binding
 *   - {StaticResource ...} → resource
 *   - Otherwise → string
 * ========================================================================= */
static JiAstNode* xp_parse_attr_value(JiXmlParser* p, const char* attr_name) {
    const JiXmlToken* tok = xp_token(p);
    if (!tok || !tok->value) {
        return ji_ast_string("", tok ? tok->line : 0, tok ? tok->column : 0);
    }

    const char* val = tok->value;
    int line = tok->line;
    int col = tok->column;

    /* Check for markup extensions: starts with { */
    if (val[0] == '{') {
        return xp_parse_markup_ext(p, val, line, col);
    }

    /* Check for boolean */
    if (strcmp(val, "true") == 0) {
        return ji_ast_bool(true, line, col);
    }
    if (strcmp(val, "false") == 0) {
        return ji_ast_bool(false, line, col);
    }

    /* Check for null */
    if (strcmp(val, "null") == 0) {
        return ji_ast_null(line, col);
    }

    /* Check for color: #RRGGBB or #AARRGGBB */
    if (val[0] == '#') {
        unsigned long color = 0;
        int digits = 0;
        for (const char* c = val + 1; *c; c++) {
            if (*c >= '0' && *c <= '9') { color = color * 16 + (*c - '0'); digits++; }
            else if (*c >= 'a' && *c <= 'f') { color = color * 16 + (*c - 'a' + 10); digits++; }
            else if (*c >= 'A' && *c <= 'F') { color = color * 16 + (*c - 'A' + 10); digits++; }
            else break;
        }
        if (digits == 6 || digits == 8) {
            if (digits == 6) color |= 0xFF000000; /* add full alpha */
            return ji_ast_color(color, line, col);
        }
    }

    /* Check for integer/float (before enum, since "14.5" has a dot) */
    {
        bool is_numeric = true;
        bool has_dot = false;
        const char* s = val;
        if (*s == '-') s++;
        if (!*s) is_numeric = false;
        for (; *s; s++) {
            if (*s == '.') { has_dot = true; continue; }
            if (*s < '0' || *s > '9') { is_numeric = false; break; }
        }
        if (is_numeric && !has_dot) {
            return ji_ast_int(atol(val), line, col);
        }
        if (is_numeric && has_dot) {
            return ji_ast_float(atof(val), line, col);
        }
    }

    /* Check for enum: Type.Value */
    {
        const char* dot = strchr(val, '.');
        if (dot && dot != val && *(dot + 1) != '\0') {
            /* Has a dot with text on both sides — treat as enum */
            return ji_ast_enum(val, line, col);
        }
    }

    /* Default: string */
    return ji_ast_string(val, line, col);
}

/* =========================================================================
 * Parse attributes of an element
 *
 * Reads Name=Value pairs until TAG_END or TAG_SELF_CLOSE
 * ========================================================================= */
static void xp_parse_attributes(JiXmlParser* p, JiAstNode* obj) {
    while (xp_peek(p) == JI_XML_TOK_NAME) {
        const JiXmlToken* name_tok = xp_token(p);
        char* attr_name = name_tok->value ? strdup(name_tok->value) : NULL;
        int line = name_tok->line;
        int col = name_tok->column;

        xp_advance(p); /* consume name */

        /* Expect = */
        if (xp_peek(p) != JI_XML_TOK_EQUALS) {
            xml_parser_error(p, "Expected '=' after attribute name '%s'", attr_name ? attr_name : "?");
            ji_free(attr_name);
            return;
        }
        xp_advance(p); /* consume = */

        /* Expect attribute value */
        if (xp_peek(p) != JI_XML_TOK_ATTR_VALUE) {
            xml_parser_error(p, "Expected attribute value after '='");
            ji_free(attr_name);
            return;
        }

        /* Parse the value into a typed AST node */
        JiAstNode* value_node = xp_parse_attr_value(p, attr_name);
        xp_advance(p); /* consume the attr value token */

        /* Check for x:Name — extract as element_name */
        if (attr_name && strcmp(attr_name, "x:Name") == 0 && value_node) {
            if (value_node->type == JI_AST_STRING_VAL && value_node->v.string_val) {
                if (obj->element_name) ji_free(obj->element_name);
                obj->element_name = strdup(value_node->v.string_val);
            }
            /* Still add as attribute for completeness */
        }

        /* Check for xmlns — register namespace */
        if (attr_name && strncmp(attr_name, "xmlns", 5) == 0) {
            const char* prefix = NULL;
            if (attr_name[5] == ':' && attr_name[6]) {
                prefix = attr_name + 6;
            } else if (attr_name[5] == '\0') {
                prefix = ""; /* default namespace */
            }
            if (prefix && value_node && value_node->type == JI_AST_STRING_VAL && value_node->v.string_val) {
                ns_map_add(&p->namespaces, prefix, value_node->v.string_val);
            }
            /* Don't add xmlns as an attribute — it's metadata */
            ji_free(attr_name);
            ji_ast_destroy(value_node);
            continue;
        }

        /* Create attribute node and add to object */
        JiAstNode* attr = ji_ast_attribute(attr_name, value_node, line, col);
        ji_ast_add_attr(obj, attr);

        ji_free(attr_name);
    }
}

/* =========================================================================
 * Parse element content (between > and </)
 *
 * Handles:
 *   - Child elements
 *   - Text content
 *   - Property elements (OwnerType.PropertyName)
 *   - Comments, CDATA, PI (skipped)
 * ========================================================================= */
static void xp_parse_content(JiXmlParser* p, JiAstNode* parent) {
    while (xp_peek(p) != JI_XML_TOK_TAG_CLOSE &&
           xp_peek(p) != JI_XML_TOK_EOF) {

        if (xp_peek(p) == JI_XML_TOK_COMMENT ||
            xp_peek(p) == JI_XML_TOK_PI ||
            xp_peek(p) == JI_XML_TOK_CDATA) {
            /* Skip comments, PIs, CDATA */
            xp_advance(p);
            continue;
        }

        if (xp_peek(p) == JI_XML_TOK_TEXT) {
            /* Text content */
            const JiXmlToken* tok = xp_token(p);
            if (tok->value && tok->value[0] != '\0') {
                JiAstNode* text = ji_ast_text_content(tok->value, tok->line, tok->column);
                ji_ast_add_child(parent, text);
            }
            xp_advance(p);
            continue;
        }

        if (xp_peek(p) == JI_XML_TOK_TAG_OPEN) {
            /* Child element */
            JiAstNode* child = xp_parse_element(p);
            if (child) {
                /* Check if this is a property element (name contains '.') */
                if (child->type_name && strchr(child->type_name, '.')) {
                    /* Property element — convert to property node */
                    char* dot = strchr(child->type_name, '.');
                    *dot = '\0';
                    char* owner = strdup(child->type_name);
                    char* prop = strdup(dot + 1);

                    /* The property value comes from the child's content */
                    JiAstNode* prop_value = NULL;
                    if (child->attr_count > 0) {
                        /* Attributes on property element become the value */
                        prop_value = child->attributes[0]->attr_value;
                        child->attributes[0]->attr_value = NULL;
                    } else if (child->child_count == 1) {
                        /* Single child becomes the value */
                        prop_value = child->children[0];
                        child->children[0] = NULL;
                        child->child_count = 0;
                    } else if (child->child_count == 0 && child->attr_count == 0) {
                        /* Check for text content in the original text */
                        prop_value = ji_ast_string("", child->line, child->column);
                    }

                    JiAstNode* prop_node = ji_ast_property(prop, owner, prop_value,
                                                             child->line, child->column);
                    ji_ast_add_child(parent, prop_node);

                    ji_free(owner);
                    ji_free(prop);
                    ji_ast_destroy(child);
                } else {
                    ji_ast_add_child(parent, child);
                }
            }
            continue;
        }

        /* Unexpected token — skip */
        xp_advance(p);
    }
}

/* =========================================================================
 * Parse a single element
 *
 * <TypeName attr="value" ...>
 *   ... content ...
 * </TypeName>
 *
 * or
 *
 * <TypeName attr="value" ... />
 * ========================================================================= */
static JiAstNode* xp_parse_element(JiXmlParser* p) {
    /* Expect TAG_OPEN */
    if (xp_peek(p) != JI_XML_TOK_TAG_OPEN) {
        xml_parser_error(p, "Expected '<' to start element");
        return NULL;
    }
    int line = xp_token(p)->line;
    int col = xp_token(p)->column;
    xp_advance(p); /* consume < */

    /* Expect element name */
    if (xp_peek(p) != JI_XML_TOK_NAME) {
        xml_parser_error(p, "Expected element name after '<'");
        return NULL;
    }
    const JiXmlToken* name_tok = xp_token(p);
    char* type_name = name_tok->value ? strdup(name_tok->value) : strdup("?");
    xp_advance(p); /* consume name */

    /* Create object node */
    JiAstNode* obj = ji_ast_object(type_name, NULL, line, col);
    ji_free(type_name);

    if (!obj) return NULL;

    /* Parse attributes */
    xp_parse_attributes(p, obj);

    /* Check for self-closing /> */
    if (xp_peek(p) == JI_XML_TOK_TAG_SELF_CLOSE) {
        xp_advance(p); /* consume /> */
        return obj;
    }

    /* Expect > */
    if (xp_peek(p) != JI_XML_TOK_TAG_END) {
        xml_parser_error(p, "Expected '>' or '/>' after attributes");
        ji_ast_destroy(obj);
        return NULL;
    }
    xp_advance(p); /* consume > */

    /* Parse content */
    xp_parse_content(p, obj);

    /* Expect </TypeName> */
    if (xp_peek(p) == JI_XML_TOK_TAG_CLOSE) {
        xp_advance(p); /* consume </ */

        if (xp_peek(p) == JI_XML_TOK_NAME) {
            /* Verify closing tag name matches opening */
            const JiXmlToken* close_name = xp_token(p);
            /* We don't strictly enforce matching for now, just consume it */
            xp_advance(p); /* consume closing name */
        }

        if (xp_peek(p) == JI_XML_TOK_TAG_END) {
            xp_advance(p); /* consume > */
        }
    }

    return obj;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

JI_API void ji_xml_parser_init(JiXmlParser* parser, const char* source, size_t source_len) {
    memset(parser, 0, sizeof(JiXmlParser));
    ji_xml_lexer_init(&parser->lexer, source, source_len);
    ns_map_init(&parser->namespaces);
    parser->has_error = false;
    parser->error_msg[0] = '\0';
}

JI_API JiAstNode* ji_xml_parser_parse(JiXmlParser* parser) {
    if (parser->has_error) return NULL;

    /* Skip processing instructions and comments at the start */
    while (xp_peek(parser) == JI_XML_TOK_PI || xp_peek(parser) == JI_XML_TOK_COMMENT) {
        xp_advance(parser);
    }

    /* Parse root element */
    if (xp_peek(parser) != JI_XML_TOK_TAG_OPEN) {
        xml_parser_error(parser, "Expected root element '<...>'");
        return NULL;
    }

    JiAstNode* root = xp_parse_element(parser);
    if (!root) return NULL;

    /* Skip trailing comments/PIs */
    while (xp_peek(parser) == JI_XML_TOK_PI || xp_peek(parser) == JI_XML_TOK_COMMENT) {
        xp_advance(parser);
    }

    /* Should be at EOF */
    if (xp_peek(parser) != JI_XML_TOK_EOF) {
        xml_parser_error(parser, "Unexpected content after root element");
        ji_ast_destroy(root);
        return NULL;
    }

    return root;
}

JI_API bool ji_xml_parser_has_error(const JiXmlParser* parser) {
    return parser->has_error;
}

JI_API const char* ji_xml_parser_error_message(const JiXmlParser* parser) {
    return parser->error_msg;
}

JI_API void ji_xml_parser_destroy(JiXmlParser* parser) {
    if (parser) {
        ji_xml_lexer_destroy(&parser->lexer);
        ns_map_destroy(&parser->namespaces);
    }
}

/* =========================================================================
 * Convenience: parse from file
 * ========================================================================= */
JI_API JiAstNode* ji_xml_parse_file(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)ji_alloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)size, f);
    buf[size] = '\0';
    fclose(f);

    JiXmlParser parser;
    ji_xml_parser_init(&parser, buf, (size_t)size);
    JiAstNode* root = ji_xml_parser_parse(&parser);

    ji_free(buf);
    ji_xml_parser_destroy(&parser);
    return root;
}

/* =========================================================================
 * Convenience: parse from string
 * ========================================================================= */
JI_API JiAstNode* ji_xml_parse_string(const char* source) {
    JiXmlParser parser;
    ji_xml_parser_init(&parser, source, strlen(source));
    JiAstNode* root = ji_xml_parser_parse(&parser);
    ji_xml_parser_destroy(&parser);
    return root;
}
