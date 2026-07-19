/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_ast.c
 * @brief Implementation of AST node construction and destruction.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_ast.h"
#include "jiui/ji_memory.h"

#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Internal: allocate and initialize a base node
 * ========================================================================= */
static JiAstNode* ji_ast_new(JiAstType type, int line, int column) {
    JiAstNode* node = JI_NEW(JiAstNode);
    if (!node) return NULL;
    memset(node, 0, sizeof(JiAstNode));
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

/* =========================================================================
 * Construction API
 * ========================================================================= */

JiAstNode* ji_ast_object(const char* type_name, const char* element_name,
                           int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_OBJECT, line, column);
    if (!node) return NULL;
    node->type_name = type_name ? strdup(type_name) : NULL;
    node->element_name = element_name ? strdup(element_name) : NULL;
    return node;
}

JiAstNode* ji_ast_property(const char* name, const char* owner,
                             JiAstNode* value, int line, int column) {
    JiAstNode* node = ji_ast_new(owner ? JI_AST_ATTACHED_PROP : JI_AST_PROPERTY, line, column);
    if (!node) return NULL;
    node->prop_name = name ? strdup(name) : NULL;
    node->prop_owner = owner ? strdup(owner) : NULL;
    node->value = value;
    return node;
}

JiAstNode* ji_ast_string(const char* val, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_STRING_VAL, line, column);
    if (!node) return NULL;
    node->v.string_val = val ? strdup(val) : NULL;
    return node;
}

JiAstNode* ji_ast_int(long val, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_INT_VAL, line, column);
    if (!node) return NULL;
    node->v.int_val = val;
    return node;
}

JiAstNode* ji_ast_float(double val, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_FLOAT_VAL, line, column);
    if (!node) return NULL;
    node->v.float_val = val;
    return node;
}

JiAstNode* ji_ast_bool(bool val, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_BOOL_VAL, line, column);
    if (!node) return NULL;
    node->v.bool_val = val;
    return node;
}

JiAstNode* ji_ast_null(int line, int column) {
    return ji_ast_new(JI_AST_NULL_VAL, line, column);
}

JiAstNode* ji_ast_color(unsigned long argb, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_COLOR_VAL, line, column);
    if (!node) return NULL;
    node->v.color_val = argb;
    return node;
}

JiAstNode* ji_ast_enum(const char* val, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_ENUM_VAL, line, column);
    if (!node) return NULL;
    node->v.string_val = val ? strdup(val) : NULL;
    return node;
}

JiAstNode* ji_ast_reference(const char* name, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_REFERENCE, line, column);
    if (!node) return NULL;
    node->v.reference_name = name ? strdup(name) : NULL;
    return node;
}

JiAstNode* ji_ast_binding(const char* path, const char* mode,
                           int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_BINDING, line, column);
    if (!node) return NULL;
    node->v.binding.path = path ? strdup(path) : NULL;
    node->v.binding.mode = mode ? strdup(mode) : NULL;
    return node;
}

JiAstNode* ji_ast_resource(const char* name, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_RESOURCE, line, column);
    if (!node) return NULL;
    node->v.resource_name = name ? strdup(name) : NULL;
    return node;
}

JiAstNode* ji_ast_grid_length(double amount, JiAstGridKind kind,
                               int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_GRID_LENGTH, line, column);
    if (!node) return NULL;
    node->v.grid.amount = amount;
    node->v.grid.kind = kind;
    return node;
}

/* =========================================================================
 * Add child
 * ========================================================================= */
void ji_ast_add_child(JiAstNode* parent, JiAstNode* child) {
    if (!parent || !child) return;
    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0 ? 8 : parent->child_capacity * 2;
        parent->children = (JiAstNode**)ji_realloc(parent->children,
            (size_t)new_cap * sizeof(JiAstNode*));
        parent->child_capacity = new_cap;
    }
    parent->children[parent->child_count++] = child;
}

/* =========================================================================
 * Add attribute
 * ========================================================================= */
void ji_ast_add_attr(JiAstNode* parent, JiAstNode* attr) {
    if (!parent || !attr) return;
    if (parent->attr_count >= parent->attr_capacity) {
        int new_cap = parent->attr_capacity == 0 ? 8 : parent->attr_capacity * 2;
        parent->attributes = (JiAstNode**)ji_realloc(parent->attributes,
            (size_t)new_cap * sizeof(JiAstNode*));
        parent->attr_capacity = new_cap;
    }
    parent->attributes[parent->attr_count++] = attr;
}

/* =========================================================================
 * Create attribute node
 * ========================================================================= */
JiAstNode* ji_ast_attribute(const char* name, JiAstNode* value,
                              int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_ATTRIBUTE, line, column);
    if (!node) return NULL;
    node->attr_name = name ? strdup(name) : NULL;
    node->attr_value = value;
    return node;
}

/* =========================================================================
 * Create text content node
 * ========================================================================= */
JiAstNode* ji_ast_text_content(const char* text, int line, int column) {
    JiAstNode* node = ji_ast_new(JI_AST_TEXT_CONTENT, line, column);
    if (!node) return NULL;
    node->text_content = text ? strdup(text) : NULL;
    return node;
}

