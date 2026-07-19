/**
 * JiUI - Stylesheet Engine implementation
 * CSS-like declarative styling with selectors, variables, and live reload.
 */

#include <jiui/ji_stylesheet.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* =========================================================================
 * Color Helpers
 * ========================================================================= */
uint32_t ji_color_rgb(uint8_t r, uint8_t g, uint8_t b) { return 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
uint32_t ji_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
uint8_t ji_color_alpha(uint32_t argb) { return (argb >> 24) & 0xFF; }
uint8_t ji_color_red(uint32_t argb) { return (argb >> 16) & 0xFF; }
uint8_t ji_color_green(uint32_t argb) { return (argb >> 8) & 0xFF; }
uint8_t ji_color_blue(uint32_t argb) { return argb & 0xFF; }

uint32_t ji_color_with_alpha(uint32_t argb, uint8_t alpha) {
    return (argb & 0x00FFFFFF) | ((uint32_t)alpha << 24);
}

uint32_t ji_color_lighten(uint32_t argb, double factor) {
    uint8_t r = ji_color_red(argb), g = ji_color_green(argb), b = ji_color_blue(argb);
    r = (uint8_t)(r + (255 - r) * factor);
    g = (uint8_t)(g + (255 - g) * factor);
    b = (uint8_t)(b + (255 - b) * factor);
    return ji_color_with_alpha(ji_color_rgb(r, g, b), ji_color_alpha(argb));
}

uint32_t ji_color_darken(uint32_t argb, double factor) {
    uint8_t r = ji_color_red(argb), g = ji_color_green(argb), b = ji_color_blue(argb);
    r = (uint8_t)(r * (1.0 - factor));
    g = (uint8_t)(g * (1.0 - factor));
    b = (uint8_t)(b * (1.0 - factor));
    return ji_color_with_alpha(ji_color_rgb(r, g, b), ji_color_alpha(argb));
}

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return -1;
}

