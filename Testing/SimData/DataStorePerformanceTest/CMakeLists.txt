# IMPORTANT: if you are getting linker errors, make sure that 
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING)
    return()
endif()

project(SimData_DataStorePerformanceTest)

add_executable(DataStorePerformanceTest DataStorePerformanceTest.cpp)
target_link_libraries(DataStorePerformanceTest PRIVATE simData simUtil)
set_target_properties(DataStorePerformanceTest PROPERTIES
    FOLDER "Performance Tests"
    PROJECT_LABEL "Performance Tests - DataStore"
)
