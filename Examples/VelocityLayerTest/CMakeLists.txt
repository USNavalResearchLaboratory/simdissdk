if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TEST_VELOCITYLAYER)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_velocitylayertest ${PROJECT_FILES})
target_link_libraries(example_velocitylayertest PRIVATE simUtil)
set_target_properties(example_velocitylayertest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Velocity Layer Test"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_velocitylayertest PRIVATE ImGuiLib)
    target_compile_definitions(example_velocitylayertest PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_velocitylayertest SDK_Examples)
