/**
 * JiUI - Stylesheet Engine header
 * CSS-like declarative styling for all widgets.
 * Surpasses Qt6 with live reload, CSS variables, and nested selectors.
 */

#ifndef JIUI_STYLESHEET_H
#define JIUI_STYLESHEET_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Style Property Types
 * ========================================================================= */
typedef enum JiStyleProperty {
    JI_STYLE_PROP_NONE = 0,
    /* Colors */
    JI_STYLE_PROP_BACKGROUND_COLOR,
    JI_STYLE_PROP_COLOR,
    JI_STYLE_PROP_BORDER_COLOR,
    JI_STYLE_PROP_SELECTION_COLOR,
    JI_STYLE_PROP_SELECTION_BACKGROUND,
    /* Geometry */
    JI_STYLE_PROP_PADDING,
    JI_STYLE_PROP_PADDING_LEFT,
    JI_STYLE_PROP_PADDING_TOP,
    JI_STYLE_PROP_PADDING_RIGHT,
    JI_STYLE_PROP_PADDING_BOTTOM,
    JI_STYLE_PROP_MARGIN,
    JI_STYLE_PROP_MARGIN_LEFT,
    JI_STYLE_PROP_MARGIN_TOP,
    JI_STYLE_PROP_MARGIN_RIGHT,
    JI_STYLE_PROP_MARGIN_BOTTOM,
    JI_STYLE_PROP_BORDER_RADIUS,
    JI_STYLE_PROP_BORDER_WIDTH,
    JI_STYLE_PROP_MIN_WIDTH,
    JI_STYLE_PROP_MIN_HEIGHT,
    JI_STYLE_PROP_MAX_WIDTH,
    JI_STYLE_PROP_MAX_HEIGHT,
    /* Typography */
    JI_STYLE_PROP_FONT_SIZE,
    JI_STYLE_PROP_FONT_WEIGHT,
    JI_STYLE_PROP_FONT_FAMILY,
    JI_STYLE_PROP_TEXT_ALIGN,
    JI_STYLE_PROP_LINE_HEIGHT,
    /* Effects */
    JI_STYLE_PROP_OPACITY,
    JI_STYLE_PROP_BOX_SHADOW_X,
    JI_STYLE_PROP_BOX_SHADOW_Y,
    JI_STYLE_PROP_BOX_SHADOW_BLUR,
    JI_STYLE_PROP_BOX_SHADOW_COLOR,
    /* Layout */
    JI_STYLE_PROP_SPACING,
    JI_STYLE_PROP_GAP,
    JI_STYLE_PROP_ALIGNMENT,
    /* Custom */
    JI_STYLE_PROP_CUSTOM = 256
} JiStyleProperty;

/* =========================================================================
 * Style Value — union of possible CSS value types
 * ========================================================================= */
typedef enum JiStyleValueType {
    JI_STYLE_VALUE_NONE = 0,
    JI_STYLE_VALUE_INT,
    JI_STYLE_VALUE_FLOAT,
    JI_STYLE_VALUE_COLOR,     /* ARGB uint32_t */
    JI_STYLE_VALUE_STRING,    /* heap-allocated string */
    JI_STYLE_VALUE_ENUM       /* integer enum value */
} JiStyleValueType;

typedef struct JiStyleValue {
    JiStyleValueType type;
    union {
        int         int_val;
        double      float_val;
        uint32_t    color_val;
        char*       string_val;
        int         enum_val;
    };
} JiStyleValue;

/* =========================================================================
 * CSS Pseudo-States (hover, pressed, disabled, etc.)
 * ========================================================================= */
typedef enum JiCSSPseudoState {
    JI_CSS_PSEUDO_NONE      = 0,
    JI_CSS_PSEUDO_HOVER     = 1 << 0,
    JI_CSS_PSEUDO_PRESSED   = 1 << 1,
    JI_CSS_PSEUDO_DISABLED  = 1 << 2,
    JI_CSS_PSEUDO_FOCUS     = 1 << 3,
    JI_CSS_PSEUDO_CHECKED   = 1 << 4,
    JI_CSS_PSEUDO_ACTIVE    = 1 << 5,
    JI_CSS_PSEUDO_VISITED   = 1 << 6,
    JI_CSS_PSEUDO_UNCHECKED = 1 << 7,
    JI_CSS_PSEUDO_SELECTED  = 1 << 8
} JiCSSPseudoState;

/* =========================================================================
 * Style Declaration — one property: value pair
 * ========================================================================= */
typedef struct JiStyleDeclaration {
    JiStyleProperty  property;
    JiStyleValue     value;
    bool             is_important;  /* !important flag */
} JiStyleDeclaration;

/* =========================================================================
 * Style Rule — selector + declarations
 * ========================================================================= */
