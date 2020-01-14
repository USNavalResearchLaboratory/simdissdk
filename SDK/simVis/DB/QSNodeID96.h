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

#ifndef QSNODEID96_H
#define QSNODEID96_H

#include <bitset>
#include "simCore/Common/Common.h"

namespace simVis_db
{
  class QSNodeID96
  {
  public:
    QSNodeID96();
    QSNodeID96(const uint32_t& value);
    ~QSNodeID96();

    bool operator==(const QSNodeID96& value) const;
    bool operator<(const QSNodeID96& value) const;
    QSNodeID96& operator=(const QSNodeID96& value);
    QSNodeID96 operator~() const;
    QSNodeID96& operator|=(const QSNodeID96& value);
    QSNodeID96& operator&=(const QSNodeID96& value);
    QSNodeID96 operator>>(int numBitsToShift) const;
    QSNodeID96 operator<<(int numBitsToShift) const;
    QSNodeID96 operator&(const QSNodeID96& value) const;

    int SizeOf() const {return 12;}
    void Pack(uint8_t*) const;
    void UnPack(const uint8_t*);
    std::string FormatAsHex(bool bLeadingZeros=true) const;

  protected:
    uint32_t one_;
    uint32_t two_;
    uint32_t three_;
  };

  //===========================================================================
  typedef QSNodeID96 QSNodeId;

} // namespace simVis_db

#endif /* QSNODEID96_H */
