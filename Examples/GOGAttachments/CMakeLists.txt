if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_GOG_ATTACHMENTS)

set(PROJECT_FILES
    src/ExampleGOGAttachments.cpp
)

add_executable(example_gogattachments ${PROJECT_FILES})
target_link_libraries(example_gogattachments PRIVATE simVis simUtil)
set_target_properties(example_gogattachments PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - GOG Attachments"
)
vsi_install_target(example_gogattachments SDK_Examples)
