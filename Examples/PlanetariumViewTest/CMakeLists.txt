if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_PLANETARIUM_VIEW)

set(PROJECT_FILES
    src/PlanetariumViewTest.cpp
)

add_executable(example_planetarium_view ${PROJECT_FILES})
target_link_libraries(example_planetarium_view PRIVATE simVis simUtil)
set_target_properties(example_planetarium_view PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Planetarium View"
)
vsi_install_target(example_planetarium_view SDK_Examples)
