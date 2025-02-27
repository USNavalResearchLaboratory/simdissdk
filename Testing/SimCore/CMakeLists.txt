# IMPORTANT: if you are getting linker errors, make sure that
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING)
    return()
endif()

project(SimCore_UnitTests)

create_test_sourcelist(SimCoreTestFiles SimCoreTests.cpp
    AngleTest.cpp
    CalculateLibTest.cpp
    CalculationTest.cpp
    CoordConvertLibTest.cpp
    CoreCommonTest.cpp
    CsvReaderTest.cpp
    CsvWriterTest.cpp
    EMTest.cpp
    FileTest.cpp
    GarsTest.cpp
    GeoFenceTest.cpp
    GeometryTest.cpp
    GogTest.cpp
    GogToGeoFenceTest.cpp
    InterpolationTest.cpp
    LutTest.cpp
    MagneticVarianceTest.cpp
    MathTest.cpp
    MgrsTest.cpp
    MultiFrameCoordTest.cpp
    SquareMatrixTest.cpp
    StringFormatTest.cpp
    StringUtilsTest.cpp
    TimeClassTest.cpp
    TimeManagerTest.cpp
    TimeStringTest.cpp
    TimeUtilsTest.cpp
    TimeJulianTest.cpp
    TokenizerTest.cpp
    ValidNumberTest.cpp
    VersionTest.cpp
    UnitsTest.cpp
    UnitsFormatter.cpp
    XmlWriterTest.cpp
)

add_executable(SimCoreTests ${SimCoreTestFiles})
find_package(Filesystem QUIET)
target_link_libraries(SimCoreTests PRIVATE simCore std::filesystem)
# If simUtil is built, then add it as a dependency for UnitsFormatter test and add a -D define
if(TARGET simUtil)
    target_link_libraries(SimCoreTests PRIVATE simUtil)
    target_compile_definitions(SimCoreTests PRIVATE HAVE_SIMUTIL)
endif()

# Suppress warning for TimeStamp methods
if(UNIX)
	target_compile_options(SimCoreTests PRIVATE "-Wno-deprecated-declarations")
else()
	target_compile_options(SimCoreTests PRIVATE "-wd4996")
endif()

set_target_properties(SimCoreTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simCore Test"
)

add_test(NAME VersionTest COMMAND SimCoreTests VersionTest)
add_test(NAME TimeManager COMMAND SimCoreTests TimeManagerTest)
add_test(NAME TokenizerTest COMMAND SimCoreTests TokenizerTest)
add_test(NAME StringUtilsTest COMMAND SimCoreTests StringUtilsTest)
add_test(NAME CoreStringFormatTest COMMAND SimCoreTests StringFormatTest)
add_test(NAME CoordConvertLibTest COMMAND SimCoreTests CoordConvertLibTest)
add_test(NAME CoreCommonTest COMMAND SimCoreTests CoreCommonTest)
add_test(NAME CalculationTest COMMAND SimCoreTests CalculationTest)
add_test(NAME CoreMathTest COMMAND SimCoreTests MathTest)
add_test(NAME CoreSquareMatrixTest COMMAND SimCoreTests SquareMatrixTest)
add_test(NAME CoreGarsTest COMMAND SimCoreTests GarsTest)
add_test(NAME CoreMgrsTest COMMAND SimCoreTests MgrsTest)
add_test(NAME CoreTimeClassTest COMMAND SimCoreTests TimeClassTest)
add_test(NAME CoreInterpolationTest COMMAND SimCoreTests InterpolationTest)
add_test(NAME CoreLutTest COMMAND SimCoreTests LutTest)
add_test(NAME CoreValidNumberTest COMMAND SimCoreTests ValidNumberTest)
add_test(NAME CoreTimeStringTest COMMAND SimCoreTests TimeStringTest)
add_test(NAME CoreTimeUtilsTest COMMAND SimCoreTests TimeUtilsTest)
add_test(NAME CoreTimeJulianTest COMMAND SimCoreTests TimeJulianTest)
add_test(NAME CoreGeoFenceTest COMMAND SimCoreTests GeoFenceTest)
add_test(NAME CoreGeometryTest COMMAND SimCoreTests GeometryTest)
add_test(NAME MultiFrameCoordTest COMMAND SimCoreTests MultiFrameCoordTest)
add_test(NAME AngleTest COMMAND SimCoreTests AngleTest)
add_test(NAME CoreUnitsTest COMMAND SimCoreTests UnitsTest)
add_test(NAME CoreUnitsFormatter COMMAND SimCoreTests UnitsFormatter)
add_test(NAME GogToGeoFenceTest COMMAND SimCoreTests GogToGeoFenceTest)
add_test(NAME CalculateLibTest COMMAND SimCoreTests CalculateLibTest ${SimCore_UnitTests_SOURCE_DIR}/CalculateInput.txt)
add_test(NAME MagneticVarianceTest COMMAND SimCoreTests MagneticVarianceTest)
add_test(NAME CsvReaderTest COMMAND SimCoreTests CsvReaderTest)
add_test(NAME CsvWriterTest COMMAND SimCoreTests CsvWriterTest)
add_test(NAME SimCoreGogTest COMMAND SimCoreTests GogTest)
add_test(NAME XmlWriterTest COMMAND SimCoreTests XmlWriterTest)
add_test(NAME SimCoreFileTest COMMAND SimCoreTests FileTest)

# Try to locate the correct file for the RCS test...
set(FILE_LOCATIONS
    $ENV{SIMDIS_SDK_FILE_PATH}/data/rcs/
    ${INSTALLSOURCE_DATA_DIR}/TestData/UpdateData/
)
find_file(RCSFILE fake_rcs_3.rcs PATHS ${FILE_LOCATIONS})
mark_as_advanced(RCSFILE)

# Try to locate the correct path for the Antenna Pattern test files
set(FILE_LOCATIONS
    $ENV{SIMDIS_SDK_FILE_PATH}/data/ant/
    ${INSTALLSOURCE_DATA_DIR}/TestData/AntPat/
)
find_path(ANT_PATH monopulse.apmf PATHS ${FILE_LOCATIONS})
mark_as_advanced(ANT_PATH)

if(EXISTS ${RCSFILE})
    add_test(NAME CoreEMTest COMMAND SimCoreTests EMTest ${RCSFILE} ${ANT_PATH})
endif()
