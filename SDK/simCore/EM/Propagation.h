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

  /// As defined in https://en.wikipedia.org/wiki/Radio_spectrum
  enum FrequencyDesignationUsEcm
  {
    FREQ_ECM_OUT_OF_BOUNDS,
    FREQ_ECM_A,
    FREQ_ECM_B,
    FREQ_ECM_C,
    FREQ_ECM_D,
    FREQ_ECM_E,
    FREQ_ECM_F,
    FREQ_ECM_G,
    FREQ_ECM_H,
    FREQ_ECM_I,
    FREQ_ECM_J,
    FREQ_ECM_K,
    FREQ_ECM_L,
    FREQ_ECM_M
  };

  /**
   * Returns the US ECM frequency band for the given frequency
   * @param freqMhz Transmitter frequency (MHz)
   * @return The frequency band.
   */
  SDKCORE_EXPORT FrequencyDesignationUsEcm toUsEcm(double freqMhz);

  /**
   * Converts a given ECM frequency band to its minimum and maximum frequencies
   * @param[in] usEcm The frequency band
   * @param[out] minFreqMhz Minimum transmitter frequency (MHz)
   * @param[out] maxFreqMhz Maximum transmitter frequency (MHz)
   */
  SDKCORE_EXPORT void getFreqMhzRange(FrequencyDesignationUsEcm usEcm, double* minFreqMhz, double* maxFreqMhz);

} // namespace simCore

#endif  /* SIMCORE_EM_PROPAGATION_H */
