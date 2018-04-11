if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOAD_LAYER)

add_executable(example_loadlayer LoadLayer.cpp)
target_link_libraries(example_loadlayer PRIVATE simVis simUtil OSGDB OSGEARTH)
set_target_properties(example_loadlayer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Load Terrain Layers"
)

vsi_install_target(example_loadlayer SDK_Examples)
