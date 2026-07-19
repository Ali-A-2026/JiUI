/**
 * JiUI - Unity Engine-like Dockable Layout Example
 *
 * Demonstrates a Unity Engine-inspired interface with:
 *   - Menu bar with dropdown menus, separators, and checkable items
 *   - Toolbar with tool buttons and separators
 *   - Dockable panels (Hierarchy, Scene, Game, Inspector, Console/Project)
 *   - Tab widgets for tabbed panels
 *   - Checkboxes for component toggling in the Inspector
 *   - Combo boxes for dropdown selections
 *   - Status bar at the bottom
 *   - Splitter-based resizable layout
 *
 * Build: Requires JiUI with OpenGL backend enabled.
 * Run:   ./unity_layout_demo
 * Quit:  Press ESC or close the window.
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Global state
 * ========================================================================= */

static bool g_running = true;

/* Panel visibility flags (toggled by View menu checkboxes) */
static bool g_show_scene     = true;
static bool g_show_game      = true;
static bool g_show_inspector = true;
static bool g_show_hierarchy = true;
static bool g_show_console   = true;

/* =========================================================================
 * Helper: Create the File menu
 * ========================================================================= */
static JiMenu* create_file_menu(void) {
    JiMenu* menu = ji_menu_new("File");

    int idx;

    idx = ji_menu_add_action(menu, "New Scene");
    ji_menu_set_shortcut(menu, idx, "Ctrl+N");

    idx = ji_menu_add_action(menu, "Open Scene");
    ji_menu_set_shortcut(menu, idx, "Ctrl+O");

    idx = ji_menu_add_action(menu, "Save");
    ji_menu_set_shortcut(menu, idx, "Ctrl+S");

    /* Separator between Save and Exit */
    ji_menu_add_separator(menu);

    idx = ji_menu_add_action(menu, "Exit");
    ji_menu_set_shortcut(menu, idx, "Ctrl+Q");

    return menu;
}

/* =========================================================================
 * Helper: Create the Edit menu
 * ========================================================================= */
static JiMenu* create_edit_menu(void) {
    JiMenu* menu = ji_menu_new("Edit");

    int idx;

    idx = ji_menu_add_action(menu, "Undo");
    ji_menu_set_shortcut(menu, idx, "Ctrl+Z");

    idx = ji_menu_add_action(menu, "Redo");
    ji_menu_set_shortcut(menu, idx, "Ctrl+Shift+Z");

    ji_menu_add_separator(menu);

    idx = ji_menu_add_action(menu, "Cut");
    ji_menu_set_shortcut(menu, idx, "Ctrl+X");

    idx = ji_menu_add_action(menu, "Copy");
    ji_menu_set_shortcut(menu, idx, "Ctrl+C");

    idx = ji_menu_add_action(menu, "Paste");
    ji_menu_set_shortcut(menu, idx, "Ctrl+V");

    ji_menu_add_separator(menu);

    idx = ji_menu_add_action(menu, "Preferences");

    return menu;
}

/* =========================================================================
 * Helper: Create the View menu (with checkable panel visibility items)
 * ========================================================================= */
static JiMenu* create_view_menu(void) {
    JiMenu* menu = ji_menu_new("View");

    int idx;

    /* Checkable items for showing/hiding panels */
    idx = ji_menu_add_action(menu, "Scene");
    ji_menu_set_checkable(menu, idx, true);
    ji_menu_set_checked(menu, idx, g_show_scene);

    idx = ji_menu_add_action(menu, "Game");
    ji_menu_set_checkable(menu, idx, true);
    ji_menu_set_checked(menu, idx, g_show_game);

    idx = ji_menu_add_action(menu, "Inspector");
    ji_menu_set_checkable(menu, idx, true);
    ji_menu_set_checked(menu, idx, g_show_inspector);

    idx = ji_menu_add_action(menu, "Hierarchy");
    ji_menu_set_checkable(menu, idx, true);
    ji_menu_set_checked(menu, idx, g_show_hierarchy);

    idx = ji_menu_add_action(menu, "Console");
    ji_menu_set_checkable(menu, idx, true);
    ji_menu_set_checked(menu, idx, g_show_console);

    return menu;
}

/* =========================================================================
 * Helper: Create the Window menu (layout presets + panel list)
 * ========================================================================= */
