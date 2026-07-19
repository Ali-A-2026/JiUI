/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file main.c
 * @brief Example: Loading .ji XML files at runtime with the JiUI loader.
 *
 * Demonstrates:
 *   - Loading a .ji XML string with ji_load_string()
 *   - Registering event handlers before loading
 *   - Finding named elements with ji_load_find_name()
 *   - Accessing the resource dictionary
 *   - Using the binding engine
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <stdlib.h>

/* ---- Event handler callbacks ---- */

static void on_button_clicked(JiObject* sender, void* args, void* user_data) {
    (void)args;
    (void)user_data;
    const char* name = ji_object_name(sender);
    printf("[Event] Button clicked: %s\n", name ? name : "(unnamed)");
}

static void on_text_changed(JiObject* sender, void* args, void* user_data) {
    (void)args;
    (void)user_data;
    const char* name = ji_object_name(sender);
    printf("[Event] Text changed: %s\n", name ? name : "(unnamed)");
}

/* ---- Main ---- */

int main(void) {
    ji_initialize();

    /* 1. Register event handlers BEFORE loading the .ji file.
     *    The loader resolves event attributes like Click="on_button_clicked"
     *    by looking up the handler name in the global registry. */
    ji_event_register_handler("on_button_clicked", on_button_clicked);
    ji_event_register_handler("on_text_changed", on_text_changed);

    /* 2. Define a .ji XML document inline.
     *    In a real app, you'd use ji_load_file("MainWindow.ji"). */
    const char* ji_source =
        "<Object x:Name=\"mainWindow\">\n"
        "  <Object.Resources>\n"
        "    <ResourceDictionary>\n"
        "      <SolidColorBrush x:Key=\"bgBrush\" Color=\"#FF2D2D30\"/>\n"
        "      <SolidColorBrush x:Key=\"fgBrush\" Color=\"#FFFFFFFF\"/>\n"
        "    </ResourceDictionary>\n"
        "  </Object.Resources>\n"
        "  <StackPanel x:Name=\"rootPanel\" Orientation=\"Vertical\" Spacing=\"8\">\n"
        "    <Label x:Name=\"titleLabel\" Text=\"XML Loader Demo\" FontSize=\"18\"/>\n"
        "    <Label x:Name=\"descLabel\" Text=\"Loaded from .ji XML at runtime\" FontSize=\"12\"/>\n"
        "    <TextBox x:Name=\"nameInput\" Text=\"{Binding Username, Mode=TwoWay}\"/>\n"
        "    <Button x:Name=\"submitBtn\" Text=\"Submit\" Click=\"on_button_clicked\"/>\n"
        "    <Button x:Name=\"cancelBtn\" Text=\"Cancel\" Click=\"on_button_clicked\"/>\n"
        "  </StackPanel>\n"
        "</Object>\n";

    /* 3. Load the .ji XML string */
    JiLoadResult result = ji_load_string(ji_source);

    if (result.has_error) {
        fprintf(stderr, "Failed to load .ji: %s\n", result.error_msg);
        ji_shutdown();
        return 1;
    }

    printf("=== XML Loader Demo ===\n\n");
    printf("Loaded successfully!\n");

    /* 4. Access the root object */
    JiObject* root = result.root;
    if (root) {
        const char* root_name = ji_object_name(root);
        printf("Root object: %s\n", root_name ? root_name : "(unnamed)");
        printf("Root has %d children\n", ji_object_get_child_count(root));
    }

    /* 5. Find named elements */
    JiObject* title = ji_load_find_name(result.root, "titleLabel");
    if (title) {
        printf("Found: titleLabel\n");
    }

    JiObject* btn = ji_load_find_name(result.root, "submitBtn");
    if (btn) {
        printf("Found: submitBtn\n");
    }

    JiObject* input = ji_load_find_name(result.root, "nameInput");
    if (input) {
        printf("Found: nameInput (has binding)\n");
    }

    /* 6. Access the resource dictionary */
    if (result.resources) {
        int count = ji_resource_dict_get_count(result.resources);
        printf("Resources: %d entries\n", count);

        JiPropertyValue val;
        if (ji_resource_dict_try_get(result.resources, "bgBrush", &val)) {
            printf("  bgBrush found (type: %d)\n", val.type);
        }
        if (ji_resource_dict_try_get(result.resources, "fgBrush", &val)) {
            printf("  fgBrush found (type: %d)\n", val.type);
        }
    }

    /* 7. Check bindings */
    printf("Bindings: %d active\n", ji_binding_engine_count(&result.bindings));

    /* 8. Check events */
    printf("Events: %d subscriptions\n", ji_event_bus_count(&result.events));

    /* 9. Simulate a button click by dispatching through the event bus */
    if (btn) {
        printf("\nDispatching Click event on submitBtn...\n");
        ji_event_bus_dispatch(&result.events, btn, "Click", NULL);
    }

    /* 10. Cleanup */
    ji_load_result_destroy(&result);

    printf("\nCleaned up successfully.\n");

    ji_shutdown();
    return 0;
}
