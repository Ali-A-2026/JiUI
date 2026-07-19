/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file jiui.hpp
 * @brief C++17 RAII wrappers for the JiUI C API.
 *
 * Provides smart pointers, scoped resource managers, and convenience
 * wrappers that make the C API ergonomic in modern C++.
 */

#ifndef JIUI_HPP
#define JIUI_HPP

#include <jiui/jiui.h>

#include <memory>
#include <string>
#include <functional>
#include <stdexcept>
#include <cstring>

namespace jiui {

/* =========================================================================
 * Library lifetime
 * ========================================================================= */

/**
 * RAII wrapper for ji_initialize() / ji_shutdown().
 * Call once at application start — or let the constructor do it.
 */
class Library {
public:
    Library()  { ji_initialize(); }
    ~Library() { ji_shutdown(); }

    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    static const char* version() { return ji_version(); }
};

/* =========================================================================
 * Smart deleters for unique_ptr
 * ========================================================================= */

struct VisualDeleter {
    void operator()(JiVisual* p) const { ji_visual_destroy(p); }
};

struct WindowDeleter {
    void operator()(JiWindow* p) const { ji_window_destroy(p); }
};

struct ErrorDeleter {
    void operator()(JiError* p) const { ji_error_destroy(p); }
};

struct BackendDeleter {
    void operator()(JiPlatformBackend* p) const {
        if (p && p->destroy_backend) p->destroy_backend(p);
    }
};

/* =========================================================================
 * Smart pointer aliases
 * ========================================================================= */

using VisualPtr  = std::unique_ptr<JiVisual, VisualDeleter>;
using WindowPtr  = std::unique_ptr<JiWindow, WindowDeleter>;
using ErrorPtr   = std::unique_ptr<JiError, ErrorDeleter>;
using BackendPtr = std::unique_ptr<JiPlatformBackend, BackendDeleter>;

/* =========================================================================
 * Factory functions
 * ========================================================================= */

inline VisualPtr make_visual() {
    return VisualPtr(ji_visual_new());
}

inline WindowPtr make_window(const std::string& title, int width, int height,
                              JiWindowFlags flags = static_cast<JiWindowFlags>(JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE),
                              JiPlatformBackend* backend = nullptr) {
    return WindowPtr(ji_window_create(title.c_str(), width, height, flags, backend));
}

/* =========================================================================
 * Window wrapper
 * ========================================================================= */

class Window {
public:
    Window(const std::string& title, int width, int height,
           JiWindowFlags flags = static_cast<JiWindowFlags>(JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE),
           JiPlatformBackend* backend = nullptr)
        : m_window(ji_window_create(title.c_str(), width, height, flags, backend))
    {
        if (!m_window) throw std::runtime_error("Failed to create JiUI window");
    }

    ~Window() {
        if (m_window) ji_window_destroy(m_window);
    }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept : m_window(other.m_window) {
        other.m_window = nullptr;
    }

    Window& operator=(Window&& other) noexcept {
        if (this != &other) {
            if (m_window) ji_window_destroy(m_window);
            m_window = other.m_window;
            other.m_window = nullptr;
        }
        return *this;
    }

    /* ---- Properties ---- */

    void set_title(const std::string& title) {
        ji_window_set_title(m_window, title.c_str());
    }

    std::string title() const {
        const char* t = ji_window_get_title(m_window);
        return t ? std::string(t) : std::string();
    }

    void get_size(int& w, int& h) const {
        ji_window_get_size(m_window, &w, &h);
    }

    int width() const { int w, h; get_size(w, h); return w; }
    int height() const { int w, h; get_size(w, h); return h; }

    bool is_open() const { return ji_window_is_open(m_window); }
    void close() { ji_window_close(m_window); }

    double fps() const { return ji_window_get_fps(m_window); }
    uint64_t frame_count() const { return ji_window_get_frame_count(m_window); }

    /* ---- Callbacks ---- */

    void set_render_callback(JiWindowRenderFunc callback, void* user_data = nullptr) {
        ji_window_set_render_callback(m_window, callback, user_data);
    }

