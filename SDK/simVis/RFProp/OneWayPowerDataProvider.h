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
#ifndef SIMVIS_RFPROP_ONEWAYPOWER_PROVIDER_H
#define SIMVIS_RFPROP_ONEWAYPOWER_PROVIDER_H

#include "simVis/RFProp/FunctionalProfileDataProvider.h"

namespace simRF
{
/**
* OneWayPowerDataProvider calculates one-way-power propagation information, based on PPF data
*/
class SDKVIS_EXPORT OneWayPowerDataProvider : public FunctionalProfileDataProvider
{
public:
  /**
  * Gets the one-way-power propagation value corresponding to given ppf and with given parameters
  * @param radarParameters  the radar parameters specified for this rf prop instance
  * @param ppfdB  the ppf specified, in dB
  * @param slantRangeM  The slant range, in meters
  * @param xmtGaindB  The transmit gain, in dB
  * @param rcvGaindB  The receiver gain, in dB
  * @return one-way-power value in dB
  */
  static double getOneWayPower(const simCore::RadarParameters& radarParameters, double ppfdB, double slantRangeM, double xmtGaindB, double rcvGaindB);

  /**
  * Construct a OneWayPowerDataProvider with given parameters
  * @param templateProvider The provider that provides PPF information for this provider
  * @param radarParameters ptr to structure containing RF parameters to use for calculation
  */
  OneWayPowerDataProvider(const ProfileDataProvider* templateProvider, const RadarParametersPtr radarParameters);

  /** @copydoc simRF::ProfileDataProvider::getValueByIndex() */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;

  /** @copydoc simRF::ProfileDataProvider::interpolateValue() */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

  /**
  * Gets the one-way-power propagation value on this profile, in support of RFPropagationData interface.
  * @param height The height of the desired sample, in meters
  * @param range The range of the desired sample, in meters
  * @param slantRange The slant range, in meters
  * @param xmtGain The transmit gain, in dB
  * @param rcvGain The receiver gain, in dB
  * @return one-way-power value, in dB
  */
  double getOneWayPower(double height, double range, double slantRange, double xmtGain, double rcvGain) const;

protected:
  /// osg::Referenced-derived
  virtual ~OneWayPowerDataProvider() {}

private:
  const RadarParametersPtr radarParameters_;
};

}

#endif /* SIMVIS_RFPROP_ONEWAYPOWER_PROVIDER_H */
