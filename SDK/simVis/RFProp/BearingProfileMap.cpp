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
#include "simCore/Calc/Angle.h"
#include "simVis/RFProp/Profile.h"
#include "simVis/RFProp/BearingProfileMap.h"

namespace simRF
{

BearingProfileMap::BearingProfileMap()
{
}

BearingProfileMap::~BearingProfileMap()
{
}

void BearingProfileMap::addProfile(Profile &profile)
{
  // if we have a profile at the given bearing
  BearingProfileMap::iterator itr = profiles_.find(profile.getBearing());
  if (itr != profiles_.end())
  {
    // remove it
    profiles_.erase(itr);
  }


  // insert the profile into the map
  profiles_[ profile.getBearing() ] = &profile;
}

BearingProfileMap::iterator BearingProfileMap::begin()
{
  return profiles_.begin();
}

BearingProfileMap::iterator BearingProfileMap::end()
{
  return profiles_.end();
}

bool BearingProfileMap::empty() const
{
  return profiles_.empty();
}

double BearingProfileMap::getSlotBearing(double bearingR) const
{
  BearingProfileMap::const_iterator itr = getSlot_(bearingR);
  return (itr != profiles_.end()) ? itr->first : simCore::angFix2PI(bearingR);
}

Profile* BearingProfileMap::getProfileByBearing(double bearingR) const
{
  BearingProfileMap::const_iterator itr = getSlot_(bearingR);
  return (itr != profiles_.end()) ? itr->second.get() : nullptr;
}

BearingProfileMap::const_iterator BearingProfileMap::getSlot_(double bearingR) const
{
  if (profiles_.empty())
    return profiles_.end();

  // profiles map is [0,360]
  bearingR = simCore::angFix2PI(bearingR);
  double halfbw = profiles_.begin()->second->getHalfBeamWidth() + 1e-06;

  // get the iterator > b - matching profile may be found at b+halfbw
  // not using lower_bound, since an exact match could be a mistake, if input bearing has seen truncation
  BearingProfileMap::const_iterator itr = profiles_.upper_bound(bearingR);
  if (itr != profiles_.end())
  {
    if (simCore::areAnglesEqual(bearingR, itr->first, halfbw))
      return itr;
  }
  else if (bearingR + halfbw >= M_TWOPI)
  {
    // handle edge case at 2PI
    if (simCore::areAnglesEqual(bearingR, profiles_.begin()->first, halfbw))
      return profiles_.begin();
  }

  // Now that we know the cell after is not correct, check the cell before
  if (itr != profiles_.begin())
  {
    --itr;
    if (simCore::areAnglesEqual(bearingR, itr->first, halfbw))
      return itr;
  }
  else if (bearingR - halfbw <= 0)
  {
    if (simCore::areAnglesEqual(bearingR, profiles_.rbegin()->first, halfbw))
      return --(profiles_.end());
  }
  return profiles_.end();
}

}
