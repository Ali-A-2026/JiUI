# ============================================================================
# JiUI - Declarative UI Framework for C/C++
# Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
#                    Hyder61112
# Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
# ============================================================================

cmake_minimum_required(VERSION 3.16)

project(JiUI
    VERSION 1.0.0
    DESCRIPTION "Declarative UI Framework for C/C++ with .ji file support"
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------------
# Project options
# ---------------------------------------------------------------------------
option(JIUI_BUILD_SHARED   "Build shared library (DLL/SO)"     ON)
option(JIUI_BUILD_STATIC   "Build static library"              ON)
option(JIUI_BUILD_EXAMPLES "Build example applications"        ON)
option(JIUI_BUILD_TESTS    "Build test suite"                  ON)
option(JIUI_ENABLE_OPENGL  "Enable OpenGL rendering backend"   ON)
option(JIUI_ENABLE_VULKAN  "Enable Vulkan rendering backend"   ON)
option(JIUI_ENABLE_X11     "Enable X11 platform backend"       ON)
option(JIUI_ENABLE_WAYLAND "Enable Wayland platform backend"   ON)

# ---------------------------------------------------------------------------
# Optional dependencies
# ---------------------------------------------------------------------------
find_package(ZLIB)

# ---------------------------------------------------------------------------
# C standard: C11 for maximum portability
# C++ standard: C++17 for bindings / higher-level API
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS on)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS on)

# ---------------------------------------------------------------------------
# Visibility & symbol export for dynamic linking API
# ---------------------------------------------------------------------------
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# ---------------------------------------------------------------------------
# Core library sources
# ---------------------------------------------------------------------------
set(JIUI_CORE_SOURCES
    src/core/ji_types.c
    src/core/ji_memory.c
    src/core/ji_error.c
    src/core/ji_type.c
    src/core/ji_property.c
    src/core/ji_binding.c
    src/core/ji_event.c
    src/core/ji_object.c
    src/parser/ji_lexer.c
    src/parser/ji_ast.c
    src/parser/ji_parser.c
    src/parser/ji_xml_lexer.c
    src/parser/ji_xml_parser.c
    src/parser/ji_tree_builder.c
    src/parser/ji_loader.c
    src/tools/jigen/jigen_codegen.c
    src/layout/ji_layout.c
    src/layout/ji_panel.c
    src/layout/ji_box_layout.c
    src/layout/ji_form_layout.c
    src/layout/ji_flow_layout.c
    src/visual/ji_visual.c
    src/visual/ji_drawing.c
    src/controls/ji_control.c
    src/controls/ji_widgets.c
    src/style/ji_style.c
    src/style/ji_palette.c
    src/style/ji_qt_style.c
    src/docking/ji_dock_widget.c
    src/docking/ji_dock_area.c
    src/docking/ji_dock_manager.c
    src/docking/ji_splitter.c
    src/docking/ji_dock_overlay.c
    src/docking/ji_dock_tab_bar.c
    src/docking/ji_dock_serializer.c
    src/controls/ji_group_box.c
    src/controls/ji_frame.c
    src/controls/ji_scroll_area.c
    src/controls/ji_tab_widget.c
    src/controls/ji_stacked_widget.c
    src/controls/ji_tool_bar.c
    src/controls/ji_status_bar.c
    src/controls/ji_menu.c
    src/controls/ji_radio_button.c
    src/controls/ji_spin_box.c
    src/controls/ji_double_spin_box.c
    src/controls/ji_combo_box.c
    src/controls/ji_text_edit.c
    src/controls/ji_plain_text_edit.c
    src/controls/ji_date_time_edit.c
    src/controls/ji_dial.c
    src/controls/ji_scroll_bar.c
    src/controls/ji_tool_button.c
    src/controls/ji_command_link_button.c
    src/controls/ji_model_view.c
    src/core/ji_animation.c
    src/core/ji_undo_stack.c
    src/visual/ji_effects.c
    src/style/ji_stylesheet.c
    src/platform/ji_platform.c
    src/platform/ji_window.c
    src/render/ji_gpu.c
    src/render/ji_gpu_software.c
    src/render/ji_scene.c
    src/render/ji_compositor.c
    src/render/ji_gpu_effects.c
    src/render/ji_frame_graph.c
    src/text/ji_font.c
    src/text/ji_text_engine.c
    src/text/ji_svg.c
    src/layout/ji_constraint.c
    src/input/ji_gesture.c
    src/input/ji_input.c
    src/input/ji_input_touch.c
    src/input/ji_input_pen.c
    src/input/ji_input_gamepad.c
    src/parser/ji_jiml_parser.c
    src/parser/ji_jiml_compiler.c
    src/core/ji_hot_reload.c
    src/core/ji_file_watcher.c
    src/docking/ji_dock_pro.c
    src/docking/ji_dock_auto_hide.c
    src/docking/ji_dock_multi_monitor.c
    src/3d/ji_3d.c
    src/3d/ji_3d_mesh.c
    src/3d/ji_3d_camera.c
    src/3d/ji_3d_light.c
    src/3d/ji_3d_gizmo.c
    src/3d/ji_3d_viewport.c
    src/render/ji_material.c
    src/render/ji_material_pbr.c
    src/render/ji_material_glass.c
    src/render/ji_material_neon.c
    src/render/ji_lighting.c
    src/core/ji_physics.c
    src/core/ji_neural.c
    src/core/ji_profiler.c
    src/core/ji_plugin.c
    src/core/ji_plugin_loader.c
    src/core/ji_automation.c
    src/core/ji_automation_record.c
    src/core/ji_hidpi.c
    src/core/ji_sandbox.c
    src/i18n/ji_i18n.c
    src/i18n/ji_i18n_plural.c
    src/i18n/ji_i18n_bidi.c
    src/a11y/ji_accessibility.c
    src/a11y/ji_a11y_atspi.c
    src/tools/ji_builder.c
    src/tools/ji_builder_widgets.c
    src/tools/ji_builder_codegen.c
    src/core/ji_threads.c
    src/multimedia/ji_video.c
    src/multimedia/ji_audio_viz.c
    src/multimedia/ji_timeline.c
    src/testing/ji_screenshot_test.c
)

