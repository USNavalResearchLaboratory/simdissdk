/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_COMMON_COMMON_H
#define SIMCORE_COMMON_COMMON_H

#if defined(_MSC_VER)
  // 4251: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
  // This shows up all over.  When a class is not declared with SDKxxxx_EXPORT and it is used in a class
  // that is, even in a private manner, this warning shows up.  This carries over even to STL; std::vector
  // gets this warning when used in a class declared as SDKxxxx_EXPORT.
  #pragma warning(disable : 4251)
#endif

#include "simCore/Common/Export.h"


// Define helper macros for declaring methods as deprecated
#ifdef WIN32
#define SDK_DEPRECATED_PRE(_TEXT) __declspec(deprecated(_TEXT))
#define SDK_DEPRECATED_POST(_TEXT)
#else
#define SDK_DEPRECATED_PRE(_TEXT)
#define SDK_DEPRECATED_POST(_TEXT) __attribute__((deprecated))
#endif

/**
 * Defines a convenient macro for marking a method or function as deprecated.
 * The macro will use the compiler-specific tags to mark a method deprecated, so
 * that if you call it in your code, you get a warning in your compile log.
 */
#define SDK_DEPRECATE(METHOD, TEXT) SDK_DEPRECATED_PRE(TEXT) METHOD SDK_DEPRECATED_POST(TEXT)


#ifdef WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  // Make sure no macro for "min" and "max"
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include "inttypes.h"
#else
  #include <inttypes.h>
#endif

#endif /* SIMCORE_COMMON_COMMON_H */
