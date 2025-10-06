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
#include "simCore/Common/SDKAssert.h"
#include "simData/DataStoreHelpers.h"
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

/// Returns the color setting for a particular platform
uint32_t color(simData::DataStore* ds, uint64_t id)
{
  simData::DataStore::Transaction t;
  const simData::PlatformPrefs* pp = ds->platformPrefs(id, &t);
  assert(pp);
  return (pp ? pp->commonprefs().color() : 0);
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
  if (prefs == nullptr)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addGateColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::GateCommand *command = ds->addGateCommand(id, &t);
  if (command == nullptr)
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
  if (command == nullptr)
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
  if (prefs == nullptr)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addBeamColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::BeamCommand *command = ds->addBeamCommand(id, &t);
  if (command == nullptr)
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
  if (command == nullptr)
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
  if (prefs == nullptr)
    return 1;

  if (prefs->commonprefs().color() != color)
    return 1;

  return 0;
}

int addPlatformColor(simData::DataStore* ds, simData::ObjectId id, double time, uint32_t color)
{
  simData::DataStore::Transaction t;
  simData::PlatformCommand *command = ds->addPlatformCommand(id, &t);
  if (command == nullptr)
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
  if (command == nullptr)
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

int validateAcceptProjectorIds(const simData::DataStore* ds, simData::ObjectId id, const std::vector<simData::ObjectId>& expectedValues)
{
  int rv = 0;
  rv += SDK_ASSERT(ds != nullptr);
  simData::DataStore::Transaction txn;
  auto* prefs = ds->commonPrefs(id, &txn);
  rv += SDK_ASSERT(prefs != nullptr);
  const auto& actualValues = prefs->acceptprojectorids();
  rv += SDK_ASSERT(actualValues == expectedValues);
  return rv;
}

int testAcceptProjectorsPrefs()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  ds->update(0.1);

  // Confirm initial state
  rv += validateAcceptProjectorIds(ds, platId1, { });

  // Add one projector ID
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->add_acceptprojectorids(4);
  t.commit();

  // Confirm new value
  rv += validateAcceptProjectorIds(ds, platId1, { 4 });

  // Add a few commands
  simData::PlatformCommand* cmd;
  // "5,6" at time 5
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(5.0);
  *cmd->mutable_updateprefs()->mutable_commonprefs()->mutable_acceptprojectorids() = { 5, 6 };
  t.complete(&cmd);
  // "6,15" at time 15
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(15.0);
  *cmd->mutable_updateprefs()->mutable_commonprefs()->mutable_acceptprojectorids() = { 6, 15 };
  t.complete(&cmd);
  // "10" only, at time 10
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(10.0);
  *cmd->mutable_updateprefs()->mutable_commonprefs()->mutable_acceptprojectorids() = { 10 };
  t.complete(&cmd);


  // Since time hasn't updated, we shouldn't have any changes -- nothing prior to time 5
  rv += validateAcceptProjectorIds(ds, platId1, { 4 });
  ds->update(1.);
  rv += validateAcceptProjectorIds(ds, platId1, { 4 });

  // Check time 5
  ds->update(5.);
  rv += validateAcceptProjectorIds(ds, platId1, { 5, 6 });
  // Check time 10
  ds->update(10.);
  rv += validateAcceptProjectorIds(ds, platId1, { 10 });
  // Check time 15
  ds->update(15.);
  rv += validateAcceptProjectorIds(ds, platId1, { 6, 15 });
  // Back to time 2, no commands before this, so we should have same value
  ds->update(2.);
  rv += validateAcceptProjectorIds(ds, platId1, { 6, 15 });

  // Clear out the projector IDs and confirm
  newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->clear_acceptprojectorids();
  t.commit();
  rv += validateAcceptProjectorIds(ds, platId1, { });

  return rv;
}

