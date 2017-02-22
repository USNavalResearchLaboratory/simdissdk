project(SIMDIS_SDK)

# Define a macro that helps with auto-installation of DLLs/SOs for AutoInstall-enabled builds.
# This macro is used in the simCore, simData, simVis, and simUtil CMake files.
macro(PostBuildInstallSharedObjects TARGET)
    # VS and non-VS handle Release and Debug configurations differently, so need to handle accordingly  
    set(SHARED_LIBRARY_OUPUT_LOC ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_LIBRARY_DIR})
    if(MSVC)
        # Windows copies the DLLs into the runtime directory
        set(SHARED_LIBRARY_OUPUT_LOC ${CMAKE_INSTALL_PREFIX}/${INSTALLSETTINGS_RUNTIME_DIR})
    endif(MSVC)

    # Make sure the target directory exists
    add_custom_command(TARGET ${TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E make_directory ${SHARED_LIBRARY_OUPUT_LOC}/
        )
    # After the build, copy the library over
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_if_different $<TARGET_FILE:${TARGET}> ${SHARED_LIBRARY_OUPUT_LOC}/
        )
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
