if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ROCKET_BURN)

set(PROJECT_FILES
    RocketBurn.cpp
)

add_executable(example_rocketburn ${PROJECT_FILES})
target_link_libraries(example_rocketburn PRIVATE simVis simUtil)
set_target_properties(example_rocketburn PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Rocket Burn"
)
vsi_install_target(example_rocketburn SDK_Examples)
