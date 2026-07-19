/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_loader.c
 * @brief Implementation of the runtime loader.
 */

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/ji_loader.h"
#include "jiui/ji_tree_builder.h"
#include "jiui/ji_xml_parser.h"
#include "jiui/ji_binding.h"
#include "jiui/ji_event.h"
#include "jiui/ji_style.h"
#include "jiui/ji_memory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* =========================================================================
 * Internal: set an error in the load result
 * ========================================================================= */
static void load_error(JiLoadResult* result, const char* fmt, ...) {
    result->has_error = true;
    va_list args;
    va_start(args, fmt);
    vsnprintf(result->error_msg, sizeof(result->error_msg), fmt, args);
    va_end(args);
}

/* =========================================================================
 * Internal: recursively find a named element in the object tree
 * ========================================================================= */
static JiObject* find_named_object(JiObject* obj, const char* name) {
    if (!obj || !name) return NULL;
    const char* obj_name = ji_object_name(obj);
    if (obj_name && strcmp(obj_name, name) == 0) {
        return obj;
    }
    for (int i = 0; i < ji_object_get_child_count(obj); i++) {
        JiObject* found = find_named_object(ji_object_get_child(obj, i), name);
        if (found) return found;
    }
    return NULL;
}

/* =========================================================================
 * Internal: extract resources from AST and populate the resource dictionary
 * ========================================================================= */
static void extract_resources(const JiAstNode* ast, JiResourceDictionary* dict) {
    if (!ast || !dict) return;

    if (ast->type == JI_AST_OBJECT && ast->type_name &&
        strcmp(ast->type_name, "ResourceDictionary") == 0) {
        for (int i = 0; i < ast->child_count; i++) {
            const JiAstNode* child = ast->children[i];
            if (child->type == JI_AST_OBJECT && child->element_name) {
                const char* key = child->element_name;
                for (int j = 0; j < child->attr_count; j++) {
                    if (child->attributes[j]->type == JI_AST_ATTRIBUTE &&
                        child->attributes[j]->attr_name &&
                        strcmp(child->attributes[j]->attr_name, "x:Key") == 0 &&
                        child->attributes[j]->attr_value &&
                        child->attributes[j]->attr_value->type == JI_AST_STRING_VAL &&
                        child->attributes[j]->attr_value->v.string_val) {
                        key = child->attributes[j]->attr_value->v.string_val;
                        break;
                    }
                }
                if (child->type_name) {
                    ji_resource_dict_set_string(dict, key, child->type_name);
                }
            }
        }
    }

    if (ast->type == JI_AST_OBJECT) {
        for (int i = 0; i < ast->child_count; i++) {
            extract_resources(ast->children[i], dict);
        }
    }
}

/* =========================================================================
 * Internal: extract bindings from AST with parallel Object tree walk
 *
 * Walks the AST and Object trees in parallel so that each AST node's
 * attributes are applied to the correct corresponding JiObject.
 * ========================================================================= */
static void extract_bindings(const JiAstNode* ast, JiObject* obj,
                               JiBindingEngine* engine) {
    if (!ast || !obj || !engine) return;

    /* Extract bindings from this node's attributes */
    for (int i = 0; i < ast->attr_count; i++) {
        const JiAstNode* attr = ast->attributes[i];
        if (attr->type != JI_AST_ATTRIBUTE || !attr->attr_value) continue;

        if (attr->attr_value->type == JI_AST_BINDING) {
            const char* path = attr->attr_value->v.binding.path;
            JiBindingMode mode = JI_BINDING_ONE_WAY;
            if (attr->attr_value->v.binding.mode) {
                mode = ji_binding_mode_from_str(attr->attr_value->v.binding.mode);
            }
            JiTypeId obj_type = ji_object_type_id(obj);
            JiPropertyId prop_id = ji_property_from_name(attr->attr_name, obj_type);
            if (prop_id != JI_PROPERTY_INVALID && path) {
                ji_binding_engine_add_path(engine, obj, prop_id, path, mode);
            }
        }
    }

    /* Walk children in parallel: match AST_OBJECT children to JiObject children */
    int obj_child_idx = 0;
    for (int i = 0; i < ast->child_count; i++) {
        const JiAstNode* child = ast->children[i];
        if (child->type == JI_AST_OBJECT) {
            if (obj_child_idx < ji_object_get_child_count(obj)) {
                JiObject* child_obj = ji_object_get_child(obj, obj_child_idx);
                extract_bindings(child, child_obj, engine);
                obj_child_idx++;
            }
        }
    }
}

