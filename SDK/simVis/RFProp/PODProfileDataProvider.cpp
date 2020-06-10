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
#include <algorithm>
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
  double lossdB = FunctionalProfileDataProvider::templateGetValueByIndex_(heightIndex, rangeIndex);
  return getPOD_(-lossdB);
}

double PODProfileDataProvider::interpolateValue(double height, double range) const
{
  double lossdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return getPOD_(-lossdB);
}

double PODProfileDataProvider::getPOD_(double lossdB) const
{
  if (lossdB > 0 || podVector_->size() != POD_VECTOR_SIZE || lossdB <= static_cast<double>((*podVector_)[0]))
  {
    return 0.0;
  }
  else if (lossdB >= static_cast<double>((*podVector_)[POD_VECTOR_SIZE - 1]))
  {
    return 99.9;
  }
  else
  {
    // highPOD is the high integral value of the Probability of Detection, 96(%), for example.
    // the((lossdB - loVal) / (hiVal - loVal)) calculates the fractional probability that is between the high POD(96) and the low POD(95)
    std::vector<float>::const_iterator iter = std::lower_bound(podVector_->begin(), podVector_->end(), lossdB);
    std::vector<float>::const_iterator beginiter = podVector_->begin();
    size_t highPOD = std::distance(beginiter, iter);
    float hiVal = *iter;
    float loVal = (*podVector_)[highPOD - 1];
    return ((highPOD)+((lossdB - loVal) / (hiVal - loVal)));
  }
}

}
