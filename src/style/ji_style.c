/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_style.c
 * @brief Implementation of the styling & theming system.
 */

#include "jiui/ji_style.h"
#include "jiui/ji_memory.h"
#include "jiui/ji_type.h"
#include "jiui/ji_property.h"
#include "jiui/ji_object.h"
#include "jiui/ji_widgets.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* =========================================================================
 * Helper: strdup using ji_alloc
 * ========================================================================= */
static char* ji_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)ji_alloc(len + 1);
    if (copy) memcpy(copy, s, len + 1);
    return copy;
}

/* =========================================================================
 * JiSelector
 * ========================================================================= */
JiSelector ji_selector_type(const char* type_name) {
    JiSelector sel;
    memset(&sel, 0, sizeof(JiSelector));
    sel.kind = JI_SELECTOR_TYPE;
    sel.type_name = ji_strdup(type_name);
    sel.class_name = NULL;
    sel.name = NULL;
    sel.pseudo = JI_PSEUDO_NONE;
    return sel;
}

JiSelector ji_selector_class(const char* class_name) {
    JiSelector sel;
    memset(&sel, 0, sizeof(JiSelector));
    sel.kind = JI_SELECTOR_CLASS;
    sel.type_name = NULL;
    sel.class_name = ji_strdup(class_name);
    sel.name = NULL;
    sel.pseudo = JI_PSEUDO_NONE;
    return sel;
}

JiSelector ji_selector_name(const char* name) {
    JiSelector sel;
    memset(&sel, 0, sizeof(JiSelector));
    sel.kind = JI_SELECTOR_NAME;
    sel.type_name = NULL;
    sel.class_name = NULL;
    sel.name = ji_strdup(name);
    sel.pseudo = JI_PSEUDO_NONE;
    return sel;
}

JiSelector ji_selector_pseudo(JiPseudoClass pseudo) {
    JiSelector sel;
    memset(&sel, 0, sizeof(JiSelector));
    sel.kind = JI_SELECTOR_PSEUDO;
    sel.type_name = NULL;
    sel.class_name = NULL;
    sel.name = NULL;
    sel.pseudo = pseudo;
    return sel;
}

JiSelector ji_selector_type_class(const char* type_name, const char* class_name) {
    JiSelector sel;
    memset(&sel, 0, sizeof(JiSelector));
    sel.kind = JI_SELECTOR_TYPE_CLASS;
    sel.type_name = ji_strdup(type_name);
    sel.class_name = ji_strdup(class_name);
    sel.name = NULL;
    sel.pseudo = JI_PSEUDO_NONE;
    return sel;
}

void ji_selector_destroy(JiSelector* sel) {
    if (!sel) return;
    if (sel->type_name)  ji_free(sel->type_name);
    if (sel->class_name) ji_free(sel->class_name);
    if (sel->name)       ji_free(sel->name);
    memset(sel, 0, sizeof(JiSelector));
}

bool ji_selector_matches(const JiSelector* sel, const JiControl* control) {
    if (!sel || !control) return false;

    switch (sel->kind) {
        case JI_SELECTOR_TYPE: {
            /* Check if the control's type name matches */
            JiTypeId control_type = control->visual.layout.base.type_id;
            while (control_type != JI_TYPE_INVALID) {
                const char* name = ji_type_name(control_type);
                if (name && strcmp(name, sel->type_name) == 0) return true;
                control_type = ji_type_parent(control_type);
            }
            return false;
        }
        case JI_SELECTOR_CLASS:
            /* Classes would be stored as a property or list on the control.
               For now, we use the name field as a simple class match. */
            return false; /* TODO: implement class storage on controls */

        case JI_SELECTOR_NAME:
            if (control->name && sel->name) {
                return strcmp(control->name, sel->name) == 0;
            }
            return false;

        case JI_SELECTOR_PSEUDO: {
            bool matches = true;
            if (sel->pseudo & JI_PSEUDO_HOVER) {
                /* JiButton has is_hovered; general controls don't yet */
            }
            if (sel->pseudo & JI_PSEUDO_FOCUS) {
                matches = matches && control->is_focused;
            }
            if (sel->pseudo & JI_PSEUDO_DISABLED) {
                matches = matches && !control->is_enabled;
            }
            if (sel->pseudo & JI_PSEUDO_PRESSED) {
                /* JiButton has is_pressed */
            }
            if (sel->pseudo & JI_PSEUDO_CHECKED) {
                /* JiCheckBox has is_checked */
            }
            return matches;
        }

        case JI_SELECTOR_TYPE_CLASS: {
            /* Must match both type and class */
            JiSelector type_sel = ji_selector_type(sel->type_name);
            bool type_matches = ji_selector_matches(&type_sel, control);
            ji_selector_destroy(&type_sel);
            return type_matches; /* class part not yet implemented */
        }

        default:
            return false;
    }
}

