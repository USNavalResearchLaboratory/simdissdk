if(NOT ENABLE_UNIT_TESTING OR NOT TARGET simUtil)
    return()
endif()

project(SimUtil_UnitTests)

create_test_sourcelist(SimUtilTestFiles SimUtilTests.cpp
    IdMapperTest.cpp
    UnitTypeConverterTest.cpp
)

add_executable(SimUtilTests ${SimUtilTestFiles})
target_link_libraries(SimUtilTests PRIVATE simUtil)
set_target_properties(SimUtilTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simUtil Test"
)

add_test(NAME IdMapperTest COMMAND SimUtilTests IdMapperTest)
