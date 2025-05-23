# ----- simCore/Common -------------------------------------------------------

set(CORE_COMMON_INC Common/)
set(CORE_COMMON_HEADERS
    ${CORE_COMMON_INC}Common.h
    ${CORE_COMMON_INC}Exception.h
    ${CORE_COMMON_INC}Export.h
    ${CORE_COMMON_INC}FileSearch.h
    ${CORE_COMMON_INC}HighPerformanceGraphics.h
    ${CORE_COMMON_INC}ScopeGuard.h
    ${CORE_COMMON_INC}SDKAssert.h
    ${CORE_COMMON_INC}Time.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/simCore/Common/Version.h
)
set(CORE_COMMON_SRC Common/)
set(CORE_COMMON_SOURCES
    ${CORE_COMMON_SRC}Version.cpp
)
source_group(Headers\\Common FILES ${CORE_COMMON_HEADERS})
source_group("Source Files\\Common" FILES ${CORE_COMMON_SOURCES})

# ----- simCore/Calc --------------------------------------------------------

set(CORE_CALC_INC Calc/)
set(CORE_CALC_HEADERS
    ${CORE_CALC_INC}Angle.h
    ${CORE_CALC_INC}Calculations.h
    ${CORE_CALC_INC}Coordinate.h
    ${CORE_CALC_INC}CoordinateConverter.h
    ${CORE_CALC_INC}CoordinateSystem.h
    ${CORE_CALC_INC}DatumConvert.h
    ${CORE_CALC_INC}Dcm.h
    ${CORE_CALC_INC}Gars.h
    ${CORE_CALC_INC}Geometry.h
    ${CORE_CALC_INC}GeoFence.h
    ${CORE_CALC_INC}GogToGeoFence.h
    ${CORE_CALC_INC}Interpolation.h
    ${CORE_CALC_INC}MagneticVariance.h
    ${CORE_CALC_INC}MathConstants.h
    ${CORE_CALC_INC}Math.h
    ${CORE_CALC_INC}Mgrs.h
    ${CORE_CALC_INC}MultiFrameCoordinate.h
    ${CORE_CALC_INC}NumericalAnalysis.h
    ${CORE_CALC_INC}Random.h
    ${CORE_CALC_INC}SquareMatrix.h
    ${CORE_CALC_INC}Units.h
    ${CORE_CALC_INC}UnitContext.h
    ${CORE_CALC_INC}Vec3.h
    ${CORE_CALC_INC}VerticalDatum.h
)
set(CORE_CALC_SRC Calc/)
set(CORE_CALC_SOURCES
    ${CORE_CALC_SRC}Angle.cpp
    ${CORE_CALC_SRC}Calculations.cpp
    ${CORE_CALC_SRC}CoordinateConverter.cpp
    ${CORE_CALC_SRC}CoordinateSystem.cpp
    ${CORE_CALC_SRC}DatumConvert.cpp
    ${CORE_CALC_SRC}Dcm.cpp
    ${CORE_CALC_SRC}Gars.cpp
    ${CORE_CALC_SRC}Geometry.cpp
    ${CORE_CALC_SRC}GeoFence.cpp
    ${CORE_CALC_SRC}GogToGeoFence.cpp
    ${CORE_CALC_SRC}Interpolation.cpp
    ${CORE_CALC_SRC}MagneticVariance.cpp
    ${CORE_CALC_SRC}Math.cpp
    ${CORE_CALC_SRC}Mgrs.cpp
    ${CORE_CALC_SRC}MultiFrameCoordinate.cpp
    ${CORE_CALC_SRC}NumericalAnalysis.cpp
    ${CORE_CALC_SRC}Random.cpp
    ${CORE_CALC_SRC}SquareMatrix.cpp
    ${CORE_CALC_SRC}Units.cpp
    ${CORE_CALC_SRC}UnitContext.cpp
)
source_group(Headers\\Calc FILES ${CORE_CALC_HEADERS})
source_group("Source Files\\Calc" FILES ${CORE_CALC_SOURCES})

# ----- simCore/Calc/LUT --------------------------------------------------------

set(CORE_CALC_LUT_INC LUT/)
set(CORE_CALC_LUT_HEADERS
    ${CORE_CALC_LUT_INC}InterpTable.h
    ${CORE_CALC_LUT_INC}LUT1.h
    ${CORE_CALC_LUT_INC}LUT2.h
)
source_group(Headers\\LUT FILES ${CORE_CALC_LUT_HEADERS})

# ----- simCore/EM --------------------------------------------------------

