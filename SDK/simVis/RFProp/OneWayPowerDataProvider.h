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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
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
* OneWayPowerDataProvider calculates one-way-power propagation information, based on AREPS PPF table data
*/
class SDKVIS_EXPORT OneWayPowerDataProvider : public FunctionalProfileDataProvider
{
public:
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
  /**
  * Gets the one-way-power propagation value corresponding to given ppf and with given parameters
  * @param ppf the ppf specified, in dB
  * @param slantRange The slant range, in meters
  * @param xmtGain The transmit gain, in dB
  * @param rcvGain The receiver gain, in dB
  * @return one-way-power value in dB
  */
  double getOneWayPower_(double ppf, double slantRange, double xmtGain, double rcvGain) const;

private:
  const RadarParametersPtr radarParameters_;
};

}

#endif /* SIMVIS_RFPROP_ONEWAYPOWER_PROVIDER_H */
