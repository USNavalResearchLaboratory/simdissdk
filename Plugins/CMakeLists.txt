if(Qt5Core_FOUND OR Qt6Core_FOUND)
    option(ENABLE_QTDESIGNER_WIDGETS "Build Qt Designer Plugins for simQt widgets" ON)
    if(ENABLE_QTDESIGNER_WIDGETS)
        add_subdirectory(QtDesignerWidgets)
    endif()
endif()

add_subdirectory(wbp)
