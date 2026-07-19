/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_tree_builder.c
 * @brief Implementation of the tree builder — converts XML AST to JiObject tree.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_tree_builder.h"
#include "jiui/ji_xml_parser.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_type.h"
#include "jiui/ji_property.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* =========================================================================
 * Internal: set an error in the build result
 * ========================================================================= */
static void build_error(JiTreeBuildResult* result, const char* fmt, ...) {
    result->has_error = true;
    va_list args;
    va_start(args, fmt);
    vsnprintf(result->error_msg, sizeof(result->error_msg), fmt, args);
    va_end(args);
}

/* =========================================================================
 * Internal: convert an AST value node to a JiPropertyValue
 * ========================================================================= */
static bool ast_value_to_prop(const JiAstNode* node, JiPropertyValue* out) {
    if (!node || !out) return false;

    memset(out, 0, sizeof(JiPropertyValue));

    switch (node->type) {
        case JI_AST_STRING_VAL:
            out->type = JI_PROP_TYPE_STRING;
            out->v.string_val = node->v.string_val ? strdup(node->v.string_val) : NULL;
            return true;

        case JI_AST_INT_VAL:
            out->type = JI_PROP_TYPE_INT;
            out->v.int_val = (int)node->v.int_val;
            return true;

        case JI_AST_FLOAT_VAL:
            out->type = JI_PROP_TYPE_DOUBLE;
            out->v.double_val = node->v.float_val;
            return true;

        case JI_AST_BOOL_VAL:
            out->type = JI_PROP_TYPE_BOOL;
            out->v.bool_val = node->v.bool_val;
            return true;

        case JI_AST_NULL_VAL:
            out->type = JI_PROP_TYPE_PTR;
            out->v.ptr_val = NULL;
            return true;

        case JI_AST_COLOR_VAL:
            out->type = JI_PROP_TYPE_COLOR;
            out->v.color_val = (uint32_t)node->v.color_val;
            return true;

        case JI_AST_ENUM_VAL:
            out->type = JI_PROP_TYPE_ENUM;
            /* Store enum as int for now; the string is "Type.Value" */
            out->v.int_val = 0;
            return true;

        case JI_AST_BINDING:
            /* Bindings are deferred — store the path as a string marker */
            out->type = JI_PROP_TYPE_STRING;
            {
                char buf[256];
                snprintf(buf, sizeof(buf), "{Binding %s}",
                         node->v.binding.path ? node->v.binding.path : "");
                out->v.string_val = strdup(buf);
            }
            return true;

        case JI_AST_RESOURCE:
            out->type = JI_PROP_TYPE_STRING;
            {
                char buf[256];
                snprintf(buf, sizeof(buf), "{Resource %s}",
                         node->v.resource_name ? node->v.resource_name : "");
                out->v.string_val = strdup(buf);
            }
            return true;

        case JI_AST_REFERENCE:
            out->type = JI_PROP_TYPE_STRING;
            out->v.string_val = node->v.reference_name ? strdup(node->v.reference_name) : NULL;
            return true;

        case JI_AST_GRID_LENGTH:
            out->type = JI_PROP_TYPE_DOUBLE;
            out->v.double_val = node->v.grid.amount;
            return true;

        default:
            return false;
    }
}

/* =========================================================================
 * Internal: apply an attribute to an object
 * ========================================================================= */
static void apply_attribute(JiObject* obj, const JiAstNode* attr, JiTreeBuildResult* result) {
    if (!obj || !attr || attr->type != JI_AST_ATTRIBUTE) return;

    const char* name = attr->attr_name;
    const JiAstNode* val_node = attr->attr_value;

    if (!name) return;

    /* Look up the property by name */
    JiTypeId owner_type = ji_object_type_id(obj);
    JiPropertyId prop_id = ji_property_from_name(name, owner_type);

    if (prop_id != JI_PROPERTY_INVALID) {
        /* Known property — set it */
        JiPropertyValue prop_val;
        if (ast_value_to_prop(val_node, &prop_val)) {
            ji_object_set_property(obj, prop_id, prop_val);
            /* Free allocated string if the property system made its own copy */
            if (prop_val.type == JI_PROP_TYPE_STRING && prop_val.v.string_val) {
                ji_free(prop_val.v.string_val);
            }
        }
    } else {
        /* Unknown property — store as a dynamic string property using the
         * type name as a key. For now, we just skip unknown properties.
         * A full implementation would add them to a dynamic property map. */
    }
}

/* =========================================================================
 * Internal: build an object tree from an AST node (recursive)
 * ========================================================================= */
