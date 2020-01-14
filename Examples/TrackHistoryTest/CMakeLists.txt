if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TRACK_HISTORY)

set(PROJECT_FILES
    TrackHistoryTest.cpp
)

add_executable(example_trackhistorytest ${PROJECT_FILES})
target_link_libraries(example_trackhistorytest PRIVATE simVis simUtil)
set_target_properties(example_trackhistorytest PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Track History Test"
)
vsi_install_target(example_trackhistorytest SDK_Examples)
