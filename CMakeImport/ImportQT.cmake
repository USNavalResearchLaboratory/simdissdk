# Setup QT library.

# sets up installation preferences for LIBNAME, i.e. Core
function(install_qt_library LIBNAME COMPONENT)
    if(NOT INSTALL_THIRDPARTY_LIBRARIES)
        return()
    endif()
    if(WIN32)
        INSTALL(FILES ${QT_BINARY_DIR}/Qt${LIBNAME}.dll
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
            CONFIGURATIONS Release
            COMPONENT ${COMPONENT}
        )
        INSTALL(FILES ${QT_BINARY_DIR}/Qt${LIBNAME}d.dll
            DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
            CONFIGURATIONS Debug
            COMPONENT ${COMPONENT}
        )
    else()
        INSTALL(FILES ${QT_LIBRARY_DIR}/libQt${LIBNAME}.so.${QT_VERSION_MAJOR}
            DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ${COMPONENT}
        )
        INSTALL(FILES ${QT_LIBRARY_DIR}/libQt${LIBNAME}.so.${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}
            DESTINATION ${INSTALLSETTINGS_LIBRARY_DIR}
            CONFIGURATIONS Release
            COMPONENT ${COMPONENT}
        )
    endif()
endfunction()

# Helper function to install Qt5 plugins to the binary directory
function(install_qt5plugins PLUGINDIR)
    set(EXPECTED_SUBDIR "${_qt5Core_install_prefix}/plugins/${PLUGINDIR}")
    if(NOT IS_DIRECTORY "${EXPECTED_SUBDIR}")
        return()
    endif()
    foreach(COMP ${ARGN})
        if(WIN32)
            INSTALL(DIRECTORY "${EXPECTED_SUBDIR}"
                DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
                COMPONENT ${COMP}
                FILES_MATCHING PATTERN *.dll
                PATTERN *d.dll EXCLUDE)
        else()
            # Note that on Linux the plugins go under the executable's directory, not the lib directory
            INSTALL(DIRECTORY "${EXPECTED_SUBDIR}"
                DESTINATION ${INSTALLSETTINGS_RUNTIME_DIR}
                COMPONENT ${COMP}
                FILES_MATCHING PATTERN *.so)
        endif()
    endforeach()
endfunction()

# Locate qmake, which could make finding the other Qt stuff much easier.
find_program(QT_QMAKE_EXECUTABLE
    NAME qmake
    HINTS
        c:/QtSDK/${BUILD_PLATFORM}_${BUILD_COMPILER}/5.9.8/bin
        $ENV{ProgramFiles}/Qt/bin
        $ENV{QTDIR}/bin
        /usr/local/Qt-5.9.8/bin
        /usr/lib/x86_64-linux-gnu/qt5/bin
        /opt/Qt5/bin
        /usr/local/bin
        /usr/bin
    DOC "Qt5 qmake location, optionally used to find Qt libraries."
)

# Inspect QT_QMAKE_EXECUTABLE if it exists to find the right folder for Qt,
# which will be used by the following call to find_package(Qt5Widgets)
if(QT_QMAKE_EXECUTABLE AND EXISTS "${QT_QMAKE_EXECUTABLE}")
    # Pull out the version -- we're only working with Qt5 at this point
    execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_VERSION
        OUTPUT_VARIABLE QMAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # If QMAKE_VERSION >= 5.0:
    if(QMAKE_VERSION AND NOT QMAKE_VERSION VERSION_LESS 5.0.0)
        # Get the lib folder from qmake
        execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query QT_HOST_LIBS
            OUTPUT_VARIABLE QT_HOST_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        # If the path to CMake files exists
        if(IS_DIRECTORY "${QT_HOST_LIBS}/cmake")
             # Set the CMAKE_PREFIX_PATH and re-attempt finding of widgets
             list(APPEND CMAKE_PREFIX_PATH "${QT_HOST_LIBS}/cmake")
        endif()
    endif()
endif()

# Try to find Qt5
find_package(Qt5Widgets QUIET)

# If Qt5 is found, try to find the OpenGL component for it
if(Qt5Widgets_FOUND)
    find_package(Qt5OpenGL QUIET)

    # We require QGLWidget, which is not provided by some packages
    find_file(QT5_OGL_QGLWidget NAMES QGLWidget
        HINTS ${Qt5OpenGL_INCLUDE_DIRS}
        NO_DEFAULT_PATH)
    set(QGLWIDGET_FOUND FALSE)
    if("${QT5_OGL_QGLWidget}" MATCHES "-NOTFOUND")
        unset(Qt5Widgets_FOUND)
    else()
        set(QGLWIDGET_FOUND TRUE)
    endif()
    unset(QT5_OGL_QGLWidget CACHE)

    # Continue to find Qt5 designer, etc.
    if(QGLWIDGET_FOUND)
        find_package(Qt5Designer QUIET)
        find_package(Qt5UiPlugin QUIET)
        if(Qt5UiPlugin_FOUND OR Qt5Designer_FOUND)
            set(QT_DESIGNER_PLUGIN_DIR "${_qt5Core_install_prefix}/plugins/designer")
        endif()
        set(QT_BINARY_DIR "${_qt5Core_install_prefix}/bin")
        set(QT_LIBRARY_DIR "${_qt5Core_install_prefix}/lib")

        set(QT_FOUND TRUE)
        set(QT5_FOUND TRUE)
        # Pull out the major, minor, and patch versions
        string(REPLACE "." ";" QT_VERSION_LIST ${Qt5Widgets_VERSION})
        list(GET QT_VERSION_LIST 0 QT_VERSION_MAJOR)
        list(GET QT_VERSION_LIST 1 QT_VERSION_MINOR)
        list(GET QT_VERSION_LIST 2 QT_VERSION_PATCH)

        # Install the Qt libraries
        install_qt_library(5Core THIRDPARTY)
        install_qt_library(5Gui THIRDPARTY)
        install_qt_library(5Widgets THIRDPARTY)
        install_qt_library(5OpenGL THIRDPARTY)

        # Copy over the Platforms and Image Formats plug-ins for Qt
        install_qt5plugins(platforms THIRDPARTY)
        install_qt5plugins(imageformats THIRDPARTY)
        # Linux may require XCB GL Integrations plug-in too
        if(NOT WIN32)
            install_qt5plugins(xcbglintegrations THIRDPARTY)
        endif()
        return()
    endif()
endif()

# Fall back to Qt 4.7
find_package(Qt4 4.7)

if(QT_FOUND)
    set(QT_DESIGNER_PLUGIN_DIR "${QT_BINARY_DIR}/../plugins/designer/")
    install_qt_library(Core4 THIRDPARTY)
    install_qt_library(Gui4 THIRDPARTY)
    install_qt_library(OpenGL4 THIRDPARTY)
endif()
