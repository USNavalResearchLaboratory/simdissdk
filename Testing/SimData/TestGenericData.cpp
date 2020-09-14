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

#include "simCore/Common/SDKAssert.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

// Helper method to turn on/off the ignoreduplicategenericdata() flag in Scenario Properties
void setIgnoreDupeGD_(simData::DataStore& ds, bool fl)
{
  simData::DataStore::Transaction txn;
  simData::ScenarioProperties* props = ds.mutable_scenarioProperties(&txn);
  props->set_ignoreduplicategenericdata(fl);
  txn.complete(&props);
}

int testDataLimiting()
{
  int rv = 0;

  simUtil::DataStoreTestHelper dsth;
  simData::DataStore* ds = dsth.dataStore();
  ds->setDataLimiting(true);
  // This test uses live mode and relies on duplicate "value" entries for correct behavior
  setIgnoreDupeGD_(*ds, false);

  uint64_t platformId = dsth.addPlatform();
  simData::PlatformPrefs prefs;
  simData::CommonPrefs* commonPrefs = prefs.mutable_commonprefs();
  commonPrefs->set_datalimitpoints(3);
  dsth.updatePlatformPrefs(prefs, platformId);

  dsth.addPlatformUpdate(0.0, platformId);
  dsth.addPlatformUpdate(100.0, platformId);

  dsth.addGenericData(platformId, "TestKey", "TestValue", 0.0);
  dsth.addGenericData(platformId, "TestKey", "TestValue", 1.0);
  dsth.addGenericData(platformId, "TestKey", "TestValue", 2.0);

  const simData::GenericDataSlice* gds = ds->genericDataSlice(platformId);

  // Put in up to the limit, so OK
  rv += SDK_ASSERT(gds->numItems() == 3);

  dsth.addGenericData(platformId, "TestKey", "TestValue", 3.0);

  // One will get dropped
  rv += SDK_ASSERT(gds->numItems() == 3);

  dsth.addGenericData(platformId, "TestKey2", "TestValue", 3.0);

  // Since two have the same time; nothing will get dropped
  gds = ds->genericDataSlice(platformId);

  dsth.addGenericData(platformId, "TestKey", "TestValue", 4.0);

  // One will get dropped
  rv += SDK_ASSERT(gds->numItems() == 4);

  dsth.addGenericData(platformId, "TestKey", "TestValue", 5.0);

  // One will get dropped
  rv += SDK_ASSERT(gds->numItems() == 4);

  dsth.addGenericData(platformId, "TestKey", "TestValue", 6.0);

  // The TestKey at time 3 gets dropped, but TestKey2 at time 3 stays
  rv += SDK_ASSERT(gds->numItems() == 4);

  return rv;
}

/** Returns 0 when the data store's 0 entry's current contains tag=value once and only once. */
int testCurrentValues(simData::DataStore& ds, const std::string& tag, const std::string& value)
{
  const simData::GenericDataSlice* gd = ds.genericDataSlice(0);
  if (gd == nullptr)
    return 1;
  const simData::GenericData* current = gd->current();
  if (current == nullptr)
    return 1;

  bool foundTag = false;
  int rv = 1;
  for (int k = 0; k < current->entry_size(); ++k)
  {
    const simData::GenericData_Entry& entry = current->entry(k);
    if (entry.key() == tag)
    {
      // Try to make sure we only ever have a tag once in "current"
      if (foundTag)
        return 1;
      foundTag = true;
      rv = (entry.value() == value) ? 0 : 1;
    }
  }
  return rv;
}

/** Returns 0 when the data store's 0 entry's current contains tag=value once and only once. */
int testShouldBeEmpty(simData::DataStore& ds)
{
  const simData::GenericDataSlice* gd = ds.genericDataSlice(0);
  if (gd == nullptr)
    return 1;
  const simData::GenericData* current = gd->current();
  if (current == nullptr)
    return 1;

  return current->entry_size();
}

int test_simpleInfinite()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(true);
  testHelper.addGenericData(0, "Tag1", "data1", 0);
  testHelper.addGenericData(0, "Tag1", "data2", 1);
  // Note that data is added out of order
  testHelper.addGenericData(0, "Tag1", "data3", 5);
  testHelper.addGenericData(0, "Tag1", "data4", 3);
  // Update and make sure we did not get data4
  ds->update(6);
  return SDK_ASSERT(testCurrentValues(*ds, "Tag1", "data3") == 0);
}

