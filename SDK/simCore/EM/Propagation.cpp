/* -*- mode: c++ -*- */
/***************************************************************************
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
#include "simCore/Calc/Math.h"
#include "simCore/EM/Constants.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"

namespace simCore
{

double getRcvdPowerFreeSpace(double rngMeters,
        double freqMhz,
        double powerWatts,
        double xmtGaindB,
        double rcvGaindB,
        double rcsSqm,
        double systemLossdB,
        bool oneWay)
{
  // Free Space Radar range equation
  double rcvPower = simCore::SMALL_DB_VAL;
  double lamdaSqrd = square(simCore::LIGHT_SPEED_AIR / (1e6 * freqMhz));
  if (oneWay == false)
  {
    // http://www.microwaves101.com/encyclopedia/Navy_Handbook.cfm  Section 4.4
    rcvPower = xmtGaindB + rcvGaindB - systemLossdB + simCore::linear2dB((rcsSqm * powerWatts * lamdaSqrd) / (simCore::RRE_CONSTANT * square(square(rngMeters))));
  }
  else
  {
    // http://www.microwaves101.com/encyclopedia/Navy_Handbook.cfm  Section 4.3
    rcvPower = xmtGaindB + rcvGaindB - systemLossdB + simCore::linear2dB((powerWatts * lamdaSqrd) / (square(4. * M_PI * rngMeters)));
  }
  return rcvPower;
}

double getRcvdPowerBlake(double rngMeters,
        double freqMhz,
        double powerWatts,
        double xmtGaindB,
        double rcvGaindB,
        double rcsSqm,
        double ppfdB,
        double systemLossdB,
        bool oneWay)
{
  double rcvPower = getRcvdPowerFreeSpace(rngMeters, freqMhz, powerWatts, xmtGaindB, rcvGaindB, rcsSqm, systemLossdB, oneWay);
  // Received signal power calculation from Blake's equation 1.18 (p 12)
  // Radar Range-Performance Analysis (1986)
  // Lamont V. Blake, ISBN 0-89006-224-2
  // Use free space value, then apply propagation factor
  return (oneWay == false) ? (rcvPower + (4. * ppfdB)) : (rcvPower + (2. * ppfdB));
}

}

