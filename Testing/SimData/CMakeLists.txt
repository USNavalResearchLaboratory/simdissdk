# IMPORTANT: if you are getting linker errors, make sure that 
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING OR NOT TARGET simData OR NOT TARGET simUtil)
    return()
endif()

project(SimData_UnitTests)

create_test_sourcelist(SimDataTestFiles SimDataTests.cpp
    CategoryDataTest.cpp
    TestDataLimiting.cpp
    TestInterpolation.cpp
    TestMemoryDataStore.cpp
    TestMemRetrieval.cpp
    TestCommands.cpp
    TestSliceBounds.cpp
    MemoryDataTableTest.cpp
    TestGenericData.cpp
    TestMemorySlice.cpp
    TestMessageVisitor.cpp
    TestListener.cpp
)

add_executable(SimDataTests ${SimDataTestFiles})
target_link_libraries(SimDataTests PRIVATE simData simUtil)
set_target_properties(SimDataTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "Unit Tests - SimData"
)

add_test(NAME simData_TestCategoryData COMMAND SimDataTests CategoryDataTest)
add_test(NAME simData_TestDataLimiting COMMAND SimDataTests TestDataLimiting)
add_test(NAME simData_TestInterpolation COMMAND SimDataTests TestInterpolation)
add_test(NAME simData_TestMemoryDataStore COMMAND SimDataTests TestMemoryDataStore)
add_test(NAME simData_TestMemRetrieval COMMAND SimDataTests TestMemRetrieval)
add_test(NAME simData_TestCommands COMMAND SimDataTests TestCommands)
add_test(NAME simData_TestSliceBounds COMMAND SimDataTests TestSliceBounds)
add_test(NAME simData_MemoryDataTableTest COMMAND SimDataTests MemoryDataTableTest)
add_test(NAME simData_TestGenericData COMMAND SimDataTests TestGenericData)
add_test(NAME simData_TestMemorySlice COMMAND SimDataTests TestMemorySlice)
add_test(NAME simData_TestMessageVisitor COMMAND SimDataTests TestMessageVisitor)
add_test(NAME simData_TestListener COMMAND SimDataTests TestListener)

add_subdirectory(DataStorePerformanceTest)