typedef enum JiCSSSelectorType {
    JI_CSS_SELECTOR_TYPE,       /* JiButton */
    JI_CSS_SELECTOR_ID,         /* #myButton */
    JI_CSS_SELECTOR_CLASS,      /* .primary */
    JI_CSS_SELECTOR_CHILD,      /* JiGroupBox > JiButton */
    JI_CSS_SELECTOR_DESCENDANT, /* JiDockArea JiButton */
    JI_CSS_SELECTOR_UNIVERSAL   /* * */
} JiCSSSelectorType;

typedef struct JiCSSSelector {
    JiCSSSelectorType   type;
    char*              name;           /* type name, id, or class */
    JiCSSPseudoState   pseudo_state;   /* :hover, :pressed, etc. */
    struct JiCSSSelector* parent;      /* for child/descendant combinators */
    bool               is_direct_child;/* true for >, false for space */
} JiCSSSelector;

typedef struct JiStyleRule {
    JiCSSSelector**       selectors;
    int                   selector_count;
    int                   selector_capacity;
    JiStyleDeclaration*   declarations;
    int                   declaration_count;
    int                   declaration_capacity;
    int                   specificity;    /* computed specificity score */
} JiStyleRule;

/* =========================================================================
 * CSS Variable (Custom Property)
 * ========================================================================= */
typedef struct JiStyleVariable {
    char*  name;     /* e.g. "accent-color" */
    char*  value;    /* e.g. "#2196F3" */
} JiStyleVariable;

/* =========================================================================
 * Stylesheet — collection of rules and variables
 * ========================================================================= */
typedef struct JiStyleSheet {
    JiStyleRule**      rules;
    int                rule_count;
    int                rule_capacity;
    JiStyleVariable*   variables;
    int                variable_count;
    int                variable_capacity;
    bool               is_live_reload;   /* beyond Qt6: auto-reload on file change */
    char*              source_path;      /* file path for live reload */
    double             last_modified;    /* timestamp of last file modification */
} JiStyleSheet;

/* ---- Stylesheet Lifecycle ---- */
JI_API JiStyleSheet* ji_stylesheet_new(void);
JI_API void ji_stylesheet_destroy(JiStyleSheet* sheet);

/* ---- Parsing ---- */
JI_API bool ji_stylesheet_parse(JiStyleSheet* sheet, const char* css_text);
JI_API bool ji_stylesheet_parse_file(JiStyleSheet* sheet, const char* path);

/* ---- Variables ---- */
JI_API void ji_stylesheet_set_variable(JiStyleSheet* sheet, const char* name, const char* value);
JI_API const char* ji_stylesheet_get_variable(const JiStyleSheet* sheet, const char* name);

/* ---- Rule Access ---- */
JI_API int ji_stylesheet_rule_count(const JiStyleSheet* sheet);
JI_API const JiStyleRule* ji_stylesheet_get_rule(const JiStyleSheet* sheet, int index);

/* ---- Matching ---- */
JI_API bool ji_stylesheet_matches_widget(const JiStyleSheet* sheet,
                                          const char* widget_type,
                                          const char* widget_id,
                                          const char* widget_class,
                                          JiCSSPseudoState pseudo_state);

/* ---- Resolution — get the effective value for a property ---- */
JI_API bool ji_stylesheet_resolve_property(const JiStyleSheet* sheet,
                                            const char* widget_type,
                                            const char* widget_id,
                                            const char* widget_class,
                                            JiCSSPseudoState pseudo_state,
                                            JiStyleProperty property,
                                            JiStyleValue* out_value);

/* ---- Live Reload (beyond Qt6) ---- */
JI_API void ji_stylesheet_enable_live_reload(JiStyleSheet* sheet, bool enable);
JI_API bool ji_stylesheet_check_reload(JiStyleSheet* sheet);

/* ---- Apply to widget tree ---- */
JI_API void ji_stylesheet_apply(JiStyleSheet* sheet, void* root_widget);

/* =========================================================================
 * Style Color Helpers
 * ========================================================================= */
JI_API uint32_t ji_color_from_string(const char* str);  /* "#RRGGBB" or "#AARRGGBB" or named */
JI_API uint32_t ji_color_rgb(uint8_t r, uint8_t g, uint8_t b);
JI_API uint32_t ji_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
JI_API uint8_t  ji_color_alpha(uint32_t argb);
JI_API uint8_t  ji_color_red(uint32_t argb);
JI_API uint8_t  ji_color_green(uint32_t argb);
JI_API uint8_t  ji_color_blue(uint32_t argb);
JI_API uint32_t ji_color_with_alpha(uint32_t argb, uint8_t alpha);
JI_API uint32_t ji_color_lighten(uint32_t argb, double factor);
JI_API uint32_t ji_color_darken(uint32_t argb, double factor);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_STYLESHEET_H */