int test_simpleInfiniteFile()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(false);
  testHelper.addGenericData(0, "SIMDIS_Callsign", "data1", 46.973);
  testHelper.addGenericData(0, "SIMDIS_Callsign", "data2", 46.974);
  testHelper.addGenericData(0, "SIMDIS_Callsign", "data2", 50.000);
  testHelper.addGenericData(0, "SIMDIS_Callsign", "data1", 50.001);

  ds->update(47);
  rv += SDK_ASSERT(testCurrentValues(*ds, "SIMDIS_Callsign", "data2") == 0);

  ds->update(51);
  rv += SDK_ASSERT(testCurrentValues(*ds, "SIMDIS_Callsign", "data1") == 0);

  ds->update(46.973);
  rv += SDK_ASSERT(testCurrentValues(*ds, "SIMDIS_Callsign", "data1") == 0);

  ds->update(50);
  rv += SDK_ASSERT(testCurrentValues(*ds, "SIMDIS_Callsign", "data2") == 0);

  return rv;
}

int test_simpleNormal()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(true);
  testHelper.addGenericData(0, "Tag1", "data1", 0);
  testHelper.addGenericData(0, "Tag1", "data2", 1);
  // Note that data is added out of order
  testHelper.addGenericData(0, "Tag1", "data3", 5);
  testHelper.addGenericData(0, "Tag1", "data4", 3);
  ds->update(10);
  return SDK_ASSERT(testCurrentValues(*ds, "Tag1", "data3") == 0);
}

int test_limitPoints()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(true);
  simData::DataStore::Transaction tx;
  simData::ScenarioProperties* props = ds->mutable_scenarioProperties(&tx);
  props->set_datalimitpoints(1);
  tx.complete(&props);

  testHelper.addGenericData(0, "Tag1", "data1", 1);
  testHelper.addGenericData(0, "Tag2", "data1", 2);
  testHelper.addGenericData(0, "Tag1", "data2", 4);
  testHelper.addGenericData(0, "Tag2", "data2", 4);
  ds->update(5.0);

  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "data2") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "data2") == 0);
  return rv;
}

int test_limitTime()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(true);
  simData::DataStore::Transaction tx;
  simData::ScenarioProperties* props = ds->mutable_scenarioProperties(&tx);
  props->set_datalimitpoints(0);
  props->set_datalimittime(1);
  tx.complete(&props);

  testHelper.addGenericData(0, "Tag1", "data1", 1);
  testHelper.addGenericData(0, "Tag2", "data1", 2);
  testHelper.addGenericData(0, "Tag1", "data2", 3);
  testHelper.addGenericData(0, "Tag2", "data2", 4);
  ds->update(4.0);

  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "data2") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "data2") == 0);

  ds->update(2.9);
  rv += SDK_ASSERT(testShouldBeEmpty(*ds) == 0);

  return rv;
}

int test_Sim4722_CurrentGenData()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(false);

  testHelper.addGenericData(0, "Tag1", "a", 0);
  testHelper.addGenericData(0, "Tag2", "b", 1);
  testHelper.addGenericData(0, "Tag1", "c", 2);
  testHelper.addGenericData(0, "Tag2", "d", 3);

  ds->update(0.0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "a") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "a") != 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "b") != 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "c") != 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "d") != 0);

  ds->update(1.0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "a") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "b") == 0);

  ds->update(2.0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "c") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "b") == 0);

  ds->update(3.0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "c") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "d") == 0);

  // Updating to the same time should get the same results
  ds->update(3.0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag1", "c") == 0);
  rv += SDK_ASSERT(testCurrentValues(*ds, "Tag2", "d") == 0);

  return rv;
}

int test_ignoreDuplicates()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  ds->setDataLimiting(true);
  setIgnoreDupeGD_(*ds, false);
  const simData::MemoryGenericDataSlice* gd = dynamic_cast<const simData::MemoryGenericDataSlice*>(ds->genericDataSlice(0));

  // Should have no filtering because of false flag on ignoreduplicategenericdata()
  testHelper.addGenericData(0, "Key1", "a", 10);
  testHelper.addGenericData(0, "Key1", "a", 20);
  rv += SDK_ASSERT(gd->numItems() == 2);

  // Should have filtering
  setIgnoreDupeGD_(*ds, true);
  testHelper.addGenericData(0, "Key2", "a", 10);
  testHelper.addGenericData(0, "Key2", "a", 20);
  rv += SDK_ASSERT(gd->numItems() == 3);

  // Should not have filtering because not in live mode
  ds->setDataLimiting(false);
  testHelper.addGenericData(0, "Key3", "a", 10);
  testHelper.addGenericData(0, "Key3", "a", 20);
  rv += SDK_ASSERT(gd->numItems() == 5);

  // Verify the generic data update correctly handles no changes
  ds->update(20.0);
  rv += SDK_ASSERT(gd->current()->entry_size() == 3);
  // Adding a platform will force a call to update on the generic data even though it has not changed
  testHelper.addPlatform();
  ds->update(20.0);
  rv += SDK_ASSERT(gd->current()->entry_size() == 3);

  // Test flush
  ds->flush(0, simData::DataStore::RECURSIVE);
  rv += SDK_ASSERT(gd->numItems() == 0);
  rv += SDK_ASSERT(gd->current()->entry_size() == 0);
  // After an update; should still be zero
  ds->update(20);
  rv += SDK_ASSERT(gd->numItems() == 0);
  rv += SDK_ASSERT(gd->current()->entry_size() == 0);

  return rv;
}

