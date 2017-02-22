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
#ifndef SIMCORE_EM_PROPAGATION_H
#define SIMCORE_EM_PROPAGATION_H

#include "simCore/Common/Common.h"

namespace simCore
{
  /**
  * This function returns the received power (dB) at the antenna using the free space received signal power calculation
  * @param rngMeters Range from radar to target (m)
  * @param freqMhz Transmitter frequency (MHz)
  * @param powerWatts Transmitter peak power (Watts)
  * @param xmtGaindB Xmt antenna gain (dB)
  * @param rcvGaindB Rcv antenna gain (dB)
  * @param rcsSqm Target radar cross section (sqm)
  * @param systemLossdB Total system loss (dB)
  * @param oneWay calculates the one way power (dB) at an isotropic antenna
  * @return Free space received power (dB).
  */
  SDKCORE_EXPORT double getRcvdPowerFreeSpace(double rngMeters,
          double freqMhz,
          double powerWatts,
          double xmtGaindB,
          double rcvGaindB,
          double rcsSqm,
          double systemLossdB,
          bool oneWay=false);

  /**
  * This function returns the received power (dB) at the antenna using the received signal power calculation from Blake's equation 1.18 (p 12) Radar Range-Performance Analysis (1986) Lamont V. Blake, ISBN 0-89006-224-2
  * @param rngMeters Range from radar to target (m)
  * @param freqMhz Transmitter frequency (MHz)
  * @param powerWatts Transmitter peak power (Watts)
  * @param xmtGaindB Xmt antenna gain (dB)
  * @param rcvGaindB Rcv antenna gain (dB)
  * @param rcsSqm Target radar cross section (sqm)
  * @param ppfdB Pattern propagation factor (dB)
  * @param systemLossdB Total system loss (dB)
  * @param oneWay calculates the one way power (dB) at an isotropic antenna
  * @return Received power at radar antenna (dB).
  */
  SDKCORE_EXPORT double getRcvdPowerBlake(double rngMeters,
          double freqMhz,
          double powerWatts,
          double xmtGaindB,
          double rcvGaindB,
          double rcsSqm,
          double ppfdB,
          double systemLossdB,
          bool oneWay=false);
} // namespace simCore

#endif  /* SIMCORE_EM_PROPAGATION_H */
