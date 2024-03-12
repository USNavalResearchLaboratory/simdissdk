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
            "#include <cstdlib>\n"
            "#include <iomanip>\n"
            "#include <sstream>\n"
            "#include <time.h>\n"
            "int main(int argc, char* argv[])\n"
            "{\n"
            "int rv = 0;\n"
            "std::tm jTm = {};\n"
            "#ifdef _MSC_VER\n"
            "std::istringstream is(\"1\");\n"
            "is >> std::get_time(&jTm, \"%j\");\n"
            "if (!is.fail())\n"
            "  rv = jTm.tm_yday;\n"
            "#else\n"
            "const char* timerv = ::strptime(\"1\", \"%j\", &jTm);\n"
            "if (timerv)\n"
            "  rv = jTm.tm_yday;\n"
            "#endif\n"
            "return rv;\n"
            "}\n"
        )
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
