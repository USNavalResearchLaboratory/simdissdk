if(NOT TARGET simUtil)
    return()
endif()

project(EXAMPLE_ASYNC_MODEL_LOADING)

set(PROJECT_FILES
    main.cpp
)

add_executable(example_async_model_loading ${PROJECT_FILES})
target_link_libraries(example_async_model_loading PRIVATE simVis simUtil)
set_target_properties(example_async_model_loading PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Asynchronous Model Loading"
)
vsi_install_target(example_async_model_loading SDK_Examples)
