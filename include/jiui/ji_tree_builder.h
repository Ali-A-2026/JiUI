/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_tree_builder.h
 * @brief Tree builder — converts an XML AST into a runtime JiObject tree.
 *
 * Walks the AST produced by ji_xml_parser and creates JiObject instances
 * using the type and property systems. Handles:
 *   - Object instantiation via type name lookup
 *   - Attribute → property assignment (with type coercion)
 *   - Child element → logical tree construction
 *   - Property element (Owner.Prop) → attached property setting
 *   - x:Name → object naming
 *   - Markup extensions (bindings, resources) → deferred resolution
 */

#ifndef JIUI_TREE_BUILDER_H
#define JIUI_TREE_BUILDER_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_ast.h"
#include "ji_object.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Build result
 * ========================================================================= */
typedef struct JiTreeBuildResult {
    JiObject*  root;          /* Root object of the built tree, or NULL on error */
    bool       has_error;     /* True if build failed */
    char       error_msg[512];/* Error description */
    int        error_line;    /* AST line where error occurred */
} JiTreeBuildResult;

/* =========================================================================
 * Tree Builder API
 * ========================================================================= */

/**
 * Build a runtime object tree from an XML AST.
 *
 * @param ast  The root AST node (typically from ji_xml_parser_parse).
 * @return     Build result containing the root object or error info.
 *             Caller must destroy the root object via ji_ref_object_release()
 *             when done.
 */
JI_API JiTreeBuildResult ji_tree_build(const JiAstNode* ast);

/**
 * Build a runtime object tree from an XML string.
 *
 * Convenience function that parses the string and builds the tree.
 * @param source  XML source text.
 * @return        Build result.
 */
JI_API JiTreeBuildResult ji_tree_build_from_string(const char* source);

/**
 * Build a runtime object tree from an XML file.
 *
 * @param filepath  Path to the .ji file.
 * @return          Build result.
 */
JI_API JiTreeBuildResult ji_tree_build_from_file(const char* filepath);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_TREE_BUILDER_H */
