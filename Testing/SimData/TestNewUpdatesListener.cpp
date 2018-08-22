/* -*- mode: c++ -*- */
/****************************************************************************
*****                                                                  *****
*****                   Classification: UNCLASSIFIED                   *****
*****                    Classified By:                                *****
*****                    Declassify On:                                *****
*****                                                                  *****
****************************************************************************
* Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* U.S. Naval Research Laboratory.
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*/
#include <map>
#include "simCore/Common/SDKAssert.h"
#include "simData/DataStoreProxy.h"
#include "simUtil/DataStoreTestHelper.h"

namespace {

/** Helper class implementation of NewUpdatesListener, used to verify that we're getting data we expect. */
class TimeCollector : public simData::DataStore::NewUpdatesListener
{
public:
  /** Record the time value of entity update */
  virtual void onEntityUpdate(simData::DataStore* source, simData::ObjectId id, double dataTime)
  {
    allData_[id].insert(dataTime);
  }

  /** Clear out the updates for the given entity and record the flush */
  virtual void onFlush(simData::DataStore* source, simData::ObjectId flushedId)
  {
    if (flushedId == 0)
    {
      flushedIds_.clear();
      flushedIds_.insert(0);
      allData_.clear();
    }
    else
    {
      flushedIds_.insert(flushedId);
      allData_.erase(flushedId);
    }
  }

  /** Clear out saved data */
  void clear()
  {
    allData_.clear();
    flushedIds_.clear();
  }

  /** Set of time stamps */
  typedef std::set<double> Timestamps;

  /** Get all timestamps recorded since last clear/flush for a given ID */
  Timestamps getTimes(simData::ObjectId id) const
  {
    auto i = allData_.find(id);
    if (i == allData_.end())
      return Timestamps();
    return i->second;
  }

  /** Returns true if the ID was flushed since last clear() */
  bool sawFlush(simData::ObjectId id) const
  {
    if (flushedIds_.empty()) // common case
      return false;
    else if (flushedIds_.find(0) != flushedIds_.end()) // uncommon case
      return true;
    // Least common case: flush on a single entity
    return flushedIds_.find(id) != flushedIds_.end();
  }

private:
  std::map<simData::ObjectId, Timestamps> allData_;
  std::set<simData::ObjectId> flushedIds_;
};

int testEntityCollection()
{
  simUtil::DataStoreTestHelper helper;

  simData::DataStore* ds = helper.dataStore();
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  ds->setNewUpdatesListener(timeCollector);

  simData::ObjectId plat1 = helper.addPlatform(1);
  simData::ObjectId plat2 = helper.addPlatform(2);
  simData::ObjectId plat3 = helper.addPlatform(3);
  helper.addPlatformUpdate(1.0, plat1);
  helper.addPlatformUpdate(1.0, plat2);
  helper.addPlatformUpdate(1.0, plat3);

  helper.addPlatformUpdate(1.5, plat3);

  helper.addPlatformUpdate(2.0, plat1);
  helper.addPlatformUpdate(2.0, plat2);

  helper.addPlatformUpdate(2.5, plat3);

  int rv = 0;

  auto p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 2);
  rv += SDK_ASSERT(p1Times.count(1.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.5) == 0);
  auto p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 2);
  rv += SDK_ASSERT(p2Times.count(1.0) != 0);
  rv += SDK_ASSERT(p2Times.count(2.0) != 0);
  rv += SDK_ASSERT(p2Times.count(2.5) == 0);
  auto p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 3);
  rv += SDK_ASSERT(p3Times.count(1.0) != 0);
  rv += SDK_ASSERT(p3Times.count(1.5) != 0);
  rv += SDK_ASSERT(p3Times.count(2.5) != 0);

  // Test that adding more points adds to collection
  helper.addPlatformUpdate(3.0, plat1);
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 3);
  rv += SDK_ASSERT(p1Times.count(1.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.0) != 0);
  rv += SDK_ASSERT(p1Times.count(3.0) != 0);

  // Clear out the updates, simulating what should happen per frame
  timeCollector->clear();
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).empty());
  // Make sure we still haven't seen a flush (clear() doesn't count)
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat1));

  // .. then add more points
  helper.addPlatformUpdate(3.2, plat1);
  helper.addPlatformUpdate(3.6, plat2);
  helper.addPlatformUpdate(4.0, plat2);
  helper.addPlatformUpdate(4.0, plat3);
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 1);
  rv += SDK_ASSERT(p1Times.count(3.2) != 0);
  p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 2);
  rv += SDK_ASSERT(p2Times.count(3.6) != 0);
  rv += SDK_ASSERT(p2Times.count(4.0) != 0);
  p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 1);
  rv += SDK_ASSERT(p3Times.count(4.0) != 0);

  // Do a single flush on one platform; still in the same frame as last time
  ds->flush(plat1);
  // .. so we should have one flush and one empty set of times on plat1
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).size() == 2);
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).size() == 1);
  rv += SDK_ASSERT(timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat3));

  // Do a single entity flush after a clear (simulated update frame)
  timeCollector->clear();
  helper.addPlatformUpdate(4.1, plat1);
  helper.addPlatformUpdate(4.2, plat2);
  helper.addPlatformUpdate(4.3, plat3);
  ds->flush(plat2);
  helper.addPlatformUpdate(4.5, plat1);
  helper.addPlatformUpdate(4.6, plat2);
  helper.addPlatformUpdate(4.7, plat3);

  // Verify expected output from time colletor
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 2);
  rv += SDK_ASSERT(p1Times.count(4.1) != 0);
  rv += SDK_ASSERT(p1Times.count(4.5) != 0);
  p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 1);
  rv += SDK_ASSERT(p2Times.count(4.2) == 0);
  rv += SDK_ASSERT(p2Times.count(4.6) != 0);
  p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 2);
  rv += SDK_ASSERT(p3Times.count(4.3) != 0);
  rv += SDK_ASSERT(p3Times.count(4.7) != 0);
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat3));

  // New frame, flush everything
  timeCollector->clear();
  helper.addPlatformUpdate(5.3, plat3);
  ds->flush(0);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).empty());
  rv += SDK_ASSERT(timeCollector->sawFlush(0));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat3));
  rv += SDK_ASSERT(timeCollector->sawFlush(100)); // random ID that doesn't exist, but should trigger because we flushed 0

  return rv;
}

