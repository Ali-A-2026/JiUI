/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_jiml.h
 * @brief JiML (JiUI Markup Language) — high-level declarative UI language.
 *
 * JiML extends the existing XML-based .ji format with:
 *   - Data binding expressions: {variable_name}
 *   - Event binding: on-click="handler_name"
 *   - Inline style blocks: <Style> ... </Style>
 *   - Component definitions: <Component name="MyButton"> ... </Component>
 *   - Conditional rendering: if="{condition}"
 *   - List rendering: each="{items}" as="item"
 *   - Expression evaluation: {a + b}, {value * 2}, {!flag}
 *
 * The JiML pipeline:
 *   1. Lex .jiml source → tokens (ji_jiml_lexer)
 *   2. Parse tokens → JiML AST (ji_jiml_parser)
 *   3. Compile JiML AST → runtime object tree (ji_jiml_compile)
 *
 * The JiML AST reuses JiAstNode from ji_ast.h for compatibility.
 */

#ifndef JIUI_JIML_H
#define JIUI_JIML_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_ast.h"
#include "ji_object.h"
#include "ji_tree_builder.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiML Expression Types
 * ========================================================================= */

/** Types of binding expressions in JiML. */
typedef enum JiBindingExprType {
    JI_BINDING_EXPR_NONE = 0,       /* Not a binding */
    JI_BINDING_EXPR_SIMPLE,         /* {variable} */
    JI_BINDING_EXPR_PATH,          /* {model.property} */
    JI_BINDING_EXPR_BINARY,        /* {a + b}, {a * b}, etc. */
    JI_BINDING_EXPR_UNARY,         /* {!flag}, {-value} */
    JI_BINDING_EXPR_FUNCTION,      /* {func(arg1, arg2)} */
    JI_BINDING_EXPR_CONDITIONAL    /* {cond ? a : b} */
} JiBindingExprType;

/** Binary operators in binding expressions. */
typedef enum JiBindingOp {
    JI_BINDING_OP_NONE = 0,
    JI_BINDING_OP_ADD,        /* + */
    JI_BINDING_OP_SUB,        /* - */
    JI_BINDING_OP_MUL,        /* * */
    JI_BINDING_OP_DIV,        /* / */
    JI_BINDING_OP_MOD,        /* % */
    JI_BINDING_OP_EQ,         /* == */
    JI_BINDING_OP_NE,         /* != */
    JI_BINDING_OP_LT,         /* < */
    JI_BINDING_OP_LE,         /* <= */
    JI_BINDING_OP_GT,         /* > */
    JI_BINDING_OP_GE,         /* >= */
    JI_BINDING_OP_AND,        /* && */
    JI_BINDING_OP_OR,         /* || */
    JI_BINDING_OP_NEG,        /* unary - */
    JI_BINDING_OP_NOT         /* unary ! */
} JiBindingOp;

/** A parsed binding expression. */
typedef struct JiBindingExpr {
    JiBindingExprType type;
    char              var_name[128];     /* For SIMPLE/PATH: variable name */
    JiBindingOp       op;                /* For BINARY/UNARY: operator */
    struct JiBindingExpr* left;          /* For BINARY/UNARY/CONDITIONAL */
    struct JiBindingExpr* right;         /* For BINARY/CONDITIONAL */
    struct JiBindingExpr* condition;     /* For CONDITIONAL */
    char              func_name[64];     /* For FUNCTION: function name */
    struct JiBindingExpr** args;         /* For FUNCTION: arguments */
    int               arg_count;
    double            literal_num;       /* For numeric literal */
    char              literal_str[256];  /* For string literal */
    bool              is_literal;        /* True if this is a literal value */
    bool              is_bool;           /* True if literal is boolean */
} JiBindingExpr;

/* =========================================================================
 * JiML Style Rule
 * ========================================================================= */

/** A single JiML style rule (selector + properties). */
typedef struct JiJimlStyleRule {
    char*   selector;          /* e.g. "JiButton", "JiButton:hover" */
    char**  prop_names;        /* Property names */
    char**  prop_values;       /* Property values (may contain bindings) */
    int     prop_count;        /* Number of properties */
    int     prop_capacity;     /* Allocated capacity */
} JiJimlStyleRule;

/** A collection of JiML style rules. */
typedef struct JiJimlStyleBlock {
    JiJimlStyleRule* rules;
    int              rule_count;
    int              rule_capacity;
} JiJimlStyleBlock;

/* =========================================================================
 * JiML Component Definition
 * ========================================================================= */

/** A component definition (reusable widget template). */
typedef struct JiComponentDef {
    char         name[128];           /* Component name */
    char**       param_names;         /* Parameter names */
    int          param_count;         /* Number of parameters */
    JiAstNode*   body;                /* AST body of the component */
} JiComponentDef;

/* =========================================================================
 * JiML Document
 * ========================================================================= */

/** A parsed JiML document. */
typedef struct JiJimlDocument {
    JiAstNode*       root;             /* Root widget AST node */
    JiJimlStyleBlock** styles;       /* Style blocks (array of pointers, may be NULL) */
    int              style_count;      /* Number of style blocks */
    JiComponentDef*  components;       /* Component definitions */
    int              component_count;  /* Number of components */
    char             error_msg[512];   /* Error message (if parse failed) */
    int              error_line;       /* Error line (if parse failed) */
    bool             has_error;        /* True if parsing failed */
} JiJimlDocument;

/* =========================================================================
 * JiML Parser API
 * ========================================================================= */

/**
 * Parse a JiML string into a JiML document.
 * @param source  JiML source text.
 * @return        Parsed document. Caller must call ji_jiml_document_destroy().
 */
JI_API JiJimlDocument* ji_jiml_parse(const char* source);

