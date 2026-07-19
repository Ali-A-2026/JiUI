/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_codegen.c
 * @brief Implementation of the ahead-of-time code generator.
 *
 * Walks a parsed AST and emits C source code that constructs the equivalent
 * object tree at runtime without any XML parsing overhead.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_codegen.h"
#include "jiui/ji_xml_parser.h"
#include "jiui/ji_binding.h"
#include "jiui/ji_event.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/* =========================================================================
 * Internal: code generation context
 * ========================================================================= */
typedef struct JiCodeGenCtx {
    FILE*       output;
    const char* class_name;
    int         var_counter;     /* unique variable counter */
    int         indent;          /* current indentation level */
    int         object_count;
    int         binding_count;
    int         event_count;
    bool        has_error;
    char        error_msg[512];
} JiCodeGenCtx;

/* =========================================================================
 * Internal: set error in context
 * ========================================================================= */
static void codegen_error(JiCodeGenCtx* ctx, const char* fmt, ...) {
    ctx->has_error = true;
    va_list args;
    va_start(args, fmt);
    vsnprintf(ctx->error_msg, sizeof(ctx->error_msg), fmt, args);
    va_end(args);
}

/* =========================================================================
 * Internal: emit indented line
 * ========================================================================= */
static void emit_indent(JiCodeGenCtx* ctx) {
    for (int i = 0; i < ctx->indent; i++) {
        fputs("    ", ctx->output);
    }
}

static void emit_line(JiCodeGenCtx* ctx, const char* fmt, ...) {
    emit_indent(ctx);
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->output, fmt, args);
    va_end(args);
    fputc('\n', ctx->output);
}

/* =========================================================================
 * Internal: sanitize a name for use as a C identifier
 * ========================================================================= */
static void sanitize_identifier(const char* name, char* buf, size_t buf_size) {
    size_t j = 0;
    for (size_t i = 0; name[i] && j < buf_size - 1; i++) {
        if (isalnum((unsigned char)name[i]) || name[i] == '_') {
            buf[j++] = name[i];
        } else if (name[i] == '.' || name[i] == ':') {
            buf[j++] = '_';
        }
    }
    buf[j] = '\0';
}

/* =========================================================================
 * Internal: known event attribute names (must match ji_loader.c)
 * ========================================================================= */
static const char* g_event_names[] = {
    "Click", "DoubleClick", "Pressed", "Released",
    "TextChanged", "SelectionChanged", "GotFocus", "LostFocus",
    "PointerEnter", "PointerLeave", "PointerMoved",
    "KeyDown", "KeyUp", "ScrollChanged",
    "Loaded", "Unloaded", NULL
};

static bool is_event_name(const char* name) {
    for (int i = 0; g_event_names[i] != NULL; i++) {
        if (strcmp(name, g_event_names[i]) == 0) return true;
    }
    return false;
}

/* =========================================================================
 * Internal: emit a C literal for an AST value
 * ========================================================================= */
static void emit_value_literal(FILE* out, const JiAstNode* val) {
    if (!val) {
        fputs("NULL", out);
        return;
    }
    switch (val->type) {
        case JI_AST_STRING_VAL:
            if (val->v.string_val) {
                fputc('"', out);
                /* Escape special characters */
                for (const char* p = val->v.string_val; *p; p++) {
                    switch (*p) {
                        case '"':  fputs("\\\"", out); break;
                        case '\\': fputs("\\\\", out); break;
                        case '\n': fputs("\\n", out); break;
                        case '\r': fputs("\\r", out); break;
                        case '\t': fputs("\\t", out); break;
                        default:   fputc(*p, out); break;
                    }
                }
                fputc('"', out);
            } else {
                fputs("NULL", out);
            }
            break;
        case JI_AST_INT_VAL:
            fprintf(out, "%lld", val->v.int_val);
            break;
        case JI_AST_FLOAT_VAL:
            fprintf(out, "%g", val->v.float_val);
            break;
        case JI_AST_BOOL_VAL:
            fputs(val->v.bool_val ? "true" : "false", out);
            break;
        case JI_AST_COLOR_VAL:
            fprintf(out, "0x%08X", (unsigned)val->v.color_val);
            break;
        case JI_AST_NULL_VAL:
            fputs("NULL", out);
            break;
        case JI_AST_ENUM_VAL:
            /* Enum values are stored as string "Type.Value" */
            if (val->v.string_val) {
                fprintf(out, "/* enum: %s */ 0", val->v.string_val);
            } else {
                fputs("0", out);
            }
            break;
        case JI_AST_GRID_LENGTH:
            fprintf(out, "%g", val->v.grid.amount);
            break;
        default:
            fputs("0", out);
            break;
    }
}