static JiMenu* create_window_menu(void) {
    JiMenu* menu = ji_menu_new("Window");

    int idx;

    /* Layout preset submenu */
    JiMenu* layout_menu = ji_menu_new("Layouts");
    ji_menu_add_action(layout_menu, "Default");
    ji_menu_add_action(layout_menu, "2 by 3");
    ji_menu_add_action(layout_menu, "4 Split");
    ji_menu_add_submenu(menu, layout_menu);

    ji_menu_add_separator(menu);

    /* Panel list — same as View but without checkmarks */
    idx = ji_menu_add_action(menu, "Scene View");
    idx = ji_menu_add_action(menu, "Game View");
    idx = ji_menu_add_action(menu, "Inspector");
    idx = ji_menu_add_action(menu, "Hierarchy");
    idx = ji_menu_add_action(menu, "Console");

    return menu;
}

/* =========================================================================
 * Helper: Create the Help menu
 * ========================================================================= */
static JiMenu* create_help_menu(void) {
    JiMenu* menu = ji_menu_new("Help");

    ji_menu_add_action(menu, "About");

    return menu;
}

/* =========================================================================
 * Helper: Create the menu bar (array of top-level menus)
 * ========================================================================= */
static void create_menu_bar(JiMenu** menus, int* count) {
    *count = 5;
    menus[0] = create_file_menu();
    menus[1] = create_edit_menu();
    menus[2] = create_view_menu();
    menus[3] = create_window_menu();
    menus[4] = create_help_menu();
}

/* =========================================================================
 * Helper: Create the toolbar with tool buttons and separators
 * ========================================================================= */
static JiToolBar* create_toolbar(void) {
    JiToolBar* toolbar = ji_tool_bar_new();
    if (!toolbar) {
        fprintf(stderr, "Warning: Failed to create toolbar\n");
        return NULL;
    }

    ji_tool_bar_set_area(toolbar, JI_TOOL_BAR_TOP);
    ji_tool_bar_set_movable(toolbar, false);

    /* Transform tools */
    ji_tool_bar_add_action(toolbar, "move", "Move");
    ji_tool_bar_add_action(toolbar, "rotate", "Rotate");
    ji_tool_bar_add_action(toolbar, "scale", "Scale");

    /* Separator between transform and play controls */
    ji_tool_bar_add_separator(toolbar);

    /* Play controls */
    ji_tool_bar_add_action(toolbar, "play", "Play");
    ji_tool_bar_add_action(toolbar, "pause", "Pause");
    ji_tool_bar_add_action(toolbar, "step", "Step");

    /* Separator between play and layer controls */
    ji_tool_bar_add_separator(toolbar);

    /* Layer visibility */
    ji_tool_bar_add_action(toolbar, "layers", "Layers");

    /* Separator before layout selector */
    ji_tool_bar_add_separator(toolbar);

    /* Layout dropdown as a combo box widget */
    JiComboBox* layout_combo = ji_combo_box_new();
    if (layout_combo) {
        ji_combo_box_add_item(layout_combo, "Default");
        ji_combo_box_add_item(layout_combo, "2 by 3");
        ji_combo_box_add_item(layout_combo, "4 Split");
        ji_combo_box_set_current_index(layout_combo, 0);
        ji_tool_bar_add_widget(toolbar, &layout_combo->control);
    }

    return toolbar;
}

/* =========================================================================
 * Helper: Create the Hierarchy panel content
 *
 * The Hierarchy panel shows a tree-like list of game objects in the scene.
 * We use labels to represent items in the hierarchy.
 * ========================================================================= */
static JiControl* create_hierarchy_content(void) {
    /* Create a vertical list of labels representing scene objects */
    JiLabel* root_label = ji_label_new("Main Camera");
    if (!root_label) return NULL;

    /* Additional hierarchy items as child labels */
    JiLabel* child_labels[6];
    const char* names[] = {
        "Directional Light",
        "Canvas",
        "EventSystem",
        "Player",
        "Enemy",
        "Environment"
    };

    for (int i = 0; i < 6; i++) {
        child_labels[i] = ji_label_new(names[i]);
        if (child_labels[i]) {
            /* Add as visual children of the root label for tree structure */
            ji_visual_add_child(&root_label->control.visual,
                                &child_labels[i]->control.visual);
        }
    }

    return &root_label->control;
}

/* =========================================================================
 * Helper: Create the Scene View panel content
 *
 * The Scene View is the main 3D viewport. In this example we use a
 * placeholder label since actual 3D rendering requires the 3D subsystem.
 * ========================================================================= */
static JiControl* create_scene_view_content(void) {
    JiLabel* label = ji_label_new("Scene View - 3D Viewport");
    if (label) {
        ji_label_set_font_size(label, 14.0);
    }
    return label ? &label->control : NULL;
}

