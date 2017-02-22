# Gracefully fail if the SIMDIS_SDKCore target does not exist
if(NOT TARGET SIMDIS_SDKCore)
    return()
endif()

# Create a new executable that does some work with simCore
add_executable(CoreExample CoreExample.cpp)
target_link_libraries(CoreExample PRIVATE SIMDIS_SDKCore)
