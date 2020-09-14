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
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simNotify/Notify.h"
#include "simVis/RFProp/LUT1ProfileDataProvider.h"

namespace
{

/**
 * @brief Helper class for performing linear interpolation using () operator
 */
template <class Type>
class LinearInterpolate
{
public:
  /**
   * @brief Performs linear interpolation between a set of bounded values
   *
   * Performs bilinear interpolation between two sets of bounded values at the specified values
   * @param[in ] lowVal  left bound
   * @param[in ] highVal  right bound
   * @param[in ] xLow Low value of X bound
   * @param[in ] xVal value to perform interpolation along X axis
   * @param[in ] xHigh High value of X bound
   * @return linear interpolated value
   */
  Type operator()(Type lowVal, Type highVal, double xLow, double xVal, double xHigh)
  {
    return simCore::linearInterpolate(lowVal, highVal, xLow, xVal, xHigh);
  }
};
}

namespace simRF
{

LUT1ProfileDataProvider::LUT1ProfileDataProvider(simCore::LUT::LUT1<short> *lut, double scalar)
  : scalar_(scalar)
{
  // LUT1ProfileDataProvider is taking ownership of the lut
  assert(lut);
  if (lut == nullptr)
  {
    SIM_ERROR << "Attempting to assign a NULL LUT to the LUT1ProfileDataProvider" << std::endl;
  }
  lut_ = lut;
}

LUT1ProfileDataProvider::LUT1ProfileDataProvider(simCore::LUT::LUT1<short> *lut, ProfileDataProvider::ThresholdType type, double scalar)
  : scalar_(scalar)
{
  // LUT1ProfileDataProvider is taking ownership of the lut
  assert(lut);
  if (lut == nullptr)
  {
    SIM_ERROR << "Attempting to assign a NULL LUT to the LUT1ProfileDataProvider" << std::endl;
  }
  lut_ = lut;
  setType_(type);
}

LUT1ProfileDataProvider::~LUT1ProfileDataProvider()
{
  delete lut_;
}

unsigned int LUT1ProfileDataProvider::getNumRanges() const
{
  return lut_->numX();
}

double LUT1ProfileDataProvider::getRangeStep() const
{
  return lut_->stepX();
}

double LUT1ProfileDataProvider::getHeightStep() const
{
  return 0.0;
}

unsigned int LUT1ProfileDataProvider::getNumHeights() const
{
  return 1;
}

double LUT1ProfileDataProvider::getMinRange() const
{
  return lut_->minX();
}

double LUT1ProfileDataProvider::getMaxRange() const
{
  return lut_->maxX();
}

double LUT1ProfileDataProvider::getMinHeight() const
{
  return 0.0;
}

double LUT1ProfileDataProvider::getMaxHeight() const
{
  return 0.0;
}

double LUT1ProfileDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  // Apply scalar to convert internal storage back to dB
  return scalar_ * (*lut_)(rangeIndex);
}


double LUT1ProfileDataProvider::interpolateValue(double height, double range) const
{
  // Apply scalar to convert internal storage back to dB
  LinearInterpolate<short> lin;
  return scalar_ * simCore::LUT::interpolate(*lut_, range, lin);
}

}
