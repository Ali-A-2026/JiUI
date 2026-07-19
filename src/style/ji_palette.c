/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_palette.c
 * @brief Qt6-style palette implementation — dark/light palettes, theme engine.
 */

#include <jiui/ji_palette.h>
#include <jiui/ji_memory.h>
#include <jiui/ji_error.h>
#include <string.h>

/* ---- ARGB helper macros ---- */
#define ARGB(a, r, g, b) (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | \
                           ((uint32_t)(g) << 8) | (uint32_t)(b))

#define RGB(r, g, b) ARGB(0xFF, r, g, b)

/* ---- Palette lifecycle ---- */

JiPalette* ji_palette_new(void) {
    JiPalette* pal = (JiPalette*)ji_calloc(1, sizeof(JiPalette));
    if (!pal) {
        JI_ERROR_LOG("ji_palette_new: out of memory");
        return NULL;
    }
    return pal;
}

void ji_palette_destroy(JiPalette* pal) {
    ji_free(pal);
}

JiPalette* ji_palette_copy(const JiPalette* src) {
    if (!src) return NULL;
    JiPalette* dst = ji_palette_new();
    if (!dst) return NULL;
    memcpy(dst->colors, src->colors, sizeof(dst->colors));
    return dst;
}

/* ---- Color access ---- */

void ji_palette_set_color(JiPalette* pal, JiColorGroup group,
                           JiColorRole role, uint32_t argb) {
    if (!pal || group < 0 || group >= JI_COLOR_GROUP_COUNT ||
        role < 0 || role >= JI_ROLE_COUNT)
        return;
    pal->colors[group][role] = argb;
}

uint32_t ji_palette_get_color(const JiPalette* pal,
                                 JiColorGroup group, JiColorRole role) {
    if (!pal || role < 0 || role >= JI_ROLE_COUNT)
        return 0;
    if (group < 0 || group >= JI_COLOR_GROUP_COUNT)
        group = JI_COLOR_GROUP_ACTIVE;

    /* Fall back to Active group if the requested group's color is 0 */
    uint32_t c = pal->colors[group][role];
    if (c == 0 && group != JI_COLOR_GROUP_ACTIVE)
        c = pal->colors[JI_COLOR_GROUP_ACTIVE][role];
    return c;
}

uint32_t ji_palette_color(const JiPalette* pal, JiColorRole role) {
    return ji_palette_get_color(pal, JI_COLOR_GROUP_ACTIVE, role);
}

/* ---- Bulk operations ---- */

void ji_palette_set_color_all(JiPalette* pal, JiColorRole role, uint32_t argb) {
    if (!pal) return;
    for (int g = 0; g < JI_COLOR_GROUP_COUNT; g++)
        pal->colors[g][role] = argb;
}

uint32_t ji_palette_resolve_disabled(uint32_t active_color) {
    /* Extract channels */
    uint8_t a = (uint8_t)((active_color >> 24) & 0xFF);
    uint8_t r = (uint8_t)((active_color >> 16) & 0xFF);
    uint8_t g = (uint8_t)((active_color >> 8) & 0xFF);
    uint8_t b = (uint8_t)(active_color & 0xFF);

    /* Blend toward gray with 60% opacity of original */
    r = (uint8_t)(r * 0.4 + 128 * 0.6);
    g = (uint8_t)(g * 0.4 + 128 * 0.6);
    b = (uint8_t)(b * 0.4 + 128 * 0.6);
    a = (uint8_t)(a * 0.7);

    return ARGB(a, r, g, b);
}

/* ---- Helper: fill disabled group from active group ---- */
static void fill_disabled(JiPalette* pal) {
    for (int r = 0; r < JI_ROLE_COUNT; r++) {
        uint32_t ac = pal->colors[JI_COLOR_GROUP_ACTIVE][r];
        if (ac != 0)
            pal->colors[JI_COLOR_GROUP_DISABLED][r] = ji_palette_resolve_disabled(ac);
    }
}

