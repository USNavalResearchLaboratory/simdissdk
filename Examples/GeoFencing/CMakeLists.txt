if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_GEOFENCING)

set(PROJECT_FILES
    ExampleGeoFencing.cpp
)

add_executable(example_geofencing ${PROJECT_FILES})
target_link_libraries(example_geofencing PRIVATE simVis simUtil)
set_target_properties(example_geofencing PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "GeoFencing"
)

if (TARGET ImGuiLib)
    target_link_libraries(example_geofencing PRIVATE ImGuiLib)
    target_compile_definitions(example_geofencing PRIVATE HAVE_IMGUI)
endif()

vsi_install_target(example_geofencing SDK_Examples)
