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
function(PostBuildInstallSharedObjects TARGET)
    set(CMD_TARGET ${TARGET})
    if(NOT "${ARGV1}" STREQUAL "" AND TARGET ${ARGV1})
        set(CMD_TARGET ${ARGV1})
    endif()

    # Make sure the target directory exists
    add_custom_command(TARGET ${CMD_TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E make_directory ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/
    )
    # After the build, copy the library over
    add_custom_command(TARGET ${CMD_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different $<TARGET_FILE:${TARGET}> ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_SHARED_LIBRARY_DIR}/
    )
endfunction()
