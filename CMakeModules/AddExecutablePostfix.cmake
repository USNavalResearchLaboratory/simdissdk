# Provide an implementation of add_executable that applies a debug postfix

if(USING_VCPKG)
    message(STATUS "VCPKG is enabled; disable add_executable() modification to add debug postfix")
else()
    function(add_executable TARGET)
        _add_executable(${TARGET} ${ARGN})
        get_target_property(IS_IMPORTED ${TARGET} IMPORTED)
        if(NOT IS_IMPORTED)
            set_target_properties(${TARGET} PROPERTIES
                DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
                RELEASE_POSTFIX "${CMAKE_RELEASE_POSTFIX}"
                RELWITHDEBINFO_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}"
                MINSIZEREL_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}"
            )
        endif()
    endfunction()
endif()