/* =========================================================================
 * Internal: collect event handler names from the AST (for extern decls)
 * ========================================================================= */
static void collect_event_handlers(const JiAstNode* ast, FILE* out, int* count) {
    if (!ast) return;

    for (int i = 0; i < ast->attr_count; i++) {
        const JiAstNode* attr = ast->attributes[i];
        if (attr->type != JI_AST_ATTRIBUTE || !attr->attr_name) continue;
        if (!is_event_name(attr->attr_name)) continue;
        if (!attr->attr_value || attr->attr_value->type != JI_AST_STRING_VAL) continue;
        const char* handler = attr->attr_value->v.string_val;
        if (!handler) continue;

        fprintf(out, "extern void %s(JiObject* sender, void* args, void* user_data);\n", handler);
        (*count)++;
    }

    if (ast->type == JI_AST_OBJECT) {
        for (int i = 0; i < ast->child_count; i++) {
            collect_event_handlers(ast->children[i], out, count);
        }
    }
}

/* =========================================================================
 * Internal: generate code for a single object node (recursive)
 *
 * Emits code that creates the object, sets properties, registers bindings,
 * subscribes events, and adds children.
 *
 * Returns the C variable name (allocated string, caller must free).
 * ========================================================================= */
static char* gen_object(JiCodeGenCtx* ctx, const JiAstNode* node) {
    if (!node || node->type != JI_AST_OBJECT) {
        codegen_error(ctx, "Expected OBJECT node");
        return NULL;
    }

    const char* type_name = node->type_name ? node->type_name : "Object";

    /* Generate a unique variable name */
    char var_name[64];
    snprintf(var_name, sizeof(var_name), "obj_%d", ctx->var_counter++);

    /* Create the object */
    emit_line(ctx, "/* Create %s */", type_name);
    emit_line(ctx, "JiObject* %s = ji_object_create();", var_name);
    ctx->object_count++;

    /* Set element name (x:Name) */
    if (node->element_name) {
        emit_line(ctx, "ji_object_set_name(%s, \"%s\");", var_name, node->element_name);
    }

    /* Process attributes: properties, bindings, events */
    for (int i = 0; i < node->attr_count; i++) {
        const JiAstNode* attr = node->attributes[i];
        if (attr->type != JI_AST_ATTRIBUTE || !attr->attr_name) continue;

        /* Skip x:Name — handled above */
        if (strcmp(attr->attr_name, "x:Name") == 0) continue;

        /* Check for binding */
        if (attr->attr_value && attr->attr_value->type == JI_AST_BINDING) {
            const char* path = attr->attr_value->v.binding.path;
            const char* mode_str = attr->attr_value->v.binding.mode;
            const char* mode_enum = "JI_BINDING_ONE_WAY";
            if (mode_str) {
                JiBindingMode mode = ji_binding_mode_from_str(mode_str);
                switch (mode) {
                    case JI_BINDING_TWO_WAY:   mode_enum = "JI_BINDING_TWO_WAY"; break;
                    case JI_BINDING_ONE_WAY_TO_SOURCE: mode_enum = "JI_BINDING_ONE_WAY_TO_SOURCE"; break;
                    default: break;
                }
            }
            emit_line(ctx, "/* Binding: %s → {Binding %s} */", attr->attr_name,
                      path ? path : "");
            emit_line(ctx, "ji_binding_engine_add_path(&bindings, %s, "
                      "ji_property_from_name(\"%s\", ji_object_type_id(%s)), "
                      "\"%s\", %s);",
                      var_name, attr->attr_name, var_name,
                      path ? path : "", mode_enum);
            ctx->binding_count++;
            continue;
        }

        /* Check for resource reference */
        if (attr->attr_value && attr->attr_value->type == JI_AST_RESOURCE) {
            const char* res_name = attr->attr_value->v.resource_name;
            emit_line(ctx, "/* Resource: %s → {StaticResource %s} */",
                      attr->attr_name, res_name ? res_name : "");
            emit_line(ctx, "{");
            ctx->indent++;
            emit_line(ctx, "const char* _res_val = ji_resource_dict_get_string(resources, \"%s\");",
                      res_name ? res_name : "");
            emit_line(ctx, "if (_res_val) {");
            ctx->indent++;
            emit_line(ctx, "JiPropertyId _pid = ji_property_from_name(\"%s\", ji_object_type_id(%s));",
                      attr->attr_name, var_name);
            emit_line(ctx, "if (_pid != JI_PROPERTY_INVALID) {");
            ctx->indent++;
            emit_line(ctx, "JiPropertyValue _pv;");
            emit_line(ctx, "_pv.type = JI_PROP_TYPE_STRING;");
            emit_line(ctx, "_pv.v.string_val = (char*)_res_val;");
            emit_line(ctx, "ji_object_set_property(%s, _pid, _pv);", var_name);
            ctx->indent--;
            emit_line(ctx, "}");
            ctx->indent--;
            emit_line(ctx, "}");
            ctx->indent--;
            emit_line(ctx, "}");
            continue;
        }

        /* Check for event */
        if (is_event_name(attr->attr_name)) {
            if (attr->attr_value && attr->attr_value->type == JI_AST_STRING_VAL &&
                attr->attr_value->v.string_val) {
                const char* handler = attr->attr_value->v.string_val;
                emit_line(ctx, "ji_event_bus_subscribe(&events, %s, \"%s\", %s, NULL);",
                          var_name, attr->attr_name, handler);
                ctx->event_count++;
            }
            continue;
        }

        /* Regular property — emit set_property call */
        if (attr->attr_value) {
            char val_buf[256];
            val_buf[0] = '\0';

            switch (attr->attr_value->type) {
                case JI_AST_STRING_VAL:
                    snprintf(val_buf, sizeof(val_buf), "\"%s\"",
                             attr->attr_value->v.string_val ? attr->attr_value->v.string_val : "");
                    break;
                case JI_AST_INT_VAL:
                    snprintf(val_buf, sizeof(val_buf), "%lld", attr->attr_value->v.int_val);
                    break;
                case JI_AST_FLOAT_VAL:
                    snprintf(val_buf, sizeof(val_buf), "%g", attr->attr_value->v.float_val);
                    break;
                case JI_AST_BOOL_VAL:
                    snprintf(val_buf, sizeof(val_buf), "%s",
                             attr->attr_value->v.bool_val ? "true" : "false");
                    break;
                case JI_AST_COLOR_VAL:
                    snprintf(val_buf, sizeof(val_buf), "0x%08X",
                             (unsigned)attr->attr_value->v.color_val);
                    break;
                case JI_AST_NULL_VAL:
                    snprintf(val_buf, sizeof(val_buf), "0");
                    break;
                default:
                    snprintf(val_buf, sizeof(val_buf), "0 /* unsupported value type %d */",
                             attr->attr_value->type);
                    break;
            }

            emit_line(ctx, "{");
            ctx->indent++;
            emit_line(ctx, "JiPropertyId _pid = ji_property_from_name(\"%s\", ji_object_type_id(%s));",
                      attr->attr_name, var_name);
            emit_line(ctx, "if (_pid != JI_PROPERTY_INVALID) {");
            ctx->indent++;
            emit_line(ctx, "JiPropertyValue _pv;");

            switch (attr->attr_value->type) {
                case JI_AST_STRING_VAL:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_STRING;");
                    emit_line(ctx, "_pv.v.string_val = %s;", val_buf);
                    break;
                case JI_AST_INT_VAL:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_INT;");
                    emit_line(ctx, "_pv.v.int_val = %s;", val_buf);
                    break;
                case JI_AST_FLOAT_VAL:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_DOUBLE;");
                    emit_line(ctx, "_pv.v.double_val = %s;", val_buf);
                    break;
                case JI_AST_BOOL_VAL:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_BOOL;");
                    emit_line(ctx, "_pv.v.bool_val = %s;", val_buf);
                    break;
                case JI_AST_COLOR_VAL:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_COLOR;");
                    emit_line(ctx, "_pv.v.color_val = %s;", val_buf);
                    break;
                default:
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_INT; _pv.v.int_val = 0;");
                    break;
            }

            emit_line(ctx, "ji_object_set_property(%s, _pid, _pv);", var_name);
            ctx->indent--;
            emit_line(ctx, "}");
            ctx->indent--;
            emit_line(ctx, "}");
        }
    }

    /* Process children */
    for (int i = 0; i < node->child_count; i++) {
        const JiAstNode* child = node->children[i];

        switch (child->type) {
            case JI_AST_OBJECT: {
                /* Check for special object types */
                if (child->type_name) {
                    /* Skip ResourceDictionary children (handled at resource level) */
                    if (strcmp(child->type_name, "ResourceDictionary") == 0) {
                        /* Emit resource population */
                        for (int j = 0; j < child->child_count; j++) {
                            const JiAstNode* res = child->children[j];
                            if (res->type == JI_AST_OBJECT) {
                                const char* key = res->element_name ? res->element_name : "unnamed";
                                /* Look for x:Key */
                                for (int k = 0; k < res->attr_count; k++) {
                                    if (res->attributes[k]->type == JI_AST_ATTRIBUTE &&
                                        res->attributes[k]->attr_name &&
                                        strcmp(res->attributes[k]->attr_name, "x:Key") == 0 &&
                                        res->attributes[k]->attr_value &&
                                        res->attributes[k]->attr_value->type == JI_AST_STRING_VAL) {
                                        key = res->attributes[k]->attr_value->v.string_val;
                                        break;
                                    }
                                }
                                if (res->type_name) {
                                    emit_line(ctx, "ji_resource_dict_set_string(resources, \"%s\", \"%s\");",
                                              key, res->type_name);
                                }
                            }
                        }
                        continue;
                    }

                    /* Skip Style children (handled separately) */
                    if (strcmp(child->type_name, "Style") == 0) {
                        /* Styles are complex — emit a comment for now */
                        emit_line(ctx, "/* TODO: Style generation */");
                        continue;
                    }
                }

                /* Regular child object */
                ctx->indent++;
                char* child_var = gen_object(ctx, child);
                if (child_var) {
                    emit_line(ctx, "ji_object_add_child(%s, %s);", var_name, child_var);
                    ji_free(child_var);
                }
                ctx->indent--;
                break;
            }

            case JI_AST_PROPERTY:
            case JI_AST_ATTACHED_PROP: {
                const char* prop_name = child->prop_name;
                const char* owner = child->prop_owner;
                if (prop_name) {
                    char full_name[256];
                    if (owner) {
                        snprintf(full_name, sizeof(full_name), "%s.%s", owner, prop_name);
                    } else {
                        snprintf(full_name, sizeof(full_name), "%s", prop_name);
                    }
                    if (child->value) {
                        emit_line(ctx, "/* Property element: %s */", full_name);
                        emit_line(ctx, "{");
                        ctx->indent++;
                        emit_line(ctx, "JiPropertyId _pid = ji_property_from_name(\"%s\", ji_object_type_id(%s));",
                                  full_name, var_name);
                        emit_line(ctx, "if (_pid != JI_PROPERTY_INVALID) {");
                        ctx->indent++;
                        emit_line(ctx, "JiPropertyValue _pv;");

                        switch (child->value->type) {
                            case JI_AST_STRING_VAL:
                                emit_line(ctx, "_pv.type = JI_PROP_TYPE_STRING;");
                                if (child->value->v.string_val) {
                                    emit_line(ctx, "_pv.v.string_val = \"%s\";", child->value->v.string_val);
                                } else {
                                    emit_line(ctx, "_pv.v.string_val = NULL;");
                                }
                                break;
                            case JI_AST_INT_VAL:
                                emit_line(ctx, "_pv.type = JI_PROP_TYPE_INT;");
                                emit_line(ctx, "_pv.v.int_val = %lld;", child->value->v.int_val);
                                break;
                            case JI_AST_FLOAT_VAL:
                                emit_line(ctx, "_pv.type = JI_PROP_TYPE_DOUBLE;");
                                emit_line(ctx, "_pv.v.double_val = %g;", child->value->v.float_val);
                                break;
                            case JI_AST_BOOL_VAL:
                                emit_line(ctx, "_pv.type = JI_PROP_TYPE_BOOL;");
                                emit_line(ctx, "_pv.v.bool_val = %s;", child->value->v.bool_val ? "true" : "false");
                                break;
                            default:
                                emit_line(ctx, "_pv.type = JI_PROP_TYPE_INT; _pv.v.int_val = 0;");
                                break;
                        }

                        emit_line(ctx, "ji_object_set_property(%s, _pid, _pv);", var_name);
                        ctx->indent--;
                        emit_line(ctx, "}");
                        ctx->indent--;
                        emit_line(ctx, "}");
                    }
                }
                break;
            }

            case JI_AST_TEXT_CONTENT: {
                if (child->text_content && child->text_content[0]) {
                    emit_line(ctx, "/* Text content */");
                    emit_line(ctx, "{");
                    ctx->indent++;
                    emit_line(ctx, "JiPropertyId _pid = ji_property_from_name(\"Content\", ji_object_type_id(%s));",
                              var_name);
                    emit_line(ctx, "if (_pid != JI_PROPERTY_INVALID) {");
                    ctx->indent++;
                    emit_line(ctx, "JiPropertyValue _pv;");
                    emit_line(ctx, "_pv.type = JI_PROP_TYPE_STRING;");
                    emit_line(ctx, "_pv.v.string_val = \"%s\";", child->text_content);
                    emit_line(ctx, "ji_object_set_property(%s, _pid, _pv);", var_name);
                    ctx->indent--;
                    emit_line(ctx, "}");
                    ctx->indent--;
                    emit_line(ctx, "}");
                }
                break;
            }

            default:
                break;
        }
    }

    /* Return the variable name */
    return strdup(var_name);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

JI_API JiCodeGenResult ji_codegen_generate(const JiAstNode* ast, FILE* output,
                                              const char* class_name) {
    JiCodeGenResult result;
    memset(&result, 0, sizeof(result));

    if (!ast || !output || !class_name) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Invalid arguments: ast=%p, output=%p, class_name=%p",
                 (const void*)ast, (void*)output, (const void*)class_name);
        return result;
    }

    JiCodeGenCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.output = output;
    ctx.class_name = class_name;
    ctx.var_counter = 0;
    ctx.indent = 0;

    /* Sanitize class name for C identifier */
    char safe_name[128];
    sanitize_identifier(class_name, safe_name, sizeof(safe_name));

    /* Emit file header */
    fprintf(output, "/* =========================================================================\n");
    fprintf(output, " * Auto-generated by jigen from %s.ji\n", class_name);
    fprintf(output, " * DO NOT EDIT — changes will be overwritten\n");
    fprintf(output, " * ========================================================================= */\n\n");
    fprintf(output, "#include <jiui/jiui.h>\n");
    fprintf(output, "#include <jiui/ji_loader.h>\n");
    fprintf(output, "#include <jiui/ji_binding.h>\n");
    fprintf(output, "#include <jiui/ji_event.h>\n");
    fprintf(output, "#include <jiui/ji_style.h>\n");
    fprintf(output, "#include <jiui/ji_property.h>\n");
    fprintf(output, "#include <string.h>\n\n");

    /* Emit extern declarations for event handlers */
    int event_decl_count = 0;
    collect_event_handlers(ast, output, &event_decl_count);
    if (event_decl_count > 0) {
        fprintf(output, "\n");
    }

    /* Emit the build function */
    fprintf(output, "JiObject* ji_build_%s(void) {\n", safe_name);
    ctx.indent = 1;

    /* Initialize binding engine and event bus */
    emit_line(&ctx, "JiBindingEngine bindings;");
    emit_line(&ctx, "ji_binding_engine_init(&bindings);");
    emit_line(&ctx, "");
    emit_line(&ctx, "JiEventBus events;");
    emit_line(&ctx, "ji_event_bus_init(&events);");
    emit_line(&ctx, "");
    emit_line(&ctx, "JiResourceDictionary* resources = ji_resource_dict_new();");
    emit_line(&ctx, "");

    /* Generate the object tree */
    char* root_var = gen_object(&ctx, ast);

    if (!root_var || ctx.has_error) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg), "%s",
                 ctx.error_msg[0] ? ctx.error_msg : "Code generation failed");
        if (root_var) ji_free(root_var);
        fprintf(output, "    return NULL;\n}\n");
        return result;
    }

    emit_line(&ctx, "");
    emit_line(&ctx, "/* Return the root object */");
    emit_line(&ctx, "return %s;", root_var);
    ctx.indent = 0;
    fprintf(output, "}\n\n");

    /* Emit a convenience load function */
    fprintf(output, "JiLoadResult ji_load_%s(void) {\n", safe_name);
    fprintf(output, "    JiLoadResult result;\n");
    fprintf(output, "    memset(&result, 0, sizeof(result));\n");
    fprintf(output, "    ji_binding_engine_init(&result.bindings);\n");
    fprintf(output, "    ji_event_bus_init(&result.events);\n");
    fprintf(output, "    result.resources = ji_resource_dict_new();\n");
    fprintf(output, "    result.root = ji_build_%s();\n", safe_name);
    fprintf(output, "    return result;\n");
    fprintf(output, "}\n");

    ji_free(root_var);

    result.object_count = ctx.object_count;
    result.binding_count = ctx.binding_count;
    result.event_count = ctx.event_count;
    return result;
}

JI_API JiCodeGenResult ji_codegen_from_file(const char* input_path, FILE* output,
                                               const char* class_name) {
    JiCodeGenResult result;
    memset(&result, 0, sizeof(result));

    if (!input_path || !output || !class_name) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg), "Invalid arguments");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_file(input_path);
    if (!ast) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg),
                 "Failed to parse XML file: %s", input_path);
        return result;
    }

    result = ji_codegen_generate(ast, output, class_name);
    ji_ast_destroy(ast);
    return result;
}

JI_API JiCodeGenResult ji_codegen_from_string(const char* source, FILE* output,
                                                 const char* class_name) {
    JiCodeGenResult result;
    memset(&result, 0, sizeof(result));

    if (!source || !output || !class_name) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg), "Invalid arguments");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_string(source);
    if (!ast) {
        result.has_error = true;
        snprintf(result.error_msg, sizeof(result.error_msg), "Failed to parse XML");
        return result;
    }

    result = ji_codegen_generate(ast, output, class_name);
    ji_ast_destroy(ast);
    return result;
}
