if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_LOAD_LAYER)

add_executable(example_load_layer LoadLayer.cpp)
target_link_libraries(example_load_layer PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil OSGDB OSGEARTH)
set_target_properties(example_load_layer PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Load Terrain Layers"
)

check_osgearth_method_exists(HAVE_OSGEARTH_LAYEROPTIONS "osgEarth/Layer" "options")
if(HAVE_OSGEARTH_LAYEROPTIONS)
    target_compile_definitions(example_load_layer PRIVATE HAVE_OSGEARTH_LAYEROPTIONS)
endif()

vsi_install_target(example_load_layer SDK_Examples)

check_osgearth_method_exists(HAVE_OSGEARTH_MAP_GETLAYERS "osgEarth/Map" "getLayers")
if(HAVE_OSGEARTH_MAP_GETLAYERS)
    target_compile_definitions(example_load_layer PRIVATE HAVE_OSGEARTH_MAP_GETLAYERS)
endif()
