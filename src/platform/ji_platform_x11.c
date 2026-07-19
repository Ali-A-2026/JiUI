/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_platform_x11.c
 * @brief X11 + GLX platform backend — window creation, event loop,
 *        OpenGL context management using X11 and GLX.
 */

#ifdef JIUI_ENABLE_X11

/* strdup is POSIX, not C99 */
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "jiui/jiui.h"
#include "jiui/ji_platform.h"
#include "jiui/ji_drawing.h"
#include "jiui/ji_error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

/* =========================================================================
 * X11 backend data
 * ========================================================================= */

typedef struct JiX11Data {
    Display*        display;
    int             screen;
    Window          root;
    XVisualInfo*    visual_info;
    GLXFBConfig     fb_config;
    GLXContext      glx_context;
    Colormap        colormap;
    Atom            wm_delete_atom;
    Atom            wm_protocols;
    Atom            net_wm_state;
    Atom            net_wm_state_fullscreen;

    /* Drawing context (OpenGL-backed) */
    JiDrawingContext drawing_ctx;

    /* Current window for GL context */
    Window          current_window;
    bool            context_current;
} JiX11Data;

/* =========================================================================
 * OpenGL drawing context implementation
 * ========================================================================= */

static void x11_gl_clear(JiDrawingContext* ctx, uint32_t argb) {
    (void)ctx;
    double a = ((argb >> 24) & 0xFF) / 255.0;
    double r = ((argb >> 16) & 0xFF) / 255.0;
    double g = ((argb >>  8) & 0xFF) / 255.0;
    double b = ((argb      ) & 0xFF) / 255.0;
    glClearColor((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void x11_gl_fill_rect(JiDrawingContext* ctx, JiRect rect, const JiBrush* brush) {
    (void)ctx;
    if (!brush || brush->kind != JI_BRUSH_SOLID) return;

    double r = ((brush->v.color >> 16) & 0xFF) / 255.0;
    double g = ((brush->v.color >>  8) & 0xFF) / 255.0;
    double b = ((brush->v.color      ) & 0xFF) / 255.0;
    double a = ((brush->v.color >> 24) & 0xFF) / 255.0;

    glColor4f((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
    glBegin(GL_QUADS);
    glVertex2f((GLfloat)rect.x,                    (GLfloat)rect.y);
    glVertex2f((GLfloat)(rect.x + rect.width),    (GLfloat)rect.y);
    glVertex2f((GLfloat)(rect.x + rect.width),    (GLfloat)(rect.y + rect.height));
    glVertex2f((GLfloat)rect.x,                    (GLfloat)(rect.y + rect.height));
    glEnd();
}

static void x11_gl_stroke_rect(JiDrawingContext* ctx, JiRect rect, const JiPen* pen) {
    (void)ctx;
    if (!pen) return;

    double r = ((pen->brush.v.color >> 16) & 0xFF) / 255.0;
    double g = ((pen->brush.v.color >>  8) & 0xFF) / 255.0;
    double b = ((pen->brush.v.color      ) & 0xFF) / 255.0;
    double a = ((pen->brush.v.color >> 24) & 0xFF) / 255.0;

    glColor4f((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
    glLineWidth((GLfloat)pen->thickness);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)rect.x,                    (GLfloat)rect.y);
    glVertex2f((GLfloat)(rect.x + rect.width),    (GLfloat)rect.y);
    glVertex2f((GLfloat)(rect.x + rect.width),    (GLfloat)(rect.y + rect.height));
    glVertex2f((GLfloat)rect.x,                    (GLfloat)(rect.y + rect.height));
    glEnd();
}

static void x11_gl_draw_line(JiDrawingContext* ctx, JiPoint start, JiPoint end,
                              const JiBrush* brush, double thickness) {
    (void)ctx;
    if (!brush || brush->kind != JI_BRUSH_SOLID) return;

    double r = ((brush->v.color >> 16) & 0xFF) / 255.0;
    double g = ((brush->v.color >>  8) & 0xFF) / 255.0;
    double b = ((brush->v.color      ) & 0xFF) / 255.0;
    double a = ((brush->v.color >> 24) & 0xFF) / 255.0;

    glColor4f((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
    glLineWidth((GLfloat)thickness);
    glBegin(GL_LINES);
    glVertex2f((GLfloat)start.x, (GLfloat)start.y);
    glVertex2f((GLfloat)end.x,   (GLfloat)end.y);
    glEnd();
}

static void x11_gl_fill_geometry(JiDrawingContext* ctx, const JiGeometry* geom,
                                   const JiBrush* brush) {
    if (!geom || !brush || brush->kind != JI_BRUSH_SOLID) return;

    double r = ((brush->v.color >> 16) & 0xFF) / 255.0;
    double g = ((brush->v.color >>  8) & 0xFF) / 255.0;
    double b = ((brush->v.color      ) & 0xFF) / 255.0;
    double a = ((brush->v.color >> 24) & 0xFF) / 255.0;

    glColor4f((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);

    switch (geom->kind) {
        case JI_GEOM_RECT:
            x11_gl_fill_rect(ctx, geom->v.rect, brush);
            break;
        case JI_GEOM_ROUNDED_RECT: {
            /* Approximate rounded rect with a regular rect */
            x11_gl_fill_rect(ctx, geom->v.rounded_rect.rect, brush);
            break;
        }
        case JI_GEOM_ELLIPSE: {
            JiRect bnd = geom->v.ellipse.bounds;
            double cx = bnd.x + bnd.width / 2.0;
            double cy = bnd.y + bnd.height / 2.0;
            double rx = bnd.width / 2.0;
            double ry = bnd.height / 2.0;
            int segments = 64;
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f((GLfloat)cx, (GLfloat)cy);
            for (int i = 0; i <= segments; i++) {
                double angle = 2.0 * 3.14159265358979323846 * (double)i / (double)segments;
                glVertex2f((GLfloat)(cx + rx * cos(angle)),
                           (GLfloat)(cy + ry * sin(angle)));
            }
            glEnd();
            break;
        }
        case JI_GEOM_LINE:
            x11_gl_draw_line(ctx, geom->v.line.start, geom->v.line.end, brush, 1.0);
            break;
        default:
            break;
    }
}

static void x11_gl_stroke_geometry(JiDrawingContext* ctx, const JiGeometry* geom,
                                    const JiPen* pen) {
    if (!geom || !pen) return;

    double r = ((pen->brush.v.color >> 16) & 0xFF) / 255.0;
    double g = ((pen->brush.v.color >>  8) & 0xFF) / 255.0;
    double b = ((pen->brush.v.color      ) & 0xFF) / 255.0;
    double a = ((pen->brush.v.color >> 24) & 0xFF) / 255.0;

    glColor4f((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
    glLineWidth((GLfloat)pen->thickness);

    switch (geom->kind) {
        case JI_GEOM_RECT:
            x11_gl_stroke_rect(ctx, geom->v.rect, pen);
            break;
        case JI_GEOM_ELLIPSE: {
            JiRect bnd = geom->v.ellipse.bounds;
            double cx = bnd.x + bnd.width / 2.0;
            double cy = bnd.y + bnd.height / 2.0;
            double rx = bnd.width / 2.0;
            double ry = bnd.height / 2.0;
            int segments = 64;
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < segments; i++) {
                double angle = 2.0 * 3.14159265358979323846 * (double)i / (double)segments;
                glVertex2f((GLfloat)(cx + rx * cos(angle)),
                           (GLfloat)(cy + ry * sin(angle)));
            }
            glEnd();
            break;
        }
        case JI_GEOM_LINE:
            glBegin(GL_LINES);
            glVertex2f((GLfloat)geom->v.line.start.x, (GLfloat)geom->v.line.start.y);
            glVertex2f((GLfloat)geom->v.line.end.x, (GLfloat)geom->v.line.end.y);
            glEnd();
            break;
        default:
            break;
    }
}

static void x11_gl_draw_text(JiDrawingContext* ctx, JiPoint origin, const char* text,
                               const JiBrush* brush, const char* font_family,
                               double font_size) {
    (void)ctx; (void)origin; (void)text; (void)brush;
    (void)font_family; (void)font_size;
    /* Text rendering via OpenGL requires a font atlas — stub for now.
     * Full text rendering will be implemented with FreeType in a future phase. */
}

static void x11_gl_push_clip(JiDrawingContext* ctx, JiRect clip) {
    (void)ctx;
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint)clip.x,
              (GLint)clip.y,
              (GLsizei)clip.width,
              (GLsizei)clip.height);
}

static void x11_gl_pop_clip(JiDrawingContext* ctx) {
    (void)ctx;
    glDisable(GL_SCISSOR_TEST);
}

static void x11_gl_push_transform(JiDrawingContext* ctx, JiMatrix transform) {
    (void)ctx;
    glPushMatrix();
    GLdouble m[16] = {
        transform.m11, transform.m12, 0.0, 0.0,
        transform.m21, transform.m22, 0.0, 0.0,
        0.0,           0.0,           1.0, 0.0,
        transform.m31, transform.m32, 0.0, 1.0
    };
    glMultMatrixd(m);
}

static void x11_gl_pop_transform(JiDrawingContext* ctx) {
    (void)ctx;
    glPopMatrix();
}

static void x11_gl_resize(JiDrawingContext* ctx, int width, int height) {
    if (!ctx) return;
    ctx->surface_width = width;
    ctx->surface_height = height;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (GLdouble)width, (GLdouble)height, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void x11_gl_present(JiDrawingContext* ctx) {
    (void)ctx;
    glFlush();
}

/* =========================================================================
 * X11 event translation
 * ========================================================================= */

static JiKeyCode x11_keycode_to_ji(KeySym keysym) {
    /* Printable ASCII range */
    if (keysym >= 0x0020 && keysym <= 0x007E) {
        return (JiKeyCode)keysym;
    }
    /* Common special keys */
    switch (keysym) {
        case XK_Escape:    return JI_KEY_ESCAPE;
        case XK_Return:    return JI_KEY_RETURN;
        case XK_Tab:       return JI_KEY_TAB;
        case XK_BackSpace: return JI_KEY_BACKSPACE;
        case XK_Delete:    return JI_KEY_DELETE;
        case XK_Insert:    return JI_KEY_INSERT;
        case XK_Home:      return JI_KEY_HOME;
        case XK_End:       return JI_KEY_END;
        case XK_Page_Up:   return JI_KEY_PAGE_UP;
        case XK_Page_Down: return JI_KEY_PAGE_DOWN;
        case XK_Left:      return JI_KEY_LEFT;
        case XK_Right:     return JI_KEY_RIGHT;
        case XK_Up:        return JI_KEY_UP;
        case XK_Down:      return JI_KEY_DOWN;
        case XK_Shift_L:   return JI_KEY_SHIFT_L;
        case XK_Shift_R:   return JI_KEY_SHIFT_R;
        case XK_Control_L: return JI_KEY_CONTROL_L;
        case XK_Control_R: return JI_KEY_CONTROL_R;
        case XK_Alt_L:     return JI_KEY_ALT_L;
        case XK_Alt_R:     return JI_KEY_ALT_R;
        case XK_Super_L:   return JI_KEY_ALT_L;  /* map Super to Alt */
        case XK_Super_R:   return JI_KEY_ALT_R;
        case XK_F1:  return JI_KEY_F1;
        case XK_F2:  return JI_KEY_F2;
        case XK_F3:  return JI_KEY_F3;
        case XK_F4:  return JI_KEY_F4;
        case XK_F5:  return JI_KEY_F5;
        case XK_F6:  return JI_KEY_F6;
        case XK_F7:  return JI_KEY_F7;
        case XK_F8:  return JI_KEY_F8;
        case XK_F9:  return JI_KEY_F9;
        case XK_F10: return JI_KEY_F10;
        case XK_F11: return JI_KEY_F11;
        case XK_F12: return JI_KEY_F12;
        case XK_space: return JI_KEY_SPACE;
        default: return JI_KEY_UNKNOWN;
    }
}

static uint32_t x11_modifiers_to_ji(unsigned int x11_state) {
    uint32_t mods = 0;
    if (x11_state & ShiftMask)   mods |= JI_MOD_SHIFT;
    if (x11_state & ControlMask) mods |= JI_MOD_CONTROL;
    if (x11_state & Mod1Mask)    mods |= JI_MOD_ALT;
    if (x11_state & Mod4Mask)    mods |= JI_MOD_SUPER;
    return mods;
}

/* =========================================================================
 * JiPlatformBackend implementation
 * ========================================================================= */

static void* x11_create_window(JiPlatformBackend* backend, const char* title,
                                 int x, int y, int width, int height) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return NULL;

    XSetWindowAttributes swa;
    swa.border_pixel     = BlackPixel(data->display, data->screen);
    swa.colormap         = data->colormap;
    swa.event_mask       = ExposureMask | StructureNotifyMask |
                           KeyPressMask | KeyReleaseMask |
                           ButtonPressMask | ButtonReleaseMask |
                           PointerMotionMask | FocusChangeMask;

    /* No CWBackPixel — OpenGL handles all rendering, no X11 background paint */
    Window win = XCreateWindow(
        data->display, data->root,
        x, y, (unsigned int)width, (unsigned int)height, 0,
        data->visual_info->depth, InputOutput,
        data->visual_info->visual,
        CWBorderPixel | CWEventMask | CWColormap,
        &swa);

    if (!win) return NULL;

    /* Set WM_DELETE_WINDOW protocol */
    XSetWMProtocols(data->display, win, &data->wm_delete_atom, 1);

    /* Set window title */
    if (title) {
        XStoreName(data->display, win, title);
    }

    /* Map the window */
    XMapWindow(data->display, win);
    XFlush(data->display);

    return (void*)(uintptr_t)win;
}

static void x11_destroy_window(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display || !native_window) return;

    Window win = (Window)(uintptr_t)native_window;
    XDestroyWindow(data->display, win);
}

static void x11_show_window(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !native_window) return;
    XMapWindow(data->display, (Window)(uintptr_t)native_window);
}

static void x11_hide_window(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !native_window) return;
    XUnmapWindow(data->display, (Window)(uintptr_t)native_window);
}

static void x11_set_window_title(JiPlatformBackend* backend, void* native_window,
                                   const char* title) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !native_window) return;
    XStoreName(data->display, (Window)(uintptr_t)native_window, title ? title : "");
}

static void x11_get_window_size(JiPlatformBackend* backend, void* native_window,
                                  int* out_width, int* out_height) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !native_window) {
        if (out_width)  *out_width = 0;
        if (out_height) *out_height = 0;
        return;
    }

    Window win = (Window)(uintptr_t)native_window;
    XWindowAttributes attrs;
    XGetWindowAttributes(data->display, win, &attrs);
    if (out_width)  *out_width = attrs.width;
    if (out_height) *out_height = attrs.height;
}