/* =========================================================================
 * Helper: Create the Game View panel content
 *
 * The Game View shows the running game preview.
 * ========================================================================= */
static JiControl* create_game_view_content(void) {
    JiLabel* label = ji_label_new("Game View - Preview");
    if (label) {
        ji_label_set_font_size(label, 14.0);
    }
    return label ? &label->control : NULL;
}

/* =========================================================================
 * Helper: Create the Inspector panel content
 *
 * The Inspector shows properties of the selected object with checkboxes
 * for enabling/disabling components (Transform, Mesh Renderer, etc.).
 * ========================================================================= */
static JiControl* create_inspector_content(void) {
    /* Title label for the Inspector */
    JiLabel* title = ji_label_new("Inspector - Player Properties");
    if (!title) return NULL;

    /* Component checkboxes — these represent Unity-style component toggles */
    JiCheckBox* checkboxes[5];
    const char* component_names[] = {
        "Transform",
        "Mesh Renderer",
        "Collider",
        "Rigidbody",
        "Audio Source"
    };
    bool default_checked[] = {
        true,   /* Transform — always on */
        true,   /* Mesh Renderer — on by default */
        false,  /* Collider — off by default */
        false,  /* Rigidbody — off by default */
        false   /* Audio Source — off by default */
    };

    for (int i = 0; i < 5; i++) {
        checkboxes[i] = ji_check_box_new(component_names[i]);
        if (checkboxes[i]) {
            ji_check_box_set_checked(checkboxes[i], default_checked[i]);
            /* Add each checkbox as a visual child of the title */
            ji_visual_add_child(&title->control.visual,
                                &checkboxes[i]->control.visual);
        }
    }

    return &title->control;
}

/* =========================================================================
 * Helper: Create the Console panel content
 *
 * The Console shows log messages and errors.
 * ========================================================================= */
static JiControl* create_console_content(void) {
    JiLabel* label = ji_label_new("Console - Log Messages");
    if (label) {
        ji_label_set_font_size(label, 12.0);
    }
    return label ? &label->control : NULL;
}

/* =========================================================================
 * Helper: Create the Project panel content
 *
 * The Project panel shows the asset browser.
 * ========================================================================= */
static JiControl* create_project_content(void) {
    JiLabel* label = ji_label_new("Project - Assets Browser");
    if (label) {
        ji_label_set_font_size(label, 12.0);
    }
    return label ? &label->control : NULL;
}

/* =========================================================================
 * Helper: Create the center tab widget (Scene + Game tabs)
 * ========================================================================= */
static JiTabWidget* create_center_tabs(void) {
    JiTabWidget* tabs = ji_tab_widget_new();
    if (!tabs) return NULL;

    /* Scene tab */
    JiControl* scene_content = create_scene_view_content();
    if (scene_content) {
        ji_tab_widget_add_tab(tabs, "Scene", scene_content);
    }

    /* Game tab */
    JiControl* game_content = create_game_view_content();
    if (game_content) {
        ji_tab_widget_add_tab(tabs, "Game", game_content);
    }

    /* Start on the Scene tab */
    ji_tab_widget_set_current_index(tabs, 0);

    return tabs;
}

/* =========================================================================
 * Helper: Create the bottom tab widget (Console + Project tabs)
 * ========================================================================= */
static JiTabWidget* create_bottom_tabs(void) {
    JiTabWidget* tabs = ji_tab_widget_new();
    if (!tabs) return NULL;

    /* Console tab */
    JiControl* console_content = create_console_content();
    if (console_content) {
        ji_tab_widget_add_tab(tabs, "Console", console_content);
    }

    /* Project tab */
    JiControl* project_content = create_project_content();
    if (project_content) {
        ji_tab_widget_add_tab(tabs, "Project", project_content);
    }

    /* Start on the Console tab */
    ji_tab_widget_set_current_index(tabs, 0);

    return tabs;
}

/* =========================================================================
 * Helper: Create the status bar
 * ========================================================================= */
static JiStatusBar* create_status_bar(void) {
    JiStatusBar* status = ji_status_bar_new();
    if (!status) return NULL;

    ji_status_bar_show_message(status, "Ready");

    return status;
}