int testAcceptProjectorsCommands()
{
  // Intended to duplicate a failure seen in SIMDIS where a 3 commands sent over in
  // serial resulted in the command structure to have 3 different values, instead of one.
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  ds->update(2.5);

  // Confirm initial state
  rv += validateAcceptProjectorIds(ds, platId1, { });

  // Add a projector ID at time 2.5
  auto* platCommand = ds->addPlatformCommand(platId1, &t);
  platCommand->set_time(2.5);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 0);
  platCommand->mutable_updateprefs()->mutable_commonprefs()->add_acceptprojectorids(4);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  t.commit();

  // Validate command slice state
  const auto* commandSlice = ds->platformCommandSlice(platId1);
  rv += SDK_ASSERT(commandSlice->numItems() == 1);
  rv += SDK_ASSERT(commandSlice->firstTime() == 2.5);
  rv += SDK_ASSERT(commandSlice->current() == nullptr);
  ds->update(2.5);
  rv += SDK_ASSERT(commandSlice->current() != nullptr);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids().at(0) == 4);

  // Change the command to point to 0
  platCommand = ds->addPlatformCommand(platId1, &t);
  platCommand->set_time(2.5);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 0);
  platCommand->mutable_updateprefs()->mutable_commonprefs()->add_acceptprojectorids(0);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  t.commit();
  ds->update(2.5);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids().at(0) == 0);

  // Change the command to point to 4 and 5
  platCommand = ds->addPlatformCommand(platId1, &t);
  platCommand->set_time(2.5);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 0);
  platCommand->mutable_updateprefs()->mutable_commonprefs()->add_acceptprojectorids(4);
  platCommand->mutable_updateprefs()->mutable_commonprefs()->add_acceptprojectorids(5);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 2);
  t.commit();
  ds->update(2.5);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids_size() == 2);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids().at(0) == 4);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids().at(1) == 5);

  // Change the command to point to 0
  platCommand = ds->addPlatformCommand(platId1, &t);
  platCommand->set_time(2.5);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 0);
  platCommand->mutable_updateprefs()->mutable_commonprefs()->add_acceptprojectorids(0);
  rv += SDK_ASSERT(platCommand->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  t.commit();
  ds->update(2.5);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids_size() == 1);
  rv += SDK_ASSERT(commandSlice->current()->updateprefs().commonprefs().acceptprojectorids().at(0) == 0);
  return rv;
}

/// Tests the command executer for platforms given different time conditions

int testCommandTiming()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();

  // set name
  simData::PlatformPrefs* newPlatPrefs = ds->mutable_platformPrefs(platId1, &t);
  newPlatPrefs->mutable_commonprefs()->set_name("Joe");
  t.complete(&newPlatPrefs);

  int rv = 0;

  // move to time around middle of 2022, where double issues started to manifest in MemoryCommandSlice
  double curTime = 1682723805.0;

  ds->update(curTime);

  simData::PlatformCommand* cmd;

  // name update at time 1 second behind current data time
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(curTime - 1.);
  cmd->mutable_updateprefs()->mutable_commonprefs()->set_name("Bill");
  t.complete(&cmd);

  curTime += 1.;

  ds->update(curTime);
  {
    const simData::PlatformPrefs* pp = ds->platformPrefs(platId1, &t);
    // since this is the first command added, it should always succeed
    rv += SDK_ASSERT(pp->commonprefs().name() == "Bill");
  }

  // another name update at time 1 second behind current data time
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(curTime - 1.);
  cmd->mutable_updateprefs()->mutable_commonprefs()->set_name("Sally");
  t.complete(&cmd);

  curTime += 1.;

  ds->update(curTime);
  {
    const simData::PlatformPrefs* pp = ds->platformPrefs(platId1, &t);
    // now the MemoryCommandSlice is going to need to apply the next sequentially inserted command, even though it's behind current scenario time
    rv += SDK_ASSERT(pp->commonprefs().name() == "Sally");
  }

  // name at time 1 seconds ahead of current data time
  cmd = ds->addPlatformCommand(platId1, &t);
  cmd->set_time(curTime + 1.);
  cmd->mutable_updateprefs()->mutable_commonprefs()->set_name("Sue");
  t.complete(&cmd);

  // move ahead only a half second
  curTime += 0.5;

  ds->update(curTime);
  {
    const simData::PlatformPrefs* pp = ds->platformPrefs(platId1, &t);
    // newest command should not have been applied yet
    rv += SDK_ASSERT(pp->commonprefs().name() == "Sally");
  }

  // move ahead another half second to reach the next command time
  curTime += 0.5;

  ds->update(curTime);
  {
    const simData::PlatformPrefs* pp = ds->platformPrefs(platId1, &t);
    // newest command should now be applied
    rv += SDK_ASSERT(pp->commonprefs().name() == "Sue");
  }

  return rv;
}

