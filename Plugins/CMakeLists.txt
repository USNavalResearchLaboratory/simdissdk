if(SQLITE3_LIBRARY_RELEASE_NAME)
    add_subdirectory(OSGEarthDBDriver)
else()
    message(WARNING "osgEarth .db Driver plug-in omitted because of missing SQLite dependency.")
endif()

if(QT_FOUND)
    option(ENABLE_QTDESIGNER_WIDGETS "Build Qt Designer Plugins for simQt widgets" ON)
    if(ENABLE_QTDESIGNER_WIDGETS)
        add_subdirectory(QtDesignerWidgets)
    endif(ENABLE_QTDESIGNER_WIDGETS)
endif(QT_FOUND)


