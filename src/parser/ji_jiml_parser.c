/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_jiml_parser.c
 * @brief JiML parser — parses .jiml markup into a JiJimlDocument.
 *
 * JiML is XML-based, so we reuse the existing XML parser (ji_xml_parser)
 * to produce the base AST, then post-process it to extract:
 *   - Style blocks (<Style>...</Style>)
 *   - Component definitions (<Component name="...">...</Component>)
 *   - Binding expressions ({variable}, {a + b}, etc.)
 *   - Event handlers (on-click="handler")
 *   - Conditional rendering (if="{condition}")
 *   - List rendering (each="{items}" as="item")
 */

#include "jiui/ji_api.h"
#include "jiui/ji_jiml.h"
#include "jiui/ji_xml_parser.h"
#include "jiui/ji_ast.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
static void jiml_extract_styles(JiJimlDocument* doc, JiAstNode* root);
static void jiml_extract_components(JiJimlDocument* doc, JiAstNode* root);
static JiAstNode* jiml_find_widget_root(JiAstNode* root);
static void jiml_set_error(JiJimlDocument* doc, const char* msg, int line);

/* =========================================================================
 * Binding Expression Parser
 * ========================================================================= */

/* Skip whitespace */
static const char* skip_ws(const char* s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

/* Parse an identifier (variable name, function name) */
static const char* parse_identifier(const char* s, char* out, size_t buf_size) {
    size_t i = 0;
    while (*s && (isalnum((unsigned char)*s) || *s == '_' || *s == '.')) {
        if (i < buf_size - 1) out[i++] = *s;
        s++;
    }
    out[i] = '\0';
    return s;
}

/* Parse a number literal */
static const char* parse_number(const char* s, double* out) {
    char* end;
    *out = strtod(s, &end);
    return end;
}

/* Forward declaration for recursive parsing */
static JiBindingExpr* parse_expr(const char** p);

/* Parse a primary expression (literal, variable, or parenthesized expr) */
static JiBindingExpr* parse_primary(const char** p) {
    const char* s = skip_ws(*p);
    if (!*s) return NULL;

    JiBindingExpr* e = ji_calloc(1, sizeof(JiBindingExpr));
    if (!e) return NULL;

    /* Parenthesized expression */
    if (*s == '(') {
        s++;
        JiBindingExpr* inner = parse_expr(&s);
        s = skip_ws(s);
        if (*s == ')') s++;
        ji_free(e);
        *p = s;
        return inner;
    }

    /* Unary not */
    if (*s == '!') {
        s++;
        e->type = JI_BINDING_EXPR_UNARY;
        e->op = JI_BINDING_OP_NOT;
        e->left = parse_primary(&s);
        *p = s;
        return e;
    }

    /* Unary minus */
    if (*s == '-') {
        s++;
        /* Check if it's a negative number */
        if (isdigit((unsigned char)*s) || *s == '.') {
            e->is_literal = true;
            e->literal_num = -strtod(s, (char**)&s);
            *p = s;
            return e;
        }
        e->type = JI_BINDING_EXPR_UNARY;
        e->op = JI_BINDING_OP_NEG;
        e->left = parse_primary(&s);
        *p = s;
        return e;
    }

    /* Numeric literal */
    if (isdigit((unsigned char)*s) || *s == '.') {
        e->is_literal = true;
        s = parse_number(s, &e->literal_num);
        *p = s;
        return e;
    }

    /* Boolean literal: true/false */
    if (strncmp(s, "true", 4) == 0 && !isalnum((unsigned char)s[4]) && s[4] != '_') {
        e->is_literal = true;
        e->is_bool = true;
        e->literal_num = 1.0;
        s += 4;
        *p = s;
        return e;
    }
    if (strncmp(s, "false", 5) == 0 && !isalnum((unsigned char)s[5]) && s[5] != '_') {
        e->is_literal = true;
        e->is_bool = true;
        e->literal_num = 0.0;
        s += 5;
        *p = s;
        return e;
    }

    /* String literal */
    if (*s == '"' || *s == '\'') {
        char quote = *s++;
        size_t i = 0;
        while (*s && *s != quote && i < sizeof(e->literal_str) - 1) {
            e->literal_str[i++] = *s++;
        }
        e->literal_str[i] = '\0';
        if (*s == quote) s++;
        e->is_literal = true;
        *p = s;
        return e;
    }

    /* Identifier or function call */
    if (isalpha((unsigned char)*s) || *s == '_') {
        char name[128];
        const char* after = parse_identifier(s, name, sizeof(name));

        /* Check for function call */
        const char* ws_after = skip_ws(after);
        if (*ws_after == '(') {
            e->type = JI_BINDING_EXPR_FUNCTION;
            strncpy(e->func_name, name, sizeof(e->func_name) - 1);
            s = ws_after + 1;
            /* Parse arguments */
            int arg_cap = 4;
            e->args = ji_calloc(arg_cap, sizeof(JiBindingExpr*));
            e->arg_count = 0;
            s = skip_ws(s);
            while (*s && *s != ')') {
                JiBindingExpr* arg = parse_expr(&s);
                if (arg) {
                    if (e->arg_count >= arg_cap) {
                        arg_cap *= 2;
                        e->args = ji_realloc(e->args, arg_cap * sizeof(JiBindingExpr*));
                    }
                    e->args[e->arg_count++] = arg;
                }
                s = skip_ws(s);
                if (*s == ',') s++;
                s = skip_ws(s);
            }
            if (*s == ')') s++;
            *p = s;
            return e;
        }

        /* Simple variable or path */
        e->type = (strchr(name, '.') != NULL) ? JI_BINDING_EXPR_PATH : JI_BINDING_EXPR_SIMPLE;
        strncpy(e->var_name, name, sizeof(e->var_name) - 1);
        *p = after;
        return e;
    }

    ji_free(e);
    return NULL;
}

/* Parse a binary operator */
static JiBindingOp parse_binop(const char** p) {
    const char* s = skip_ws(*p);
    JiBindingOp op = JI_BINDING_OP_NONE;

    if (strncmp(s, "==", 2) == 0) { op = JI_BINDING_OP_EQ; s += 2; }
    else if (strncmp(s, "!=", 2) == 0) { op = JI_BINDING_OP_NE; s += 2; }
    else if (strncmp(s, "<=", 2) == 0) { op = JI_BINDING_OP_LE; s += 2; }
    else if (strncmp(s, ">=", 2) == 0) { op = JI_BINDING_OP_GE; s += 2; }
    else if (strncmp(s, "&&", 2) == 0) { op = JI_BINDING_OP_AND; s += 2; }
    else if (strncmp(s, "||", 2) == 0) { op = JI_BINDING_OP_OR; s += 2; }
    else if (*s == '+') { op = JI_BINDING_OP_ADD; s++; }
    else if (*s == '-') { op = JI_BINDING_OP_SUB; s++; }
    else if (*s == '*') { op = JI_BINDING_OP_MUL; s++; }
    else if (*s == '/') { op = JI_BINDING_OP_DIV; s++; }
    else if (*s == '%') { op = JI_BINDING_OP_MOD; s++; }
    else if (*s == '<') { op = JI_BINDING_OP_LT; s++; }
    else if (*s == '>') { op = JI_BINDING_OP_GT; s++; }

    *p = s;
    return op;
}

/* Operator precedence (higher = binds tighter) */
static int op_precedence(JiBindingOp op) {
    switch (op) {
        case JI_BINDING_OP_OR:  return 1;
        case JI_BINDING_OP_AND: return 2;
        case JI_BINDING_OP_EQ:  case JI_BINDING_OP_NE: return 3;
        case JI_BINDING_OP_LT:  case JI_BINDING_OP_LE:
        case JI_BINDING_OP_GT:  case JI_BINDING_OP_GE: return 4;
        case JI_BINDING_OP_ADD: case JI_BINDING_OP_SUB: return 5;
        case JI_BINDING_OP_MUL: case JI_BINDING_OP_DIV:
        case JI_BINDING_OP_MOD: return 6;
        default: return 0;
    }
}

/* Recursive descent: parse expression with precedence climbing */
static JiBindingExpr* parse_expr_prec(const char** p, int min_prec) {
    JiBindingExpr* left = parse_primary(p);
    if (!left) return NULL;

    for (;;) {
        const char* save = *p;
        JiBindingOp op = parse_binop(p);
        if (op == JI_BINDING_OP_NONE) break;

        int prec = op_precedence(op);
        if (prec < min_prec) {
            *p = save;
            break;
        }

        /* Check for ternary conditional */
        const char* after_op = skip_ws(*p);
        if (op == JI_BINDING_OP_EQ && *after_op == '?') {
            /* This shouldn't happen with ==, but handle ternary separately */
        }

        JiBindingExpr* right = parse_expr_prec(p, prec + 1);
        if (!right) {
            ji_binding_expr_destroy(right);
            break;
        }

        JiBindingExpr* bin = ji_calloc(1, sizeof(JiBindingExpr));
        if (!bin) { ji_binding_expr_destroy(right); break; }
        bin->type = JI_BINDING_EXPR_BINARY;
        bin->op = op;
        bin->left = left;
        bin->right = right;
        left = bin;
    }

    /* Check for ternary conditional: cond ? a : b */
    const char* save = *p;
    const char* s = skip_ws(*p);
    if (*s == '?') {
        s++;
        JiBindingExpr* true_expr = parse_expr(&s);
        s = skip_ws(s);
        if (*s == ':') {
            s++;
            JiBindingExpr* false_expr = parse_expr(&s);
            JiBindingExpr* cond = ji_calloc(1, sizeof(JiBindingExpr));
            if (cond) {
                cond->type = JI_BINDING_EXPR_CONDITIONAL;
                cond->condition = left;
                cond->left = true_expr;
                cond->right = false_expr;
                left = cond;
                *p = s;
                return left;
            }
        }
        ji_binding_expr_destroy(true_expr);
        *p = save;
    }

    return left;
}

static JiBindingExpr* parse_expr(const char** p) {
    return parse_expr_prec(p, 0);
}

JI_API JiBindingExpr* ji_binding_expr_parse(const char* expr) {
    if (!expr) return NULL;
    const char* p = expr;
    JiBindingExpr* result = parse_expr(&p);
    return result;
}

JI_API void ji_binding_expr_destroy(JiBindingExpr* expr) {
    if (!expr) return;
    ji_binding_expr_destroy(expr->left);
    ji_binding_expr_destroy(expr->right);
    ji_binding_expr_destroy(expr->condition);
    if (expr->args) {
        for (int i = 0; i < expr->arg_count; i++) {
            ji_binding_expr_destroy(expr->args[i]);
        }
        ji_free(expr->args);
    }
    ji_free(expr);
}

JI_API bool ji_binding_expr_eval(const JiBindingExpr* expr,
                                   const void* context,
                                   char* out_val, size_t buf_size) {
    if (!expr || !out_val || buf_size == 0) return false;
    out_val[0] = '\0';

    /* For now, we support literal evaluation and simple variable lookup.
     * Full evaluation with data context requires the binding engine. */

    if (expr->is_literal) {
        if (expr->is_bool) {
            snprintf(out_val, buf_size, "%s", expr->literal_num != 0.0 ? "true" : "false");
        } else if (expr->literal_str[0]) {
            snprintf(out_val, buf_size, "%s", expr->literal_str);
        } else {
            snprintf(out_val, buf_size, "%g", expr->literal_num);
        }
        return true;
    }

    if (expr->type == JI_BINDING_EXPR_SIMPLE || expr->type == JI_BINDING_EXPR_PATH) {
        /* Variable lookup from context — requires binding engine integration.
         * For now, return the variable name as a placeholder. */
        snprintf(out_val, buf_size, "{%s}", expr->var_name);
        return true;
    }

    if (expr->type == JI_BINDING_EXPR_BINARY) {
        char left_val[256], right_val[256];
        if (!ji_binding_expr_eval(expr->left, context, left_val, sizeof(left_val)))
            return false;
        if (!ji_binding_expr_eval(expr->right, context, right_val, sizeof(right_val)))
            return false;

        /* Try numeric evaluation */
        double lv = atof(left_val);
        double rv = atof(right_val);
        double result = 0;
        bool is_numeric = true;

        switch (expr->op) {
            case JI_BINDING_OP_ADD: result = lv + rv; break;
            case JI_BINDING_OP_SUB: result = lv - rv; break;
            case JI_BINDING_OP_MUL: result = lv * rv; break;
            case JI_BINDING_OP_DIV: result = rv != 0 ? lv / rv : 0; break;
            case JI_BINDING_OP_MOD: result = (int)lv % (int)rv; break;
            case JI_BINDING_OP_EQ:  result = (lv == rv) ? 1 : 0; break;
            case JI_BINDING_OP_NE:  result = (lv != rv) ? 1 : 0; break;
            case JI_BINDING_OP_LT:  result = (lv < rv) ? 1 : 0; break;
            case JI_BINDING_OP_LE:  result = (lv <= rv) ? 1 : 0; break;
            case JI_BINDING_OP_GT:  result = (lv > rv) ? 1 : 0; break;
            case JI_BINDING_OP_GE:  result = (lv >= rv) ? 1 : 0; break;
            case JI_BINDING_OP_AND: result = (lv != 0 && rv != 0) ? 1 : 0; break;
            case JI_BINDING_OP_OR:  result = (lv != 0 || rv != 0) ? 1 : 0; break;
            default: is_numeric = false; break;
        }

        if (is_numeric) {
            if (expr->op >= JI_BINDING_OP_EQ && expr->op <= JI_BINDING_OP_OR) {
                snprintf(out_val, buf_size, "%s", result != 0 ? "true" : "false");
            } else {
                snprintf(out_val, buf_size, "%g", result);
            }
            return true;
        }
    }

    if (expr->type == JI_BINDING_EXPR_UNARY) {
        char val[256];
        if (!ji_binding_expr_eval(expr->left, context, val, sizeof(val)))
            return false;
        double v = atof(val);
        if (expr->op == JI_BINDING_OP_NEG) {
            snprintf(out_val, buf_size, "%g", -v);
        } else if (expr->op == JI_BINDING_OP_NOT) {
            snprintf(out_val, buf_size, "%s", v == 0 ? "true" : "false");
        }
        return true;
    }

    if (expr->type == JI_BINDING_EXPR_CONDITIONAL) {
        char cond_val[256];
        if (!ji_binding_expr_eval(expr->condition, context, cond_val, sizeof(cond_val)))
            return false;
        bool cond = (atof(cond_val) != 0 || strcmp(cond_val, "true") == 0);
        return ji_binding_expr_eval(cond ? expr->left : expr->right,
                                      context, out_val, buf_size);
    }

    return false;
}

JI_API bool ji_is_binding_expr(const char* str) {
    if (!str) return false;
    /* Skip leading whitespace */
    while (*str && isspace((unsigned char)*str)) str++;
    if (*str != '{') return false;
    /* Find closing brace */
    const char* end = strchr(str, '}');
    return end != NULL;
}

JI_API bool ji_binding_expr_extract(const char* str, char* out, size_t buf_size) {
    if (!str || !out || buf_size == 0) return false;
    out[0] = '\0';

    /* Skip leading whitespace */
    while (*str && isspace((unsigned char)*str)) str++;
    if (*str != '{') return false;
    str++;

    /* Find closing brace */
    const char* end = strchr(str, '}');
    if (!end) return false;

    size_t len = end - str;
    if (len >= buf_size) len = buf_size - 1;

    /* Trim whitespace */
    while (len > 0 && isspace((unsigned char)*str)) { str++; len--; }
    while (len > 0 && isspace((unsigned char)str[len-1])) len--;

    memcpy(out, str, len);
    out[len] = '\0';
    return true;
}

/* =========================================================================
 * Style Block Implementation
 * ========================================================================= */

JI_API JiJimlStyleBlock* ji_style_block_new(void) {
    JiJimlStyleBlock* block = ji_calloc(1, sizeof(JiJimlStyleBlock));
    if (!block) return NULL;
    block->rule_capacity = 8;
    block->rules = ji_calloc(block->rule_capacity, sizeof(JiJimlStyleRule));
    block->rule_count = 0;
    return block;
}

JI_API void ji_style_block_destroy(JiJimlStyleBlock* block) {
    if (!block) return;
    for (int i = 0; i < block->rule_count; i++) {
        ji_free(block->rules[i].selector);
        for (int j = 0; j < block->rules[i].prop_count; j++) {
            ji_free(block->rules[i].prop_names[j]);
            ji_free(block->rules[i].prop_values[j]);
        }
        ji_free(block->rules[i].prop_names);
        ji_free(block->rules[i].prop_values);
    }
    ji_free(block->rules);
    ji_free(block);
}

JI_API int ji_style_block_add_rule(JiJimlStyleBlock* block, const char* selector) {
    if (!block || !selector) return -1;

    if (block->rule_count >= block->rule_capacity) {
        block->rule_capacity *= 2;
        block->rules = ji_realloc(block->rules,
                                    block->rule_capacity * sizeof(JiJimlStyleRule));
    }

    JiJimlStyleRule* rule = &block->rules[block->rule_count];
    memset(rule, 0, sizeof(JiJimlStyleRule));
    rule->selector = ji_alloc(strlen(selector) + 1);
    if (!rule->selector) return -1;
    strcpy(rule->selector, selector);
    rule->prop_capacity = 8;
    rule->prop_names = ji_calloc(rule->prop_capacity, sizeof(char*));
    rule->prop_values = ji_calloc(rule->prop_capacity, sizeof(char*));
    rule->prop_count = 0;

    return block->rule_count++;
}

JI_API bool ji_style_rule_add_prop(JiJimlStyleBlock* block, int rule_idx,
                                     const char* name, const char* value) {
    if (!block || !name || !value) return false;
    if (rule_idx < 0 || rule_idx >= block->rule_count) return false;

    JiJimlStyleRule* rule = &block->rules[rule_idx];
    if (rule->prop_count >= rule->prop_capacity) {
        rule->prop_capacity *= 2;
        rule->prop_names = ji_realloc(rule->prop_names, rule->prop_capacity * sizeof(char*));
        rule->prop_values = ji_realloc(rule->prop_values, rule->prop_capacity * sizeof(char*));
    }

    rule->prop_names[rule->prop_count] = ji_alloc(strlen(name) + 1);
    rule->prop_values[rule->prop_count] = ji_alloc(strlen(value) + 1);
    if (!rule->prop_names[rule->prop_count] || !rule->prop_values[rule->prop_count]) {
        ji_free(rule->prop_names[rule->prop_count]);
        ji_free(rule->prop_values[rule->prop_count]);
        return false;
    }
    strcpy(rule->prop_names[rule->prop_count], name);
    strcpy(rule->prop_values[rule->prop_count], value);
    rule->prop_count++;
    return true;
}

JI_API int ji_style_block_find_rules(const JiJimlStyleBlock* block,
                                       const char* type_name,
                                       int* out_indices, int max_count) {
    if (!block || !type_name || !out_indices) return 0;
    int found = 0;
    for (int i = 0; i < block->rule_count && found < max_count; i++) {
        /* Check if selector matches type_name (exact match or prefix with pseudo-class) */
        const char* sel = block->rules[i].selector;
        size_t type_len = strlen(type_name);
        if (strncmp(sel, type_name, type_len) == 0) {
            /* After the type name, there should be ':' (pseudo-class) or end of string */
            if (sel[type_len] == '\0' || sel[type_len] == ':') {
                out_indices[found++] = i;
            }
        }
    }
    return found;
}

/* =========================================================================
 * Component Definition Implementation
 * ========================================================================= */

JI_API JiComponentDef* ji_component_def_new(const char* name) {
    if (!name) return NULL;
    JiComponentDef* def = ji_calloc(1, sizeof(JiComponentDef));
    if (!def) return NULL;
    strncpy(def->name, name, sizeof(def->name) - 1);
    def->param_names = NULL;
    def->param_count = 0;
    def->body = NULL;
    return def;
}

JI_API void ji_component_def_destroy(JiComponentDef* def) {
    if (!def) return;
    if (def->param_names) {
        for (int i = 0; i < def->param_count; i++) {
            ji_free(def->param_names[i]);
        }
        ji_free(def->param_names);
    }
    /* Note: body is owned by the AST, don't destroy here */
    ji_free(def);
}

JI_API bool ji_component_def_add_param(JiComponentDef* def, const char* name) {
    if (!def || !name) return false;
    def->param_names = ji_realloc(def->param_names,
                                    (def->param_count + 1) * sizeof(char*));
    if (!def->param_names) return false;
    def->param_names[def->param_count] = ji_alloc(strlen(name) + 1);
    if (!def->param_names[def->param_count]) return false;
    strcpy(def->param_names[def->param_count], name);
    def->param_count++;
    return true;
}

JI_API const JiComponentDef* ji_jiml_document_find_component(
    const JiJimlDocument* doc, const char* name) {
    if (!doc || !name) return NULL;
    for (int i = 0; i < doc->component_count; i++) {
        if (strcmp(doc->components[i].name, name) == 0) {
            return &doc->components[i];
        }
    }
    return NULL;
}

/* =========================================================================
 * JiML Document Parser
 * ========================================================================= */

static void jiml_set_error(JiJimlDocument* doc, const char* msg, int line) {
    if (!doc) return;
    doc->has_error = true;
    doc->error_line = line;
    strncpy(doc->error_msg, msg, sizeof(doc->error_msg) - 1);
    doc->error_msg[sizeof(doc->error_msg) - 1] = '\0';
}

/* Check if an AST node is a <Style> element */
static bool is_style_node(const JiAstNode* node) {
    if (!node || node->type != JI_AST_OBJECT) return false;
    return node->type_name && strcmp(node->type_name, "Style") == 0;
}

/* Check if an AST node is a <Component> element */
static bool is_component_node(const JiAstNode* node) {
    if (!node || node->type != JI_AST_OBJECT) return false;
    return node->type_name && strcmp(node->type_name, "Component") == 0;
}

/* Extract style rules from a <Style> AST node */
static void jiml_parse_style_node(JiJimlStyleBlock* block, const JiAstNode* style_node) {
    if (!block || !style_node) return;

    /* Style node children are style rules (e.g. "JiButton { ... }") */
    for (int i = 0; i < style_node->child_count; i++) {
        const JiAstNode* child = style_node->children[i];
        if (!child || child->type != JI_AST_OBJECT) continue;

        /* The child's type_name is the selector */
        const char* selector = child->type_name;
        if (!selector) continue;

        int rule_idx = ji_style_block_add_rule(block, selector);
        if (rule_idx < 0) continue;

        /* Child's attributes are properties */
        for (int j = 0; j < child->attr_count; j++) {
            const JiAstNode* attr = child->attributes[j];
            if (!attr || !attr->attr_name || !attr->attr_value) continue;
            const char* val = NULL;
            if (attr->attr_value->type == JI_AST_STRING_VAL) {
                val = attr->attr_value->v.string_val;
            }
            if (val) {
                ji_style_rule_add_prop(block, rule_idx, attr->attr_name, val);
            }
        }

        /* Also check text content for CSS-like syntax */
        if (child->text_content) {
            /* Parse CSS-like "prop: value; prop2: value2;" */
            const char* p = child->text_content;
            while (*p) {
                /* Skip whitespace */
                while (*p && isspace((unsigned char)*p)) p++;
                if (!*p) break;

                /* Parse property name */
                char prop_name[128] = {0};
                int pi = 0;
                while (*p && *p != ':' && !isspace((unsigned char)*p) && pi < 127) {
                    prop_name[pi++] = *p++;
                }
                prop_name[pi] = '\0';

                /* Skip to ':' */
                while (*p && *p != ':') p++;
                if (*p == ':') p++;

                /* Skip whitespace */
                while (*p && isspace((unsigned char)*p)) p++;

                /* Parse value */
                char prop_val[256] = {0};
                int vi = 0;
                while (*p && *p != ';' && *p != '\n' && vi < 255) {
                    prop_val[vi++] = *p++;
                }
                prop_val[vi] = '\0';
                /* Trim trailing whitespace */
                while (vi > 0 && isspace((unsigned char)prop_val[vi-1])) {
                    prop_val[--vi] = '\0';
                }

                if (prop_name[0] && prop_val[0]) {
                    ji_style_rule_add_prop(block, rule_idx, prop_name, prop_val);
                }

                /* Skip ';' */
                if (*p == ';') p++;
            }
        }
    }
}

static void jiml_extract_styles(JiJimlDocument* doc, JiAstNode* root) {
    if (!doc || !root) return;

    /* Count style nodes */
    int style_count = 0;
    for (int i = 0; i < root->child_count; i++) {
        if (is_style_node(root->children[i])) style_count++;
    }

    if (style_count == 0) return;

    doc->styles = ji_calloc(style_count, sizeof(JiJimlStyleBlock*));
    doc->style_count = 0;

    for (int i = 0; i < root->child_count && doc->style_count < style_count; i++) {
        if (is_style_node(root->children[i])) {
            JiJimlStyleBlock* block = ji_style_block_new();
            if (block) {
                jiml_parse_style_node(block, root->children[i]);
                doc->styles[doc->style_count++] = block;
            }
        }
    }
}

static void jiml_extract_components(JiJimlDocument* doc, JiAstNode* root) {
    if (!doc || !root) return;

    /* Count component nodes */
    int comp_count = 0;
    for (int i = 0; i < root->child_count; i++) {
        if (is_component_node(root->children[i])) comp_count++;
    }

    if (comp_count == 0) return;

    doc->components = ji_calloc(comp_count, sizeof(JiComponentDef));
    doc->component_count = 0;

    for (int i = 0; i < root->child_count && doc->component_count < comp_count; i++) {
        JiAstNode* node = root->children[i];
        if (!is_component_node(node)) continue;

        /* Get component name from attributes */
        const char* name = NULL;
        for (int j = 0; j < node->attr_count; j++) {
            if (node->attributes[j] && node->attributes[j]->attr_name &&
                strcmp(node->attributes[j]->attr_name, "name") == 0 &&
                node->attributes[j]->attr_value &&
                node->attributes[j]->attr_value->type == JI_AST_STRING_VAL) {
                name = node->attributes[j]->attr_value->v.string_val;
                break;
            }
        }

        if (!name) continue;

        JiComponentDef* def = &doc->components[doc->component_count];
        memset(def, 0, sizeof(JiComponentDef));
        strncpy(def->name, name, sizeof(def->name) - 1);

        /* Extract parameters from "params" attribute (comma-separated) */
        for (int j = 0; j < node->attr_count; j++) {
            if (node->attributes[j] && node->attributes[j]->attr_name &&
                strcmp(node->attributes[j]->attr_name, "params") == 0 &&
                node->attributes[j]->attr_value &&
                node->attributes[j]->attr_value->type == JI_AST_STRING_VAL) {
                const char* params = node->attributes[j]->attr_value->v.string_val;
                /* Parse comma-separated param names */
                const char* p = params;
                while (*p) {
                    while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
                    if (!*p) break;
                    char pname[64] = {0};
                    int pi = 0;
                    while (*p && !isspace((unsigned char)*p) && *p != ',' && pi < 63) {
                        pname[pi++] = *p++;
                    }
                    pname[pi] = '\0';
                    if (pname[0]) {
                        ji_component_def_add_param(def, pname);
                    }
                }
                break;
            }
        }

        /* First child is the body */
        if (node->child_count > 0) {
            def->body = node->children[0];
        }

        doc->component_count++;
    }
}

/* Find the first non-Style, non-Component child — that's the widget root */
static JiAstNode* jiml_find_widget_root(JiAstNode* root) {
    if (!root) return NULL;
    for (int i = 0; i < root->child_count; i++) {
        JiAstNode* child = root->children[i];
        if (!child) continue;
        if (!is_style_node(child) && !is_component_node(child)) {
            return child;
        }
    }
    return NULL;
}

JI_API JiJimlDocument* ji_jiml_parse(const char* source) {
    if (!source) return NULL;

    JiJimlDocument* doc = ji_calloc(1, sizeof(JiJimlDocument));
    if (!doc) return NULL;

    /* Use the existing XML parser to produce the base AST */
    JiXmlParser parser;
    ji_xml_parser_init(&parser, source, strlen(source));
    JiAstNode* ast = ji_xml_parser_parse(&parser);

    if (ji_xml_parser_has_error(&parser)) {
        jiml_set_error(doc, ji_xml_parser_error_message(&parser), 0);
        ji_xml_parser_destroy(&parser);
        return doc;
    }

    ji_xml_parser_destroy(&parser);

    if (!ast) {
        jiml_set_error(doc, "Failed to parse JiML: no AST produced", 0);
        return doc;
    }

    /* Extract style blocks */
    jiml_extract_styles(doc, ast);

    /* Extract component definitions */
    jiml_extract_components(doc, ast);

    /* Find the widget root (first non-Style, non-Component child) */
    doc->root = jiml_find_widget_root(ast);

    if (!doc->root) {
        jiml_set_error(doc, "No widget root found in JiML document", 0);
    }

    /* Note: we keep the AST alive by storing root in doc.
     * The full AST is not freed here because doc->root points into it.
     * In a production implementation, we'd clone the root or manage
     * the full AST lifecycle. For now, the AST nodes are ref-counted
     * via the object system when compiled. */

    return doc;
}

JI_API JiJimlDocument* ji_jiml_parse_file(const char* filepath) {
    if (!filepath) return NULL;

    FILE* f = fopen(filepath, "r");
    if (!f) {
        JiJimlDocument* doc = ji_calloc(1, sizeof(JiJimlDocument));
        if (doc) {
            char msg[600];
            snprintf(msg, sizeof(msg), "Cannot open file: %s", filepath);
            jiml_set_error(doc, msg, 0);
        }
        return doc;
    }

    /* Read file contents */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        JiJimlDocument* doc = ji_calloc(1, sizeof(JiJimlDocument));
        if (doc) jiml_set_error(doc, "Empty file", 0);
        return doc;
    }

    char* source = ji_alloc(size + 1);
    if (!source) {
        fclose(f);
        return NULL;
    }
    size_t read = fread(source, 1, size, f);
    source[read] = '\0';
    fclose(f);

    JiJimlDocument* doc = ji_jiml_parse(source);
    ji_free(source);
    return doc;
}

JI_API void ji_jiml_document_destroy(JiJimlDocument* doc) {
    if (!doc) return;

    /* Destroy style blocks */
    if (doc->styles) {
        for (int i = 0; i < doc->style_count; i++) {
            ji_style_block_destroy(doc->styles[i]);
        }
        ji_free(doc->styles);
    }

    /* Destroy component definitions */
    if (doc->components) {
        for (int i = 0; i < doc->component_count; i++) {
            /* Don't destroy body (owned by AST) */
            if (doc->components[i].param_names) {
                for (int j = 0; j < doc->components[i].param_count; j++) {
                    ji_free(doc->components[i].param_names[j]);
                }
                ji_free(doc->components[i].param_names);
            }
        }
        ji_free(doc->components);
    }

    /* Note: doc->root points into the AST which is managed by the parser.
     * In a full implementation we'd manage AST lifecycle. For now, we
     * don't destroy the AST here. */

    ji_free(doc);
}
