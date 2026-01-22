# ----- simCore/Common -------------------------------------------------------

set(CORE_COMMON_HEADERS
    Common/Common.h
    Common/Exception.h
    Common/Export.h
    Common/FileSearch.h
    Common/HighPerformanceGraphics.h
    Common/ScopeGuard.h
    Common/SDKAssert.h
    Common/Time.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/simCore/Common/Version.h
)
set(CORE_COMMON_SOURCES
    Common/Version.cpp
)
source_group(Headers\\Common FILES ${CORE_COMMON_HEADERS})
source_group("Source Files\\Common" FILES ${CORE_COMMON_SOURCES})

# ----- simCore/Calc --------------------------------------------------------

set(CORE_CALC_HEADERS
    Calc/Angle.h
    Calc/Calculations.h
    Calc/Coordinate.h
    Calc/CoordinateConverter.h
    Calc/CoordinateSystem.h
    Calc/DatumConvert.h
    Calc/Dcm.h
    Calc/Gars.h
    Calc/Geometry.h
    Calc/GeoFence.h
    Calc/GogToGeoFence.h
    Calc/Interpolation.h
    Calc/MagneticVariance.h
    Calc/MathConstants.h
    Calc/Math.h
    Calc/Mgrs.h
    Calc/MultiFrameCoordinate.h
    Calc/NumericalAnalysis.h
    Calc/Random.h
    Calc/SquareMatrix.h
    Calc/Units.h
    Calc/UnitContext.h
    Calc/Vec3.h
    Calc/VerticalDatum.h
)
set(CORE_CALC_SOURCES
    Calc/Angle.cpp
    Calc/Calculations.cpp
    Calc/CoordinateConverter.cpp
    Calc/CoordinateSystem.cpp
    Calc/DatumConvert.cpp
    Calc/Dcm.cpp
    Calc/Gars.cpp
    Calc/Geometry.cpp
    Calc/GeoFence.cpp
    Calc/GogToGeoFence.cpp
    Calc/Interpolation.cpp
    Calc/MagneticVariance.cpp
    Calc/Math.cpp
    Calc/Mgrs.cpp
    Calc/MultiFrameCoordinate.cpp
    Calc/NumericalAnalysis.cpp
    Calc/Random.cpp
    Calc/SquareMatrix.cpp
    Calc/Units.cpp
    Calc/UnitContext.cpp
)
source_group(Headers\\Calc FILES ${CORE_CALC_HEADERS})
source_group("Source Files\\Calc" FILES ${CORE_CALC_SOURCES})

# ----- simCore/EM --------------------------------------------------------

set(CORE_EM_HEADERS
    EM/AntennaPattern.h
    EM/Constants.h
    EM/RadarCrossSection.h
    EM/Decibel.h
    EM/ElectroMagRange.h
    EM/Propagation.h
)
set(CORE_EM_SOURCES
    EM/AntennaPattern.cpp
    EM/Propagation.cpp
    EM/RadarCrossSection.cpp
)
source_group(Headers\\EM FILES ${CORE_EM_HEADERS})
source_group("Source Files\\EM" FILES ${CORE_EM_SOURCES})

# ----- simCore/Formats --------------------------------------------------------

set(CORE_FORMATS_HEADERS
    Formats/DisModels.h
)
set(CORE_FORMATS_SOURCES
    Formats/DisModels.cpp
)
source_group(Headers\\Formats FILES ${CORE_FORMATS_HEADERS})
source_group("Source Files\\Formats" FILES ${CORE_FORMATS_SOURCES})

# ----- simCore/LUT --------------------------------------------------------

set(CORE_LUT_HEADERS
    LUT/InterpTable.h
    LUT/LUT1.h
    LUT/LUT2.h
)
source_group(Headers\\LUT FILES ${CORE_LUT_HEADERS})

# ----- simCore/String --------------------------------------------------------

set(CORE_STRING_HEADERS
    String/Angle.h
    String/Constants.h
    String/CsvReader.h
    String/CsvWriter.h
    String/FilePatterns.h
    String/Tokenizer.h
    String/Format.h
    String/TextFormatter.h
    String/TextReplacer.h
    String/UnitContextFormatter.h
    String/UtfUtils.h
    String/Utils.h
    String/ValidNumber.h
    String/XmlWriter.h
)
set(CORE_STRING_SOURCES
    String/Angle.cpp
    String/CsvReader.cpp
    String/CsvWriter.cpp
    String/Format.cpp
    String/Tokenizer.cpp
    String/TextFormatter.cpp
    String/TextReplacer.cpp
    String/UnitContextFormatter.cpp
    String/UtfUtils.cpp
    String/Utils.cpp
    String/ValidNumber.cpp
    String/XmlWriter.cpp
)
source_group(Headers\\String FILES ${CORE_STRING_HEADERS})
source_group("Source Files\\String" FILES ${CORE_STRING_SOURCES})

