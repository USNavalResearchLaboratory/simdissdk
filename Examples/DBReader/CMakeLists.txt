if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_DB_READER)

set(PROJECT_FILES
    DBReader.cpp
)

add_executable(example_dbreader ${PROJECT_FILES})
target_link_libraries(example_dbreader PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_dbreader PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - DB Reader"
)
vsi_install_target(example_dbreader SDK_Examples)

check_osgearth_method_exists(HAVE_OSGEARTH_MAP_GETLAYERS "osgEarth/Map" "getLayers")
if(HAVE_OSGEARTH_MAP_GETLAYERS)
    target_compile_definitions(example_dbreader PRIVATE HAVE_OSGEARTH_MAP_GETLAYERS)
endif()
