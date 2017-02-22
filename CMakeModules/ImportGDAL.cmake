# Finds and imports GDAL library.

# We only care about GDAL if it's packaged as a public or VSI default, and
# we only care then so that we can install it to the distribution lib folder
set(GDAL_FOUND FALSE)
if(NOT GDAL_DIR)
    return()
endif()

# Find path from CMake module
find_library(GDAL_LIBRARY
    NAMES ${_gdal_lib} gdal gdal_i gdal201 gdal111 gdal1.5.0 gdal1.4.0 gdal1.3.2 GDAL
    HINTS
        ENV GDAL_DIR
        ENV GDAL_ROOT
        "${GDAL_DIR}"
        ${_gdal_libpath}
    PATH_SUFFIXES lib
    PATHS
        /sw
        /opt/local
        /opt/csw
        /opt
        /usr/freeware
)

if(NOT GDAL_LIBRARY)
    set(GDAL_FOUND FALSE)
    mark_as_advanced(CLEAR GDAL_DIR GDAL_LIBRARY GDAL_LIBRARY_DEBUG)
    return()
endif()
set(GDAL_FOUND TRUE)

# Try to find a debug library
if(NOT GDAL_LIBRARY_DEBUG)
    get_filename_component(_GDAL_DIR "${GDAL_LIBRARY}" DIRECTORY)
    get_filename_component(_GDAL_WE "${GDAL_LIBRARY}" NAME_WE)
    set(LIB_EXTENSION lib)
    if(NOT WIN32)
        set(LIB_EXTENSION a)
    endif()
    if(EXISTS "${_GDAL_DIR}/${_GDAL_WE}d.${LIB_EXTENSION}")
        set(GDAL_LIBRARY_DEBUG "${_GDAL_DIR}/${_GDAL_WE}d.${LIB_EXTENSION}" CACHE FILEPATH "Debug GDAL library")
    elseif(EXISTS "${_GDAL_DIR}/${_GDAL_WE}_d.${LIB_EXTENSION}")
        set(GDAL_LIBRARY_DEBUG "${_GDAL_DIR}/${_GDAL_WE}_d.${LIB_EXTENSION}" CACHE FILEPATH "Debug GDAL library")
    endif()
endif()
mark_as_advanced(FORCE GDAL_DIR GDAL_LIBRARY GDAL_LIBRARY_DEBUG)

# Create the library target
add_library(GDAL SHARED IMPORTED)
set_target_properties(GDAL PROPERTIES
    IMPORTED_IMPLIB "${GDAL_LIBRARY}"
)
if(GDAL_LIBRARY_DEBUG)
    set_target_properties(GDAL PROPERTIES IMPORTED_IMPLIB_DEBUG "${GDAL_LIBRARY_DEBUG}")
endif()
vsi_set_imported_locations_from_implibs(GDAL)

if(INSTALL_THIRDPARTY_LIBRARIES)
    vsi_install_target(GDAL THIRDPARTY)
endif()