/**
 * Parse a JiML file from disk.
 * @param filepath  Path to the .jiml file.
 * @return          Parsed document. Caller must call ji_jiml_document_destroy().
 */
JI_API JiJimlDocument* ji_jiml_parse_file(const char* filepath);

/**
 * Destroy a JiML document and free all resources.
 * @param doc  Document to destroy (may be NULL).
 */
JI_API void ji_jiml_document_destroy(JiJimlDocument* doc);

/* =========================================================================
 * Binding Expression API
 * ========================================================================= */

/**
 * Parse a binding expression from a string.
 * @param expr  Expression text without surrounding braces (e.g. "a + b").
 * @return      Parsed expression tree, or NULL on error.
 *              Caller must call ji_binding_expr_destroy().
 */
JI_API JiBindingExpr* ji_binding_expr_parse(const char* expr);

/**
 * Destroy a binding expression tree.
 * @param expr  Expression to destroy (may be NULL).
 */
JI_API void ji_binding_expr_destroy(JiBindingExpr* expr);

/**
 * Evaluate a binding expression against a data context.
 * @param expr     Expression to evaluate.
 * @param context  Data context (object with named properties).
 * @param out_val  Output: evaluated value as a string (caller provides buffer).
 * @param buf_size Size of out_val buffer.
 * @return         True on success, false on error.
 */
JI_API bool ji_binding_expr_eval(const JiBindingExpr* expr,
                                   const void* context,
                                   char* out_val, size_t buf_size);

/**
 * Check if a string is a binding expression (starts with '{' and ends with '}').
 * @param str  String to check.
 * @return     True if the string is a binding expression.
 */
JI_API bool ji_is_binding_expr(const char* str);

/**
 * Extract the inner expression from a binding string "{expr}".
 * @param str       Binding string (e.g. "{my_var}").
 * @param out       Output buffer for the inner expression.
 * @param buf_size  Size of output buffer.
 * @return          True on success, false if not a binding or buffer too small.
 */
JI_API bool ji_binding_expr_extract(const char* str, char* out, size_t buf_size);

/* =========================================================================
 * JiML Compiler API
 * ========================================================================= */

/**
 * Compile a JiML document into a runtime object tree.
 * @param doc       Parsed JiML document.
 * @param context   Optional data context for binding resolution (may be NULL).
 * @return          Build result containing the root object or error info.
 *                  Caller must release the root object via ji_ref_object_release().
 */
JI_API JiTreeBuildResult ji_jiml_compile(const JiJimlDocument* doc,
                                          const void* context);

/**
 * Compile a JiML string into a runtime object tree.
 * Convenience function: parse + compile.
 * @param source   JiML source text.
 * @param context  Optional data context (may be NULL).
 * @return         Build result.
 */
JI_API JiTreeBuildResult ji_jiml_compile_string(const char* source,
                                                  const void* context);

/**
 * Compile a JiML file into a runtime object tree.
 * @param filepath  Path to the .jiml file.
 * @param context   Optional data context (may be NULL).
 * @return          Build result.
 */
JI_API JiTreeBuildResult ji_jiml_compile_file(const char* filepath,
                                                const void* context);

/* =========================================================================
 * Style Block API
 * ========================================================================= */

/**
 * Create a new empty style block.
 * @return  New style block (caller must destroy with ji_style_block_destroy).
 */
JI_API JiJimlStyleBlock* ji_style_block_new(void);

/**
 * Destroy a style block and free all resources.
 * @param block  Style block to destroy (may be NULL).
 */
JI_API void ji_style_block_destroy(JiJimlStyleBlock* block);

/**
 * Add a style rule to a style block.
 * @param block     Style block.
 * @param selector  CSS-like selector (e.g. "JiButton:hover").
 * @return          Index of the new rule, or -1 on error.
 */
JI_API int ji_style_block_add_rule(JiJimlStyleBlock* block, const char* selector);

/**
 * Add a property to a style rule.
 * @param block   Style block.
 * @param rule_idx  Rule index (from ji_style_block_add_rule).
 * @param name    Property name.
 * @param value   Property value (may contain binding expressions).
 * @return        True on success, false on error.
 */
JI_API bool ji_style_rule_add_prop(JiJimlStyleBlock* block, int rule_idx,
                                     const char* name, const char* value);

/**
 * Find style rules matching a widget type name.
 * @param block      Style block.
 * @param type_name  Widget type name (e.g. "JiButton").
 * @param out_indices Output array of matching rule indices.
 * @param max_count   Maximum number of indices to return.
 * @return           Number of matching rules found.
 */
JI_API int ji_style_block_find_rules(const JiJimlStyleBlock* block,
                                       const char* type_name,
                                       int* out_indices, int max_count);

/* =========================================================================
 * Component Definition API
 * ========================================================================= */

/**
 * Create a new component definition.
 * @param name  Component name.
 * @return      New component definition (caller must destroy).
 */
JI_API JiComponentDef* ji_component_def_new(const char* name);

/**
 * Destroy a component definition.
 * @param def  Component definition to destroy (may be NULL).
 */
JI_API void ji_component_def_destroy(JiComponentDef* def);

/**
 * Add a parameter to a component definition.
 * @param def   Component definition.
 * @param name  Parameter name.
 * @return     True on success, false on error.
 */
JI_API bool ji_component_def_add_param(JiComponentDef* def, const char* name);

/**
 * Find a component definition by name in a JiML document.
 * @param doc   JiML document.
 * @param name  Component name.
 * @return     Component definition, or NULL if not found.
 */
JI_API const JiComponentDef* ji_jiml_document_find_component(
    const JiJimlDocument* doc, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_JIML_H */