/* =========================================================================
 * Helper: Build the complete dockable layout
 *
 * Layout structure (mimicking Unity Editor):
 *
 * +----------------------------------------------------------+
 * | Menu Bar: File | Edit | View | Window | Help             |
 * +----------------------------------------------------------+
 * | Toolbar: [Move][Rotate][Scale] | [Play][Pause] | [...]  |
 * +----------+---------------------------+-------------------+
 * | Hierarchy|  Scene | Game            | Inspector         |
 * |          |  (center tabs)           | - Transform  [x]  |
 * | - Camera |                          | - Mesh Renderer[x] |
 * | - Light  |                           | - Collider   [ ]  |
 * | - Player |                           | - Rigidbody  [ ]  |
 * |          |                           | - Audio Src  [ ]  |
 * |          +---------------------------+                   |
 * |          | Console | Project          |                   |
 * |          | (bottom tabs)             |                   |
 * +----------+---------------------------+-------------------+
 * | Status Bar: Ready                                         |
 * +----------------------------------------------------------+
 *
 * Implementation uses:
 *   - JiDockManager as the top-level coordinator
 *   - JiDockArea for each panel region
 *   - JiDockWidget for each dockable panel
 *   - JiSplitter for resizable divisions
 *   - JiTabWidget for tabbed panels (Scene/Game, Console/Project)
 * ========================================================================= */
