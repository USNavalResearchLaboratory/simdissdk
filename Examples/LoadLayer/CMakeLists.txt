if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOAD_LAYER)

add_executable(example_load_layer LoadLayer.cpp)
target_link_libraries(example_load_layer PRIVATE simVis simUtil OSGDB OSGEARTH)
set_target_properties(example_load_layer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Load Terrain Layers"
)

vsi_install_target(example_load_layer SDK_Examples)
