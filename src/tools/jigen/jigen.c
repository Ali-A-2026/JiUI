/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file jigen.c
 * @brief jigen — Ahead-of-time code generator for .ji XML files.
 *
 * Usage:
 *   jigen <input.ji> [output.c] [--class ClassName]
 *
 * If output is not specified, writes to <input>.ji.c
 * If --class is not specified, derives from the input filename.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jiui/ji_codegen.h"

static void print_usage(const char* prog) {
    fprintf(stderr, "Usage: %s <input.ji> [output.c] [--class ClassName]\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Compiles a .ji XML file to C source code that constructs\n");
    fprintf(stderr, "the equivalent object tree at runtime without XML parsing.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --class NAME   Set the build function name (default: derived from filename)\n");
    fprintf(stderr, "  --help         Show this help message\n");
}

/* Extract a class name from a filename (strip path and extension) */
static void derive_class_name(const char* filepath, char* buf, size_t buf_size) {
    /* Find the last path separator */
    const char* base = strrchr(filepath, '/');
    if (!base) base = strrchr(filepath, '\\');
    if (base) base++; else base = filepath;

    /* Copy and strip extension */
    size_t j = 0;
    for (size_t i = 0; base[i] && base[i] != '.' && j < buf_size - 1; i++) {
        buf[j++] = base[i];
    }
    buf[j] = '\0';

    /* Capitalize first letter */
    if (buf[0] >= 'a' && buf[0] <= 'z') {
        buf[0] = buf[0] - 'a' + 'A';
    }
}

int main(int argc, char* argv[]) {
    const char* input_path = NULL;
    const char* output_path = NULL;
    const char* class_name = NULL;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--class") == 0) {
            if (i + 1 < argc) {
                class_name = argv[++i];
            } else {
                fprintf(stderr, "Error: --class requires an argument\n");
                return 1;
            }
        } else if (!input_path) {
            input_path = argv[i];
        } else if (!output_path) {
            output_path = argv[i];
        }
    }

    if (!input_path) {
        fprintf(stderr, "Error: No input file specified\n\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Derive class name from filename if not specified */
    char derived_class[256];
    if (!class_name) {
        derive_class_name(input_path, derived_class, sizeof(derived_class));
        class_name = derived_class;
    }

    /* Derive output path if not specified */
    char derived_output[1024];
    if (!output_path) {
        snprintf(derived_output, sizeof(derived_output), "%s.c", input_path);
        output_path = derived_output;
    }

    /* Open output file */
    FILE* out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Error: Cannot open output file: %s\n", output_path);
        return 1;
    }

    /* Generate code */
    JiCodeGenResult result = ji_codegen_from_file(input_path, out, class_name);
    fclose(out);

    if (result.has_error) {
        fprintf(stderr, "Error: Code generation failed: %s\n", result.error_msg);
        /* Remove the output file on failure */
        remove(output_path);
        return 1;
    }

    printf("Generated %s: %d objects, %d bindings, %d events\n",
           output_path, result.object_count, result.binding_count, result.event_count);

    return 0;
}
