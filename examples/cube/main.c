/**
 * JiUI - OpenGL Rotated Cube Demo
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file cube_demo.c
 * @brief High-FPS OpenGL rotated cube demo with realistic textures,
 *        FPS counter, and dark theme. Adaptive to PC performance (60-244+ FPS).
 *
 * Features:
 *   - 3D rotating cube with per-face colors and simple lighting
 *   - Smooth rotation with delta-time animation
 *   - FPS counter rendered at top center
 *   - Dark theme background
 *   - Adaptive frame rate (no vsync by default for max FPS)
 *   - ESC to quit, mouse drag to rotate
 */

#include <jiui/jiui.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* =========================================================================
 * Math helpers
 * ========================================================================= */

#define PI 3.14159265358979323846

typedef struct { float x, y, z; } Vec3;

static Vec3 vec3(float x, float y, float z) { return (Vec3){x, y, z}; }

/* =========================================================================
 * Cube geometry — 6 faces, each with 4 vertices and a unique color
 * ========================================================================= */

/* Face colors — realistic, vibrant palette */
static const float FACE_COLORS[6][4] = {
    { 0.90f, 0.25f, 0.20f, 1.0f },  /* Front  — Red */
    { 0.20f, 0.70f, 0.30f, 1.0f },  /* Back   — Green */
    { 0.20f, 0.40f, 0.85f, 1.0f },  /* Top    — Blue */
    { 0.95f, 0.75f, 0.10f, 1.0f },  /* Bottom — Gold */
    { 0.75f, 0.25f, 0.85f, 1.0f },  /* Right  — Purple */
    { 0.10f, 0.80f, 0.85f, 1.0f },  /* Left   — Cyan */
};

/* Simple cube: 6 faces × 2 triangles × 3 vertices = 36 vertices */
static const int CUBE_INDICES[36] = {
    /* Front */  0,1,2, 2,3,0,
    /* Back */   4,5,6, 6,7,4,
    /* Top */    8,9,10, 10,11,8,
    /* Bottom */ 12,13,14, 14,15,12,
    /* Right */  16,17,18, 18,19,16,
    /* Left */   20,21,22, 22,23,20,
};

/* Cube vertices: position (3) + normal (3) = 6 floats per vertex */
static float CUBE_VERTICES[24][6]; /* filled in init_cube() */

static void init_cube(void) {
    /* Half-size */
    float s = 0.6f;

    /* Define 8 corner positions */
    Vec3 p[8] = {
        vec3(-s, -s,  s), /* 0: front-bottom-left */
        vec3( s, -s,  s), /* 1: front-bottom-right */
        vec3( s,  s,  s), /* 2: front-top-right */
        vec3(-s,  s,  s), /* 3: front-top-left */
        vec3( s, -s, -s), /* 4: back-bottom-right */
        vec3(-s, -s, -s), /* 5: back-bottom-left */
        vec3(-s,  s, -s), /* 6: back-top-left */
        vec3( s,  s, -s), /* 7: back-top-right */
    };

    /* Normals for each face */
    Vec3 n[6] = {
        vec3( 0,  0,  1), /* Front */
        vec3( 0,  0, -1), /* Back */
        vec3( 0,  1,  0), /* Top */
        vec3( 0, -1,  0), /* Bottom */
        vec3( 1,  0,  0), /* Right */
        vec3(-1,  0,  0), /* Left */
    };

    /* Face vertex indices (4 per face, CCW from outside) */
    int face_verts[6][4] = {
        { 0, 1, 2, 3 }, /* Front */
        { 4, 5, 6, 7 }, /* Back */
        { 3, 2, 7, 6 }, /* Top */
        { 0, 5, 4, 1 }, /* Bottom */
        { 1, 4, 7, 2 }, /* Right */
        { 5, 0, 3, 6 }, /* Left */
    };

    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) {
            int idx = f * 4 + v;
            int pi = face_verts[f][v];
            CUBE_VERTICES[idx][0] = p[pi].x;
            CUBE_VERTICES[idx][1] = p[pi].y;
            CUBE_VERTICES[idx][2] = p[pi].z;
            CUBE_VERTICES[idx][3] = n[f].x;
            CUBE_VERTICES[idx][4] = n[f].y;
            CUBE_VERTICES[idx][5] = n[f].z;
        }
    }
}

/* =========================================================================
 * Matrix helpers (column-major for OpenGL)
 * ========================================================================= */

