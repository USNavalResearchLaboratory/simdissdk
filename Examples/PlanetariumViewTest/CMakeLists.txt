if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PLANETARIUM_VIEW)

set(PROJECT_FILES
    PlanetariumViewTest.cpp
)

add_executable(example_planetariumviewtest ${PROJECT_FILES})
target_link_libraries(example_planetariumviewtest PRIVATE simVis simUtil)
set_target_properties(example_planetariumviewtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Planetarium View Test"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_planetariumviewtest PRIVATE ImGuiLib)
    target_compile_definitions(example_planetariumviewtest PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_planetariumviewtest SDK_Examples)
