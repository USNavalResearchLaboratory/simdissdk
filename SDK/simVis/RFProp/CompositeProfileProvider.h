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
#ifndef SIMVIS_RFPROP_COMPOSITE_PROFILE_PROVIDER_H
#define SIMVIS_RFPROP_COMPOSITE_PROFILE_PROVIDER_H

#include <vector>
#include "osg/ref_ptr"
#include "simVis/RFProp/ProfileDataProvider.h"

namespace simRF
{

/**
 * CompositeProfileProvider takes multiple ProfileDataProviders and allows you to
 * select which one is active so you can easily swap out the underlying data provider
 */
class SDKVIS_EXPORT CompositeProfileProvider : public ProfileDataProvider
{
public:
  /** Creates a new CompositeProfileProvider */
  CompositeProfileProvider();

  /**
   * Gets the index of the provider
   * @return index of the active provider
   */
  int getActiveProviderIndex() const;

  /**
   * Returns the active provider
   * @return pointer to ProfileDataProvider that is the active provider
   */
  const ProfileDataProvider* getActiveProvider() const;

  /**
   * Returns the provider of the specified type
   * @param type Threshold type of provider to select
   * @return pointer to ProfileDataProvider that is the active provider
   */
  const ProfileDataProvider* getProvider(ProfileDataProvider::ThresholdType type) const;

  /**
   * Sets the index of the provider
   * @param index index of the provider to activate
   * @return 0 on success, 1 if index was out of range
   */
  int setActiveProvider(int index);

  /**
   * Sets the provider with requested type to be the active provider, if it exists
   * @param type Threshold type of provider to select
   * @return 0 on success, 1 if provider with Threshold type cannot be found
   */
  int setActiveProvider(ProfileDataProvider::ThresholdType type);

  /** Gets the number of providers */
  unsigned int getNumProviders() const;

  /** Sentinel value returned by getHeightIndex to indicate invalid height */
  static const unsigned int INVALID_HEIGHT_INDEX;

  /**
  * Gets the index corresponding to the specified height
  * @param heightM height in meters
  * @return INVALID_HEIGHT_INDEX on error.  When not an error, the return value is the
  *   index for the specified height, clamped between index 0 and index [max_index].
  */
  unsigned int getHeightIndex(double heightM) const;

  /** @name data access
   * @{
   */
  virtual unsigned int getNumRanges() const;
  virtual double getRangeStep() const;
  virtual double getMinRange() const;
  virtual double getMaxRange() const;

  virtual unsigned int getNumHeights() const;
  virtual double getMinHeight() const;
  virtual double getMaxHeight() const;
  virtual double getHeightStep() const;
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;
  ///@}

  /**
  * Interpolates the value on this Profile at the given height and range.
  * @param hgtMeters The height index of the desired sample, in meters
  * @param gndRngMeters The range of the desired sample, in meters
  * @return value at the specified height and range
  */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

  /** Adds a ProfileDataProvider to this CompositeProfileProvider; if it is the first, make it the active provider */
  void addProvider(ProfileDataProvider* provider);

protected:
  /// osg::Referenced-derived
  virtual ~CompositeProfileProvider() {}

protected:
  /** Index into providers_ that represents the current profile provider */
  int activeIndex_;
  /** Height provider index */
  int heightProviderIndex_;
  /** Vector of ProfileDataProvider ref_ptr */
  typedef std::vector<osg::ref_ptr<ProfileDataProvider> > ProfileDataProviderList;
  /** List of providers this instance can represent */
  ProfileDataProviderList providers_;
};
}

#endif /* SIMVIS_RFPROP_COMPOSITE_PROFILE_PROVIDER_H */
