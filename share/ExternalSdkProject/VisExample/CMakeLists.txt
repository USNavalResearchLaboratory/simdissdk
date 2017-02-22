# Gracefully fail if the SIMDIS_SDKUtil target does not exist
if(NOT TARGET SIMDIS_SDKUtil)
    return()
endif()

# Create a new executable that does some work with simVis
add_executable(VisExample VisExample.cpp)
# Note that SIMDIS_SDKUtil automatically pulls in the SIMDIS_SDKVis dependency
target_link_libraries(VisExample PRIVATE SIMDIS_SDKUtil)
