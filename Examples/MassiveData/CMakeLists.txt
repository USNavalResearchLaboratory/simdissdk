if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_MASSIVE_DATA)

set(PROJECT_FILES
    src/MassiveData.cpp
)

add_executable(example_massive_data ${PROJECT_FILES})
target_link_libraries(example_massive_data PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_massive_data PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - Massive Data"
)
vsi_install_target(example_massive_data SDK_Examples)