/* =========================================================================
 * JiSetter
 * ========================================================================= */
JiSetter ji_setter_string(const char* prop_name, const char* value) {
    JiSetter s;
    s.property_name = ji_strdup(prop_name);
    s.value = ji_value_string(value);
    return s;
}

JiSetter ji_setter_double(const char* prop_name, double value) {
    JiSetter s;
    s.property_name = ji_strdup(prop_name);
    s.value = ji_value_double(value);
    return s;
}

JiSetter ji_setter_int(const char* prop_name, int value) {
    JiSetter s;
    s.property_name = ji_strdup(prop_name);
    s.value = ji_value_int(value);
    return s;
}

JiSetter ji_setter_color(const char* prop_name, uint32_t argb) {
    JiSetter s;
    s.property_name = ji_strdup(prop_name);
    s.value = ji_value_color(argb);
    return s;
}

JiSetter ji_setter_bool(const char* prop_name, bool value) {
    JiSetter s;
    s.property_name = ji_strdup(prop_name);
    s.value = ji_value_bool(value);
    return s;
}

void ji_setter_destroy(JiSetter* setter) {
    if (!setter) return;
    if (setter->property_name) ji_free(setter->property_name);
    if (setter->value.type == JI_PROP_TYPE_STRING && setter->value.v.string_val) {
        ji_free(setter->value.v.string_val);
    }
    memset(setter, 0, sizeof(JiSetter));
}

/* =========================================================================
 * JiStyle
 * ========================================================================= */
JiStyle* ji_style_new(JiSelector selector) {
    JiStyle* style = JI_NEW(JiStyle);
    if (!style) return NULL;
    style->selector = selector;
    style->setters = NULL;
    style->setter_count = 0;
    style->setter_capacity = 0;
    return style;
}

void ji_style_destroy(JiStyle* style) {
    if (!style) return;
    ji_selector_destroy(&style->selector);
    for (int i = 0; i < style->setter_count; i++) {
        ji_setter_destroy(&style->setters[i]);
    }
    ji_free(style->setters);
    ji_free(style);
}

void ji_style_add_setter(JiStyle* style, JiSetter setter) {
    if (!style) return;
    if (style->setter_count >= style->setter_capacity) {
        int new_cap = style->setter_capacity == 0 ? 4 : style->setter_capacity * 2;
        style->setters = (JiSetter*)ji_realloc(style->setters,
            (size_t)new_cap * sizeof(JiSetter));
        style->setter_capacity = new_cap;
    }
    style->setters[style->setter_count++] = setter;
}

int ji_style_get_setter_count(const JiStyle* style) {
    return style ? style->setter_count : 0;
}

bool ji_style_matches(const JiStyle* style, const JiControl* control) {
    if (!style || !control) return false;
    return ji_selector_matches(&style->selector, control);
}

void ji_style_apply(const JiStyle* style, JiControl* control) {
    if (!style || !control) return;
    for (int i = 0; i < style->setter_count; i++) {
        const JiSetter* s = &style->setters[i];
        /* Apply well-known property names directly */
        if (strcmp(s->property_name, "opacity") == 0 && s->value.type == JI_PROP_TYPE_DOUBLE) {
            ji_visual_set_opacity(&control->visual, s->value.v.double_val);
        } else if (strcmp(s->property_name, "visibility") == 0 && s->value.type == JI_PROP_TYPE_INT) {
            ji_visual_set_visibility(&control->visual, (JiVisibility)s->value.v.int_val);
        } else if (strcmp(s->property_name, "is_enabled") == 0 && s->value.type == JI_PROP_TYPE_BOOL) {
            ji_control_set_enabled(control, s->value.v.bool_val);
        } else if (strcmp(s->property_name, "tab_index") == 0 && s->value.type == JI_PROP_TYPE_INT) {
            ji_control_set_tab_index(control, s->value.v.int_val);
        }
        /* Widget-specific properties can be extended here */
    }
}

/* =========================================================================
 * JiResourceDictionary
 * ========================================================================= */
