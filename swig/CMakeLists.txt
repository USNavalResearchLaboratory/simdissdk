if(NOT SWIG_FOUND OR NOT TARGET Python3::Python)
    return()
endif()

# Avoid making SWIG targets with older CMake, to avoid extra complications
if("${CMAKE_VERSION}" VERSION_LESS 3.13)
    return()
endif()

# UseSWIG relies on UseSWIG_TARGET_NAME_PREFERENCE variable to specify explicit preference for target names.
# Setting this to NEW causes Plug-in API modules to not create targets of expected names.
if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()
# SWIG passes option -module <module_name> to SWIG compiler if file property SWIG_MODULE_NAME is specified
if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
endif()

project(SIMDIS_SDK_SWIG)

mark_as_advanced(SWIG_EXECUTABLE)

include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
# Linux uses LIBRARY for .so, Windows uses RUNTIME for pyd
if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        # Typical out-of-source build
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        # Embedded in VSI build
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    endif()
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif()
include_directories("${Python3_INCLUDE_DIRS}")
include_directories("${SIMDIS_SDK_SOURCE_DIR}")

#macro for installing the specified Python file and the library
macro(InstallPythonFiles TARGET)

    if(WIN32)
        set(PYD_EXTENSION "pyd")
        set(PYD_DESTINATION "${INSTALLSETTINGS_LIBRARY_DIR}/python3.${Python3_VERSION_MINOR}")
    else()
        set(PYD_EXTENSION "so")
        set(PYD_DESTINATION "${INSTALLSETTINGS_LIBRARY_DIR}/python3.${Python3_VERSION_MINOR}/lib-dynload")
    endif()

    install(FILES ${CMAKE_SWIG_OUTDIR}/${TARGET}.py
        DESTINATION bin/pythonScripts
        COMPONENT SIMDIS
    )

    if(WIN32)
        install(PROGRAMS
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release/_${TARGET}.${PYD_EXTENSION}
            DESTINATION ${PYD_DESTINATION}
            COMPONENT SIMDIS
            CONFIGURATIONS Release
        )
        install(PROGRAMS
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/RelWithDebInfo/_${TARGET}.${PYD_EXTENSION}
            DESTINATION ${PYD_DESTINATION}
            COMPONENT SIMDIS
            CONFIGURATIONS RelWithDebInfo
        )
        install(PROGRAMS
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug/_${TARGET}_d.${PYD_EXTENSION}
            DESTINATION ${PYD_DESTINATION}
            COMPONENT SIMDIS
            CONFIGURATIONS Debug
        )
    else()
        # The debug library does not have a _d so only one install is needed
        install(PROGRAMS
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/_${TARGET}.${PYD_EXTENSION}
            DESTINATION ${PYD_DESTINATION}
            COMPONENT SIMDIS
        )
    endif()

endmacro()

add_subdirectory(simCore)
add_subdirectory(Testing)
