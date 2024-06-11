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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Time/Utils.h"
#include "simData/DataStore.h"
#include "DefaultDataStoreValues.h"

namespace simUtil {

void DefaultEntityPrefs::initializeDataStorePrefs(simData::DataStore& dataStore)
{
  // most of the default values are in the protobuf definitions

  simData::PlatformPrefs platformPrefs;
  DefaultEntityPrefs::initializeDefaultPlatformPrefs(platformPrefs);
  simData::LaserPrefs laserPrefs;
  DefaultEntityPrefs::initializeDefaultLaserPrefs(laserPrefs);
  simData::LobGroupPrefs lobPrefs;
  DefaultEntityPrefs::initializeDefaultLobGroupPrefs(lobPrefs);
  dataStore.setDefaultPrefs(platformPrefs, simData::BeamPrefs(), simData::GatePrefs(), laserPrefs, lobPrefs, simData::ProjectorPrefs());
}

void DefaultEntityPrefs::initializeDefaultPlatformPrefs(simData::PlatformPrefs& prefs)
{
  // Platforms-only get default hover settings for position and course/speed on
  simData::LabelPrefs_DisplayFields* hoverFields = prefs.mutable_commonprefs()->mutable_labelprefs()->mutable_hoverdisplayfields();
  hoverFields->set_xlat(true);
  hoverFields->set_ylon(true);
  hoverFields->set_zalt(true);
  hoverFields->set_yaw(true);
  hoverFields->set_speed(true);

  // Default hook window content
  simData::LabelPrefs_DisplayFields* hookFields = prefs.mutable_commonprefs()->mutable_labelprefs()->mutable_hookdisplayfields();
  hookFields->set_xlat(true);
  hookFields->set_ylon(true);
  hookFields->set_zalt(true);
  hookFields->set_genericdata(true);
  hookFields->set_categorydata(true);
  hookFields->set_yaw(true);
  hookFields->set_pitch(true);
  hookFields->set_roll(true);
  hookFields->set_displayvx(true);
  hookFields->set_displayvy(true);
  hookFields->set_displayvz(true);
  hookFields->set_speed(true);
}

void DefaultEntityPrefs::initializeDefaultLaserPrefs(simData::LaserPrefs& prefs)
{
  // Lasers default to red, not yellow (colors are 0xRRGGBBAA in protobuf)
  prefs.mutable_commonprefs()->set_color(0xff0000ff);
}

void DefaultEntityPrefs::initializeDefaultLobGroupPrefs(simData::LobGroupPrefs& prefs)
{
  // LOBs get default hover settings for position and az/el on
  simData::LabelPrefs_DisplayFields* hoverFields = prefs.mutable_commonprefs()->mutable_labelprefs()->mutable_hoverdisplayfields();
  hoverFields->set_xlat(true);
  hoverFields->set_ylon(true);
  hoverFields->set_zalt(true);
  hoverFields->set_yaw(true);
  hoverFields->set_pitch(true);

  // Default hook window content should show position, az/el, and generic/category values
  simData::LabelPrefs_DisplayFields* hookFields = prefs.mutable_commonprefs()->mutable_labelprefs()->mutable_hookdisplayfields();
  hookFields->set_xlat(true);
  hookFields->set_ylon(true);
  hookFields->set_zalt(true);
  hookFields->set_genericdata(true);
  hookFields->set_categorydata(true);
  hookFields->set_yaw(true);
  hookFields->set_pitch(true);
}

}