JiResourceDictionary* ji_resource_dict_new(void) {
    JiResourceDictionary* dict = JI_NEW(JiResourceDictionary);
    if (!dict) return NULL;
    dict->entries = NULL;
    dict->entry_count = 0;
    dict->entry_capacity = 0;
    return dict;
}

void ji_resource_dict_destroy(JiResourceDictionary* dict) {
    if (!dict) return;
    for (int i = 0; i < dict->entry_count; i++) {
        if (dict->entries[i].key) ji_free(dict->entries[i].key);
        if (dict->entries[i].value.type == JI_PROP_TYPE_STRING && dict->entries[i].value.v.string_val) {
            ji_free(dict->entries[i].value.v.string_val);
        }
    }
    ji_free(dict->entries);
    ji_free(dict);
}

static void ji_resource_dict_grow(JiResourceDictionary* dict) {
    if (dict->entry_count >= dict->entry_capacity) {
        int new_cap = dict->entry_capacity == 0 ? 8 : dict->entry_capacity * 2;
        dict->entries = (JiResourceEntry*)ji_realloc(dict->entries,
            (size_t)new_cap * sizeof(JiResourceEntry));
        dict->entry_capacity = new_cap;
    }
}

static int ji_resource_dict_find(const JiResourceDictionary* dict, const char* key) {
    if (!dict || !key) return -1;
    for (int i = 0; i < dict->entry_count; i++) {
        if (strcmp(dict->entries[i].key, key) == 0) return i;
    }
    return -1;
}

void ji_resource_dict_set_string(JiResourceDictionary* dict,
                                  const char* key, const char* value) {
    if (!dict || !key) return;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        /* Update existing */
        if (dict->entries[idx].value.type == JI_PROP_TYPE_STRING && dict->entries[idx].value.v.string_val) {
            ji_free(dict->entries[idx].value.v.string_val);
        }
        dict->entries[idx].value = ji_value_string(value);
    } else {
        ji_resource_dict_grow(dict);
        dict->entries[dict->entry_count].key = ji_strdup(key);
        dict->entries[dict->entry_count].value = ji_value_string(value);
        dict->entry_count++;
    }
}

void ji_resource_dict_set_double(JiResourceDictionary* dict,
                                   const char* key, double value) {
    if (!dict || !key) return;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        dict->entries[idx].value = ji_value_double(value);
    } else {
        ji_resource_dict_grow(dict);
        dict->entries[dict->entry_count].key = ji_strdup(key);
        dict->entries[dict->entry_count].value = ji_value_double(value);
        dict->entry_count++;
    }
}

void ji_resource_dict_set_color(JiResourceDictionary* dict,
                                  const char* key, uint32_t argb) {
    if (!dict || !key) return;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        dict->entries[idx].value = ji_value_color(argb);
    } else {
        ji_resource_dict_grow(dict);
        dict->entries[dict->entry_count].key = ji_strdup(key);
        dict->entries[dict->entry_count].value = ji_value_color(argb);
        dict->entry_count++;
    }
}

bool ji_resource_dict_try_get(const JiResourceDictionary* dict,
                                const char* key, JiPropertyValue* out_val) {
    if (!dict || !key || !out_val) return false;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        *out_val = dict->entries[idx].value;
        return true;
    }
    return false;
}

int ji_resource_dict_get_count(const JiResourceDictionary* dict) {
    return dict ? dict->entry_count : 0;
}

void ji_resource_dict_set_int(JiResourceDictionary* dict,
                                const char* key, int value) {
    if (!dict || !key) return;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        dict->entries[idx].value = ji_value_int(value);
    } else {
        ji_resource_dict_grow(dict);
        dict->entries[dict->entry_count].key = ji_strdup(key);
        dict->entries[dict->entry_count].value = ji_value_int(value);
        dict->entry_count++;
    }
}

void ji_resource_dict_set_bool(JiResourceDictionary* dict,
                                 const char* key, bool value) {
    if (!dict || !key) return;
    int idx = ji_resource_dict_find(dict, key);
    if (idx >= 0) {
        dict->entries[idx].value = ji_value_bool(value);
    } else {
        ji_resource_dict_grow(dict);
        dict->entries[dict->entry_count].key = ji_strdup(key);
        dict->entries[dict->entry_count].value = ji_value_bool(value);
        dict->entry_count++;
    }
}

bool ji_resource_dict_has(const JiResourceDictionary* dict, const char* key) {
    if (!dict || !key) return false;
    return ji_resource_dict_find(dict, key) >= 0;
}

