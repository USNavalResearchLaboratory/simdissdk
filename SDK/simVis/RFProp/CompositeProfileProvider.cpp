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
#include <limits>
#include "simNotify/Notify.h"
#include "simVis/RFProp/CompositeProfileProvider.h"

namespace simRF
{

/// Sentinel value for invalid return from getHeightIndex()
const unsigned int CompositeProfileProvider::INVALID_HEIGHT_INDEX = std::numeric_limits<unsigned int>::max();

CompositeProfileProvider::CompositeProfileProvider()
  : activeIndex_(-1),
  heightProviderIndex_(-1)
{
}

int CompositeProfileProvider::getActiveProviderIndex() const
{
  return activeIndex_;
}

const ProfileDataProvider* CompositeProfileProvider::getActiveProvider() const
{
  return (activeIndex_ != -1) ? providers_[activeIndex_].get() : NULL;
}

int CompositeProfileProvider::setActiveProvider(int index)
{
  if (index < 0 || static_cast<size_t>(index) >= providers_.size())
  {
    SIM_NOTICE << "Error:  setActiveProvider( " << index << ") index out of range" << std::endl;
    activeIndex_ = -1;
    return 1;
  }

  activeIndex_ = index;
  return 0;
}

int CompositeProfileProvider::setActiveProvider(ProfileDataProvider::ThresholdType type)
{
  size_t count = getNumProviders();
  for (size_t i = 0; i < count; ++i)
  {
    if (providers_[i]->getType() == type)
    {
      activeIndex_ = i;
      return 0;
    }
  }
  activeIndex_ = -1;
  return 1;
}

const ProfileDataProvider* CompositeProfileProvider::getProvider(ProfileDataProvider::ThresholdType type) const
{
  size_t count = getNumProviders();
  for (size_t i = 0; i < count; ++i)
  {
    if (providers_[i]->getType() == type)
    {
      return providers_[i].get();
    }
  }
  return NULL;
}

unsigned int CompositeProfileProvider::getNumProviders() const
{
  return providers_.size();
}

void CompositeProfileProvider::addProvider(ProfileDataProvider* provider)
{
  providers_.push_back(provider);
  if (providers_.size() == 1)
  {
    setActiveProvider(0);
    heightProviderIndex_ = 0;
  }
  else if (heightProviderIndex_ <= 0 && provider->getType() != simRF::ProfileDataProvider::THRESHOLDTYPE_CNR)
  {
    // some providers (CNR) do not have height information, so we maintain a special index to a provider that does have that info
    heightProviderIndex_ = providers_.size() - 1;
  }
}

unsigned int CompositeProfileProvider::getNumRanges() const
{
  return getActiveProvider() ? getActiveProvider()->getNumRanges() : 0;
}

double CompositeProfileProvider::getRangeStep() const
{
  return getActiveProvider() ? getActiveProvider()->getRangeStep() : 0;
}

double CompositeProfileProvider::getMinRange() const
{
  return getActiveProvider() ? getActiveProvider()->getMinRange() : 0;
}

double CompositeProfileProvider::getMaxRange() const
{
  return getActiveProvider() ? getActiveProvider()->getMaxRange() : 0;
}

// some providers may not have height information, so these methods use heightProviderIndex_ to find a provider that does have that info
unsigned int CompositeProfileProvider::getHeightIndex(double heightM) const
{
  const ProfileDataProvider* provider = (heightProviderIndex_ != -1) ? providers_[heightProviderIndex_].get() : NULL;
  // No provider? error out
  if (!provider)
    return INVALID_HEIGHT_INDEX;
  // No slots?  error out
  if (provider->getNumHeights() == 0)
    return INVALID_HEIGHT_INDEX;

  // Invalid height step?  error out
  const double heightStep = provider->getHeightStep();
  if (heightStep < 0)
    return INVALID_HEIGHT_INDEX;
  // Avoid divide by zero
  if (heightStep == 0.0)
    return 0;

  // Scale the height between min and max to get an index
  if (heightM >= provider->getMaxHeight())
    return provider->getNumHeights() - 1;
  const double minHeight = provider->getMinHeight();
  return (heightM <= minHeight) ? 0 : static_cast<unsigned int>((heightM - minHeight) / heightStep);
}

unsigned int CompositeProfileProvider::getNumHeights() const
{
  return (heightProviderIndex_ != -1) ? providers_[heightProviderIndex_]->getNumHeights() : 0;
}

double CompositeProfileProvider::getMinHeight() const
{
  return (heightProviderIndex_ != -1) ? providers_[heightProviderIndex_]->getMinHeight() : 0;
}

double CompositeProfileProvider::getMaxHeight() const
{
  return (heightProviderIndex_ != -1) ? providers_[heightProviderIndex_]->getMaxHeight() : 0;
}

double CompositeProfileProvider::getHeightStep() const
{
  return (heightProviderIndex_ != -1) ? providers_[heightProviderIndex_]->getHeightStep() : 0;
}


double CompositeProfileProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  return getActiveProvider() ? getActiveProvider()->getValueByIndex(heightIndex, rangeIndex) : 0;
}

double CompositeProfileProvider::interpolateValue(double height, double range) const
{
  return getActiveProvider() ? getActiveProvider()->interpolateValue(height, range) : 0;
}
}