set(CORE_EM_INC EM/)
set(CORE_EM_HEADERS
    ${CORE_EM_INC}AntennaPattern.h
    ${CORE_EM_INC}Constants.h
    ${CORE_EM_INC}RadarCrossSection.h
    ${CORE_EM_INC}Decibel.h
    ${CORE_EM_INC}ElectroMagRange.h
    ${CORE_EM_INC}Propagation.h
)
set(CORE_EM_SRC EM/)
set(CORE_EM_SOURCES
    ${CORE_EM_SRC}AntennaPattern.cpp
    ${CORE_EM_SRC}Propagation.cpp
    ${CORE_EM_SRC}RadarCrossSection.cpp
)
source_group(Headers\\EM FILES ${CORE_EM_HEADERS})
source_group("Source Files\\EM" FILES ${CORE_EM_SOURCES})

# ----- simCore/String --------------------------------------------------------

set(CORE_STRING_INC String/)
set(CORE_STRING_SRC String/)
set(CORE_STRING_HEADERS
    ${CORE_STRING_INC}Angle.h
    ${CORE_STRING_INC}Constants.h
    ${CORE_STRING_INC}CsvReader.h
    ${CORE_STRING_INC}CsvWriter.h
    ${CORE_STRING_INC}FilePatterns.h
    ${CORE_STRING_INC}Tokenizer.h
    ${CORE_STRING_INC}Format.h
    ${CORE_STRING_INC}TextFormatter.h
    ${CORE_STRING_INC}TextReplacer.h
    ${CORE_STRING_INC}UnitContextFormatter.h
    ${CORE_STRING_INC}UtfUtils.h
    ${CORE_STRING_INC}Utils.h
    ${CORE_STRING_INC}ValidNumber.h
    ${CORE_STRING_INC}XmlWriter.h
)
set(CORE_STRING_SOURCES
    ${CORE_STRING_SRC}Angle.cpp
    ${CORE_STRING_SRC}CsvReader.cpp
    ${CORE_STRING_SRC}CsvWriter.cpp
    ${CORE_STRING_SRC}Format.cpp
    ${CORE_STRING_SRC}Tokenizer.cpp
    ${CORE_STRING_SRC}TextFormatter.cpp
    ${CORE_STRING_SRC}TextReplacer.cpp
    ${CORE_STRING_SRC}UnitContextFormatter.cpp
    ${CORE_STRING_SRC}UtfUtils.cpp
    ${CORE_STRING_SRC}Utils.cpp
    ${CORE_STRING_SRC}ValidNumber.cpp
    ${CORE_STRING_SRC}XmlWriter.cpp
)
source_group(Headers\\String FILES ${CORE_STRING_HEADERS})
source_group("Source Files\\String" FILES ${CORE_STRING_SOURCES})

# ----- simCore/System --------------------------------------------------------

set(CORE_SYSTEM_INC System/)
set(CORE_SYSTEM_SRC System/)
set(CORE_SYSTEM_HEADERS
    ${CORE_SYSTEM_INC}DescriptorStringCapture.h
    ${CORE_SYSTEM_INC}File.h
    ${CORE_SYSTEM_INC}ShellWindow.h
    ${CORE_SYSTEM_INC}Utils.h
)
set(CORE_SYSTEM_SOURCES
    ${CORE_SYSTEM_SRC}DescriptorStringCapture.cpp
    ${CORE_SYSTEM_SRC}File.cpp
    ${CORE_SYSTEM_INC}ShellWindow.cpp
    ${CORE_SYSTEM_SRC}Utils.cpp
)
source_group(Headers\\System FILES ${CORE_SYSTEM_HEADERS})
source_group("Source Files\\System" FILES ${CORE_SYSTEM_SOURCES})

# ----- simCore/Time --------------------------------------------------------

set(CORE_TIME_INC Time/)
set(CORE_TIME_HEADERS
    ${CORE_TIME_INC}Clock.h
    ${CORE_TIME_INC}ClockImpl.h
    ${CORE_TIME_INC}Constants.h
    ${CORE_TIME_INC}CountDown.h
    ${CORE_TIME_INC}DeprecatedStrings.h
    ${CORE_TIME_INC}Exception.h
    ${CORE_TIME_INC}Julian.h
    ${CORE_TIME_INC}String.h
    ${CORE_TIME_INC}TimeClass.h
    ${CORE_TIME_INC}TimeClock.h
    ${CORE_TIME_INC}Utils.h
)
set(CORE_TIME_SRC Time/)
set(CORE_TIME_SOURCES
    ${CORE_TIME_SRC}ClockImpl.cpp
    ${CORE_TIME_SRC}CountDown.cpp
    ${CORE_TIME_SRC}DeprecatedStrings.cpp
    ${CORE_TIME_SRC}Julian.cpp
    ${CORE_TIME_SRC}String.cpp
    ${CORE_TIME_SRC}TimeClass.cpp
    ${CORE_TIME_SRC}TimeClock.cpp
    ${CORE_TIME_SRC}Utils.cpp
)
source_group(Headers\\Time FILES ${CORE_TIME_HEADERS})
source_group("Source Files\\Time" FILES ${CORE_TIME_SOURCES})

