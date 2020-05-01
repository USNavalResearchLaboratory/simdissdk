if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ECI_TRACK_HISTORY)

set(PROJECT_FILES
    EciTrackHistoryTest.cpp
)

add_executable(example_ecitrackhistorytest ${PROJECT_FILES})
target_link_libraries(example_ecitrackhistorytest PRIVATE simVis simUtil)
set_target_properties(example_ecitrackhistorytest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "ECI Track History Test"
)
vsi_install_target(example_ecitrackhistorytest SDK_Examples)
