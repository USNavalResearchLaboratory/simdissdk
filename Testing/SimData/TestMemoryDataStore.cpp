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
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>

#include "simCore/Common/Version.h"
#include "simCore/Common/Common.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/SDKAssert.h"
#include "simUtil/DataStoreTestHelper.h"

using namespace std;

namespace {
class AssertionException : public std::exception
{
public:
  AssertionException()
  {
  }
};

void assertTrue(bool value)
{
  if (!value)
  {
    throw AssertionException();
  }
}

template <class T> void assertEquals(const T& expected, const T& actual)
{
  if (!(expected == actual))
  {
    throw AssertionException();
  }
}

void assertEquals(const std::string& expected, const std::string& actual)
{
  if (!(expected == actual))
  {
    throw AssertionException();
  }
}

template <class T> void assertNotEquals(const T& expected, const T& actual)
{
  if (expected == actual)
  {
    throw AssertionException();
  }
}

class TestListener : public simData::DataStore::DefaultListener
{
public:
  TestListener()
  : remove_(0),
    delete_(0)
  {
  }

  uint32_t removeCount() const
  {
    return remove_;
  }

  uint32_t deleteCount() const
  {
    return delete_;
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::DataStore::ObjectType ot)
  {
    ++remove_;
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source)
  {
    ++delete_;
  }
protected:
  uint32_t remove_;
  uint32_t delete_;
};
}

void testPlatform_insert()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::DataStore::Transaction t;

  uint64_t platId = testHelper.addPlatform();

  // insert data point
  {
    simData::PlatformUpdate* u = ds->addPlatformUpdate(platId, &t);
    u->set_time(1.0);
    u->set_x(10.0);
    u->set_y(11.0);
    u->set_z(12.0);
    t.commit();
  }

  // insert data point
  {
    simData::PlatformUpdate* u = ds->addPlatformUpdate(platId, &t);
    u->set_time(2.0);
    u->set_x(13.0);
    u->set_y(14.0);
    u->set_z(15.0);
    t.commit();
  }

  // retrieve data points
  struct PlatformSliceCopy : public simData::PlatformUpdateSlice::Visitor
  {
    vector<simData::PlatformUpdate> updates;
    virtual void operator()(const simData::PlatformUpdate *update)
    {
      updates.push_back(*update);
    }
  };

  PlatformSliceCopy psc;
  const simData::PlatformUpdateSlice *pslice = ds->platformUpdateSlice(platId);
  pslice->visit(&psc);

  // verify number of data points
  assertEquals(psc.updates.size(), (size_t)2);

  // verify data point values
  assertEquals(psc.updates[0].x(), 10.0);
  assertEquals(psc.updates[0].y(), 11.0);
  assertEquals(psc.updates[0].z(), 12.0);

  assertEquals(psc.updates[1].x(), 13.0);
  assertEquals(psc.updates[1].y(), 14.0);
  assertEquals(psc.updates[1].z(), 15.0);

  // update current time
  ds->update(1.0);
  const simData::PlatformUpdate *c1 = pslice->current();
  assertEquals(c1->x(), 10.0);
  assertEquals(c1->y(), 11.0);
  assertEquals(c1->z(), 12.0);

  ds->update(2.0);
  const simData::PlatformUpdate *c2 = pslice->current();
  assertEquals(c2->x(), 13.0);
  assertEquals(c2->y(), 14.0);
  assertEquals(c2->z(), 15.0);
}

void testPlatform_insertStatic()
{
  simUtil::DataStoreTestHelper testHelper;

  // insert platform
  uint64_t pID = testHelper.addPlatform();

  // insert data point
  testHelper.addPlatformUpdate(-1.0, pID);

  // insert second platform
  pID = testHelper.addPlatform();

  // insert data point
  testHelper.addPlatformUpdate(10, pID);

  std::pair<double, double> bounds = testHelper.dataStore()->timeBounds(0);
  assertEquals(bounds.first, 10.0);
  assertEquals(bounds.second, 10.0);
}

// callback when new lob is added
class NewLobListener : public simData::DataStore::DefaultListener
{
public:
  void onAddEntity(simData::DataStore *source, simData::ObjectId id, simData::DataStore::ObjectType ot)
  {
    if (ot != simData::DataStore::LOB_GROUP)
      return;

    int rv = 0;
    simData::DataStore::Transaction transaction;
    const simData::LobGroupProperties* props = source->lobGroupProperties(id, &transaction);
    rv += SDK_ASSERT(props != NULL);
    transaction.complete(&props);
    if (rv != 0)
      throw AssertionException();
  }
};

// callback when lob prefs updated
struct LobPrefListener : public simData::DataStore::DefaultListener
{
  void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    if (source->objectType(id) != simData::DataStore::LOB_GROUP)
      return;

    simData::DataStore::Transaction transaction;
    int rv = 0;
    const simData::LobGroupPrefs* prefs = source->lobGroupPrefs(id, &transaction);
    if (prefs)
    {
      // default value is initialized to 2, our test sets to 5, so check both values for validity
      rv += SDK_ASSERT(prefs->lobwidth() == 5 || prefs->lobwidth() == 2);
    }

    transaction.complete(&prefs);

    if (rv != 0)
      throw AssertionException();
  }
};