# ----- simCore/System --------------------------------------------------------

set(CORE_SYSTEM_HEADERS
    System/DescriptorStringCapture.h
    System/File.h
    System/MemoryInfo.h
    System/ShellWindow.h
    System/Utils.h
)
set(CORE_SYSTEM_SOURCES
    System/DescriptorStringCapture.cpp
    System/File.cpp
    System/MemoryInfo.cpp
    System/ShellWindow.cpp
    System/Utils.cpp
)
source_group(Headers\\System FILES ${CORE_SYSTEM_HEADERS})
source_group("Source Files\\System" FILES ${CORE_SYSTEM_SOURCES})

# ----- simCore/Time --------------------------------------------------------

set(CORE_TIME_HEADERS
    Time/Clock.h
    Time/ClockImpl.h
    Time/Constants.h
    Time/CountDown.h
    Time/DeprecatedStrings.h
    Time/Exception.h
    Time/Julian.h
    Time/String.h
    Time/TimeClass.h
    Time/TimeClock.h
    Time/Utils.h
)
set(CORE_TIME_SOURCES
    Time/ClockImpl.cpp
    Time/CountDown.cpp
    Time/DeprecatedStrings.cpp
    Time/Julian.cpp
    Time/String.cpp
    Time/TimeClass.cpp
    Time/TimeClock.cpp
    Time/Utils.cpp
)
source_group(Headers\\Time FILES ${CORE_TIME_HEADERS})
source_group("Source Files\\Time" FILES ${CORE_TIME_SOURCES})

# ----- simCore/GOG --------------------------------------------------------

set(CORE_GOG_HEADERS
    GOG/GogUtils.h
    GOG/GogShape.h
    GOG/ParsedShape.h
    GOG/Parser.h
)
set(CORE_GOG_SOURCES
    GOG/GogUtils.cpp
    GOG/GogShape.cpp
    GOG/ParsedShape.cpp
    GOG/Parser.cpp
)
source_group(Headers\\GOG FILES ${CORE_GOG_HEADERS})
source_group("Source Files\\GOG" FILES ${CORE_GOG_SOURCES})

# ----------------------------------------------------------------------

# Avoid false MSVC 2017/2019 MSB8027 warning from Unity build on Utils.cpp and Angle.cpp
set_source_files_properties(String/Angle.cpp String/Utils.cpp
    Calc/Angle.cpp Time/Utils.cpp System/MemoryInfo.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

set(CORE_PROJECT_FILES
    ${CORE_CALC_HEADERS} ${CORE_CALC_SOURCES}
    ${CORE_COMMON_HEADERS} ${CORE_COMMON_SOURCES}
    ${CORE_EM_HEADERS} ${CORE_EM_SOURCES}
    ${CORE_FORMATS_HEADERS} ${CORE_FORMATS_SOURCES}
    ${CORE_GOG_HEADERS} ${CORE_GOG_SOURCES}
    ${CORE_LUT_HEADERS}
    ${CORE_STRING_HEADERS} ${CORE_STRING_SOURCES}
    ${CORE_SYSTEM_HEADERS} ${CORE_SYSTEM_SOURCES}
    ${CORE_TIME_HEADERS} ${CORE_TIME_SOURCES}
)

# Create the Version.h file
configure_file(Common/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/simCore/Common/Version.h @ONLY)

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
    install(FILES ${CORE_EM_HEADERS}
        DESTINATION include/simCore/EM)
    install(FILES ${CORE_FORMATS_HEADERS}
        DESTINATION include/simCore/Formats)
    install(FILES ${CORE_GOG_HEADERS}
        DESTINATION include/simCore/GOG)
    install(FILES ${CORE_LUT_HEADERS}
        DESTINATION include/simCore/LUT)
    install(FILES ${CORE_STRING_HEADERS}
        DESTINATION include/simCore/String)
    install(FILES ${CORE_SYSTEM_HEADERS}
        DESTINATION include/simCore/System)
    install(FILES ${CORE_TIME_HEADERS}
        DESTINATION include/simCore/Time)
endif()
