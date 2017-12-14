if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_MAP_SCALE)

set(PROJECT_FILES
    MapScale.cpp
)

add_executable(example_map_scale ${PROJECT_FILES})
target_link_libraries(example_map_scale PRIVATE simVis simUtil)
set_target_properties(example_map_scale PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Map Scale"
)
vsi_install_target(example_map_scale SDK_Examples)
