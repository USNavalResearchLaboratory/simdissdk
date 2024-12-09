# Setup EnTT library
# Setting the ENTT_DIR environment variable will allow use of a custom built library

set(ENTT_SUBDIR 3.14.0)

# Setup search paths
initialize_ENV(ENTT_DIR)
set(ENTT_ROOT_DIRS
    $ENV{ENTT_DIR}
    ${THIRD_DIR}/EnTT/${ENTT_SUBDIR}
)
find_path(EnTT_ROOT NAME include/entt/entt.hpp PATHS ${ENTT_ROOT_DIRS} NO_DEFAULT_PATH)

# Creates target EnTT::EnTT
find_package(EnTT QUIET)
mark_as_advanced(EnTT_ROOT)
