# IMPORTANT: if you are getting linker errors, make sure that 
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING OR NOT TARGET SIMDIS_SDKQt)
    return()
endif()

project(SimQt_UnitTests)

create_test_sourcelist(SimQtTestFiles SimQtTests.cpp
    ActionRegistryTest.cpp
    QColorTest.cpp
    SettingsTest.cpp
    PersistentLoggerTest.cpp
)

VSI_INCLUDE_QT_USE_FILE()

add_executable(SimQtTests ${SimQtTestFiles})
target_link_libraries(SimQtTests PRIVATE SIMDIS_SDKQt SIMDIS_SDKCore)
target_include_directories(SimQtTests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(SimQtTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "Unit Tests - SimQt"
)

VSI_QT_USE_MODULES(SimQtTests LINK_PRIVATE Widgets)

add_test(NAME ActionRegistryTest COMMAND SimQtTests ActionRegistryTest)
add_test(NAME QColorTest COMMAND SimQtTests QColorTest)
add_test(NAME SettingsTest COMMAND SimQtTests SettingsTest)
add_test(NAME PersistentLoggerTest COMMAND SimQtTests PersistentLoggerTest)
