if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PROJECTORS)

set(PROJECT_FILES
    Projectors.cpp
)

add_executable(example_projectors ${PROJECT_FILES})
target_link_libraries(example_projectors PRIVATE simVis simUtil)
set_target_properties(example_projectors PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Projectors"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_projectors PRIVATE ImGuiLib)
    target_compile_definitions(example_projectors PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_projectors SDK_Examples)
