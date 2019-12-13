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
#ifndef SIMVIS_RFPROP_FUNCTIONAL_PROFILE_DATA_PROVIDER_H
#define SIMVIS_RFPROP_FUNCTIONAL_PROFILE_DATA_PROVIDER_H

#include <memory>
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simCore/EM/Propagation.h"
#include "simVis/RFProp/ProfileDataProvider.h"

namespace simRF
{
/** Shared pointer of a RadarParameters */
typedef std::shared_ptr<simCore::RadarParameters> RadarParametersPtr;

/**
 * FunctionalProfileDataProvider is an adapter class to support creation of data providers that depend on other data providers.
 * It exists to be the base class for dependent data providers.
 */
class SDKVIS_EXPORT FunctionalProfileDataProvider: public ProfileDataProvider
{
public:
  /**
   * Creates a new FunctionalProfileDataProvider
   * @param templateProvider the provider that the depending provider uses as input
   */
  FunctionalProfileDataProvider(const ProfileDataProvider *templateProvider);

  /** Gets the number of range values */
  virtual unsigned int getNumRanges() const;

  /** Gets the spacing between range samples, in meters */
  virtual double getRangeStep() const;

  /** Gets the min range, in meters */
  virtual double getMinRange() const;

  /** Gets the max range, in meters */
  virtual double getMaxRange() const;

  /** Gets the number of height values */
  virtual unsigned int getNumHeights() const;

  /** Gets the min height, in meters */
  virtual double getMinHeight() const;

  /** Gets the max height, in meters */
  virtual double getMaxHeight() const;

  /** Gets the spacing between height samples, in meters */
  virtual double getHeightStep() const;

protected:
  /// osg::Referenced-derived
  virtual ~FunctionalProfileDataProvider();

  /**
  * Gets the value on this profile from the templateProvider_
  * @param heightIndex The height index of the desired sample
  * @param rangeIndex The range index of the desired sample
  * @return value at the specified height index and range index
  */
  double templateGetValueByIndex_(unsigned int heightIndex, unsigned int rangeIndex) const;

  /**
  * Gets the value on this profile from the templateProvider_
  * @param height The height of the desired sample, in meters
  * @param range The range of the desired sample, in meters
  * @return value at the specified height and range
  */
  double templateInterpolateValue_(double height, double range) const;

  /**
  * Gets the range value corresponding to a range index
  * @param rangeIndex The index of the desired range
  * @return range in meters
  */
  double getRange_(unsigned int rangeIndex) const;

private:
  osg::ref_ptr<const ProfileDataProvider> templateProvider_;
};
}

#endif /* SIMVIS_RFPROP_FUNCTIONAL_PROFILE_DATA_PROVIDER_H */

