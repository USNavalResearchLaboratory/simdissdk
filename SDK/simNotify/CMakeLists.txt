# ----- simNotify -------------------------------------------------------

set( NOTIFY_INC )
set( NOTIFY_HEADERS
    ${NOTIFY_INC}Notify.h
    ${NOTIFY_INC}NotifyHandler.h
    ${NOTIFY_INC}NotifySeverity.h
    ${NOTIFY_INC}NullNotifyHandler.h
    ${NOTIFY_INC}StandardNotifyHandlers.h
)
set( NOTIFY_SRC )
set( NOTIFY_SOURCES
    ${NOTIFY_SRC}Notify.cpp
    ${NOTIFY_SRC}NotifyHandler.cpp
    ${NOTIFY_SRC}StandardNotifyHandlers.cpp
)
set( NOTIFY_ALL_HEADER
    ../simNotify.h
)

source_group( Headers FILES ${NOTIFY_HEADERS} )
source_group( Sources FILES ${NOTIFY_SOURCES} )
source_group( "" FILES ${NOTIFY_ALL_HEADER} )

set( NOTIFY_PROJECT_FILES
    ${NOTIFY_HEADERS} ${NOTIFY_SOURCES} ${NOTIFY_ALL_HEADER}
)

# Fix the library for static or shared
set( NOTIFY_DEFINITIONS )
set( STATIC_OR_SHARED STATIC )
if( SIMNOTIFY_SHARED )
    set( STATIC_OR_SHARED SHARED )
endif()

add_library(simNotify ${STATIC_OR_SHARED} ${NOTIFY_PROJECT_FILES})
set_target_properties(simNotify PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "SIMDIS SDK - Notify"
)
ApplySDKVersion(simNotify)
target_include_directories(simNotify PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)
if(SIMNOTIFY_SHARED)
    target_compile_definitions(simNotify PRIVATE simNotify_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simNotify PUBLIC simNotify_LIB_EXPORT_STATIC)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    set_target_properties(simNotify PROPERTIES TARGET_EXPORT_NAME "SIMDIS_SDK")
    vsi_install_target(simNotify SDK_Library)
endif()

if(AUTOINSTALL_LIBS AND SIMNOTIFY_SHARED)
    PostBuildInstallSdkSharedObjects( simNotify )
endif()

# Setup header file installation
if(INSTALL_HEADERS)
    install( FILES ${NOTIFY_HEADERS}
        DESTINATION include/simNotify )
    install( FILES ${NOTIFY_ALL_HEADER}
        DESTINATION include )
endif()
