# configure_shaders(<TEMPLATE_FILE> <OUTPUT_CPP_FILE> shader1 [shader2 ...])
#
# Reads shader (GLSL) files from provided shader source files and creates a
# generated .cpp file that includes the original source code.  This works by
# generating another CMake file that is executed at compile time that generates
# the source files.
#
# For example:
#   configure_shaders(Template.cpp.in Template.cpp file1.glsl file2.glsl)
#
# Adapted from osgEarth configure_shaders()
#
macro(configure_shaders TEMPLATE_FILE OUTPUT_CPP_FILE)
    # Configure the "@" variables used in the ConfigureShadersTemplate.cmake.in file
    set(SHADER_FILES ${ARGN})
    get_filename_component(TEMPLATE_FILE "${TEMPLATE_FILE}" ABSOLUTE)
    set(OUTPUT_CPP_FILE "${OUTPUT_CPP_FILE}")

    # Generate the build-time script that will create output CPP file with inline shaders
    configure_file(
        "${CMAKE_SOURCE_DIR}/CMakeModules/ConfigureShadersTemplate.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ConfigureShadersTemplate.cmake"
        @ONLY
    )

    # Add a custom build-time command to run the script
    add_custom_command(OUTPUT "${OUTPUT_CPP_FILE}"
        COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/ConfigureShadersTemplate.cmake"
        DEPENDS
            ${ARGN}
            "${TEMPLATE_FILE}"
            "${CMAKE_SOURCE_DIR}/CMakeModules/ConfigureShadersTemplate.cmake.in"
    )
endmacro()
