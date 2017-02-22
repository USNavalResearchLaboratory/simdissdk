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
#ifndef QS_COMMON_INT_TYPES_H
#define QS_COMMON_INT_TYPES_H

#ifndef USE_SIMDIS_SDK
#include "inttypesc.h"
#else
#include "simCore/Common/Common.h"

namespace simVis_db
{
#endif
  typedef int16_t LevelInt;
  static const LevelInt QT_MIN_LEVEL = 0;
  static const LevelInt QT_MAX_LEVEL = 32;

  typedef uint8_t ChildIndexInt;
  static const ChildIndexInt QT_CHILD_NE = 0;
  static const ChildIndexInt QT_CHILD_NW = 1;
  static const ChildIndexInt QT_CHILD_SW = 2;
  static const ChildIndexInt QT_CHILD_SE = 3;

  typedef uint8_t FaceIndexType;
  static const FaceIndexType QsFaceIndexWW = 0;
  static const FaceIndexType QsFaceIndexW  = 1;
  static const FaceIndexType QsFaceIndexE  = 2;
  static const FaceIndexType QsFaceIndexEE = 3;
  static const FaceIndexType QsFaceIndexN  = 4;
  static const FaceIndexType QsFaceIndexS  = 5;

  typedef float AltitudeDataType;

  static const int16_t MAX_NUM_READ_THREADS = 128;

  typedef uint64_t QsPosType;

#ifdef USE_SIMDIS_SDK
} // namespace simVis_db
#endif

#endif /* QS_COMMON_INT_TYPES_H */
