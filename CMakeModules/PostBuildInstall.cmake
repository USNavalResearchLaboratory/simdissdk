# post_build_install(<target> <rel_directory>)
#
# Adds custom commands to the build stages of TARGET, such that it will copy
# the output file of the TARGET into the CMAKE_INSTALL_PREFIX/REL_DIRECTORY.
# The REL_DIRECTORY is provided as an argument and must be relative to the
# CMAKE_INSTALL_PREFIX.
macro(post_build_install TARGET REL_DIRECTORY)
    # Make sure the target directory exists
    add_custom_command(TARGET ${TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E make_directory ${CMAKE_INSTALL_PREFIX}/${REL_DIRECTORY}/
        )
    # After the build, copy the library over
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy $<TARGET_FILE:${TARGET}> ${CMAKE_INSTALL_PREFIX}/${REL_DIRECTORY}/
        )
endmacro()
