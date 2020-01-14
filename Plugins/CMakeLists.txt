# DB plugin no longer needed with osgEarth 3.x
#if(SQLITE3_LIBRARY_RELEASE_NAME)
#    add_subdirectory(OSGEarthDBDriver)
#else()
#    message(STATUS "Skipping osgEarth .db Driver plug-in because of missing dependencies.")
#endif()

if(QT_FOUND)
    option(ENABLE_QTDESIGNER_WIDGETS "Build Qt Designer Plugins for simQt widgets" ON)
    if(ENABLE_QTDESIGNER_WIDGETS)
        add_subdirectory(QtDesignerWidgets)
    endif()
endif()