bool ji_resource_dict_remove(JiResourceDictionary* dict, const char* key) {
    if (!dict || !key) return false;
    int idx = ji_resource_dict_find(dict, key);
    if (idx < 0) return false;
    ji_free(dict->entries[idx].key);
    for (int i = idx; i < dict->entry_count - 1; i++) {
        dict->entries[i] = dict->entries[i + 1];
    }
    dict->entry_count--;
    return true;
}

void ji_resource_dict_merge(JiResourceDictionary* dict,
                              const JiResourceDictionary* other) {
    if (!dict || !other) return;
    for (int i = 0; i < other->entry_count; i++) {
        /* Use try_get to check if already exists locally, then set */
        const char* key = other->entries[i].key;
        if (ji_resource_dict_find(dict, key) < 0) {
            ji_resource_dict_grow(dict);
            dict->entries[dict->entry_count].key = ji_strdup(key);
            dict->entries[dict->entry_count].value = other->entries[i].value;
            if (dict->entries[dict->entry_count].value.type == JI_PROP_TYPE_STRING &&
                dict->entries[dict->entry_count].value.v.string_val) {
                dict->entries[dict->entry_count].value.v.string_val =
                    ji_strdup(dict->entries[dict->entry_count].value.v.string_val);
            }
            dict->entry_count++;
        }
    }
}

/* =========================================================================
 * JiTheme
 * ========================================================================= */
JiTheme* ji_theme_new(const char* name) {
    JiTheme* theme = JI_NEW(JiTheme);
    if (!theme) return NULL;
    theme->name = ji_strdup(name);
    theme->styles = NULL;
    theme->style_count = 0;
    theme->style_capacity = 0;
    theme->resources = ji_resource_dict_new();
    return theme;
}

void ji_theme_destroy(JiTheme* theme) {
    if (!theme) return;
    if (theme->name) ji_free(theme->name);
    for (int i = 0; i < theme->style_count; i++) {
        ji_style_destroy(theme->styles[i]);
    }
    ji_free(theme->styles);
    if (theme->resources) ji_resource_dict_destroy(theme->resources);
    ji_free(theme);
}

void ji_theme_add_style(JiTheme* theme, JiStyle* style) {
    if (!theme || !style) return;
    if (theme->style_count >= theme->style_capacity) {
        int new_cap = theme->style_capacity == 0 ? 8 : theme->style_capacity * 2;
        theme->styles = (JiStyle**)ji_realloc(theme->styles,
            (size_t)new_cap * sizeof(JiStyle*));
        theme->style_capacity = new_cap;
    }
    theme->styles[theme->style_count++] = style;
}

int ji_theme_get_style_count(const JiTheme* theme) {
    return theme ? theme->style_count : 0;
}

/* =========================================================================
 * XML-style selector and setter parsing
 * ========================================================================= */

JiSelector ji_style_parse_selector(const char* str) {
    JiSelector sel;
    memset(&sel, 0, sizeof(sel));

    if (!str || !str[0]) {
        sel.kind = JI_SELECTOR_TYPE;
        sel.type_name = ji_strdup("Object");
        return sel;
    }

    /* Pseudo-class: :hover, :focus, etc. */
    if (str[0] == ':') {
        sel.kind = JI_SELECTOR_PSEUDO;
        if (strcmp(str, ":hover") == 0)        sel.pseudo = JI_PSEUDO_HOVER;
        else if (strcmp(str, ":focus") == 0)   sel.pseudo = JI_PSEUDO_FOCUS;
        else if (strcmp(str, ":pressed") == 0)  sel.pseudo = JI_PSEUDO_PRESSED;
        else if (strcmp(str, ":disabled") == 0) sel.pseudo = JI_PSEUDO_DISABLED;
        else if (strcmp(str, ":checked") == 0)  sel.pseudo = JI_PSEUDO_CHECKED;
        else                                   sel.pseudo = JI_PSEUDO_NONE;
        return sel;
    }

    /* Name selector: #myName */
    if (str[0] == '#') {
        sel.kind = JI_SELECTOR_NAME;
        sel.name = ji_strdup(str + 1);
        return sel;
    }

    /* Class selector: .className */
    if (str[0] == '.') {
        sel.kind = JI_SELECTOR_CLASS;
        sel.class_name = ji_strdup(str + 1);
        return sel;
    }

    /* Type+class: TypeName.className */
    const char* dot = strchr(str, '.');
    if (dot) {
        sel.kind = JI_SELECTOR_TYPE_CLASS;
        size_t tlen = (size_t)(dot - str);
        sel.type_name = ji_alloc(tlen + 1);
        memcpy(sel.type_name, str, tlen);
        sel.type_name[tlen] = '\0';
        sel.class_name = ji_strdup(dot + 1);
        return sel;
    }

    /* Plain type name */
    sel.kind = JI_SELECTOR_TYPE;
    sel.type_name = ji_strdup(str);
    return sel;
}

