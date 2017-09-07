if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_IMAGE_ICONS)

set(PROJECT_FILES
    ImageIcons.cpp
)

add_executable(example_image_icons ${PROJECT_FILES})
target_link_libraries(example_image_icons PRIVATE simVis simUtil)
set_target_properties(example_image_icons PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Image Icons"
)
vsi_install_target(example_image_icons SDK_Examples)
