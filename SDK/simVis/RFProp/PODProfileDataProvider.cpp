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
#include <algorithm>
#include <cassert>
#include "simVis/RFProp/PODProfileDataProvider.h"

namespace simRF
{
  PODProfileDataProvider::PODProfileDataProvider(const ProfileDataProvider *templateProvider, const PODVectorPtr podVector)
  : FunctionalProfileDataProvider(templateProvider),
   podVector_(podVector)
{
  setType_(ProfileDataProvider::THRESHOLDTYPE_POD);
}

PODProfileDataProvider::~PODProfileDataProvider()
{
}

double PODProfileDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  const double lossdB = FunctionalProfileDataProvider::templateGetValueByIndex_(heightIndex, rangeIndex);
  return getPOD(-lossdB, podVector_);
}

double PODProfileDataProvider::interpolateValue(double height, double range) const
{
  const double lossdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return getPOD(-lossdB, podVector_);
}

// static
double PODProfileDataProvider::getPOD(double lossdB, const PODVectorPtr podVector)
{
  // cast to float to avoid float vs double artifacts
  const float lossdBfloat = static_cast<float>(lossdB);
  if (lossdBfloat > 0 || podVector->size() != POD_VECTOR_SIZE || lossdBfloat < (*podVector)[0])
    return 0.0;
  else if (lossdBfloat == (*podVector)[0])
    return 1.0;
  else if (lossdBfloat >= (*podVector)[POD_VECTOR_SIZE - 1])
    return 99.9;
  else
  {
    // highPOD is the high integral value of the Probability of Detection, 96(%), for example.
    // the((lossdB - loVal) / (hiVal - loVal)) calculates the fractional probability that is between the high POD(96) and the low POD(95)
    const std::vector<float>::const_iterator iter = std::lower_bound(podVector->begin(), podVector->end(), lossdBfloat);
    const std::vector<float>::const_iterator beginiter = podVector->begin();
    const size_t highPOD = std::distance(beginiter, iter);
    // highPOD = 0 is: lossdBfloat == (*podVector)[0] above
    assert(highPOD != 0);
    const float hiVal = *iter;
    const float loVal = (*podVector)[highPOD - 1];
    if (hiVal - loVal == 0.0)
      return loVal;
    return ((highPOD)+((lossdB - loVal) / (hiVal - loVal)));
  }
}

}
