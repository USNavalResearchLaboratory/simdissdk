if(NOT ENABLE_UNIT_TESTING OR NOT TARGET SIMDIS_SDKVis)
    return()
endif()

project(SimVis_UnitTests)

create_test_sourcelist(SimVisTestFiles SimVisTests.cpp
    FontSizeTest.cpp
    LocatorTest.cpp
)

add_executable(SimVisTests ${SimVisTestFiles})
target_link_libraries(SimVisTests PRIVATE SIMDIS_SDKCore SIMDIS_SDKVis)
set_target_properties(SimVisTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "Unit Tests - SimVis"
)

add_test(NAME LocatorTest COMMAND SimVisTests LocatorTest)
add_test(NAME FontSizeTest COMMAND SimVisTests FontSizeTest)