static int x11_poll_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display || !out_events || max_events <= 0) return 0;

    int count = 0;

    while (count < max_events && XPending(data->display) > 0) {
        XEvent xev;
        XNextEvent(data->display, &xev);

        JiEvent* ev = &out_events[count];
        memset(ev, 0, sizeof(JiEvent));

        switch (xev.type) {
            case KeyPress: {
                KeySym keysym = XLookupKeysym(&xev.xkey, 0);
                ev->kind      = JI_EVENT_KEY_PRESS;
                ev->key       = x11_keycode_to_ji(keysym);
                ev->modifiers = x11_modifiers_to_ji(xev.xkey.state);
                count++;
                break;
            }
            case KeyRelease: {
                KeySym keysym = XLookupKeysym(&xev.xkey, 0);
                ev->kind      = JI_EVENT_KEY_RELEASE;
                ev->key       = x11_keycode_to_ji(keysym);
                ev->modifiers = x11_modifiers_to_ji(xev.xkey.state);
                count++;
                break;
            }
            case ButtonPress: {
                ev->kind    = JI_EVENT_MOUSE_PRESS;
                ev->mouse_x = xev.xbutton.x;
                ev->mouse_y = xev.xbutton.y;
                switch (xev.xbutton.button) {
                    case 1: ev->button = JI_MOUSE_LEFT;   break;
                    case 2: ev->button = JI_MOUSE_MIDDLE;  break;
                    case 3: ev->button = JI_MOUSE_RIGHT;   break;
                    case 4: ev->kind = JI_EVENT_MOUSE_WHEEL; ev->wheel_delta = 1.0; break;
                    case 5: ev->kind = JI_EVENT_MOUSE_WHEEL; ev->wheel_delta = -1.0; break;
                    default: ev->button = JI_MOUSE_NONE;   break;
                }
                ev->modifiers = x11_modifiers_to_ji(xev.xbutton.state);
                count++;
                break;
            }
            case ButtonRelease: {
                ev->kind    = JI_EVENT_MOUSE_RELEASE;
                ev->mouse_x = xev.xbutton.x;
                ev->mouse_y = xev.xbutton.y;
                switch (xev.xbutton.button) {
                    case 1: ev->button = JI_MOUSE_LEFT;   break;
                    case 2: ev->button = JI_MOUSE_MIDDLE;  break;
                    case 3: ev->button = JI_MOUSE_RIGHT;   break;
                    default: ev->button = JI_MOUSE_NONE;   break;
                }
                ev->modifiers = x11_modifiers_to_ji(xev.xbutton.state);
                count++;
                break;
            }
            case MotionNotify: {
                ev->kind    = JI_EVENT_MOUSE_MOVE;
                ev->mouse_x = xev.xmotion.x;
                ev->mouse_y = xev.xmotion.y;
                ev->modifiers = x11_modifiers_to_ji(xev.xmotion.state);
                count++;
                break;
            }
            case ConfigureNotify: {
                ev->kind   = JI_EVENT_RESIZE;
                ev->width  = xev.xconfigure.width;
                ev->height = xev.xconfigure.height;
                count++;
                break;
            }
            case FocusIn: {
                ev->kind = JI_EVENT_FOCUS_IN;
                count++;
                break;
            }
            case FocusOut: {
                ev->kind = JI_EVENT_FOCUS_OUT;
                count++;
                break;
            }
            case ClientMessage: {
                if ((Atom)xev.xclient.data.l[0] == data->wm_delete_atom) {
                    ev->kind = JI_EVENT_CLOSE;
                    count++;
                }
                break;
            }
            default:
                break;
        }
    }

    return count;
}

