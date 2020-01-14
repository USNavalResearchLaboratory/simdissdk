if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_OVERHEAD)

set(PROJECT_FILES
    OverheadExample.cpp
)

add_executable(example_overhead ${PROJECT_FILES})
target_link_libraries(example_overhead PRIVATE simVis simUtil)
set_target_properties(example_overhead PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Overhead"
)
vsi_install_target(example_overhead SDK_Examples)
