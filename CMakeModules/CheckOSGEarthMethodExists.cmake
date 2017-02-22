# check_osgearth_method_exists(<VARIABLE> <FILENAME> <METHOD>
#
# Looks in osgEarth header <FILENAME> for method named <METHOD>, setting
# the value of <VARIABLE> to 1 if found, or empty string if not found.
# Output status messages can be suppressed by setting the
# CMAKE_REQUIRED_QUIET flag before calling this macro.  The <METHOD> name
# should be a whole word, properly capitalized, without parentheses.  The
# <FILENAME> should be relative to the osgEarth library include path.
# This method uses a string search through the file and false positives
# are therefore possible, though unlikely.
#
# Example usage:
#
# include(CheckOSGEarthMethodExists)
# check_osgearth_method_exists(HAVE_SETSTARSVISIBLE "osgEarthUtil/Sky" "setStarsVisible")
# if(HAVE_SETSTARSVISIBLE)
#     target_compile_definitions(SkyModelExtension PRIVATE HAVE_SETSTARSVISIBLE)
# endif()
#
#
macro(check_osgearth_method_exists VARIABLE FILENAME METHOD)
    if(NOT DEFINED "${VARIABLE}" OR "x${${VARIABLE}}" STREQUAL "x${VARIABLE}")
        if(NOT CMAKE_REQUIRED_QUIET)
            message(STATUS "Looking for ${METHOD}() in ${FILENAME}")
        endif()
        if(EXISTS "${OSGEARTH_LIBRARY_INCLUDE_PATH}/${FILENAME}")
            file(STRINGS "${OSGEARTH_LIBRARY_INCLUDE_PATH}/${FILENAME}"
                _OUTPUT
                REGEX "^.* ${METHOD} *\\(.*\\)"
            )
        endif()
        if(NOT "${_OUTPUT}" STREQUAL "")
            set(${VARIABLE} 1 CACHE INTERNAL "Have method for ${METHOD}() in ${FILENAME}")
            if(NOT CMAKE_REQUIRED_QUIET)
                message(STATUS "Looking for ${METHOD}() in ${FILENAME} - found")
            endif()
        else()
            set(${VARIABLE} "" CACHE INTERNAL "Have method for ${METHOD}() in ${FILENAME}")
            if(NOT CMAKE_REQUIRED_QUIET)
                message(STATUS "Looking for ${METHOD}() in ${FILENAME} - not found")
            endif()
        endif()
    endif()
endmacro()
