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
#ifndef SIMVIS_RFPROP_SNR_PROVIDER_H
#define SIMVIS_RFPROP_SNR_PROVIDER_H

#include "simVis/RFProp/TwoWayPowerDataProvider.h"

namespace simRF
{
/**
* SNRDataProvider provides Signal-to-Noise-Ratio data.
*/
class SDKVIS_EXPORT SNRDataProvider : public FunctionalProfileDataProvider
{
public:
  /** Constructor */
  SNRDataProvider(const TwoWayPowerDataProvider* templateProvider, const RadarParametersPtr radarParameters);

  /** @copydoc simRF::ProfileDataProvider::getValueByIndex() */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;

  /** @copydoc simRF::ProfileDataProvider::interpolateValue() */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

  /**
  * Gets the SNR value on this profile
  * @param height The height of the desired sample, in meters
  * @param range The range of the desired sample, in meters
  * @param slantRangeM The slant range, in meters
  * @param xmtGaindB The transmit gain in dB
  * @param rcvGaindB The receiver gain in dB
  * @param rcsSqm The radar-cross-section to use for calculation, in square meters
  * @return SNR value at the specified height and range, in dB
  */
  double getSNR(double height, double range, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm) const;

protected:
  // osg::Referenced-derived
  virtual ~SNRDataProvider();

private:
  osg::ref_ptr<const TwoWayPowerDataProvider> twoWayPowerProvider_;
  const RadarParametersPtr radarParameters_;
};

}

#endif /* SIMVIS_RFPROP_SNR_PROVIDER_H */
