# Finds and imports sqlite3 library.

# Search for SQLite library and include path if not in cache
find_library(SQLITE3_LIBRARY_RELEASE_NAME
    NAMES
        sqlite-3.8
        sqlite-3
        sqlite3
    HINTS
        ${SQLITE3_DIR}/lib
        $ENV{SQLITE3_DIR}/lib
        $ENV{SQLITE_DIR}/lib
)
find_library(SQLITE3_LIBRARY_DEBUG_NAME
    NAMES
        sqlite-3.8_d
        sqlite-3.8d
        sqlite-3_d
        sqlite-3d
        sqlite3_d
        sqlite3d
    HINTS
        ${SQLITE3_DIR}/lib
        $ENV{SQLITE3_DIR}/lib
        $ENV{SQLITE_DIR}/lib
)
find_path(SQLITE3_LIBRARY_INCLUDE_PATH
    NAME sqlite/sqlite3.h
    HINTS
        ${SQLITE3_DIR}/include
        $ENV{SQLITE3_DIR}/include
        $ENV{SQLITE_DIR}/include
)

# Configure the link libraries based on OS
if(WIN32)
    set(SQLITE3_LINK_LIBRARIES Wldap32)
else()
    set(SQLITE3_LINK_LIBRARIES rt dl -pthread)
endif()

# Report not found
if(NOT SQLITE3_LIBRARY_RELEASE_NAME)
    set(SQLITE3_FOUND FALSE)
    mark_as_advanced(CLEAR SQLITE3_LIBRARY_RELEASE_NAME SQLITE3_LIBRARY_DEBUG_NAME SQLITE3_LIBRARY_INCLUDE_PATH)
    return()
endif()
mark_as_advanced(FORCE SQLITE3_LIBRARY_RELEASE_NAME SQLITE3_LIBRARY_DEBUG_NAME SQLITE3_LIBRARY_INCLUDE_PATH)
set(SQLITE3_FOUND TRUE)

# Create the target
add_library(SQLITE3 STATIC IMPORTED)
set_target_properties(SQLITE3 PROPERTIES
    IMPORTED_LOCATION "${SQLITE3_LIBRARY_RELEASE_NAME}"
    INTERFACE_INCLUDE_DIRECTORIES "${SQLITE3_LIBRARY_INCLUDE_PATH}"
    INTERFACE_LINK_LIBRARIES "${SQLITE3_LINK_LIBRARIES}"
)
if(SQLITE3_LIBRARY_DEBUG_NAME)
    set_target_properties(SQLITE3 PROPERTIES
        IMPORTED_LOCATION_DEBUG "${SQLITE3_LIBRARY_DEBUG_NAME}"
    )
endif()
