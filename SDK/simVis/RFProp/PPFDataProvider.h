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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_RFPROP_PPF_PROVIDER_H
#define SIMVIS_RFPROP_PPF_PROVIDER_H

#include "simVis/RFProp/FunctionalProfileDataProvider.h"

namespace simRF
{
/**
* PPFDataProvider provides Pattern Propagation Factor data, based on Loss data
*/
class SDKVIS_EXPORT PPFDataProvider : public FunctionalProfileDataProvider
{
public:
  /**
  * Construct a PPFDataProvider with given parameters
  * @param templateProvider The provider that provides Loss information for this provider
  * @param radarParameters ptr to structure containing RF parameters to use for calculation
  */
  PPFDataProvider(const ProfileDataProvider* templateProvider, const RadarParametersPtr radarParameters);

  /** @copydoc simRF::ProfileDataProvider::getValueByIndex() */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;

  /** @copydoc simRF::ProfileDataProvider::interpolateValue() */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

protected:
  /// osg::Referenced-derived
  virtual ~PPFDataProvider() {}

private:
  /**
  * Gets the PPF value corresponding to a loss in dB
  * @param lossdB  the loss specified in dB
  * @param height  The height, in meters
  * @param range  The (non-slant) range, in meters
  * @return PPF value, in dB
  */
  double getPPF_(double lossdB, double height, double range) const;

  /// RF system parameter values used in RF Propagation calculations
  const RadarParametersPtr radarParameters_;
};

}
#endif