/* ---- Helper: fill inactive group from active group ---- */
static void fill_inactive(JiPalette* pal) {
    for (int r = 0; r < JI_ROLE_COUNT; r++) {
        uint32_t ac = pal->colors[JI_COLOR_GROUP_ACTIVE][r];
        pal->colors[JI_COLOR_GROUP_INACTIVE][r] = ac;
    }
    /* Inactive window text is slightly dimmer */
    uint32_t wt = pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_WINDOW_TEXT];
    if (wt != 0) {
        uint8_t a = (uint8_t)((wt >> 24) & 0xFF);
        uint8_t r = (uint8_t)((wt >> 16) & 0xFF);
        uint8_t g = (uint8_t)((wt >> 8) & 0xFF);
        uint8_t b = (uint8_t)(wt & 0xFF);
        r = (uint8_t)(r * 0.85);
        g = (uint8_t)(g * 0.85);
        b = (uint8_t)(b * 0.85);
        pal->colors[JI_COLOR_GROUP_INACTIVE][JI_ROLE_WINDOW_TEXT] = ARGB(a, r, g, b);
    }
}

/* =========================================================================
 * Qt6 Fusion Dark Palette
 * ========================================================================= */
JiPalette* ji_qt_palette_dark(void) {
    JiPalette* pal = ji_palette_new();
    if (!pal) return NULL;

    /* Active group — Qt6 Fusion Dark */
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_WINDOW]           = RGB(0x1e, 0x1e, 0x1e);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_WINDOW_TEXT]     = RGB(0xe0, 0xe0, 0xe0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BASE]            = RGB(0x2d, 0x2d, 0x2d);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_ALTERNATE_BASE]   = RGB(0x35, 0x35, 0x35);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TEXT]            = RGB(0xe0, 0xe0, 0xe0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BUTTON]          = RGB(0x3d, 0x3d, 0x3d);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BUTTON_TEXT]     = RGB(0xe0, 0xe0, 0xe0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BRIGHT_TEXT]      = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_HIGHLIGHT]       = RGB(0x2d, 0x5f, 0x8f);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_HIGHLIGHTED_TEXT] = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LINK]            = RGB(0x4a, 0x90, 0xd9);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LINK_VISITED]    = RGB(0x7b, 0x68, 0xee);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_PLACEHOLDER_TEXT] = RGB(0x80, 0x80, 0x80);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_ACCENT]          = RGB(0x2d, 0x5f, 0x8f);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LIGHT]           = RGB(0x4a, 0x4a, 0x4a);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_MIDLIGHT]        = RGB(0x40, 0x40, 0x40);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_MID]             = RGB(0x2a, 0x2a, 0x2a);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_DARK]            = RGB(0x1a, 0x1a, 0x1a);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_SHADOW]          = RGB(0x10, 0x10, 0x10);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TOOL_TIP_BASE]   = RGB(0x1e, 0x1e, 0x1e);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TOOL_TIP_TEXT]   = RGB(0xe0, 0xe0, 0xe0);

    fill_disabled(pal);
    fill_inactive(pal);

    return pal;
}

/* =========================================================================
 * Qt6 Fusion Light Palette
 * ========================================================================= */
JiPalette* ji_qt_palette_light(void) {
    JiPalette* pal = ji_palette_new();
    if (!pal) return NULL;

    /* Active group — Qt6 Fusion Light */
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_WINDOW]           = RGB(0xf0, 0xf0, 0xf0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_WINDOW_TEXT]     = RGB(0x20, 0x20, 0x20);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BASE]            = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_ALTERNATE_BASE]   = RGB(0xf7, 0xf7, 0xf7);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TEXT]            = RGB(0x20, 0x20, 0x20);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BUTTON]          = RGB(0xe0, 0xe0, 0xe0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BUTTON_TEXT]     = RGB(0x20, 0x20, 0x20);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_BRIGHT_TEXT]      = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_HIGHLIGHT]       = RGB(0x00, 0x78, 0xd4);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_HIGHLIGHTED_TEXT] = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LINK]            = RGB(0x00, 0x66, 0xcc);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LINK_VISITED]    = RGB(0x66, 0x33, 0x99);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_PLACEHOLDER_TEXT] = RGB(0xa0, 0xa0, 0xa0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_ACCENT]          = RGB(0x00, 0x78, 0xd4);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_LIGHT]           = RGB(0xff, 0xff, 0xff);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_MIDLIGHT]        = RGB(0xf0, 0xf0, 0xf0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_MID]             = RGB(0xc0, 0xc0, 0xc0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_DARK]            = RGB(0xa0, 0xa0, 0xa0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_SHADOW]          = RGB(0x80, 0x80, 0x80);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TOOL_TIP_BASE]   = RGB(0xff, 0xff, 0xe0);
    pal->colors[JI_COLOR_GROUP_ACTIVE][JI_ROLE_TOOL_TIP_TEXT]   = RGB(0x20, 0x20, 0x20);

    fill_disabled(pal);
    fill_inactive(pal);

    return pal;
}