void testLobGroup_insert()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  int rv = 0;
  simData::DataStore::Transaction t;

  // test observer
  ds->addListener(simData::DataStore::ListenerPtr(new NewLobListener));
  ds->addListener(simData::DataStore::ListenerPtr(new LobPrefListener));

  // insert host platform
  uint64_t platId1 = testHelper.addPlatform();
  // insert lobgroup
  uint64_t lobId1 = testHelper.addLOB(platId1);
  std::string lobName1 = ds->lobGroupPrefs(lobId1, &t)->commonprefs().name();

  simData::LobGroupPrefs lobPrefs;
  lobPrefs.set_maxdatapoints(2);
  lobPrefs.set_maxdataseconds(2.0);
  lobPrefs.set_lobwidth(5);
  testHelper.updateLOBPrefs(lobPrefs, lobId1);

  // set prefs
  {
    simData::LobGroupCommand command;
    simData::LobGroupPrefs* cp = command.mutable_updateprefs();
    command.set_time(1.0);
    cp->set_maxdatapoints(2);
    cp->set_maxdataseconds(2.0);
    cp->set_color1(0xff00ff00);
    cp->set_color2(0x00ff00ff);
    cp->set_lobwidth(5);
    cp->set_stipple1(0x0FF0);
    cp->set_stipple2(0x00FF);
    testHelper.addLOBCommand(command, lobId1);
  }

  // insert another lobgroup
  uint64_t lobId2 = testHelper.addLOB(platId1);
  std::string lobName2 = ds->lobGroupPrefs(lobId2, &t)->commonprefs().name();
  simData::LobGroupPrefs lobPrefs2;
  lobPrefs2.set_maxdatapoints(4);
  lobPrefs2.set_maxdataseconds(8.0);
  lobPrefs2.set_lobwidth(5);
  testHelper.updateLOBPrefs(lobPrefs2, lobId2);

  // set prefs
  {
    simData::LobGroupCommand command;
    simData::LobGroupPrefs* cp = command.mutable_updateprefs();
    command.set_time(1.0);
    cp->set_maxdatapoints(4);
    cp->set_maxdataseconds(8.0);
    cp->set_color1(0xff00ff00);
    cp->set_color2(0x000000ff);
    cp->set_lobwidth(5);
    cp->set_stipple1(0x00F0);
    cp->set_stipple2(0xF0FF);
    testHelper.addLOBCommand(command, lobId2);
  }

  // check that lobs are in the data store
  simData::DataStore::IdList idList;
  ds->idListByName(lobName1, &idList, simData::DataStore::LOB_GROUP);
  rv += SDK_ASSERT(idList.size() == 1);
  rv += SDK_ASSERT(idList[0] == lobId1);

  idList.clear();
  ds->idListByName(lobName2, &idList, simData::DataStore::LOB_GROUP);
  rv += SDK_ASSERT(idList.size() == 1);
  rv += SDK_ASSERT(idList[0] == lobId2);

  idList.clear();
  ds->idList(&idList, simData::DataStore::LOB_GROUP);
  rv += SDK_ASSERT(idList.size() == 2);
  int numLobsFound = 0;
  for (size_t tIndex = 0; tIndex < idList.size(); tIndex++)
  {
    if (idList[tIndex] == lobId1 || idList[tIndex] == lobId2)
      numLobsFound++;
  }
  rv += SDK_ASSERT(numLobsFound == 2);

  // get lobs by host
  idList.clear();
  ds->lobGroupIdListForHost(platId1, &idList);
  rv += SDK_ASSERT(idList.size() == 2);

  // insert data point
  {
    simData::LobGroupUpdate* u = ds->addLobGroupUpdate(lobId1, &t);
    u->set_time(0.0);
    simData::LobGroupUpdatePoint *up = u->mutable_datapoints()->Add();
    up->set_time(0.0);
    up->set_azimuth(19.0);
    up->set_elevation(100.0);
    up->set_range(3450.0);
    t.commit();
  }

  // insert data point
  {
    simData::LobGroupUpdate* u = ds->addLobGroupUpdate(lobId1, &t);
    u->set_time(1.0);
    simData::LobGroupUpdatePoint *up = u->mutable_datapoints()->Add();
    up->set_time(1.0);
    up->set_azimuth(11.0);
    up->set_elevation(100.0);
    up->set_range(3000.0);
    t.commit();
  }

  // insert data point at same time
  {
    simData::LobGroupUpdate* u = ds->addLobGroupUpdate(lobId1, &t);
    u->set_time(1.0);
    simData::LobGroupUpdatePoint *up2 = u->mutable_datapoints()->Add();
    up2->set_time(1.0);
    up2->set_azimuth(12.0);
    up2->set_elevation(100.0);
    up2->set_range(3000.0);
    t.commit();
  }


  // insert data point
  {
    simData::LobGroupUpdate* u = ds->addLobGroupUpdate(lobId1, &t);
    u->set_time(2.0);
    simData::LobGroupUpdatePoint *up = u->mutable_datapoints()->Add();
    up->set_time(2.0);
    up->set_azimuth(15.0);
    up->set_elevation(150.0);
    up->set_range(3000.0);
    simData::LobGroupUpdatePoint *up2 = u->mutable_datapoints()->Add();
    up2->set_time(2.0);
    up2->set_azimuth(16.0);
    up2->set_elevation(150.0);
    up2->set_range(3000.0);
    t.commit();
  }

  // insert data point
  {
    simData::LobGroupUpdate* u = ds->addLobGroupUpdate(lobId1, &t);
    u->set_time(3.0);
    simData::LobGroupUpdatePoint *up = u->mutable_datapoints()->Add();
    up->set_time(3.0);
    up->set_azimuth(24.0);
    up->set_elevation(170.0);
    up->set_range(4000.0);
    simData::LobGroupUpdatePoint *up2 = u->mutable_datapoints()->Add();
    up2->set_time(4.0); // this time will be reset to 3.0 on insert
    up2->set_azimuth(35.0);
    up2->set_elevation(180.0);
    up2->set_range(4000.0);
    simData::LobGroupUpdatePoint *up3 = u->mutable_datapoints()->Add();
    up3->set_time(3.0);
    up3->set_azimuth(25.0);
    up3->set_elevation(175.0);
    up3->set_range(4000.0);
    t.commit();
  }

  const simData::LobGroupCommandSlice* cslice = ds->lobGroupCommandSlice(lobId1);

  // retrieve data points
  struct LobGroupSliceCopy : public simData::LobGroupUpdateSlice::Visitor
  {
    vector<simData::LobGroupUpdate> updates;
    virtual void operator()(const simData::LobGroupUpdate *update)
    {
      updates.push_back(*update);
    }
  };

  LobGroupSliceCopy lobSliceCopy;
  const simData::LobGroupUpdateSlice *lobPointSlice = ds->lobGroupUpdateSlice(lobId1);
  lobPointSlice->visit(&lobSliceCopy);

  // verify number of data points
  assertEquals(lobSliceCopy.updates.size(), (size_t)4);

  // verify data point values
  assertEquals(lobSliceCopy.updates[1].datapoints_size(), 2);
  rv += SDK_ASSERT(lobSliceCopy.updates[1].datapoints(0).azimuth() == 12.0 ||
    lobSliceCopy.updates[1].datapoints(0).azimuth() == 11.0);
  assertEquals(lobSliceCopy.updates[1].datapoints(0).elevation(), 100.0);
  assertEquals(lobSliceCopy.updates[1].datapoints(0).range(), 3000.0);
  rv += SDK_ASSERT(lobSliceCopy.updates[1].datapoints(1).azimuth() == 11.0 ||
    lobSliceCopy.updates[1].datapoints(1).azimuth() == 12.0);
  rv += SDK_ASSERT(lobSliceCopy.updates[2].datapoints(0).azimuth() == 16.0 ||
    lobSliceCopy.updates[2].datapoints(0).azimuth() == 15.0);
  rv += SDK_ASSERT(lobSliceCopy.updates[2].datapoints(1).azimuth() == 16.0 ||
    lobSliceCopy.updates[2].datapoints(1).azimuth() == 15.0);
  assertEquals(lobSliceCopy.updates[2].datapoints(0).elevation(), 150.0);

  // update to first time
  ds->update(0.0);
  const simData::LobGroupUpdate *c0 = lobPointSlice->current();
  assertEquals(c0->datapoints_size(), 1);


  // update current time
  ds->update(1.0);
  const simData::LobGroupUpdate *c1 = lobPointSlice->current();
  assertEquals(c1->datapoints_size(), 3); // since only 1 second of data available, only 3 points from time 0.0 and 1.0
  rv += SDK_ASSERT(c1->datapoints(1).azimuth() == 12.0 ||
    c1->datapoints(1).azimuth() == 11.0);
  rv += SDK_ASSERT(c1->datapoints(1).azimuth() == 11.0 ||
    c1->datapoints(2).azimuth() == 12.0);
  assertEquals(c1->datapoints(1).elevation(), 100.0);
  assertEquals(c1->datapoints(1).range(), 3000.0);

  // test prefs
  const simData::LobGroupCommand* com1 = cslice->current();
  rv += SDK_ASSERT(com1->updateprefs().maxdatapoints() == 2);
  rv += SDK_ASSERT(com1->updateprefs().maxdataseconds() == 2.0);
  rv += SDK_ASSERT(com1->updateprefs().color1() == 0xff00ff00);
  rv += SDK_ASSERT(com1->updateprefs().color2() == 0x00ff00ff);
  rv += SDK_ASSERT(com1->updateprefs().lobwidth() == 5);
  rv += SDK_ASSERT(com1->updateprefs().stipple1() == 0x0FF0);
  rv += SDK_ASSERT(com1->updateprefs().stipple2() == 0x00FF);

  ds->update(2.0);
  const simData::LobGroupUpdate *c2 = lobPointSlice->current();
  assertEquals(c2->datapoints_size(), 4); // only 2 seconds of data, and all point are within limits, so 4 points
  rv += SDK_ASSERT(c2->datapoints(0).azimuth() == 11.0 ||
    c2->datapoints(0).azimuth() == 12.0);
  assertEquals(c2->datapoints(0).elevation(), 100.0);
  assertEquals(c2->datapoints(0).range(), 3000.0);
  rv += SDK_ASSERT(c2->datapoints(1).azimuth() == 11.0 ||
    c2->datapoints(1).azimuth() == 12.0);
  rv += SDK_ASSERT(c2->datapoints(2).azimuth() == 15.0 ||
    c2->datapoints(2).azimuth() == 16.0);
  rv += SDK_ASSERT(c2->datapoints(3).azimuth() == 15.0 ||
    c2->datapoints(3).azimuth() == 16.0);
  assertEquals(c2->datapoints(2).elevation(), 150.0);

  // test prefs
  const simData::LobGroupCommand* com2 = cslice->current();
  rv += SDK_ASSERT(com2->updateprefs().maxdatapoints() == 2);
  rv += SDK_ASSERT(com2->updateprefs().maxdataseconds() == 2.0);
  rv += SDK_ASSERT(com2->updateprefs().color1() == 0xff00ff00);
  rv += SDK_ASSERT(com2->updateprefs().color2() == 0x00ff00ff);
  rv += SDK_ASSERT(com2->updateprefs().lobwidth() == 5);
  rv += SDK_ASSERT(com2->updateprefs().stipple1() == 0x0FF0);
  rv += SDK_ASSERT(com2->updateprefs().stipple2() == 0x00FF);


  // set new pref command for time 2.0, setting the data point limit to 1
  {
    simData::LobGroupCommand command;
    simData::LobGroupPrefs* cp = command.mutable_updateprefs();
    command.set_time(2.0);
    cp->set_maxdatapoints(1);
    cp->set_maxdataseconds(2.0);
    testHelper.addLOBCommand(command, lobId1);
  }
  ds->update(2.0);
  const simData::LobGroupUpdate *c3 = lobPointSlice->current();
  assertEquals(c3->datapoints_size(), 2); // only 1 data point time set, so should only have the 2 points at time 2.0
  rv += SDK_ASSERT(c3->datapoints(0).azimuth() == 15.0 ||
    c3->datapoints(0).azimuth() == 16.0);

  // update time
  ds->update(3.0);
  const simData::LobGroupUpdate *c4 = lobPointSlice->current();
  assertEquals(c4->datapoints_size(), 3); // only 1 data point time set, so should only have the 3 points at time 3.0
  assertEquals(c4->datapoints(0).time(), 3.0);
  assertEquals(c4->datapoints(1).time(), 3.0); // note that point added with time 4.0 should now be 3.0
  assertEquals(c4->datapoints(2).time(), 3.0);
  rv += SDK_ASSERT(c4->datapoints(0).azimuth() == 24.0 ||
   c4->datapoints(0).azimuth() == 25.0 ||
   c4->datapoints(0).azimuth() == 35.0);
  rv += SDK_ASSERT(c4->datapoints(1).azimuth() == 24.0 ||
   c4->datapoints(1).azimuth() == 25.0 ||
   c4->datapoints(1).azimuth() == 35.0);
  rv += SDK_ASSERT(c4->datapoints(0).elevation() == 170.0 ||
    c4->datapoints(0).elevation() == 175.0 ||
    c4->datapoints(0).elevation() == 180.0);

  // now setting max data points to 3, but max data seconds is 2
  {
    simData::LobGroupCommand command;
    simData::LobGroupPrefs* commandPrefs = command.mutable_updateprefs();
    command.set_time(2.0);
    commandPrefs->set_maxdatapoints(3);
    commandPrefs->set_maxdataseconds(2.0);
    commandPrefs->set_color1(0xff00ffff);
    commandPrefs->set_color2(0xffff00ff);
    commandPrefs->set_lobwidth(5);
    commandPrefs->set_stipple1(0xF000);
    commandPrefs->set_stipple2(0x0FFF);
    testHelper.addLOBCommand(command, lobId1);
  }

  ds->update(3.0);
  const simData::LobGroupUpdate *c5a = lobPointSlice->current();
  assertEquals(c5a->datapoints_size(), 7); //  should have data from [1.0, 3.0]

  ds->update(3.0 + FLT_EPSILON);
  const simData::LobGroupUpdate *c5 = lobPointSlice->current();
  assertEquals(c5->datapoints_size(), 5); //  should have data from [1.0+epsilon, 3.0+epsilon]
  // data should be time ordered
  assertEquals(c5->datapoints(0).time(), 2.0);
  assertEquals(c5->datapoints(1).time(), 2.0);
  assertEquals(c5->datapoints(2).time(), 3.0);
  assertEquals(c5->datapoints(3).time(), 3.0);
  assertEquals(c5->datapoints(4).time(), 3.0);
  rv += SDK_ASSERT(c5->datapoints(0).azimuth() == 15.0 ||
    c5->datapoints(0).azimuth() == 16.0);
  rv += SDK_ASSERT(c5->datapoints(1).azimuth() == 15.0 ||
    c5->datapoints(1).azimuth() == 16.0);
  rv += SDK_ASSERT(c5->datapoints(2).azimuth() == 24.0 ||
   c5->datapoints(2).azimuth() == 25.0 ||
   c5->datapoints(2).azimuth() == 35.0);

  // test prefs
  const simData::LobGroupCommand* com3 = cslice->current();
  rv += SDK_ASSERT(com3->updateprefs().maxdatapoints() == 3);
  rv += SDK_ASSERT(com3->updateprefs().maxdataseconds() == 2.0);
  rv += SDK_ASSERT(com3->updateprefs().color1() == 0xff00ffff);
  rv += SDK_ASSERT(com3->updateprefs().color2() == 0xffff00ff);
  rv += SDK_ASSERT(com3->updateprefs().lobwidth() == 5);
  rv += SDK_ASSERT(com3->updateprefs().stipple1() == 0xF000);
  rv += SDK_ASSERT(com3->updateprefs().stipple2() == 0x0FFF);

  // now set both to unlimited, should have all points
  {
    simData::LobGroupCommand command;
    simData::LobGroupPrefs* cp = command.mutable_updateprefs();
    command.set_time(3.0);
    cp->set_maxdatapoints(std::numeric_limits<int>::max());
    cp->set_maxdataseconds(std::numeric_limits<double>::max());
    testHelper.addLOBCommand(command, lobId1);
  }
  ds->update(3.0);
  const simData::LobGroupUpdate *c6 = lobPointSlice->current();
  assertEquals(c6->datapoints_size(), 8); //  should have data from time 0.0, 1.0, 2.0, and 3.0
  // data should be time ordered
  assertEquals(c6->datapoints(0).time(), 0.0);
  assertEquals(c6->datapoints(1).time(), 1.0);
  assertEquals(c6->datapoints(2).time(), 1.0);
  assertEquals(c6->datapoints(3).time(), 2.0);
  assertEquals(c6->datapoints(4).time(), 2.0);
  assertEquals(c6->datapoints(5).time(), 3.0);
  assertEquals(c6->datapoints(6).time(), 3.0);
  assertEquals(c6->datapoints(7).time(), 3.0);

  // test iterator
  simData::LobGroupUpdateSlice::Iterator iter = lobPointSlice->lower_bound(2.0);
  rv += SDK_ASSERT(iter.hasNext());
  const simData::LobGroupUpdate* lobUpdate = iter.next();
  rv += SDK_ASSERT(lobUpdate->datapoints(0).azimuth() == 15 ||
                   lobUpdate->datapoints(1).azimuth() == 16);

  TestListener *testListen = new TestListener;
  simData::DataStore::ListenerPtr testListenShared(testListen);
  ds->addListener(testListenShared);
  ds->removeEntity(platId1);
  // Need to do an update to force the deletion
  ds->update(0.0);
  // the platform and two lobs == 3
  rv += SDK_ASSERT(testListen->removeCount() == 3);

  if (rv != 0)
    throw AssertionException();
}