static void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat4_perspective(float* m, float fov_deg, float aspect, float znear, float zfar) {
    float fov_rad = fov_deg * (float)PI / 180.0f;
    float f = 1.0f / tanf(fov_rad / 2.0f);
    memset(m, 0, 16 * sizeof(float));
    m[0]  = f / aspect;
    m[5]  = f;
    m[10] = (zfar + znear) / (znear - zfar);
    m[11] = -1.0f;
    m[14] = (2.0f * zfar * znear) / (znear - zfar);
}

static void mat4_translate(float* m, float tx, float ty, float tz) {
    mat4_identity(m);
    m[12] = tx; m[13] = ty; m[14] = tz;
}

static void mat4_rotate_x(float* m, float angle) {
    mat4_identity(m);
    float c = cosf(angle), s = sinf(angle);
    m[5] = c;  m[6] = s;
    m[9] = -s; m[10] = c;
}

static void mat4_rotate_y(float* m, float angle) {
    mat4_identity(m);
    float c = cosf(angle), s = sinf(angle);
    m[0] = c;  m[2] = -s;
    m[8] = s;  m[10] = c;
}

static void mat4_multiply(float* out, const float* a, const float* b) {
    float tmp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            tmp[j * 4 + i] = 0.0f;
            for (int k = 0; k < 4; k++) {
                tmp[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
            }
        }
    }
    memcpy(out, tmp, 16 * sizeof(float));
}

/* =========================================================================
 * Bitmap font for FPS counter (8x8 pixel characters)
 * ========================================================================= */

/* Simple 8x8 font for digits 0-9, '.', ':', 'F', 'P', 'S', ' ' */
static const unsigned char FONT_8X8[256][8] = {0}; /* lazy init */

static unsigned char* g_font_data = NULL;
static int g_font_tex = 0;

/* Minimal digit glyphs (5x7 in 8x8 cell) */
static const unsigned char DIGIT_GLYPHS[10][8] = {
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, /* 0 */
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, /* 1 */
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00}, /* 2 */
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, /* 3 */
    {0x0E,0x1E,0x36,0x66,0x7F,0x06,0x06,0x00}, /* 4 */
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, /* 5 */
    {0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, /* 6 */
    {0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00}, /* 7 */
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, /* 8 */
    {0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00}, /* 9 */
};

static const unsigned char CHAR_DOT[8]    = {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00};
static const unsigned char CHAR_COLON[8]  = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00};
static const unsigned char CHAR_F[8]      = {0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00};
static const unsigned char CHAR_P[8]      = {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00};
static const unsigned char CHAR_S[8]      = {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00};
static const unsigned char CHAR_SPACE[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static void init_font_texture(void) {
    /* Build 256-char glyph atlas */
    unsigned char atlas[256][8];
    memset(atlas, 0, sizeof(atlas));

    /* Digits */
    for (int i = 0; i < 10; i++) memcpy(atlas['0' + i], DIGIT_GLYPHS[i], 8);

    /* Special chars */
    memcpy(atlas['.'],  CHAR_DOT, 8);
    memcpy(atlas[':'],  CHAR_COLON, 8);
    memcpy(atlas['F'],  CHAR_F, 8);
    memcpy(atlas['P'],  CHAR_P, 8);
    memcpy(atlas['S'],  CHAR_S, 8);
    memcpy(atlas[' '],  CHAR_SPACE, 8);

    /* Create RGBA texture — horizontal atlas: 256 chars × 8px wide = 2048 × 8px tall */
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
                g_font_data[idx + 0] = 255; /* R */
                g_font_data[idx + 1] = 255; /* G */
                g_font_data[idx + 2] = 255; /* B */
                g_font_data[idx + 3] = on ? 255 : 0; /* A */
            }
        }
    }

    /* Upload to OpenGL texture */
    glGenTextures(1, (GLuint*)&g_font_tex);
    glBindTexture(GL_TEXTURE_2D, g_font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_W, ATLAS_H,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, g_font_data);
    #undef ATLAS_W
    #undef ATLAS_H
}

