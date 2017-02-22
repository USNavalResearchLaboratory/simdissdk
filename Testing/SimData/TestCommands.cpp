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
#include "simCore.h"
#include "simData.h"
#include "simUtil/DataStoreTestHelper.h"

namespace {

/// Returns the icon setting for a particular platform
std::string icon(simData::DataStore* ds, uint64_t id)
{
  simData::DataStore::Transaction t;
  const simData::PlatformPrefs *pp = ds->platformPrefs(id, &t);
  return (pp ? pp->icon() : "");
}

/// Returns the draw setting for a particular platform
bool draw(simData::DataStore* ds, uint64_t id)
{
  simData::DataStore::Transaction t;
  const simData::PlatformPrefs *pp = ds->platformPrefs(id, &t);
  assert(pp);
  return (pp ? pp->commonprefs().draw() : false);
}

bool labelDraw(simData::DataStore* ds, uint64_t id)
{
  simData::DataStore::Transaction t;
  const simData::PlatformPrefs *pp = ds->platformPrefs(id, &t);
  assert(pp);
  return (pp ? pp->commonprefs().labelprefs().draw() : false);
}

/// Tests the command executer for platforms
int testCommand()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  // set draw to true
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->set_draw(true);
  newPlatPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  int rv = 0;
  rv += SDK_ASSERT(icon(ds, platId1) == "icon1"); // this is the default icon name

  // Add a few commands
  simData::PlatformCommand* cmd;
  // Icon at time 5
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(5.0);
  cmd->mutable_updateprefs()->set_icon("icon5");
  cmd->mutable_updateprefs()->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.complete(&cmd);
  // Icon at time 15
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(15.0);
  cmd->mutable_updateprefs()->set_icon("icon15");
  t.complete(&cmd);
  // Icon at time 10
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(10.0);
  cmd->mutable_updateprefs()->set_icon("icon10");
  // Draw is a sparse command in this test -- sets at time 10, should apply even after update 15
  cmd->mutable_updateprefs()->mutable_commonprefs()->set_draw(false);
  cmd->mutable_updateprefs()->mutable_commonprefs()->mutable_labelprefs()->set_draw(false);
  t.complete(&cmd);

  // Should still point to icon1
  rv += SDK_ASSERT(icon(ds, platId1) == "icon1");
  rv += SDK_ASSERT(draw(ds, platId1) == true);
  rv += SDK_ASSERT(labelDraw(ds, platId1) == true);
  // Update the data store to time 5; expect icon 5
  ds->update(5.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "icon5");
  rv += SDK_ASSERT(draw(ds, platId1) == true);
  rv += SDK_ASSERT(labelDraw(ds, platId1) == true);
  // Update the data store to time 100; expect icon 15
  ds->update(100.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "icon15");
  rv += SDK_ASSERT(draw(ds, platId1) == false);
  rv += SDK_ASSERT(labelDraw(ds, platId1) == false);
  // Update the data store to time 7.5; expect icon 5
  ds->update(7.5);
  rv += SDK_ASSERT(icon(ds, platId1) == "icon5");
  rv += SDK_ASSERT(draw(ds, platId1) == false);  // Still false because no command exists to turn it back on
  rv += SDK_ASSERT(labelDraw(ds, platId1) == true);
  // Update the data store to time 10; expect icon 10
  ds->update(10.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "icon10");
  rv += SDK_ASSERT(draw(ds, platId1) == false);
  rv += SDK_ASSERT(labelDraw(ds, platId1) == false);

  return rv;
}

int validateGateColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  if (time != -1)
    ds->update(time);
  simData::DataStore::Transaction t;
  const simData::GatePrefs *prefs = ds->gatePrefs(id, &t);
  if (prefs == NULL)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addGateColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::GateCommand *command = ds->addGateCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::GatePrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_color(color);
  t.complete(&command);

  return 0;
}

int addGateOverrideColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::GateCommand *command = ds->addGateCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::GatePrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_overridecolor(color);
  t.complete(&command);

  return 0;
}

