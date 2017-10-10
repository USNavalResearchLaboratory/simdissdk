if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOB_TEST)

set(PROJECT_FILES
    src/ExampleLOBTest.cpp
)

add_executable(example_lob_test ${PROJECT_FILES})
target_link_libraries(example_lob_test PRIVATE simVis simUtil)
set_target_properties(example_lob_test PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Line of Bearing"
)
vsi_install_target(example_lob_test SDK_Examples)
