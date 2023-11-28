# detect_gnucxx_abi(<VARIABLE> [QUIET] [USE_CURRENT_DEFINES] [FORCE]
#             [MESSAGE <message>] [COMPILER_DEFINITIONS ...])
#
# Sets variable <VARIABLE> to either "0" (cxx03) or "1" (cxx11) based on whether the current
# compiler will generate executables for the cxx03 or cxx11 ABI. Although users may attempt
# to force this compiler feature with the compiler definition "-D_GLIBCXX_USE_CXX11_ABI=0" (or
# 1), the OS and compiler can refuse. This function detects the effective ABI used, even if
# the user's compiler refuses to upgrade or downgrade.
#
# If QUIET is specified, no output is written to status. If output is written, it uses the
# MESSAGE text as a prefix. If USE_CURRENT_DEFINES is provided, then the current directory's
# COMPILER_DEFINITIONS will be queried and appended to the provided COMPILER_DEFINITIONS. This
# is useful for picking up calls to add_compile_definitions(), and supplying custom
# COMPILER_DEFINITIONS is useful to determine whether a compiler macro will have an impact
# on the effective ABI.
#
# This detection is cached based on the variable name. If you need to re-run the detection
# based on e.g. an optional adjustment to the compile definitions, pass in the FORCE flag.
#
# This function is only useful for GNU CXX; non-GNU CXX compilers will automatically set
# the VARIABLE to "0".
function(detect_gnucxx11_abi VARIABLE)
    cmake_parse_arguments(CXXABI "QUIET;USE_CURRENT_DEFINES;FORCE" "MESSAGE" "COMPILER_DEFINITIONS" "${ARGV}")

    # Don't retest the same variable multiple times
    if(DEFINED HAVE_${VARIABLE} AND NOT CXXABI_FORCE)
        return()
    endif()

    # Only supports gcc
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
        set(${VARIABLE} "03")
        set(HAVE_${VARIABLE} ON)
        return()
    endif()
    # gcc 5.1 introduces the dual ABI; we're using 03 presumptively if under 5.1
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
        set(${VARIABLE} "03")
        set(HAVE_${VARIABLE} ON)
        return()
    endif()

    if(NOT DEFINED CXXABI_MESSAGE)
        set(CXXABI_MESSAGE "Check if the build uses C++-11 ABI")
    endif()

    # Tell the user what is happening
    if(NOT CXXABI_QUIET)
        message(STATUS "${CXXABI_MESSAGE}")
    endif()

    # Including a C++ library header is required to pick up the CXX11_ABI define if
    # the OS overrides it, so include iostream even if not seemingly directly used.
    set(_TESTCXX11ABI_CPP "\
#include <iostream>\n\
inline bool isCxx11Abi() \n\
{ \n\
#ifdef WIN32 \n\
  return false; \n\
#else \n\
  #if defined(__GNUC__) && __GNUC__ < 5 \n\
    return false; \n\
  #elif defined(_GLIBCXX_USE_CXX11_ABI) \n\
    #if _GLIBCXX_USE_CXX11_ABI == 0 \n\
      return false; \n\
    #else \n\
      return true; \n\
    #endif \n\
  #else \n\
    return true; \n\
  #endif \n\
#endif \n\
} \n\
int main(int argc, char* argv[]) { return isCxx11Abi() ? 1 : 0; }\n")
    # Write a temporary file containing the test
    set(_TMP_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/TestCxxAbi.cpp")
    file(WRITE "${_TMP_FILE}" "${_TESTCXX11ABI_CPP}")

    if(CXXABI_USE_CURRENT_DEFINES)
        get_property(_COMPILE_DEFINITIONS DIRECTORY "." PROPERTY COMPILE_DEFINITIONS)
        list(TRANSFORM _COMPILE_DEFINITIONS PREPEND "-D")
        list(APPEND CXXABI_COMPILE_DEFINITIONS "${_COMPILE_DEFINITIONS}")
    endif()

    # Compile and run the program
    try_run(HAVE_${VARIABLE} COMPILE_${VARIABLE}
        "${CMAKE_BINARY_DIR}"
        "${_TMP_FILE}"
        COMPILE_DEFINITIONS "${CXXABI_COMPILE_DEFINITIONS}"
    )

    if(NOT COMPILE_${VARIABLE})
        set(${VARIABLE} 0 CACHE INTERNAL "Result of detect_gnucxx_abi" FORCE)
        if(NOT CXXABI_QUIET)
            message(WARNING "${CXXABI_MESSAGE} - failed")
        endif()
    elseif(HAVE_${VARIABLE} AND COMPILE_${VARIABLE})
        set(${VARIABLE} 1 CACHE INTERNAL "Result of detect_gnucxx_abi" FORCE)
        if(NOT CXXABI_QUIET)
            message(STATUS "${CXXABI_MESSAGE} - abi-cxx11")
        endif()
    else()
        set(${VARIABLE} 0 CACHE INTERNAL "Result of detect_gnucxx_abi" FORCE)
        if(NOT CXXABI_QUIET)
            message(STATUS "${CXXABI_MESSAGE} - abi-cxx03")
        endif()
    endif()
    unset(COMPILE_${VARIABLE} CACHE)
endfunction()
