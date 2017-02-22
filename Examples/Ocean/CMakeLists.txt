if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_OCEAN)

set(PROJECT_FILES
    Ocean.cpp
)

add_executable(example_ocean ${PROJECT_FILES})
target_link_libraries(example_ocean PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_ocean PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Ocean"
)
vsi_install_target(example_ocean SDK_Examples)

# Pass a variable to compiler if the Triton support is through the NodeKit interface
if(OSGEARTH_TRITON_SUPPORT STREQUAL NODEKIT)
    target_compile_definitions(example_ocean PRIVATE HAVE_TRITON_NODEKIT)
    target_link_libraries(example_ocean PRIVATE OSGEARTH_TRITON)
endif()
# Pass a variable to compiler if the SilverLining support is through the NodeKit interface
if(OSGEARTH_SILVERLINING_SUPPORT STREQUAL NODEKIT)
    target_compile_definitions(example_ocean PRIVATE HAVE_SILVERLINING_NODEKIT)
    target_link_libraries(example_ocean PRIVATE OSGEARTH_SILVERLINING)
endif()

check_osgearth_method_exists(HAVE_OSGEARTH_MAP_GETLAYERS "osgEarth/Map" "getLayers")
if(HAVE_OSGEARTH_MAP_GETLAYERS)
    target_compile_definitions(example_ocean PRIVATE HAVE_OSGEARTH_MAP_GETLAYERS)
endif()
