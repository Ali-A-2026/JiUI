/**
 * JiUI - Comprehensive UI Showcase Demo
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file showcase_demo.c
 * @brief Collective example showing all capabilities of the JiUI framework:
 *        collapsible menus, sliders, buttons, checkboxes, text boxes,
 *        progress bars, labels, and layout panels.
 *
 * Features:
 *   - Collapsible sections (click header to expand/collapse)
 *   - Interactive buttons with click feedback
 *   - Sliders with real-time value display
 *   - Checkboxes with toggle state
 *   - Text input boxes
 *   - Progress bars (determinate and indeterminate)
 *   - Labels with different font sizes
 *   - Dark theme with accent colors
 *   - FPS counter at top right
 *   - Mouse interaction for all widgets
 *   - ESC to quit
 */

#include <jiui/jiui.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* =========================================================================
 * Color theme — switchable between Dark, Light, Blue, Purple
 * ========================================================================= */

typedef struct {
    float bg_r, bg_g, bg_b;
    float panel_r, panel_g, panel_b;
    float accent_r, accent_g, accent_b;
    float text_r, text_g, text_b;
    float dim_r, dim_g, dim_b;
    float green_r, green_g, green_b;
    float red_r, red_g, red_b;
    float gold_r, gold_g, gold_b;
} ThemeColors;

static ThemeColors g_theme = {
    0.102f, 0.102f, 0.180f,    /* bg */
    0.157f, 0.157f, 0.263f,    /* panel */
    0.369f, 0.251f, 0.878f,    /* accent */
    0.933f, 0.933f, 0.933f,    /* text */
    0.533f, 0.533f, 0.627f,    /* dim */
    0.200f, 0.800f, 0.400f,    /* green */
    0.900f, 0.250f, 0.200f,    /* red */
    0.950f, 0.750f, 0.100f,    /* gold */
};

/* Theme presets */
static const ThemeColors THEME_DARK = {
    0.102f, 0.102f, 0.180f,
    0.157f, 0.157f, 0.263f,
    0.369f, 0.251f, 0.878f,
    0.933f, 0.933f, 0.933f,
    0.533f, 0.533f, 0.627f,
    0.200f, 0.800f, 0.400f,
    0.900f, 0.250f, 0.200f,
    0.950f, 0.750f, 0.100f,
};

static const ThemeColors THEME_LIGHT = {
    0.940f, 0.940f, 0.960f,    /* bg */
    1.000f, 1.000f, 1.000f,    /* panel */
    0.200f, 0.400f, 0.850f,    /* accent */
    0.133f, 0.133f, 0.200f,    /* text */
    0.500f, 0.500f, 0.550f,    /* dim */
    0.100f, 0.650f, 0.300f,    /* green */
    0.850f, 0.200f, 0.150f,    /* red */
    0.800f, 0.600f, 0.050f,    /* gold */
};

static const ThemeColors THEME_BLUE = {
    0.060f, 0.100f, 0.200f,    /* bg */
    0.100f, 0.160f, 0.300f,    /* panel */
    0.200f, 0.550f, 0.950f,    /* accent */
    0.900f, 0.930f, 1.000f,    /* text */
    0.450f, 0.500f, 0.650f,    /* dim */
    0.100f, 0.800f, 0.500f,    /* green */
    0.950f, 0.300f, 0.250f,    /* red */
    0.900f, 0.800f, 0.200f,    /* gold */
};

static const ThemeColors THEME_PURPLE = {
    0.120f, 0.060f, 0.180f,    /* bg */
    0.180f, 0.100f, 0.260f,    /* panel */
    0.600f, 0.300f, 0.900f,    /* accent */
    0.920f, 0.880f, 0.960f,    /* text */
    0.550f, 0.450f, 0.620f,    /* dim */
    0.300f, 0.800f, 0.500f,    /* green */
    0.950f, 0.350f, 0.300f,    /* red */
    0.950f, 0.750f, 0.200f,    /* gold */
};

#define COLOR_BG_R       g_theme.bg_r
#define COLOR_BG_G       g_theme.bg_g
#define COLOR_BG_B       g_theme.bg_b

#define COLOR_PANEL_R    g_theme.panel_r
#define COLOR_PANEL_G    g_theme.panel_g
#define COLOR_PANEL_B    g_theme.panel_b

#define COLOR_ACCENT_R   g_theme.accent_r
#define COLOR_ACCENT_G   g_theme.accent_g
#define COLOR_ACCENT_B   g_theme.accent_b

#define COLOR_TEXT_R     g_theme.text_r
#define COLOR_TEXT_G     g_theme.text_g
#define COLOR_TEXT_B     g_theme.text_b

#define COLOR_DIM_R      g_theme.dim_r
#define COLOR_DIM_G      g_theme.dim_g
#define COLOR_DIM_B      g_theme.dim_b

#define COLOR_GREEN_R    g_theme.green_r
#define COLOR_GREEN_G    g_theme.green_g
#define COLOR_GREEN_B    g_theme.green_b

#define COLOR_RED_R      g_theme.red_r
#define COLOR_RED_G      g_theme.red_g
#define COLOR_RED_B      g_theme.red_b

#define COLOR_GOLD_R     g_theme.gold_r
#define COLOR_GOLD_G     g_theme.gold_g
#define COLOR_GOLD_B     g_theme.gold_b

/* =========================================================================
 * Widget data structures (rendered via OpenGL)
 * ========================================================================= */

typedef enum {
    WIDGET_SECTION,     /* Collapsible section header */
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_CHECKBOX,
    WIDGET_SLIDER,
    WIDGET_TEXTBOX,
    WIDGET_PROGRESSBAR,
    WIDGET_DROPDOWN,    /* Dropdown menu */
} WidgetKind;

typedef struct UIWidget {
    WidgetKind kind;
    float x, y, w, h;          /* position and size */
    char text[128];             /* label text */
    bool hovered;
    bool pressed;
    bool active;               /* for checkbox: checked; for section: expanded */
    double value;               /* for slider/progressbar */
    double min_val, max_val;    /* for slider/progressbar range */
    bool indeterminate;         /* for progressbar */
    char input_text[256];       /* for textbox */
    int cursor_pos;             /* for textbox */
    bool focused;               /* for textbox */
    struct UIWidget* parent_section;  /* which section this belongs to */
    /* Dropdown-specific */
    char options[8][64];        /* dropdown option labels */
    int option_count;           /* number of options */
    int selected_index;         /* currently selected option (-1 = none) */
    bool open;                  /* dropdown list is visible */
    /* Animation state */
    float hover_anim;           /* 0.0-1.0 smooth hover transition */
    float press_anim;           /* 0.0-1.0 smooth press transition */
    float focus_anim;           /* 0.0-1.0 smooth focus pulse */
    /* Tooltip */
    char tooltip[128];          /* tooltip text shown on hover */
} UIWidget;

#define MAX_WIDGETS 160
static UIWidget g_widgets[MAX_WIDGETS];
static int g_widget_count = 0;

/* =========================================================================
 * Demo state
 * ========================================================================= */

static bool g_running = true;
static int g_mouse_x = 0, g_mouse_y = 0;
static bool g_mouse_down = false;

/* FPS tracking */
static int    g_frame_count = 0;
static double g_fps_timer = 0.0;
static double g_current_fps = 0.0;
static double g_last_time = 0.0;

/* Scroll offset */
static float g_scroll_y = 0.0f;
static float g_content_height = 0.0f;

/* Click counter for button demo */
static int g_button_clicks = 0;

/* Progress animation */
static double g_progress_value = 0.0;
static bool g_progress_direction = true;

/* Font scale from dropdown (0=Small, 1=Medium, 2=Large, 3=XL) */
static float g_font_scale = 1.0f;

/* Current language from dropdown */
static int g_language = 0; /* 0=English, 1=French, 2=Japanese, 3=Spanish, 4=German */

/* Tooltip state */
static UIWidget* g_tooltip_widget = NULL;
static float g_tooltip_timer = 0.0f;
#define TOOLTIP_DELAY 0.6f   /* seconds before tooltip appears */
#define TOOLTIP_FADE  0.2f   /* seconds for tooltip fade-in */

/* Focus pulse animation */
static float g_focus_pulse = 0.0f;

/* =========================================================================
 * Internationalization (i18n) — translation table
 * ========================================================================= */

typedef enum {
    STR_TITLE,
    STR_BUTTONS,
    STR_CLICK_ME,
    STR_TOGGLE_BTN,
    STR_SLIDERS,
    STR_VOLUME,
    STR_SPEED,
    STR_CHECKBOXES,
    STR_DARK_MODE,
    STR_SHOW_FPS,
    STR_ANIMATIONS,
    STR_TEXT_INPUT,
    STR_NAME,
    STR_TYPE_HERE,
    STR_PROGRESS,
    STR_DOWNLOAD,
    STR_LOADING,
    STR_DROPDOWNS,
    STR_THEME,
    STR_LANGUAGE,
    STR_FONT_SIZE,
    STR_SELECT_THEME,
    STR_SELECT_LANG,
    STR_SELECT_SIZE,
    STR_FOOTER,
    STR_BTN_CLICKS,
    STR_COUNT
} StrID;

