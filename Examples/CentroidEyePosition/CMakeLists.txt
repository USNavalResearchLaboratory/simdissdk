if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_CENTROID_EYE_POSITION)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_centroid_eye_position ${PROJECT_FILES})
target_link_libraries(example_centroid_eye_position PRIVATE simVis simUtil)
set_target_properties(example_centroid_eye_position PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Centroid Eye Position"
)
vsi_install_target(example_centroid_eye_position SDK_Examples)
