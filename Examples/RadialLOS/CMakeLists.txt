if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_RADIAL_LOS)

set(PROJECT_FILES
    ExampleRadialLOS.cpp
)

add_executable(example_radiallos ${PROJECT_FILES})
target_link_libraries(example_radiallos PRIVATE simVis simUtil)
set_target_properties(example_radiallos PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Radial LOS"
)

if(TARGET ImGuiLib)
    target_link_libraries(example_radiallos PRIVATE ImGuiLib)
    target_compile_definitions(example_radiallos PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_radiallos SDK_Examples)