// Verifies that a generic key value pair appears only once
bool findOnce(const simData::GenericData *g1, const std::string& key, const std::string& value)
{
  int occurances = 0;
  for (int ii = 0; ii < g1->entry().size(); ii++)
  {
    if (g1->entry().Get(ii).key() == key)
    {
      occurances++;
      if (g1->entry().Get(ii).value() != value)
        return false;
    }
  }

  return occurances == 1;
}

// Verifies that only the expected generic data appears
bool findMany(const simData::GenericData *g1, const std::string& key, const std::vector<std::string>& values)
{
  size_t occurances = 0;
  for (int ii = 0; ii < g1->entry().size(); ii++)
  {
    if (g1->entry().Get(ii).key() == key)
    {
      occurances++;
      bool found = false;
      for (size_t jj = 0; jj < values.size(); jj++)
      {
        if (g1->entry().Get(ii).value() == values[jj])
        {
          found = true;
          break;
        }
      }

      if (!found)
        return false;
    }
  }

  return occurances == values.size();
}

void testGenericData_insert()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId = testHelper.addPlatform();

  // insert generic data point
  {
    simData::GenericData *gd = ds->addGenericData(platId, &t);
    gd->set_time(1.0);
    gd->set_duration(10.0);

    simData::GenericData_Entry *e1 = gd->add_entry();
    e1->set_key("key1");
    e1->set_value("value1");

    simData::GenericData_Entry *e2 = gd->add_entry();
    e2->set_key("key2");
    e2->set_value("value2");

    t.commit();
  }

  // insert generic data point
  {
    simData::GenericData *gd = ds->addGenericData(platId, &t);
    gd->set_time(2.0);
    gd->set_duration(5.0);

    simData::GenericData_Entry *e1 = gd->add_entry();
    e1->set_key("key3");
    e1->set_value("value3");

    simData::GenericData_Entry *e2 = gd->add_entry();
    e2->set_key("key4");
    e2->set_value("value4");

    t.commit();
  }

   // retrieve data points
  struct GenericDataSliceCopy : public simData::GenericDataSlice::Visitor
  {
    vector<simData::GenericData_Entry> entries;
    virtual void operator()(const simData::GenericData *update)
    {
      if (update == NULL)
        return;
      for (int k = 0; k < update->entry_size(); ++k)
        entries.push_back(update->entry(k));
    }
  };

  GenericDataSliceCopy sc;
  const simData::GenericDataSlice *gdslice = ds->genericDataSlice(platId);
  gdslice->visit(&sc);

   // verify number of data points
  assertEquals(sc.entries.size(), static_cast<size_t>(4));

  // verify data point values
  assertEquals(sc.entries[0].key(), "key1");
  assertEquals(sc.entries[0].value(), "value1");
  assertEquals(sc.entries[1].key(), "key2");
  assertEquals(sc.entries[1].value(), "value2");

  assertEquals(sc.entries[2].key(), "key3");
  assertEquals(sc.entries[2].value(), "value3");
  assertEquals(sc.entries[3].key(), "key4");
  assertEquals(sc.entries[3].value(), "value4");

  // Do a flush and the visitor should come back empty
  ds->flush(platId, simData::DataStore::RECURSIVE);
  GenericDataSliceCopy sc2;
  gdslice->visit(&sc2);
  assertEquals(sc2.entries.size(), static_cast<size_t>(0));

  // Doing a visitor with NULL should not crash
  gdslice->visit(NULL);
}