int testGateCommand()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  // set draw to true
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->set_draw(true);
  newPlatPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  // insert beam
  uint64_t beamId1 = testHelper.addBeam(platId1);
  // set draw to true
  simData::BeamPrefs* newBeamPrefs = ds->mutable_beamPrefs(beamId1, &t);
  newBeamPrefs->mutable_commonprefs()->set_draw(true);
  newBeamPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  // insert gate
  uint64_t gateId1 = testHelper.addGate(beamId1);
  // set draw to true
  simData::GatePrefs* newGatePrefs = ds->mutable_gatePrefs(gateId1, &t);
  newGatePrefs->mutable_commonprefs()->set_draw(true);
  newGatePrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  // Verify default color before adding any commands
  rv += validateGateColor(ds, gateId1, 0.0, 0xFFFF00FF);

  // Add a new color at time 1
  rv += addGateColor(ds, gateId1, 1.0, 0x1);
  rv += validateGateColor(ds, gateId1, 1.0, 0x1);

  // Go back before first command and will get the last color and NOT the default color
  rv += validateGateColor(ds, gateId1, 0.0, 0x1);

  // Add a new color at time 5
  rv += addGateColor(ds, gateId1, 5.0, 0x5);
  rv += validateGateColor(ds, gateId1, 5.0, 0x5);

  // Go back and add a color at time 4
  rv += addGateColor(ds, gateId1, 4.0, 0x4);
  // Do not update time for this call
  rv += validateGateColor(ds, gateId1, -1.0, 0x5);
  // Update time on this call
  rv += validateGateColor(ds, gateId1, 6.0, 0x5);
  // Update time back to the 4
  rv += validateGateColor(ds, gateId1, 4.0, 0x4);
  // Update to time 6 which does not have a color so it will return time 5's color (5)
  rv += validateGateColor(ds, gateId1, 6.0, 0x5);
  // Now add color at the current time
  rv += addGateColor(ds, gateId1, 6.0, 0x6);
  // do not call update, so the color will not update to the color of time 6 (this might be considered a bug)
  rv += validateGateColor(ds, gateId1, -1.0, 0x5);
  // Call update this time and color will update
  rv += validateGateColor(ds, gateId1, 6.0, 0x6);

  // Add two commands for the same time to verify the right one is picked up
  // Add a override color
  rv += addGateOverrideColor(ds, gateId1, 7.0, 0xF007);
  // Update to 7.0
  ds->update(7.0);
  // Now add color at 7.0
  rv += addGateColor(ds, gateId1, 7.0, 0x7);
  // do not call update, so the color will not update to the color of time 7
  rv += validateGateColor(ds, gateId1, -1.0, 0x6);
  // Call update this time and color will update; this test verifies the right one is picked up
  rv += validateGateColor(ds, gateId1, 7.0, 0x7);

  return rv;
}



int validateBeamColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  if (time != -1)
    ds->update(time);
  simData::DataStore::Transaction t;
  const simData::BeamPrefs *prefs = ds->beamPrefs(id, &t);
  if (prefs == NULL)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addBeamColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::BeamCommand *command = ds->addBeamCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::BeamPrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_color(color);
  t.complete(&command);

  return 0;
}

int addBeamOverrideColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::BeamCommand *command = ds->addBeamCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::BeamPrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_overridecolor(color);
  t.complete(&command);

  return 0;
}

int testBeamCommand()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  // set draw to true
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->set_draw(true);
  newPlatPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  // insert beam
  uint64_t beamId1 = testHelper.addBeam(platId1);
  // set draw to true
  simData::BeamPrefs* newBeamPrefs = ds->mutable_beamPrefs(beamId1, &t);
  newBeamPrefs->mutable_commonprefs()->set_draw(true);
  newBeamPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();

  // Verify default color before adding any commands
  rv += validateBeamColor(ds, beamId1, 0.0, 0xFFFF00FF);

  // Add a new color at time 1
  rv += addBeamColor(ds, beamId1, 1.0, 0x1);
  rv += validateBeamColor(ds, beamId1, 1.0, 0x1);

  // Go back before first command and will get the last color and NOT the default color
  rv += validateBeamColor(ds, beamId1, 0.0, 0x1);

  // Add a new color at time 5
  rv += addBeamColor(ds, beamId1, 5.0, 0x5);
  rv += validateBeamColor(ds, beamId1, 5.0, 0x5);

  // Go back and add a color at time 4
  rv += addBeamColor(ds, beamId1, 4.0, 0x4);
  // Do not update time for this call
  rv += validateBeamColor(ds, beamId1, -1.0, 0x5);
  // Update time on this call
  rv += validateBeamColor(ds, beamId1, 6.0, 0x5);
  // Update time back to the 4
  rv += validateBeamColor(ds, beamId1, 4.0, 0x4);
  // Update to time 6 which does not have a color so it will return time 5's color (5)
  rv += validateBeamColor(ds, beamId1, 6.0, 0x5);
  // Now add color at the current time
  rv += addBeamColor(ds, beamId1, 6.0, 0x6);
  // do not call update, so the color will not update to the color of time 6 (this might be considered a bug)
  rv += validateBeamColor(ds, beamId1, -1.0, 0x5);
  // Call update this time and color will update
  rv += validateBeamColor(ds, beamId1, 6.0, 0x6);

  // Add two commands for the same time to verify the right one is picked up
  // Add a override color
  rv += addBeamOverrideColor(ds, beamId1, 7.0, 0xF007);
  // Update to 7.0
  ds->update(7.0);
  // Now add color at 7.0
  rv += addBeamColor(ds, beamId1, 7.0, 0x7);
  // do not call update, so the color will not update to the color of time 7
  rv += validateBeamColor(ds, beamId1, -1.0, 0x6);
  // Call update this time and color will update; this test verifies the right one is picked up
  rv += validateBeamColor(ds, beamId1, 7.0, 0x7);

  return rv;
}

int validatePlatformColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  if (time != -1)
    ds->update(time);
  simData::DataStore::Transaction t;
  const simData::PlatformPrefs *prefs = ds->platformPrefs(id, &t);
  if (prefs == NULL)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addPlatformColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::PlatformCommand *command = ds->addPlatformCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::PlatformPrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_color(color);
  t.complete(&command);

  return 0;
}

int addPlatformOverrideColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::PlatformCommand *command = ds->addPlatformCommand(id, &t);
  if (command == NULL)
    return 1;

  command->set_time(time);
  simData::PlatformPrefs* prefs = command->mutable_updateprefs();
  simData::CommonPrefs* commonPrefs = prefs->mutable_commonprefs();
  commonPrefs->set_overridecolor(color);
  t.complete(&command);

  return 0;
}

int testPlatformCommand()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  // set draw to true
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->set_draw(true);
  newPlatPrefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  t.commit();


  // Verify default color before adding any commands
  rv += validatePlatformColor(ds, platId1, 0.0, 0xFFFF00FF);

  // Add a new color at time 1
  rv += addPlatformColor(ds, platId1, 1.0, 0x1);
  rv += validatePlatformColor(ds, platId1, 1.0, 0x1);

  // Go back before first command and will get the last color and NOT the default color
  rv += validatePlatformColor(ds, platId1, 0.0, 0x1);

  // Add a new color at time 5
  rv += addPlatformColor(ds, platId1, 5.0, 0x5);
  rv += validatePlatformColor(ds, platId1, 5.0, 0x5);

  // Go back and add a color at time 4
  rv += addPlatformColor(ds, platId1, 4.0, 0x4);
  // Do not update time for this call
  rv += validatePlatformColor(ds, platId1, -1.0, 0x5);
  // Update time on this call
  rv += validatePlatformColor(ds, platId1, 6.0, 0x5);
  // Update time back to the 4
  rv += validatePlatformColor(ds, platId1, 4.0, 0x4);
  // Update to time 6 which does not have a color so it will return time 5's color (5)
  rv += validatePlatformColor(ds, platId1, 6.0, 0x5);
  // Now add color at the current time
  rv += addPlatformColor(ds, platId1, 6.0, 0x6);
  // do not call update, so the color will not update to the color of time 6 (this might be considered a bug)
  rv += validatePlatformColor(ds, platId1, -1.0, 0x5);
  // Call update this time and color will update
  rv += validatePlatformColor(ds, platId1, 6.0, 0x6);

  // Add two commands for the same time to verify the right one is picked up
  // Add a override color
  rv += addPlatformOverrideColor(ds, platId1, 7.0, 0xF007);
  // Update to 7.0
  ds->update(7.0);
  // Now add color at 7.0
  rv += addPlatformColor(ds, platId1, 7.0, 0x7);
  // do not call update, so the color will not update to the color of time 7
  rv += validatePlatformColor(ds, platId1, -1.0, 0x6);
  // Call update this time and color will update; this test verifies the right one is picked up
  rv += validatePlatformColor(ds, platId1, 7.0, 0x7);

  return rv;
}

}

int TestCommands(int argc, char* argv[])
{
  int rv = 0;
  rv += testCommand();
  rv += testGateCommand();
  rv += testBeamCommand();
  rv += testPlatformCommand();

  return rv;
}
