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
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"
#include "simQt/RegExpImpl.h"


namespace
{

void addCategoryData(uint64_t entityId, simData::DataStore* ds,
  const std::string& catName,
  const std::string& catVal,
  double time)
{
  simData::DataStore::Transaction t;
  simData::CategoryData *cd = ds->addCategoryData(entityId, &t);
  cd->set_time(time);
  simData::CategoryData_Entry *e1 = cd->add_entry();
  e1->set_key(catName);
  e1->set_value(catVal);
  t.commit();
}

int categoryFilterRegExpTest()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::CategoryNameManager& catNameMgr = ds->categoryNameManager();

  uint64_t platId1 = testHelper.addPlatform();
  uint64_t platId2 = testHelper.addPlatform();
  uint64_t platId3 = testHelper.addPlatform();
  uint64_t platId4 = testHelper.addPlatform();
  uint64_t platId5 = testHelper.addPlatform();

  simData::CategoryFilter catFilter(ds, true);

  addCategoryData(platId1, ds, "Cat1", "3412", -1.0);
  addCategoryData(platId2, ds, "Cat1", "3000", -1.0);
  addCategoryData(platId3, ds, "Cat1", "3476", -1.0);
  addCategoryData(platId4, ds, "Cat1", "3477", -1.0);
  addCategoryData(platId5, ds, "Cat1", "1234", -1.0);

  addCategoryData(platId1, ds, "Cat2", "099", -1.0);
  addCategoryData(platId2, ds, "Cat2", "100", -1.0);
  addCategoryData(platId3, ds, "Cat2", "450", -1.0);
  addCategoryData(platId4, ds, "Cat2", "032", -1.0);
  addCategoryData(platId5, ds, "Cat2", "455", -1.0);

  addCategoryData(platId1, ds, "Cat3", "someText", -1.0);
  addCategoryData(platId2, ds, "Cat3", "4000", -1.0);
  addCategoryData(platId3, ds, "Cat3", "4500", -1.0);
  addCategoryData(platId4, ds, "Cat3", "4501", -1.0);
  addCategoryData(platId5, ds, "Cat3", "moreText", -1.0);

  ds->update(0.0);
  catFilter.updateAll(false);
  simQt::RegExpFilterFactoryImpl regExpFactory;

  // test Cat1 values 0072, 1234, 3400-3476, 6100-6110
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat1"), regExpFactory.createRegExpFilter("^0072|1234|34[0-6][0-9]|347[0-6]|610[0-9]|6110$"));
  rv += SDK_ASSERT(catFilter.match(platId1)); // Cat1 of 3412 is within the 3400-3476 range
  rv += SDK_ASSERT(!catFilter.match(platId2)); // Cat1 of 3000 is outside of all ranges, no match
  rv += SDK_ASSERT(catFilter.match(platId3)); // Cat1 of 3476 is at the limit of the 3400-3476 range
  rv += SDK_ASSERT(!catFilter.match(platId4)); // Cat1 of 3477 is outside the range of 3400-3476
  rv += SDK_ASSERT(catFilter.match(platId5)); // Cat1 of 1234 matches an item in the list

  // test Cat2 values 032, 100-110, 450-455, adding a second regexp
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat2"), regExpFactory.createRegExpFilter("^032|10[0-9]|110|45[0-5]$"));
  rv += SDK_ASSERT(!catFilter.match(platId1)); // Cat2 of 099 fails, so failure, even though Cat1 still matches
  rv += SDK_ASSERT(!catFilter.match(platId2)); // Cat2 of 100 matchs, but still fails, since Cat1 still fails
  rv += SDK_ASSERT(catFilter.match(platId3)); // both Cat1 and Cat2 now match, so pass
  rv += SDK_ASSERT(!catFilter.match(platId4)); // Cat2 of 032 matches, but still fails since Cat1 still fails
  rv += SDK_ASSERT(catFilter.match(platId5)); // Cat2 of 455 matches, limit of range 450-455, passes since Cat1 also matches

