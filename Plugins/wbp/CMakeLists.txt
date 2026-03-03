if(NOT OPENSCENEGRAPH_FOUND)
    return()
endif()

add_library(osgdb_wbp SHARED ReaderWriterWbp.cpp)
target_link_libraries(osgdb_wbp PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
target_include_directories(osgdb_wbp PRIVATE "${OPENSCENEGRAPH_INCLUDE_DIR}")
set_target_properties(osgdb_wbp PROPERTIES
    PREFIX ""
    FOLDER "OSG Plugins"
    PROJECT_LABEL "Plugin - OSG .wbp Driver"
)
vsi_install_shared_library(osgdb_wbp SDK_OSG_Plugins "${INSTALLSETTINGS_OSGPLUGINS_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}")