/* Translations: [language][string_id] */
static const char* g_translations[5][STR_COUNT] = {
    /* English */
    {
        "JiUI Showcase - All Widgets Demo",
        "Buttons",
        "Click Me!",
        "Toggle Button",
        "Sliders",
        "Volume:",
        "Speed:",
        "Checkboxes",
        "Enable dark mode",
        "Show FPS counter",
        "Enable animations",
        "Text Input",
        "Name:",
        "Type here...",
        "Progress Bars",
        "Download progress:",
        "Loading...",
        "Dropdown Menus",
        "Theme:",
        "Language:",
        "Font Size:",
        "Select theme...",
        "Select language...",
        "Select size...",
        "JiUI v0.1.0 | ESC to quit | Scroll",
        "Button clicks: %d",
    },
    /* German */
    {
        "JiUI Vitrine - Alle Widgets Demo",
        "Knoepfe",
        "Klick mich!",
        "Schalter",
        "Schieberegler",
        "Lautstaerke:",
        "Geschwindigkeit:",
        "Kontrollkaestchen",
        "Dunkelmodus aktivieren",
        "FPS-Zaehler anzeigen",
        "Animationen aktivieren",
        "Texteingabe",
        "Name:",
        "Hier tippen...",
        "Fortschrittsbalken",
        "Download-Fortschritt:",
        "Laden...",
        "Dropdown-Menues",
        "Thema:",
        "Sprache:",
        "Schriftgroesse:",
        "Thema waehlen...",
        "Sprache waehlen...",
        "Groesse waehlen...",
        "JiUI v0.1.0 | ESC zum Beenden | Scrollen",
        "Klicks: %d",
    },
    /* French */
    {
        "Vitrine JiUI - Demo de tous les widgets",
        "Boutons",
        "Cliquez-moi!",
        "Bouton bascule",
        "Curseurs",
        "Volume:",
        "Vitesse:",
        "Cases a cocher",
        "Activer le mode sombre",
        "Afficher le compteur FPS",
        "Activer les animations",
        "Saisie de texte",
        "Nom:",
        "Tapez ici...",
        "Barres de progression",
        "Progression du telechargement:",
        "Chargement...",
        "Menus deroulants",
        "Theme:",
        "Langue:",
        "Taille de police:",
        "Choisir un theme...",
        "Choisir une langue...",
        "Choisir une taille...",
        "JiUI v0.1.0 | ESC pour quitter | Defiler",
        "Clics sur le bouton: %d",
    },
    /* Japanese */
    {
        "JiUI Showcase - Widget Demo",
        "Botan",
        "Click!",
        "Toggle Botan",
        "Saida-",
        "Onryo:",
        "Sokudo:",
        "Chekku Bokkusu",
        "Daku moodo ON",
        "FPS hyouji",
        "Anime-shon ON",
        "Tekisuto Nyuuryoku",
        "Namae:",
        "Koko ni nyuryoku...",
        "Puroguresu Ba-",
        "Daunload shinkyoku:",
        "Roodo-chu...",
        "Doroppu Daun Menyu-",
        "Te-ma:",
        "Gengo:",
        "Fonto Saizu:",
        "Te-ma o sentaku...",
        "Gengo o sentaku...",
        "Saizu o sentaku...",
        "JiUI v0.1.0 | ESC de shuuryou | Sukurooru",
        "Botan kurikku: %d",
    },
    /* Spanish */
    {
        "Vitrina JiUI - Demo de todos los widgets",
        "Botones",
        "Haz clic!",
        "Boton de alternar",
        "Deslizadores",
        "Volumen:",
        "Velocidad:",
        "Casillas de verificacion",
        "Activar modo oscuro",
        "Mostrar contador FPS",
        "Activar animaciones",
        "Entrada de texto",
        "Nombre:",
        "Escribe aqui...",
        "Barras de progreso",
        "Progreso de descarga:",
        "Cargando...",
        "Menus desplegables",
        "Tema:",
        "Idioma:",
        "Tamano de fuente:",
        "Seleccionar tema...",
        "Seleccionar idioma...",
        "Seleccionar tamano...",
        "JiUI v0.1.0 | ESC para salir | Desplazar",
        "Clics en boton: %d",
    },
};

/* Get translated string for current language */
static const char* T(StrID id) {
    return g_translations[g_language][id];
}

/* Forward declaration */
static void apply_language(void);

/* =========================================================================
 * Bitmap font (same as cube demo)
 * ========================================================================= */

static const unsigned char DIGIT_GLYPHS[10][8] = {
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00},
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    {0x0E,0x1E,0x36,0x66,0x7F,0x06,0x06,0x00},
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00},
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
};

/* Full ASCII 8x8 font glyphs */
static const unsigned char CHAR_A[8] = {0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00};
static const unsigned char CHAR_B[8] = {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00};
static const unsigned char CHAR_C[8] = {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00};
static const unsigned char CHAR_D[8] = {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00};
static const unsigned char CHAR_E[8] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00};
static const unsigned char CHAR_F[8] = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00};
static const unsigned char CHAR_G[8] = {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00};
static const unsigned char CHAR_H[8] = {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00};
static const unsigned char CHAR_I[8] = {0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00};
static const unsigned char CHAR_K[8] = {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00};
static const unsigned char CHAR_L[8] = {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00};
static const unsigned char CHAR_M[8] = {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00};
static const unsigned char CHAR_N[8] = {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00};
static const unsigned char CHAR_O[8] = {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00};
static const unsigned char CHAR_P[8] = {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00};
static const unsigned char CHAR_R[8] = {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00};
static const unsigned char CHAR_S[8] = {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00};
static const unsigned char CHAR_T[8] = {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00};
static const unsigned char CHAR_U[8] = {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00};
static const unsigned char CHAR_V[8] = {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00};
static const unsigned char CHAR_W[8] = {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00};
static const unsigned char CHAR_X[8] = {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00};
static const unsigned char CHAR_Y[8] = {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00};
static const unsigned char CHAR_Z[8] = {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00};

