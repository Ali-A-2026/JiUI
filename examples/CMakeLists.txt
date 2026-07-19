# JiUI Examples

add_executable(hello_jiui hello/main.c)
target_link_libraries(hello_jiui PRIVATE jiui)

add_executable(xml_loader_demo xml_loader/main.c)
target_link_libraries(xml_loader_demo PRIVATE jiui)

# OpenGL-dependent examples — only built when OpenGL backend is enabled
if(JIUI_ENABLE_OPENGL)
    find_package(OpenGL REQUIRED)

    add_executable(window_demo window/main.c)
    target_link_libraries(window_demo PRIVATE jiui m OpenGL::GL)

    add_executable(cube_demo cube/main.c)
    target_link_libraries(cube_demo PRIVATE jiui m OpenGL::GL)

    add_executable(showcase_demo showcase/main.c)
    target_link_libraries(showcase_demo PRIVATE jiui m OpenGL::GL)

    add_subdirectory(unity_layout)
endif()
