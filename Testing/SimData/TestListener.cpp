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

#include "simCore/Common/SDKAssert.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

// The different types of callbacks
enum CallbackTypes
{
  AddEntity,
  RemoveEntity,
  PrefsChange,
  TimeChange,
  CategoryDataChange,
  NameChange,
  Flush,
  ScenarioDelete
};

// Count each type of callback
class CounterListener : public simData::DataStore::DefaultListener
{
public:
  CounterListener()
    : add_(0),
      remove_(0),
      pref_(0),
      time_(0),
      category_(0),
      name_(0),
      flush_(0),
      scenario_(0)
  {
  }

  bool compareAndClear(uint32_t add, uint32_t remove, uint32_t pref, uint32_t time, uint32_t category, uint32_t name, uint32_t flush, uint32_t scenario)
  {
    bool rv = (add_ == add);
    rv = rv && (remove_ == remove);
    rv = rv && (pref_ == pref);
    rv = rv && (time_ == time);
    rv = rv && (category_ == category);
    rv = rv && (name_ == name);
    rv = rv && (flush_ == flush);
    rv = rv && (scenario_ == scenario);

    add_ = 0;
    remove_ = 0;
    pref_ = 0;
    time_ = 0;
    category_ = 0;
    name_ = 0;
    flush_ = 0;
    scenario_ = 0;

    return rv;
  }

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    ++add_;
  }

  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    ++remove_;
  }

  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    ++pref_;
  }

  virtual void onChange(simData::DataStore *source)
  {
    ++time_;
  }

  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    ++category_;
  }

  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    ++name_;
  }

  virtual void onFlush(simData::DataStore *source, simData::ObjectId flushedId)
  {
    ++flush_;
  }

  virtual void onScenarioDelete(simData::DataStore* source)
  {
    ++scenario_;
  }

protected:
  uint32_t add_;
  uint32_t remove_;
  uint32_t pref_;
  uint32_t time_;
  uint32_t category_;
  uint32_t name_;
  uint32_t flush_;
  uint32_t scenario_;
};

/// Self documenting callback
class DoesNothingCallback : public simData::DataStore::DefaultListener
{
public:
  DoesNothingCallback()
  {
  }
};

/// Add a Listener during a callback to verify recursion works.
class AddDuringCallback : public simData::DataStore::DefaultListener
{
public:
  AddDuringCallback(CallbackTypes type, simData::DataStore::ListenerPtr listener)
    : type_(type),
      listener_(listener)
  {
  }

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == AddEntity))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == RemoveEntity))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    if ((listener_ != nullptr) && (type_ == PrefsChange))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onChange(simData::DataStore *source)
  {
    if ((listener_ != nullptr) && (type_ == TimeChange))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == CategoryDataChange))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    if ((listener_ != nullptr) && (type_ == NameChange))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onFlush(simData::DataStore *source, simData::ObjectId flushedId)
  {
    if ((listener_ != nullptr) && (type_ == Flush))
      source->addListener(listener_);
    listener_.reset();
  }

  virtual void onScenarioDelete(simData::DataStore* source)
  {
    if ((listener_ != nullptr) && (type_ == ScenarioDelete))
      source->addListener(listener_);
    listener_.reset();
  }

protected:
  CallbackTypes type_;
  simData::DataStore::ListenerPtr listener_;
};

