/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_style.h
 * @brief Styling & theming system — styles, selectors, resource dictionaries,
 *        and theme management.
 *
 * Style system. Styles contain setters that override
 * properties on matching controls. Themes are named collections of styles.
 * Resource dictionaries store shared values (brushes, fonts, etc.).
 */

#ifndef JIUI_STYLE_H
#define JIUI_STYLE_H

#include "ji_defines.h"
#include "ji_api.h"
#include "ji_control.h"
#include "ji_types.h"
#include "ji_resource.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiSelector — matches controls for style application
 * ========================================================================= */
typedef enum JiSelectorKind {
    JI_SELECTOR_TYPE       = 0,  /* match by type name (e.g. "JiButton") */
    JI_SELECTOR_CLASS      = 1,  /* match by CSS-like class (e.g. ".primary") */
    JI_SELECTOR_NAME       = 2,  /* match by name (e.g. "#myButton") */
    JI_SELECTOR_PSEUDO     = 3,  /* match by pseudo-class (:hover, :focus, :pressed) */
    JI_SELECTOR_TYPE_CLASS = 4   /* match by type AND class */
} JiSelectorKind;

typedef enum JiPseudoClass {
    JI_PSEUDO_NONE     = 0,
    JI_PSEUDO_HOVER    = 1,
    JI_PSEUDO_FOCUS    = 2,
    JI_PSEUDO_PRESSED  = 4,
    JI_PSEUDO_DISABLED = 8,
    JI_PSEUDO_CHECKED  = 16
} JiPseudoClass;

typedef struct JiSelector {
    JiSelectorKind kind;
    char*  type_name;     /* for TYPE / TYPE_CLASS selectors */
    char*  class_name;    /* for CLASS / TYPE_CLASS selectors */
    char*  name;          /* for NAME selectors */
    JiPseudoClass pseudo; /* for PSEUDO selectors */
} JiSelector;

/** Create a type selector (matches by type name). */
JI_API JiSelector ji_selector_type(const char* type_name);

/** Create a class selector (matches by CSS class). */
JI_API JiSelector ji_selector_class(const char* class_name);

/** Create a name selector (matches by control name). */
JI_API JiSelector ji_selector_name(const char* name);

/** Create a pseudo-class selector. */
JI_API JiSelector ji_selector_pseudo(JiPseudoClass pseudo);

/** Create a type+class selector. */
JI_API JiSelector ji_selector_type_class(const char* type_name, const char* class_name);

/** Destroy a selector (frees allocated strings). */
JI_API void ji_selector_destroy(JiSelector* sel);

/** Check if a selector matches a control. */
JI_API bool ji_selector_matches(const JiSelector* sel, const JiControl* control);

/* =========================================================================
 * JiSetter — a single property value setter within a style
 * ========================================================================= */
typedef struct JiSetter {
    char*         property_name;  /* property to set */
    JiPropertyValue value;        /* value to set */
} JiSetter;

/** Create a setter with a string value. */
JI_API JiSetter ji_setter_string(const char* prop_name, const char* value);

/** Create a setter with a double value. */
JI_API JiSetter ji_setter_double(const char* prop_name, double value);

/** Create a setter with an int value. */
JI_API JiSetter ji_setter_int(const char* prop_name, int value);

/** Create a setter with a color value. */
JI_API JiSetter ji_setter_color(const char* prop_name, uint32_t argb);

/** Create a setter with a bool value. */
JI_API JiSetter ji_setter_bool(const char* prop_name, bool value);

/** Destroy a setter (frees allocated strings). */
JI_API void ji_setter_destroy(JiSetter* setter);

/* =========================================================================
 * JiStyle — a collection of setters with a selector
 * ========================================================================= */
typedef struct JiStyle {
    JiSelector  selector;
    JiSetter*   setters;
    int         setter_count;
    int         setter_capacity;
} JiStyle;

/** Create a new style with the given selector. */
JI_API JiStyle* ji_style_new(JiSelector selector);

/** Destroy a style. */
JI_API void ji_style_destroy(JiStyle* style);

/** Add a setter to the style. */
JI_API void ji_style_add_setter(JiStyle* style, JiSetter setter);

/** Get the number of setters. */
JI_API int ji_style_get_setter_count(const JiStyle* style);

/** Check if this style matches a control. */
JI_API bool ji_style_matches(const JiStyle* style, const JiControl* control);

/** Apply the style's setters to a control. */
JI_API void ji_style_apply(const JiStyle* style, JiControl* control);

/**
 * Parse a selector string into a JiSelector.
 * Supports: "TypeName", ".className", "#name", "TypeName.className",
 *           ":hover", ":focus", ":pressed", ":disabled", ":checked"
 * @return Parsed selector. Caller must call ji_selector_destroy().
 */
JI_API JiSelector ji_style_parse_selector(const char* str);

/**
 * Parse a setter from XML attribute-style "Property=Value" string.
 * Auto-detects value type (bool, int, double, color, string).
 * @return Parsed setter. Caller must call ji_setter_destroy().
 */
JI_API JiSetter ji_style_parse_setter(const char* prop_name, const char* value_str);

/* JiResourceDictionary types and API are now in ji_resource.h */
/* =========================================================================
 * JiTheme — named collection of styles and resources
 * ========================================================================= */
typedef struct JiTheme {
    char*                name;
    JiStyle**            styles;
    int                  style_count;
    int                  style_capacity;
    JiResourceDictionary* resources;
} JiTheme;

/** Create a new theme with the given name. */
JI_API JiTheme* ji_theme_new(const char* name);

/** Destroy a theme. */
JI_API void ji_theme_destroy(JiTheme* theme);

/** Add a style to the theme. The theme takes ownership. */
JI_API void ji_theme_add_style(JiTheme* theme, JiStyle* style);

/** Get the number of styles. */
JI_API int ji_theme_get_style_count(const JiTheme* theme);

/** Get a style by index. */
JI_API JiStyle* ji_theme_get_style(const JiTheme* theme, int index);

/** Get the resource dictionary. */
JI_API JiResourceDictionary* ji_theme_get_resources(JiTheme* theme);

/* =========================================================================
 * Theme manager — global current theme
 * ========================================================================= */

/** Set the current theme. The library takes ownership. */
JI_API void ji_set_current_theme(JiTheme* theme);

/** Get the current theme. */
JI_API JiTheme* ji_get_current_theme(void);

/** Apply the current theme to a control (matches all styles). */
JI_API void ji_apply_theme_to_control(JiControl* control);

/** Apply the current theme to a control and all its visual children. */
JI_API void ji_apply_theme_recursive(JiControl* control);

/** Initialize the style system (called during ji_initialize). */
JI_API void ji_style_type_init(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_STYLE_H */
