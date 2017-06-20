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

FrequencyDesignationUsEcm toUsEcm(double freqMhz)
{
  // As defined in https://en.wikipedia.org/wiki/Radio_spectrum

  if (freqMhz < 0.0)
    return FREQ_ECM_OUT_OF_BOUNDS;

  if (freqMhz < 250.0)
    return FREQ_ECM_A;

  if (freqMhz < 500.0)
    return FREQ_ECM_B;

  if (freqMhz < 1000.0)
    return FREQ_ECM_C;

  if (freqMhz < 2000.0)
    return FREQ_ECM_D;

  if (freqMhz < 3000.0)
    return FREQ_ECM_E;

  if (freqMhz < 4000.0)
    return FREQ_ECM_F;

  if (freqMhz < 6000.0)
    return FREQ_ECM_G;

  if (freqMhz < 8000.0)
    return FREQ_ECM_H;

  if (freqMhz < 10000.0)
    return FREQ_ECM_I;

  if (freqMhz < 20000.0)
    return FREQ_ECM_J;

  if (freqMhz < 40000.0)
    return FREQ_ECM_K;

  if (freqMhz < 60000.0)
    return FREQ_ECM_L;

  if (freqMhz < 100000.0)
    return FREQ_ECM_M;

  return FREQ_ECM_OUT_OF_BOUNDS;
}

void getFreqMhzRange(FrequencyDesignationUsEcm usEcm, double* minFreqMhz, double* maxFreqMhz)
{
  double minFreq = 0.0;
  double maxFreq = 0.0;

  switch (usEcm)
  {
    case FREQ_ECM_A:
      minFreq = 0.0;
      maxFreq = 250;
      break;
    case FREQ_ECM_B:
      minFreq = 250;
      maxFreq = 500;
      break;
    case FREQ_ECM_C:
      minFreq = 500;
      maxFreq = 1000;
      break;
    case FREQ_ECM_D:
      minFreq = 1000;
      maxFreq = 2000;
      break;
    case FREQ_ECM_E:
      minFreq = 2000;
      maxFreq = 3000;
      break;
    case FREQ_ECM_F:
      minFreq = 3000;
      maxFreq = 4000;
      break;
    case FREQ_ECM_G:
      minFreq = 4000;
      maxFreq = 6000;
      break;
    case FREQ_ECM_H:
      minFreq = 6000;
      maxFreq = 8000;
      break;
    case FREQ_ECM_I:
      minFreq = 8000;
      maxFreq = 10000;
      break;
    case FREQ_ECM_J:
      minFreq = 10000;
      maxFreq = 20000;
      break;
    case FREQ_ECM_K:
      minFreq = 20000;
      maxFreq = 40000;
      break;
    case FREQ_ECM_L:
      minFreq = 40000;
      maxFreq = 60000;
      break;
    case FREQ_ECM_M:
      minFreq = 60000;
      maxFreq = 100000;
      break;
    case FREQ_ECM_OUT_OF_BOUNDS:
      break;
    default:
      assert(0); // New band added to enum, but not here
      break;
  }

  if (minFreqMhz != NULL)
    *minFreqMhz = minFreq;
  if (maxFreqMhz != NULL)
    *maxFreqMhz = maxFreq;

  return;
}

}