/// Removes a Listener during a callback to verify recursion works.
class RemoveDuringCallback : public simData::DataStore::DefaultListener
{
public:
  RemoveDuringCallback(CallbackTypes type, simData::DataStore::ListenerPtr listener)
    : type_(type),
      listener_(listener)
  {
  }

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == AddEntity))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == RemoveEntity))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    if ((listener_ != nullptr) && (type_ == PrefsChange))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onChange(simData::DataStore *source)
  {
    if ((listener_ != nullptr) && (type_ == TimeChange))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    if ((listener_ != nullptr) && (type_ == CategoryDataChange))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    if ((listener_ != nullptr) && (type_ == NameChange))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onFlush(simData::DataStore *source, simData::ObjectId flushedId)
  {
    if ((listener_ != nullptr) && (type_ == Flush))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

  virtual void onScenarioDelete(simData::DataStore* source)
  {
    if ((listener_ != nullptr) && (type_ == ScenarioDelete))
    {
      source->removeListener(listener_);
      listener_.reset();
    }
  }

protected:
  CallbackTypes type_;
  simData::DataStore::ListenerPtr listener_;
};

/// Removes a Listener during a callback to verify recursion works.
class RemoveMultipleDuringCallback : public simData::DataStore::DefaultListener
{
public:
  RemoveMultipleDuringCallback(CallbackTypes type, simData::DataStore::ListenerPtr listener1, simData::DataStore::ListenerPtr listener2)
    : type_(type),
      listener1_(listener1),
      listener2_(listener2)
  {
  }

  virtual void onChange(simData::DataStore *source)
  {
    if ((listener1_ != nullptr) && (type_ == TimeChange))
    {
      source->removeListener(listener1_);
      source->removeListener(listener2_);
      listener1_.reset();
      listener2_.reset();
    }
  }
protected:
  CallbackTypes type_;
  simData::DataStore::ListenerPtr listener1_;
  simData::DataStore::ListenerPtr listener2_;
};

int testAddEntity()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(AddEntity, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));
  // Added DURING the Add so the Add counter should NOT have been incremented, but the others will be updated
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 1, 0, 0, 1, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(AddEntity, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  testHelper.addPlatform();

  // Added BEFORE the Add so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));
  // Removed DURING the Add but AFTER it was called so the Add counter should have been incremented, but the others will NOT be incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(1, 0, 0, 0, 0, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(AddEntity, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  testHelper.addPlatform();

  // Added BEFORE the Add so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));
  // Removed DURING the Add but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testRemoveEntity()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  uint64_t platId1 = testHelper.addPlatform();
  uint64_t platId2 = testHelper.addPlatform();
  uint64_t platId3 = testHelper.addPlatform();

  // Added 3 platforms, so the add, pref and name counters should be 3
  rv += SDK_ASSERT(counter->compareAndClear(3, 0, 3, 0, 0, 3, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(RemoveEntity, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->removeEntity(platId1);

  // Removed a platform, so the remove counter should be 1
  rv += SDK_ASSERT(counter->compareAndClear(0, 1, 0, 0, 0, 0, 0, 0));
  // Added DURING the remove so the counters should 0
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(RemoveEntity, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->removeEntity(platId2);

  // Added BEFORE the Add so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 1, 0, 0, 0, 0, 0, 0));
  // Removed DURING the Add but AFTER it was called so the Add counter should have been incremented, but the others will NOT be incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 1, 0, 0, 0, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(RemoveEntity, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->removeEntity(platId3);

  // Added BEFORE the Add so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 1, 0, 0, 0, 0, 0, 0));
  // Removed DURING the Add but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testPrefsChange()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(PrefsChange, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  uint64_t platId1 = testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  simData::PlatformPrefs prefs;
  prefs.mutable_commonprefs()->set_color(1);
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the color change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 0, 0, 0));
  // Added DURING the color change so NO counters should have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(PrefsChange, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  prefs.mutable_commonprefs()->set_color(2);
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the color change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 0, 0, 0));
  // Removed DURING the color changed but AFTER it was called so the counters should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 0, 1, 0, 0, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(PrefsChange, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  prefs.mutable_commonprefs()->set_color(3);
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the color change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 0, 0, 0));
  // Removed DURING the color changed but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testTimeChange()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(TimeChange, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  ds->update(0.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Added DURING the update so the counter should NOT have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(TimeChange, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(1.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Removed DURING the update but AFTER it was called so the counter should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(TimeChange, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(2.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Removed DURING the update but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testCategoryDataChange()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(CategoryDataChange, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  uint64_t platId1 = testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  testHelper.addCategoryData(platId1, "Key", "Value1", 0.0);
  testHelper.addCategoryData(platId1, "Key", "Value2", 0.5);
  testHelper.addCategoryData(platId1, "Key", "Value3", 1.5);
  testHelper.addCategoryData(platId1, "Key", "Value4", 2.5);

  ds->update(0.0);

  // Added BEFORE the update so the time and category counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 1, 0, 0, 0));
  // Added DURING the update so NO counters should have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(CategoryDataChange, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(1.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 1, 0, 0, 0));
  // Removed DURING the update but AFTER it was called so the counter should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 0, 0, 0, 1, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(CategoryDataChange, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(2.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 1, 0, 0, 0));
  // Removed DURING the update but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testNameChange()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(NameChange, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  uint64_t platId1 = testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  simData::PlatformPrefs prefs;
  prefs.mutable_commonprefs()->set_name("NewName1");
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the name change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 1, 0, 0));
  // Added DURING the pref change so NO counters should have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(NameChange, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  prefs.mutable_commonprefs()->set_name("NewName2");
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the name change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 1, 0, 0));
  // Removed DURING the name changed but AFTER it was called so the counters should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 0, 1, 0, 0, 1, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(NameChange, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  prefs.mutable_commonprefs()->set_name("NewName3");
  testHelper.updatePlatformPrefs(prefs, platId1);

  // Added BEFORE the name change so the pref and name counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 1, 0, 0, 1, 0, 0));
  // Removed DURING the update but BEFORE it was called so the counters should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testFlush()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(Flush, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  uint64_t platId1 = testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  ds->flush(platId1);

  // Added BEFORE the flush so the time and category counters should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 0, 0, 0, 1, 0));
  // Added DURING the flush so NO counters should have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemoveShared(calledBeforeRemove);
  ds->addListener(calledBeforeRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(Flush, calledBeforeRemoveShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->flush(platId1);

  // Added BEFORE the flush so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 0, 0, 0, 1, 0));
  // Removed DURING the flush but AFTER it was called so the counter should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove->compareAndClear(0, 0, 0, 0, 0, 0, 1, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemoveShared = simData::DataStore::ListenerPtr(calledAfterRemove);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveDuringCallback(Flush, calledAfterRemoveShared)));
  ds->addListener(calledAfterRemoveShared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->flush(platId1);

  // Added BEFORE the flush so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 0, 0, 0, 1, 0));
  // Removed DURING the flush but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

int testScenarioDelete()
{
  int rv = 0;

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);

  {
    simUtil::DataStoreTestHelper testHelper;
    simData::DataStore* ds = testHelper.dataStore();
    ds->addListener(counterShared);
  }

  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 1));

  // Not a reasonable use case to add and remove observers while deleting a scenario
  return rv;
}

