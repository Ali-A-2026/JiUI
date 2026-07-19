/**
 * JiUI - Window Demo Example
 * Demonstrates platform backend initialization, window creation,
 * OpenGL rendering, event handling, and FPS counting.
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <math.h>

/* ---- Global state ---- */
static double g_angle = 0.0;
static int    g_mouse_x = 0;
static int    g_mouse_y = 0;
static bool   g_running = true;

/* ---- Render callback ---- */
static void on_render(JiWindow* window, void* user_data) {
    (void)user_data;

    int w, h;
    ji_window_get_size(window, &w, &h);

    JiDrawingContext* ctx = ji_window_get_drawing_context(window);
    if (!ctx) return;

    /* Clear to dark background */
    ji_draw_clear(ctx, 0xFF1A1A2E);

    /* Draw a rotating colored rectangle */
    double cx = w / 2.0;
    double cy = h / 2.0;
    double size = 150.0;

    /* Push transform for rotation */
    JiMatrix rot = ji_matrix_translation(cx, cy);
    /* Apply rotation via a 2D rotation matrix */
    double cos_a = cos(g_angle);
    double sin_a = sin(g_angle);
    JiMatrix rotation;
    rotation.m11 = cos_a;  rotation.m12 = sin_a;
    rotation.m21 = -sin_a; rotation.m22 = cos_a;
    rotation.m31 = 0.0;    rotation.m32 = 0.0;

    /* Combine: translate to center, then rotate */
    JiMatrix combined;
    combined.m11 = rot.m11 * rotation.m11 + rot.m12 * rotation.m21;
    combined.m12 = rot.m11 * rotation.m12 + rot.m12 * rotation.m22;
    combined.m21 = rot.m21 * rotation.m11 + rot.m22 * rotation.m21;
    combined.m22 = rot.m21 * rotation.m12 + rot.m22 * rotation.m22;
    combined.m31 = rot.m31;
    combined.m32 = rot.m32;

    ji_draw_push_transform(ctx, combined);

    /* Draw the rectangle */
    JiRect rect = ji_rect(-size / 2.0, -size / 2.0, size, size);
    JiBrush fill = ji_brush_solid(0xFFE94560);
    ji_draw_fill_rect(ctx, rect, &fill);

    /* Draw border */
    JiPen border = ji_pen_solid(0xFFFFD700, 3.0);
    ji_draw_stroke_rect(ctx, rect, &border);

    ji_draw_pop_transform(ctx);

    /* Draw FPS counter at top center */
    double fps = ji_window_get_fps(window);
    char fps_text[64];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", fps);

    JiBrush white = ji_brush_solid(0xFFFFFFFF);
    ji_draw_text(ctx, ji_point(cx - 40.0, 20.0), fps_text,
                  &white, "monospace", 18.0);

    /* Draw mouse position */
    char mouse_text[64];
    snprintf(mouse_text, sizeof(mouse_text), "Mouse: (%d, %d)", g_mouse_x, g_mouse_y);
    ji_draw_text(ctx, ji_point(10.0, h - 30.0), mouse_text,
                  &white, "monospace", 14.0);

    /* Advance rotation */
    g_angle += 0.02;
}

/* ---- Event callback ---- */
static void on_event(JiWindow* window, const JiEvent* event, void* user_data) {
    (void)window; (void)user_data;

    switch (event->kind) {
        case JI_EVENT_MOUSE_MOVE:
            g_mouse_x = (int)event->mouse_x;
            g_mouse_y = (int)event->mouse_y;
            break;
        case JI_EVENT_KEY_PRESS:
            if (event->key == JI_KEY_ESCAPE) {
                g_running = false;
            }
            break;
        case JI_EVENT_CLOSE:
            g_running = false;
            break;
        default:
            break;
    }
}

/* ---- Close callback ---- */
static void on_close(JiWindow* window, void* user_data) {
    (void)window; (void)user_data;
    g_running = false;
}

/* ---- Main ---- */
int main(void) {
    JiResultCode result = ji_initialize();
    if (JI_FAILED(result)) {
        fprintf(stderr, "Failed to initialize JiUI: %s\n",
                ji_result_to_string(result));
        return 1;
    }

    printf("JiUI v%s - Window Demo\n", ji_version());

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
        fprintf(stderr, "Failed to create X11 backend\n");
        ji_shutdown();
        return 1;
    }

    /* Create window with OpenGL context */
    JiWindow* window = ji_window_create(
        "JiUI Window Demo",
        800, 600,
        JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE,
        backend
    );

    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        ji_shutdown();
        return 1;
    }

    /* Set callbacks */
    ji_window_set_render_callback(window, on_render, NULL);
    ji_window_set_event_callback(window, on_event, NULL);
    ji_window_set_close_callback(window, on_close, NULL);

    /* Main loop */
    while (g_running && ji_window_is_open(window)) {
        if (!ji_window_frame(window)) {
            break;
        }
    }

    /* Cleanup */
    ji_window_destroy(window);
    ji_shutdown();

    printf("Window demo closed.\n");
    return 0;
}
