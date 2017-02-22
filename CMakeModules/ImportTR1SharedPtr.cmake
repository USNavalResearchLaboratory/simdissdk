if(NOT TESTED_TR1SHAREDPTR)
    try_compile(HAS_TR1SHAREDPTR
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/CMakeModules/ImportTR1SharedPtr_Test.cpp
    )

    if(HAS_TR1SHAREDPTR)
        message(STATUS "Support for tr1::shared_ptr<> detected in <memory>.")
    else(HAS_TR1SHAREDPTR)
        message(STATUS "Relying on on <tr1/memory> for tr1::shared_ptr<>.")
    endif(HAS_TR1SHAREDPTR)
    
    # Prevent testing in the future
    set(TESTED_TR1SHAREDPTR ON CACHE INTERNAL "Indicates that we've tested for tr1::shared_ptr<>" FORCE)
endif(NOT TESTED_TR1SHAREDPTR)
