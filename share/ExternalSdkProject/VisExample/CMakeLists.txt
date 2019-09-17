# Gracefully fail if the simUtil target does not exist
if(NOT TARGET VSI::simUtil)
    return()
endif()

# Create a new executable that does some work with simVis
add_executable(VisExample VisExample.cpp)
# Note that simUtil automatically pulls in the simVis dependency
target_link_libraries(VisExample PRIVATE VSI::simUtil)