/* Internal: check if a string looks like a color (#RGB, #RRGGBB, #AARRGGBB) */
static bool str_is_color(const char* s) {
    if (!s || s[0] != '#') return false;
    size_t len = strlen(s);
    return len == 4 || len == 7 || len == 9;
}

/* Internal: parse a hex color string to ARGB uint32 */
static uint32_t parse_color_hex(const char* s) {
    if (!s || s[0] != '#') return 0xFF000000;
    size_t len = strlen(s);
    unsigned int val = 0;
    sscanf(s + 1, "%x", &val);
    if (len == 4) {
        /* #RGB → #FFRRGGBB */
        unsigned int r = (val >> 8) & 0xF;
        unsigned int g = (val >> 4) & 0xF;
        unsigned int b = val & 0xF;
        return 0xFF000000u | (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
    }
    if (len == 7) {
        /* #RRGGBB → #FFRRGGBB */
        return 0xFF000000u | val;
    }
    if (len == 9) {
        /* #AARRGGBB */
        return val;
    }
    return 0xFF000000u;
}

JiSetter ji_style_parse_setter(const char* prop_name, const char* value_str) {
    JiSetter s;
    memset(&s, 0, sizeof(s));
    s.property_name = prop_name ? ji_strdup(prop_name) : NULL;

    if (!value_str) {
        s.value = ji_value_string("");
        return s;
    }

    /* Bool */
    if (strcmp(value_str, "true") == 0 || strcmp(value_str, "True") == 0) {
        s.value = ji_value_bool(true);
        return s;
    }
    if (strcmp(value_str, "false") == 0 || strcmp(value_str, "False") == 0) {
        s.value = ji_value_bool(false);
        return s;
    }

    /* Color */
    if (str_is_color(value_str)) {
        s.value = ji_value_color(parse_color_hex(value_str));
        return s;
    }

    /* Try integer */
    char* endp = NULL;
    long lval = strtol(value_str, &endp, 10);
    if (endp && *endp == '\0' && endp != value_str) {
        s.value = ji_value_int((int)lval);
        return s;
    }

    /* Try double */
    double dval = strtod(value_str, &endp);
    if (endp && *endp == '\0' && endp != value_str) {
        s.value = ji_value_double(dval);
        return s;
    }

    /* Default: string */
    s.value = ji_value_string(value_str);
    return s;
}

JiStyle* ji_theme_get_style(const JiTheme* theme, int index) {
    if (!theme || index < 0 || index >= theme->style_count) return NULL;
    return theme->styles[index];
}

JiResourceDictionary* ji_theme_get_resources(JiTheme* theme) {
    return theme ? theme->resources : NULL;
}

/* =========================================================================
 * Theme manager
 * ========================================================================= */
static JiTheme* s_current_theme = NULL;

void ji_set_current_theme(JiTheme* theme) {
    if (s_current_theme) {
        ji_theme_destroy(s_current_theme);
    }
    s_current_theme = theme;
}

JiTheme* ji_get_current_theme(void) {
    return s_current_theme;
}

void ji_apply_theme_to_control(JiControl* control) {
    if (!s_current_theme || !control) return;
    for (int i = 0; i < s_current_theme->style_count; i++) {
        JiStyle* style = s_current_theme->styles[i];
        if (ji_style_matches(style, control)) {
            ji_style_apply(style, control);
        }
    }
}

void ji_apply_theme_recursive(JiControl* control) {
    if (!control) return;
    ji_apply_theme_to_control(control);

    /* Apply to visual children */
    JiVisual* v = &control->visual;
    for (int i = 0; i < v->visual_child_count; i++) {
        /* If the child is a control, apply theme */
        JiVisual* child = v->visual_children[i];
        JiTypeId child_type = child->layout.base.type_id;
        if (ji_type_is_a(child_type, ji_control_type_id())) {
            ji_apply_theme_recursive((JiControl*)child);
        }
    }
}

/* =========================================================================
 * Style type init (no types to register, but provides the init hook)
 * ========================================================================= */
void ji_style_type_init(void) {
    /* No types to register — styles are not type-system objects.
       This hook exists for future extension and consistency. */
}