JiPalette* ji_qt_palette_system(void) {
    /* Default to dark theme for now.
     * TODO: detect system dark/light via X11 _GTK_THEME_VARIANT property. */
    return ji_qt_palette_dark();
}

/* =========================================================================
 * Theme Engine
 * ========================================================================= */
static JiThemeEngine* g_theme_engine = NULL;

JiThemeEngine* ji_theme_engine_new(void) {
    JiThemeEngine* engine = (JiThemeEngine*)ji_calloc(1, sizeof(JiThemeEngine));
    if (!engine) {
        JI_ERROR_LOG("ji_theme_engine_new: out of memory");
        return NULL;
    }
    engine->variant = JI_THEME_SYSTEM;
    engine->palette = ji_qt_palette_system();
    engine->palette_owned = true;
    return engine;
}

void ji_theme_engine_destroy(JiThemeEngine* engine) {
    if (!engine) return;
    if (engine->palette_owned && engine->palette)
        ji_palette_destroy(engine->palette);
    ji_free(engine);
}

void ji_theme_engine_set_variant(JiThemeEngine* engine, JiThemeVariant variant) {
    if (!engine) return;
    if (engine->variant == variant) return;
    engine->variant = variant;

    /* Replace palette */
    if (engine->palette_owned && engine->palette)
        ji_palette_destroy(engine->palette);

    switch (variant) {
        case JI_THEME_DARK:
            engine->palette = ji_qt_palette_dark();
            break;
        case JI_THEME_LIGHT:
            engine->palette = ji_qt_palette_light();
            break;
        case JI_THEME_SYSTEM:
        default:
            engine->palette = ji_qt_palette_system();
            break;
    }
    engine->palette_owned = true;

    JI_INFO("Theme variant changed to %s",
           variant == JI_THEME_DARK ? "dark" :
           variant == JI_THEME_LIGHT ? "light" : "system");
}

JiThemeVariant ji_theme_engine_get_variant(const JiThemeEngine* engine) {
    return engine ? engine->variant : JI_THEME_SYSTEM;
}

const JiPalette* ji_theme_engine_get_palette(const JiThemeEngine* engine) {
    return engine ? engine->palette : NULL;
}

void ji_theme_engine_set_palette(JiThemeEngine* engine, JiPalette* pal, bool own) {
    if (!engine) return;
    if (engine->palette_owned && engine->palette)
        ji_palette_destroy(engine->palette);
    engine->palette = pal;
    engine->palette_owned = own;
}

JiThemeEngine* ji_theme_engine_get_global(void) {
    return g_theme_engine;
}

void ji_theme_engine_set_global(JiThemeEngine* engine) {
    if (g_theme_engine && g_theme_engine != engine)
        ji_theme_engine_destroy(g_theme_engine);
    g_theme_engine = engine;
}

void ji_theme_engine_init(void) {
    if (!g_theme_engine) {
        g_theme_engine = ji_theme_engine_new();
        JI_INFO("Theme engine initialized (variant: system/dark)");
    }
}

void ji_theme_engine_shutdown(void) {
    if (g_theme_engine) {
        ji_theme_engine_destroy(g_theme_engine);
        g_theme_engine = NULL;
    }
}
