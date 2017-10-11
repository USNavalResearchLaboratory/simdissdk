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
#ifndef SIMVIS_RFPROP_RADAR_PARAMETERS_H
#define SIMVIS_RFPROP_RADAR_PARAMETERS_H

#include "simCore/Common/Memory.h"

namespace simRF
{

/**
 * RadarParameters contains RF system parameter values used in RF Propagation calculations by One-Way, Two-Way and SNR data providers.
 */
struct RadarParameters
{
  /** Frequency in MHz */
  double freqMHz;
  /** Antenna gain in dB */
  double antennaGaindB;
  /** Noise figure in dB */
  double noiseFiguredB;
  /** Pulse width in uSec */
  double pulseWidth_uSec;
  /** Noise power in dB, calculated from noiseFiguredB and pulseWidth_uSec */
  double noisePowerdB;
  /** System loss in dB */
  double systemLossdB;
  /** Transmit power in KW */
  double xmtPowerKW;
  /** Transmit power in W, calculated from xmtPowerKW */
  double xmtPowerW;
  /** Horizontal beam width in degrees */
  double hbwD;
};

/** Shared pointer of a RadarParameters */
typedef std::shared_ptr<struct RadarParameters> RadarParametersPtr;

}

#endif /* SIMVIS_RFPROP_RADAR_PARAMETERS_H */


