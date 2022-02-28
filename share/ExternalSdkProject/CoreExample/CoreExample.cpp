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
#include <iostream>
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Angle.h"

// LLA posits from http://dateandtime.info
static const simCore::Vec3 DC_LLA(38.89511 * simCore::DEG2RAD, -77.03637 * simCore::DEG2RAD, 0.0);
static const simCore::Vec3 BALTIMORE_LLA(39.29038 * simCore::DEG2RAD, -76.61219 * simCore::DEG2RAD, 0.0);

int main(int argc, char* argv[])
{
  // Note that the WGS_84 earth model does not require a CoordConvert
  // instance to calculate ground distance, so we pass in nullptr
  const double distanceKm = simCore::calculateGroundDist(DC_LLA, BALTIMORE_LLA, simCore::WGS_84, nullptr) / 1000.0;

  // Calculate the azimuth (true) as well between the two posits
  double azimuth = 0.0;
  simCore::calculateAbsAzEl(DC_LLA, BALTIMORE_LLA, &azimuth, nullptr, nullptr, simCore::WGS_84, nullptr);
  azimuth *= simCore::RAD2DEG;

  // Report the value on console
  std::cout << "Washington, DC to Baltimore, MD:\n"
    << "  Distance:  " << distanceKm << " km\n"
    << "  Direction: " << azimuth << " T" << std::endl;
  return 0;
}
