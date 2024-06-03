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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

/** Helper function to reduce number of calls in another test. Returns true if ids matches idListByName(name). */
bool idListByNameEquals(simData::DataStore& ds, const std::string& name, const simData::DataStore::IdList& ids)
{
  simData::DataStore::IdList returnedIds;
  ds.idListByName(name, &returnedIds);
  return ids == returnedIds;
}

int testAliasInvalidation()
{
  // Intended to test SIM-14208: If a name changes occurs while the Alias flag is set then
  // EntityNameCache::nameChange is not called and the cache is no longer valid.
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  const uint64_t id = testHelper.addPlatform();

  simData::DataStore& dataStore = *testHelper.dataStore();

  // Set the call sign and the alias at the same time, confirm behavior
  simData::DataStore::Transaction txn;
  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_name("name");
  prefs->mutable_commonprefs()->set_alias("alias");
  prefs->mutable_commonprefs()->set_usealias(false);
  txn.complete(&prefs);

  const simData::DataStore::IdList LIST_EMPTY;
  const simData::DataStore::IdList LIST_WITH_ID = { id };

  // "Identity" test should return the ID
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name", LIST_WITH_ID));
  // Should not return alias since alias is not enabled
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias", LIST_EMPTY));
  // Subsets of "name" shouldn't match
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "nam", LIST_EMPTY));

  // Update the name value and retest
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_name("name2");
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name", LIST_EMPTY));

  // Update alias, should have no impact on results because use-alias is unset
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_alias("alias2");
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias2", LIST_EMPTY));

  // Turn on alias flag. This should not change results, since entity name cache (idListByName) only functions on name.
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_usealias(true);
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias2", LIST_EMPTY));

  // Change the name. Even though alias is off, this should update the cache.
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_name("name3");
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name3", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias2", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_EMPTY));

  // Update the alias. Should not impact results.
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_alias("alias3");
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name3", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias3", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias2", LIST_EMPTY));

  // Turn off the alias flag. Should still have no impact on results.
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_usealias(false);
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name3", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name2", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias3", LIST_EMPTY));

  // Change the name. Should update results still.
  prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->set_name("name4");
  txn.complete(&prefs);
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name4", LIST_WITH_ID));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "alias2", LIST_EMPTY));
  rv += SDK_ASSERT(idListByNameEquals(dataStore, "name3", LIST_EMPTY));

  return rv;
}

}

int TestEntityNameCache(int argc, char* argv[])
{
  int rv = 0;

  rv += testAliasInvalidation();

  std::cout << "TestEntityNameCache: " << (rv == 0 ? "PASSED" : "FAILED") << std::endl;

  return rv;
}
