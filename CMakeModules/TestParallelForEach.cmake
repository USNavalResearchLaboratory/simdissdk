#.rst:
# TestParallelForEach
# -------------
#
# Define macro to determine whether std::for_each(std::execution::par, ) is supported
#
# Check if the system has a support for std::for_each(std::execution::par, )
#
# ::
#
#   TEST_PARALLEL_FOREACH(VARIABLE)
#   VARIABLE - variable to store the result to

macro(TEST_PARALLEL_FOREACH VARIABLE)
    if(NOT DEFINED HAVE_${VARIABLE})

        # Tell the user what is happening
        message(STATUS "Check if the system supports std::for_each with execution policy")
        # Write a temporary file containing the test
        set(TMP_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestForEach.cpp")
        file(WRITE "${TMP_FILE}"
            "#include <execution>\n"
            "#include <vector>\n"
            "int main(int argc, char* argv[])\n"
            "{\n"
            "  std::vector<int> vec;\n"
            "  std::for_each(std::execution::par, vec.begin(), vec.end(), [](int) {});\n"
            "  return 0;\n"
            "}\n"
        )
        # Try to build the file
        try_compile(HAVE_${VARIABLE}
            "${CMAKE_BINARY_DIR}"
            "${TMP_FILE}"
            CXX_STANDARD 17
        )

        if(HAVE_${VARIABLE})
            set(${VARIABLE} 1 CACHE INTERNAL "Result of TEST_PARALLEL_FOREACH" FORCE)
            message(STATUS "Check if the system supports std::for_each with execution policy - yes")
        else()
            set(${VARIABLE} 0 CACHE INTERNAL "Result of TEST_PARALLEL_FOREACH" FORCE)
            message(STATUS "Check if the system supports std::for_each with execution policy - no")
        endif()

    endif()
endmacro()
