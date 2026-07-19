/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_palette.h
 * @brief Qt6-style color palette system — color roles, color groups,
 *        and palette management for dark/light theme support.
 *
 * Mirrors Qt6's QPalette API. Each palette contains colors for three
 * groups (Active, Disabled, Inactive) across multiple color roles
 * (Window, WindowText, Base, Text, Button, etc.).
 */

#ifndef JIUI_PALETTE_H
#define JIUI_PALETTE_H

#include "ji_defines.h"
#include "ji_api.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * JiColorGroup — matches Qt6 QPalette::ColorGroup
 * ========================================================================= */
typedef enum JiColorGroup {
    JI_COLOR_GROUP_ACTIVE    = 0,
    JI_COLOR_GROUP_DISABLED  = 1,
    JI_COLOR_GROUP_INACTIVE  = 2,
    JI_COLOR_GROUP_COUNT     = 3
} JiColorGroup;

/* =========================================================================
 * JiColorRole — matches Qt6 QPalette::ColorRole
 * ========================================================================= */
typedef enum JiColorRole {
    JI_ROLE_WINDOW          = 0,   /* general background */
    JI_ROLE_WINDOW_TEXT    = 1,   /* general foreground */
    JI_ROLE_BASE           = 2,   /* input background (text fields, list views) */
    JI_ROLE_ALTERNATE_BASE  = 3,   /* alternate row background */
    JI_ROLE_TOOL_TIP_BASE   = 4,   /* tooltip background */
    JI_ROLE_TOOL_TIP_TEXT   = 5,   /* tooltip text */
    JI_ROLE_TEXT           = 6,   /* text on base background */
    JI_ROLE_BUTTON         = 7,   /* button background */
    JI_ROLE_BUTTON_TEXT    = 8,   /* button text */
    JI_ROLE_BRIGHT_TEXT     = 9,   /* bright contrast text (e.g. on highlight) */
    JI_ROLE_HIGHLIGHT      = 10,  /* selection background */
    JI_ROLE_HIGHLIGHTED_TEXT= 11,  /* selection text */
    JI_ROLE_LINK           = 12,  /* hyperlink color */
    JI_ROLE_LINK_VISITED   = 13,  /* visited hyperlink color */
    JI_ROLE_PLACEHOLDER_TEXT= 14,  /* placeholder text in inputs */
    JI_ROLE_ACCENT         = 15,  /* accent color (Qt6.6+) */
    /* Mid-tone shades for 3D effects */
    JI_ROLE_LIGHT          = 16,  /* lighter than button */
    JI_ROLE_MIDLIGHT        = 17,  /* between button and light */
    JI_ROLE_MID            = 18,  /* between button and dark */
    JI_ROLE_DARK           = 19,  /* darker than button */
    JI_ROLE_SHADOW         = 20,  /* shadow color */
    JI_ROLE_COUNT          = 21
} JiColorRole;

/* =========================================================================
 * JiPalette — color table for all groups and roles
 * ========================================================================= */
typedef struct JiPalette {
    /* colors[group][role] = ARGB uint32_t */
    uint32_t colors[JI_COLOR_GROUP_COUNT][JI_ROLE_COUNT];
} JiPalette;

/* ---- Palette lifecycle ---- */

/** Create a new palette with all colors initialized to 0 (transparent). */
JI_API JiPalette* ji_palette_new(void);

/** Destroy a palette. */
JI_API void ji_palette_destroy(JiPalette* pal);

/** Copy a palette. */
JI_API JiPalette* ji_palette_copy(const JiPalette* src);

/* ---- Color access ---- */

/** Set a color for a specific group and role. */
JI_API void ji_palette_set_color(JiPalette* pal, JiColorGroup group,
                                   JiColorRole role, uint32_t argb);

/** Get a color for a specific group and role.
 *  Falls back to Active group if the requested group's color is 0. */
JI_API uint32_t ji_palette_get_color(const JiPalette* pal,
                                       JiColorGroup group, JiColorRole role);

/** Get a color from the Active group. */
JI_API uint32_t ji_palette_color(const JiPalette* pal, JiColorRole role);

/* ---- Bulk operations ---- */

/** Set a color for all three groups at once. */
JI_API void ji_palette_set_color_all(JiPalette* pal, JiColorRole role, uint32_t argb);

/** Resolve a disabled color from an active color (darken/mute). */
JI_API uint32_t ji_palette_resolve_disabled(uint32_t active_color);

/* ---- Built-in Qt6 palettes ---- */

/** Create a palette matching Qt6 Fusion Dark theme. Caller must destroy. */
JI_API JiPalette* ji_qt_palette_dark(void);

/** Create a palette matching Qt6 Fusion Light theme. Caller must destroy. */
JI_API JiPalette* ji_qt_palette_light(void);

/** Create a palette matching the system theme (auto-detect dark/light).
 *  Currently returns the dark palette as default. */
JI_API JiPalette* ji_qt_palette_system(void);

/* =========================================================================
 * JiThemeVariant — dark/light/system theme selection
 * ========================================================================= */
typedef enum JiThemeVariant {
    JI_THEME_DARK    = 0,
    JI_THEME_LIGHT   = 1,
    JI_THEME_SYSTEM  = 2
} JiThemeVariant;

/* =========================================================================
 * JiThemeEngine — manages current theme variant and active palette
 * ========================================================================= */
typedef struct JiThemeEngine {
    JiThemeVariant  variant;
    JiPalette*      palette;
    bool            palette_owned;  /* true if engine owns the palette */
} JiThemeEngine;

/** Create a new theme engine with the system default variant. */
JI_API JiThemeEngine* ji_theme_engine_new(void);

/** Destroy a theme engine. */
JI_API void ji_theme_engine_destroy(JiThemeEngine* engine);

/** Set the theme variant. This updates the active palette. */
JI_API void ji_theme_engine_set_variant(JiThemeEngine* engine, JiThemeVariant variant);

/** Get the current theme variant. */
JI_API JiThemeVariant ji_theme_engine_get_variant(const JiThemeEngine* engine);

/** Get the active palette. */
JI_API const JiPalette* ji_theme_engine_get_palette(const JiThemeEngine* engine);

/** Set a custom palette (engine takes ownership if own=true). */
JI_API void ji_theme_engine_set_palette(JiThemeEngine* engine, JiPalette* pal, bool own);

/** Get the current global theme engine (created by ji_initialize). */
JI_API JiThemeEngine* ji_theme_engine_get_global(void);

/** Set the global theme engine (replaces the one created by ji_initialize). */
JI_API void ji_theme_engine_set_global(JiThemeEngine* engine);

/** Initialize the theme engine subsystem (called by ji_initialize). */
JI_API void ji_theme_engine_init(void);

/** Shutdown the theme engine subsystem (called by ji_shutdown). */
JI_API void ji_theme_engine_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* JIUI_PALETTE_H */
