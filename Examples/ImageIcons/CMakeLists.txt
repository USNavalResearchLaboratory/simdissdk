if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_IMAGE_ICONS)

set(PROJECT_FILES
    ImageIcons.cpp
)

add_executable(example_imageicons ${PROJECT_FILES})
target_link_libraries(example_imageicons PRIVATE simVis simUtil)
set_target_properties(example_imageicons PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Image Icons"
)
vsi_install_target(example_imageicons SDK_Examples)
