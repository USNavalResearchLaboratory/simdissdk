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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/Common/SDKAssert.h"
#include "simUtil/IdMapper.h"
#include "simUtil/DataStoreTestHelper.h"

namespace {

/** Helper function to set an entity name */
int setName(simData::DataStore& ds, uint64_t id, const std::string& name)
{
  simData::DataStore::Transaction txn;
  auto commonPrefs = ds.mutable_commonPrefs(id, &txn);
  if (!commonPrefs)
    return 1;
  commonPrefs->set_name(name);
  txn.complete(&commonPrefs);
  return 0;
}

/** Test the simUtil::IdMapper routines */
int testMapping()
{
  int rv = 0;

  // Create two platforms
  simUtil::DataStoreTestHelper dsHelper;
  simData::DataStore& dataStore = *dsHelper.dataStore();
  const uint64_t plat1 = dsHelper.addPlatform(10);
  // If plat1 had a 0 ID, then we'd break down the line...
  rv += SDK_ASSERT(plat1 != 0);
  setName(dataStore, plat1, "plat1");
  const uint64_t plat2 = dsHelper.addPlatform(20);
  setName(dataStore, plat2, "plat2");

  simUtil::DataStoreIdMapper map(dataStore);

  // Expect nothing back because the mapper has not "remote" mappings
  rv += SDK_ASSERT(map.map(0) == 0);
  rv += SDK_ASSERT(map.map(plat1) == 0);
  rv += SDK_ASSERT(map.map(1) == 0);
  rv += SDK_ASSERT(map.map(10) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);

  // Remote data store tells us it has a mapping of 210 that matches our plat1
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 210) == 0);
  rv += SDK_ASSERT(map.map(0) == 0);
  rv += SDK_ASSERT(map.map(plat1) == 0);
  rv += SDK_ASSERT(map.map(1) == 0);
  rv += SDK_ASSERT(map.map(10) == 0);
  // 210 should match "plat1" ID
  rv += SDK_ASSERT(map.map(210) == plat1);

  // Clear out the mappings and ensure it really cleared everything out
  map.clearMappings();
  rv += SDK_ASSERT(map.map(0) == 0);
  rv += SDK_ASSERT(map.map(plat1) == 0);
  rv += SDK_ASSERT(map.map(1) == 0);
  rv += SDK_ASSERT(map.map(10) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);

  // Re-add plat1, then explicitly remove it
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 210) == 0);
  rv += SDK_ASSERT(map.map(210) == plat1);
  rv += SDK_ASSERT(map.removeId(210) == 0);
  // Everything should be unmapped
  rv += SDK_ASSERT(map.map(0) == 0);
  rv += SDK_ASSERT(map.map(plat1) == 0);
  rv += SDK_ASSERT(map.map(1) == 0);
  rv += SDK_ASSERT(map.map(10) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);

  // Re-add plat1, but use the other interface
  simUtil::DataStoreIdMapper::EntityIdData idStruct;
  idStruct.id = 210;
  idStruct.originalId = 10;
  idStruct.entityName = "plat1";
  idStruct.hostPlatformId = 210;
  rv += SDK_ASSERT(map.addMapping(idStruct) == 0);
  rv += SDK_ASSERT(map.map(210) == plat1);
  map.clearMappings();

  // Ensure that we have to set the host platform ID correctly for platforms to match
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 0) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);
  map.clearMappings();
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 100) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);
  map.clearMappings();

  // Removing the ID 210 again should cause issues.  So should a new unknown ID
  rv += SDK_ASSERT(map.removeId(210) != 0);
  rv += SDK_ASSERT(map.removeId(310) != 0);

  // Make sure that addMapping() does not fail on changing the mapping, which might happen
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 210) == 0);
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 210) == 0);
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat1", 200) == 0);
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat2", 210) == 0);
  rv += SDK_ASSERT(map.addMapping(210, 11, "plat1", 210) == 0);

  // Make sure we get notifications in the mapper when something starts to match
  rv += SDK_ASSERT(map.addMapping(230, 30, "plat3", 230) == 0);
  // Make sure that this doesn't match anything
  rv += SDK_ASSERT(map.map(230) == 0);

  // Add a platform that DOES match
  uint64_t plat3 = dsHelper.addPlatform(30);
  setName(dataStore, plat3, "plat3");
  rv += SDK_ASSERT(map.map(230) == plat3);

  // Remove the platform from the data store and make sure mapping lost it
  dataStore.removeEntity(plat3);
  plat3 = 0;
  rv += SDK_ASSERT(map.map(230) == 0);

  // Make sure our mappings for platforms match up before we continue
  rv += SDK_ASSERT(map.addMapping(210, 10, "plat2", 210) == 0);
  rv += SDK_ASSERT(map.addMapping(220, 20, "plat2", 220) == 0);
  rv += SDK_ASSERT(map.map(210) == plat1);
  rv += SDK_ASSERT(map.map(220) == plat2);

  // Create a few gates of different hosts
  const uint64_t p1beam = dsHelper.addBeam(plat1, 41);
  setName(dataStore, p1beam, "p1b");
  const uint64_t p2beam = dsHelper.addBeam(plat2, 42);
  setName(dataStore, p1beam, "p2b");

  // DIFFERENT HOST: Create 2 gates with same original ID and name but different host
  const uint64_t gate1_diffhost = dsHelper.addGate(p1beam, 51);
  setName(dataStore, gate1_diffhost, "diffhost");
  const uint64_t gate2_diffhost = dsHelper.addGate(p2beam, 51);
  setName(dataStore, gate2_diffhost, "diffhost");
  // Make sure we can discriminate
  rv += SDK_ASSERT(map.map(251) == 0);
  rv += SDK_ASSERT(map.map(252) == 0);
  // Reverse the order to ensure it's not a first-found thing
  rv += SDK_ASSERT(map.addMapping(252, 51, "diffhost", 220) == 0); // 252 -> gate2
  rv += SDK_ASSERT(map.addMapping(251, 51, "diffhost", 210) == 0); // 251 -> gate1
  rv += SDK_ASSERT(map.map(251) == gate1_diffhost);
  rv += SDK_ASSERT(map.map(252) == gate2_diffhost);

  // DIFFERENT ORIGINAL ID: Create 2 gates with same name and host, but different original ID
  const uint64_t gate1_diffoid = dsHelper.addGate(p1beam, 61);
  setName(dataStore, gate1_diffoid, "diffoid");
  const uint64_t gate2_diffoid = dsHelper.addGate(p2beam, 62);
  setName(dataStore, gate2_diffoid, "diffoid");
  // Make sure we can discriminate
  rv += SDK_ASSERT(map.map(261) == 0);
  rv += SDK_ASSERT(map.map(262) == 0);
  rv += SDK_ASSERT(map.addMapping(261, 61, "diffoid", 210) == 0); // 261 -> gate1
  rv += SDK_ASSERT(map.addMapping(262, 62, "diffoid", 210) == 0); // 262 -> gate2
  rv += SDK_ASSERT(map.map(261) == gate1_diffoid);
  rv += SDK_ASSERT(map.map(262) == gate2_diffoid);

  // DIFFERENT NAME: Create 2 gates with same OID and host, but different names
  const uint64_t gate1_diffname = dsHelper.addGate(p1beam, 71);
  setName(dataStore, gate1_diffname, "diffname1");
  const uint64_t gate2_diffname = dsHelper.addGate(p2beam, 72);
  setName(dataStore, gate2_diffname, "diffname2");
  // Make sure we can discriminate
  rv += SDK_ASSERT(map.map(271) == 0);
  rv += SDK_ASSERT(map.map(272) == 0);
  rv += SDK_ASSERT(map.addMapping(271, 71, "diffname1", 220) == 0); // 271 -> gate1
  rv += SDK_ASSERT(map.addMapping(272, 72, "diffname2", 220) == 0); // 272 -> gate2
  rv += SDK_ASSERT(map.map(271) == gate1_diffname);
  rv += SDK_ASSERT(map.map(272) == gate2_diffname);

  return rv;
}

