#.rst:
# TestTimespec
# -------------
#
# Define macro to determine whether time.h includes timespec type
#
# Check if the system has a time.h that supports timespec
#
# ::
#
#   TEST_TIMESPEC(VARIABLE)
#   VARIABLE - variable to store the result to

macro(TEST_TIMESPEC VARIABLE)
    if(NOT DEFINED HAVE_${VARIABLE})

        # Tell the user what is happening
        message(STATUS "Check if the system includes timespec in <time.h>")
        # Write a temporary file containing the test
        set(TMP_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestTimespec.cpp")
        file(WRITE "${TMP_FILE}"
        [=[
            #include <time.h>
            int main(int argc, char* argv[])
            {
              timespec ts;
              ts.tv_sec = 0;
              ts.tv_nsec = 0;
              return 0;
            }
        ]=]
        )
        # Try to build the file
        try_compile(HAVE_${VARIABLE}
            "${CMAKE_BINARY_DIR}"
            "${TMP_FILE}"
        )

        # Determine whether we have timespec in time.h or not
        if(HAVE_${VARIABLE})
            set(${VARIABLE} 1 CACHE INTERNAL "Result of TEST_TIMESPEC" FORCE)
            message(STATUS "Check if the system includes timespec in <time.h> - yes")
        else()
            # time.h does not have timespec
            set(${VARIABLE} 0 CACHE INTERNAL "Result of TEST_TIMESPEC" FORCE)
            message(STATUS "Check if the system includes timespec in <time.h> - no")
        endif()

    endif()
endmacro()
