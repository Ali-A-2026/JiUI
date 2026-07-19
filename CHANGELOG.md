# Changelog

All notable changes to JiUI will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-07-18

### Added — Massive Phased Upgrade (27 features across 10 phases)

#### Phase 1: Rendering Engine Foundation
- **GPU Backend Abstraction** (`ji_gpu.h`) — Vulkan, OpenGL, Metal, D3D11, D3D12, WebGPU, Software backends
- **Scene Graph** (`ji_scene.h`) — Render passes, framebuffers, pipelines, command buffers
- **Software Renderer** (`ji_gpu_software.c`) — CPU fallback rendering backend

#### Phase 2: Advanced Layout & Graphics
- **Constraint Layout** (`ji_constraint.h`) — Auto-layout constraint solver
- **GPU Effects** (`ji_gpu_effects.h`) — GPU-accelerated visual effects
- **Frame Graph** (`ji_frame_graph.h`) — Render pipeline control
- **Compositor** (`ji_compositor.h`) — Layer composition engine

#### Phase 3: 3D Integration
- **3D Viewport** (`ji_3d.h`) — 3D mesh, camera, light, gizmo, viewport
- **PBR Materials** (`ji_material.h`) — Physically-based rendering materials
- **Glass & Neon Materials** — Advanced material effects
- **Dynamic Lighting** (`ji_lighting.c`) — Multi-light scene lighting

#### Phase 4: Physics & Neural
- **Spring Physics** (`ji_physics.h`) — Stiffness/damping/mass-based physics solver
- **Neural Network Inference** (`ji_neural.h`) — Built-in feedforward neural engine

#### Phase 5: Profiling & Plugins
- **Profiler** (`ji_profiler.h`) — Frame-level performance profiling
- **Plugin System** (`ji_plugin.h`) — Dynamic plugin loading (`.so` / `.dll`)
- **Plugin Loader** (`ji_plugin_loader.c`) — Plugin lifecycle management

#### Phase 6: Automation & HiDPI
- **Automation** (`ji_automation.h`) — Record and replay UI interactions
- **Automation Record** (`ji_automation_record.c`) — Session recording
- **HiDPI Support** (`ji_hidpi.h`) — Per-monitor DPI awareness
- **Sandboxed Execution** (`ji_sandbox.h`) — Restricted execution environment

#### Phase 7: Internationalization & Accessibility
- **i18n System** (`ji_i18n.h`) — String translation, plural rules (CLDR-based), BiDi text
- **Accessibility** (`ji_accessibility.h`) — Screen reader support, role/label/state management
- **AT-SPI Bridge** (`ji_a11y_atspi.c`) — Linux accessibility bridge

#### Phase 8: Input & Hot Reload
- **Gesture Recognition** (`ji_gesture.h`) — Swipe, pinch, pan, tap
- **Input System** (`ji_input.h`) — Touch, pen, gamepad support
- **Hot Reload** (`ji_hot_reload.h`) — Live code/style reload
- **File Watcher** (`ji_file_watcher.c`) — Filesystem change monitoring

#### Phase 9: Advanced Docking
- **DockPro** (`ji_dock_pro.h`) — Auto-hide, multi-monitor, advanced tabbing
- **Dock Auto-Hide** (`ji_dock_auto_hide.c`) — Auto-hide dock panels
- **Dock Multi-Monitor** (`ji_dock_multi_monitor.c`) — Multi-monitor docking

#### Phase 10: Threading, Multimedia & Testing
- **Multi-threaded Pipeline** (`ji_threads.h`) — 4-thread architecture (UI, Render, Resource, Asset) with lock-free SPSC queues
- **Multimedia** (`ji_multimedia.h`) — Video widget, audio visualization, timeline controls
- **Screenshot Testing** (`ji_screenshot_test.h`) — Visual regression testing with built-in PNG I/O

#### Additional Systems
- **Font Engine** (`ji_font.h`) — Custom font loading and glyph rendering
- **Text Engine** (`ji_text_engine.h`) — Text layout and rendering
- **SVG Support** (`ji_svg.h`) — Built-in SVG parser and renderer
- **JIML Compiler** (`ji_jiml.h`) — JiUI Markup Language → C code generation
- **Builder System** (`ji_builder.h`) — Fluent UI builder API

### Changed
- Version bumped from 0.2.0 to 1.0.0
- **Zero external dependencies** — Removed zlib dependency; implemented built-in DEFLATE stored-block encoder/decoder and CRC32 for PNG I/O
- Fixed symbol visibility — all public API functions now tagged with `JI_API` for proper shared library export
- Fixed config header redefinition warnings — added `#ifndef` guards around `#cmakedefine` directives
- Updated CI workflow — added no-platform-backend build job, removed zlib from dependency list
- README.md rewritten with comprehensive v1.0.0 feature documentation
- 46 tests, 100% passing

### Removed
- External zlib dependency (replaced with built-in `ji_zlib_compat.h`)
- `JISPEC.md` and `JISPEC_XML.md` (consolidated into README)

## [0.2.0] - 2026-07-16

### Added
- **Animation Framework** — 40+ easing curves, spring physics, property animation, animation groups, driver
- **Undo/Redo Framework** — Command pattern with merging, groups, clean state tracking
- **Model/View Architecture** — AbstractItemModel, StringListModel, ListView, TreeView, TableView, virtual scrolling
- **Graphics Effects** — Blur, DropShadow, Opacity, Colorize, Glow, Grayscale, EffectChain
- **Docking System** — DockManager, DockArea, DockWidget, DockOverlay, DockTabBar, DockSerializer, Splitter
- **20+ Widgets** — GroupBox, Frame, ScrollArea, TabWidget, StackedWidget, ToolBar, StatusBar, Menu, RadioButton, SpinBox, DoubleSpinBox, ComboBox, TextEdit, PlainTextEdit, DateTimeEdit, Dial, ScrollBar, ToolButton, CommandLinkButton
- **Layout System** — BoxLayout, FormLayout, FlowLayout
- **Theme System** — Palette (50+ colors), Qt Fusion style renderer
- **CI/CD** — GitHub Actions for Linux, macOS, Windows + coverage
- **Community** — CONTRIBUTING.md, CODE_OF_CONDUCT.md, issue/PR templates

### Changed
- Version bumped from 0.1.0 to 0.2.0
- README.md rewritten with comprehensive feature documentation

## [0.1.0] - 2026-01-15

### Added
- Initial release with core framework
- Memory management, error handling, type system
- Property system with bindings
- Object system with reference counting
- Ji markup parser, XML parser, AST
- Tree builder and code generator (jigen)
- Resource system, event system
- Basic widgets: Button, CheckBox, Label, ProgressBar, Slider
- Visual tree and drawing primitives
- X11 platform backend
- Window management