static int x11_wait_events(JiPlatformBackend* backend, JiEvent* out_events, int max_events) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return 0;

    /* Block until an event arrives */
    XEvent xev;
    XNextEvent(data->display, &xev);

    /* Put it back so poll_events can read it */
    XPutBackEvent(data->display, &xev);

    return x11_poll_events(backend, out_events, max_events);
}

static bool x11_should_close(JiPlatformBackend* backend, void* native_window) {
    (void)backend; (void)native_window;
    /* This is tracked by the JiWindow via the CLOSE event */
    return false;
}

static void x11_request_close(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display || !native_window) return;

    XClientMessageEvent msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = ClientMessage;
    msg.window = (Window)(uintptr_t)native_window;
    msg.message_type = data->wm_protocols;
    msg.format = 32;
    msg.data.l[0] = (long)data->wm_delete_atom;
    msg.data.l[1] = CurrentTime;

    XSendEvent(data->display, (Window)(uintptr_t)native_window, False, NoEventMask,
               (XEvent*)&msg);
    XFlush(data->display);
}

static JiGLContext* x11_create_gl_context(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return NULL;
    (void)native_window;

    /* The GLX context is created during backend init — just return it */
    return (JiGLContext*)data->glx_context;
}

static void x11_destroy_gl_context(JiPlatformBackend* backend, JiGLContext* ctx) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return;
    (void)ctx;

    if (data->glx_context) {
        if (data->context_current) {
            glXMakeCurrent(data->display, None, NULL);
            data->context_current = false;
        }
        glXDestroyContext(data->display, data->glx_context);
        data->glx_context = NULL;
    }
}