/* =========================================================================
 * Destroy
 * ========================================================================= */
void ji_ast_destroy(JiAstNode* node) {
    if (!node) return;

    /* Destroy children */
    for (int i = 0; i < node->child_count; i++) {
        ji_ast_destroy(node->children[i]);
    }
    if (node->children) ji_free(node->children);

    /* Destroy attributes */
    for (int i = 0; i < node->attr_count; i++) {
        ji_ast_destroy(node->attributes[i]);
    }
    if (node->attributes) ji_free(node->attributes);

    /* Destroy value */
    if (node->value) ji_ast_destroy(node->value);

    /* Destroy attr_value */
    if (node->attr_value) ji_ast_destroy(node->attr_value);

    /* Destroy strings */
    ji_free(node->type_name);
    ji_free(node->element_name);
    ji_free(node->prop_name);
    ji_free(node->prop_owner);
    ji_free(node->attr_name);
    ji_free(node->text_content);

    /* Destroy type-specific data */
    switch (node->type) {
        case JI_AST_STRING_VAL:
        case JI_AST_ENUM_VAL:
            ji_free(node->v.string_val);
            break;
        case JI_AST_REFERENCE:
            ji_free(node->v.reference_name);
            break;
        case JI_AST_BINDING:
            ji_free(node->v.binding.path);
            ji_free(node->v.binding.mode);
            break;
        case JI_AST_RESOURCE:
            ji_free(node->v.resource_name);
            break;
        default:
            break;
    }

    ji_free(node);
}

/* =========================================================================
 * Print (debug)
 * ========================================================================= */
static void ji_print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void ji_ast_print(const JiAstNode* node, int indent) {
    if (!node) return;

    ji_print_indent(indent);

    switch (node->type) {
        case JI_AST_OBJECT:
            printf("Object[%s", node->type_name ? node->type_name : "?");
            if (node->element_name) printf(" #%s", node->element_name);
            printf("]");
            /* Print attributes */
            if (node->attr_count > 0) {
                printf(" attrs=[");
                for (int i = 0; i < node->attr_count; i++) {
                    if (i > 0) printf(", ");
                    ji_ast_print(node->attributes[i], 0);
                }
                printf("]");
            }
            printf(" {\n");
            for (int i = 0; i < node->child_count; i++) {
                ji_ast_print(node->children[i], indent + 1);
            }
            ji_print_indent(indent);
            printf("}\n");
            break;
        case JI_AST_PROPERTY:
            printf("Property[%s] = ", node->prop_name ? node->prop_name : "?");
            if (node->value) ji_ast_print(node->value, 0);
            else printf("null\n");
            break;
        case JI_AST_ATTACHED_PROP:
            printf("AttachedProp[%s.%s] = ",
                   node->prop_owner ? node->prop_owner : "?",
                   node->prop_name ? node->prop_name : "?");
            if (node->value) ji_ast_print(node->value, 0);
            else printf("null\n");
            break;
        case JI_AST_ATTRIBUTE:
            printf("%s=", node->attr_name ? node->attr_name : "?");
            if (node->attr_value) ji_ast_print(node->attr_value, 0);
            else printf("null");
            break;
        case JI_AST_TEXT_CONTENT:
            printf("Text[%s]\n", node->text_content ? node->text_content : "");
            break;
        case JI_AST_STRING_VAL:
            printf("\"%s\"\n", node->v.string_val ? node->v.string_val : "");
            break;
        case JI_AST_INT_VAL:
            printf("%ld\n", node->v.int_val);
            break;
        case JI_AST_FLOAT_VAL:
            printf("%g\n", node->v.float_val);
            break;
        case JI_AST_BOOL_VAL:
            printf("%s\n", node->v.bool_val ? "true" : "false");
            break;
        case JI_AST_NULL_VAL:
            printf("null\n");
            break;
        case JI_AST_COLOR_VAL:
            printf("#%08lX\n", node->v.color_val);
            break;
        case JI_AST_ENUM_VAL:
            printf("%s\n", node->v.string_val ? node->v.string_val : "?");
            break;
        case JI_AST_REFERENCE:
            printf("@%s\n", node->v.reference_name ? node->v.reference_name : "?");
            break;
        case JI_AST_BINDING:
            printf("{bind %s", node->v.binding.path ? node->v.binding.path : "?");
            if (node->v.binding.mode) printf(", Mode=%s", node->v.binding.mode);
            printf("}\n");
            break;
        case JI_AST_RESOURCE:
            printf("{Resource %s}\n", node->v.resource_name ? node->v.resource_name : "?");
            break;
        case JI_AST_GRID_LENGTH:
            switch (node->v.grid.kind) {
                case JI_GRID_AST_AUTO:  printf("auto\n"); break;
                case JI_GRID_AST_PIXEL: printf("%gpx\n", node->v.grid.amount); break;
                case JI_GRID_AST_STAR:  printf("%g*\n", node->v.grid.amount); break;
            }
            break;
    }
}
