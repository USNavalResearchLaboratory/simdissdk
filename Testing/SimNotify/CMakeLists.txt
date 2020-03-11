# IMPORTANT: if you are getting linker errors, make sure that 
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING)
    return()
endif()

project(SimNotify_UnitTests)

create_test_sourcelist(SimNotifyTestFiles SimNotifyTests.cpp
    TestNotify.cpp
    NotifyTest.cpp
)

add_executable(SimNotifyTests ${SimNotifyTestFiles} NotifySupport.h NotifySupport.cpp)
target_link_libraries(SimNotifyTests PRIVATE simNotify simCore)
set_target_properties(SimNotifyTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simNotify Test"
)
add_test(NAME TestNotify1 COMMAND SimNotifyTests TestNotify)
add_test(NAME TestNotify2 COMMAND SimNotifyTests NotifyTest)