static JiDockManager* build_dock_layout(void) {
    /* ---- Create the dock manager ---- */
    JiDockManager* manager = ji_dock_manager_new();
    if (!manager) {
        fprintf(stderr, "Failed to create dock manager\n");
        return NULL;
    }

    /* ---- Create dock areas for each region ---- */

    /* Left area: Hierarchy panel */
    JiDockArea* left_area = ji_dock_area_new("LeftArea");
    if (left_area) {
        ji_dock_area_set_region(left_area, JI_DOCK_REGION_LEFT);
    }

    /* Center area: Scene/Game tabs */
    JiDockArea* center_area = ji_dock_area_new("CenterArea");
    if (center_area) {
        ji_dock_area_set_region(center_area, JI_DOCK_REGION_CENTER);
    }

    /* Right area: Inspector panel */
    JiDockArea* right_area = ji_dock_area_new("RightArea");
    if (right_area) {
        ji_dock_area_set_region(right_area, JI_DOCK_REGION_RIGHT);
    }

    /* Bottom area: Console/Project tabs */
    JiDockArea* bottom_area = ji_dock_area_new("BottomArea");
    if (bottom_area) {
        ji_dock_area_set_region(bottom_area, JI_DOCK_REGION_BOTTOM);
    }

    /* ---- Create dock widgets for each panel ---- */

    /* Hierarchy panel (left) */
    JiDockWidget* hierarchy_dock = ji_dock_widget_new("hierarchy", "Hierarchy");
    if (hierarchy_dock) {
        JiControl* hierarchy_content = create_hierarchy_content();
        if (hierarchy_content) {
            ji_dock_widget_set_widget(hierarchy_dock, hierarchy_content);
        }
        ji_dock_widget_set_features(hierarchy_dock,
            JI_DOCK_FEATURE_CLOSABLE | JI_DOCK_FEATURE_MOVABLE |
            JI_DOCK_FEATURE_FLOATABLE | JI_DOCK_FEATURE_RESIZABLE);

        if (left_area) {
            ji_dock_area_add_widget(left_area, hierarchy_dock);
        }
    }

    /* Center panel: Scene/Game as tabbed dock widgets */
    JiDockWidget* scene_dock = ji_dock_widget_new("scene", "Scene");
    if (scene_dock) {
        JiTabWidget* center_tabs = create_center_tabs();
        if (center_tabs) {
            ji_dock_widget_set_widget(scene_dock, &center_tabs->control);
        }
        ji_dock_widget_set_features(scene_dock,
            JI_DOCK_FEATURE_CLOSABLE | JI_DOCK_FEATURE_MOVABLE |
            JI_DOCK_FEATURE_FLOATABLE | JI_DOCK_FEATURE_RESIZABLE);

        if (center_area) {
            ji_dock_area_add_widget(center_area, scene_dock);
        }
    }

    /* Inspector panel (right) */
    JiDockWidget* inspector_dock = ji_dock_widget_new("inspector", "Inspector");
    if (inspector_dock) {
        JiControl* inspector_content = create_inspector_content();
        if (inspector_content) {
            ji_dock_widget_set_widget(inspector_dock, inspector_content);
        }
        ji_dock_widget_set_features(inspector_dock,
            JI_DOCK_FEATURE_CLOSABLE | JI_DOCK_FEATURE_MOVABLE |
            JI_DOCK_FEATURE_FLOATABLE | JI_DOCK_FEATURE_RESIZABLE);

        if (right_area) {
            ji_dock_area_add_widget(right_area, inspector_dock);
        }
    }

    /* Bottom panel: Console/Project as tabbed dock widgets */
    JiDockWidget* console_dock = ji_dock_widget_new("console", "Console");
    if (console_dock) {
        JiTabWidget* bottom_tabs = create_bottom_tabs();
        if (bottom_tabs) {
            ji_dock_widget_set_widget(console_dock, &bottom_tabs->control);
        }
        ji_dock_widget_set_features(console_dock,
            JI_DOCK_FEATURE_CLOSABLE | JI_DOCK_FEATURE_MOVABLE |
            JI_DOCK_FEATURE_FLOATABLE | JI_DOCK_FEATURE_RESIZABLE);

        if (bottom_area) {
            ji_dock_area_add_widget(bottom_area, console_dock);
        }
    }

    /* ---- Build splitter hierarchy for resizable layout ---- */

    /*
     * The splitter hierarchy:
     *
     * main_splitter (HORIZONTAL)
     *   ├── left_area (Hierarchy)
     *   ├── center_vsplit (VERTICAL)
     *   │   ├── center_area (Scene/Game)
     *   │   └── bottom_area (Console/Project)
     *   └── right_area (Inspector)
     *
     * This creates the classic Unity 3-column layout with a
     * horizontal split between left/center/right, and a vertical
     * split within the center column for the bottom panel.
     */

    /* Vertical splitter for center column (Scene/Game on top, Console/Project on bottom) */
    JiSplitter* center_vsplit = ji_splitter_new(JI_SPLITTER_VERTICAL);
    if (center_vsplit) {
        /* Center area takes 3/4 of the vertical space */
        ji_splitter_add_panel(center_vsplit, center_area, 100, 3);
        /* Bottom area takes 1/4 of the vertical space */
        ji_splitter_add_panel(center_vsplit, bottom_area, 80, 1);
    }

    /* Main horizontal splitter: left | center | right */
    JiSplitter* main_splitter = ji_splitter_new(JI_SPLITTER_HORIZONTAL);
    if (main_splitter) {
        /* Left panel (Hierarchy) — narrow, stretch factor 1 */
        ji_splitter_add_panel(main_splitter, left_area, 120, 1);
        /* Center column (Scene/Game + Console/Project) — wide, stretch factor 4 */
        ji_splitter_add_panel(main_splitter, center_vsplit, 400, 4);
        /* Right panel (Inspector) — medium, stretch factor 2 */
        ji_splitter_add_panel(main_splitter, right_area, 200, 2);

        /* Make the left and right panels collapsible */
        ji_splitter_set_collapsible(main_splitter, 0, true);
        ji_splitter_set_collapsible(main_splitter, 2, true);
    }

    /* ---- Register areas with the dock manager ---- */
    if (left_area)   ji_dock_manager_add_area(manager, left_area);
    if (center_area) ji_dock_manager_add_area(manager, center_area);
    if (right_area)  ji_dock_manager_add_area(manager, right_area);
    if (bottom_area) ji_dock_manager_add_area(manager, bottom_area);

    /* Set the center area as the root */
    ji_dock_manager_set_root_area(manager, center_area);

    /* Set the active widget to the Scene dock */
    if (scene_dock) {
        ji_dock_manager_set_active_widget(manager, scene_dock);
    }

    /* Store the main splitter in the root area for layout management */
    if (center_area) {
        center_area->splitter = main_splitter;
    }

    return manager;
}

/* =========================================================================
 * Render callback — draws the Unity-like layout
 * ========================================================================= */