int testClear()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  testHelper.addPlatformUpdate(0, platId1);
  testHelper.addPlatformUpdate(100, platId1);

  simData::PlatformCommand command;
  command.mutable_updateprefs()->set_icon("1");
  command.mutable_updateprefs()->mutable_commonprefs()->set_color(1);
  command.set_time(1.0);
  testHelper.addPlatformCommand(command, platId1);

  command.set_time(2.0);
  command.set_isclearcommand(true);
  command.mutable_updateprefs()->mutable_commonprefs()->clear_color();
  testHelper.addPlatformCommand(command, platId1);

  command.mutable_updateprefs()->set_icon("3");
  command.set_time(3.0);
  command.set_isclearcommand(false);
  testHelper.addPlatformCommand(command, platId1);

  // Default values set by the DataStoreTestHelper
  ds->update(0.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "icon1");
  rv += SDK_ASSERT(color(ds, platId1) == 0xffff00ff); // Yellow

  // First command
  ds->update(1.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "1");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // Clear command only affects the icon
  ds->update(2.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // Third command
  ds->update(3.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "3");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // Go back in time, but the default values are lost since they were overwritten by a command
  ds->update(0.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "3");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // First command
  ds->update(1.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "1");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // Clear command only affects the icon
  ds->update(2.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  // Third command
  ds->update(3.0);
  rv += SDK_ASSERT(icon(ds, platId1) == "3");
  rv += SDK_ASSERT(color(ds, platId1) == 1);

  return rv;
}

int testModify()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId1 = testHelper.addPlatform();
  testHelper.addPlatformUpdate(0, platId1);
  testHelper.addPlatformUpdate(100, platId1);

  simData::PlatformCommand command;
  command.mutable_updateprefs()->set_icon("1");
  command.mutable_updateprefs()->mutable_commonprefs()->set_color(1);
  command.set_time(1.0);
  testHelper.addPlatformCommand(command, platId1);

  command.set_time(2.0);
  command.set_isclearcommand(true);
  command.mutable_updateprefs()->mutable_commonprefs()->clear_color();
  testHelper.addPlatformCommand(command, platId1);

  command.mutable_updateprefs()->set_icon("3");
  command.set_time(3.0);
  command.set_isclearcommand(false);
  testHelper.addPlatformCommand(command, platId1);

  /** Remove icons from platform commands*/
  class RemoveIconCommand : public simData::VisitableDataSlice<simData::PlatformCommand>::Modifier
  {
  public:

    RemoveIconCommand()
    {
    }

    virtual ~RemoveIconCommand()
    {
    }

    virtual int modify(simData::FieldList& message) override
    {
      auto command = dynamic_cast<simData::PlatformCommand*>(&message);
      // Wrong message was passed in
      assert(command != nullptr);
      if (command == nullptr)
        return 0;

      if (!command->has_updateprefs())
        return 0;

      if (!command->updateprefs().has_icon())
        return 0;

      command->mutable_updateprefs()->clear_icon();
      return -1;
    }
  };

  /** Remove Color from platform commands*/
  class RemoveColorCommand : public simData::VisitableDataSlice<simData::PlatformCommand>::Modifier
  {
  public:

    RemoveColorCommand()
    {
    }

    virtual ~RemoveColorCommand()
    {
    }

    virtual int modify(simData::FieldList& message) override
    {
      auto command = dynamic_cast<simData::PlatformCommand*>(&message);
      // Wrong message was passed in
      assert(command != nullptr);
      if (command == nullptr)
        return 0;

      if (!command->has_updateprefs())
        return 0;

      if (!command->updateprefs().has_commonprefs())
        return 0;

      if (!command->updateprefs().commonprefs().has_color())
        return 0;

      command->mutable_updateprefs()->mutable_commonprefs()->clear_color();
      return -1;
    }
  };

  // Should start with 3 commands
  rv += SDK_ASSERT(ds->platformCommandSlice(platId1)->numItems() == 3);

  // Remove 2 icon commands
  RemoveIconCommand remove;
  ds->modifyPlatformCommandSlice(platId1, &remove);

  // Should be one color command left
  rv += SDK_ASSERT(ds->platformCommandSlice(platId1)->numItems() == 1);

  // Remove the remaining color command
  RemoveColorCommand color;
  ds->modifyPlatformCommandSlice(platId1, &color);

  // All commands removed
  rv += SDK_ASSERT(ds->platformCommandSlice(platId1)->numItems() == 0);

  return rv;
}

}

int TestCommands(int argc, char* argv[])
{
  int rv = 0;
  rv += testCommandTiming();
  rv += testCommand();
  rv += testGateCommand();
  rv += testBeamCommand();
  rv += testPlatformCommand();
  rv += testAcceptProjectorsPrefs();
  rv += testAcceptProjectorsCommands();
  rv += testClear();
  rv += testModify();

  return rv;
}
