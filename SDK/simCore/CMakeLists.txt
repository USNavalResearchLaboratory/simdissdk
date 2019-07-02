# ----- simCore/Common -------------------------------------------------------

set(CORE_COMMON_INC Common/)
set(CORE_COMMON_HEADERS
    ${CORE_COMMON_INC}inttypes.h
    ${CORE_COMMON_INC}stdint.h
    ${CORE_COMMON_INC}Common.h
    ${CORE_COMMON_INC}Exception.h
    ${CORE_COMMON_INC}Export.h
    ${CORE_COMMON_INC}FileSearch.h
    ${CORE_COMMON_INC}HighPerformanceGraphics.h
    ${CORE_COMMON_INC}Time.h
    ${CORE_COMMON_INC}SDKAssert.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/simCore/Common/Version.h
)
set(CORE_COMMON_SRC Common/)
set(CORE_COMMON_SOURCES
    ${CORE_COMMON_SRC}Version.cpp
)
source_group(Headers\\Common FILES ${CORE_COMMON_HEADERS})
source_group(Sources\\Common FILES ${CORE_COMMON_SOURCES})

# ----- simCore/Calc --------------------------------------------------------

set(CORE_CALC_INC Calc/)
set(CORE_CALC_HEADERS
    ${CORE_CALC_INC}Angle.h
    ${CORE_CALC_INC}Calculations.h
    ${CORE_CALC_INC}Coordinate.h
    ${CORE_CALC_INC}CoordinateConverter.h
    ${CORE_CALC_INC}CoordinateSystem.h
    ${CORE_CALC_INC}DatumConvert.h
    ${CORE_CALC_INC}Gars.h
    ${CORE_CALC_INC}Geometry.h
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
    ${CORE_CALC_SRC}DatumConvert.cpp
    ${CORE_CALC_SRC}Gars.cpp
    ${CORE_CALC_SRC}Geometry.cpp
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
source_group(Sources\\Calc FILES ${CORE_CALC_SOURCES})

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
source_group(Sources\\EM FILES ${CORE_EM_SOURCES})

# ----- simCore/String --------------------------------------------------------

set(CORE_STRING_INC String/)
set(CORE_STRING_SRC String/)
set(CORE_STRING_HEADERS
    ${CORE_STRING_INC}Angle.h
    ${CORE_STRING_INC}Constants.h
    ${CORE_STRING_INC}FilePatterns.h
    ${CORE_STRING_INC}Tokenizer.h
    ${CORE_STRING_INC}Format.h
    ${CORE_STRING_INC}TextFormatter.h
    ${CORE_STRING_INC}TextReplacer.h
    ${CORE_STRING_INC}UnitContextFormatter.h
    ${CORE_STRING_INC}Utils.h
    ${CORE_STRING_INC}ValidNumber.h
)
set(CORE_STRING_SOURCES
    ${CORE_STRING_SRC}Angle.cpp
    ${CORE_STRING_SRC}Format.cpp
    ${CORE_STRING_SRC}Tokenizer.cpp
    ${CORE_STRING_SRC}TextFormatter.cpp
    ${CORE_STRING_SRC}TextReplacer.cpp
    ${CORE_STRING_SRC}UnitContextFormatter.cpp
    ${CORE_STRING_SRC}Utils.cpp
    ${CORE_STRING_SRC}ValidNumber.cpp
)
source_group(Headers\\String FILES ${CORE_STRING_HEADERS})
source_group(Sources\\String FILES ${CORE_STRING_SOURCES})

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
source_group(Sources\\Time FILES ${CORE_TIME_SOURCES})

# ----- simCore.h --------------------------------------------------------

set(CORE_ALL_HEADER
    ../simCore.h
)
source_group("" FILES ${CORE_ALL_HEADER})

# ----------------------------------------------------------------------

set(CORE_PROJECT_FILES
    ${CORE_COMMON_HEADERS} ${CORE_COMMON_SOURCES}
    ${CORE_CALC_HEADERS} ${CORE_CALC_SOURCES}
    ${CORE_CALC_LUT_HEADERS}
    ${CORE_EM_HEADERS} ${CORE_EM_SOURCES}
    ${CORE_TIME_HEADERS} ${CORE_TIME_SOURCES}
    ${CORE_STRING_HEADERS} ${CORE_STRING_SOURCES}
    ${CORE_ALL_HEADER}
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
    PROJECT_LABEL "SIMDIS SDK - Core"
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

# Determine whether time.h includes timespec
include(TestTimespec)
test_timespec(SDK_TIMESPEC)
if(SDK_TIMESPEC)
    target_compile_definitions(simCore PUBLIC DEFINED_TIMESPEC_T)
endif()

if(INSTALL_SIMDIS_SDK_LIBRARIES)
    vsi_install_export(simCore ${SIMDIS_SDK_VERSION_STRING} AnyNewerVersion)
endif()

if(AUTOINSTALL_LIBS AND SIMCORE_SHARED)
    PostBuildInstallSdkSharedObjects(simCore)
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
    install(FILES ${CORE_ALL_HEADER}
        DESTINATION include)
endif()
