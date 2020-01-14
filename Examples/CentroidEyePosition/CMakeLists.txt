if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_CENTROID_EYE_POSITION)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_centroideyeposition ${PROJECT_FILES})
target_link_libraries(example_centroideyeposition PRIVATE simVis simUtil)
set_target_properties(example_centroideyeposition PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Centroid Eye Position"
)
vsi_install_target(example_centroideyeposition SDK_Examples)
