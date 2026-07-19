/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_ast.h
 * @brief Abstract Syntax Tree nodes for .ji file parsing.
 */

#ifndef JIUI_AST_H
#define JIUI_AST_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_lexer.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * AST Node types
 * ========================================================================= */
typedef enum JiAstType {
    JI_AST_OBJECT,       /* TypeName [#Name] { members } */
    JI_AST_PROPERTY,     /* PropertyName: value */
    JI_AST_ATTACHED_PROP,/* OwnerType.PropertyName: value */
    JI_AST_STRING_VAL,
    JI_AST_INT_VAL,
    JI_AST_FLOAT_VAL,
    JI_AST_BOOL_VAL,
    JI_AST_NULL_VAL,
    JI_AST_COLOR_VAL,
    JI_AST_ENUM_VAL,     /* Type.Value */
    JI_AST_REFERENCE,    /* @Name */
    JI_AST_BINDING,      /* {bind Path, Mode=...} */
    JI_AST_RESOURCE,     /* {Resource Name} */
    JI_AST_GRID_LENGTH,  /* auto, Npx, N* */

    /* XML-specific node types */
    JI_AST_ATTRIBUTE,    /* XML attribute (name=value) */
    JI_AST_TEXT_CONTENT, /* Character data between tags */
} JiAstType;

/* =========================================================================
 * Grid length kind
 * ========================================================================= */
typedef enum JiAstGridKind {
    JI_GRID_AST_AUTO,
    JI_GRID_AST_PIXEL,
    JI_GRID_AST_STAR
} JiAstGridKind;

/* =========================================================================
 * JiAstNode — a node in the AST
 * ========================================================================= */
typedef struct JiAstNode JiAstNode;

struct JiAstNode {
    JiAstType    type;
    int          line;
    int          column;

    /* For JI_AST_OBJECT */
    char*        type_name;      /* e.g. "Window" */
    char*        element_name;   /* e.g. "myButton" (x:Name) */
    JiAstNode**  attributes;     /* XML attributes (JI_AST_ATTRIBUTE nodes) */
    int          attr_count;
    int          attr_capacity;
    JiAstNode**  children;       /* array of child nodes */
    int          child_count;
    int          child_capacity;

    /* For JI_AST_PROPERTY / JI_AST_ATTACHED_PROP */
    char*        prop_name;      /* e.g. "Width" */
    char*        prop_owner;     /* e.g. "Grid" (for attached), NULL otherwise */
    JiAstNode*   value;          /* the value expression */

    /* For JI_AST_ATTRIBUTE */
    char*        attr_name;      /* attribute name (e.g. "Content", "x:Name") */
    JiAstNode*   attr_value;     /* attribute value node (literal or markup extension) */

    /* For JI_AST_TEXT_CONTENT */
    char*        text_content;   /* decoded text between tags */

    /* For literal values */
    union {
        char*       string_val;  /* JI_AST_STRING_VAL, JI_AST_ENUM_VAL */
        long        int_val;     /* JI_AST_INT_VAL */
        double      float_val;   /* JI_AST_FLOAT_VAL */
        bool        bool_val;     /* JI_AST_BOOL_VAL */
        unsigned long color_val; /* JI_AST_COLOR_VAL */
        struct {
            double      amount;  /* JI_AST_GRID_LENGTH */
            JiAstGridKind kind;
        } grid;
        struct {
            char*       path;    /* JI_AST_BINDING */
            char*       mode;    /* "OneWay", "TwoWay", etc. */
        } binding;
        char*       resource_name; /* JI_AST_RESOURCE */
        char*       reference_name; /* JI_AST_REFERENCE */
    } v;
};

/* =========================================================================
 * AST API
 * ========================================================================= */

/** Create an object AST node. */
JI_API JiAstNode* ji_ast_object(const char* type_name, const char* element_name,
                                 int line, int column);

/** Create a property AST node. */
JI_API JiAstNode* ji_ast_property(const char* name, const char* owner,
                                   JiAstNode* value, int line, int column);

/** Create a string value node. */
JI_API JiAstNode* ji_ast_string(const char* val, int line, int column);

/** Create an integer value node. */
JI_API JiAstNode* ji_ast_int(long val, int line, int column);

/** Create a float value node. */
JI_API JiAstNode* ji_ast_float(double val, int line, int column);

/** Create a boolean value node. */
JI_API JiAstNode* ji_ast_bool(bool val, int line, int column);

/** Create a null value node. */
JI_API JiAstNode* ji_ast_null(int line, int column);

/** Create a color value node. */
JI_API JiAstNode* ji_ast_color(unsigned long argb, int line, int column);

/** Create an enum value node (e.g. "HorizontalAlignment.Center"). */
JI_API JiAstNode* ji_ast_enum(const char* val, int line, int column);

/** Create a reference node (e.g. "@myButton"). */
JI_API JiAstNode* ji_ast_reference(const char* name, int line, int column);

/** Create a binding node. */
JI_API JiAstNode* ji_ast_binding(const char* path, const char* mode,
                                   int line, int column);

/** Create a resource reference node. */
JI_API JiAstNode* ji_ast_resource(const char* name, int line, int column);

/** Create a grid length node. */
JI_API JiAstNode* ji_ast_grid_length(double amount, JiAstGridKind kind,
                                      int line, int column);

/** Add a child node to an object node. */
JI_API void ji_ast_add_child(JiAstNode* parent, JiAstNode* child);

/** Add an attribute node to an object node. */
JI_API void ji_ast_add_attr(JiAstNode* parent, JiAstNode* attr);

/** Create an attribute AST node (name=value). */
JI_API JiAstNode* ji_ast_attribute(const char* name, JiAstNode* value,
                                    int line, int column);

/** Create a text content AST node. */
JI_API JiAstNode* ji_ast_text_content(const char* text, int line, int column);

/** Destroy an AST node and all its children. */
JI_API void ji_ast_destroy(JiAstNode* node);

/** Print an AST tree to stdout (for debugging). */
JI_API void ji_ast_print(const JiAstNode* node, int indent);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_AST_H */