int testGenericData_update()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId = testHelper.addPlatform();

  // Typical values
  testHelper.addGenericData(platId, "key1", "value1", 1.0);
  testHelper.addGenericData(platId, "key2", "value2", 1.0);
  testHelper.addGenericData(platId, "key3", "value3", 2.0);
  testHelper.addGenericData(platId, "key4", "value4", 2.0);
  // singleValue only has one value at a time
  testHelper.addGenericData(platId, "singleValue", "value1", 1.0);
  testHelper.addGenericData(platId, "singleValue", "value2", 2.0);
  testHelper.addGenericData(platId, "singleValue", "value3", 3.0);
  testHelper.addGenericData(platId, "singleValue", "value4", 4.0);
  testHelper.addGenericData(platId, "singleValue", "value5", 5.0);
  testHelper.addGenericData(platId, "singleValue", "value6", 6.0);
  testHelper.addGenericData(platId, "singleValue", "value7", 7.0);
  testHelper.addGenericData(platId, "singleValue", "value8", 8.0);
  testHelper.addGenericData(platId, "singleValue", "value9", 9.0);
  testHelper.addGenericData(platId, "singleValue", "value10", 10.0);

  const simData::GenericDataSlice *gdslice = ds->genericDataSlice(platId);

  ds->update(0.5);
  const simData::GenericData *g0 = gdslice->current();
  rv += SDK_ASSERT(g0->entry().size() == 0);

  // update current time
  ds->update(1.0);
  const simData::GenericData *g1 = gdslice->current();
  rv += SDK_ASSERT(g1->entry().size() == 3);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g1, "singleValue", "value1"));

  // pick up a second set
  ds->update(2.0);
  const simData::GenericData *g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 5);
  rv += SDK_ASSERT(findOnce(g2, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g2, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g2, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g2, "key4", "value4"));
  rv += SDK_ASSERT(findOnce(g2, "singleValue", "value2"));

  // Only singleValue should change
  ds->update(3.5);
  const simData::GenericData *g5 = gdslice->current();
  rv += SDK_ASSERT(g5->entry().size() == 5);
  rv += SDK_ASSERT(findOnce(g5, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g5, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g5, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g5, "key4", "value4"));
  rv += SDK_ASSERT(findOnce(g5, "singleValue", "value3"));

  return rv;
}

