vsi_require_target(PYTHON3)

set(SIMCORE_FILES
    simCore.i
    simCoreCalc.i
    simCoreCommon.i
    simCoreEM.i
    simCore.i
    simCoreLUT.i
    simCoreString.i
    simCoreTime.i
)

set_source_files_properties(simCore.i PROPERTIES
    CPLUSPLUS ON
    DEPENDS "${SIMCORE_FILES}"
    SWIG_MODULE_NAME "simCore"
)
# SWIG does not define WIN32, _WIN32, _MSC_VER, etc.  Need to provide it manually
if(MSVC)
    set_source_files_properties(simCore.i PROPERTIES SWIG_FLAGS "-DMSVC")
endif()
# Avoid incorrect import declaration with debug libraries
set_source_files_properties(simCore.i PROPERTIES COMPILE_OPTIONS "-interface;_simCore")

# LEGACY / STANDARD
set(UseSWIG_TARGET_NAME_PREFERENCE STANDARD)

swig_add_library(SwigSimCore LANGUAGE python SOURCES simCore.i)
target_link_libraries(SwigSimCore PRIVATE PYTHON3 simCore)
set_target_properties(SwigSimCore PROPERTIES
    FOLDER "SIMDIS SDK"
    PROJECT_LABEL "Python Wrapper - simCore"
    OUTPUT_NAME "simCore"
)

# On Linux do not want the _d suffix on the library
if(NOT WIN32)
    set_target_properties(SwigSimCore PROPERTIES DEBUG_POSTFIX "")
endif()

InstallPythonFiles(simCore)

install(FILES ${SIMCORE_FILES}
    DESTINATION doc/SIMDIS/LabelScripting
    COMPONENT SIMDIS
)
