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
#ifndef SIMVIS_RFPROP_POD_PROFILE_DATA_PROVIDER_H
#define SIMVIS_RFPROP_POD_PROFILE_DATA_PROVIDER_H

#include <memory>
#include <vector>
#include "simVis/RFProp/FunctionalProfileDataProvider.h"

namespace simRF
{
typedef std::shared_ptr<std::vector<float> > PODVectorPtr;

/**
 * PODProfileDataProvider provides 1D loss data (in dB) indexed by probability of detection (POD).
 * The table has 100 loss values corresponding to integral probabilities from 0 to 99.
 * The class provides for interpolation between those integral probability values.
 */
class SDKVIS_EXPORT PODProfileDataProvider: public FunctionalProfileDataProvider
{
public:
  /** Size of the POD Vector (i.e. 1 element per percentage) */
  static const unsigned int POD_VECTOR_SIZE = 100;

  /**
   * Creates a new PODProfileDataProvider
   * @param templateProvider the loss provider that this provider uses as input
   * @param podVector the POD table, as a vector of floats
   */
  PODProfileDataProvider(const ProfileDataProvider *templateProvider, const PODVectorPtr podVector);

  /**
   * Gets the value on this Profile
   * @param heightIndex The height index of the desired sample
   * @param rangeIndex The range index of the desired sample
   * @return POD value at the specified height and range, a probability between 0 and 99.9
   */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;

  /**
   * Gets the value on this Profile
   * @param height The height of the desired sample, in meters
   * @param range The range of the desired sample, in meters
   * @return POD value at the specified height and range, a probability between 0 and 99.9
   */
  virtual double interpolateValue(double height, double range) const;

  /**
  * Gets the POD value corresponding to a loss in dB
  * @param lossdB the loss specified in dB, must be a negative number
  * @param podVector  shared_ptr to the POD table, as a vector of floats
  * @return POD value for the specified loss, a probability between 0 and 99.9
  */
  static double getPOD(double lossdB, const PODVectorPtr podVector);

protected:
  /// osg::Referenced-derived
  virtual ~PODProfileDataProvider();

private:
  const PODVectorPtr podVector_;
};
}

#endif /* SIMVIS_RFPROP_POD_PROFILE_DATA_PROVIDER_H */