int testGenericDataMixExpiration_update()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId = testHelper.addPlatform();

  // Add various tags for generic data (all with infinite expiration)
  testHelper.addGenericData(platId, "key1", "value1", 1.0);
  testHelper.addGenericData(platId, "key2", "value2", 1.0);
  testHelper.addGenericData(platId, "key3", "value3", 2.0);
  testHelper.addGenericData(platId, "key4", "value4", 2.0);
  testHelper.addGenericData(platId, "key5", "value5", 3.0);
  testHelper.addGenericData(platId, "key6", "value6", 3.0);

  // Add a few other tags at different times
  testHelper.addGenericData(platId, "finite2infinite", "value7", 12.0);
  testHelper.addGenericData(platId, "finite2infinite", "unlimited", 14.0);
  testHelper.addGenericData(platId, "infinite2finite", "unlimited", 12.0);
  testHelper.addGenericData(platId, "infinite2finite", "value8", 14.0);

  const simData::GenericDataSlice *gdslice = ds->genericDataSlice(platId);

  // A time before anything existed
  ds->update(0.1);
  const simData::GenericData *g0 = gdslice->current();
  rv += SDK_ASSERT(g0->entry().size() == 0);

  // update current time
  ds->update(1.0);
  const simData::GenericData *g1 = gdslice->current();
  rv += SDK_ASSERT(g1->entry().size() == 2);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));

  // No change
  ds->update(1.5);
  g1 = gdslice->current();
  rv += SDK_ASSERT(g1->entry().size() == 2);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));

  // Add new keys
  ds->update(2.0);
  const simData::GenericData *g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 4);
  rv += SDK_ASSERT(findOnce(g2, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g2, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g2, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g2, "key4", "value4"));

  // No change
  ds->update(2.5);
  g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 4);
  rv += SDK_ASSERT(findOnce(g2, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g2, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g2, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g2, "key4", "value4"));

  // Pick up new keys
  ds->update(3.0);
  g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 6);
  rv += SDK_ASSERT(findOnce(g2, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g2, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g2, "key5", "value5"));
  rv += SDK_ASSERT(findOnce(g2, "key6", "value6"));
  rv += SDK_ASSERT(findOnce(g2, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g2, "key4", "value4"));

  // No change
  ds->update(3.5);
  g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 6);
  rv += SDK_ASSERT(findOnce(g2, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g2, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g2, "key5", "value5"));
  rv += SDK_ASSERT(findOnce(g2, "key6", "value6"));
  rv += SDK_ASSERT(findOnce(g2, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g2, "key4", "value4"));

  // Back to a time before anything existed
  ds->update(0.0);
  const simData::GenericData *g4 = gdslice->current();
  rv += SDK_ASSERT(g4->entry().size() == 0);

  return rv;
}


