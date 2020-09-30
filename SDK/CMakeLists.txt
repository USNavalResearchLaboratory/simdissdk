project(SIMDIS_SDK)

# Configure various version fields
set(SIMDIS_SDK_MAJOR_VERSION 1)
set(SIMDIS_SDK_MINOR_VERSION 14)
set(SIMDIS_SDK_REVISION_VERSION 0)
set(SIMDIS_SDK_SOVERSION 14)
set(SIMDIS_SDK_VERSION_STRING ${SIMDIS_SDK_MAJOR_VERSION}.${SIMDIS_SDK_MINOR_VERSION}.${SIMDIS_SDK_REVISION_VERSION} CACHE INTERNAL "SIMDIS SDK Version String" FORCE)

# Applies version data to static and shared library targets
macro(ApplySDKVersion TARGET)
    set_target_properties(${TARGET} PROPERTIES
        VERSION ${SIMDIS_SDK_VERSION_STRING}
        SOVERSION ${SIMDIS_SDK_SOVERSION}
    )
    # Apply a prefix of the SOVERSION on Windows platform to the DLL names
    if(WIN32)
        # Do not apply to static libraries
        get_target_property(TARGET_TYPE ${TARGET} TYPE)
        if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
            set_target_properties(${TARGET} PROPERTIES PREFIX "sdk${SIMDIS_SDK_SOVERSION}-")
        endif()
    endif()
endmacro()

# Include the definition for installing shared objects post-build
include(PostBuildInstallSharedObjects)

add_subdirectory(simNotify)
add_subdirectory(simCore)
add_subdirectory(simData)
add_subdirectory(simVis)
add_subdirectory(simUtil)
if(QT_FOUND)
    add_subdirectory(simQt)
endif()
