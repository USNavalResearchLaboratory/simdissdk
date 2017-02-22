if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_GEOFENCING)

set(PROJECT_FILES
    src/ExampleGeoFencing.cpp
)

add_executable(example_geofencing ${PROJECT_FILES})
target_link_libraries(example_geofencing PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_geofencing PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - GeoFencing"
)
vsi_install_target(example_geofencing SDK_Examples)