    void set_event_callback(JiWindowEventFunc callback, void* user_data = nullptr) {
        ji_window_set_event_callback(m_window, callback, user_data);
    }

    void set_close_callback(JiWindowCloseFunc callback, void* user_data = nullptr) {
        ji_window_set_close_callback(m_window, callback, user_data);
    }

    /* ---- Main loop ---- */

    void run() { ji_window_run(m_window); }
    bool frame() { return ji_window_frame(m_window); }

    /* ---- Raw access ---- */

    JiWindow* native() const { return m_window; }
    JiDrawingContext* drawing_context() const { return ji_window_get_drawing_context(m_window); }

private:
    JiWindow* m_window;
};

/* =========================================================================
 * Drawing helpers
 * ========================================================================= */

inline JiBrush solid_brush(uint32_t argb) {
    return ji_brush_solid(argb);
}

inline JiPen solid_pen(uint32_t argb, double thickness) {
    return ji_pen_solid(argb, thickness);
}

inline JiGeometry rect_geometry(JiRect rect) {
    return ji_geometry_rect(rect);
}

inline JiGeometry ellipse_geometry(JiRect bounds) {
    return ji_geometry_ellipse(bounds);
}

/* =========================================================================
 * Error checking helper
 * ========================================================================= */

inline void check_result(JiResultCode code, const char* context = nullptr) {
    if (code != JI_OK) {
        std::string msg = "JiUI error: ";
        msg += ji_result_to_string(code);
        if (context) { msg += " ("; msg += context; msg += ")"; }
        throw std::runtime_error(msg);
    }
}

/* =========================================================================
 * Theme helpers
 * ========================================================================= */

inline void set_theme(JiTheme* theme) { ji_set_current_theme(theme); }
inline JiTheme* get_theme() { return ji_get_current_theme(); }

/* =========================================================================
 * Event helpers
 * ========================================================================= */

/**
 * Register a named event handler that can be referenced from XML.
 * Usage: jiui::register_handler("on_click", my_callback);
 */
inline void register_handler(const std::string& name, JiEventHandler handler) {
    ji_event_register_handler(name.c_str(), handler);
}

/* =========================================================================
 * Loader — RAII wrapper for JiLoadResult
 * ========================================================================= */

/**
 * RAII wrapper for loading .ji XML files.
 * Automatically destroys the load result on scope exit.
 */
class LoadResult {
public:
    /** Load from a .ji file. */
    static LoadResult from_file(const std::string& path) {
        return LoadResult(ji_load_file(path.c_str()));
    }

    /** Load from an XML string. */
    static LoadResult from_string(const std::string& source) {
        return LoadResult(ji_load_string(source.c_str()));
    }

    ~LoadResult() { ji_load_result_destroy(&m_result); }

    // Move-only
    LoadResult(LoadResult&& o) noexcept : m_result(o.m_result) {
        memset(&o.m_result, 0, sizeof(o.m_result));
    }
    LoadResult& operator=(LoadResult&& o) noexcept {
        if (this != &o) {
            ji_load_result_destroy(&m_result);
            m_result = o.m_result;
            memset(&o.m_result, 0, sizeof(o.m_result));
        }
        return *this;
    }
    LoadResult(const LoadResult&) = delete;
    LoadResult& operator=(const LoadResult&) = delete;

    /** Check if loading succeeded. */
    bool ok() const { return !m_result.has_error; }

    /** Get error message (empty string if no error). */
    std::string error() const {
        return m_result.has_error ? m_result.error_msg : std::string();
    }

    /** Get the root object (do NOT release — lifetime managed by LoadResult). */
    JiObject* root() const { return m_result.root; }

    /** Find a named element in the loaded tree. */
    JiObject* find(const std::string& name) const {
        return ji_load_find_name(m_result.root, name.c_str());
    }

    /** Get the binding engine. */
    JiBindingEngine* bindings() { return &m_result.bindings; }

