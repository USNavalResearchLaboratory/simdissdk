project(SIMDIS_SDK)

# Configure various version fields
set(SIMDIS_SDK_MAJOR_VERSION 1)
set(SIMDIS_SDK_MINOR_VERSION 9)
set(SIMDIS_SDK_REVISION_VERSION 0)
set(SIMDIS_SDK_SOVERSION 9)
set(SIMDIS_SDK_VERSION_STRING ${SIMDIS_SDK_MAJOR_VERSION}.${SIMDIS_SDK_MINOR_VERSION}.${SIMDIS_SDK_REVISION_VERSION} CACHE INTERNAL "SIMDIS SDK Version String" FORCE)

# PostBuildInstallSharedObjects(TARGET [CUSTOM_TARGET_NAME])
#
# Define a macro that helps with auto-installation of DLLs/SOs for AutoInstall-enabled builds.
# This macro is used in the simCore, simData, simVis, and simUtil CMake files.
# Copies DLLs/SOs into the correct CMAKE_INSTALL_PREFIX library path for
# the given TARGET.
#
# If the CUSTOM_TARGET_NAME is supplied, then the add_custom_command attaches
# to that target name.  This is useful in cases where the installation of the
# library needs to happen even when the TARGET is not out-of-date and does
# not build, as this command only executes POST_BUILD.
macro(PostBuildInstallSharedObjects TARGET)
    set(CMD_TARGET ${TARGET})
    if(NOT "${ARGV1}" STREQUAL "" AND TARGET ${ARGV1})
        set(CMD_TARGET ${ARGV1})
    endif()
    # VS and non-VS handle Release and Debug configurations differently, so need to handle accordingly
    set(SHARED_LIBRARY_OUPUT_LOC ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
    if(MSVC)
        # Windows copies the DLLs into the runtime directory
        set(SHARED_LIBRARY_OUPUT_LOC ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_RUNTIME_DIR})
    endif(MSVC)

    # Make sure the target directory exists
    add_custom_command(TARGET ${CMD_TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E make_directory ${SHARED_LIBRARY_OUPUT_LOC}/
        )
    # After the build, copy the library over
    add_custom_command(TARGET ${CMD_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different $<TARGET_FILE:${TARGET}> ${SHARED_LIBRARY_OUPUT_LOC}/
        )
endmacro()

# Applies version data to static and shared library targets
macro(ApplySDKVersion TARGET)
    set_target_properties(${TARGET} PROPERTIES
        VERSION ${SIMDIS_SDK_VERSION_STRING}
        SOVERSION ${SIMDIS_SDK_SOVERSION}
    )
    # Apply a prefix of the SOVERSION on Windows platform to the DLL names
    if(WIN32)
        # Do not apply to static libraries
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            set_target_properties(${TARGET} PROPERTIES PREFIX "sdk${SIMDIS_SDK_SOVERSION}-")
        endif()
    endif()
endmacro()

# Define a macro that helps with auto-installation of DLLs/SOs for AutoInstall-enabled builds.
# This macro is used in the simCore, simData, simVis, and simUtil CMake files.
macro(PostBuildInstallSdkSharedObjects TARGET)
    PostBuildInstallSharedObjects(${TARGET})
endmacro()

add_subdirectory(simNotify)
add_subdirectory(simCore)
add_subdirectory(simData)
add_subdirectory(simVis)
add_subdirectory(simUtil)
if(QT_FOUND)
    add_subdirectory(simQt)
endif()
