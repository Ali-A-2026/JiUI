/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_codegen.h
 * @brief Ahead-of-time code generation — compiles .ji XML files to C source.
 *
 * The jigen tool uses this API to walk a parsed AST and emit C code that
 * constructs the equivalent object tree at runtime without any XML parsing
 * overhead.
 */

#ifndef JIUI_CODEGEN_H
#define JIUI_CODEGEN_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_ast.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Code generation result
 * ========================================================================= */
typedef struct JiCodeGenResult {
    bool    has_error;      /* True if code generation failed */
    char    error_msg[512]; /* Error description */
    int     object_count;   /* Number of objects generated */
    int     binding_count;  /* Number of bindings generated */
    int     event_count;    /* Number of event subscriptions generated */
} JiCodeGenResult;

/* =========================================================================
 * Code Generation API
 * ========================================================================= */

/**
 * Generate C code from a parsed AST, writing to the given FILE.
 *
 * The generated code defines a function `ji_build_<class_name>()` that
 * constructs the object tree, sets properties, registers bindings,
 * subscribes event handlers, and returns the root object.
 *
 * @param ast         Root AST node (from ji_xml_parser).
 * @param output      FILE to write C source code to.
 * @param class_name  Name for the generated build function (e.g. "MainWindow").
 * @return            Code generation result with error info and statistics.
 */
JI_API JiCodeGenResult ji_codegen_generate(const JiAstNode* ast, FILE* output,
                                             const char* class_name);

/**
 * Generate C code from a .ji XML file.
 *
 * Convenience function that parses the file and generates code.
 *
 * @param input_path  Path to the .ji XML file.
 * @param output      FILE to write C source code to.
 * @param class_name  Name for the generated build function.
 * @return            Code generation result.
 */
JI_API JiCodeGenResult ji_codegen_from_file(const char* input_path, FILE* output,
                                               const char* class_name);

/**
 * Generate C code from an XML string.
 *
 * @param source      XML source text.
 * @param output      FILE to write C source code to.
 * @param class_name  Name for the generated build function.
 * @return            Code generation result.
 */
JI_API JiCodeGenResult ji_codegen_from_string(const char* source, FILE* output,
                                                 const char* class_name);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_CODEGEN_H */
