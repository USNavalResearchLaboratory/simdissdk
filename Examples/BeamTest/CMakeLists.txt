if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_BEAM)

set(PROJECT_FILES
    BeamTest.cpp
)

add_executable(example_beamtest ${PROJECT_FILES})
target_link_libraries(example_beamtest PRIVATE simVis simUtil)
set_target_properties(example_beamtest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Beams Test"
)
vsi_install_target(example_beamtest SDK_Examples)