int testHostlessCustomRendering()
{
  int rv = 0;

  // Create two CR
  simUtil::DataStoreTestHelper dsHelper;
  simData::DataStore& dataStore = *dsHelper.dataStore();

  const uint64_t cr1 = dsHelper.addCustomRendering(0, 0);
  setName(dataStore, cr1, "cr1");

  const uint64_t cr2 = dsHelper.addCustomRendering(0, 0);
  setName(dataStore, cr2, "cr2");

  simUtil::DataStoreIdMapper map(dataStore);

  // Expect nothing back because the mapper has not "remote" mappings
  rv += SDK_ASSERT(map.map(0) == 0);
  rv += SDK_ASSERT(map.map(cr1) == 0);
  rv += SDK_ASSERT(map.map(cr2) == 0);
  rv += SDK_ASSERT(map.map(1) == 0);
  rv += SDK_ASSERT(map.map(10) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);

  // Remote data store tells us it has a mapping of 210 that matches our cr1
  rv += SDK_ASSERT(map.addMapping(210, 0, "cr1", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.map(210) == cr1);

  // Remote data store tells us it has a mapping of 220 that matches our cr2
  rv += SDK_ASSERT(map.addMapping(220, 0, "cr2", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.map(210) == cr1);
  rv += SDK_ASSERT(map.map(220) == cr2);

  // Clear the map and make sure there are no matches
  map.clearMappings();
  rv += SDK_ASSERT(map.map(210) == 0);
  rv += SDK_ASSERT(map.map(220) == 0);

  // Add back in with wrong original ids
  rv += SDK_ASSERT(map.addMapping(210, 1, "cr1", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.addMapping(220, 2, "cr2", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.map(210) == 0);
  rv += SDK_ASSERT(map.map(220) == 0);

  // Clear the map and make sure there are no matches
  map.clearMappings();
  rv += SDK_ASSERT(map.map(210) == 0);
  rv += SDK_ASSERT(map.map(220) == 0);

  // Add back in
  rv += SDK_ASSERT(map.addMapping(210, 0, "cr1", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.addMapping(220, 0, "cr2", 0, simData::CUSTOM_RENDERING) == 0);
  rv += SDK_ASSERT(map.map(210) == cr1);
  rv += SDK_ASSERT(map.map(220) == cr2);

  // Remove just one
  map.removeId(210);
  rv += SDK_ASSERT(map.map(210) == 0);
  rv += SDK_ASSERT(map.map(220) == cr2);

  // Remove the second entry
  map.removeId(220);
  rv += SDK_ASSERT(map.map(210) == 0);
  rv += SDK_ASSERT(map.map(220) == 0);

  return rv;
}

}

int IdMapperTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testMapping() == 0);
  rv += SDK_ASSERT(testHostlessCustomRendering() == 0);

  return rv;
}
