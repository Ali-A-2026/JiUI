# JiUI Tests

# Select the appropriate library target for linking tests
if(TARGET jiui)
    set(JIUI_TEST_LIB jiui)
elseif(TARGET jiui_static)
    set(JIUI_TEST_LIB jiui_static)
else()
    message(FATAL_ERROR "No JiUI library target found")
endif()

add_executable(test_types test_types.c)
target_link_libraries(test_types PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_types COMMAND test_types)

add_executable(test_memory test_memory.c)
target_link_libraries(test_memory PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_memory COMMAND test_memory)

add_executable(test_error test_error.c)
target_link_libraries(test_error PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_error COMMAND test_error)

add_executable(test_object test_object.c)
target_link_libraries(test_object PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_object COMMAND test_object)

add_executable(test_parser test_parser.c)
target_link_libraries(test_parser PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_layout test_layout.c)
target_link_libraries(test_layout PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_layout COMMAND test_layout)

add_executable(test_visual test_visual.c)
target_link_libraries(test_visual PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_visual COMMAND test_visual)

add_executable(test_controls test_controls.c)
target_link_libraries(test_controls PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_controls COMMAND test_controls)

add_executable(test_style test_style.c)
target_link_libraries(test_style PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_style COMMAND test_style)

add_executable(test_palette test_palette.c)
target_link_libraries(test_palette PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_palette COMMAND test_palette)

add_executable(test_dock test_dock.c)
target_link_libraries(test_dock PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_dock COMMAND test_dock)

add_executable(test_splitter test_splitter.c)
target_link_libraries(test_splitter PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_splitter COMMAND test_splitter)

add_executable(test_containers test_containers.c)
target_link_libraries(test_containers PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_containers COMMAND test_containers)

add_executable(test_inputs test_inputs.c)
target_link_libraries(test_inputs PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_inputs COMMAND test_inputs)

add_executable(test_buttons test_buttons.c)
target_link_libraries(test_buttons PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_buttons COMMAND test_buttons)

add_executable(test_layouts test_layouts.c)
target_link_libraries(test_layouts PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_layouts COMMAND test_layouts)

add_executable(test_dock_advanced test_dock_advanced.c)
target_link_libraries(test_dock_advanced PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_dock_advanced COMMAND test_dock_advanced)

add_executable(test_advanced test_advanced.c)
target_link_libraries(test_advanced PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_advanced COMMAND test_advanced)

add_executable(test_platform test_platform.c)
target_link_libraries(test_platform PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_platform COMMAND test_platform)

add_executable(test_xml_lexer test_xml_lexer.c)
target_link_libraries(test_xml_lexer PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_xml_lexer COMMAND test_xml_lexer)

add_executable(test_xml_parser test_xml_parser.c)
target_link_libraries(test_xml_parser PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_xml_parser COMMAND test_xml_parser)

add_executable(test_tree_builder test_tree_builder.c)
target_link_libraries(test_tree_builder PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_tree_builder COMMAND test_tree_builder)

add_executable(test_resource test_resource.c)
target_link_libraries(test_resource PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_resource COMMAND test_resource)

add_executable(test_binding test_binding.c)
target_link_libraries(test_binding PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_binding COMMAND test_binding)

add_executable(test_loader test_loader.c)
target_link_libraries(test_loader PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_loader COMMAND test_loader)

add_executable(test_codegen test_codegen.c)
target_link_libraries(test_codegen PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_codegen COMMAND test_codegen)

add_executable(test_cpp test_cpp.cpp)
target_link_libraries(test_cpp PRIVATE ${JIUI_TEST_LIB})
add_test(NAME test_cpp COMMAND test_cpp)

add_executable(test_gpu_scene test_gpu_scene.c)
target_link_libraries(test_gpu_scene PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_gpu_scene COMMAND test_gpu_scene)

add_executable(test_constraint test_constraint.c)
target_link_libraries(test_constraint PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_constraint COMMAND test_constraint)

add_executable(test_gesture test_gesture.c)
target_link_libraries(test_gesture PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_gesture COMMAND test_gesture)

add_executable(test_input test_input.c)
target_link_libraries(test_input PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_input COMMAND test_input)

add_executable(test_jiml test_jiml.c)
target_link_libraries(test_jiml PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_jiml COMMAND test_jiml)

add_executable(test_hot_reload test_hot_reload.c)
target_link_libraries(test_hot_reload PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_hot_reload COMMAND test_hot_reload)

add_executable(test_dock_pro test_dock_pro.c)
target_link_libraries(test_dock_pro PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_dock_pro COMMAND test_dock_pro)

add_executable(test_3d test_3d.c)
target_link_libraries(test_3d PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_3d COMMAND test_3d)

add_executable(test_material test_material.c)
target_link_libraries(test_material PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_material COMMAND test_material)

add_executable(test_physics test_physics.c)
target_link_libraries(test_physics PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_physics COMMAND test_physics)

add_executable(test_profiler test_profiler.c)
target_link_libraries(test_profiler PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_profiler COMMAND test_profiler)

add_executable(test_plugin test_plugin.c)
target_link_libraries(test_plugin PRIVATE ${JIUI_TEST_LIB} m ${CMAKE_DL_LIBS})
add_test(NAME test_plugin COMMAND test_plugin)

add_executable(test_automation test_automation.c)
target_link_libraries(test_automation PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_automation COMMAND test_automation)

add_executable(test_i18n test_i18n.c)
target_link_libraries(test_i18n PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_i18n COMMAND test_i18n)

add_executable(test_accessibility test_accessibility.c)
target_link_libraries(test_accessibility PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_accessibility COMMAND test_accessibility)

add_executable(test_hidpi test_hidpi.c)
target_link_libraries(test_hidpi PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_hidpi COMMAND test_hidpi)

add_executable(test_threads test_threads.c)
target_link_libraries(test_threads PRIVATE ${JIUI_TEST_LIB} m pthread)
add_test(NAME test_threads COMMAND test_threads)

add_executable(test_multimedia test_multimedia.c)
target_link_libraries(test_multimedia PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_multimedia COMMAND test_multimedia)

add_executable(test_screenshot test_screenshot.c)
target_link_libraries(test_screenshot PRIVATE ${JIUI_TEST_LIB} m)
add_test(NAME test_screenshot COMMAND test_screenshot)
