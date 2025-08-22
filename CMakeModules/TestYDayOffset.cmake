#.rst:
# TestYDayOffset
# -------------
#
# Define macro to determine what the std::tm tm_yday offset is for the system when parsing %j.
# Some systems parse %j into tm_yday in the range [0-365] and others in the range [1-366]
#
# Check how the system parses %j into tm_yday
#
# ::
#
#   GET_YDAY_OFFSET(VARIABLE)
#   VARIABLE - variable to store the result to, expected to be 0 or 1, result of parsing 1 into %j

macro(GET_YDAY_OFFSET VARIABLE)
    if(NOT DEFINED HAVE_${VARIABLE})

        # Tell the user what is happening
        message(STATUS "Check if time parse %j needs an offset for tm_yday")
        # Write a temporary file containing the test
        set(TMP_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestYDayOffset.cpp")
        file(WRITE "${TMP_FILE}"
[=[
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <time.h>
int main(int argc, char* argv[])
{
    int rv = 0;
    std::tm jTm = {};
    #ifdef _MSC_VER
    std::istringstream is("1");
    is >> std::get_time(&jTm, "%j");
    if (!is.fail())
      rv = jTm.tm_yday;
    #else
    const char* timerv = ::strptime("1", "%j", &jTm);
    if (timerv)
      rv = jTm.tm_yday;
    #endif
    return rv;
}
]=])
        # Try to build the file
        try_run(HAVE_${VARIABLE} COMPILE_${VARIABLE}
            "${CMAKE_BINARY_DIR}"
            "${TMP_FILE}"
            CXX_STANDARD 17
        )

        if(NOT COMPILE_${VARIABLE} OR NOT HAVE_${VARIABLE})
            set(${VARIABLE} 0 CACHE INTERNAL "Result of GET_YDAY_OFFSET" FORCE)
            message(STATUS "Check if time parse %j needs an offset for tm_yday - no")
        else()
            set(${VARIABLE} ${HAVE_${VARIABLE}} CACHE INTERNAL "Result of GET_YDAY_OFFSET" FORCE)
            message(STATUS "Check if time parse %j needs an offset for tm_yday - yes")
        endif()

    endif()
endmacro()
