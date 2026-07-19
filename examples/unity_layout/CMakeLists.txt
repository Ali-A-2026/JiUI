# JiUI - Unity Layout Demo Example
# Demonstrates a Unity Engine-like dockable interface with menus,
# toolbars, dockable panels, checkboxes, and status bar.

add_executable(unity_layout main.c)
target_link_libraries(unity_layout PRIVATE jiui m OpenGL::GL)
