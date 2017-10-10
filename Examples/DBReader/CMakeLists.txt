if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_DB_READER)

set(PROJECT_FILES
    DBReader.cpp
)

add_executable(example_dbreader ${PROJECT_FILES})
target_link_libraries(example_dbreader PRIVATE simVis simUtil)
set_target_properties(example_dbreader PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - DB Reader"
)
vsi_install_target(example_dbreader SDK_Examples)