/* =========================================================================
 * Internal: add a style to the load result
 * ========================================================================= */
static void add_style_to_result(JiLoadResult* result, JiStyle* style) {
    if (!result || !style) return;
    if (result->style_count >= result->style_capacity) {
        int new_cap = result->style_capacity == 0 ? 8 : result->style_capacity * 2;
        result->styles = (JiStyle**)ji_realloc(result->styles,
            (size_t)new_cap * sizeof(JiStyle*));
        result->style_capacity = new_cap;
    }
    result->styles[result->style_count++] = style;
}

/* =========================================================================
 * Internal: extract styles from AST
 * ========================================================================= */
static void extract_styles(const JiAstNode* ast, JiLoadResult* result) {
    if (!ast || !result) return;

    if (ast->type == JI_AST_OBJECT && ast->type_name &&
        strcmp(ast->type_name, "Style") == 0) {

        const char* selector_str = "Object";
        for (int i = 0; i < ast->attr_count; i++) {
            if (ast->attributes[i]->type == JI_AST_ATTRIBUTE &&
                ast->attributes[i]->attr_name &&
                strcmp(ast->attributes[i]->attr_name, "Selector") == 0 &&
                ast->attributes[i]->attr_value &&
                ast->attributes[i]->attr_value->type == JI_AST_STRING_VAL) {
                selector_str = ast->attributes[i]->attr_value->v.string_val;
                break;
            }
        }

        JiSelector sel = ji_style_parse_selector(selector_str);
        JiStyle* style = ji_style_new(sel);
        ji_selector_destroy(&sel);

        for (int i = 0; i < ast->child_count; i++) {
            const JiAstNode* child = ast->children[i];
            if (child->type == JI_AST_OBJECT && child->type_name &&
                strcmp(child->type_name, "Setter") == 0) {

                const char* prop = NULL;
                const char* val = NULL;
                char* val_buf = NULL; /* for numeric conversions */

                for (int j = 0; j < child->attr_count; j++) {
                    if (child->attributes[j]->type != JI_AST_ATTRIBUTE) continue;
                    const char* aname = child->attributes[j]->attr_name;
                    if (!aname) continue;

                    if (strcmp(aname, "Property") == 0 &&
                        child->attributes[j]->attr_value &&
                        child->attributes[j]->attr_value->type == JI_AST_STRING_VAL) {
                        prop = child->attributes[j]->attr_value->v.string_val;
                    } else if (strcmp(aname, "Value") == 0 &&
                               child->attributes[j]->attr_value) {
                        const JiAstNode* av = child->attributes[j]->attr_value;
                        switch (av->type) {
                            case JI_AST_STRING_VAL: val = av->v.string_val; break;
                            case JI_AST_INT_VAL:
                                val_buf = ji_alloc(32);
                                snprintf(val_buf, 32, "%lld", av->v.int_val);
                                val = val_buf;
                                break;
                            case JI_AST_FLOAT_VAL:
                                val_buf = ji_alloc(64);
                                snprintf(val_buf, 64, "%g", av->v.float_val);
                                val = val_buf;
                                break;
                            case JI_AST_BOOL_VAL:
                                val = av->v.bool_val ? "true" : "false";
                                break;
                            case JI_AST_COLOR_VAL:
                                val_buf = ji_alloc(16);
                                snprintf(val_buf, 16, "#%08X", (unsigned)av->v.color_val);
                                val = val_buf;
                                break;
                            default: break;
                        }
                    }
                }

                if (prop && val) {
                    JiSetter setter = ji_style_parse_setter(prop, val);
                    ji_style_add_setter(style, setter);
                }
                if (val_buf) ji_free(val_buf);
            }
        }

        add_style_to_result(result, style);
    }

    if (ast->type == JI_AST_OBJECT) {
        for (int i = 0; i < ast->child_count; i++) {
            extract_styles(ast->children[i], result);
        }
    }
}

/* =========================================================================
 * Internal: extract event handlers from AST with parallel Object tree walk
 *
 * Known event attribute names are matched against AST attributes, and
 * the handler name is looked up in the global handler registry.
 * The subscription is created on the correct per-node JiObject.
 * ========================================================================= */
