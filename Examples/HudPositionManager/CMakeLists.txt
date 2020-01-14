if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_HUD_POSITION_MANAGER)

set(PROJECT_FILES
    HudPositionTest.cpp
)

add_executable(example_hudpositionmanager ${PROJECT_FILES})
target_link_libraries(example_hudpositionmanager PRIVATE simVis simUtil)
set_target_properties(example_hudpositionmanager PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "HUD Position Manager"
)
vsi_install_target(example_hudpositionmanager SDK_Examples)