static const unsigned char CHAR_DOT[8]    = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00};
static const unsigned char CHAR_COLON[8]  = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00};
static const unsigned char CHAR_SPACE[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_DASH[8]  = {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_PLUS[8]  = {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00};
static const unsigned char CHAR_EQ[8]    = {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00};
static const unsigned char CHAR_LANGLE[8] = {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00};
static const unsigned char CHAR_RANGLE[8] = {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00};
static const unsigned char CHAR_VLINE[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00};
static const unsigned char CHAR_UNDER[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00};
static const unsigned char CHAR_BANG[8]  = {0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00};
static const unsigned char CHAR_QUES[8]  = {0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00};
static const unsigned char CHAR_AT[8]    = {0x3C,0x66,0x6E,0x6E,0x60,0x60,0x3C,0x00};
static const unsigned char CHAR_HASH[8]  = {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00};
static const unsigned char CHAR_PCT[8]   = {0x7C,0x0C,0x18,0x30,0x60,0x7C,0x00,0x00};
static const unsigned char CHAR_CARET[8] = {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_AMP[8]   = {0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00};
static const unsigned char CHAR_STAR[8]  = {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00};
static const unsigned char CHAR_LPAREN[8] = {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00};
static const unsigned char CHAR_RPAREN[8] = {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00};
static const unsigned char CHAR_LBRACE[8] = {0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00};
static const unsigned char CHAR_RBRACE[8] = {0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00};
static const unsigned char CHAR_SLASH[8]  = {0x02,0x06,0x0C,0x18,0x30,0x60,0x40,0x00};
static const unsigned char CHAR_BSLASH[8] = {0x40,0x60,0x30,0x18,0x0C,0x06,0x02,0x00};
static const unsigned char CHAR_COMMA[8]  = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30};
static const unsigned char CHAR_SEMI[8]  = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30};
static const unsigned char CHAR_QUOTE[8] = {0x00,0x66,0x66,0x00,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_TILDE[8] = {0x00,0x00,0x76,0xDC,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_GRAVE[8] = {0x30,0x18,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char CHAR_DOLLAR[8] = {0x18,0x3C,0x60,0x3C,0x06,0x7C,0x18,0x00};

static unsigned char* g_font_data = NULL;
static int g_font_tex = 0;

static void init_font_texture(void) {
    unsigned char atlas[256][8];
    memset(atlas, 0, sizeof(atlas));

    for (int i = 0; i < 10; i++) memcpy(atlas['0' + i], DIGIT_GLYPHS[i], 8);
    memcpy(atlas['.'],  CHAR_DOT, 8);
    memcpy(atlas[':'],  CHAR_COLON, 8);
    memcpy(atlas[' '],  CHAR_SPACE, 8);
    memcpy(atlas['-'],  CHAR_DASH, 8);
    memcpy(atlas['+'],  CHAR_PLUS, 8);
    memcpy(atlas['='],  CHAR_EQ, 8);
    memcpy(atlas['<'],  CHAR_LANGLE, 8);
    memcpy(atlas['>'],  CHAR_RANGLE, 8);
    memcpy(atlas['|'],  CHAR_VLINE, 8);
    memcpy(atlas['_'],  CHAR_UNDER, 8);
    memcpy(atlas['!'],  CHAR_BANG, 8);
    memcpy(atlas['?'],  CHAR_QUES, 8);
    memcpy(atlas['@'],  CHAR_AT, 8);
    memcpy(atlas['#'],  CHAR_HASH, 8);
    memcpy(atlas['%'],  CHAR_PCT, 8);
    memcpy(atlas['^'],  CHAR_CARET, 8);
    memcpy(atlas['&'],  CHAR_AMP, 8);
    memcpy(atlas['*'],  CHAR_STAR, 8);
    memcpy(atlas['('],  CHAR_LPAREN, 8);
    memcpy(atlas[')'],  CHAR_RPAREN, 8);
    memcpy(atlas['{'],  CHAR_LBRACE, 8);
    memcpy(atlas['}'],  CHAR_RBRACE, 8);
    memcpy(atlas['/'],  CHAR_SLASH, 8);
    memcpy(atlas['\\'], CHAR_BSLASH, 8);
    memcpy(atlas[','],  CHAR_COMMA, 8);
    memcpy(atlas[';'],  CHAR_SEMI, 8);
    memcpy(atlas['\''], CHAR_QUOTE, 8);
    memcpy(atlas['~'],  CHAR_TILDE, 8);
    memcpy(atlas['`'],  CHAR_GRAVE, 8);
    memcpy(atlas['$'],  CHAR_DOLLAR, 8);

    /* Uppercase */
    memcpy(atlas['A'], CHAR_A, 8); memcpy(atlas['B'], CHAR_B, 8);
    memcpy(atlas['C'], CHAR_C, 8); memcpy(atlas['D'], CHAR_D, 8);
    memcpy(atlas['E'], CHAR_E, 8); memcpy(atlas['F'], CHAR_F, 8);
    memcpy(atlas['G'], CHAR_G, 8); memcpy(atlas['H'], CHAR_H, 8);
    memcpy(atlas['I'], CHAR_I, 8);
    memcpy(atlas['K'], CHAR_K, 8); memcpy(atlas['L'], CHAR_L, 8);
    memcpy(atlas['M'], CHAR_M, 8); memcpy(atlas['N'], CHAR_N, 8);
    memcpy(atlas['O'], CHAR_O, 8); memcpy(atlas['P'], CHAR_P, 8);
    memcpy(atlas['R'], CHAR_R, 8); memcpy(atlas['S'], CHAR_S, 8);
    memcpy(atlas['T'], CHAR_T, 8); memcpy(atlas['U'], CHAR_U, 8);
    memcpy(atlas['V'], CHAR_V, 8); memcpy(atlas['W'], CHAR_W, 8);
    memcpy(atlas['X'], CHAR_X, 8); memcpy(atlas['Y'], CHAR_Y, 8);
    memcpy(atlas['Z'], CHAR_Z, 8);

    /* Lowercase = same as uppercase for this bitmap font */
    for (int c = 'a'; c <= 'z'; c++) memcpy(atlas[c], atlas[c - 'a' + 'A'], 8);

    #define ATLAS_W (256 * 8)
    #define ATLAS_H 8
    g_font_data = (unsigned char*)malloc(ATLAS_W * ATLAS_H * 4);
    if (!g_font_data) return;
    memset(g_font_data, 0, ATLAS_W * ATLAS_H * 4);

    for (int c = 0; c < 256; c++) {
        for (int row = 0; row < 8; row++) {
            unsigned char bits = atlas[c][row];
            for (int col = 0; col < 8; col++) {
                int x = c * 8 + col;
                int idx = (row * ATLAS_W + x) * 4;
                int on = (bits >> (7 - col)) & 1;
                g_font_data[idx + 0] = 255;
                g_font_data[idx + 1] = 255;
                g_font_data[idx + 2] = 255;
                g_font_data[idx + 3] = on ? 255 : 0;
            }
        }
    }

    glGenTextures(1, (GLuint*)&g_font_tex);
    glBindTexture(GL_TEXTURE_2D, g_font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_W, ATLAS_H,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, g_font_data);
    #undef ATLAS_W
    #undef ATLAS_H
}

/* =========================================================================
 * Text rendering
 * ========================================================================= */

static void draw_text(const char* text, float x, float y, float scale,
                      float r, float g, float b, float a,
                      int screen_w, int screen_h) {
    if (!g_font_tex || !text) return;
    scale *= g_font_scale;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, g_font_tex);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(r, g, b, a);

    float cx = x;
    for (const char* p = text; *p; p++) {
        unsigned char c = (unsigned char)*p;
        float u0 = (float)(c * 8) / (256.0f * 8.0f);
        float u1 = (float)(c * 8 + 8) / (256.0f * 8.0f);
        float w = 8.0f * scale;
        float h = 8.0f * scale;

        glBegin(GL_QUADS);
        glTexCoord2f(u0, 0.0f); glVertex2f(cx,      y);
        glTexCoord2f(u1, 0.0f); glVertex2f(cx + w,  y);
        glTexCoord2f(u1, 1.0f); glVertex2f(cx + w,  y + h);
        glTexCoord2f(u0, 1.0f); glVertex2f(cx,      y + h);
        glEnd();

        cx += w;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

static float text_width(const char* text, float scale) {
    return (float)strlen(text) * 8.0f * scale * g_font_scale;
}

/* =========================================================================
 * 2D drawing helpers
 * ========================================================================= */

static void draw_rect(float x, float y, float w, float h,
                       float r, float g, float b, float a,
                       int screen_w, int screen_h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void draw_rect_outline(float x, float y, float w, float h,
                                float r, float g, float b, float a,
                                int screen_w, int screen_h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* Rounded rect using filled quads + corner arcs */
static void draw_rounded_rect(float x, float y, float w, float h, float radius,
                                float r, float g, float b, float a,
                                int screen_w, int screen_h) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_w, screen_h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    glColor4f(r, g, b, a);

    /* Main body */
    glBegin(GL_QUADS);
    /* Center */
    glVertex2f(x + radius, y);
    glVertex2f(x + w - radius, y);
    glVertex2f(x + w - radius, y + h);
    glVertex2f(x + radius, y + h);
    /* Left */
    glVertex2f(x, y + radius);
    glVertex2f(x + radius, y + radius);
    glVertex2f(x + radius, y + h - radius);
    glVertex2f(x, y + h - radius);
    /* Right */
    glVertex2f(x + w - radius, y + radius);
    glVertex2f(x + w, y + radius);
    glVertex2f(x + w, y + h - radius);
    glVertex2f(x + w - radius, y + h - radius);
    glEnd();

    /* Corner arcs */
    const int segs = 8;
    for (int corner = 0; corner < 4; corner++) {
        float cx, cy;
        float start_angle;
        switch (corner) {
            case 0: cx = x + radius; cy = y + radius; start_angle = (float)M_PI; break;
            case 1: cx = x + w - radius; cy = y + radius; start_angle = 1.5f * (float)M_PI; break;
            case 2: cx = x + w - radius; cy = y + h - radius; start_angle = 0.0f; break;
            case 3: cx = x + radius; cy = y + h - radius; start_angle = 0.5f * (float)M_PI; break;
        }
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segs; i++) {
            float angle = start_angle + (float)i / segs * ((float)M_PI / 2.0f);
            glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* =========================================================================
 * Widget creation helpers
 * ========================================================================= */

static UIWidget* add_widget(WidgetKind kind, float x, float y, float w, float h,
                             const char* text, UIWidget* parent) {
    if (g_widget_count >= MAX_WIDGETS) return NULL;
    UIWidget* wd = &g_widgets[g_widget_count++];
    memset(wd, 0, sizeof(UIWidget));
    wd->kind = kind;
    wd->x = x; wd->y = y; wd->w = w; wd->h = h;
    strncpy(wd->text, text ? text : "", sizeof(wd->text) - 1);
    wd->parent_section = parent;
    wd->min_val = 0.0;
    wd->max_val = 100.0;
    wd->value = 50.0;
    return wd;
}

/* =========================================================================
 * Build the UI layout
 * ========================================================================= */

static UIWidget* g_section_buttons = NULL;
static UIWidget* g_section_sliders = NULL;
static UIWidget* g_section_checks  = NULL;
static UIWidget* g_section_text    = NULL;
static UIWidget* g_section_progress = NULL;

static UIWidget* g_btn_click = NULL;
static UIWidget* g_btn_toggle = NULL;
static UIWidget* g_slider_volume = NULL;
static UIWidget* g_slider_speed = NULL;
static UIWidget* g_check_option1 = NULL;
static UIWidget* g_check_option2 = NULL;
static UIWidget* g_check_option3 = NULL;
static UIWidget* g_textbox_name = NULL;
static UIWidget* g_progress_bar = NULL;
static UIWidget* g_progress_indet = NULL;
static UIWidget* g_section_dropdown = NULL;
static UIWidget* g_dropdown_theme = NULL;
static UIWidget* g_dropdown_lang = NULL;
static UIWidget* g_dropdown_size = NULL;

static void build_ui(void) {
    float margin = 20.0f;
    float section_w = 700.0f;
    float y = margin;
    float text_scale = 2.0f;
    float line_h = 20.0f * text_scale;

    /* Title */
    add_widget(WIDGET_LABEL, margin, y, section_w, 30.0f * text_scale,
               "JiUI Showcase - All Widgets Demo", NULL);
    y += 35.0f * text_scale;

    /* ---- Buttons Section ---- */
    g_section_buttons = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                     "Buttons", NULL);
    g_section_buttons->active = true; /* expanded */
    y += 35.0f;

    g_btn_click = add_widget(WIDGET_BUTTON, margin + 15.0f, y, 200.0f, 36.0f,
                               "Click Me!", g_section_buttons);
    y += 46.0f;

    g_btn_toggle = add_widget(WIDGET_BUTTON, margin + 15.0f, y, 200.0f, 36.0f,
                                "Toggle Button", g_section_buttons);
    y += 46.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 400.0f, 20.0f,
               "Button clicks: 0", g_section_buttons);
    y += 30.0f;

    /* ---- Sliders Section ---- */
    y += 10.0f;
    g_section_sliders = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                     "Sliders", NULL);
    g_section_sliders->active = true;
    y += 35.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Volume:", g_section_sliders);
    g_slider_volume = add_widget(WIDGET_SLIDER, margin + 15.0f, y + 25.0f, 400.0f, 24.0f,
                                   "Volume", g_section_sliders);
    g_slider_volume->value = 75.0;
    y += 60.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Speed:", g_section_sliders);
    g_slider_speed = add_widget(WIDGET_SLIDER, margin + 15.0f, y + 25.0f, 400.0f, 24.0f,
                                  "Speed", g_section_sliders);
    g_slider_speed->value = 50.0;
    y += 60.0f;

    /* ---- Checkboxes Section ---- */
    y += 10.0f;
    g_section_checks = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                    "Checkboxes", NULL);
    g_section_checks->active = true;
    y += 35.0f;

    g_check_option1 = add_widget(WIDGET_CHECKBOX, margin + 15.0f, y, 300.0f, 24.0f,
                                   "Enable dark mode", g_section_checks);
    g_check_option1->active = true;
    y += 32.0f;

    g_check_option2 = add_widget(WIDGET_CHECKBOX, margin + 15.0f, y, 300.0f, 24.0f,
                                   "Show FPS counter", g_section_checks);
    g_check_option2->active = true;
    y += 32.0f;

    g_check_option3 = add_widget(WIDGET_CHECKBOX, margin + 15.0f, y, 300.0f, 24.0f,
                                   "Enable animations", g_section_checks);
    g_check_option3->active = true;
    y += 32.0f;

    /* ---- Text Input Section ---- */
    y += 10.0f;
    g_section_text = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                  "Text Input", NULL);
    g_section_text->active = true;
    y += 35.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Name:", g_section_text);
    g_textbox_name = add_widget(WIDGET_TEXTBOX, margin + 15.0f, y + 25.0f, 400.0f, 32.0f,
                                  "Type here...", g_section_text);
    strncpy(g_textbox_name->input_text, "JiUI User", sizeof(g_textbox_name->input_text) - 1);
    y += 65.0f;

    /* ---- Progress Bars Section ---- */
    y += 10.0f;
    g_section_progress = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                      "Progress Bars", NULL);
    g_section_progress->active = true;
    y += 35.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Download progress:", g_section_progress);
    g_progress_bar = add_widget(WIDGET_PROGRESSBAR, margin + 15.0f, y + 25.0f, 400.0f, 24.0f,
                                  "Progress", g_section_progress);
    g_progress_bar->value = 42.0;
    y += 60.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Loading...", g_section_progress);
    g_progress_indet = add_widget(WIDGET_PROGRESSBAR, margin + 15.0f, y + 25.0f, 400.0f, 24.0f,
                                    "Indeterminate", g_section_progress);
    g_progress_indet->indeterminate = true;
    y += 60.0f;

    /* ---- Dropdown Menus Section ---- */
    y += 10.0f;
    g_section_dropdown = add_widget(WIDGET_SECTION, margin, y, section_w, 30.0f,
                                      "Dropdown Menus", NULL);
    g_section_dropdown->active = true;
    y += 35.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Theme:", g_section_dropdown);
    g_dropdown_theme = add_widget(WIDGET_DROPDOWN, margin + 15.0f, y + 25.0f, 250.0f, 32.0f,
                                    "Select theme...", g_section_dropdown);
    strncpy(g_dropdown_theme->options[0], "Dark", sizeof(g_dropdown_theme->options[0]) - 1);
    strncpy(g_dropdown_theme->options[1], "Light", sizeof(g_dropdown_theme->options[1]) - 1);
    strncpy(g_dropdown_theme->options[2], "Blue", sizeof(g_dropdown_theme->options[2]) - 1);
    strncpy(g_dropdown_theme->options[3], "Purple", sizeof(g_dropdown_theme->options[3]) - 1);
    g_dropdown_theme->option_count = 4;
    g_dropdown_theme->selected_index = 0;
    y += 65.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Language:", g_section_dropdown);
    g_dropdown_lang = add_widget(WIDGET_DROPDOWN, margin + 15.0f, y + 25.0f, 250.0f, 32.0f,
                                   "Select language...", g_section_dropdown);
    strncpy(g_dropdown_lang->options[0], "English", sizeof(g_dropdown_lang->options[0]) - 1);
    strncpy(g_dropdown_lang->options[1], "French", sizeof(g_dropdown_lang->options[1]) - 1);
    strncpy(g_dropdown_lang->options[2], "Japanese", sizeof(g_dropdown_lang->options[2]) - 1);
    strncpy(g_dropdown_lang->options[3], "Spanish", sizeof(g_dropdown_lang->options[3]) - 1);
    strncpy(g_dropdown_lang->options[4], "German", sizeof(g_dropdown_lang->options[4]) - 1);
    g_dropdown_lang->option_count = 5;
    g_dropdown_lang->selected_index = 0;
    y += 65.0f;

    add_widget(WIDGET_LABEL, margin + 15.0f, y, 200.0f, 20.0f,
               "Font Size:", g_section_dropdown);
    g_dropdown_size = add_widget(WIDGET_DROPDOWN, margin + 15.0f, y + 25.0f, 250.0f, 32.0f,
                                   "Select size...", g_section_dropdown);
    strncpy(g_dropdown_size->options[0], "Small", sizeof(g_dropdown_size->options[0]) - 1);
    strncpy(g_dropdown_size->options[1], "Medium", sizeof(g_dropdown_size->options[1]) - 1);
    strncpy(g_dropdown_size->options[2], "Large", sizeof(g_dropdown_size->options[2]) - 1);
    strncpy(g_dropdown_size->options[3], "Extra Large", sizeof(g_dropdown_size->options[3]) - 1);
    g_dropdown_size->option_count = 4;
    g_dropdown_size->selected_index = 1;
    y += 65.0f;

    /* Footer */
    y += 20.0f;
    add_widget(WIDGET_LABEL, margin, y, section_w, 20.0f,
               "JiUI v0.1.0 | ESC to quit | Scroll to navigate", NULL);

    g_content_height = y + 40.0f;

    /* Apply initial language */
    apply_language();

    /* Set tooltips */
    if (g_btn_click)    strncpy(g_btn_click->tooltip, "Click this button to increment counter", sizeof(g_btn_click->tooltip) - 1);
    if (g_btn_toggle)   strncpy(g_btn_toggle->tooltip, "Toggle button with active state", sizeof(g_btn_toggle->tooltip) - 1);
    if (g_slider_volume) strncpy(g_slider_volume->tooltip, "Adjust volume 0-100", sizeof(g_slider_volume->tooltip) - 1);
    if (g_slider_speed)  strncpy(g_slider_speed->tooltip, "Adjust speed 0-100", sizeof(g_slider_speed->tooltip) - 1);
    if (g_check_option1) strncpy(g_check_option1->tooltip, "Switch between dark and light theme", sizeof(g_check_option1->tooltip) - 1);
    if (g_check_option2) strncpy(g_check_option2->tooltip, "Show/hide FPS counter overlay", sizeof(g_check_option2->tooltip) - 1);
    if (g_check_option3) strncpy(g_check_option3->tooltip, "Enable/disable progress animations", sizeof(g_check_option3->tooltip) - 1);
    if (g_textbox_name)  strncpy(g_textbox_name->tooltip, "Type your name here", sizeof(g_textbox_name->tooltip) - 1);
    if (g_dropdown_theme) strncpy(g_dropdown_theme->tooltip, "Choose a color theme", sizeof(g_dropdown_theme->tooltip) - 1);
    if (g_dropdown_lang)  strncpy(g_dropdown_lang->tooltip, "Choose display language", sizeof(g_dropdown_lang->tooltip) - 1);
    if (g_dropdown_size)   strncpy(g_dropdown_size->tooltip, "Choose font size", sizeof(g_dropdown_size->tooltip) - 1);
}

/* Apply current language to all widget text labels */
static void apply_language(void) {
    /* Section headers */
    if (g_section_buttons)   strncpy(g_section_buttons->text, T(STR_BUTTONS), sizeof(g_section_buttons->text) - 1);
    if (g_section_sliders)   strncpy(g_section_sliders->text, T(STR_SLIDERS), sizeof(g_section_sliders->text) - 1);
    if (g_section_checks)    strncpy(g_section_checks->text, T(STR_CHECKBOXES), sizeof(g_section_checks->text) - 1);
    if (g_section_text)      strncpy(g_section_text->text, T(STR_TEXT_INPUT), sizeof(g_section_text->text) - 1);
    if (g_section_progress)  strncpy(g_section_progress->text, T(STR_PROGRESS), sizeof(g_section_progress->text) - 1);
    if (g_section_dropdown)  strncpy(g_section_dropdown->text, T(STR_DROPDOWNS), sizeof(g_section_dropdown->text) - 1);

    /* Buttons */
    if (g_btn_click)   strncpy(g_btn_click->text, T(STR_CLICK_ME), sizeof(g_btn_click->text) - 1);
    if (g_btn_toggle)  strncpy(g_btn_toggle->text, T(STR_TOGGLE_BTN), sizeof(g_btn_toggle->text) - 1);

    /* Checkboxes */
    if (g_check_option1)  strncpy(g_check_option1->text, T(STR_DARK_MODE), sizeof(g_check_option1->text) - 1);
    if (g_check_option2)  strncpy(g_check_option2->text, T(STR_SHOW_FPS), sizeof(g_check_option2->text) - 1);
    if (g_check_option3)  strncpy(g_check_option3->text, T(STR_ANIMATIONS), sizeof(g_check_option3->text) - 1);

    /* Textbox placeholder */
    if (g_textbox_name)  strncpy(g_textbox_name->text, T(STR_TYPE_HERE), sizeof(g_textbox_name->text) - 1);

    /* Dropdown placeholders */
    if (g_dropdown_theme) strncpy(g_dropdown_theme->text, T(STR_SELECT_THEME), sizeof(g_dropdown_theme->text) - 1);
    if (g_dropdown_lang)   strncpy(g_dropdown_lang->text, T(STR_SELECT_LANG), sizeof(g_dropdown_lang->text) - 1);
    if (g_dropdown_size)   strncpy(g_dropdown_size->text, T(STR_SELECT_SIZE), sizeof(g_dropdown_size->text) - 1);

    /* Update all label widgets by scanning for known parent sections */
    for (int i = 0; i < g_widget_count; i++) {
        UIWidget* wd = &g_widgets[i];
        if (wd->kind != WIDGET_LABEL) continue;

        /* Title label (no parent section) */
        if (wd->parent_section == NULL) {
            if (i == 0) {
                strncpy(wd->text, T(STR_TITLE), sizeof(wd->text) - 1);
            } else {
                strncpy(wd->text, T(STR_FOOTER), sizeof(wd->text) - 1);
            }
            continue;
        }

        /* Labels inside sections — match by position/text pattern */
        if (wd->parent_section == g_section_sliders) {
            if (wd->y < g_slider_volume->y)
                strncpy(wd->text, T(STR_VOLUME), sizeof(wd->text) - 1);
            else
                strncpy(wd->text, T(STR_SPEED), sizeof(wd->text) - 1);
        }
        else if (wd->parent_section == g_section_text) {
            strncpy(wd->text, T(STR_NAME), sizeof(wd->text) - 1);
        }
        else if (wd->parent_section == g_section_progress) {
            if (wd->y < g_progress_indet->y)
                strncpy(wd->text, T(STR_DOWNLOAD), sizeof(wd->text) - 1);
            else
                strncpy(wd->text, T(STR_LOADING), sizeof(wd->text) - 1);
        }
        else if (wd->parent_section == g_section_dropdown) {
            /* Theme / Language / Font Size labels — match by Y position */
            if (wd->y < g_dropdown_lang->y)
                strncpy(wd->text, T(STR_THEME), sizeof(wd->text) - 1);
            else if (wd->y < g_dropdown_size->y)
                strncpy(wd->text, T(STR_LANGUAGE), sizeof(wd->text) - 1);
            else
                strncpy(wd->text, T(STR_FONT_SIZE), sizeof(wd->text) - 1);
        }
        else if (wd->parent_section == g_section_buttons) {
            /* Button clicks label — updated dynamically */
        }
    }
}

/* =========================================================================
 * Widget rendering
 * ========================================================================= */

static bool is_widget_visible(UIWidget* wd) {
    if (!wd->parent_section) return true;
    if (wd->parent_section->kind != WIDGET_SECTION) return true;
    return wd->parent_section->active; /* section is expanded */
}

static void render_widget(UIWidget* wd, int screen_w, int screen_h) {
    float x = wd->x;
    float y = wd->y - g_scroll_y;
    float w = wd->w;
    float h = wd->h;

    /* Skip if off-screen */
    if (y + h < 0 || y > screen_h) return;

    /* Skip if parent section is collapsed (except the section header itself) */
    if (wd->parent_section && !is_widget_visible(wd)) return;

    switch (wd->kind) {
    case WIDGET_SECTION: {
        /* Section header — clickable bar */
        float bg_r = wd->active ? COLOR_ACCENT_R : COLOR_PANEL_R;
        float bg_g = wd->active ? COLOR_ACCENT_G : COLOR_PANEL_G;
        float bg_b = wd->active ? COLOR_ACCENT_B : COLOR_PANEL_B;
        draw_rounded_rect(x, y, w, h, 6.0f, bg_r, bg_g, bg_b, 1.0f, screen_w, screen_h);

        /* Expand/collapse arrow */
        const char* arrow = wd->active ? "v " : "> ";
        char header[132];
        snprintf(header, sizeof(header), "%s%s", arrow, wd->text);
        draw_text(header, x + 10.0f, y + 7.0f, 1.8f,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
        break;
    }

    case WIDGET_BUTTON: {
        /* Smooth hover/press animation */
        float ha = wd->hover_anim;
        float pa = wd->press_anim;
        float bg_r = COLOR_ACCENT_R * (1.0f + ha * 0.2f - pa * 0.3f);
        float bg_g = COLOR_ACCENT_G * (1.0f + ha * 0.2f - pa * 0.3f);
        float bg_b = COLOR_ACCENT_B * (1.0f + ha * 0.2f - pa * 0.3f);
        /* Shadow under button */
        draw_rounded_rect(x + 2.0f, y + 3.0f, w, h, 6.0f,
                          0.0f, 0.0f, 0.0f, 0.15f + ha * 0.1f, screen_w, screen_h);
        draw_rounded_rect(x, y, w, h, 6.0f, bg_r, bg_g, bg_b, 1.0f, screen_w, screen_h);
        /* Subtle top highlight for 3D effect */
        draw_rect(x + 4.0f, y + 1.0f, w - 8.0f, 2.0f,
                  1.0f, 1.0f, 1.0f, 0.08f + ha * 0.05f, screen_w, screen_h);
        draw_text(wd->text, x + (w - text_width(wd->text, 1.6f)) / 2.0f, y + 8.0f, 1.6f,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
        break;
    }

    case WIDGET_LABEL: {
        draw_text(wd->text, x, y, 1.6f,
                  COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 1.0f, screen_w, screen_h);
        break;
    }

    case WIDGET_CHECKBOX: {
        /* Checkbox box */
        float box_size = 20.0f;
        float box_x = x;
        float box_y = y + 2.0f;
        float ha = wd->hover_anim;
        if (wd->active) {
            draw_rounded_rect(box_x, box_y, box_size, box_size, 4.0f,
                              COLOR_GREEN_R + ha * 0.1f, COLOR_GREEN_G + ha * 0.1f, COLOR_GREEN_B + ha * 0.1f, 1.0f, screen_w, screen_h);
            /* Checkmark */
            draw_text("V", box_x + 3.0f, box_y + 1.0f, 1.8f,
                      1.0f, 1.0f, 1.0f, 1.0f, screen_w, screen_h);
        } else {
            float cbg = 0.05f * ha;
            draw_rounded_rect(box_x, box_y, box_size, box_size, 4.0f,
                              COLOR_PANEL_R + cbg, COLOR_PANEL_G + cbg, COLOR_PANEL_B + cbg, 1.0f, screen_w, screen_h);
            draw_rect_outline(box_x, box_y, box_size, box_size,
                              COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 0.5f + ha * 0.3f, screen_w, screen_h);
        }
        /* Label text */
        draw_text(wd->text, box_x + box_size + 10.0f, box_y + 2.0f, 1.6f,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
        break;
    }

    case WIDGET_SLIDER: {
        float ha = wd->hover_anim;
        /* Track */
        float track_h = 8.0f;
        float track_y = y + (h - track_h) / 2.0f;
        draw_rounded_rect(x, track_y, w, track_h, 4.0f,
                          COLOR_PANEL_R * 1.5f, COLOR_PANEL_G * 1.5f, COLOR_PANEL_B * 1.5f, 1.0f,
                          screen_w, screen_h);

        /* Fill */
        double pct = (wd->max_val > wd->min_val) ?
                     (wd->value - wd->min_val) / (wd->max_val - wd->min_val) : 0.0;
        if (pct < 0.0) pct = 0.0;
        if (pct > 1.0) pct = 1.0;
        float fill_w = (float)pct * w;
        if (fill_w > 0.0f) {
            draw_rounded_rect(x, track_y, fill_w, track_h, 4.0f,
                              COLOR_ACCENT_R + ha * 0.1f, COLOR_ACCENT_G + ha * 0.1f, COLOR_ACCENT_B + ha * 0.1f, 1.0f, screen_w, screen_h);
        }

        /* Thumb with hover scale */
        float thumb_r = 10.0f + ha * 2.0f;
        float thumb_x = x + fill_w - thumb_r;
        float thumb_y_pos = y + h / 2.0f - thumb_r;
        /* Thumb shadow */
        draw_rounded_rect(thumb_x + 1.0f, thumb_y_pos + 2.0f, thumb_r * 2.0f, thumb_r * 2.0f, thumb_r,
                          0.0f, 0.0f, 0.0f, 0.15f, screen_w, screen_h);
        draw_rounded_rect(thumb_x, thumb_y_pos, thumb_r * 2.0f, thumb_r * 2.0f, thumb_r,
                          COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);

        /* Value text */
        char val_text[64];
        snprintf(val_text, sizeof(val_text), "%.0f", wd->value);
        draw_text(val_text, x + w + 10.0f, y + 4.0f, 1.4f,
                  COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 1.0f, screen_w, screen_h);
        break;
    }

    case WIDGET_TEXTBOX: {
        float ha = wd->hover_anim;
        float fa = wd->focus_anim;
        /* Background */
        float bg_r = COLOR_PANEL_R + fa * 0.05f + ha * 0.03f;
        float bg_g = COLOR_PANEL_G + fa * 0.05f + ha * 0.03f;
        float bg_b = COLOR_PANEL_B + fa * 0.08f + ha * 0.03f;
        draw_rounded_rect(x, y, w, h, 6.0f, bg_r, bg_g, bg_b, 1.0f, screen_w, screen_h);

        /* Border — focus pulse */
        float pulse = fa * (0.7f + 0.3f * sinf(g_focus_pulse * 3.0f));
        float brd_r = COLOR_DIM_R + fa * (COLOR_ACCENT_R - COLOR_DIM_R);
        float brd_g = COLOR_DIM_G + fa * (COLOR_ACCENT_G - COLOR_DIM_G);
        float brd_b = COLOR_DIM_B + fa * (COLOR_ACCENT_B - COLOR_DIM_B);
        draw_rect_outline(x, y, w, h, brd_r, brd_g, brd_b, pulse, screen_w, screen_h);
        /* Focus glow */
        if (fa > 0.1f) {
            draw_rect_outline(x - 1.0f, y - 1.0f, w + 2.0f, h + 2.0f,
                              COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, fa * 0.25f, screen_w, screen_h);
        }

        /* Text content */
        const char* display = g_textbox_name->input_text;
        if (display[0] == '\0') display = wd->text; /* placeholder */
        float text_alpha = (g_textbox_name->input_text[0] == '\0') ? 0.5f : 1.0f;
        draw_text(display, x + 10.0f, y + 8.0f, 1.6f,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, text_alpha, screen_w, screen_h);

        /* Cursor blink */
        if (wd->focused) {
            static double blink_timer = 0.0;
            blink_timer += 0.016;
            if (fmod(blink_timer, 1.0) < 0.5) {
                float cursor_x = x + 10.0f + text_width(g_textbox_name->input_text, 1.6f);
                draw_rect(cursor_x, y + 6.0f, 2.0f, h - 12.0f,
                          COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
            }
        }
        break;
    }

    case WIDGET_PROGRESSBAR: {
        /* Track */
        draw_rounded_rect(x, y, w, h, 6.0f,
                          COLOR_PANEL_R * 1.5f, COLOR_PANEL_G * 1.5f, COLOR_PANEL_B * 1.5f, 1.0f,
                          screen_w, screen_h);

        if (wd->indeterminate) {
            /* Animated indeterminate bar */
            static double indet_pos = 0.0;
            indet_pos += 0.02;
            if (indet_pos > 1.5) indet_pos = -0.3;
            float bar_w = w * 0.3f;
            float bar_x = x + (float)indet_pos * w;
            if (bar_x + bar_w > x + w) bar_w = (x + w) - bar_x;
            if (bar_x < x) {
                bar_w -= (x - bar_x);
                bar_x = x;
            }
            if (bar_w > 0.0f) {
                draw_rounded_rect(bar_x, y, bar_w, h, 6.0f,
                                  COLOR_GOLD_R, COLOR_GOLD_G, COLOR_GOLD_B, 1.0f, screen_w, screen_h);
            }
        } else {
            /* Determinate fill */
            double pct = (wd->max_val > wd->min_val) ?
                         (wd->value - wd->min_val) / (wd->max_val - wd->min_val) : 0.0;
            if (pct < 0.0) pct = 0.0;
            if (pct > 1.0) pct = 1.0;
            float fill_w = (float)pct * w;
            if (fill_w > 0.0f) {
                draw_rounded_rect(x, y, fill_w, h, 6.0f,
                                  COLOR_GREEN_R, COLOR_GREEN_G, COLOR_GREEN_B, 1.0f, screen_w, screen_h);
            }
            /* Percentage text */
            char pct_text[32];
            snprintf(pct_text, sizeof(pct_text), "%.0f%%", pct * 100.0);
            draw_text(pct_text, x + w / 2.0f - text_width(pct_text, 1.4f) / 2.0f, y + 4.0f, 1.4f,
                      COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
        }
        break;
    }

    case WIDGET_DROPDOWN: {
        /* Closed state: show selected option or placeholder */
        float ha = wd->hover_anim;
        float bg_r = COLOR_PANEL_R + ha * 0.07f;
        float bg_g = COLOR_PANEL_G + ha * 0.07f;
        float bg_b = COLOR_PANEL_B + ha * 0.1f;
        draw_rounded_rect(x, y, w, h, 6.0f, bg_r, bg_g, bg_b, 1.0f, screen_w, screen_h);

        /* Border */
        float brd_r = wd->open ? COLOR_ACCENT_R : COLOR_DIM_R;
        float brd_g = wd->open ? COLOR_ACCENT_G : COLOR_DIM_G;
        float brd_b = wd->open ? COLOR_ACCENT_B : COLOR_DIM_B;
        draw_rect_outline(x, y, w, h, brd_r, brd_g, brd_b, 0.7f, screen_w, screen_h);

        /* Selected text or placeholder */
        const char* display_text;
        if (wd->selected_index >= 0 && wd->selected_index < wd->option_count) {
            display_text = wd->options[wd->selected_index];
        } else {
            display_text = wd->text; /* placeholder */
        }
        float text_alpha = (wd->selected_index >= 0) ? 1.0f : 0.5f;
        draw_text(display_text, x + 10.0f, y + 8.0f, 1.6f,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, text_alpha, screen_w, screen_h);

        /* Down arrow on the right */
        float arrow_x = x + w - 20.0f;
        float arrow_y = y + h / 2.0f - 3.0f;
        draw_text("v", arrow_x, arrow_y, 1.4f,
                  COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 1.0f, screen_w, screen_h);

        /* Open dropdown list */
        if (wd->open) {
            float item_h = 30.0f;
            float list_y = y + h + 2.0f;
            float list_h = (float)wd->option_count * item_h;

            /* Background panel */
            draw_rounded_rect(x, list_y, w, list_h, 6.0f,
                              0.18f, 0.18f, 0.28f, 1.0f, screen_w, screen_h);
            draw_rect_outline(x, list_y, w, list_h,
                              COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 0.5f, screen_w, screen_h);

            /* Each option */
            for (int i = 0; i < wd->option_count; i++) {
                float item_y = list_y + (float)i * item_h;
                bool is_hovered = (g_mouse_x >= (int)x && g_mouse_x <= (int)(x + w) &&
                                   g_mouse_y >= (int)item_y && g_mouse_y <= (int)(item_y + item_h));

                if (is_hovered) {
                    draw_rounded_rect(x + 2.0f, item_y + 1.0f, w - 4.0f, item_h - 2.0f, 4.0f,
                                      COLOR_ACCENT_R * 0.5f, COLOR_ACCENT_G * 0.5f, COLOR_ACCENT_B * 0.5f, 0.8f,
                                      screen_w, screen_h);
                }
                if (i == wd->selected_index) {
                    /* Highlight selected */
                    draw_rounded_rect(x + 2.0f, item_y + 1.0f, w - 4.0f, item_h - 2.0f, 4.0f,
                                      COLOR_ACCENT_R * 0.3f, COLOR_ACCENT_G * 0.3f, COLOR_ACCENT_B * 0.3f, 0.5f,
                                      screen_w, screen_h);
                }
                draw_text(wd->options[i], x + 12.0f, item_y + 7.0f, 1.5f,
                          COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, screen_w, screen_h);
            }
        }
        break;
    }
    }
}

/* =========================================================================
 * Hit testing
 * ========================================================================= */

static UIWidget* widget_at(float mx, float my) {
    float adjusted_y = my + g_scroll_y;
    for (int i = g_widget_count - 1; i >= 0; i--) {
        UIWidget* wd = &g_widgets[i];
        if (wd->parent_section && !is_widget_visible(wd)) continue;
        if (wd->kind == WIDGET_LABEL) continue; /* labels aren't interactive */
        if (mx >= wd->x && mx <= wd->x + wd->w &&
            adjusted_y >= wd->y && adjusted_y <= wd->y + wd->h) {
            return wd;
        }
    }
    return NULL;
}

/* =========================================================================
 * Timing
 * ========================================================================= */

static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =========================================================================
 * Render callback
 * ========================================================================= */

static void on_render(JiWindow* window, void* user_data) {
    (void)user_data;

    int w, h;
    ji_window_get_size(window, &w, &h);

    /* Delta time */
    double now = get_time();
    double dt = g_last_time > 0.0 ? (now - g_last_time) : 1.0 / 60.0;
    g_last_time = now;

    /* FPS calculation */
    g_frame_count++;
    g_fps_timer += dt;
    if (g_fps_timer >= 0.5) {
        g_current_fps = (double)g_frame_count / g_fps_timer;
        g_frame_count = 0;
        g_fps_timer = 0.0;
    }

    /* Animate progress bar */
    if (g_check_option3 && g_check_option3->active) { /* animations enabled */
        if (g_progress_direction) {
            g_progress_value += dt * 15.0;
            if (g_progress_value >= 100.0) { g_progress_value = 100.0; g_progress_direction = false; }
        } else {
            g_progress_value -= dt * 15.0;
            if (g_progress_value <= 0.0) { g_progress_value = 0.0; g_progress_direction = true; }
        }
        if (g_progress_bar) g_progress_bar->value = g_progress_value;
    }

    /* Update smooth hover/press/focus animations for all widgets */
    g_focus_pulse += (float)dt;
    for (int i = 0; i < g_widget_count; i++) {
        UIWidget* wd = &g_widgets[i];
        float speed = 8.0f; /* animation speed */
        /* Hover animation */
        float target_h = wd->hovered ? 1.0f : 0.0f;
        wd->hover_anim += (target_h - wd->hover_anim) * (float)dt * speed;
        if (wd->hover_anim < 0.001f) wd->hover_anim = 0.0f;
        if (wd->hover_anim > 0.999f) wd->hover_anim = 1.0f;
        /* Press animation */
        float target_p = wd->pressed ? 1.0f : 0.0f;
        wd->press_anim += (target_p - wd->press_anim) * (float)dt * speed * 1.5f;
        if (wd->press_anim < 0.001f) wd->press_anim = 0.0f;
        if (wd->press_anim > 0.999f) wd->press_anim = 1.0f;
        /* Focus animation */
        float target_f = wd->focused ? 1.0f : 0.0f;
        wd->focus_anim += (target_f - wd->focus_anim) * (float)dt * speed;
        if (wd->focus_anim < 0.001f) wd->focus_anim = 0.0f;
        if (wd->focus_anim > 0.999f) wd->focus_anim = 1.0f;
    }

    /* Tooltip timer */
    UIWidget* hit = widget_at((float)g_mouse_x, (float)g_mouse_y);
    if (hit && hit->tooltip[0] != '\0') {
        if (g_tooltip_widget == hit) {
            g_tooltip_timer += (float)dt;
        } else {
            g_tooltip_widget = hit;
            g_tooltip_timer = 0.0f;
        }
    } else {
        g_tooltip_widget = NULL;
        g_tooltip_timer = 0.0f;
    }

    /* Update button click label */
    for (int i = 0; i < g_widget_count; i++) {
        if (g_widgets[i].parent_section == g_section_buttons &&
            g_widgets[i].kind == WIDGET_LABEL) {
            snprintf(g_widgets[i].text, sizeof(g_widgets[i].text),
                     "Button clicks: %d", g_button_clicks);
        }
    }

    /* Clear */
    glClearColor(COLOR_BG_R, COLOR_BG_G, COLOR_BG_B, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Render all widgets (skip open dropdown list popups — render as overlays) */
    for (int i = 0; i < g_widget_count; i++) {
        UIWidget* wd = &g_widgets[i];
        if (wd->kind == WIDGET_DROPDOWN && wd->open) {
            /* Render only the closed-state button part */
            float dx = wd->x;
            float dy = wd->y - g_scroll_y;
            float dw = wd->w;
            float dh = wd->h;
            float bg_r = wd->hovered ? 0.22f : COLOR_PANEL_R;
            float bg_g = wd->hovered ? 0.22f : COLOR_PANEL_G;
            float bg_b = wd->hovered ? 0.32f : COLOR_PANEL_B;
            draw_rounded_rect(dx, dy, dw, dh, 6.0f, bg_r, bg_g, bg_b, 1.0f, w, h);
            draw_rect_outline(dx, dy, dw, dh, COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 0.7f, w, h);
            const char* dtxt;
            if (wd->selected_index >= 0 && wd->selected_index < wd->option_count)
                dtxt = wd->options[wd->selected_index];
            else
                dtxt = wd->text;
            float dtalpha = (wd->selected_index >= 0) ? 1.0f : 0.5f;
            draw_text(dtxt, dx + 10.0f, dy + 8.0f, 1.6f,
                      COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, dtalpha, w, h);
            draw_text("v", dx + dw - 20.0f, dy + dh / 2.0f - 3.0f, 1.4f,
                      COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 1.0f, w, h);
        } else {
            render_widget(&g_widgets[i], w, h);
        }
    }

    /* Render open dropdown lists as overlays (on top of everything) */
    for (int i = 0; i < g_widget_count; i++) {
        UIWidget* wd = &g_widgets[i];
        if (wd->kind != WIDGET_DROPDOWN || !wd->open) continue;
        if (wd->parent_section && !is_widget_visible(wd)) continue;

        float dx = wd->x;
        float dy = wd->y - g_scroll_y;
        float dw = wd->w;
        float item_h = 30.0f;
        float list_y = dy + wd->h + 2.0f;
        float list_h = (float)wd->option_count * item_h;

        /* Shadow backdrop */
        draw_rounded_rect(dx - 2.0f, list_y - 2.0f, dw + 4.0f, list_h + 4.0f, 8.0f,
                          0.0f, 0.0f, 0.0f, 0.3f, w, h);

        /* Background panel */
        draw_rounded_rect(dx, list_y, dw, list_h, 6.0f,
                          0.18f, 0.18f, 0.28f, 1.0f, w, h);
        draw_rect_outline(dx, list_y, dw, list_h,
                          COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 0.5f, w, h);

        /* Each option */
        for (int j = 0; j < wd->option_count; j++) {
            float item_y = list_y + (float)j * item_h;
            bool is_hovered = (g_mouse_x >= (int)dx && g_mouse_x <= (int)(dx + dw) &&
                               g_mouse_y >= (int)item_y && g_mouse_y <= (int)(item_y + item_h));

            if (is_hovered) {
                draw_rounded_rect(dx + 2.0f, item_y + 1.0f, dw - 4.0f, item_h - 2.0f, 4.0f,
                                  COLOR_ACCENT_R * 0.5f, COLOR_ACCENT_G * 0.5f, COLOR_ACCENT_B * 0.5f, 0.8f,
                                  w, h);
            }
            if (j == wd->selected_index) {
                draw_rounded_rect(dx + 2.0f, item_y + 1.0f, dw - 4.0f, item_h - 2.0f, 4.0f,
                                  COLOR_ACCENT_R * 0.3f, COLOR_ACCENT_G * 0.3f, COLOR_ACCENT_B * 0.3f, 0.5f,
                                  w, h);
            }
            draw_text(wd->options[j], dx + 12.0f, item_y + 7.0f, 1.5f,
                      COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, 1.0f, w, h);
        }
    }

    /* FPS counter at top right */
    if (g_check_option2 && g_check_option2->active) {
        char fps_text[64];
        snprintf(fps_text, sizeof(fps_text), "FPS: %.0f", g_current_fps);
        float tw = text_width(fps_text, 1.6f);
        draw_text(fps_text, (float)w - tw - 15.0f, 15.0f, 1.6f,
                  COLOR_GREEN_R, COLOR_GREEN_G, COLOR_GREEN_B, 1.0f, w, h);
    }

    /* Language & font size indicator at top left */
    {
        static const char* lang_names[] = {"EN", "FR", "JP", "ES", "DE"};
        char info_text[64];
        snprintf(info_text, sizeof(info_text), "Lang: %s | Size: %.0f%%",
                 lang_names[g_language], g_font_scale * 100.0f);
        draw_text(info_text, 15.0f, 15.0f, 1.4f,
                  COLOR_DIM_R, COLOR_DIM_G, COLOR_DIM_B, 0.8f, w, h);
    }

    /* Tooltip rendering */
    if (g_tooltip_widget && g_tooltip_timer >= TOOLTIP_DELAY) {
        float fade = (g_tooltip_timer - TOOLTIP_DELAY) / TOOLTIP_FADE;
        if (fade > 1.0f) fade = 1.0f;
        const char* tip = g_tooltip_widget->tooltip;
        float tip_scale = 1.4f;
        float tip_w = text_width(tip, tip_scale) + 16.0f;
        float tip_h = 24.0f;
        float tip_x = (float)g_mouse_x + 15.0f;
        float tip_y = (float)g_mouse_y - tip_h - 5.0f;
        /* Keep tooltip on screen */
        if (tip_x + tip_w > (float)w) tip_x = (float)w - tip_w - 5.0f;
        if (tip_y < 0.0f) tip_y = (float)g_mouse_y + 20.0f;
        /* Shadow */
        draw_rounded_rect(tip_x + 2.0f, tip_y + 2.0f, tip_w, tip_h, 4.0f,
                          0.0f, 0.0f, 0.0f, 0.2f * fade, w, h);
        /* Background */
        draw_rounded_rect(tip_x, tip_y, tip_w, tip_h, 4.0f,
                          0.15f, 0.15f, 0.22f, 0.95f * fade, w, h);
        /* Border */
        draw_rect_outline(tip_x, tip_y, tip_w, tip_h,
                          COLOR_ACCENT_R, COLOR_ACCENT_G, COLOR_ACCENT_B, 0.4f * fade, w, h);
        /* Text */
        draw_text(tip, tip_x + 8.0f, tip_y + 5.0f, tip_scale,
                  COLOR_TEXT_R, COLOR_TEXT_G, COLOR_TEXT_B, fade, w, h);
    }
}

/* =========================================================================
 * Event callback
 * ========================================================================= */

static void on_event(JiWindow* window, const JiEvent* event, void* user_data) {
    (void)window; (void)user_data;

    switch (event->kind) {
    case JI_EVENT_KEY_PRESS:
        if (event->key == JI_KEY_ESCAPE) {
            g_running = false;
        }
        /* Handle text input for focused textbox */
        if (g_textbox_name && g_textbox_name->focused) {
            /* Simple ASCII input handling */
            if (event->key >= 0x0020 && event->key <= 0x007E) {
                int len = (int)strlen(g_textbox_name->input_text);
                if (len < (int)sizeof(g_textbox_name->input_text) - 1) {
                    g_textbox_name->input_text[len] = (char)event->key;
                    g_textbox_name->input_text[len + 1] = '\0';
                }
            } else if (event->key == JI_KEY_BACKSPACE) {
                int len = (int)strlen(g_textbox_name->input_text);
                if (len > 0) g_textbox_name->input_text[len - 1] = '\0';
            }
        }
        break;

    case JI_EVENT_MOUSE_PRESS:
        if (event->button == JI_MOUSE_LEFT) {
            g_mouse_down = true;
            float mx = (float)event->mouse_x;
            float my = (float)event->mouse_y;

            /* Priority: check open dropdown overlay lists FIRST */
            {
                bool handled_by_overlay = false;
                for (int i = 0; i < g_widget_count; i++) {
                    UIWidget* dd = &g_widgets[i];
                    if (dd->kind != WIDGET_DROPDOWN || !dd->open) continue;
                    if (dd->parent_section && !is_widget_visible(dd)) continue;

                    float item_h = 30.0f;
                    float list_y = dd->y + dd->h + 2.0f - g_scroll_y;
                    float list_h = (float)dd->option_count * item_h;

                    if (mx >= dd->x && mx <= dd->x + dd->w &&
                        my >= list_y && my <= list_y + list_h) {
                        /* Clicked inside this dropdown's overlay list */
                        for (int j = 0; j < dd->option_count; j++) {
                            float item_y = list_y + (float)j * item_h;
                            if (my >= item_y && my <= item_y + item_h) {
                                dd->selected_index = j;
                                dd->open = false;
                                /* Apply effects based on which dropdown */
                                if (dd == g_dropdown_theme) {
                                    switch (j) {
                                    case 0: g_theme = THEME_DARK; break;
                                    case 1: g_theme = THEME_LIGHT; break;
                                    case 2: g_theme = THEME_BLUE; break;
                                    case 3: g_theme = THEME_PURPLE; break;
                                    }
                                } else if (dd == g_dropdown_lang) {
                                    g_language = j;
                                    apply_language();
                                } else if (dd == g_dropdown_size) {
                                    switch (j) {
                                    case 0: g_font_scale = 0.75f; break;  /* Small */
                                    case 1: g_font_scale = 1.0f; break;   /* Medium */
                                    case 2: g_font_scale = 1.3f; break;   /* Large */
                                    case 3: g_font_scale = 1.6f; break;   /* Extra Large */
                                    }
                                }
                                break;
                            }
                        }
                        handled_by_overlay = true;
                        break;
                    }
                    /* Also check if clicking the dropdown button itself while open */
                    float btn_y = dd->y - g_scroll_y;
                    if (mx >= dd->x && mx <= dd->x + dd->w &&
                        my >= btn_y && my <= btn_y + dd->h) {
                        dd->open = false;
                        handled_by_overlay = true;
                        break;
                    }
                }
                if (handled_by_overlay) break;
            }

            UIWidget* hit = widget_at(mx, my);
            if (hit) {
                switch (hit->kind) {
                case WIDGET_SECTION:
                    hit->active = !hit->active; /* toggle collapse */
                    break;
                case WIDGET_BUTTON:
                    hit->pressed = true;
                    if (hit == g_btn_click) {
                        g_button_clicks++;
                    } else if (hit == g_btn_toggle) {
                        hit->active = !hit->active;
                        if (hit->active) {
                            strncpy(hit->text, "ON", sizeof(hit->text) - 1);
                        } else {
                            strncpy(hit->text, "OFF", sizeof(hit->text) - 1);
                        }
                    }
                    break;
                case WIDGET_CHECKBOX:
                    hit->active = !hit->active;
                    break;
                case WIDGET_TEXTBOX:
                    /* Focus this textbox, unfocus others */
                    for (int i = 0; i < g_widget_count; i++) {
                        if (g_widgets[i].kind == WIDGET_TEXTBOX)
                            g_widgets[i].focused = false;
                    }
                    hit->focused = true;
                    break;
                case WIDGET_SLIDER:
                    /* Start slider drag */
                    hit->pressed = true;
                    {
                        float rel_x = mx - hit->x;
                        double pct = (double)(rel_x / hit->w);
                        if (pct < 0.0) pct = 0.0;
                        if (pct > 1.0) pct = 1.0;
                        hit->value = hit->min_val + pct * (hit->max_val - hit->min_val);
                    }
                    break;
                case WIDGET_DROPDOWN:
                    if (hit->open) {
                        /* Check if clicking on an option in the list */
                        float item_h = 30.0f;
                        float list_y = hit->y + hit->h + 2.0f - g_scroll_y;
                        for (int i = 0; i < hit->option_count; i++) {
                            float item_y = list_y + (float)i * item_h;
                            if (mx >= hit->x && mx <= hit->x + hit->w &&
                                my >= item_y && my <= item_y + item_h) {
                                hit->selected_index = i;
                                hit->open = false;
                                /* Apply theme if this is the theme dropdown */
                                if (hit == g_dropdown_theme) {
                                    switch (i) {
                                    case 0: g_theme = THEME_DARK; break;
                                    case 1: g_theme = THEME_LIGHT; break;
                                    case 2: g_theme = THEME_BLUE; break;
                                    case 3: g_theme = THEME_PURPLE; break;
                                    }
                                }
                                break;
                            }
                        }
                        /* Close if clicked outside the list */
                        if (my < hit->y - g_scroll_y || my > list_y + (float)hit->option_count * item_h) {
                            hit->open = false;
                        }
                    } else {
                        /* Close all other dropdowns, open this one */
                        for (int i = 0; i < g_widget_count; i++) {
                            if (g_widgets[i].kind == WIDGET_DROPDOWN)
                                g_widgets[i].open = false;
                        }
                        hit->open = true;
                    }
                    break;
                default:
                    break;
                }
            } else {
                /* Check if clicking inside an open dropdown list overlay */
                bool clicked_dropdown_list = false;
                for (int i = 0; i < g_widget_count; i++) {
                    UIWidget* dd = &g_widgets[i];
                    if (dd->kind != WIDGET_DROPDOWN || !dd->open) continue;
                    if (dd->parent_section && !is_widget_visible(dd)) continue;

                    float item_h = 30.0f;
                    float list_y = dd->y + dd->h + 2.0f - g_scroll_y;
                    float list_h = (float)dd->option_count * item_h;

                    if (mx >= dd->x && mx <= dd->x + dd->w &&
                        my >= list_y && my <= list_y + list_h) {
                        /* Clicked inside this dropdown's list */
                        for (int j = 0; j < dd->option_count; j++) {
                            float item_y = list_y + (float)j * item_h;
                            if (my >= item_y && my <= item_y + item_h) {
                                dd->selected_index = j;
                                dd->open = false;
                                /* Apply effects based on which dropdown */
                                if (dd == g_dropdown_theme) {
                                    switch (j) {
                                    case 0: g_theme = THEME_DARK; break;
                                    case 1: g_theme = THEME_LIGHT; break;
                                    case 2: g_theme = THEME_BLUE; break;
                                    case 3: g_theme = THEME_PURPLE; break;
                                    }
                                } else if (dd == g_dropdown_lang) {
                                    g_language = j;
                                    apply_language();
                                } else if (dd == g_dropdown_size) {
                                    switch (j) {
                                    case 0: g_font_scale = 0.75f; break;
                                    case 1: g_font_scale = 1.0f; break;
                                    case 2: g_font_scale = 1.3f; break;
                                    case 3: g_font_scale = 1.6f; break;
                                    }
                                }
                                break;
                            }
                        }
                        clicked_dropdown_list = true;
                        break;
                    }
                }

                if (!clicked_dropdown_list) {
                    /* Click on empty space — unfocus textboxes & close dropdowns */
                    for (int i = 0; i < g_widget_count; i++) {
                        if (g_widgets[i].kind == WIDGET_TEXTBOX)
                            g_widgets[i].focused = false;
                        if (g_widgets[i].kind == WIDGET_DROPDOWN)
                            g_widgets[i].open = false;
                    }
                }
            }
        }
        break;

    case JI_EVENT_MOUSE_RELEASE:
        if (event->button == JI_MOUSE_LEFT) {
            g_mouse_down = false;
            /* Release all pressed buttons/sliders */
            for (int i = 0; i < g_widget_count; i++) {
                if (g_widgets[i].kind == WIDGET_BUTTON)
                    g_widgets[i].pressed = false;
                if (g_widgets[i].kind == WIDGET_SLIDER)
                    g_widgets[i].pressed = false;
            }
        }
        break;

    case JI_EVENT_MOUSE_MOVE: {
        g_mouse_x = (int)event->mouse_x;
        g_mouse_y = (int)event->mouse_y;

        /* Update hover state */
        UIWidget* hit = widget_at((float)g_mouse_x, (float)g_mouse_y);
        for (int i = 0; i < g_widget_count; i++) {
            g_widgets[i].hovered = false;
        }
        if (hit) hit->hovered = true;

        /* Slider drag */
        if (g_mouse_down) {
            for (int i = 0; i < g_widget_count; i++) {
                if (g_widgets[i].kind == WIDGET_SLIDER && g_widgets[i].pressed) {
                    float rel_x = (float)g_mouse_x - g_widgets[i].x;
                    double pct = (double)(rel_x / g_widgets[i].w);
                    if (pct < 0.0) pct = 0.0;
                    if (pct > 1.0) pct = 1.0;
                    g_widgets[i].value = g_widgets[i].min_val +
                                          pct * (g_widgets[i].max_val - g_widgets[i].min_val);
                }
            }
        }
        break;
    }

    case JI_EVENT_MOUSE_WHEEL: {
        /* Scroll */
        g_scroll_y -= (float)event->wheel_delta * 30.0f;
        if (g_scroll_y < 0.0f) g_scroll_y = 0.0f;
        float max_scroll = g_content_height - 600.0f;
        if (max_scroll < 0.0f) max_scroll = 0.0f;
        if (g_scroll_y > max_scroll) g_scroll_y = max_scroll;
        break;
    }

    case JI_EVENT_CLOSE:
        g_running = false;
        break;

    default:
        break;
    }
}

/* =========================================================================
 * Close callback
 * ========================================================================= */

static void on_close(JiWindow* window, void* user_data) {
    (void)window; (void)user_data;
    g_running = false;
}

/* =========================================================================
 * Main
 * ========================================================================= */

int main(void) {
    JiResultCode result = ji_initialize();
    if (JI_FAILED(result)) {
        fprintf(stderr, "Failed to initialize JiUI: %s\n",
                ji_result_to_string(result));
        return 1;
    }

    printf("=== JiUI Comprehensive Showcase Demo ===\n");
    printf("JiUI v%s\n", ji_version());
    printf("Features: Buttons, Sliders, Checkboxes, TextBoxes, ProgressBars\n");
    printf("Controls: Click widgets | Scroll to navigate | ESC to quit\n\n");

    /* Build the UI */
    build_ui();

    /* Create X11 backend */
    JiPlatformBackend* backend = NULL;
#ifdef JIUI_ENABLE_WAYLAND
    backend = ji_wayland_backend_create();
#endif
#ifdef JIUI_ENABLE_X11
    if (!backend) backend = ji_x11_backend_create();
#endif
    if (!backend) { fprintf(stderr, "No platform backend available\n"); return 1; }
    if (!backend) {
        fprintf(stderr, "Failed to create X11 backend. Is X11 running?\n");
        ji_shutdown();
        return 1;
    }

    /* Create window with OpenGL */
    JiWindow* window = ji_window_create(
        "JiUI - Widget Showcase",
        1200, 800,
        JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE,
        backend
    );

    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        ji_shutdown();
        return 1;
    }

    /* Initialize font texture */
    init_font_texture();

    /* Set callbacks */
    ji_window_set_render_callback(window, on_render, NULL);
    ji_window_set_event_callback(window, on_event, NULL);
    ji_window_set_close_callback(window, on_close, NULL);

    /* Initialize timing */
    g_last_time = get_time();
    g_fps_timer = 0.0;

    /* Main loop */
    while (g_running && ji_window_is_open(window)) {
        if (!ji_window_frame(window)) {
            break;
        }
    }

    /* Cleanup */
    if (g_font_tex) {
        glDeleteTextures(1, (GLuint*)&g_font_tex);
    }
    free(g_font_data);

    ji_window_destroy(window);
    ji_shutdown();

    printf("\nShowcase demo closed. Final FPS: %.1f\n", g_current_fps);
    return 0;
}
