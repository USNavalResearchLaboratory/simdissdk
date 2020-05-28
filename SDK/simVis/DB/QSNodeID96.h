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
#ifndef SIMVIS_DB_QSNODEID96_H
#define SIMVIS_DB_QSNODEID96_H

#include <bitset>
#include "simCore/Common/Common.h"

namespace simVis_db
{
  class QSNodeID96
  {
  public:
    QSNodeID96();
    explicit QSNodeID96(const uint32_t& value);
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

    int sizeOf() const {return 12;}
    void pack(uint8_t*) const;
    void unpack(const uint8_t*);
    std::string formatAsHex(bool bLeadingZeros=true) const;

  protected:
    uint32_t one_;
    uint32_t two_;
    uint32_t three_;
  };

  //===========================================================================
  typedef QSNodeID96 QSNodeId;

} // namespace simVis_db

#endif /* SIMVIS_DB_QSNODEID96_H */