int testGenericDataNoExpiration_update()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId = testHelper.addPlatform();

  // Typical values
  testHelper.addGenericData(platId, "key1", "value1", 1.0);
  testHelper.addGenericData(platId, "key2", "value2", 1.0);
  testHelper.addGenericData(platId, "key3", "value3", 2.0);
  testHelper.addGenericData(platId, "key4", "value4", 2.0);

  // Overwrite previous value
  testHelper.addGenericData(platId, "overWrite", "value1", 1.0);
  testHelper.addGenericData(platId, "overWrite", "value2", 1.5);
  testHelper.addGenericData(platId, "overWrite", "value3", 2.0);
  testHelper.addGenericData(platId, "overWrite", "value4", 2.5);

  const simData::GenericDataSlice *gdslice = ds->genericDataSlice(platId);

  // A time before anything existed
  ds->update(0.1);
  const simData::GenericData *g0 = gdslice->current();
  rv += SDK_ASSERT(g0->entry().size() == 0);

  // update current time
  ds->update(1.0);
  const simData::GenericData *g1 = gdslice->current();
  rv += SDK_ASSERT(g1->entry().size() == 3);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g1, "overWrite", "value1"));

  // No new keys, but overwrite changes
  ds->update(1.5);
  g1 = gdslice->current();
  rv += SDK_ASSERT(g1->entry().size() == 3);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g1, "overWrite", "value2"));

  // New keys get added and overwrite changes
  ds->update(2.0);
  const simData::GenericData *g2 = gdslice->current();
  rv += SDK_ASSERT(g2->entry().size() == 5);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g1, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g1, "key4", "value4"));
  rv += SDK_ASSERT(findOnce(g1, "overWrite", "value3"));

  // No new keys, but overwrite changes
  ds->update(10000000000.0);
  g2 = gdslice->current();
  assertNotEquals(g2, (const simData::GenericData *)NULL);
  rv += SDK_ASSERT(g2->entry().size() == 5);
  rv += SDK_ASSERT(findOnce(g1, "key1", "value1"));
  rv += SDK_ASSERT(findOnce(g1, "key2", "value2"));
  rv += SDK_ASSERT(findOnce(g1, "key3", "value3"));
  rv += SDK_ASSERT(findOnce(g1, "key4", "value4"));
  rv += SDK_ASSERT(findOnce(g1, "overWrite", "value4"));

  // Back to a time before anything existed
  ds->update(0.0);
  const simData::GenericData *g4 = gdslice->current();
  rv += SDK_ASSERT(g4->entry().size() == 0);

  return rv;
}

