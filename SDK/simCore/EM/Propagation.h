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
  * @param rngMeters Range from radar to target (m), must be non-zero
  * @param freqMhz Transmitter frequency (MHz), must be non-zero
  * @param powerWatts Transmitter peak power (Watts)
  * @param xmtGaindB Xmt antenna gain (dB)
  * @param rcvGaindB Rcv antenna gain (dB)
  * @param rcsSqm Target radar cross section (sqm)
  * @param systemLossdB Total system loss (dB)
  * @param oneWay calculates the one way power (dB) at an isotropic antenna
  * @return Free space received power (dB).
  */
  SDKCORE_EXPORT double getRcvdPowerFreeSpace(double rngMeters, double freqMhz, double powerWatts, double xmtGaindB, double rcvGaindB, double rcsSqm, double systemLossdB, bool oneWay=false);

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
  SDKCORE_EXPORT double getRcvdPowerBlake(double rngMeters, double freqMhz, double powerWatts, double xmtGaindB, double rcvGaindB, double rcsSqm, double ppfdB, double systemLossdB, bool oneWay=false);

  /**
  * This function returns the free space detection range (m) for an ESM receiver as well as an optional free space path loss (dB)
  * @param xmtGaindB Xmt antenna gain (dB)
  * @param xmtFreqMhz Transmitter frequency (MHz), must be non-zero
  * @param xmtrPwrWatts Transmitter peak power (Watts), must be non-zero
  * @param rcvrSensDbm Receiver sensitivity (dBm)
  * @param fsLossDb Optional free space path loss calculation (dB)
  * @return Free space detection range for an ESM receiver (m).
  */
  SDKCORE_EXPORT double getOneWayFreeSpaceRangeAndLoss(double xmtGaindB, double xmtFreqMhz, double xmtrPwrWatts, double rcvrSensDbm, double* fsLossDb);

  /// As defined in https://en.wikipedia.org/wiki/Radio_spectrum
  enum FrequencyBandUsEcm
  {
    USECM_FREQ_OUT_OF_BOUNDS,
    USECM_FREQ_A,
    USECM_FREQ_B,
    USECM_FREQ_C,
    USECM_FREQ_D,
    USECM_FREQ_E,
    USECM_FREQ_F,
    USECM_FREQ_G,
    USECM_FREQ_H,
    USECM_FREQ_I,
    USECM_FREQ_J,
    USECM_FREQ_K,
    USECM_FREQ_L,
    USECM_FREQ_M
  };

  /**
   * Returns the US ECM frequency band for the given frequency
   * @param freqMhz Transmitter frequency (MHz)
   * @return The frequency band.
   */
  SDKCORE_EXPORT FrequencyBandUsEcm toUsEcm(double freqMhz);

  /**
   * Converts a given ECM frequency band to its minimum and maximum frequencies
   * @param[in] usEcm The frequency band
   * @param[out] minFreqMhz Minimum transmitter frequency (MHz)
   * @param[out] maxFreqMhz Maximum transmitter frequency (MHz)
   */
  SDKCORE_EXPORT void getFreqMhzRange(FrequencyBandUsEcm usEcm, double* minFreqMhz, double* maxFreqMhz);

  /// As defined in https://en.wikipedia.org/wiki/Radio_spectrum
  enum FrequencyBandIEEE
  {
    IEEE_FREQ_OUT_OF_BOUNDS,
    IEEE_FREQ_HF,
    IEEE_FREQ_VHF,
    IEEE_FREQ_UHF,
    IEEE_FREQ_L,
    IEEE_FREQ_S,
    IEEE_FREQ_C,
    IEEE_FREQ_X,
    IEEE_FREQ_KU,
    IEEE_FREQ_K,
    IEEE_FREQ_KA,
    IEEE_FREQ_V,
    IEEE_FREQ_W,
    IEEE_FREQ_G,
    IEEE_FREQ_MM // Note: the mm (millimeter) band encompasses part of KA (30GHz) through G (300GHz)
  };

  /**
   * Returns the IEEE ECM frequency band for the given frequency
   * @param freqMhz Transmitter frequency (MHz)
   * @param useMM Will return IEEE_FREQ_MM if freqMhz is in the millimeter band, rather than the band in KA-G
   * @return The frequency band.
   */
  SDKCORE_EXPORT FrequencyBandIEEE toIeeeBand(double freqMhz, bool useMM = false);

  /**
   * Converts a given IEEE frequency band to its minimum and maximum frequencies
   * @param[in] ieeeBand The frequency band
   * @param[out] minFreqMhz Minimum transmitter frequency (MHz)
   * @param[out] maxFreqMhz Maximum transmitter frequency (MHz)
   */
  SDKCORE_EXPORT void getFreqMhzRange(FrequencyBandIEEE ieeeBand, double* minFreqMhz, double* maxFreqMhz);

} // namespace simCore

#endif  /* SIMCORE_EM_PROPAGATION_H */
