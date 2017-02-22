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

#include <algorithm>

#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "PlatformFilter.h"


namespace simVis {


/// Platform filtering for Altitude Clamping
class AltitudeMinMaxClamping : public PlatformTspiFilter
{
public:
  AltitudeMinMaxClamping()
    : PlatformTspiFilter()
  {
  }

  virtual ~AltitudeMinMaxClamping()
  {
  }

  virtual bool isApplicable(const simData::PlatformPrefs& prefs) const
  {
    return prefs.useclampalt();
  }

  virtual PlatformTspiFilterManager::FilterResponse filter(simCore::Coordinate& llaCoord, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props)
  {
    PlatformTspiFilterManager::FilterResponse modified = PlatformTspiFilterManager::POINT_UNCHANGED;

    if (!prefs.useclampalt())
      return modified;

    if (prefs.clampvalaltmax() < llaCoord.alt())
    {
      llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), prefs.clampvalaltmax());
      modified = PlatformTspiFilterManager::POINT_CHANGED;
    }

    if (prefs.clampvalaltmin() > llaCoord.alt())
    {
      llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), prefs.clampvalaltmin());
      modified = PlatformTspiFilterManager::POINT_CHANGED;
    }

    return modified;
  }
};

//-----------------------------------------------------------------------------------------------------------------------------

/// Platform filtering for Orientation Clamping
class OrientationClamping : public PlatformTspiFilter
{
public:
  OrientationClamping()
    : PlatformTspiFilter()
  {
  }

  virtual ~OrientationClamping()
  {
  }

  virtual bool isApplicable(const simData::PlatformPrefs& prefs) const
  {
    return prefs.useclampyaw() || prefs.useclamppitch() || prefs.useclamproll() || prefs.clamporientationatlowvelocity();
  }

  virtual PlatformTspiFilterManager::FilterResponse filter(simCore::Coordinate& llaCoord, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props)
  {
    bool autoClamp = false;
    if (prefs.clamporientationatlowvelocity())
      autoClamp = simCore::v3Length(llaCoord.velocity()) < 0.001;

    if (!prefs.useclampyaw() && !prefs.useclamppitch() && !prefs.useclamproll() && !autoClamp)
      return PlatformTspiFilterManager::POINT_UNCHANGED;

    double yaw = llaCoord.yaw();
    double pitch = llaCoord.pitch();
    double roll = llaCoord.roll();

    if (prefs.useclampyaw() || autoClamp)
      yaw =  prefs.clampvalyaw();

    if (prefs.useclamppitch() || autoClamp)
      pitch = prefs.clampvalpitch();

    if (prefs.useclamproll() || autoClamp)
      roll = prefs.clampvalroll();

    llaCoord.setOrientation(yaw, pitch, roll);
    return PlatformTspiFilterManager::POINT_CHANGED;
  }
};


//-----------------------------------------------------------------------------------------------------------------------------

PlatformTspiFilterManager::PlatformTspiFilterManager()
{
  // Order matters; the last filter to modify wins
  platformFilters_.push_back(new AltitudeMinMaxClamping());
  platformFilters_.push_back(new OrientationClamping());
}

PlatformTspiFilterManager::~PlatformTspiFilterManager()
{
  for (std::vector<PlatformTspiFilter*>::iterator it = platformFilters_.begin(); it != platformFilters_.end(); ++it)
    delete *it;
}

void PlatformTspiFilterManager::addFilter(PlatformTspiFilter* filter)
{
  std::vector<PlatformTspiFilter*>::iterator it = std::find(platformFilters_.begin(), platformFilters_.end(), filter);

  // Attempting to add a duplicate filter
  assert(it == platformFilters_.end());

  if (it == platformFilters_.end())
    platformFilters_.push_back(filter);
}

void PlatformTspiFilterManager::removeFilter(PlatformTspiFilter* filter)
{
  std::vector<PlatformTspiFilter*>::iterator it = std::find(platformFilters_.begin(), platformFilters_.end(), filter);

  // Attempting to remove a non-existing filter
  assert(it != platformFilters_.end());

  if (it != platformFilters_.end())
    platformFilters_.erase(it);
}

PlatformTspiFilterManager::FilterResponse PlatformTspiFilterManager::filter(simData::PlatformUpdate& update, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props)
{
  // See if a filter possibly applies before converting from ECEF to LLA
  std::vector<PlatformTspiFilter*> possibleFilters;
  for (std::vector<PlatformTspiFilter*>::const_iterator it = platformFilters_.begin(); it != platformFilters_.end(); ++it)
  {
    if ((*it)->isApplicable(prefs))
      possibleFilters.push_back(*it);
  }

  // No filter wants to look at the data
  if (possibleFilters.empty())
  {
    return PlatformTspiFilterManager::POINT_UNCHANGED;
  }

  simCore::Coordinate ecefCoord(toCoordinate_(update));
  simCore::Coordinate llaCoord;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord);

  PlatformTspiFilterManager::FilterResponse modified = PlatformTspiFilterManager::POINT_UNCHANGED;
  for (std::vector<PlatformTspiFilter*>::const_iterator it = platformFilters_.begin(); it != platformFilters_.end(); ++it)
  {
    PlatformTspiFilterManager::FilterResponse rv = (*it)->filter(llaCoord, prefs, props);
    if (rv == PlatformTspiFilterManager::POINT_DROPPED)
      return rv;

    if (rv == PlatformTspiFilterManager::POINT_CHANGED)
      modified = rv;
  }

  if (modified == PlatformTspiFilterManager::POINT_CHANGED)
  {
    simCore::CoordinateConverter::convertGeodeticToEcef(llaCoord, ecefCoord);
    toPlatformUpdate_(ecefCoord, update);
  }

  return modified;
}

simCore::Coordinate PlatformTspiFilterManager::toCoordinate_(const simData::PlatformUpdate& update) const
{
  simCore::Coordinate rv(simCore::COORD_SYS_ECEF, simCore::Vec3(update.x(), update.y(), update.z()));

  if (update.has_orientation())
    rv.setOrientation(simCore::Vec3(update.psi(), update.theta(), update.phi()));

  if (update.has_velocity())
    rv.setVelocity(simCore::Vec3(update.vx(), update.vy(), update.vz()));

  return rv;
}

void PlatformTspiFilterManager::toPlatformUpdate_(const simCore::Coordinate& coord, simData::PlatformUpdate& update) const
{
  update.setPosition(coord.position());
  if (coord.hasOrientation())
    update.setOrientation(coord.orientation());
  if (coord.hasVelocity())
    update.setVelocity(coord.velocity());
}


}