static JiObject* build_object(const JiAstNode* node, JiTreeBuildResult* result) {
    if (!node || node->type != JI_AST_OBJECT) {
        build_error(result, "Expected OBJECT node at line %d", node ? node->line : 0);
        return NULL;
    }

    /* Look up the type */
    const char* type_name = node->type_name ? node->type_name : "Object";
    JiTypeId type_id = ji_type_from_name(type_name);

    JiObject* obj;
    if (type_id != JI_TYPE_INVALID) {
        obj = ji_object_new(type_id);
    } else {
        /* Unknown type — create a base JiObject and store the type name */
        obj = ji_object_create();
        if (obj) {
            /* Store the intended type name as the object name prefix */
            /* This allows later resolution or dynamic type handling */
        }
    }

    if (!obj) {
        build_error(result, "Failed to create object of type '%s'", type_name);
        return NULL;
    }

    /* Set element name (x:Name) */
    if (node->element_name) {
        ji_object_set_name(obj, node->element_name);
    }

    /* Apply attributes */
    for (int i = 0; i < node->attr_count; i++) {
        if (node->attributes[i]->type == JI_AST_ATTRIBUTE) {
            apply_attribute(obj, node->attributes[i], result);
        }
    }

    /* Process children */
    for (int i = 0; i < node->child_count; i++) {
        const JiAstNode* child = node->children[i];

        switch (child->type) {
            case JI_AST_OBJECT: {
                JiObject* child_obj = build_object(child, result);
                if (child_obj) {
                    ji_object_add_child(obj, child_obj);
                }
                break;
            }

            case JI_AST_PROPERTY:
            case JI_AST_ATTACHED_PROP: {
                /* Property element — set as a property on the parent */
                const char* prop_name = child->prop_name;
                const char* owner = child->prop_owner;

                if (prop_name) {
                    char full_name[256];
                    if (owner) {
                        snprintf(full_name, sizeof(full_name), "%s.%s", owner, prop_name);
                    } else {
                        snprintf(full_name, sizeof(full_name), "%s", prop_name);
                    }

                    JiTypeId obj_type = ji_object_type_id(obj);
                    JiPropertyId prop_id = ji_property_from_name(full_name, obj_type);

                    if (prop_id != JI_PROPERTY_INVALID && child->value) {
                        JiPropertyValue prop_val;
                        if (ast_value_to_prop(child->value, &prop_val)) {
                            ji_object_set_property(obj, prop_id, prop_val);
                            if (prop_val.type == JI_PROP_TYPE_STRING && prop_val.v.string_val) {
                                ji_free(prop_val.v.string_val);
                            }
                        }
                    }
                }
                break;
            }

            case JI_AST_TEXT_CONTENT: {
                /* Text content — could be the Content property or similar */
                /* For now, try to set as "Content" property if it exists */
                if (child->text_content && child->text_content[0]) {
                    JiTypeId obj_type = ji_object_type_id(obj);
                    JiPropertyId prop_id = ji_property_from_name("Content", obj_type);
                    if (prop_id != JI_PROPERTY_INVALID) {
                        JiPropertyValue prop_val;
                        prop_val.type = JI_PROP_TYPE_STRING;
                        prop_val.v.string_val = strdup(child->text_content);
                        ji_object_set_property(obj, prop_id, prop_val);
                        ji_free(prop_val.v.string_val);
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    return obj;
}

/* =========================================================================
 * Public API
 * ========================================================================= */

JI_API JiTreeBuildResult ji_tree_build(const JiAstNode* ast) {
    JiTreeBuildResult result;
    memset(&result, 0, sizeof(result));

    if (!ast) {
        build_error(&result, "AST is NULL");
        return result;
    }

    result.root = build_object(ast, &result);
    if (!result.root && !result.has_error) {
        build_error(&result, "Failed to build object tree");
    }

    return result;
}

JI_API JiTreeBuildResult ji_tree_build_from_string(const char* source) {
    JiTreeBuildResult result;
    memset(&result, 0, sizeof(result));

    if (!source) {
        build_error(&result, "Source string is NULL");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_string(source);
    if (!ast) {
        build_error(&result, "Failed to parse XML");
        return result;
    }

    result = ji_tree_build(ast);
    ji_ast_destroy(ast);
    return result;
}

JI_API JiTreeBuildResult ji_tree_build_from_file(const char* filepath) {
    JiTreeBuildResult result;
    memset(&result, 0, sizeof(result));

    if (!filepath) {
        build_error(&result, "File path is NULL");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_file(filepath);
    if (!ast) {
        build_error(&result, "Failed to parse XML file: %s", filepath);
        return result;
    }

    result = ji_tree_build(ast);
    ji_ast_destroy(ast);
    return result;
}
