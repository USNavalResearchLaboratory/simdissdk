if(NOT SWIG_FOUND OR NOT TARGET PYTHON3)
    return()
endif()

# Avoid making SWIG targets with older CMake, to avoid extra complications
if("${CMAKE_VERSION}" VERSION_LESS 3.13)
    return()
endif()

project(SIMDIS_SDK_SWIG)

mark_as_advanced(SWIG_EXECUTABLE)

include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
# Linux uses LIBRARY for .so, Windows uses RUNTIME for pyd
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
include_directories("${PYTHON3_LIBRARY_INCLUDE_PATH}")
include_directories("${SIMDIS_SDK_SOURCE_DIR}")

#macro for installing the specified Python file and the library
macro(InstallPythonFiles TARGET)

    if(WIN32)
        set(PYD_EXTENSION "pyd")
        set(PYD_DESTINATION "lib/${BUILD_SYSTEM_ARCH}-nt/python3.${PYTHON3_MINOR}")
    else()
        set(PYD_EXTENSION "so")
        set(PYD_DESTINATION "lib/amd64-linux/python3.${PYTHON3_MINOR}/lib-dynload")
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