static void on_render(JiWindow* window, void* user_data) {
    (void)user_data;

    int w, h;
    ji_window_get_size(window, &w, &h);

    JiDrawingContext* ctx = ji_window_get_drawing_context(window);
    if (!ctx) return;

    /* Clear to Unity-style dark gray background */
    ji_draw_clear(ctx, 0xFF2D2D2D);

    /* ---- Draw menu bar background ---- */
    JiRect menu_rect = ji_rect(0.0, 0.0, (double)w, 24.0);
    JiBrush menu_bg = ji_brush_solid(0xFF3C3C3C);
    ji_draw_fill_rect(ctx, menu_rect, &menu_bg);

    /* Draw menu titles */
    const char* menu_names[] = {"File", "Edit", "View", "Window", "Help"};
    JiBrush menu_text = ji_brush_solid(0xFFCCCCCC);
    double menu_x = 8.0;
    for (int i = 0; i < 5; i++) {
        ji_draw_text(ctx, ji_point(menu_x, 5.0), menu_names[i],
                      &menu_text, "sans-serif", 12.0);
        menu_x += strlen(menu_names[i]) * 8.0 + 16.0;
    }

    /* ---- Draw toolbar background ---- */
    JiRect toolbar_rect = ji_rect(0.0, 24.0, (double)w, 28.0);
    JiBrush toolbar_bg = ji_brush_solid(0xFF444444);
    ji_draw_fill_rect(ctx, toolbar_rect, &toolbar_bg);

    /* Draw toolbar button labels */
    const char* tool_labels[] = {"Move", "Rotate", "Scale", "|",
                                  "Play", "Pause", "Step", "|",
                                  "Layers"};
    double tool_x = 8.0;
    JiBrush tool_text = ji_brush_solid(0xFFDDDDDD);
    for (int i = 0; i < 9; i++) {
        if (strcmp(tool_labels[i], "|") == 0) {
            /* Draw separator */
            JiPoint sep_top = ji_point(tool_x + 4.0, 28.0);
            JiPoint sep_bot = ji_point(tool_x + 4.0, 48.0);
            JiBrush sep_brush = ji_brush_solid(0xFF666666);
            ji_draw_line(ctx, sep_top, sep_bot, &sep_brush, 1.0);
            tool_x += 12.0;
        } else {
            ji_draw_text(ctx, ji_point(tool_x, 30.0), tool_labels[i],
                          &tool_text, "sans-serif", 11.0);
            tool_x += strlen(tool_labels[i]) * 7.0 + 12.0;
        }
    }

    /* ---- Draw panel areas ---- */
    double content_y = 52.0;
    double content_h = (double)h - content_y - 22.0; /* 22px for status bar */

    /* Left panel (Hierarchy) */
    double left_w = 200.0;
    JiRect left_panel = ji_rect(0.0, content_y, left_w, content_h);
    JiBrush panel_bg = ji_brush_solid(0xFF2A2A2A);
    ji_draw_fill_rect(ctx, left_panel, &panel_bg);

    /* Left panel header */
    JiRect left_header = ji_rect(0.0, content_y, left_w, 20.0);
    JiBrush header_bg = ji_brush_solid(0xFF3A3A3A);
    ji_draw_fill_rect(ctx, left_header, &header_bg);
    ji_draw_text(ctx, ji_point(6.0, content_y + 3.0), "Hierarchy",
                  &menu_text, "sans-serif", 11.0);

    /* Hierarchy items */
    const char* hierarchy_items[] = {
        "Main Camera",
        "Directional Light",
        "Canvas",
        "EventSystem",
        "Player",
        "Enemy",
        "Environment"
    };
    JiBrush item_text = ji_brush_solid(0xFFBBBBBB);
    for (int i = 0; i < 7; i++) {
        double item_y = content_y + 24.0 + i * 18.0;
        ji_draw_text(ctx, ji_point(16.0, item_y), hierarchy_items[i],
                      &item_text, "sans-serif", 11.0);
    }

    /* Right panel (Inspector) */
    double right_w = 260.0;
    double right_x = (double)w - right_w;
    JiRect right_panel = ji_rect(right_x, content_y, right_w, content_h);
    ji_draw_fill_rect(ctx, right_panel, &panel_bg);

    /* Right panel header */
    JiRect right_header = ji_rect(right_x, content_y, right_w, 20.0);
    ji_draw_fill_rect(ctx, right_header, &header_bg);
    ji_draw_text(ctx, ji_point(right_x + 6.0, content_y + 3.0), "Inspector",
                  &menu_text, "sans-serif", 11.0);

    /* Inspector component checkboxes */
    const char* components[] = {
        "Transform",
        "Mesh Renderer",
        "Collider",
        "Rigidbody",
        "Audio Source"
    };
    bool checked[] = { true, true, false, false, false };

    for (int i = 0; i < 5; i++) {
        double comp_y = content_y + 28.0 + i * 22.0;
        /* Draw checkbox indicator */
        JiRect checkbox = ji_rect(right_x + 10.0, comp_y, 14.0, 14.0);
        JiBrush checkbox_bg = ji_brush_solid(0xFF505050);
        JiBrush checkbox_fill = ji_brush_solid(0xFF5E9E5E);
        ji_draw_fill_rect(ctx, checkbox, checked[i] ? &checkbox_fill : &checkbox_bg);
        JiPen checkbox_border = ji_pen_solid(0xFF888888, 1.0);
        ji_draw_stroke_rect(ctx, checkbox, &checkbox_border);

        /* Draw checkmark if checked */
        if (checked[i]) {
            JiBrush check_mark = ji_brush_solid(0xFFFFFFFF);
            ji_draw_text(ctx, ji_point(right_x + 12.0, comp_y - 1.0), "x",
                          &check_mark, "sans-serif", 10.0);
        }

        /* Draw component label */
        ji_draw_text(ctx, ji_point(right_x + 30.0, comp_y), components[i],
                      &item_text, "sans-serif", 11.0);
    }

    /* Center area (Scene/Game tabs + bottom Console/Project) */
    double center_x = left_w;
    double center_w = (double)w - left_w - right_w;
    double center_top_h = content_h * 0.7;
    double center_bottom_h = content_h * 0.3;

    /* Center top: Scene/Game tab bar */
    JiRect center_top = ji_rect(center_x, content_y, center_w, center_top_h);
    ji_draw_fill_rect(ctx, center_top, &panel_bg);

    /* Tab bar */
    JiRect tab_bar = ji_rect(center_x, content_y, center_w, 22.0);
    JiBrush tab_bg = ji_brush_solid(0xFF3A3A3A);
    ji_draw_fill_rect(ctx, tab_bar, &tab_bg);

    /* Active tab indicator */
    JiRect active_tab = ji_rect(center_x, content_y, 60.0, 22.0);
    JiBrush active_tab_bg = ji_brush_solid(0xFF2A2A2A);
    ji_draw_fill_rect(ctx, active_tab, &active_tab_bg);

    JiBrush active_text = ji_brush_solid(0xFFFFFFFF);
    ji_draw_text(ctx, ji_point(center_x + 8.0, content_y + 3.0), "Scene",
                  &active_text, "sans-serif", 11.0);
    ji_draw_text(ctx, ji_point(center_x + 68.0, content_y + 3.0), "Game",
                  &menu_text, "sans-serif", 11.0);

    /* Scene viewport placeholder */
    JiBrush scene_text = ji_brush_solid(0xFF666666);
    ji_draw_text(ctx, ji_point(center_x + center_w / 2.0 - 60.0,
                                 content_y + center_top_h / 2.0),
                  "Scene View - 3D Viewport",
                  &scene_text, "sans-serif", 14.0);

    /* Center bottom: Console/Project tabs */
    double bottom_y = content_y + center_top_h;
    JiRect center_bottom = ji_rect(center_x, bottom_y, center_w, center_bottom_h);
    ji_draw_fill_rect(ctx, center_bottom, &panel_bg);

    /* Bottom tab bar */
    JiRect bottom_tab_bar = ji_rect(center_x, bottom_y, center_w, 22.0);
    ji_draw_fill_rect(ctx, bottom_tab_bar, &tab_bg);

    JiRect bottom_active = ji_rect(center_x, bottom_y, 70.0, 22.0);
    ji_draw_fill_rect(ctx, bottom_active, &active_tab_bg);
    ji_draw_text(ctx, ji_point(center_x + 8.0, bottom_y + 3.0), "Console",
                  &active_text, "sans-serif", 11.0);
    ji_draw_text(ctx, ji_point(center_x + 78.0, bottom_y + 3.0), "Project",
                  &menu_text, "sans-serif", 11.0);

    /* Console placeholder text */
    ji_draw_text(ctx, ji_point(center_x + 10.0, bottom_y + 30.0),
                  "[Info] Application started", &scene_text, "monospace", 11.0);
    ji_draw_text(ctx, ji_point(center_x + 10.0, bottom_y + 48.0),
                  "[Info] Scene loaded successfully", &scene_text, "monospace", 11.0);

    /* ---- Draw splitter handles ---- */
    JiBrush splitter_brush = ji_brush_solid(0xFF505050);

    /* Vertical splitter between left and center */
    JiRect v_split1 = ji_rect(left_w - 2.0, content_y, 4.0, content_h);
    ji_draw_fill_rect(ctx, v_split1, &splitter_brush);

    /* Vertical splitter between center and right */
    JiRect v_split2 = ji_rect(right_x - 2.0, content_y, 4.0, content_h);
    ji_draw_fill_rect(ctx, v_split2, &splitter_brush);

    /* Horizontal splitter between center top and bottom */
    JiRect h_split = ji_rect(center_x, bottom_y - 2.0, center_w, 4.0);
    ji_draw_fill_rect(ctx, h_split, &splitter_brush);

    /* ---- Draw status bar ---- */
    JiRect status_rect = ji_rect(0.0, (double)h - 22.0, (double)w, 22.0);
    JiBrush status_bg = ji_brush_solid(0xFF3C3C3C);
    ji_draw_fill_rect(ctx, status_rect, &status_bg);

    JiBrush status_text = ji_brush_solid(0xFFAAAAAA);
    ji_draw_text(ctx, ji_point(8.0, (double)h - 18.0), "Ready",
                  &status_text, "sans-serif", 11.0);

    /* FPS counter on the right side of status bar */
    double fps = ji_window_get_fps(window);
    char fps_str[32];
    snprintf(fps_str, sizeof(fps_str), "FPS: %.0f", fps);
    ji_draw_text(ctx, ji_point((double)w - 80.0, (double)h - 18.0), fps_str,
                  &status_text, "monospace", 11.0);
}

