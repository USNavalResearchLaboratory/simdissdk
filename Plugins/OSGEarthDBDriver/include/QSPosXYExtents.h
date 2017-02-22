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

#ifndef QS_POSXY_EXTENTS_H
#define QS_POSXY_EXTENTS_H

#ifndef USE_SIMDIS_SDK
#include "inttypesc.h"
#else
#include "simCore/Common/Common.h"
#endif

#include "simCore/Calc/MathConstants.h"
#include "QSCommonIntTypes.h"

#ifdef USE_SIMDIS_SDK
namespace simVis_db
{
#endif
  #if defined Linux || defined Solaris
  static const QsPosType gQsMaxLength = 4294967296LL;
  static const QsPosType gQsHalfMaxLength = 2147483648LL;
  #else
  static const QsPosType gQsMaxLength = 4294967296;
  static const QsPosType gQsHalfMaxLength = 2147483648;
  #endif
  static const double gQsDMaxLength = 4294967296.0;
  static const double gQsDHalfMaxLength = 2147483648.0;
  static const double gQsLatLonDelta = M_PI_2 / gQsDMaxLength;


  /** A bounding rectangle of x/y extents */
  struct PosXPosYExtents
  {
      QsPosType minX;
      QsPosType maxX;
      QsPosType minY;
      QsPosType maxY;

      PosXPosYExtents(QsPosType minX=gQsMaxLength, QsPosType maxX=0, QsPosType minY=gQsMaxLength, QsPosType maxY=0);

      /** Sets up invalid extents */
      void Initialize();

      /** Confirms validity of extents */
      bool Valid() const;

      /** Sets the extents */
      void SetAll(const PosXPosYExtents& given);
      void SetAll(const QsPosType& minX, const QsPosType& maxX, const QsPosType& minY, const QsPosType& maxY);

      /** Packs/unpacks the extents into or from a buffer */
      void Pack(uint8_t*) const;
      void UnPack(const uint8_t*);
      void UnPackHexChars(const char*);

      /** Prints the extents to the console */
      void Print();
  };

  //=====================================================================================
  bool equalTo(const PosXPosYExtents& a, const PosXPosYExtents& b);
  bool operator==(const PosXPosYExtents& a, const PosXPosYExtents& b);
  bool operator!=(const PosXPosYExtents& a, const PosXPosYExtents& b);

  //=====================================================================================
  /** Updates extents such that the given x/y is within the extents */
  void UpdateExtents(const QsPosType& posX, const QsPosType& posY, PosXPosYExtents* extents);

  /** Copies an array of 6 extents */
  bool Copy6Extents(const PosXPosYExtents*, PosXPosYExtents*);

  /** Checks for any overlap between two rectangles */
  bool AnyOverlap(const PosXPosYExtents&, const PosXPosYExtents&);

  /** Checks if the given x/y is within the given extents */
  bool AnyOverlap(const QsPosType& posX, const QsPosType& posY, const PosXPosYExtents&);

#ifdef USE_SIMDIS_SDK
} // Namespace simVis_db
#endif

#endif /* QS_POSXY_EXTENTS_H */
