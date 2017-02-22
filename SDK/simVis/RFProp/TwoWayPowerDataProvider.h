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
#ifndef SIMVIS_RFPROP_TWOWAYPOWER_PROVIDER_H
#define SIMVIS_RFPROP_TWOWAYPOWER_PROVIDER_H

#include "simVis/RFProp/RadarParameters.h"
#include "simVis/RFProp/FunctionalProfileDataProvider.h"

namespace simRF
{
/**
* TwoWayPowerDataProvider calculates two-way/received power propagation information, based on AREPS PPF table data
*/
class SDKVIS_EXPORT TwoWayPowerDataProvider : public FunctionalProfileDataProvider
{
public:
  /**
  * Construct a TwoWayPowerDataProvider with given parameters
  * @param templateProvider The provider that provides PPF information for this provider
  * @param radarParameters ptr to structure containing RF parameters to use for calculation
  */
  TwoWayPowerDataProvider(const ProfileDataProvider* templateProvider, const RadarParametersPtr radarParameters);

  /** @copydoc simRF::ProfileDataProvider::getValueByIndex() */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;

  /** @copydoc simRF::ProfileDataProvider::interpolateValue() */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

  /**
  * Gets the two-way-power value for the specified parameters, in dB
  * @param height The height of the desired sample, in meters
  * @param range The range of the desired sample, in meters
  * @param slantRangeM The slant range, in meters
  * @param xmtGaindB The transmit gain in dB
  * @param rcvGaindB The receiver gain in dB
  * @param rcsSqm The radar-cross-section to sue for calculation, in square meters
  * @return two-way-power value, in dB
  */
  double getTwoWayPower(double height, double range, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm) const;

protected:
  /// osg::Referenced-derived
  virtual ~TwoWayPowerDataProvider() {}

private:
  /**
  * Gets the two-way-power value corresponding to a ppf in dB
  * @param ppfdB the ppf specified in dB
  * @param slantRangeM The slant range, in meters
  * @param xmtGaindB The transmit gain in dB
  * @param rcvGaindB The receiver gain in dB
  * @param rcsSqm The radar-cross-section to use for calculation, in square meters
  * @return two-way-power value, in dB
  */
  double getTwoWayPower_(double ppfdB, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm = 1.0) const;

private:
  const RadarParametersPtr radarParameters_;
};

}

#endif /* SIMVIS_RFPROP_TWOWAYPOWER_PROVIDER_H */