/* =========================================================================
 * Event callback — handle keyboard and close events
 * ========================================================================= */
static void on_event(JiWindow* window, const JiEvent* event, void* user_data) {
    (void)window; (void)user_data;

    switch (event->kind) {
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

/* =========================================================================
 * Close callback
 * ========================================================================= */
static void on_close(JiWindow* window, void* user_data) {
    (void)window; (void)user_data;
    g_running = false;
}

/* =========================================================================
 * Main entry point
 * ========================================================================= */
int main(void) {
    /* ---- Initialize JiUI ---- */
    JiResultCode result = ji_initialize();
    if (JI_FAILED(result)) {
        fprintf(stderr, "Failed to initialize JiUI: %s\n",
                ji_result_to_string(result));
        return 1;
    }

    printf("JiUI v%s - Unity Layout Demo\n", ji_version());

    /* ---- Create platform backend ---- */
    JiPlatformBackend* backend = NULL;
#ifdef JIUI_ENABLE_WAYLAND
    backend = ji_wayland_backend_create();
#endif
#ifdef JIUI_ENABLE_X11
    if (!backend) backend = ji_x11_backend_create();
#endif
    if (!backend) {
        fprintf(stderr, "No platform backend available\n");
        ji_shutdown();
        return 1;
    }

    /* ---- Create the main window ---- */
    JiWindow* window = ji_window_create(
        "Unity Layout Demo - JiUI",
        1280, 720,
        JI_WINDOW_OPENGL | JI_WINDOW_RESIZABLE,
        backend
    );
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        ji_shutdown();
        return 1;
    }

    /* ---- Build the Unity-like UI ---- */

    /* Create menu bar */
    JiMenu* menus[5];
    int menu_count = 0;
    create_menu_bar(menus, &menu_count);
    printf("Created %d menus\n", menu_count);

    /* Create toolbar */
    JiToolBar* toolbar = create_toolbar();
    if (toolbar) {
        printf("Toolbar created with actions and separators\n");
    }

    /* Build the dockable layout */
    JiDockManager* dock_manager = build_dock_layout();
    if (dock_manager) {
        printf("Dock layout built: %d areas, %d floating widgets\n",
               ji_dock_manager_area_count(dock_manager),
               ji_dock_manager_floating_count(dock_manager));
    }

    /* Create status bar */
    JiStatusBar* status_bar = create_status_bar();
    if (status_bar) {
        ji_status_bar_show_message(status_bar, "Ready - Unity Layout Demo");
        printf("Status bar created\n");
    }

    /* ---- Set window callbacks ---- */
    ji_window_set_render_callback(window, on_render, NULL);
    ji_window_set_event_callback(window, on_event, NULL);
    ji_window_set_close_callback(window, on_close, NULL);

    /* ---- Main loop ---- */
    printf("Running Unity Layout Demo... Press ESC to quit.\n");

    while (g_running && ji_window_is_open(window)) {
        if (!ji_window_frame(window)) {
            break;
        }
    }

    /* ---- Cleanup ---- */
    printf("Cleaning up...\n");

    if (status_bar)   ji_status_bar_destroy(status_bar);
    if (dock_manager) ji_dock_manager_destroy(dock_manager);
    if (toolbar)      ji_tool_bar_destroy(toolbar);

    /* Destroy menus */
    for (int i = 0; i < menu_count; i++) {
        if (menus[i]) ji_menu_destroy(menus[i]);
    }

    ji_window_destroy(window);
    ji_shutdown();

    printf("Unity Layout Demo closed.\n");
    return 0;
}