# Platform-specific sources
if(JIUI_ENABLE_X11)
    set(JIUI_CORE_SOURCES ${JIUI_CORE_SOURCES}
        src/platform/ji_platform_x11.c
    )
endif()

if(JIUI_ENABLE_WAYLAND)
    set(JIUI_CORE_SOURCES ${JIUI_CORE_SOURCES}
        src/platform/ji_platform_wayland.c
    )
endif()

set(JIUI_CORE_HEADERS
    include/jiui/ji_config.h.in
    include/jiui/ji_defines.h
    include/jiui/ji_api.h
    include/jiui/ji_types.h
    include/jiui/ji_memory.h
    include/jiui/ji_error.h
    include/jiui/ji_type.h
    include/jiui/ji_property.h
    include/jiui/ji_binding.h
    include/jiui/ji_event.h
    include/jiui/ji_object.h
    include/jiui/ji_lexer.h
    include/jiui/ji_ast.h
    include/jiui/ji_parser.h
    include/jiui/ji_xml_lexer.h
    include/jiui/ji_xml_parser.h
    include/jiui/ji_tree_builder.h
    include/jiui/ji_loader.h
    include/jiui/ji_resource.h
    include/jiui/ji_codegen.h
    include/jiui/ji_layout.h
    include/jiui/ji_panel.h
    include/jiui/ji_visual.h
    include/jiui/ji_drawing.h
    include/jiui/ji_control.h
    include/jiui/ji_widgets.h
    include/jiui/ji_style.h
    include/jiui/ji_palette.h
    include/jiui/ji_qt_style.h
    include/jiui/ji_dock_widget.h
    include/jiui/ji_dock_area.h
    include/jiui/ji_dock_manager.h
    include/jiui/ji_splitter.h
    include/jiui/ji_dock_overlay.h
    include/jiui/ji_dock_tab_bar.h
    include/jiui/ji_dock_serializer.h
    include/jiui/ji_gpu.h
    include/jiui/ji_scene.h
    include/jiui/ji_group_box.h
    include/jiui/ji_frame.h
    include/jiui/ji_scroll_area.h
    include/jiui/ji_tab_widget.h
    include/jiui/ji_stacked_widget.h
    include/jiui/ji_tool_bar.h
    include/jiui/ji_status_bar.h
    include/jiui/ji_menu.h
    include/jiui/ji_radio_button.h
    include/jiui/ji_spin_box.h
    include/jiui/ji_double_spin_box.h
    include/jiui/ji_combo_box.h
    include/jiui/ji_text_edit.h
    include/jiui/ji_plain_text_edit.h
    include/jiui/ji_date_time_edit.h
    include/jiui/ji_dial.h
    include/jiui/ji_scroll_bar.h
    include/jiui/ji_tool_button.h
    include/jiui/ji_command_link_button.h
    include/jiui/ji_model_view.h
    include/jiui/ji_animation.h
    include/jiui/ji_undo_stack.h
    include/jiui/ji_effects.h
    include/jiui/ji_stylesheet.h
    include/jiui/ji_box_layout.h
    include/jiui/ji_form_layout.h
    include/jiui/ji_flow_layout.h
    include/jiui/ji_platform.h
    include/jiui/ji_window.h
    include/jiui/ji_material.h
    include/jiui/ji_physics.h
    include/jiui/ji_neural.h
    include/jiui/ji_compositor.h
    include/jiui/ji_gpu_effects.h
    include/jiui/ji_frame_graph.h
    include/jiui/ji_font.h
    include/jiui/ji_text_engine.h
    include/jiui/ji_svg.h
    include/jiui/ji_constraint.h
    include/jiui/ji_gesture.h
    include/jiui/ji_input.h
    include/jiui/ji_jiml.h
    include/jiui/ji_hot_reload.h
    include/jiui/ji_dock_pro.h
    include/jiui/ji_3d.h
    include/jiui/ji_profiler.h
    include/jiui/ji_plugin.h
    include/jiui/ji_automation.h
    include/jiui/ji_builder.h
    include/jiui/ji_i18n.h
    include/jiui/ji_accessibility.h
    include/jiui/ji_hidpi.h
    include/jiui/ji_sandbox.h
    include/jiui/ji_threads.h
    include/jiui/ji_multimedia.h
    include/jiui/ji_screenshot_test.h
    include/jiui/jiui.h
    include/jiui/jiui.hpp
)