uint32_t ji_color_from_string(const char* str) {
    if (!str) return 0;
    /* Named colors */
    if (strcmp(str, "white") == 0) return 0xFFFFFFFF;
    if (strcmp(str, "black") == 0) return 0xFF000000;
    if (strcmp(str, "red") == 0) return 0xFFFF0000;
    if (strcmp(str, "green") == 0) return 0xFF008000;
    if (strcmp(str, "blue") == 0) return 0xFF0000FF;
    if (strcmp(str, "transparent") == 0) return 0x00000000;
    if (strcmp(str, "gray") == 0 || strcmp(str, "grey") == 0) return 0xFF808080;
    /* Hex */
    if (str[0] == '#') {
        int len = (int)strlen(str + 1);
        if (len == 6) { /* #RRGGBB */
            int r = (hex_digit(str[1]) << 4) | hex_digit(str[2]);
            int g = (hex_digit(str[3]) << 4) | hex_digit(str[4]);
            int b = (hex_digit(str[5]) << 4) | hex_digit(str[6]);
            if (r < 0 || g < 0 || b < 0) return 0;
            return ji_color_rgb((uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
        if (len == 8) { /* #AARRGGBB */
            int a = (hex_digit(str[1]) << 4) | hex_digit(str[2]);
            int r = (hex_digit(str[3]) << 4) | hex_digit(str[4]);
            int g = (hex_digit(str[5]) << 4) | hex_digit(str[6]);
            int b = (hex_digit(str[7]) << 4) | hex_digit(str[8]);
            if (a < 0 || r < 0 || g < 0 || b < 0) return 0;
            return ji_color_argb((uint8_t)a, (uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
    }
    return 0;
}

/* =========================================================================
 * Internal Parser State
 * ========================================================================= */
typedef struct JiCSSParser {
    const char*  text;
    int           pos;
    int           len;
    JiStyleSheet* sheet;
} JiCSSParser;

static char ji_css_peek(JiCSSParser* p) {
    return (p->pos < p->len) ? p->text[p->pos] : '\0';
}

static char ji_css_next(JiCSSParser* p) {
    return (p->pos < p->len) ? p->text[p->pos++] : '\0';
}

static void ji_css_skip_ws(JiCSSParser* p) {
    while (p->pos < p->len) {
        char c = p->text[p->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { p->pos++; continue; }
        if (c == '/' && p->pos + 1 < p->len && p->text[p->pos+1] == '*') {
            p->pos += 2;
            while (p->pos + 1 < p->len && !(p->text[p->pos] == '*' && p->text[p->pos+1] == '/')) p->pos++;
            if (p->pos + 1 < p->len) p->pos += 2;
            continue;
        }
        break;
    }
}

static char* ji_css_read_ident(JiCSSParser* p) {
    int start = p->pos;
    while (p->pos < p->len) {
        char c = p->text[p->pos];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-' ||
            (c >= '0' && c <= '9' && p->pos > start)) {
            p->pos++;
        } else break;
    }
    int len = p->pos - start;
    if (len == 0) return NULL;
    char* s = (char*)ji_alloc(len + 1);
    if (s) { memcpy(s, p->text + start, len); s[len] = '\0'; }
    return s;
}

static char* ji_css_read_value(JiCSSParser* p) {
    int start = p->pos;
    int depth = 0;
    while (p->pos < p->len) {
        char c = p->text[p->pos];
        if (c == '(') depth++;
        if (c == ')') depth--;
        if ((c == ';' || c == '}') && depth == 0) break;
        p->pos++;
    }
    int len = p->pos - start;
    /* trim trailing whitespace */
    while (len > 0 && (p->text[start + len - 1] == ' ' || p->text[start + len - 1] == '\t')) len--;
    char* s = (char*)ji_alloc(len + 1);
    if (s) { memcpy(s, p->text + start, len); s[len] = '\0'; }
    return s;
}

/* =========================================================================
 * Selector Parsing
 * ========================================================================= */
static JiCSSSelector* ji_selector_new(JiCSSSelectorType type, const char* name, JiCSSPseudoState pseudo) {
    JiCSSSelector* s = (JiCSSSelector*)ji_calloc(1, sizeof(JiCSSSelector));
    if (!s) return NULL;
    s->type = type;
    s->name = name ? (char*)ji_alloc(strlen(name) + 1) : NULL;
    if (s->name && name) memcpy(s->name, name, strlen(name) + 1);
    s->pseudo_state = pseudo;
    return s;
}

static void ji_selector_destroy(JiCSSSelector* s) {
    if (!s) return;
    ji_free(s->name);
    ji_selector_destroy(s->parent);
    ji_free(s);
}

static JiCSSPseudoState ji_css_parse_pseudo(JiCSSParser* p) {
    if (ji_css_peek(p) != ':') return JI_CSS_PSEUDO_NONE;
    p->pos++; /* skip ':' */
    char* name = ji_css_read_ident(p);
    JiCSSPseudoState ps = JI_CSS_PSEUDO_NONE;
    if (name) {
        if (strcmp(name, "hover") == 0) ps = JI_CSS_PSEUDO_HOVER;
        else if (strcmp(name, "pressed") == 0) ps = JI_CSS_PSEUDO_PRESSED;
        else if (strcmp(name, "disabled") == 0) ps = JI_CSS_PSEUDO_DISABLED;
        else if (strcmp(name, "focus") == 0) ps = JI_CSS_PSEUDO_FOCUS;
        else if (strcmp(name, "checked") == 0) ps = JI_CSS_PSEUDO_CHECKED;
        else if (strcmp(name, "active") == 0) ps = JI_CSS_PSEUDO_ACTIVE;
        else if (strcmp(name, "selected") == 0) ps = JI_CSS_PSEUDO_SELECTED;
        ji_free(name);
    }
    return ps;
}

static JiCSSSelector* ji_css_parse_selector(JiCSSParser* p) {
    ji_css_skip_ws(p);
    char c = ji_css_peek(p);
    JiCSSSelectorType type = JI_CSS_SELECTOR_TYPE;
    char* name = NULL;

    if (c == '#') { p->pos++; type = JI_CSS_SELECTOR_ID; name = ji_css_read_ident(p); }
    else if (c == '.') { p->pos++; type = JI_CSS_SELECTOR_CLASS; name = ji_css_read_ident(p); }
    else if (c == '*') { p->pos++; type = JI_CSS_SELECTOR_UNIVERSAL; name = NULL; }
    else { type = JI_CSS_SELECTOR_TYPE; name = ji_css_read_ident(p); }

    JiCSSPseudoState pseudo = ji_css_parse_pseudo(p);
    JiCSSSelector* sel = ji_selector_new(type, name, pseudo);
    ji_free(name);
    return sel;
}

/* =========================================================================
 * Property Name Mapping
 * ========================================================================= */
typedef struct { const char* name; JiStyleProperty prop; } JiPropMap;

static const JiPropMap s_prop_map[] = {
    {"background-color", JI_STYLE_PROP_BACKGROUND_COLOR}, {"background", JI_STYLE_PROP_BACKGROUND_COLOR},
    {"color", JI_STYLE_PROP_COLOR}, {"border-color", JI_STYLE_PROP_BORDER_COLOR},
    {"border-radius", JI_STYLE_PROP_BORDER_RADIUS}, {"border-width", JI_STYLE_PROP_BORDER_WIDTH},
    {"padding", JI_STYLE_PROP_PADDING}, {"padding-left", JI_STYLE_PROP_PADDING_LEFT},
    {"padding-top", JI_STYLE_PROP_PADDING_TOP}, {"padding-right", JI_STYLE_PROP_PADDING_RIGHT},
    {"padding-bottom", JI_STYLE_PROP_PADDING_BOTTOM},
    {"margin", JI_STYLE_PROP_MARGIN}, {"margin-left", JI_STYLE_PROP_MARGIN_LEFT},
    {"margin-top", JI_STYLE_PROP_MARGIN_TOP}, {"margin-right", JI_STYLE_PROP_MARGIN_RIGHT},
    {"margin-bottom", JI_STYLE_PROP_MARGIN_BOTTOM},
    {"min-width", JI_STYLE_PROP_MIN_WIDTH}, {"min-height", JI_STYLE_PROP_MIN_HEIGHT},
    {"max-width", JI_STYLE_PROP_MAX_WIDTH}, {"max-height", JI_STYLE_PROP_MAX_HEIGHT},
    {"font-size", JI_STYLE_PROP_FONT_SIZE}, {"font-weight", JI_STYLE_PROP_FONT_WEIGHT},
    {"font-family", JI_STYLE_PROP_FONT_FAMILY}, {"text-align", JI_STYLE_PROP_TEXT_ALIGN},
    {"line-height", JI_STYLE_PROP_LINE_HEIGHT},
    {"opacity", JI_STYLE_PROP_OPACITY},
    {"selection-color", JI_STYLE_PROP_SELECTION_COLOR}, {"selection-background-color", JI_STYLE_PROP_SELECTION_BACKGROUND},
    {"spacing", JI_STYLE_PROP_SPACING}, {"gap", JI_STYLE_PROP_GAP},
    {NULL, JI_STYLE_PROP_NONE}
};

static JiStyleProperty ji_lookup_property(const char* name) {
    for (int i = 0; s_prop_map[i].name; i++) {
        if (strcmp(s_prop_map[i].name, name) == 0) return s_prop_map[i].prop;
    }
    return JI_STYLE_PROP_NONE;
}

/* =========================================================================
 * Value Parsing
 * ========================================================================= */
static JiStyleValue ji_parse_value(const char* str) {
    JiStyleValue v = { JI_STYLE_VALUE_NONE, {0} };
    if (!str) return v;
    /* Try integer */
    char* end;
    long lval = strtol(str, &end, 10);
    if (*end == '\0' || strcmp(end, "px") == 0) {
        v.type = JI_STYLE_VALUE_INT; v.int_val = (int)lval; return v;
    }
    /* Try float */
    double dval = strtod(str, &end);
    if (*end == '\0' || strcmp(end, "em") == 0 || strcmp(end, "%") == 0) {
        v.type = JI_STYLE_VALUE_FLOAT; v.float_val = dval; return v;
    }
    /* Try color */
    if (str[0] == '#' || strncmp(str, "rgb", 3) == 0) {
        v.type = JI_STYLE_VALUE_COLOR; v.color_val = ji_color_from_string(str); return v;
    }
    /* Named color */
    uint32_t c = ji_color_from_string(str);
    if (c != 0 || strcmp(str, "black") == 0 || strcmp(str, "transparent") == 0) {
        v.type = JI_STYLE_VALUE_COLOR; v.color_val = c; return v;
    }
    /* String */
    v.type = JI_STYLE_VALUE_STRING;
    size_t len = strlen(str);
    v.string_val = (char*)ji_alloc(len + 1);
    if (v.string_val) memcpy(v.string_val, str, len + 1);
    return v;
}

/* =========================================================================
 * Rule Parsing
 * ========================================================================= */
static void ji_css_parse_declarations(JiCSSParser* p, JiStyleRule* rule) {
    while (p->pos < p->len && ji_css_peek(p) != '}') {
        ji_css_skip_ws(p);
        char* prop_name = ji_css_read_ident(p);
        if (!prop_name) { if (p->pos < p->len) p->pos++; continue; }
        ji_css_skip_ws(p);
        if (ji_css_peek(p) == ':') p->pos++; /* skip ':' */
        ji_css_skip_ws(p);
        char* val_str = ji_css_read_value(p);
        if (ji_css_peek(p) == ';') p->pos++;
        if (!val_str) { ji_free(prop_name); continue; }
        /* Check !important */
        bool important = false;
        int vlen = (int)strlen(val_str);
        if (vlen >= 10 && strcmp(val_str + vlen - 10, "!important") == 0) {
            important = true;
            val_str[vlen - 10] = '\0';
        }
        JiStyleProperty prop = ji_lookup_property(prop_name);
        if (prop != JI_STYLE_PROP_NONE) {
            if (rule->declaration_count >= rule->declaration_capacity) {
                rule->declaration_capacity = rule->declaration_capacity ? rule->declaration_capacity * 2 : 4;
                JiStyleDeclaration* new_arr = (JiStyleDeclaration*)ji_alloc(sizeof(JiStyleDeclaration) * rule->declaration_capacity);
                if (new_arr) { memcpy(new_arr, rule->declarations, sizeof(JiStyleDeclaration) * rule->declaration_count); ji_free(rule->declarations); rule->declarations = new_arr; }
            }
            JiStyleDeclaration* d = &rule->declarations[rule->declaration_count++];
            d->property = prop;
            d->value = ji_parse_value(val_str);
            d->is_important = important;
        }
        ji_free(prop_name);
        ji_free(val_str);
    }
}

static int ji_compute_specificity(JiStyleRule* rule) {
    int spec = 0;
    for (int i = 0; i < rule->selector_count; i++) {
        JiCSSSelector* s = rule->selectors[i];
        switch (s->type) {
            case JI_CSS_SELECTOR_ID: spec += 100; break;
            case JI_CSS_SELECTOR_CLASS: spec += 10; break;
            case JI_CSS_SELECTOR_TYPE: spec += 1; break;
            case JI_CSS_SELECTOR_UNIVERSAL: spec += 0; break;
            default: spec += 1; break;
        }
        if (s->pseudo_state != JI_CSS_PSEUDO_NONE) spec += 10;
    }
    return spec;
}

/* =========================================================================
 * Stylesheet Lifecycle
 * ========================================================================= */
JiStyleSheet* ji_stylesheet_new(void) {
    JiStyleSheet* s = (JiStyleSheet*)ji_calloc(1, sizeof(JiStyleSheet));
    if (!s) { JI_ERROR_LOG("ji_stylesheet_new: out of memory"); return NULL; }
    s->rule_capacity = 8;
    s->rules = (JiStyleRule**)ji_calloc(s->rule_capacity, sizeof(JiStyleRule*));
    s->variable_capacity = 8;
    s->variables = (JiStyleVariable*)ji_calloc(s->variable_capacity, sizeof(JiStyleVariable));
    return s;
}

static void ji_style_rule_destroy(JiStyleRule* rule) {
    if (!rule) return;
    for (int i = 0; i < rule->selector_count; i++) ji_selector_destroy(rule->selectors[i]);
    ji_free(rule->selectors);
    for (int i = 0; i < rule->declaration_count; i++) {
        if (rule->declarations[i].value.type == JI_STYLE_VALUE_STRING)
            ji_free(rule->declarations[i].value.string_val);
    }
    ji_free(rule->declarations);
    ji_free(rule);
}

void ji_stylesheet_destroy(JiStyleSheet* sheet) {
    if (!sheet) return;
    for (int i = 0; i < sheet->rule_count; i++) ji_style_rule_destroy(sheet->rules[i]);
    ji_free(sheet->rules);
    for (int i = 0; i < sheet->variable_count; i++) {
        ji_free(sheet->variables[i].name);
        ji_free(sheet->variables[i].value);
    }
    ji_free(sheet->variables);
    ji_free(sheet->source_path);
    ji_free(sheet);
}

/* =========================================================================
 * Parsing
 * ========================================================================= */
bool ji_stylesheet_parse(JiStyleSheet* sheet, const char* css_text) {
    if (!sheet || !css_text) return false;
    JiCSSParser p = { css_text, 0, (int)strlen(css_text), sheet };

    while (p.pos < p.len) {
        ji_css_skip_ws(&p);
        if (p.pos >= p.len) break;

        /* Variable: @name: value; */
        if (ji_css_peek(&p) == '@') {
            p.pos++; /* skip @ */
            char* var_name = ji_css_read_ident(&p);
            ji_css_skip_ws(&p);
            if (ji_css_peek(&p) == ':') p.pos++;
            ji_css_skip_ws(&p);
            char* var_val = ji_css_read_value(&p);
            if (ji_css_peek(&p) == ';') p.pos++;
            if (var_name && var_val) ji_stylesheet_set_variable(sheet, var_name, var_val);
            ji_free(var_name);
            ji_free(var_val);
            continue;
        }

        /* Parse selectors */
        JiStyleRule* rule = (JiStyleRule*)ji_calloc(1, sizeof(JiStyleRule));
        if (!rule) break;
        rule->selector_capacity = 4;
        rule->selectors = (JiCSSSelector**)ji_calloc(rule->selector_capacity, sizeof(JiCSSSelector*));
        rule->declaration_capacity = 4;
        rule->declarations = (JiStyleDeclaration*)ji_calloc(rule->declaration_capacity, sizeof(JiStyleDeclaration));

        /* Read selectors until { */
        while (p.pos < p.len && ji_css_peek(&p) != '{') {
            JiCSSSelector* sel = ji_css_parse_selector(&p);
            if (sel) {
                if (rule->selector_count >= rule->selector_capacity) {
                    rule->selector_capacity *= 2;
                    JiCSSSelector** new_arr = (JiCSSSelector**)ji_alloc(sizeof(JiCSSSelector*) * rule->selector_capacity);
                    if (new_arr) { memcpy(new_arr, rule->selectors, sizeof(JiCSSSelector*) * rule->selector_count); ji_free(rule->selectors); rule->selectors = new_arr; }
                }
                rule->selectors[rule->selector_count++] = sel;
            }
            ji_css_skip_ws(&p);
            if (ji_css_peek(&p) == ',') { p.pos++; continue; }
        }

        if (ji_css_peek(&p) == '{') {
            p.pos++; /* skip { */
            ji_css_parse_declarations(&p, rule);
            if (ji_css_peek(&p) == '}') p.pos++; /* skip } */
        }

        rule->specificity = ji_compute_specificity(rule);

        /* Add rule to sheet */
        if (sheet->rule_count >= sheet->rule_capacity) {
            sheet->rule_capacity *= 2;
            JiStyleRule** new_arr = (JiStyleRule**)ji_alloc(sizeof(JiStyleRule*) * sheet->rule_capacity);
            if (new_arr) { memcpy(new_arr, sheet->rules, sizeof(JiStyleRule*) * sheet->rule_count); ji_free(sheet->rules); sheet->rules = new_arr; }
        }
        sheet->rules[sheet->rule_count++] = rule;
    }
    return true;
}

bool ji_stylesheet_parse_file(JiStyleSheet* sheet, const char* path) {
    if (!sheet || !path) return false;
    FILE* f = fopen(path, "r");
    if (!f) { JI_ERROR_LOG("ji_stylesheet_parse_file: cannot open %s", path); return false; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)ji_alloc(size + 1);
    if (!buf) { fclose(f); return false; }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    bool result = ji_stylesheet_parse(sheet, buf);
    ji_free(buf);
    /* Store path for live reload */
    ji_free(sheet->source_path);
    size_t plen = strlen(path);
    sheet->source_path = (char*)ji_alloc(plen + 1);
    if (sheet->source_path) memcpy(sheet->source_path, path, plen + 1);
    return result;
}

/* =========================================================================
 * Variables
 * ========================================================================= */
void ji_stylesheet_set_variable(JiStyleSheet* sheet, const char* name, const char* value) {
    if (!sheet || !name) return;
    /* Update existing */
    for (int i = 0; i < sheet->variable_count; i++) {
        if (strcmp(sheet->variables[i].name, name) == 0) {
            ji_free(sheet->variables[i].value);
            size_t vlen = strlen(value);
            sheet->variables[i].value = (char*)ji_alloc(vlen + 1);
            if (sheet->variables[i].value) memcpy(sheet->variables[i].value, value, vlen + 1);
            return;
        }
    }
    /* Add new */
    if (sheet->variable_count >= sheet->variable_capacity) {
        sheet->variable_capacity *= 2;
        JiStyleVariable* new_arr = (JiStyleVariable*)ji_alloc(sizeof(JiStyleVariable) * sheet->variable_capacity);
        if (new_arr) { memcpy(new_arr, sheet->variables, sizeof(JiStyleVariable) * sheet->variable_count); ji_free(sheet->variables); sheet->variables = new_arr; }
    }
    JiStyleVariable* v = &sheet->variables[sheet->variable_count++];
    size_t nlen = strlen(name);
    v->name = (char*)ji_alloc(nlen + 1);
    if (v->name) memcpy(v->name, name, nlen + 1);
    size_t vlen = strlen(value);
    v->value = (char*)ji_alloc(vlen + 1);
    if (v->value) memcpy(v->value, value, vlen + 1);
}

const char* ji_stylesheet_get_variable(const JiStyleSheet* sheet, const char* name) {
    if (!sheet || !name) return NULL;
    for (int i = 0; i < sheet->variable_count; i++) {
        if (strcmp(sheet->variables[i].name, name) == 0) return sheet->variables[i].value;
    }
    return NULL;
}

/* =========================================================================
 * Rule Access
 * ========================================================================= */
int ji_stylesheet_rule_count(const JiStyleSheet* sheet) { return sheet ? sheet->rule_count : 0; }
const JiStyleRule* ji_stylesheet_get_rule(const JiStyleSheet* sheet, int index) {
    return (sheet && index >= 0 && index < sheet->rule_count) ? sheet->rules[index] : NULL;
}

/* =========================================================================
 * Matching
 * ========================================================================= */
static bool ji_selector_matches(const JiCSSSelector* sel, const char* widget_type,
                                  const char* widget_id, const char* widget_class,
                                  JiCSSPseudoState pseudo_state) {
    if (!sel) return false;
    switch (sel->type) {
        case JI_CSS_SELECTOR_TYPE:
            return widget_type && strcmp(sel->name, widget_type) == 0;
        case JI_CSS_SELECTOR_ID:
            return widget_id && strcmp(sel->name, widget_id) == 0;
        case JI_CSS_SELECTOR_CLASS:
            return widget_class && strcmp(sel->name, widget_class) == 0;
        case JI_CSS_SELECTOR_UNIVERSAL:
            return true;
        default:
            return false;
    }
    (void)pseudo_state;
}

bool ji_stylesheet_matches_widget(const JiStyleSheet* sheet,
                                    const char* widget_type,
                                    const char* widget_id,
                                    const char* widget_class,
                                    JiCSSPseudoState pseudo_state) {
    if (!sheet) return false;
    for (int i = 0; i < sheet->rule_count; i++) {
        JiStyleRule* rule = sheet->rules[i];
        for (int j = 0; j < rule->selector_count; j++) {
            JiCSSSelector* sel = rule->selectors[j];
            if (ji_selector_matches(sel, widget_type, widget_id, widget_class, pseudo_state)) {
                if (sel->pseudo_state == JI_CSS_PSEUDO_NONE || (sel->pseudo_state & pseudo_state))
                    return true;
            }
        }
    }
    return false;
}

/* =========================================================================
 * Resolution
 * ========================================================================= */
bool ji_stylesheet_resolve_property(const JiStyleSheet* sheet,
                                      const char* widget_type,
                                      const char* widget_id,
                                      const char* widget_class,
                                      JiCSSPseudoState pseudo_state,
                                      JiStyleProperty property,
                                      JiStyleValue* out_value) {
    if (!sheet || !out_value) return false;
    /* Find highest-specificity rule that matches and has the property */
    JiStyleDeclaration* best = NULL;
    int best_spec = -1;
    for (int i = 0; i < sheet->rule_count; i++) {
        JiStyleRule* rule = sheet->rules[i];
        bool matches = false;
        for (int j = 0; j < rule->selector_count; j++) {
            JiCSSSelector* sel = rule->selectors[j];
            if (ji_selector_matches(sel, widget_type, widget_id, widget_class, pseudo_state)) {
                if (sel->pseudo_state == JI_CSS_PSEUDO_NONE || (sel->pseudo_state & pseudo_state))
                    matches = true;
            }
        }
        if (!matches) continue;
        for (int j = 0; j < rule->declaration_count; j++) {
            if (rule->declarations[j].property == property) {
                int spec = rule->specificity;
                if (rule->declarations[j].is_important) spec += 10000;
                if (spec > best_spec) { best = &rule->declarations[j]; best_spec = spec; }
            }
        }
    }
    if (best) { *out_value = best->value; return true; }
    return false;
}

/* =========================================================================
 * Live Reload
 * ========================================================================= */
void ji_stylesheet_enable_live_reload(JiStyleSheet* sheet, bool enable) {
    if (sheet) sheet->is_live_reload = enable;
}

bool ji_stylesheet_check_reload(JiStyleSheet* sheet) {
    if (!sheet || !sheet->is_live_reload || !sheet->source_path) return false;
    /* Check file modification time */
    FILE* f = fopen(sheet->source_path, "r");
    if (!f) return false;
    fclose(f);
    /* In a real implementation, stat() the file and compare mtime */
    return false;
}

/* =========================================================================
 * Apply (stub — real implementation walks widget tree)
 * ========================================================================= */
void ji_stylesheet_apply(JiStyleSheet* sheet, void* root_widget) {
    (void)sheet; (void)root_widget;
    /* Walk the widget tree and apply matched properties */
}
