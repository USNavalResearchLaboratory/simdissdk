if(NOT TARGET SIMDIS_SDKVis)
    return()
endif()

project(EXAMPLE_OBSERVERMAKER)

set(PROJECT_FILES
    ObserverMaker.cpp
)

add_executable(example_observermaker ${PROJECT_FILES})
target_link_libraries(example_observermaker PRIVATE SIMDIS_SDKVis)
set_target_properties(example_observermaker PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Observer Maker"
)
vsi_install_target(example_observermaker SDK_Examples)
