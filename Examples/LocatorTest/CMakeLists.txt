if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_LOCATOR_TEST)

set(PROJECT_FILES
    src/LocatorTestExample.cpp
)

add_executable(example_locator_test ${PROJECT_FILES})
target_link_libraries(example_locator_test PRIVATE simVis simUtil)
set_target_properties(example_locator_test PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Locator Test"
)
vsi_install_target(example_locator_test SDK_Examples)
