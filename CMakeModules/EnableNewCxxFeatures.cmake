# All VSI-related code is built with the pre-CXX11 ABI
if(CMAKE_COMPILER_IS_GNUCXX)
    option(ENABLE_CXX11_ABI "Use the new C++-11 ABI, which is not backwards compatible." OFF)
    if(NOT ENABLE_CXX11_ABI)
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
elseif(MSVC)
    # Fix __cplusplus on MSVC 2015+ (https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/)
    add_definitions(-Zc:__cplusplus)
endif()

# CMake 3.1 is required for CMAKE_CXX_STANDARD
cmake_minimum_required(VERSION 3.1)

# Use C++17, but permit decay to older versions (i.e. 17 is not a hard requirement)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
