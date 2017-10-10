if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_TRACK_HISTORY)

set(PROJECT_FILES
    src/TrackHistoryTest.cpp
)

add_executable(example_track_history ${PROJECT_FILES})
target_link_libraries(example_track_history PRIVATE simVis simUtil)
set_target_properties(example_track_history PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Track History"
)
vsi_install_target(example_track_history SDK_Examples)
