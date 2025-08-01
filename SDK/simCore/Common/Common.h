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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
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


/** Disable copy constructor and copy assignment, on class "K" */
#define SDK_DISABLE_COPY(K) \
  K(const K&) = delete; \
  K& operator=(const K&) = delete;
/** Disable move constructor and move assignment, on class "K" */
#define SDK_DISABLE_MOVE(K) \
  K(K&&) = delete; \
  K& operator=(K&&) = delete;
/** Disable copy and move constructor, and copy and move assignment, on class "K" */
#define SDK_DISABLE_COPY_MOVE(K) \
  SDK_DISABLE_COPY(K) \
  SDK_DISABLE_MOVE(K)


#ifdef WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  // Make sure no macro for "min" and "max"
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
#endif

#include <stdint.h>

#endif /* SIMCORE_COMMON_COMMON_H */
