if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_TEST_VIEWMANAGER)

set(PROJECT_FILES
    TestViewManager.cpp
)

add_executable(example_viewmanager ${PROJECT_FILES})
target_link_libraries(example_viewmanager PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_viewmanager PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - View Manager"
)
vsi_install_target(example_viewmanager SDK_Examples)