int testDataStoreProxy()
{
  int rv = 0;

  // Make sure that when assigning a new scenario through data store proxy, the time collector lives on
  simData::MemoryDataStore* ds1 = new simData::MemoryDataStore;
  const auto& ds1UpdatesListener = ds1->newUpdatesListener();
  simData::DataStoreProxy proxy(ds1);
  // Listener should not have changed
  rv += SDK_ASSERT(&ds1->newUpdatesListener() == &ds1UpdatesListener);

  simData::MemoryDataStore* ds2 = new simData::MemoryDataStore;
  const auto& ds2UpdatesListener = ds2->newUpdatesListener();
  rv += SDK_ASSERT(&ds2->newUpdatesListener() == &ds2UpdatesListener);
  proxy.reset(ds2);
  ds1 = NULL; // reset() will delete it
  // Listener should have changed
  rv += SDK_ASSERT(&ds2->newUpdatesListener() == &ds1UpdatesListener);
  rv += SDK_ASSERT(&ds2->newUpdatesListener() != &ds2UpdatesListener);

  // Now update it to a custom one we provide, TIme Collector
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  proxy.setNewUpdatesListener(timeCollector);
  rv += SDK_ASSERT(&ds2->newUpdatesListener() == timeCollector.get());

  // Reset a new proxy and our Time Collector should have carried over
  simData::MemoryDataStore* ds3 = new simData::MemoryDataStore;
  const auto& ds3UpdatesListener = ds3->newUpdatesListener();
  rv += SDK_ASSERT(&ds3->newUpdatesListener() == &ds3UpdatesListener);
  proxy.reset(ds3);
  ds2 = NULL; // reset() will delete it
  // Listener should have changed
  rv += SDK_ASSERT(&ds3->newUpdatesListener() == timeCollector.get());
  rv += SDK_ASSERT(&ds3->newUpdatesListener() != &ds3UpdatesListener);
  return rv;
}

}

int TestNewUpdatesListener(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testEntityCollection() == 0);
  rv += SDK_ASSERT(testDataStoreProxy() == 0);
  return rv;
}