  // test Cat3 values "more*", adding a third regexp
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat3"), regExpFactory.createRegExpFilter("more*"));
  // note that only platId5 should now pass, since it is the only one that matches all 3 regexp filters
  rv += SDK_ASSERT(!catFilter.match(platId1));
  rv += SDK_ASSERT(!catFilter.match(platId2));
  rv += SDK_ASSERT(!catFilter.match(platId3));
  rv += SDK_ASSERT(!catFilter.match(platId4));
  rv += SDK_ASSERT(catFilter.match(platId5));

  // update the Cat2 on platId5 to now fail, testing response to category data change
  addCategoryData(platId5, ds, "Cat2", "456", 1.0); // now outside of range 450-455
  ds->update(1.0);
  rv += SDK_ASSERT(!catFilter.match(platId5)); // now fails, since Cat2 should now fail, though Cat1 still matches

  // unset the regexp filters; some rely on pattern.empty()
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat1"), regExpFactory.createRegExpFilter(""));
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat2"), regExpFactory.createRegExpFilter(""));
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat3"), simData::RegExpFilterPtr());
  // all should match now, since the filter is empty
  rv += SDK_ASSERT(catFilter.match(platId1));
  rv += SDK_ASSERT(catFilter.match(platId2));
  rv += SDK_ASSERT(catFilter.match(platId3));
  rv += SDK_ASSERT(catFilter.match(platId4));
  rv += SDK_ASSERT(catFilter.match(platId5));

  // now test out some serializing and deserializing

  // first deserialize the Cat1 regexp
  std::string cat1String = "Cat1(1)^^0072|1234|34[0-6][0-9]|347[0-6]|610[0-9]|6110$";
  catFilter.deserialize(cat1String, regExpFactory);
  rv += SDK_ASSERT(catFilter.serialize() == cat1String);
  // behavior should be the same as above
  rv += SDK_ASSERT(catFilter.match(platId1));
  rv += SDK_ASSERT(!catFilter.match(platId2));
  rv += SDK_ASSERT(catFilter.match(platId3));
  rv += SDK_ASSERT(!catFilter.match(platId4));
  rv += SDK_ASSERT(catFilter.match(platId5));

  // now deserialize the Cat1 and Cat2 regexps
  std::string cat2String = cat1String + "`Cat2(1)^^032|1[0-1][0-9]|45[0-5]$";
  catFilter.deserialize(cat2String, regExpFactory);
  rv += SDK_ASSERT(catFilter.serialize() == cat2String);

  // behavior should be the same as above
  rv += SDK_ASSERT(!catFilter.match(platId1));
  rv += SDK_ASSERT(!catFilter.match(platId2));
  rv += SDK_ASSERT(catFilter.match(platId3));
  rv += SDK_ASSERT(!catFilter.match(platId4));
  rv += SDK_ASSERT(!catFilter.match(platId5));

  // clear out the regexp
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat1"), simData::RegExpFilterPtr());
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat2"), simData::RegExpFilterPtr());
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat3"), simData::RegExpFilterPtr());

  // test bad regular expression syntax
  std::string badString = "SomeCategory(1)^87[0-";
  std::cout << "Error output is expected below.\n";
  simData::RegExpFilterPtr invalidPtr = regExpFactory.createRegExpFilter(badString);
  rv += SDK_ASSERT(!invalidPtr); // ptr should be invalid

  // add a new platform with no data
  uint64_t platId6 = testHelper.addPlatform();

  // test '.*', all should pass
  catFilter.setCategoryRegExp(catNameMgr.nameToInt("Cat1"), regExpFactory.createRegExpFilter(".*"));
  rv += SDK_ASSERT(catFilter.match(platId1));
  rv += SDK_ASSERT(catFilter.match(platId2));
  rv += SDK_ASSERT(catFilter.match(platId3));
  rv += SDK_ASSERT(catFilter.match(platId4));
  rv += SDK_ASSERT(catFilter.match(platId5));
  rv += SDK_ASSERT(catFilter.match(platId6)); // has no values, so it should pass

  return rv;
}
}

int CategoryRegExpTest(int argc, char *argv[])
{
  // start test
  int rv = 0;

  rv += categoryFilterRegExpTest();

  return rv;
}