int testMultipleRemoval()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  CounterListener* counter = new CounterListener;
  simData::DataStore::ListenerPtr counterShared(counter);
  ds->addListener(counterShared);

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* addedCounter = new CounterListener;
  simData::DataStore::ListenerPtr addedCounterShared(addedCounter);
  ds->addListener(simData::DataStore::ListenerPtr(new AddDuringCallback(TimeChange, addedCounterShared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  testHelper.addPlatform();
  // Added a platform, so the add, pref and name counters should be 1
  rv += SDK_ASSERT(counter->compareAndClear(1, 0, 1, 0, 0, 1, 0, 0));

  ds->update(0.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Added DURING the update so the counter should NOT have been incremented
  rv += SDK_ASSERT(addedCounter->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  CounterListener* calledBeforeRemove1 = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemove1Shared(calledBeforeRemove1);
  ds->addListener(calledBeforeRemove1Shared);
  CounterListener* calledBeforeRemove2 = new CounterListener;
  simData::DataStore::ListenerPtr calledBeforeRemove2Shared(calledBeforeRemove2);
  ds->addListener(calledBeforeRemove2Shared);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveMultipleDuringCallback(TimeChange, calledBeforeRemove1Shared, calledBeforeRemove2Shared)));
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(1.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Removed DURING the update but AFTER it was called so the counter should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove1->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Removed DURING the update but AFTER it was called so the counter should have been incremented
  rv += SDK_ASSERT(calledBeforeRemove2->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));

  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));
  CounterListener* calledAfterRemove1 = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemove1Shared = simData::DataStore::ListenerPtr(calledAfterRemove1);
  CounterListener* calledAfterRemove2 = new CounterListener;
  simData::DataStore::ListenerPtr calledAfterRemove2Shared = simData::DataStore::ListenerPtr(calledAfterRemove2);
  ds->addListener(simData::DataStore::ListenerPtr(new RemoveMultipleDuringCallback(TimeChange, calledAfterRemove1Shared, calledAfterRemove2Shared)));
  ds->addListener(calledAfterRemove1Shared);
  ds->addListener(calledAfterRemove2Shared);
  ds->addListener(simData::DataStore::ListenerPtr(new DoesNothingCallback));

  ds->update(2.0);

  // Added BEFORE the update so the counter should have been incremented
  rv += SDK_ASSERT(counter->compareAndClear(0, 0, 0, 1, 0, 0, 0, 0));
  // Removed DURING the update but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove1->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));
  // Removed DURING the update but BEFORE it was called so the counter should NOT have been incremented
  rv += SDK_ASSERT(calledAfterRemove2->compareAndClear(0, 0, 0, 0, 0, 0, 0, 0));

  return rv;
}

}

int TestListener(int argc, char* argv[])
{
  int rv = 0;

  rv += testAddEntity();
  rv += testRemoveEntity();
  rv += testPrefsChange();
  rv += testTimeChange();
  rv += testCategoryDataChange();
  rv += testNameChange();
  rv += testFlush();
  rv += testScenarioDelete();
  rv += testMultipleRemoval();

  return rv;
}
