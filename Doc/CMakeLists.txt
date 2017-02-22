cmake_minimum_required(VERSION 2.8.12)

# Install the code style guide and SDK footer for Doxygen
install(FILES CodeStyleGuide.pdf
    DESTINATION doc
    COMPONENT Documentation
)

# Create a target to generate the code documentation with Doxygen
find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    return()
endif()

# Doxygen docs require SDKFooter.html
install(FILES SDKFooter.html
    DESTINATION doc
    COMPONENT Documentation
)

# Pull out version variables for SDK configuration
file(STRINGS ../SDK/simCore/Common/Version.h
    MAJOR_MINOR_CODE
    REGEX "^ *static const int SDKVERSION_M..OR.*; *$"
)
string(REGEX REPLACE
    ".*MAJOR = ([0-9]+).*MINOR = ([0-9]+).*" "\\1.\\2"
    SDK_VERSION_MAJOR_MINOR
    "${MAJOR_MINOR_CODE}"
)

# Generate the Doxyfile for use in the build
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
)

# Create a custom target for documentation
add_custom_target(doc
    COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating documentation with Doxygen"
)
set_target_properties(doc PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "Documentation"
)

# Install the documentation if it exists
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION doc
    COMPONENT Documentation
    OPTIONAL
)
