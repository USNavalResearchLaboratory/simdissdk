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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_GOG_GOG_H
#define SIMVIS_GOG_GOG_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"

namespace simVis { namespace GOG
{
  /**
   * Default reference point for relative GOG data.
   */
  extern SDKVIS_EXPORT const simCore::Coordinate BSTUR;

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_GOG_H
