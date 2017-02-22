if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

project(EXAMPLE_GOG_READER)

set(PROJECT_FILES
    GOGReader.cpp
)

add_executable(example_gogreader ${PROJECT_FILES})
target_link_libraries(example_gogreader PRIVATE SIMDIS_SDKVis SIMDIS_SDKUtil)
set_target_properties(example_gogreader PROPERTIES
    FOLDER "Examples"
    PROJECT_LABEL "Example - GOG Reader"
)
vsi_install_target(example_gogreader SDK_Examples)
