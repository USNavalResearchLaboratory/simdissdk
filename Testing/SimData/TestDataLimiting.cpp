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
#include <cmath>
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"

// anonymous namespace replaces static
namespace
{


void addPlatformCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::PlatformCommand *u = dataStore->addPlatformCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addBeamCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::BeamCommand *u = dataStore->addBeamCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addGateCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::GateCommand *u = dataStore->addGateCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addLaserCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::LaserCommand *u = dataStore->addLaserCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addProjectorCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::ProjectorCommand *u = dataStore->addProjectorCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addLobCommand(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::LobGroupCommand *u = dataStore->addLobGroupCommand(id, &t);
  u->set_time(time);
  u->mutable_updateprefs()->mutable_commonprefs()->set_draw(fmod(time, 2) < 1.0);
  t.commit();
}

void addGenericData(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::GenericData *u = dataStore->addGenericData(id, &t);
  simData::GenericData::Entry *e = u->add_entry();
  e->set_key("Test");
  e->set_value("TestValue");
  t.commit();
}

void addCategoryData(simData::DataStore *dataStore, uint64_t id, double time)
{
  simData::DataStore::Transaction t;
  simData::CategoryData *u = dataStore->addCategoryData(id, &t);
  simData::CategoryData::Entry *e = u->add_entry();
  e->set_key("TestCat");
  e->set_value("TestCatValue");
  t.commit();
}


template<typename T>
struct DsHelper
{
typedef T* (simData::DataStore::*QueryFun)(uint64_t);
};

class TestHelper
{
public:
  TestHelper()
  {
    ds_ = testHelper_.dataStore();
    platformId_ = 1;
    beamId_ = 2;
    gateId_ = 3;
    lobId_ = 4;
    laserId_ = 5;
    projId_ = 6;
  }

  void init()
  {
    platformId_ = testHelper_.addPlatform();
    beamId_ = testHelper_.addBeam(platformId_);
    gateId_ = testHelper_.addGate(beamId_);
    lobId_ = testHelper_.addLOB(platformId_);
    laserId_ = testHelper_.addLaser(platformId_);
    projId_ = testHelper_.addProjector(platformId_);

    fillData_();
  }

  // T must be a DataSlice
  template<typename T>
  int runEntityTest(const T *constSlice, simData::ObjectId id)
  {
    T *slice = const_cast<T*>(constSlice);
    int rv = 0;
    simData::CommonPrefs prefs;

    // test with limits unset (should have no effect)
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 10);

    // test with negative time limit (should have no effect)
    prefs.set_datalimittime(-1);
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 10);

    // limit by points
    prefs.set_datalimitpoints(9);
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 9);

    // limit by time
    prefs.set_datalimittime(4);
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 4);

    // should have no effect
    prefs.set_datalimitpoints(0);
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 4);

    // should limit to 1 point
    prefs.set_datalimittime(0);
    slice->limitByPrefs(prefs);
    rv += SDK_ASSERT(slice->numItems() == 1);

    // make sure last point is the one that was saved
    rv += SDK_ASSERT(slice->lower_bound(20.0).peekPrevious()->time() == 19.0);

    // flush
    ds_->flush(id);
    rv += SDK_ASSERT(slice->numItems() == 0);

    return rv;
  }

  int runTest()
  {
    int rv = 0;

    ds_->update(19); // advance to end

    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::PlatformUpdate>*>(ds_->platformUpdateSlice(platformId_)), platformId_);
    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::BeamUpdate>*>(ds_->beamUpdateSlice(beamId_)), beamId_);
    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::GateUpdate>*>(ds_->gateUpdateSlice(gateId_)), gateId_);
    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::LobGroupUpdate>*>(ds_->lobGroupUpdateSlice(lobId_)), lobId_);
    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::LaserUpdate>*>(ds_->laserUpdateSlice(laserId_)), laserId_);
    rv += runEntityTest(dynamic_cast<const simData::MemoryDataSlice<simData::ProjectorUpdate>*>(ds_->projectorUpdateSlice(projId_)), projId_);

    // TODO (investigate - tests were failing for some reason, early in development)
    //rv += runEntityTest(ds_.genericDataSlice(platformId_));
    //rv += runEntityTest(ds_.categoryDataSlice(platformId_));

    // test static flush
    uint64_t id = testHelper_.addPlatform();
    testHelper_.addPlatformUpdate(-1, id);
    const simData::PlatformUpdateSlice *slice = ds_->platformUpdateSlice(id);
    ds_->flush(0);
    rv += SDK_ASSERT(slice->numItems() == 1);

    return rv;
  }

protected: // functions
  void fillData_()
  {
    for (uint32_t i = 0; i < 10; ++i)
    {
      const double t = i + 10.0;
      testHelper_.addPlatformUpdate(t, platformId_);
      addPlatformCommand(ds_, platformId_, t);

      testHelper_.addBeamUpdate(t, beamId_);
      addBeamCommand(ds_, beamId_, t);

      testHelper_.addGateUpdate(t, gateId_);
      addGateCommand(ds_, gateId_, t);

      testHelper_.addLOBUpdate(t, lobId_);
      addLobCommand(ds_, lobId_, t);

      testHelper_.addLaserUpdate(t, laserId_);
      addLaserCommand(ds_, laserId_, t);

      testHelper_.addProjectorUpdate(t, projId_);
      addProjectorCommand(ds_, projId_, t);

      addGenericData(ds_, platformId_, t);
      addCategoryData(ds_, platformId_, t);
    }
  }

protected: // data
  simUtil::DataStoreTestHelper testHelper_;
  simData::DataStore* ds_;
  uint64_t platformId_;
  uint64_t beamId_;
  uint64_t gateId_;
  uint64_t lobId_;
  uint64_t laserId_;
  uint64_t projId_;
};
} // anonymous namespace

int TestDataLimiting(int argc, char *argv[])
{
  int rv = 0;
  simCore::checkVersionThrow();

  TestHelper th;
  th.init();

  rv += th.runTest();

  return rv;
}

