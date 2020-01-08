if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOAD_EARTH_FILE)

add_executable(example_loadearthfile LoadEarthFile.cpp)
target_link_libraries(example_loadearthfile PRIVATE simVis simUtil OSGDB OSGEARTH)
set_target_properties(example_loadearthfile PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Load Earth File"
)
vsi_install_target(example_loadearthfile SDK_Examples)
