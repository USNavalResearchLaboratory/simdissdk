if(NOT TARGET OSG OR NOT TARGET OSGDB OR NOT TARGET OPENTHREADS)
    return()
endif()

add_library(osgdb_wbp SHARED ReaderWriterWbp.cpp)
target_link_libraries(osgdb_wbp PRIVATE OSG OSGDB OPENTHREADS)
set_target_properties(osgdb_wbp PROPERTIES
    PREFIX ""
    FOLDER "OSG Plugins"
    PROJECT_LABEL "Plugin - OSG .wbp Driver"
)
vsi_install_shared_library(osgdb_wbp SDK_OSG_Plugins "${INSTALLSETTINGS_OSGPLUGIN_DIR}")