# ----- simCore/GOG --------------------------------------------------------

set(CORE_GOG_INC GOG/)
set(CORE_GOG_HEADERS
    ${CORE_GOG_INC}GogUtils.h
    ${CORE_GOG_INC}GogShape.h
    ${CORE_GOG_INC}ParsedShape.h
    ${CORE_GOG_INC}Parser.h
)
set(CORE_GOG_SRC GOG/)
set(CORE_GOG_SOURCES
    ${CORE_GOG_SRC}GogUtils.cpp
    ${CORE_GOG_SRC}GogShape.cpp
    ${CORE_GOG_SRC}ParsedShape.cpp
    ${CORE_GOG_SRC}Parser.cpp
)
source_group(Headers\\GOG FILES ${CORE_GOG_HEADERS})
source_group("Source Files\\GOG" FILES ${CORE_GOG_SOURCES})

# ----------------------------------------------------------------------

# Avoid false MSVC 2017/2019 MSB8027 warning from Unity build on Utils.cpp and Angle.cpp
set_source_files_properties(${CORE_STRING_SRC}Angle.cpp ${CORE_STRING_SRC}Utils.cpp
    ${CORE_CALC_SRC}Angle.cpp ${CORE_TIME_SRC}Utils.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

set(CORE_PROJECT_FILES
    ${CORE_COMMON_HEADERS} ${CORE_COMMON_SOURCES}
    ${CORE_CALC_HEADERS} ${CORE_CALC_SOURCES}
    ${CORE_CALC_LUT_HEADERS}
    ${CORE_EM_HEADERS} ${CORE_EM_SOURCES}
    ${CORE_TIME_HEADERS} ${CORE_TIME_SOURCES}
    ${CORE_STRING_HEADERS} ${CORE_STRING_SOURCES}
    ${CORE_GOG_HEADERS} ${CORE_GOG_SOURCES}
    ${CORE_SYSTEM_HEADERS} ${CORE_SYSTEM_SOURCES}
)

# Create the Version.h file
configure_file(${CORE_COMMON_INC}Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/simCore/Common/Version.h @ONLY)

# Fix the library for static or shared
set(STATIC_OR_SHARED STATIC)
if(SIMCORE_SHARED)
    set(STATIC_OR_SHARED SHARED)
endif()

add_library(simCore ${STATIC_OR_SHARED} ${CORE_PROJECT_FILES})
set_target_properties(simCore PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "simCore"
)
ApplySDKVersion(simCore)
target_include_directories(simCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(simCore PUBLIC simNotify)

if(SIMCORE_SHARED)
    target_compile_definitions(simCore PRIVATE simCore_LIB_EXPORT_SHARED)
else()
    target_compile_definitions(simCore PUBLIC simCore_LIB_EXPORT_STATIC)
endif()

# Part of simCore relies on std::filesystem
find_package(Filesystem QUIET)
if(TARGET std::filesystem)
    get_target_property(STDFS_ILL std::filesystem INTERFACE_LINK_LIBRARIES)
    if(STDFS_ILL)
        target_link_libraries(simCore PRIVATE ${STDFS_ILL})
    endif()
endif()

# Determine whether time.h includes timespec
include(TestTimespec)
test_timespec(SDK_TIMESPEC)
if(SDK_TIMESPEC)
    target_compile_definitions(simCore PUBLIC DEFINED_TIMESPEC_T)
endif()

# simCore TimeStampStr needs to know the yday offset
include(TestYDayOffset)
GET_YDAY_OFFSET(NEED_YDAY_OFFSET_MINUS1)
if(NEED_YDAY_OFFSET_MINUS1)
    target_compile_definitions(simCore PRIVATE NEED_YDAY_OFFSET_MINUS1)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    vsi_install_export(simCore ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
endif()

if(AUTOINSTALL_LIBS AND SIMCORE_SHARED)
    PostBuildInstallSharedObjects(simCore)
endif()

if(INSTALL_HEADERS)
    # Setup header file installation
    install(FILES ${CORE_COMMON_HEADERS}
        DESTINATION include/simCore/Common)
    install(FILES ${CORE_CALC_HEADERS}
        DESTINATION include/simCore/Calc)
    install(FILES ${CORE_CALC_LUT_HEADERS}
        DESTINATION include/simCore/LUT)
    install(FILES ${CORE_EM_HEADERS}
        DESTINATION include/simCore/EM)
    install(FILES ${CORE_STRING_HEADERS}
        DESTINATION include/simCore/String)
    install(FILES ${CORE_TIME_HEADERS}
        DESTINATION include/simCore/Time)
    install(FILES ${CORE_GOG_HEADERS}
        DESTINATION include/simCore/GOG)
    install(FILES ${CORE_SYSTEM_HEADERS}
        DESTINATION include/simCore/System)
endif()
