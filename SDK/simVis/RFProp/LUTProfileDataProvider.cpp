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
#include "simCore/LUT/InterpTable.h"
#include "simNotify/Notify.h"
#include "simVis/RFProp/LUTProfileDataProvider.h"

namespace simRF
{
LUTProfileDataProvider::LUTProfileDataProvider(simCore::LUT::LUT2<short> *lut, double scalar)
 : scalar_(scalar)
{
  // LUTProfileDataProvider is taking ownership of the lut
  assert(lut);
  if (lut == nullptr)
  {
    SIM_ERROR << "Attempting to assign a NULL LUT to the LUTProfileDataProvider" << std::endl;
  }
  lut_ = lut;
}

LUTProfileDataProvider::LUTProfileDataProvider(simCore::LUT::LUT2<short> *lut, ProfileDataProvider::ThresholdType type, double scalar)
  : scalar_(scalar)
{
  // LUTProfileDataProvider is taking ownership of the lut
  assert(lut);
  if (lut == nullptr)
  {
    SIM_ERROR << "Attempting to assign a NULL LUT to the LUTProfileDataProvider" << std::endl;
  }
  lut_ = lut;
  setType_(type);
}

LUTProfileDataProvider::~LUTProfileDataProvider()
{
  delete lut_;
}

unsigned int LUTProfileDataProvider::getNumRanges() const
{
  return lut_->numY();
}

double LUTProfileDataProvider::getRangeStep() const
{
  return lut_->stepY();
}

double LUTProfileDataProvider::getHeightStep() const
{
  return lut_->stepX();
}

unsigned int LUTProfileDataProvider::getNumHeights() const
{
  return lut_->numX();
}

double LUTProfileDataProvider::getMinRange() const
{
  return lut_->minY();
}

double LUTProfileDataProvider::getMaxRange() const
{
  return lut_->maxY();
}

double LUTProfileDataProvider::getMinHeight() const
{
  return lut_->minX();
}

double LUTProfileDataProvider::getMaxHeight() const
{
  return lut_->maxX();
}

double LUTProfileDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  // Apply scalar to convert internal storage back to dB
  const double val = (*lut_)(heightIndex, rangeIndex);
  return ((val > AREPS_GROUND_VALUE) ? scalar_ * val : val);
}

double LUTProfileDataProvider::interpolateValue(double height, double range) const
{
  // Apply scalar to convert internal storage back to dB
  BilinearInterpolate<short> bil;
  return scalar_ * simCore::LUT::interpolate(*lut_, height, range, bil);
}

}

