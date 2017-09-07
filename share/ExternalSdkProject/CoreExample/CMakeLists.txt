# Gracefully fail if the simCore target does not exist
if(NOT TARGET simCore)
    return()
endif()

# Create a new executable that does some work with simCore
add_executable(CoreExample CoreExample.cpp)
target_link_libraries(CoreExample PRIVATE simCore)
