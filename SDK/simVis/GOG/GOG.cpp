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
#include "simVis/GOG/GOG.h"
#include "simCore/Calc/Angle.h"

//------------------------------------------------------------------------

// GOG default reference origin: a location off the Pacific Missile Range Facility "BARSTRUR Center"
// (ref SIMDIS user manual, sec. 4.8.1)
const simCore::Coordinate simVis::GOG::BSTUR(
  simCore::COORD_SYS_LLA,
  simCore::Vec3(simCore::DEG2RAD*22.1194392, simCore::DEG2RAD*-159.9194988, 0.0));