static bool x11_make_current(JiPlatformBackend* backend, void* native_window,
                               JiGLContext* ctx) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return false;

    Window win = native_window ? (Window)(uintptr_t)native_window : None;
    GLXContext glx_ctx = ctx ? (GLXContext)ctx : NULL;

    Bool result = glXMakeCurrent(data->display, win, glx_ctx);
    data->context_current = (result != False);
    data->current_window = win;
    return data->context_current;
}

static void x11_swap_buffers(JiPlatformBackend* backend, void* native_window) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display || !native_window) return;

    glXSwapBuffers(data->display, (Window)(uintptr_t)native_window);
}

static void x11_set_swap_interval(JiPlatformBackend* backend, int interval) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data || !data->display) return;

    /* Try EXT_swap_control (most common on Linux) */
    PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT =
        (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB(
            (const GLubyte*)"glXSwapIntervalEXT");

    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(data->display, data->current_window, interval);
        JI_DEBUG("Set swap interval to %d via EXT", interval);
        return;
    }

    /* Try SGI_swap_control */
    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI =
        (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddressARB(
            (const GLubyte*)"glXSwapIntervalSGI");

    if (glXSwapIntervalSGI) {
        glXSwapIntervalSGI(interval);
        JI_DEBUG("Set swap interval to %d via SGI", interval);
        return;
    }

    JI_WARN("No GLX swap interval extension available");
}

