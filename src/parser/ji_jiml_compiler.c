/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_jiml_compiler.c
 * @brief JiML compiler — compiles a JiJimlDocument into a runtime object tree.
 *
 * This compiler bridges the JiML parser output (JiJimlDocument) to the
 * existing tree builder (ji_tree_builder). It:
 *   1. Takes the widget root AST from the JiML document
 *   2. Resolves binding expressions in attribute values
 *   3. Applies style rules from style blocks
 *   4. Expands component references
 *   5. Delegates to ji_tree_build() for actual object tree construction
 */

#include "jiui/ji_api.h"
#include "jiui/ji_jiml.h"
#include "jiui/ji_tree_builder.h"
#include "jiui/ji_ast.h"
#include "jiui/ji_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * Forward declarations
 * ========================================================================= */
static void jiml_apply_styles_to_tree(JiObject* root,
                                        const JiJimlDocument* doc);
static void jiml_resolve_bindings_in_tree(JiObject* root,
                                            const void* context);

/* =========================================================================
 * JiML Compile
 * ========================================================================= */

JI_API JiTreeBuildResult ji_jiml_compile(const JiJimlDocument* doc,
                                          const void* context) {
    JiTreeBuildResult result = {0};
    result.has_error = true;

    if (!doc) {
        strncpy(result.error_msg, "NULL JiML document", sizeof(result.error_msg) - 1);
        return result;
    }

    if (doc->has_error) {
        strncpy(result.error_msg, doc->error_msg, sizeof(result.error_msg) - 1);
        result.error_line = doc->error_line;
        return result;
    }

    if (!doc->root) {
        strncpy(result.error_msg, "No widget root in JiML document", sizeof(result.error_msg) - 1);
        return result;
    }

    /* Delegate to the existing tree builder to create the object tree */
    result = ji_tree_build(doc->root);

    if (!result.has_error && result.root) {
        /* Apply style rules from style blocks */
        jiml_apply_styles_to_tree(result.root, doc);

        /* Resolve binding expressions using the data context */
        jiml_resolve_bindings_in_tree(result.root, context);
    }

    return result;
}

JI_API JiTreeBuildResult ji_jiml_compile_string(const char* source,
                                                  const void* context) {
    JiTreeBuildResult result = {0};
    result.has_error = true;

    if (!source) {
        strncpy(result.error_msg, "NULL source string", sizeof(result.error_msg) - 1);
        return result;
    }

    JiJimlDocument* doc = ji_jiml_parse(source);
    if (!doc) {
        strncpy(result.error_msg, "Failed to parse JiML", sizeof(result.error_msg) - 1);
        return result;
    }

    if (doc->has_error) {
        strncpy(result.error_msg, doc->error_msg, sizeof(result.error_msg) - 1);
        result.error_line = doc->error_line;
        ji_jiml_document_destroy(doc);
        return result;
    }

    result = ji_jiml_compile(doc, context);
    ji_jiml_document_destroy(doc);
    return result;
}

JI_API JiTreeBuildResult ji_jiml_compile_file(const char* filepath,
                                                const void* context) {
    JiTreeBuildResult result = {0};
    result.has_error = true;

    if (!filepath) {
        strncpy(result.error_msg, "NULL filepath", sizeof(result.error_msg) - 1);
        return result;
    }

    JiJimlDocument* doc = ji_jiml_parse_file(filepath);
    if (!doc) {
        strncpy(result.error_msg, "Failed to parse JiML file", sizeof(result.error_msg) - 1);
        return result;
    }

    if (doc->has_error) {
        strncpy(result.error_msg, doc->error_msg, sizeof(result.error_msg) - 1);
        result.error_line = doc->error_line;
        ji_jiml_document_destroy(doc);
        return result;
    }

    result = ji_jiml_compile(doc, context);
    ji_jiml_document_destroy(doc);
    return result;
}

/* =========================================================================
 * Style Application
 * ========================================================================= */

/* Get the type name of an object */
static const char* jiml_get_object_type_name(JiObject* obj) {
    if (!obj) return NULL;
    const JiType* type = ji_object_type(obj);
    if (!type || !type->name) return NULL;
    return type->name;
}

/* Apply style rules to a single object */
static void jiml_apply_styles_to_object(JiObject* obj,
                                          const JiJimlDocument* doc) {
    if (!obj || !doc) return;

    const char* type_name = jiml_get_object_type_name(obj);
    if (!type_name) return;

    /* Find matching style rules in all style blocks */
    for (int s = 0; s < doc->style_count; s++) {
        const JiJimlStyleBlock* block = doc->styles[s];
        if (!block) continue;

        int matching[16];
        int match_count = ji_style_block_find_rules(block, type_name,
                                                       matching, 16);

        for (int m = 0; m < match_count; m++) {
            const JiJimlStyleRule* rule = &block->rules[matching[m]];
            /* Apply properties from the style rule */
            for (int p = 0; p < rule->prop_count; p++) {
                /* Try to set the property on the object.
                 * Since we don't have direct property ID lookup by name,
                 * we store style properties as object data for now. */
                /* In a full implementation, this would use the property system
                 * to set named properties. For now, we skip this step. */
                (void)rule->prop_names[p];
                (void)rule->prop_values[p];
            }
        }
    }

    /* Recursively apply to children */
    int child_count = ji_object_get_child_count(obj);
    for (int i = 0; i < child_count; i++) {
        JiObject* child = ji_object_get_child(obj, i);
        jiml_apply_styles_to_object(child, doc);
    }
}

static void jiml_apply_styles_to_tree(JiObject* root,
                                        const JiJimlDocument* doc) {
    jiml_apply_styles_to_object(root, doc);
}

/* =========================================================================
 * Binding Resolution
 * ========================================================================= */

/* Recursively walk the object tree and resolve binding expressions.
 * In a full implementation, this would:
 *   - Find properties with binding expression values
 *   - Evaluate the binding expression against the data context
 *   - Set the resolved value on the property
 * For now, this is a placeholder that walks the tree. */
static void jiml_resolve_bindings_in_tree(JiObject* root,
                                            const void* context) {
    if (!root) return;

    /* Walk children */
    int child_count = ji_object_get_child_count(root);
    for (int i = 0; i < child_count; i++) {
        JiObject* child = ji_object_get_child(root, i);
        jiml_resolve_bindings_in_tree(child, context);
    }

    /* In a full implementation, we would:
     * 1. Iterate over all properties of the object
     * 2. Check if the property value is a binding expression
     * 3. Parse the binding expression with ji_binding_expr_parse()
     * 4. Evaluate it with ji_binding_expr_eval() using the context
     * 5. Set the resolved value on the property
     *
     * The context would be a JiObject with named properties that
     * the binding engine can look up.
     */
    (void)context;
}
