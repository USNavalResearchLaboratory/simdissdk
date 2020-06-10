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
#ifndef SIMDATA_OBJECTID_H
#define SIMDATA_OBJECTID_H

#include "simCore/Common/Common.h"

namespace simData
{
/// uniquely identify an object
typedef uint64_t ObjectId;

/// Bitmask representing the types of objects that are assigned ObjectIds
enum ObjectType
{
  NONE      = 0x00,
  PLATFORM  = 0x01,
  BEAM      = 0x02,
  GATE      = 0x04,
  LASER     = 0x08,
  PROJECTOR = 0x10,
  LOB_GROUP = 0x20,
  CUSTOM_RENDERING    = 0x40,
  ALL = (PLATFORM | BEAM | GATE | LASER | PROJECTOR | LOB_GROUP | CUSTOM_RENDERING)
};

} // End of namespace simData

#endif // SIMDATA_OBJECTID_H