static JiDrawingContext* x11_get_drawing_context(JiPlatformBackend* backend) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (!data) return NULL;
    return &data->drawing_ctx;
}

static void x11_destroy_backend(JiPlatformBackend* backend) {
    if (!backend) return;

    JiX11Data* data = (JiX11Data*)backend->backend_data;
    if (data) {
        if (data->glx_context) {
            if (data->context_current) {
                glXMakeCurrent(data->display, None, NULL);
            }
            glXDestroyContext(data->display, data->glx_context);
        }
        if (data->visual_info) {
            XFree(data->visual_info);
        }
        if (data->display) {
            XCloseDisplay(data->display);
        }
        free(data);
    }

    ji_platform_unregister(backend);
    free(backend);
}

static const char* x11_get_name(JiPlatformBackend* backend) {
    (void)backend;
    return "X11";
}

static void* x11_get_native_display(JiPlatformBackend* backend) {
    JiX11Data* data = (JiX11Data*)backend->backend_data;
    return data ? (void*)data->display : NULL;
}

static void* x11_get_native_window(JiPlatformBackend* backend, void* native_window) {
    (void)backend;
    return native_window;
}

/* =========================================================================
 * Backend creation
 * ========================================================================= */

JiPlatformBackend* ji_x11_backend_create(void) {
    /* Open X11 display */
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        JI_WARN("Failed to open X11 display");
        return NULL;
    }

    /* Allocate backend and data */
    JiPlatformBackend* backend = (JiPlatformBackend*)calloc(1, sizeof(JiPlatformBackend));
    if (!backend) {
        XCloseDisplay(display);
        return NULL;
    }

    JiX11Data* data = (JiX11Data*)calloc(1, sizeof(JiX11Data));
    if (!data) {
        free(backend);
        XCloseDisplay(display);
        return NULL;
    }

    data->display = display;
    data->screen  = DefaultScreen(display);
    data->root    = RootWindow(display, data->screen);

    /* Choose FB config for OpenGL */
    static int fb_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        GLX_ALPHA_SIZE,   8,
        GLX_DEPTH_SIZE,   24,
        GLX_STENCIL_SIZE, 8,
        None
    };

    int fb_count = 0;
    GLXFBConfig* fb_configs = glXChooseFBConfig(display, data->screen,
                                                  fb_attribs, &fb_count);
    if (!fb_configs || fb_count == 0) {
        JI_WARN("Failed to choose GLX FB config");
        XCloseDisplay(display);
        free(data);
        free(backend);
        return NULL;
    }

    data->fb_config = fb_configs[0];

    /* Get visual info from FB config */
    data->visual_info = glXGetVisualFromFBConfig(display, data->fb_config);
    XFree(fb_configs);

    if (!data->visual_info) {
        JI_WARN("Failed to get visual from GLX FB config");
        XCloseDisplay(display);
        free(data);
        free(backend);
        return NULL;
    }

    /* Create GLX context */
    typedef GLXContext (*glXCreateContextAttribsARBProc)(
        Display*, GLXFBConfig, GLXContext, Bool, const int*);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)glXGetProcAddressARB(
            (const GLubyte*)"glXCreateContextAttribsARB");

    if (glXCreateContextAttribsARB) {
        /* Request OpenGL 3.3 compatibility profile (supports legacy + modern GL) */
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            None
        };

        data->glx_context = glXCreateContextAttribsARB(
            display, data->fb_config, NULL, True, context_attribs);

        if (!data->glx_context) {
            JI_WARN("Failed to create OpenGL 3.3 core context, falling back to legacy");
        }
    }

    /* Fallback to legacy context creation */
    if (!data->glx_context) {
        data->glx_context = glXCreateContext(display, data->visual_info, NULL, True);
    }

    if (!data->glx_context) {
        JI_WARN("Failed to create GLX context");
        XFree(data->visual_info);
        XCloseDisplay(display);
        free(data);
        free(backend);
        return NULL;
    }

    /* Create colormap for the GLX visual (required for non-default visuals) */
    data->colormap = XCreateColormap(display, data->root,
                                      data->visual_info->visual, AllocNone);

    /* Intern atoms for window management */
    data->wm_delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
    data->wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    data->net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    data->net_wm_state_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    /* Set up drawing context function pointers */
    ji_drawing_context_init(&data->drawing_ctx);
    data->drawing_ctx.push_clip      = x11_gl_push_clip;
    data->drawing_ctx.pop_clip       = x11_gl_pop_clip;
    data->drawing_ctx.push_transform = x11_gl_push_transform;
    data->drawing_ctx.pop_transform  = x11_gl_pop_transform;
    data->drawing_ctx.fill_geometry  = x11_gl_fill_geometry;
    data->drawing_ctx.stroke_geometry= x11_gl_stroke_geometry;
    data->drawing_ctx.draw_line      = x11_gl_draw_line;
    data->drawing_ctx.fill_rect      = x11_gl_fill_rect;
    data->drawing_ctx.stroke_rect    = x11_gl_stroke_rect;
    data->drawing_ctx.draw_text      = x11_gl_draw_text;
    data->drawing_ctx.clear          = x11_gl_clear;
    data->drawing_ctx.resize         = x11_gl_resize;
    data->drawing_ctx.present        = x11_gl_present;
    data->drawing_ctx.backend_data   = data;

    /* Fill in backend function pointers */
    backend->create_window      = x11_create_window;
    backend->destroy_window     = x11_destroy_window;
    backend->show_window        = x11_show_window;
    backend->hide_window        = x11_hide_window;
    backend->set_window_title   = x11_set_window_title;
    backend->get_window_size    = x11_get_window_size;
    backend->poll_events        = x11_poll_events;
    backend->wait_events        = x11_wait_events;
    backend->should_close       = x11_should_close;
    backend->request_close      = x11_request_close;
    backend->create_gl_context  = x11_create_gl_context;
    backend->destroy_gl_context = x11_destroy_gl_context;
    backend->make_current       = x11_make_current;
    backend->swap_buffers       = x11_swap_buffers;
    backend->set_swap_interval  = x11_set_swap_interval;
    backend->get_drawing_context= x11_get_drawing_context;
    backend->destroy_backend    = x11_destroy_backend;
    backend->get_name           = x11_get_name;
    backend->get_native_display = x11_get_native_display;
    backend->get_native_window  = x11_get_native_window;
    backend->backend_data       = data;

    /* Register as default backend */
    ji_platform_register(backend);

    JI_INFO("X11+GLX backend initialized (OpenGL %s)",
            glGetString(GL_VERSION) ? (const char*)glGetString(GL_VERSION) : "unknown");

    return backend;
}

#endif /* JIUI_ENABLE_X11 */