static void extract_events(const JiAstNode* ast, JiObject* obj, JiEventBus* bus) {
    if (!ast || !obj || !bus) return;

    /* Known event attribute names */
    static const char* event_names[] = {
        "Click", "DoubleClick", "Pressed", "Released",
        "TextChanged", "SelectionChanged", "GotFocus", "LostFocus",
        "PointerEnter", "PointerLeave", "PointerMoved",
        "KeyDown", "KeyUp", "ScrollChanged",
        "Loaded", "Unloaded", NULL
    };

    /* Check this node's attributes for event bindings */
    for (int i = 0; i < ast->attr_count; i++) {
        const JiAstNode* attr = ast->attributes[i];
        if (attr->type != JI_AST_ATTRIBUTE || !attr->attr_name) continue;

        /* Check if this attribute name matches a known event */
        const char* event_name = NULL;
        for (int j = 0; event_names[j] != NULL; j++) {
            if (strcmp(attr->attr_name, event_names[j]) == 0) {
                event_name = event_names[j];
                break;
            }
        }
        if (!event_name) continue;

        /* Get the handler name from the value */
        if (!attr->attr_value || attr->attr_value->type != JI_AST_STRING_VAL) continue;
        const char* handler_name = attr->attr_value->v.string_val;
        if (!handler_name) continue;

        /* Look up the registered handler */
        JiEventHandler handler = ji_event_lookup_handler(handler_name);
        if (handler) {
            ji_event_bus_subscribe(bus, obj, event_name, handler, NULL);
        }
    }

    /* Walk children in parallel: match AST_OBJECT children to JiObject children */
    int obj_child_idx = 0;
    for (int i = 0; i < ast->child_count; i++) {
        const JiAstNode* child = ast->children[i];
        if (child->type == JI_AST_OBJECT) {
            if (obj_child_idx < ji_object_get_child_count(obj)) {
                JiObject* child_obj = ji_object_get_child(obj, obj_child_idx);
                extract_events(child, child_obj, bus);
                obj_child_idx++;
            }
        }
    }
}

/* =========================================================================
 * Internal: common load logic from a parsed AST
 * ========================================================================= */
static void load_from_ast(JiAstNode* ast, JiLoadResult* result) {
    JiTreeBuildResult build = ji_tree_build(ast);
    if (build.has_error || !build.root) {
        load_error(result, "Failed to build tree: %s", build.error_msg);
        ji_ast_destroy(ast);
        return;
    }

    result->root = build.root;
    extract_resources(ast, result->resources);
    extract_bindings(ast, result->root, &result->bindings);
    extract_styles(ast, result);
    extract_events(ast, result->root, &result->events);
    ji_ast_destroy(ast);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

JI_API JiLoadResult ji_load_file(const char* filepath) {
    JiLoadResult result;
    memset(&result, 0, sizeof(result));
    ji_binding_engine_init(&result.bindings);
    ji_event_bus_init(&result.events);
    result.resources = ji_resource_dict_new();

    if (!filepath) {
        load_error(&result, "File path is NULL");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_file(filepath);
    if (!ast) {
        load_error(&result, "Failed to parse XML file: %s", filepath);
        return result;
    }

    load_from_ast(ast, &result);
    return result;
}

JI_API JiLoadResult ji_load_string(const char* source) {
    JiLoadResult result;
    memset(&result, 0, sizeof(result));
    ji_binding_engine_init(&result.bindings);
    ji_event_bus_init(&result.events);
    result.resources = ji_resource_dict_new();

    if (!source) {
        load_error(&result, "Source string is NULL");
        return result;
    }

    JiAstNode* ast = ji_xml_parse_string(source);
    if (!ast) {
        load_error(&result, "Failed to parse XML");
        return result;
    }

    load_from_ast(ast, &result);
    return result;
}

JI_API void ji_load_result_destroy(JiLoadResult* result) {
    if (!result) return;

    ji_binding_engine_destroy(&result->bindings);
    ji_event_bus_destroy(&result->events);

    if (result->resources) {
        ji_resource_dict_destroy(result->resources);
        result->resources = NULL;
    }

    /* Destroy parsed styles */
    for (int i = 0; i < result->style_count; i++) {
        ji_style_destroy(result->styles[i]);
    }
    ji_free(result->styles);
    result->styles = NULL;
    result->style_count = 0;
    result->style_capacity = 0;

    /* Note: does NOT destroy result->root — caller must release it */
    result->root = NULL;
    result->has_error = false;
}

JI_API JiObject* ji_load_find_name(JiObject* root, const char* name) {
    return find_named_object(root, name);
}
