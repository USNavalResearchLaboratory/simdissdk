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
#include <cassert>
#include "simVis/RFProp/FunctionalProfileDataProvider.h"

namespace simRF
{
FunctionalProfileDataProvider::FunctionalProfileDataProvider(const ProfileDataProvider *templateProvider)
 : templateProvider_(templateProvider)
{
  assert(templateProvider_);
}

FunctionalProfileDataProvider::~FunctionalProfileDataProvider()
{
  templateProvider_ = nullptr;
}

unsigned int FunctionalProfileDataProvider::getNumRanges() const
{
  return templateProvider_->getNumRanges();
}

double FunctionalProfileDataProvider::getRangeStep() const
{
  return templateProvider_->getRangeStep();
}

double FunctionalProfileDataProvider::getMinRange() const
{
  return templateProvider_->getMinRange();
}

double FunctionalProfileDataProvider::getMaxRange() const
{
  return templateProvider_->getMaxRange();
}

unsigned int FunctionalProfileDataProvider::getNumHeights() const
{
  return templateProvider_->getNumHeights();
}

double FunctionalProfileDataProvider::getMinHeight() const
{
  return templateProvider_->getMinHeight();
}

double FunctionalProfileDataProvider::getMaxHeight() const
{
  return templateProvider_->getMaxHeight();
}

double FunctionalProfileDataProvider::getHeightStep() const
{
  return templateProvider_->getHeightStep();
}

double FunctionalProfileDataProvider::templateGetValueByIndex_(unsigned int heightIndex, unsigned int rangeIndex) const
{
  return templateProvider_->getValueByIndex(heightIndex, rangeIndex);
}

double FunctionalProfileDataProvider::templateInterpolateValue_(double height, double range) const
{
  return templateProvider_->interpolateValue(height, range);
}

double FunctionalProfileDataProvider::getRange_(unsigned int rangeIndex) const
{
  if (rangeIndex >= getNumRanges())
  {
    assert(false);
    rangeIndex = getNumRanges() - 1;
  }
  return getRangeStep() * rangeIndex + getMinRange();
}

double FunctionalProfileDataProvider::getHeight_(unsigned int heightIndex) const
{
  if (heightIndex >= getNumHeights())
  {
    assert(false);
    heightIndex = getNumHeights() - 1;
  }
  return getHeightStep() * heightIndex + getMinHeight();
}
}
