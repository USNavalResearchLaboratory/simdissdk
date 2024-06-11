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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
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

RadarParameters::RadarParameters()
  : freqMHz(0.),
  antennaGaindBi(0.),
  noiseFiguredB(0.),
  pulseWidth_uSec(0.),
  noisePowerdB(0.),
  systemLossdB(0.),
  xmtPowerKW(0.),
  xmtPowerW(0.),
  hbwD(0.)
{
}

double getRcvdPowerFreeSpace(double rngMeters, double freqMhz, double powerWatts, double xmtGaindB, double rcvGaindB, double rcsSqm, double systemLossdB, bool oneWay)
{
  if (freqMhz == 0.0 || rngMeters == 0.0)
  {
    assert(0); // Must be non-zero to avoid divide by zero below
    return 0.0;
  }
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

double getRcvdPowerBlake(double rngMeters, double freqMhz, double powerWatts, double xmtGaindB, double rcvGaindB, double rcsSqm, double ppfdB, double systemLossdB, bool oneWay)
{
  double rcvPower = getRcvdPowerFreeSpace(rngMeters, freqMhz, powerWatts, xmtGaindB, rcvGaindB, rcsSqm, systemLossdB, oneWay);
  // Received signal power calculation from Blake's equation 1.18 (p 12)
  // Radar Range-Performance Analysis (1986)
  // Lamont V. Blake, ISBN 0-89006-224-2
  // Use free space value, then apply propagation factor
  return (oneWay == false) ? (rcvPower + (4. * ppfdB)) : (rcvPower + (2. * ppfdB));
}

double getOneWayFreeSpaceRangeAndLoss(double xmtGaindB, double xmtFreqMhz, double xmtrPwrWatts, double rcvrSensDbm, double* fsLossDb)
{
  if (xmtFreqMhz == 0.0)
  {
    assert(0); // Should not receive 0
    xmtFreqMhz = 1.0; // Protect against divide by zero and log10(0) below
  }
  if (xmtrPwrWatts == 0.0)
  {
    assert(0); // Should not receive 0
    xmtrPwrWatts = 1.0; // Protect against log10(0) below
  }

  // Compute transmitter power in dB, function requires power in kilowatts
  const double xmtPwrDb = 10. * log10((xmtrPwrWatts * 1e-3) / (xmtFreqMhz * xmtFreqMhz));

  // Free-space range equation (km) for an ESM receiver, derived from Kerr (1951, Eq 2-15)
  // also found in "Specification for Radar Free-Space Detection Range and Free-Space Intercept Range Calculations", C; P. Hatton (p. 7, Eq 13)
  const double esmRngKm = pow(10., ((xmtPwrDb + xmtGaindB - rcvrSensDbm + 27.5517) / 20.));

  // One-way free space loss equation from "Electronic Warfare and Radar Systems Handbook", NAWCWPNS TP 8347, Rev 2 April 1999, p 4-3.1
  // 32.45 is the K1 term in the one way free-space loss equation when the range units are in km and freq in MHz and using LIGHT_SPEED_AIR
  if (fsLossDb)
    *fsLossDb = 20. * log10(xmtFreqMhz * esmRngKm) + 32.45;

  // Free-space detection range (m) for an ESM receiver
  return esmRngKm * 1000.0;
}

double lossToPpf(double slantRange, double freqMHz, double loss_dB)
{
  if (!std::isfinite(loss_dB) || loss_dB <= simCore::SMALL_DB_VAL)
    return simCore::SMALL_DB_VAL;
  if (slantRange <= 0.0 || freqMHz <= 0.0)
  {
    assert(0); // Should not receive <=0
    return simCore::SMALL_DB_VAL;
  }
  // loss_db (power pattern path loss) and ppf_db (power pattern propagation factor) are related by:
  // loss_db = one way free space loss - ppf_db
  // one way free space loss: 20 * log10(2 * k0 * R)
  // k0: vacuum wavenumber
  const double vacuumWavenumber = (M_TWOPI * 1e6 * freqMHz) / simCore::LIGHT_SPEED_VACUUM;
  const double fsLossDb = 20 * log10(2 * vacuumWavenumber * slantRange);
  const double ppf_dB = fsLossDb - loss_dB;
  return ppf_dB;
}

FrequencyBandUsEcm toUsEcm(double freqMhz)
{
  // As defined in https://en.wikipedia.org/wiki/Radio_spectrum

  if (freqMhz < 0.0)
    return USECM_FREQ_OUT_OF_BOUNDS;

  if (freqMhz < 250.0)
    return USECM_FREQ_A;

  if (freqMhz < 500.0)
    return USECM_FREQ_B;

  if (freqMhz < 1000.0)
    return USECM_FREQ_C;

  if (freqMhz < 2000.0)
    return USECM_FREQ_D;

  if (freqMhz < 3000.0)
    return USECM_FREQ_E;

  if (freqMhz < 4000.0)
    return USECM_FREQ_F;

  if (freqMhz < 6000.0)
    return USECM_FREQ_G;

  if (freqMhz < 8000.0)
    return USECM_FREQ_H;

  if (freqMhz < 10000.0)
    return USECM_FREQ_I;

  if (freqMhz < 20000.0)
    return USECM_FREQ_J;

  if (freqMhz < 40000.0)
    return USECM_FREQ_K;

  if (freqMhz < 60000.0)
    return USECM_FREQ_L;

  if (freqMhz < 100000.0)
    return USECM_FREQ_M;

  return USECM_FREQ_OUT_OF_BOUNDS;
}

void getFreqMhzRange(FrequencyBandUsEcm usEcm, double* minFreqMhz, double* maxFreqMhz)
{
  double minFreq = 0.0;
  double maxFreq = 0.0;

  switch (usEcm)
  {
  case USECM_FREQ_A:
    minFreq = 0.0;
    maxFreq = 250;
    break;
  case USECM_FREQ_B:
    minFreq = 250;
    maxFreq = 500;
    break;
  case USECM_FREQ_C:
    minFreq = 500;
    maxFreq = 1000;
    break;
  case USECM_FREQ_D:
    minFreq = 1000;
    maxFreq = 2000;
    break;
  case USECM_FREQ_E:
    minFreq = 2000;
    maxFreq = 3000;
    break;
  case USECM_FREQ_F:
    minFreq = 3000;
    maxFreq = 4000;
    break;
  case USECM_FREQ_G:
    minFreq = 4000;
    maxFreq = 6000;
    break;
  case USECM_FREQ_H:
    minFreq = 6000;
    maxFreq = 8000;
    break;
  case USECM_FREQ_I:
    minFreq = 8000;
    maxFreq = 10000;
    break;
  case USECM_FREQ_J:
    minFreq = 10000;
    maxFreq = 20000;
    break;
  case USECM_FREQ_K:
    minFreq = 20000;
    maxFreq = 40000;
    break;
  case USECM_FREQ_L:
    minFreq = 40000;
    maxFreq = 60000;
    break;
  case USECM_FREQ_M:
    minFreq = 60000;
    maxFreq = 100000;
    break;
  case USECM_FREQ_OUT_OF_BOUNDS:
    break;
  default:
    assert(0); // New band added to enum, but not here
    break;
  }

  if (minFreqMhz != nullptr)
    *minFreqMhz = minFreq;
  if (maxFreqMhz != nullptr)
    *maxFreqMhz = maxFreq;

  return;
}

FrequencyBandIEEE toIeeeBand(double freqMhz, bool useMM)
{
  // As defined in https://en.wikipedia.org/wiki/Radio_spectrum#IEEE

  if (freqMhz < 3.0)
    return IEEE_FREQ_OUT_OF_BOUNDS;

  if (freqMhz < 30.0)
    return IEEE_FREQ_HF;

  if (freqMhz < 300.0)
    return IEEE_FREQ_VHF;

  if (freqMhz < 1000.0)
    return IEEE_FREQ_UHF;

  if (freqMhz < 2000.0)
    return IEEE_FREQ_L;

  if (freqMhz < 4000.0)
    return IEEE_FREQ_S;

  if (freqMhz < 8000.0)
    return IEEE_FREQ_C;

  if (freqMhz < 12000.0)
    return IEEE_FREQ_X;

  if (freqMhz < 18000.0)
    return IEEE_FREQ_KU;

  if (freqMhz < 27000.0)
    return IEEE_FREQ_K;

  if (useMM && freqMhz >= 30000.0 && freqMhz < 300000.0)
    return IEEE_FREQ_MM;

  if (freqMhz < 40000.0)
    return IEEE_FREQ_KA;

  if (freqMhz < 75000.0)
    return IEEE_FREQ_V;

  if (freqMhz < 110000.0)
    return IEEE_FREQ_W;

  if (freqMhz < 300000.0)
    return IEEE_FREQ_G;

  return IEEE_FREQ_OUT_OF_BOUNDS;
}

void getFreqMhzRange(FrequencyBandIEEE ieeeEcm, double* minFreqMhz, double* maxFreqMhz)
{
  double minFreq = 0.0;
  double maxFreq = 0.0;

  switch (ieeeEcm)
  {
  case IEEE_FREQ_OUT_OF_BOUNDS:
    break;
  case IEEE_FREQ_HF:
    minFreq = 3.0;
    maxFreq = 30.0;
    break;
  case IEEE_FREQ_VHF:
    minFreq = 30.0;
    maxFreq = 300.0;
    break;
  case IEEE_FREQ_UHF:
    minFreq = 300.0;
    maxFreq = 1000.0;
    break;
  case IEEE_FREQ_L:
    minFreq = 1000.0;
    maxFreq = 2000.0;
    break;
  case IEEE_FREQ_S:
    minFreq = 2000.0;
    maxFreq = 4000.0;
    break;
  case IEEE_FREQ_C:
    minFreq = 4000.0;
    maxFreq = 8000.0;
    break;
  case IEEE_FREQ_X:
    minFreq = 8000.0;
    maxFreq = 12000.0;
    break;
  case IEEE_FREQ_KU:
    minFreq = 12000.0;
    maxFreq = 18000.0;
    break;
  case IEEE_FREQ_K:
    minFreq = 18000.0;
    maxFreq = 27000.0;
    break;
  case IEEE_FREQ_KA:
    minFreq = 27000.0;
    maxFreq = 40000.0;
    break;
  case IEEE_FREQ_V:
    minFreq = 40000.0;
    maxFreq = 75000.0;
    break;
  case IEEE_FREQ_W:
    minFreq = 75000.0;
    maxFreq = 110000.0;
    break;
  case IEEE_FREQ_G:
    minFreq = 110000.0;
    maxFreq = 300000.0;
    break;
  case IEEE_FREQ_MM:
    minFreq = 30000.0;
    maxFreq = 300000.0;
    break;
  default:
    assert(0); // New band added to enum, but not here
    break;
  }

  if (minFreqMhz != nullptr)
    *minFreqMhz = minFreq;
  if (maxFreqMhz != nullptr)
    *maxFreqMhz = maxFreq;

  return;
}

}

