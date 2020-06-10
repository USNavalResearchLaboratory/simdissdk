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
#ifndef SIMDATA_MEMORYTABLE_DATALIMITSPROVIDER_H
#define SIMDATA_MEMORYTABLE_DATALIMITSPROVIDER_H

#include "simData/DataTable.h"

namespace simData { namespace MemoryTable {

/** Provides data limits based on the table */
class DataLimitsProvider
{
public:
  virtual ~DataLimitsProvider() {}

  /**
   * Retrieves the data limits for a given table in points and seconds.
   * @param table Table for which to retrieve the data limits.
   * @param[out] pointsLimit Maximum number of points to store; 0 for no limit
   * @param[out] secondsLimit Maximum number of seconds to store; 0 for no limit
   * @return Table status structure indicating success or error; error indicates no limiting should be applied
   *    and implies that the values in pointsLimit and secondsLimit may be uninitialized/unchanged.
   */
  virtual TableStatus getLimits(const simData::DataTable& table, size_t& pointsLimit, double& secondsLimit) const = 0;
};

} }

#endif /* SIMDATA_MEMORYTABLE_DATALIMITSPROVIDER_H */
