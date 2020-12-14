# IMPORTANT: if you are getting linker errors, make sure that
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING OR NOT TARGET simData OR NOT TARGET simUtil)
    return()
endif()

project(SimData_UnitTests)

set(TEST_FILENAMES
    MemoryDataTableTest.cpp
    TestCommands.cpp
    TestDataLimiting.cpp
    TestFlush.cpp
    TestGenericData.cpp
    TestInterpolation.cpp
    TestListener.cpp
    TestMemoryDataStore.cpp
    TestMemorySlice.cpp
    TestMemRetrieval.cpp
    TestMessageVisitor.cpp
    TestNewUpdatesListener.cpp
    TestSliceBounds.cpp
)

# simQt is used in CategoryDataTest for its Regular Expression implementation
if(TARGET simQt)
    list(APPEND TEST_FILENAMES CategoryDataTest.cpp CategoryRegExpTest.cpp)
endif()

create_test_sourcelist(SimDataTestFiles SimDataTests.cpp ${TEST_FILENAMES})

add_executable(SimDataTests ${SimDataTestFiles})
target_link_libraries(SimDataTests PRIVATE simData simUtil)
set_target_properties(SimDataTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simData Test"
)

# simQt is used in CategoryDataTest for its Regular Expression implementation
if(TARGET simQt)
    target_link_libraries(SimDataTests PRIVATE simQt)
    add_test(NAME simData_TestCategoryData COMMAND SimDataTests CategoryDataTest)
    add_test(NAME simData_TestCategoryRegExp COMMAND SimDataTests CategoryRegExpTest)
endif()

add_test(NAME simData_MemoryDataTableTest COMMAND SimDataTests MemoryDataTableTest)
add_test(NAME simData_TestCommands COMMAND SimDataTests TestCommands)
add_test(NAME simData_TestDataLimiting COMMAND SimDataTests TestDataLimiting)
add_test(NAME simData_TestFlush COMMAND SimDataTests TestFlush)
add_test(NAME simData_TestGenericData COMMAND SimDataTests TestGenericData)
add_test(NAME simData_TestInterpolation COMMAND SimDataTests TestInterpolation)
add_test(NAME simData_TestListener COMMAND SimDataTests TestListener)
add_test(NAME simData_TestMemoryDataStore COMMAND SimDataTests TestMemoryDataStore)
add_test(NAME simData_TestMemorySlice COMMAND SimDataTests TestMemorySlice)
add_test(NAME simData_TestMemRetrieval COMMAND SimDataTests TestMemRetrieval)
add_test(NAME simData_TestMessageVisitor COMMAND SimDataTests TestMessageVisitor)
add_test(NAME simData_TestNewUpdatesListener COMMAND SimDataTests TestNewUpdatesListener)
add_test(NAME simData_TestSliceBounds COMMAND SimDataTests TestSliceBounds)

add_subdirectory(DataStorePerformanceTest)
