if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOAD_EARTH_FILE)

add_executable(example_load_earth_file LoadEarthFile.cpp)
target_link_libraries(example_load_earth_file PRIVATE simVis simUtil OSGDB OSGEARTH)
set_target_properties(example_load_earth_file PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Load Earth File"
)
vsi_install_target(example_load_earth_file SDK_Examples)
