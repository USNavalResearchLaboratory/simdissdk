find_package(Protobuf QUIET)
find_package(EnTT QUIET)

if(NOT TARGET protobuf::libprotobuf)
    message(STATUS "Skipping simData because of missing PROTOBUF dependencies.")
    return()
endif()

# Variables used in the simDataConfig.cmake generation
include(CreateProtobufLibrary)
set(SIMDATA_HAVE_ENTT OFF)

# ----- simData --------------------------------------------------------

set(DATA_INC)
set(DATA_SRC)
set(DATA_HEADERS
    ${DATA_INC}DataEntry.h
    ${DATA_INC}DataLimiter.h
    ${DATA_INC}DataSlice.h
    ${DATA_INC}DataStore.h
    ${DATA_INC}DataStoreHelpers.h
    ${DATA_INC}DataStoreProxy.h
    ${DATA_INC}DataSliceUpdaters.h
    ${DATA_INC}DataTable.h
    ${DATA_INC}DataTypes.h
    ${DATA_INC}EntityNameCache.h
    ${DATA_INC}GenericIterator.h
    ${DATA_INC}Interpolator.h
    ${DATA_INC}LimitData.h
    ${DATA_INC}LinearInterpolator.h
    ${DATA_INC}MemoryDataEntry.h
    ${DATA_INC}MemoryDataStore.h
    ${DATA_INC}MemoryDataSlice.h
    ${DATA_INC}MemoryDataSlice-inl.h
    ${DATA_INC}MemoryGenericDataSlice.h
    ${DATA_INC}NearestNeighborInterpolator.h
    ${DATA_INC}ObjectId.h
    ${DATA_INC}PrefRulesManager.h
    ${DATA_INC}TableCellTranslator.h
    ${DATA_INC}TableStatus.h
    ${DATA_INC}UpdateComp.h
)

set(DATA_SOURCES
    ${DATA_SRC}BeamMemoryCommandSlice.cpp
    ${DATA_SRC}DataStore.cpp
    ${DATA_SRC}DataStoreHelpers.cpp
    ${DATA_SRC}DataStoreProxy.cpp
    ${DATA_SRC}DataTable.cpp
    ${DATA_SRC}DataTypes.cpp
    ${DATA_SRC}EntityNameCache.cpp
    ${DATA_SRC}GateMemoryCommandSlice.cpp
    ${DATA_SRC}LinearInterpolator.cpp
    ${DATA_SRC}LobGroupMemoryDataSlice.cpp
    ${DATA_SRC}MemoryDataStore.cpp
    ${DATA_SRC}MemoryGenericDataSlice.cpp
    ${DATA_SRC}NearestNeighborInterpolator.cpp
    ${DATA_SRC}TableStatus.cpp
)

set (CATEGORY_DATA_HEADERS
    ${DATA_INC}CategoryData/CategoryData.h
    ${DATA_INC}CategoryData/CategoryFilter.h
    ${DATA_INC}CategoryData/CategoryNameManager.h
    ${DATA_INC}CategoryData/MemoryCategoryDataSlice.h
)

set(CATEGORY_DATA_SOURCES
    ${DATA_SRC}CategoryData/CategoryFilter.cpp
    ${DATA_SRC}CategoryData/CategoryNameManager.cpp
    ${DATA_SRC}CategoryData/MemoryCategoryDataSlice.cpp
)

set (MEMORYTABLE_HEADERS
    ${DATA_INC}MemoryTable/TableManager.h
    ${DATA_INC}MemoryTable/Table.h
    ${DATA_INC}MemoryTable/SubTable.h
    ${DATA_INC}MemoryTable/TimeContainer.h
    ${DATA_INC}MemoryTable/DoubleBufferTimeContainer.h
    ${DATA_INC}MemoryTable/DataColumn.h
    ${DATA_INC}MemoryTable/DataContainer.h
    ${DATA_INC}MemoryTable/DataLimitsProvider.h
)

set(MEMORYTABLE_SOURCES
    ${DATA_SRC}MemoryTable/TableManager.cpp
    ${DATA_SRC}MemoryTable/Table.cpp
    ${DATA_SRC}MemoryTable/SubTable.cpp
    ${DATA_SRC}MemoryTable/DoubleBufferTimeContainer.cpp
    ${DATA_SRC}MemoryTable/DataColumn.cpp
)

