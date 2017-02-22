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

  initializeDefaultPlatformPrefs(platformPrefs);
  dataStore.setDefaultPrefs(platformPrefs, simData::BeamPrefs(), simData::GatePrefs(), simData::LaserPrefs(), simData::LobGroupPrefs(), simData::ProjectorPrefs());
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
}

}