# ---------------------------------------------------------------------------
# Generate config header (version info)
# ---------------------------------------------------------------------------
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/jiui/ji_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/jiui/ji_config.h
    @ONLY
)

# ---------------------------------------------------------------------------
# Shared / Static library targets
# ---------------------------------------------------------------------------
# Shared library
if(JIUI_BUILD_SHARED)
    add_library(jiui SHARED ${JIUI_CORE_SOURCES})
    target_include_directories(jiui
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(jiui
        PRIVATE JIUI_BUILDING_DLL
        PUBLIC  JIUI_SHARED
    )
    set_target_properties(jiui PROPERTIES
        VERSION   ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "jiui"
        PREFIX     ""
    )
    # Link math library on Linux
    if(UNIX)
        target_link_libraries(jiui PRIVATE m ${CMAKE_DL_LIBS})
    endif()

    # Platform-specific libraries
    if(JIUI_ENABLE_X11)
        find_package(X11 REQUIRED)
        target_link_libraries(jiui PRIVATE X11)
        target_compile_definitions(jiui PRIVATE JIUI_ENABLE_X11)
    endif()

    if(JIUI_ENABLE_WAYLAND)
        # JIUI_ENABLE_WAYLAND is already set via ji_config.h
    endif()

    if(JIUI_ENABLE_OPENGL)
        find_package(OpenGL REQUIRED)
        target_link_libraries(jiui PRIVATE OpenGL::GL)
        target_compile_definitions(jiui PRIVATE JIUI_ENABLE_OPENGL)
    endif()
endif()

# Static library
if(JIUI_BUILD_STATIC)
    add_library(jiui_static STATIC ${JIUI_CORE_SOURCES})
    target_include_directories(jiui_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(jiui_static
        PUBLIC JIUI_STATIC
    )
    set_target_properties(jiui_static PROPERTIES
        OUTPUT_NAME "jiui_static"
    )
    if(UNIX)
        target_link_libraries(jiui_static PRIVATE m ${CMAKE_DL_LIBS})
    endif()

    # Platform-specific libraries
    if(JIUI_ENABLE_X11)
        target_link_libraries(jiui_static PRIVATE X11)
        target_compile_definitions(jiui_static PRIVATE JIUI_ENABLE_X11)
    endif()

    if(JIUI_ENABLE_OPENGL)
        target_link_libraries(jiui_static PRIVATE OpenGL::GL)
        target_compile_definitions(jiui_static PRIVATE JIUI_ENABLE_OPENGL)
    endif()
endif()

# ---------------------------------------------------------------------------
# jigen — AOT code generator tool
# ---------------------------------------------------------------------------
option(JIUI_BUILD_JIGEN "Build the jigen code generator tool" ON)
if(JIUI_BUILD_JIGEN AND JIUI_BUILD_STATIC)
    add_executable(jigen
        src/tools/jigen/jigen.c
        src/tools/jigen/jigen_codegen.c
    )
    target_include_directories(jigen
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    )
    target_link_libraries(jigen jiui_static)
    if(UNIX)
        target_link_libraries(jigen m)
    endif()
elseif(JIUI_BUILD_JIGEN AND JIUI_BUILD_SHARED)
    add_executable(jigen
        src/tools/jigen/jigen.c
        src/tools/jigen/jigen_codegen.c
    )
    target_include_directories(jigen
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    )
    target_link_libraries(jigen jiui)
    if(UNIX)
        target_link_libraries(jigen m)
    endif()
endif()

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(JIUI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(JIUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(DIRECTORY include/jiui
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/jiui/ji_config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jiui
)

if(JIUI_BUILD_SHARED)
    install(TARGETS jiui
        EXPORT JiUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(JIUI_BUILD_STATIC)
    install(TARGETS jiui_static
        EXPORT JiUITargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

install(EXPORT JiUITargets
    FILE JiUITargets.cmake
    NAMESPACE JiUI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/JiUI
)

# ---------------------------------------------------------------------------
# pkg-config
# ---------------------------------------------------------------------------

# Platform-specific private libraries for pkg-config
if(UNIX AND NOT APPLE)
    set(JIUI_PC_PRIVATE_LIBS "-lX11 -lGL -lm")
elseif(APPLE)
    set(JIUI_PC_PRIVATE_LIBS "-framework OpenGL -framework AppKit")
else()
    set(JIUI_PC_PRIVATE_LIBS "")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/jiui.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/jiui.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/jiui.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
