# IMPORTANT: if you are getting linker errors, make sure that 
# "SIMDIS_SDK_LIB_EXPORT_SHARED" is not in your test's Preprocessor Definitions

if(NOT ENABLE_UNIT_TESTING OR NOT TARGET simQt)
    return()
endif()

project(SimQt_UnitTests)

set(SimQtTestsSourceList
    SettingsTest.cpp
    PersistentLoggerTest.cpp
    SegmentedTextsTest.cpp
)

if(TARGET simData)
    list(APPEND SimQtTestsSourceList
        RangeToRegExpTest.cpp
    )
endif()
if(TARGET simVis)
    # QColorTest requires QtConversion.h, which requires osg/Vec4
    list(APPEND SimQtTestsSourceList
        ActionRegistryTest.cpp QColorTest.cpp
    )
    set_source_files_properties(ActionRegistryTest.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
endif()
if(OSG_FOUND)
    list(APPEND SimQtTestsSourceList
        GradientTest.cpp
    )
endif()

create_test_sourcelist(SimQtTestFiles SimQtTests.cpp
    ${SimQtTestsSourceList}
)

add_executable(SimQtTests ${SimQtTestFiles})
target_link_libraries(SimQtTests PRIVATE simQt simCore)
target_include_directories(SimQtTests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(SimQtTests PROPERTIES
    FOLDER "Unit Tests"
    PROJECT_LABEL "simQt Test"
)

VSI_QT_USE_MODULES(SimQtTests LINK_PRIVATE Widgets)

add_test(NAME SettingsTest COMMAND SimQtTests SettingsTest)
add_test(NAME PersistentLoggerTest COMMAND SimQtTests PersistentLoggerTest)
add_test(NAME SegmentedTextsTest COMMAND SimQtTests SegmentedTextsTest)
if(TARGET simData)
    add_test(NAME RangeToRegExpTest COMMAND SimQtTests RangeToRegExpTest)
endif()
if(TARGET simVis)
    add_test(NAME ActionRegistryTest COMMAND SimQtTests ActionRegistryTest)
    add_test(NAME QColorTest COMMAND SimQtTests QColorTest)
    target_link_libraries(SimQtTests PRIVATE simVis)

    # Extract the installation prefix
    set(_QT_INSTALL_PREFIX "${QT6_INSTALL_PREFIX}")
    if(Qt5Core_FOUND)
        set(_QT_INSTALL_PREFIX "${_qt5Gui_install_prefix}")
    endif()

    # Set QT_PLUGIN_PATH, required for Qt 5.14 but not 5.9
    set_tests_properties(
            ActionRegistryTest
        PROPERTIES
            ENVIRONMENT "QT_PLUGIN_PATH=${_QT_INSTALL_PREFIX}/plugins"
)
endif()
if(OSG_FOUND)
    add_test(NAME GradientTest COMMAND SimQtTests GradientTest)
endif()

