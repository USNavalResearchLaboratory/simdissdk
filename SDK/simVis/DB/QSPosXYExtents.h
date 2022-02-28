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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef SIMVIS_DB_POSXYEXTENTS_H
#define SIMVIS_DB_POSXYEXTENTS_H

#include "simCore/Common/Common.h"

namespace simVis_db
{
  typedef uint64_t QsPosType;

#ifndef WIN32
  static const QsPosType QS_MAX_LENGTH_UINT64 = 4294967296LL;
#else
  static const QsPosType QS_MAX_LENGTH_UINT64 = 4294967296;
#endif
  static const double QS_MAX_LENGTH_DOUBLE = 4294967296.0;

  /** A bounding rectangle of x/y extents */
  struct PosXPosYExtents
  {
      QsPosType minX;
      QsPosType maxX;
      QsPosType minY;
      QsPosType maxY;

      PosXPosYExtents(QsPosType minX=QS_MAX_LENGTH_UINT64, QsPosType maxX=0, QsPosType minY=QS_MAX_LENGTH_UINT64, QsPosType maxY=0);

      /** Sets up invalid extents */
      void initialize();

      /** Confirms validity of extents */
      bool isValid() const;

      /** Sets the extents */
      void setAll(const PosXPosYExtents& given);
      void setAll(const QsPosType& minX, const QsPosType& maxX, const QsPosType& minY, const QsPosType& maxY);

      /** Packs/unpacks the extents into or from a buffer */
      void pack(uint8_t*) const;
      void unpack(const uint8_t*);
  };

  //=====================================================================================
  bool equalTo(const PosXPosYExtents& a, const PosXPosYExtents& b);
  bool operator==(const PosXPosYExtents& a, const PosXPosYExtents& b);
  bool operator!=(const PosXPosYExtents& a, const PosXPosYExtents& b);

} // Namespace simVis_db

#endif /* SIMVIS_DB_POSXYEXTENTS_H */
