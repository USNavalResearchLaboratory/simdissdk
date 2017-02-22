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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_COMMON_EXPORT_H
#define SIMCORE_COMMON_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
  // Notify
  # if defined( SIMDIS_SDKNotify_LIB_EXPORT_STATIC )
  #  define SDKNOTIFY_EXPORT
  # elif defined( SIMDIS_SDKNotify_LIB_EXPORT_SHARED )
  #  define SDKNOTIFY_EXPORT __declspec(dllexport)
  # else
  #  define SDKNOTIFY_EXPORT __declspec(dllimport)
  # endif
  // Core
  # if defined( SIMDIS_SDKCore_LIB_EXPORT_STATIC )
  #  define SDKCORE_EXPORT
  # elif defined( SIMDIS_SDKCore_LIB_EXPORT_SHARED )
  #  define SDKCORE_EXPORT __declspec(dllexport)
  # else
  #  define SDKCORE_EXPORT __declspec(dllimport)
  # endif
  // Data
  # if defined( SIMDIS_SDKData_LIB_EXPORT_STATIC )
  #  define SDKDATA_EXPORT
  # elif defined( SIMDIS_SDKData_LIB_EXPORT_SHARED )
  #  define SDKDATA_EXPORT __declspec(dllexport)
  # else
  #  define SDKDATA_EXPORT __declspec(dllimport)
  # endif
  // Vis
  # if defined( SIMDIS_SDKVis_LIB_EXPORT_STATIC )
  #  define SDKVIS_EXPORT
  # elif defined( SIMDIS_SDKVis_LIB_EXPORT_SHARED )
  #  define SDKVIS_EXPORT __declspec(dllexport)
  # else
  #  define SDKVIS_EXPORT __declspec(dllimport)
  # endif
  // Util
  # if defined( SIMDIS_SDKUtil_LIB_EXPORT_STATIC )
  #  define SDKUTIL_EXPORT
  # elif defined( SIMDIS_SDKUtil_LIB_EXPORT_SHARED )
  #  define SDKUTIL_EXPORT __declspec(dllexport)
  # else
  #  define SDKUTIL_EXPORT __declspec(dllimport)
  # endif
  // Qt
  # if defined( SIMDIS_SDKQt_LIB_EXPORT_STATIC )
  #  define SDKQT_EXPORT
  # elif defined( SIMDIS_SDKQt_LIB_EXPORT_SHARED )
  #  define SDKQT_EXPORT __declspec(dllexport)
  # else
  #  define SDKQT_EXPORT __declspec(dllimport)
  # endif
#else
  #  define SDKNOTIFY_EXPORT
  #  define SDKCORE_EXPORT
  #  define SDKDATA_EXPORT
  #  define SDKVIS_EXPORT
  #  define SDKUTIL_EXPORT
  #  define SDKQT_EXPORT
#endif

#endif /* SIMCORE_COMMON_EXPORT_H */