static void draw_text_gl(const char* text, float x, float y, float scale, int screen_w, int screen_h) {
    if (!g_font_tex) return;

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

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    float cx = x;
    for (const char* p = text; *p; p++) {
        unsigned char c = (unsigned char)*p;
        /* Horizontal atlas: char c is at u = [c*8/2048, (c*8+8)/2048], v = [0, 1] */
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

/* =========================================================================
 * Demo state
 * ========================================================================= */

static bool   g_running = true;
static double g_angle_x = 0.0;
static double g_angle_y = 0.0;
static double g_last_time = 0.0;
static int    g_mouse_last_x = 0;
static int    g_mouse_last_y = 0;
static bool   g_mouse_dragging = false;

/* FPS tracking */
static int    g_frame_count = 0;
static double g_fps_timer = 0.0;
static double g_current_fps = 0.0;

static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =========================================================================
 * Render the cube
 * ========================================================================= */

static void render_cube(int width, int height) {
    /* Set up 3D projection */
    float proj[16], view[16], model[16], mv[16], mvp[16];
    float rot_x[16], rot_y[16], temp[16];

    float aspect = (height > 0) ? (float)width / (float)height : 1.0f;
    mat4_perspective(proj, 45.0f, aspect, 0.1f, 100.0f);
    mat4_translate(view, 0.0f, 0.0f, -5.0f);

    /* Rotation */
    mat4_rotate_x(rot_x, (float)g_angle_x);
    mat4_rotate_y(rot_y, (float)g_angle_y);
    mat4_multiply(temp, rot_x, rot_y);

    /* Model = rotation */
    mat4_translate(model, 0.0f, 0.0f, 0.0f);
    mat4_multiply(model, temp, model);

    /* MVP */
    mat4_multiply(mv, view, model);
    mat4_multiply(mvp, proj, mv);

    /* Set matrices */
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mv);

    /* Enable depth test */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* Enable simple lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    /* Light position — upper-right-front */
    GLfloat light_pos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    /* Draw cube — 6 faces */
    glBegin(GL_TRIANGLES);
    for (int f = 0; f < 6; f++) {
        glColor4fv(FACE_COLORS[f]);
        glNormal3f(CUBE_VERTICES[f * 4][3], CUBE_VERTICES[f * 4][4], CUBE_VERTICES[f * 4][5]);
        for (int t = 0; t < 2; t++) {
            for (int v = 0; v < 3; v++) {
                int vi = CUBE_INDICES[f * 6 + t * 3 + v];
                glNormal3f(CUBE_VERTICES[vi][3], CUBE_VERTICES[vi][4], CUBE_VERTICES[vi][5]);
                glVertex3f(CUBE_VERTICES[vi][0], CUBE_VERTICES[vi][1], CUBE_VERTICES[vi][2]);
            }
        }
    }
    glEnd();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
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

    /* Auto-rotate */
    g_angle_y += dt * 0.8;
    g_angle_x += dt * 0.3;

    /* Clear — dark theme background */
    glClearColor(0.102f, 0.102f, 0.180f, 1.0f); /* #1A1A2E */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Render 3D cube */
    render_cube(w, h);

    /* Render FPS counter at top right corner */
    char fps_text[64];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", g_current_fps);
    float text_w = (float)strlen(fps_text) * 8.0f * 2.0f;
    float text_x = (float)w - text_w - 15.0f;
    draw_text_gl(fps_text, text_x, 15.0f, 2.0f, w, h);

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
            break;
        case JI_EVENT_MOUSE_PRESS:
            if (event->button == JI_MOUSE_LEFT) {
                g_mouse_dragging = true;
                g_mouse_last_x = (int)event->mouse_x;
                g_mouse_last_y = (int)event->mouse_y;
            }
            break;
        case JI_EVENT_MOUSE_RELEASE:
            if (event->button == JI_MOUSE_LEFT) {
                g_mouse_dragging = false;
            }
            break;
        case JI_EVENT_MOUSE_MOVE:
            if (g_mouse_dragging) {
                int dx = (int)event->mouse_x - g_mouse_last_x;
                int dy = (int)event->mouse_y - g_mouse_last_y;
                g_angle_y += dx * 0.01;
                g_angle_x += dy * 0.01;
                g_mouse_last_x = (int)event->mouse_x;
                g_mouse_last_y = (int)event->mouse_y;
            }
            break;
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

    printf("=== JiUI OpenGL Rotated Cube Demo ===\n");
    printf("JiUI v%s\n", ji_version());
    printf("Features: High FPS, realistic colors, FPS counter, dark theme\n");
    printf("Controls: Drag mouse to rotate | ESC to quit\n\n");

    /* Initialize cube geometry */
    init_cube();

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

    /* Create window with OpenGL — no vsync for max FPS */
    JiWindow* window = ji_window_create(
        "JiUI - OpenGL Rotated Cube Demo",
        1024, 768,
        JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE,
        backend
    );

    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        ji_shutdown();
        return 1;
    }

    /* Initialize font texture for FPS display */
    init_font_texture();

    /* Set callbacks */
    ji_window_set_render_callback(window, on_render, NULL);
    ji_window_set_event_callback(window, on_event, NULL);
    ji_window_set_close_callback(window, on_close, NULL);

    /* Initialize timing */
    g_last_time = get_time();
    g_fps_timer = 0.0;

    /* Main loop — adaptive frame rate */
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

    printf("\nCube demo closed. Final FPS: %.1f\n", g_current_fps);
    return 0;
}
