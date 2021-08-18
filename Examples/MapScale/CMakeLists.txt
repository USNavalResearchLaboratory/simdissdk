if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_MAP_SCALE)

set(PROJECT_FILES
    MapScale.cpp
)

add_executable(example_mapscale ${PROJECT_FILES})
target_link_libraries(example_mapscale PRIVATE simVis simUtil)
set_target_properties(example_mapscale PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Map Scale"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_mapscale PRIVATE ImGuiLib)
    target_compile_definitions(example_mapscale PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_mapscale SDK_Examples)