void testPerformanceRepeating()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  ds->setDataLimiting(true);
  setIgnoreDupeGD_(*ds, false);

  for (int ii = 0; ii < 1000000; ++ii)
  {
    double time = ii / 10.0;
    testHelper.addGenericData(0, "Key1", "a", time);
    testHelper.addGenericData(0, "Key2", "b", time);
    testHelper.addGenericData(0, "Key3", "c", time);
    ds->update(time);
    ds->genericDataSlice(0)->current();
  }
}

void testPerformanceNonrepeating()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  ds->setDataLimiting(true);
  setIgnoreDupeGD_(*ds, false);

  for (int ii = 0; ii < 1000000; ++ii)
  {
    double time = ii / 10.0;
    std::stringstream s;
    s << time;
    std::string value = s.str();
    testHelper.addGenericData(0, "Key1", value, time);
    ds->update(time);
    ds->genericDataSlice(0)->current();
  }
}

// More than 5 values between repeating string
void testPerformanceWorstCase()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  ds->setDataLimiting(true);
  setIgnoreDupeGD_(*ds, false);

  for (int ii = 0; ii < 1000000; ++ii)
  {
    double time = ii / 10.0;
    testHelper.addGenericData(0, "Key1", "a", time + 0.001);
    testHelper.addGenericData(0, "Key1", "b", time + 0.002);
    testHelper.addGenericData(0, "Key1", "c", time + 0.003);
    testHelper.addGenericData(0, "Key1", "d", time + 0.004);
    testHelper.addGenericData(0, "Key1", "e", time + 0.005);
    testHelper.addGenericData(0, "Key1", "f", time + 0.006);
    ds->update(time);
    ds->genericDataSlice(0)->current();
  }
}

/// Nothing to test; there will be an STL assert on failure.
void test_5743()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  ds->setDataLimiting(true);

  // Make sure old values get discarded
  for (int ii = 0; ii < 1000; ++ii)
  {
    double time = ii / 10.0;
    testHelper.addGenericData(0, "Key1", "a", time + 0.001);
    testHelper.addGenericData(0, "Key1", "b", time + 0.002);
    testHelper.addGenericData(0, "Key1", "c", time + 0.003);
    testHelper.addGenericData(0, "Key1", "d", time + 0.004);
    testHelper.addGenericData(0, "Key1", "e", time + 0.005);
    // Cause 'a' to be reused every other time
    if ((ii % 2) == 1)
      testHelper.addGenericData(0, "Key1", "f", time + 0.006);
    ds->update(time);
    ds->genericDataSlice(0)->current();
  }

  // retrieve data points
  struct GenericDataSliceCopy : public simData::GenericDataSlice::Visitor
  {
    std::vector<simData::GenericData_Entry> entries;
    virtual void operator()(const simData::GenericData *update)
    {
      if (update == nullptr)
        return;
      for (int k = 0; k < update->entry_size(); ++k)
        entries.push_back(update->entry(k));
    }
  };

  // Make sure visit accounts for discarded old values
  GenericDataSliceCopy sc;
  const simData::GenericDataSlice *gdslice = ds->genericDataSlice(0);
  gdslice->visit(&sc);
}

void testPerformance()
{
  testPerformanceRepeating();
  testPerformanceNonrepeating();
  testPerformanceWorstCase();
}

}

int TestGenericData(int argc, char *argv[])
{
  int rv = 0;

  rv += testDataLimiting();
  rv += test_simpleInfinite();
  rv += test_simpleInfiniteFile();
  rv += test_simpleNormal();
  rv += test_limitPoints();
  rv += test_limitTime();
  rv += test_Sim4722_CurrentGenData();
  rv += test_ignoreDuplicates();
  test_5743();

  // The performance tests are not part of the commit, since they take time and don't generate
  // any errors.
  //testPerformance();

  return rv;
}