int testCategoryData_insert()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  // insert platform
  simData::DataStore::Transaction t;
  uint64_t platId = testHelper.addPlatform();

  // insert category data point
  {
    simData::CategoryData *cd = ds->addCategoryData(platId, &t);
    cd->set_time(1.0);

    simData::CategoryData_Entry *e1 = cd->add_entry();
    e1->set_key("key1");
    e1->set_value("value1");

    simData::CategoryData_Entry *e2 = cd->add_entry();
    e2->set_key("key2");
    e2->set_value("value2");

    t.commit();
  }

  // insert category data point
  {
    simData::CategoryData *cd = ds->addCategoryData(platId, &t);
    cd->set_time(2.0);

    simData::CategoryData_Entry *e1 = cd->add_entry();
    e1->set_key("key3");
    e1->set_value("value3");

    simData::CategoryData_Entry *e2 = cd->add_entry();
    e2->set_key("key4");
    e2->set_value("value4");

    t.commit();
  }

  ds->update(2.0);

   // retrieve data points
  struct CategoryDataSliceCopy : public simData::CategoryDataSlice::Visitor
  {
    vector<simData::CategoryData> updates;
    virtual void operator()(const simData::CategoryData *update)
    {
      updates.push_back(*update);
    }
  };

  CategoryDataSliceCopy sc;
  const simData::CategoryDataSlice *cdslice = ds->categoryDataSlice(platId);
  cdslice->visit(&sc);

   // verify number of data points
  rv += SDK_ASSERT(sc.updates.size() == 4 && sc.updates[0].entry_size() == 1);

  // verify data point values
  rv += SDK_ASSERT(sc.updates[0].entry().Get(0).key() == "key1");
  rv += SDK_ASSERT(sc.updates[0].entry().Get(0).value() == "value1");
  rv += SDK_ASSERT(sc.updates[1].entry().Get(0).key() == "key2");
  rv += SDK_ASSERT(sc.updates[1].entry().Get(0).value() == "value2");

  rv += SDK_ASSERT(sc.updates[2].entry().Get(0).key() == "key3");
  rv += SDK_ASSERT(sc.updates[2].entry().Get(0).value() == "value3");
  rv += SDK_ASSERT(sc.updates[3].entry().Get(0).key() == "key4");
  rv += SDK_ASSERT(sc.updates[3].entry().Get(0).value() == "value4");

  return rv;
}

size_t count(simData::CategoryDataSlice::Iterator cIter)
{
  size_t number = 0;
  while (cIter.hasNext())
  {
    number++;
    cIter.next();
  }

  return number;
}

bool hasKeyValue(simData::CategoryDataSlice::Iterator cIter, const std::string& key, const std::string& value)
{
  size_t number = 0;
  while (cIter.hasNext())
  {

    std::shared_ptr<simData::CategoryDataPair> nextCat = cIter.next();
    if (nextCat->name() == key)
    {
      number++;
      if (nextCat->value() != value)
        return false;
    }
  }

  return number == 1;
}

int testCategoryData_update()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::DataStore::Transaction t;

  // insert platform
  uint64_t platId = testHelper.addPlatform();

  // typical values
  testHelper.addCategoryData(platId, "key1", "value1", 1.0);
  testHelper.addCategoryData(platId, "key2", "value2", 1.0);
  testHelper.addCategoryData(platId, "key3", "value3", 2.0);
  testHelper.addCategoryData(platId, "key4", "value4", 2.0);

  testHelper.addCategoryData(platId, "overWrite", "value1", 1.0);
  testHelper.addCategoryData(platId, "overWrite", "value2", 2.0);
  testHelper.addCategoryData(platId, "overWrite", "value3", 3.0);


  // start test
  const simData::CategoryDataSlice *cdslice = ds->categoryDataSlice(platId);

  // Should be empty
  ds->update(0.0);
  rv += SDK_ASSERT(0 == count(cdslice->current()));

  testHelper.addCategoryData(platId, "Infinite", "AlwaysHere", -1.0);

  ds->update(0.0);
  rv += SDK_ASSERT(1 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));

  // update current time
  ds->update(1.0);
  rv += SDK_ASSERT(4 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key1", "value1"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key2", "value2"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "overWrite", "value1"));

  // Update in the middle
  ds->update(2.0);
  rv += SDK_ASSERT(6 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key1", "value1"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key2", "value2"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key3", "value3"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key4", "value4"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "overWrite", "value2"));

  // the last value for a category data point should remain until its value is changed
  ds->update(3.0);
  rv += SDK_ASSERT(6 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key1", "value1"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key2", "value2"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key3", "value3"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key4", "value4"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "overWrite", "value3"));

  // the last value for a category data point should remain until its value is changed
  ds->update(200000000000.0);
  rv += SDK_ASSERT(6 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key1", "value1"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key2", "value2"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key3", "value3"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "key4", "value4"));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "overWrite", "value3"));

  // Back to the start
  ds->update(0.0);
  rv += SDK_ASSERT(1 == count(cdslice->current()));
  rv += SDK_ASSERT(hasKeyValue(cdslice->current(), "Infinite", "AlwaysHere"));

  return rv;
}

class CategoryChangeCounter : public simData::DataStore::DefaultListener
{
public:
  CategoryChangeCounter()
    : simData::DataStore::DefaultListener(),
      counter_(0)
  {
  }

