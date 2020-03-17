# All VSI-related code is built with the pre-CXX11 ABI
if(CMAKE_COMPILER_IS_GNUCXX)
    option(ENABLE_CXX11_ABI "Use the new C++-11 ABI, which is not backwards compatible." OFF)
    if(NOT ENABLE_CXX11_ABI)
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

# CMake 3.1 is required for CMAKE_CXX_STANDARD
cmake_minimum_required(VERSION 3.1)

# Use C++14, but permit decay to older versions (i.e. 14 is not a hard requirement)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