    /** Get the event bus. */
    JiEventBus* events() { return &m_result.events; }

    /** Get the resource dictionary. */
    JiResourceDictionary* resources() const { return m_result.resources; }

    /** Get the underlying C result. */
    const JiLoadResult& c_result() const { return m_result; }

private:
    explicit LoadResult(JiLoadResult result) : m_result(result) {}
    JiLoadResult m_result;
};

/* =========================================================================
 * Platform helpers
 * ========================================================================= */

#ifdef JIUI_ENABLE_X11
inline BackendPtr create_x11_backend() {
    return BackendPtr(ji_x11_backend_create());
}
#endif

inline JiPlatformBackend* default_backend() {
    return ji_platform_get_default();
}

/* =========================================================================
 * v1.0.0 — RAII wrappers for new subsystems
 * ========================================================================= */

/* ---- Constraint Layout (Cassowary) ---- */

struct ConstraintSolverDeleter {
    void operator()(JiConstraintSolver* p) const { ji_constraint_solver_destroy(p); }
};
using ConstraintSolverPtr = std::unique_ptr<JiConstraintSolver, ConstraintSolverDeleter>;

inline ConstraintSolverPtr make_constraint_solver() {
    return ConstraintSolverPtr(ji_constraint_solver_new());
}

/* ---- Gesture Manager ---- */

struct GestureManagerDeleter {
    void operator()(JiGestureManager* p) const { ji_gesture_manager_destroy(p); }
};
using GestureManagerPtr = std::unique_ptr<JiGestureManager, GestureManagerDeleter>;

inline GestureManagerPtr make_gesture_manager() {
    return GestureManagerPtr(ji_gesture_manager_new());
}

/* ---- Input Manager ---- */

struct InputManagerDeleter {
    void operator()(JiInputManager* p) const { ji_input_manager_destroy(p); }
};
using InputManagerPtr = std::unique_ptr<JiInputManager, InputManagerDeleter>;

inline InputManagerPtr make_input_manager() {
    return InputManagerPtr(ji_input_manager_new());
}

/* ---- Physics (Spring) ---- */

struct PhysicsWorldDeleter {
    void operator()(JiPhysicsWorld* p) const { ji_physics_world_destroy(p); }
};
using PhysicsWorldPtr = std::unique_ptr<JiPhysicsWorld, PhysicsWorldDeleter>;

inline PhysicsWorldPtr make_physics_world() {
    return PhysicsWorldPtr(ji_physics_world_new());
}

/* ---- Profiler ---- */

struct ProfilerDeleter {
    void operator()(JiProfiler* p) const { ji_profiler_free(p); }
};
using ProfilerPtr = std::unique_ptr<JiProfiler, ProfilerDeleter>;

inline ProfilerPtr make_profiler() {
    return ProfilerPtr(ji_profiler_new());
}

/* ---- i18n ---- */

struct I18nEngineDeleter {
    void operator()(JiI18nEngine* p) const { ji_i18n_free(p); }
};
using I18nEnginePtr = std::unique_ptr<JiI18nEngine, I18nEngineDeleter>;

inline I18nEnginePtr make_i18n_engine() {
    return I18nEnginePtr(ji_i18n_new());
}

inline std::string tr(JiI18nEngine* engine, const std::string& key) {
    const char* r = ji_i18n_translate(engine, key.c_str());
    return r ? std::string(r) : key;
}

/* ---- HiDPI ---- */

struct HidpiManagerDeleter {
    void operator()(JiHidpiManager* p) const { ji_hidpi_free(p); }
};
using HidpiManagerPtr = std::unique_ptr<JiHidpiManager, HidpiManagerDeleter>;

inline HidpiManagerPtr make_hidpi_manager() {
    return HidpiManagerPtr(ji_hidpi_new());
}

/* ---- Hot Reload ---- */

struct HotReloadEngineDeleter {
    void operator()(JiHotReloadEngine* p) const { ji_hot_reload_destroy(p); }
};
using HotReloadEnginePtr = std::unique_ptr<JiHotReloadEngine, HotReloadEngineDeleter>;

inline HotReloadEnginePtr make_hot_reload_engine() {
    return HotReloadEnginePtr(ji_hot_reload_new());
}

/* ---- Screenshot Testing ---- */

struct ScreenshotImageDeleter {
    void operator()(JiScreenshotImage* p) const { ji_ss_image_free(p); }
};
using ScreenshotImagePtr = std::unique_ptr<JiScreenshotImage, ScreenshotImageDeleter>;

inline ScreenshotImagePtr make_screenshot_image(uint32_t w, uint32_t h) {
    return ScreenshotImagePtr(ji_ss_image_new(w, h));
}

inline bool screenshot_save_png(const JiScreenshotImage* img, const std::string& path) {
    return ji_ss_image_save_png(img, path.c_str()) == 0;
}

inline bool screenshots_equal(const JiScreenshotImage* a, const JiScreenshotImage* b) {
    return ji_ss_images_equal(a, b);
}

/* ---- Automation ---- */

struct AutomationDeleter {
    void operator()(JiAutomation* p) const { ji_automation_free(p); }
};
using AutomationPtr = std::unique_ptr<JiAutomation, AutomationDeleter>;

inline AutomationPtr make_automation() {
    return AutomationPtr(ji_automation_new());
}

/* ---- Threads ---- */

struct ThreadManagerDeleter {
    void operator()(JiThreadManager* p) const { ji_thread_manager_free(p); }
};
using ThreadManagerPtr = std::unique_ptr<JiThreadManager, ThreadManagerDeleter>;

inline ThreadManagerPtr make_thread_manager() {
    return ThreadManagerPtr(ji_thread_manager_new());
}

/* ---- Multimedia (Video) ---- */

struct VideoWidgetDeleter {
    void operator()(JiVideoWidget* p) const { ji_video_free(p); }
};
using VideoWidgetPtr = std::unique_ptr<JiVideoWidget, VideoWidgetDeleter>;

inline VideoWidgetPtr make_video_widget() {
    return VideoWidgetPtr(ji_video_new());
}

/* ---- Neural Network ---- */

struct NeuralModelDeleter {
    void operator()(JiNeuralModel* p) const { ji_neural_destroy(p); }
};
using NeuralModelPtr = std::unique_ptr<JiNeuralModel, NeuralModelDeleter>;

inline NeuralModelPtr make_neural_model() {
    return NeuralModelPtr(ji_neural_new());
}

/* ---- 3D Viewport ---- */

struct Viewport3DDeleter {
    void operator()(Ji3DViewport* p) const { ji_3d_viewport_destroy(p); }
};
using Viewport3DPtr = std::unique_ptr<Ji3DViewport, Viewport3DDeleter>;

inline Viewport3DPtr make_3d_viewport() {
    return Viewport3DPtr(ji_3d_viewport_new());
}

/* ---- Plugin Manager ---- */

struct PluginManagerDeleter {
    void operator()(JiPluginManager* p) const { ji_plugin_manager_free(p); }
};
using PluginManagerPtr = std::unique_ptr<JiPluginManager, PluginManagerDeleter>;

inline PluginManagerPtr make_plugin_manager() {
    return PluginManagerPtr(ji_plugin_manager_new());
}

/* ---- Sandbox ---- */

struct SandboxDeleter {
    void operator()(JiSandboxManager* p) const { ji_sandbox_free(p); }
};
using SandboxPtr = std::unique_ptr<JiSandboxManager, SandboxDeleter>;

inline SandboxPtr make_sandbox() {
    return SandboxPtr(ji_sandbox_new());
}

/* ---- Builder ---- */

struct BuilderDeleter {
    void operator()(JiBuilder* p) const { ji_builder_free(p); }
};
using BuilderPtr = std::unique_ptr<JiBuilder, BuilderDeleter>;

inline BuilderPtr make_builder() {
    return BuilderPtr(ji_builder_new());
}

} /* namespace jiui */

#endif /* JIUI_HPP */
