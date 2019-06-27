if(NOT SQLITE3_FOUND OR NOT OSG_FOUND OR NOT OPENTHREADS_FOUND OR NOT OSGEARTH_FOUND)
    return()
endif()

project(PLUGIN_OSGEARTH_DB_DRIVER)

set(PROJECT_SRC
    src/Plugin.cpp
    src/DBTileSource.cpp
    src/QSError.cpp
    src/QSNodeID96.cpp
    src/QSPosXYExtents.cpp
    src/SQLiteDataBaseReadUtil.cpp
    src/Utils/Raster/RasterCommon.cpp
)

set(PROJECT_HEADERS
    include/DBTileSource.h
    include/QSCommon.h
    include/QSCommonGeo.h
    include/QSCommonIntTypes.h
    include/QSError.h
    include/QSNodeID96.h
    include/QSPosXYExtents.h
    include/SQLiteDataBaseReadUtil.h
    include/Utils/Raster/RasterCommon.h
    include/swapbytes.h
)

add_library(osgdb_osgearth_db SHARED ${PROJECT_HEADERS} ${PROJECT_SRC})
target_include_directories(osgdb_osgearth_db PRIVATE include ${SIMDIS_SDK_BINARY_DIR}/simVis/include)
target_link_libraries(osgdb_osgearth_db
    PRIVATE
        SQLITE3
        OSG OSGDB OPENTHREADS
        OSGEARTH
        simCore
)
target_compile_definitions(osgdb_osgearth_db PRIVATE USE_SIMDIS_SDK)
if(SDK_BIG_ENDIAN)
    target_compile_definitions(osgdb_osgearth_db PRIVATE SIM_BIG_ENDIAN)
else()
    target_compile_definitions(osgdb_osgearth_db PRIVATE SIM_LITTLE_ENDIAN)
endif()
if(NOT WIN32)
    target_compile_definitions(osgdb_osgearth_db PRIVATE Linux)
endif()

set_target_properties(osgdb_osgearth_db PROPERTIES
    PREFIX ""
    FOLDER "OSG Plugins"
    PROJECT_LABEL "Plugin - OSGEarth .db Driver"
)
vsi_install_shared_library(osgdb_osgearth_db SDK_OSG_Plugins "${INSTALLSETTINGS_OSGPLUGIN_DIR}")
