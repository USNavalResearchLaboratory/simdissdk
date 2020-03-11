if(NOT ENABLE_UNIT_TESTING OR NOT TARGET simVis)
    return()
endif()

project(SimVis_UnitTests)

create_test_sourcelist(SimVisTestFiles SimVisTests.cpp
    FontSizeTest.cpp
    GogTest.cpp
    LocatorTest.cpp
)

add_executable(SimVisTests ${SimVisTestFiles})
target_link_libraries(SimVisTests PRIVATE simCore simVis)
set_target_properties(SimVisTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simVis Test"
)

add_test(NAME LocatorTest COMMAND SimVisTests LocatorTest)
add_test(NAME FontSizeTest COMMAND SimVisTests FontSizeTest)
add_test(NAME GogTest COMMAND SimVisTests GogTest)
