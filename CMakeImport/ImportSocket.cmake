if(WIN32)
    add_library(VSI::SOCKET UNKNOWN IMPORTED)
    set_target_properties(VSI::SOCKET PROPERTIES IMPORTED_LOCATION "ws2_32.lib")
else()
    # Create an empty interface library
    add_library(VSI_SOCKET INTERFACE)
    add_library(VSI::SOCKET ALIAS VSI_SOCKET)
endif()