set(MESSAGEVISITOR_HEADERS
    ${DATA_INC}MessageVisitor/Message.h
    ${DATA_INC}MessageVisitor/MessageVisitor.h
    ${DATA_INC}MessageVisitor/protobuf.h
)

set(MESSAGEVISITOR_SOURCES
    ${DATA_SRC}MessageVisitor/Message.cpp
    ${DATA_SRC}MessageVisitor/MessageVisitor.cpp
)

set(DATA_GENERATED_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/simData.pb.h
    ${DATA_INC}GeneratedCode/simData.proto
)

set(DATA_GENERATED_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/simData.pb.cc
)

source_group(Headers FILES ${DATA_HEADERS})
source_group("Source Files" FILES ${DATA_SOURCES})
source_group(Headers\\MemoryTable FILES ${MEMORYTABLE_HEADERS})
source_group("Source Files\\MemoryTable" FILES ${MEMORYTABLE_SOURCES})
source_group(Headers\\CategoryData FILES ${CATEGORY_DATA_HEADERS})
source_group("Source Files\\CategoryData" FILES ${CATEGORY_DATA_SOURCES})
source_group(Headers\\MessageVisitor FILES ${MESSAGEVISITOR_HEADERS})
source_group("Source Files\\MessageVisitor" FILES ${MESSAGEVISITOR_SOURCES})
source_group(Headers\\GeneratedCode
    FILES
        ${DATA_GENERATED_HEADERS}
        ${DATA_GENERATED_SOURCES}
)


# Generate the .pb.cc and .pb.h file from protobuf
vsi_protobuf_generate(simData_protobuf_generation "${CMAKE_CURRENT_SOURCE_DIR}/GeneratedCode" "simData.proto"
    SIMDATA_PROTO_H SIMDATA_PROTO_CC =dllexport_decl=SDKDATA_EXPORT:.)
set_target_properties(simData_protobuf_generation PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "simData Protobuf Generation")

# ----------------------------------------------------------------------

set(DATA_PROJECT_FILES
    ${DATA_HEADERS} ${DATA_SOURCES} ${DATA_GENERATED_HEADERS}
    ${MEMORYTABLE_HEADERS} ${MEMORYTABLE_SOURCES} ${CATEGORY_DATA_HEADERS} ${CATEGORY_DATA_SOURCES} ${MESSAGEVISITOR_HEADERS} ${MESSAGEVISITOR_SOURCES}
)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMDATA_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(simData ${STATIC_OR_SHARED} ${DATA_PROJECT_FILES})
set_target_properties(simData PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "simData"
)
ApplySDKVersion(simData)
# Need binary path for generated CMake files
target_include_directories(simData PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)


target_link_libraries(simData PUBLIC protobuf::libprotobuf simCore simNotify)
add_dependencies(simData simData_protobuf_generation)
if(SIMDATA_SHARED)
    target_compile_definitions(simData PRIVATE simData_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simData PUBLIC simData_LIB_EXPORT_STATIC)
endif()

if(EnTT_FOUND)
    target_compile_definitions(simData PUBLIC HAVE_ENTT)
    target_link_libraries(simData PUBLIC EnTT::EnTT)
    set(SIMDATA_HAVE_ENTT ON)
endif()

if(WIN32)
    target_compile_options(simData PRIVATE -bigobj)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    # Configure the simDataConfig.cmake file; its dependencies are dynamic
    configure_file(simDataConfig.cmake.in simDataConfig.cmake @ONLY)
    vsi_install_export(simData ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
endif()

if(AUTOINSTALL_LIBS AND SIMDATA_SHARED)
    PostBuildInstallSharedObjects(simData)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(
        FILES
            ${DATA_HEADERS}
        DESTINATION include/simData
    )

    # Headers for data tables
    install(
        FILES
            ${MEMORYTABLE_HEADERS}
        DESTINATION include/simData/MemoryTable
    )

    # Headers for category data
    install(
        FILES
            ${CATEGORY_DATA_HEADERS}
        DESTINATION include/simData/CategoryData
    )

    # Headers for protobuf processing
    install(
        FILES
            ${MESSAGEVISITOR_HEADERS}
        DESTINATION include/simData/MessageVisitor
    )

    # Headers for generated files
    install(
        FILES
            ${DATA_GENERATED_HEADERS}
        DESTINATION include/simData
    )
endif()