  virtual ~CategoryChangeCounter()
  {
  }

  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::DataStore::ObjectType ot)
  {
    ++counter_;
  }

  void clearCounter()
  {
    counter_ = 0;
  }

  int counter() const
  {
    return counter_;
  }

private:
  int counter_;
};

int testCategoryData_change()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CategoryChangeCounter* categoryCounter = new CategoryChangeCounter();
  simData::DataStore::ListenerPtr listener = simData::DataStore::ListenerPtr(categoryCounter);
  ds->addListener(listener);

  // Just started, so no changes
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // insert platform
  uint64_t platId = testHelper.addPlatform();
  testHelper.addCategoryData(platId, "overWrite", "value0", 0.0);
  testHelper.addCategoryData(platId, "overWrite", "value1", 1.0);
  testHelper.addCategoryData(platId, "overWrite", "value2", 2.0);
  testHelper.addCategoryData(platId, "overWrite", "value3", 3.0);

  // Only a call to update results in a category change, so still no changes
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Going from an undefined state to a defined state so there is a change
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // Repeat the time so there should be no change
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // small time step so there should be no change
  ds->update(0.1);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Time step that should result in a change
  ds->update(1.1);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // A small time step backwards that should not result in a change
  ds->update(1.05);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // A big time step backwards that should result in a change
  ds->update(0.95);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // A big time step to the end that should result in a change
  ds->update(4.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // Back to start which should result in a change
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // insert a second platform
  uint64_t platId2 = testHelper.addPlatform();
  testHelper.addCategoryData(platId2, "overWrite", "value1", 0.5);
  testHelper.addCategoryData(platId2, "overWrite", "value3", 3.0);

  // Only a call to update results in a category change, so still no changes
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Repeat start time
  // platId = No Change and platId2 = No Change to counter = 0
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Step before platId2 first value
  // platId = No Change and platId2 = No Change to counter = 0
  ds->update(0.49);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Step to platId2 first value
  // platId = No Change and platId2 = Change to counter = 1
  ds->update(0.5);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // Repeat platId2 first value
  // platId = No Change and platId2 = No Change to counter = 0
  ds->update(0.5);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Step pass platId2 first value
  // platId = No Change and platId2 = No Change to counter = 0
  ds->update(0.55);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Step back before platId2 first value
  // platId = No Change and platId2 = Change to counter = 1
  ds->update(0.4);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  // Another step before platId2 first value
  // platId = No Change and platId2 = Change to counter = 0
  ds->update(0.45);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Jump to a time where both platforms will get a change
  ds->update(3.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 2);
  categoryCounter->clearCounter();

  // Repeat the time so neither platform changed
  ds->update(3.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Back up a little so both platforms will get a change
  ds->update(2.90);
  rv += SDK_ASSERT(categoryCounter->counter() == 2);
  categoryCounter->clearCounter();

  // Small step forward so neither platform will change
  ds->update(2.95);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // Step over 3.0 so both platforms will get a change
  ds->update(3.05);
  rv += SDK_ASSERT(categoryCounter->counter() == 2);
  categoryCounter->clearCounter();

  // Back to start which should result in a change
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 2);
  categoryCounter->clearCounter();

  // insert a third platform with non changing category data
  uint64_t platId3 = testHelper.addPlatform();
  testHelper.addCategoryData(platId3, "sameData", "value1", 0.6);
  testHelper.addCategoryData(platId3, "sameData", "value1", 0.9);
  testHelper.addCategoryData(platId3, "sameData", "value1", 2.0);
  testHelper.addCategoryData(platId3, "sameData", "value1", 3.0);

  // Repeat start time
  // platId = No Change and platId2 = No Change platId3 = No Change to counter = 0
  ds->update(0.0);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // platId = No Change and platId2 = Change platId3 = Change to counter = 2
  ds->update(0.6);
  rv += SDK_ASSERT(categoryCounter->counter() == 2);
  categoryCounter->clearCounter();

  // platId = No Change and platId2 = No Change platId3 = No Change to counter = 0
  ds->update(0.9);
  rv += SDK_ASSERT(categoryCounter->counter() == 0);
  categoryCounter->clearCounter();

  // platId = No Change and platId2 = No Change platId3 = Change to counter = 1
  ds->update(0.55);
  rv += SDK_ASSERT(categoryCounter->counter() == 1);
  categoryCounter->clearCounter();

  ds->removeListener(listener);
  // no need to delete categoryCounter since listener took ownership

  return rv;
}

int testScenarioDeleteCallback()
{
  int rv = 0;

  simUtil::DataStoreTestHelper* testHelper = new simUtil::DataStoreTestHelper;
  simData::DataStore* ds = testHelper->dataStore();

  TestListener *testListen = new TestListener;
  simData::DataStore::ListenerPtr testListenShared(testListen);
  ds->addListener(testListenShared);

  rv += SDK_ASSERT(testListen->deleteCount() == 0);
  delete testHelper;
  rv += SDK_ASSERT(testListen->deleteCount() == 1);

  return rv;
}

int TestMemoryDataStore(int argc, char* argv[])
{
  simCore::checkVersionThrow();

  try
  {
    testPlatform_insert();
    testPlatform_insertStatic();
    testLobGroup_insert();
    testGenericData_insert();
    int rv = testGenericData_update();
    rv += testGenericDataNoExpiration_update();
    rv += testGenericDataMixExpiration_update();
    rv += testCategoryData_insert();
    rv += testCategoryData_update();
    rv += testCategoryData_change();
    rv += testScenarioDeleteCallback();
    return rv;
  }
  catch (AssertionException& e)
  {
    cout << e.what() << endl;
    return 1;
  }
}

