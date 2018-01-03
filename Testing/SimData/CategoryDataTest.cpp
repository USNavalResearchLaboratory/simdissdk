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
#include "simCore/Common/SDKAssert.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/DataStoreTestHelper.h"
// some overlap with TestMemoryDataStore

namespace
{
const uint64_t PLATFORM_ID = 1;

class Finisher
{
public:
  ~Finisher()
  {
    google::protobuf::ShutdownProtobufLibrary();
  }

} finsher;

void loadCategoryData(simData::DataStore &ds)
{
  // insert platform
  simData::DataStore::Transaction t;
  simData::PlatformProperties *p = ds.addPlatform(&t);
  p->set_id(PLATFORM_ID);
  t.commit();
  simData::PlatformPrefs *pp = ds.mutable_platformPrefs(1, &t);
  pp->mutable_commonprefs()->set_name("platform1");
  pp->set_icon("icon1");
  t.commit();

  // insert first category data points
  {
    simData::CategoryData *cd = ds.addCategoryData(p->id(), &t);
    cd->set_time(1.0);

    simData::CategoryData_Entry *e = cd->add_entry();
    e->set_key("key1");
    e->set_value("value1a"); // this value shouldn't be seen (overwritten below) (but will still count as an item in the slice)

    e = cd->add_entry();
    e->set_key("key2");
    e->set_value("value2");

    t.commit();
  }

  // insert duplicate data (should overwrite)
  {
    simData::CategoryData *cd = ds.addCategoryData(p->id(), &t);
    cd->set_time(1.0);

    simData::CategoryData_Entry *e = cd->add_entry();
    e->set_key("key1");
    e->set_value("value1");

    t.commit();
  }

  // insert second category data point
  {
    simData::CategoryData *cd = ds.addCategoryData(p->id(), &t);
    cd->set_time(2.0);

    simData::CategoryData_Entry *e1 = cd->add_entry();
    e1->set_key("key1");
    e1->set_value("value3");

    simData::CategoryData_Entry *e2 = cd->add_entry();
    e2->set_key("key3");
    e2->set_value("value1");

    t.commit();
  }

  // insert second category data point
  {
    simData::CategoryData *cd = ds.addCategoryData(p->id(), &t);
    cd->set_time(3.0);

    simData::CategoryData_Entry *e1 = cd->add_entry();
    e1->set_key("key1");
    e1->set_value("value4");

    t.commit();
  }
}

int testTime1(const simData::CategoryDataSlice &cdslice)
{
  int rv = 0;

  simData::CategoryDataSlice::Iterator c1 = cdslice.current();
  rv += SDK_ASSERT(!c1.hasPrevious());
  rv += SDK_ASSERT(c1.hasNext());
  std::shared_ptr<simData::CategoryDataPair> nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key1");
  rv += SDK_ASSERT(nextCat->value() == "value1");

  rv += SDK_ASSERT(c1.hasNext());
  nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key2");
  rv += SDK_ASSERT(nextCat->value() == "value2");

  rv += SDK_ASSERT(c1.hasPrevious());
  nextCat = c1.previous();
  rv += SDK_ASSERT(nextCat->name() == "key2");
  rv += SDK_ASSERT(nextCat->value() == "value2");

  rv += SDK_ASSERT(c1.hasPrevious());
  nextCat = c1.previous();
  rv += SDK_ASSERT(!c1.hasPrevious());
  rv += SDK_ASSERT(nextCat->name() == "key1");
  rv += SDK_ASSERT(nextCat->value() == "value1");

  return rv;
}

int testTime2(const simData::CategoryDataSlice &cdslice)
{
  int rv = 0;

  simData::CategoryDataSlice::Iterator c1 = cdslice.current();
  rv += SDK_ASSERT(!c1.hasPrevious());
  rv += SDK_ASSERT(c1.hasNext());
  std::shared_ptr<simData::CategoryDataPair> nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key1");
  rv += SDK_ASSERT(nextCat->value() == "value3"); // value changed

  rv += SDK_ASSERT(c1.hasNext());
  nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key2");
  rv += SDK_ASSERT(nextCat->value() == "value2"); // value same

  rv += SDK_ASSERT(c1.hasNext());
  nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key3"); // new key
  rv += SDK_ASSERT(nextCat->value() == "value1");

  return rv;
}

int testTime3(const simData::CategoryDataSlice &cdslice)
{
  int rv = 0;

  simData::CategoryDataSlice::Iterator c1 = cdslice.current();
  rv += SDK_ASSERT(!c1.hasPrevious());
  rv += SDK_ASSERT(c1.hasNext());
  std::shared_ptr<simData::CategoryDataPair> nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key1");
  rv += SDK_ASSERT(nextCat->value() == "value4"); // value changed

  rv += SDK_ASSERT(c1.hasNext());
  nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key2");
  rv += SDK_ASSERT(nextCat->value() == "value2"); // value same

  rv += SDK_ASSERT(c1.hasNext());
  nextCat = c1.next();
  rv += SDK_ASSERT(nextCat->name() == "key3"); // value same
  rv += SDK_ASSERT(nextCat->value() == "value1");

  return rv;
}

int testIterator(simData::MemoryDataStore &ds)
{
  const simData::CategoryDataSlice *cdslice = ds.categoryDataSlice(PLATFORM_ID);

  int rv = 0;

  // time 0, no data
  ds.update(0);
  rv += SDK_ASSERT(!cdslice->current().hasNext());

  // time 0.5, still no data (try to advance, but get nothing)
  ds.update(0.5);
  rv += SDK_ASSERT(!cdslice->current().hasNext());

  // time 1, things start to happen
  ds.update(1.0);
  rv += testTime1(*cdslice);

  // time 1.5, should be same as 1
  ds.update(1.5);
  rv += testTime1(*cdslice);

  // time 2, some new data
  ds.update(2);
  rv += testTime2(*cdslice);

  // time 2.5, no change
  ds.update(2.5);
  rv += testTime2(*cdslice);

  // time 3, some change
  ds.update(3);
  rv += testTime3(*cdslice);

  // time 100 (past all data), should be same
  ds.update(100);
  rv += testTime3(*cdslice);

  return rv;
}

int testFlush(simData::MemoryDataStore &ds)
{
  const simData::CategoryDataSlice* cdslice = ds.categoryDataSlice(PLATFORM_ID);
  const simData::MemoryCategoryDataSlice* cd = dynamic_cast<const simData::MemoryCategoryDataSlice*>(cdslice);
  int rv = 0;

  // six items are added in loadCategoryData; overwriting an existing value does increment the count.
  rv += (SDK_ASSERT(cd->numItems() == 6));

  // flush should retain current category data, should be same as final state of testIterator test
  ds.flush(PLATFORM_ID, simData::DataStore::RECURSIVE);
  rv += testTime3(*cdslice);

  rv += (SDK_ASSERT(cd->numItems() == 3));

  return rv;
}

/// Helper struct for counting callbacks
struct Counters
{
  Counters() : addCategory(0), addValue(0), clear(0)
  {
  }
  int addCategory;
  int addValue;
  int clear;
};

/// Class for counting callbacks
class CategoryFilterCounter : public simData::CategoryNameManager::Listener
{
public:
  explicit CategoryFilterCounter(Counters *counters) :  counters_(counters)
  {
  }

  virtual ~CategoryFilterCounter()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex)
  {
    counters_->addCategory++;
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex)
  {
    counters_->addValue++;
  }

  /// Invoked when a new value is added to a category
  virtual void onClear()
  {
    counters_->clear++;
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing()
  {
    // noop
  }

private:
    Counters *counters_;
};


int testCatMan(simData::MemoryDataStore &ds)
{
  Counters counters;

  simData::CategoryNameManager &catMan = ds.categoryNameManager();
  simData::CategoryNameManager::ListenerPtr listenerPtr = simData::CategoryNameManager::ListenerPtr(new CategoryFilterCounter(&counters));
  catMan.addListener(listenerPtr);

  int rv = 0;

  // string to int to string
  rv += SDK_ASSERT(catMan.nameIntToString(catMan.nameToInt("key1")) == "key1");

  // make sure all the categories are represented
  std::vector<int> nameIntVec;
  catMan.allCategoryNameInts(nameIntVec);
  rv += SDK_ASSERT(nameIntVec.size() == 3);
  std::vector<std::string> nameVec;
  catMan.allCategoryNames(nameVec);
  rv += SDK_ASSERT(nameVec.size() == 3);

  // removed one
  catMan.removeCategory(catMan.nameToInt("key3"));
  nameIntVec.clear();
  catMan.allCategoryNameInts(nameIntVec);
  rv += SDK_ASSERT(nameIntVec.size() == 2);
  nameVec.clear();
  catMan.allCategoryNames(nameVec);
  rv += SDK_ASSERT(nameVec.size() == 2);

  // add another
  catMan.addCategoryName("test");
  nameIntVec.clear();
  catMan.allCategoryNameInts(nameIntVec);
  rv += SDK_ASSERT(nameIntVec.size() == 3);

  // add a value
  const int key1Int = catMan.nameToInt("key1");
  catMan.addCategoryValue(key1Int, "testValue");

  // make sure the category values come back
  std::vector<std::string> categoryValueVec;
  catMan.allValuesInCategory(key1Int, categoryValueVec);
  rv += SDK_ASSERT(categoryValueVec.size() == 5);
  rv += SDK_ASSERT(categoryValueVec[0] == "value1a");
  rv += SDK_ASSERT(categoryValueVec[1] == "value1");
  rv += SDK_ASSERT(categoryValueVec[2] == "value3");
  rv += SDK_ASSERT(categoryValueVec[3] == "value4");
  rv += SDK_ASSERT(categoryValueVec[4] == "testValue");

  // remove a value
  catMan.removeValue(key1Int, catMan.valueToInt("testValue"));
  categoryValueVec.clear();
  catMan.allValuesInCategory(key1Int, categoryValueVec);
  rv += SDK_ASSERT(categoryValueVec.size() == 4);

  // check for not present category and value
  rv += SDK_ASSERT(catMan.nameToInt("Not Present") == simData::CategoryNameManager::NO_CATEGORY_NAME);
  rv += SDK_ASSERT(catMan.valueToInt("Not Present") == simData::CategoryNameManager::NO_CATEGORY_VALUE);

  // Check callback counters
  rv += SDK_ASSERT(counters.addCategory == 1);
  rv += SDK_ASSERT(counters.addValue == 1);
  rv += SDK_ASSERT(counters.clear == 0);

  catMan.addCategoryValue(key1Int, "DoOnce");

  // It is new so value counter should increase by one
  rv += SDK_ASSERT(counters.addCategory == 1);
  rv += SDK_ASSERT(counters.addValue == 2);
  rv += SDK_ASSERT(counters.clear == 0);

  catMan.addCategoryValue(key1Int, "DoOnce");

  // Since it is a repeat value the value counter should stay the same
  rv += SDK_ASSERT(counters.addCategory == 1);
  rv += SDK_ASSERT(counters.addValue == 2);
  rv += SDK_ASSERT(counters.clear == 0);

  catMan.addCategoryName("DoOnce");

  // It is new so category counter should increase by one
  rv += SDK_ASSERT(counters.addCategory == 2);
  rv += SDK_ASSERT(counters.addValue == 2);
  rv += SDK_ASSERT(counters.clear == 0);

  catMan.addCategoryValue(key1Int, "DoOnce");

  // Since it is a repeat value the category counter should stay the same
  rv += SDK_ASSERT(counters.addCategory == 2);
  rv += SDK_ASSERT(counters.addValue == 2);
  rv += SDK_ASSERT(counters.clear == 0);

  catMan.clear();
  // Only clear should increase
  rv += SDK_ASSERT(counters.addCategory == 2);
  rv += SDK_ASSERT(counters.addValue == 2);
  rv += SDK_ASSERT(counters.clear == 1);


  catMan.removeListener(listenerPtr);
  return rv;
}

// Makes sure that deleting an entity cleans up after its category data
int testDeleteEntity(simData::DataStore& ds)
{
  int rv = 0;
  simUtil::DataStoreTestHelper helper(&ds);
  uint64_t plat100 = helper.addPlatform();
  helper.addCategoryData(plat100, "Plat100", "100", -1.0);
  rv += SDK_ASSERT(ds.categoryDataSlice(plat100) != NULL);
  rv += SDK_ASSERT(ds.categoryDataSlice(plat100+1) == NULL); // random sanity check
  // Validate that removing the entity removes its category data slice too
  ds.removeEntity(plat100);
  rv += SDK_ASSERT(ds.categoryDataSlice(plat100) == NULL);

  // Same test, recursive on a LOB
  uint64_t plat101 = helper.addPlatform();
  uint64_t lob102 = helper.addLOB(plat101);
  helper.addCategoryData(lob102, "LOB102", "102", -1.0);
  rv += SDK_ASSERT(ds.categoryDataSlice(lob102) != NULL); // random sanity check
  // Remove it and double check
  ds.removeEntity(plat101);
  rv += SDK_ASSERT(ds.categoryDataSlice(plat101) == NULL);
  rv += SDK_ASSERT(ds.categoryDataSlice(lob102) == NULL);
  return rv;
}

int testFilterSerialize()
{
  // map of input strings to expected optimized output strings
  std::map<std::string, std::string> inputToOptimizedOutput;

  // All values on simplifies to empty string
  inputToOptimizedOutput["Platform Type(1)~Unlisted Value(1)~No Value(1)~Unknown(1)~Surface Ship(1)~Submarine(1)~Aircraft(1)~Satellite(1)~Helicopter(1)~Missile(1)~Decoy(1)~Buoy(1)~Reference Site(1)~Land Vehicle(1)~Land Site(1)~Torpedo(1)~Contact(1)"] = " ";
  // All values on simplifies to empty
  inputToOptimizedOutput["a(1)~Unlisted Value(1)~No Value(1)~Something(1)"] = " ";
  // All values at default values simplifies to empty
  inputToOptimizedOutput["a(1)~Unlisted Value(0)"] = " ";
  // Hand-edit case: 0 in category, non-zero values. Note that this should simplify to empty, since unchecked categories are skipped when deserializing, as they will be ignored in match()
  // See SIM-5259 for more information
  inputToOptimizedOutput["a(0)~SomeValue(1)~SomeOtherValue(1)~UnsetValue(0)"] = " ";
  inputToOptimizedOutput["a(0)~SomeValue(1)~SomeOtherValue(1)"] = " ";
  // Identity case: input matches output
  inputToOptimizedOutput["a(1)~Unlisted Value(1)"] = "a(1)~Unlisted Value(1)";
  inputToOptimizedOutput["a(1)~Something(1)"] = "a(1)~Something(1)";
  // Unlisted value is on, but there's a state without unlisted value
  inputToOptimizedOutput["a(1)~Unlisted Value(1)~Unknown(0)~Surface Ship(1)"] = "a(1)~Unlisted Value(1)~Unknown(0)";
  // Unlisted value is on, but there's a state without unlisted value (with lots of cropping)
  inputToOptimizedOutput["a(1)~Unlisted Value(1)~No Value(1)~Unknown(1)~Surface Ship(0)~Submarine(1)~Aircraft(1)~Satellite(1)~Helicopter(1)~Missile(1)~Decoy(1)~Buoy(1)~Reference Site(1)~Land Vehicle(1)~Land Site(1)~Torpedo(1)~Contact(1)"] = "a(1)~Unlisted Value(1)~No Value(1)~Surface Ship(0)";
  // All values simplifies to empty string, with 2 categories
  inputToOptimizedOutput["Platform Type(1)~Unlisted Value(1)~No Value(1)~Unknown(1)~Surface Ship(1)~Submarine(1)~Aircraft(1)~Satellite(1)~Helicopter(1)~Missile(1)~Decoy(1)~Buoy(1)~Reference Site(1)~Land Vehicle(1)~Land Site(1)~Torpedo(1)~Contact(1)`a(1)~Unlisted Value(1)"] = "a(1)~Unlisted Value(1)";
  inputToOptimizedOutput["Platform Type(1)~Unlisted Value(1)~No Value(1)~Unknown(1)~Surface Ship(1)~Submarine(1)~Aircraft(1)~Satellite(1)~Helicopter(1)~Missile(1)~Decoy(1)~Buoy(1)~Reference Site(1)~Land Vehicle(1)~Land Site(1)~Torpedo(1)~Contact(1)`a(1)~Unlisted Value(0)"] = " ";
  // One of the two categories isn't fully empty
  inputToOptimizedOutput["Platform Type(1)~Unlisted Value(1)~No Value(1)~Unknown(1)~Surface Ship(1)~Submarine(1)~Aircraft(1)~Satellite(1)~Helicopter(1)~Missile(1)~Decoy(1)~Buoy(1)~Reference Site(1)~Land Vehicle(1)~Land Site(1)~Torpedo(1)~Contact(1)`a(1)~Something(1)"] = "a(1)~Something(1)";

  // Empty string identity inputs
  std::map<std::string, std::string> emptyStrings;
  emptyStrings[""] = " ";
  emptyStrings[" "] = " ";
  emptyStrings["  "] = " ";

  simData::MemoryDataStore ds;
  simData::CategoryFilter filter(&ds);
  int rv = 0;

  // test optimized serialization and deserialization, which is used for category filters in pref rules
  for (auto iter = inputToOptimizedOutput.begin(); iter != inputToOptimizedOutput.end(); ++iter)
  {
    rv += SDK_ASSERT(filter.deserialize(iter->first));
    rv += SDK_ASSERT(filter.serialize() == iter->second);
  }
  for (auto iter = emptyStrings.begin(); iter != emptyStrings.end(); ++iter)
  {
    rv += SDK_ASSERT(filter.deserialize(iter->first));
    rv += SDK_ASSERT(filter.serialize() == iter->second);
  }

  // test out deserializing with the skip flag set to false, which should preserve the full state when serializing out again, so output should match input
  for (auto iter = inputToOptimizedOutput.begin(); iter != inputToOptimizedOutput.end(); ++iter)
  {
    rv += SDK_ASSERT(filter.deserialize(iter->first, false));
    rv += SDK_ASSERT(filter.serialize(false) == iter->first);
  }
  // empty strings still all convert to the standard empty serialization
  for (auto iter = emptyStrings.begin(); iter != emptyStrings.end(); ++iter)
  {
    rv += SDK_ASSERT(filter.deserialize(iter->first, false));
    rv += SDK_ASSERT(filter.serialize(false) == iter->second);
  }

  return rv;
}

int testCategoryFilterRules()
{
  simData::MemoryDataStore ds;
  loadCategoryData(ds);
  int rv = 0;

  // Rule 1 does not need testing; it describes the separators

  // Test rule 2: Categories not listed will not impact filter results
  {
    ds.update(2.0);

    // key1=value3
    // key2=value2
    // key3=value1

    // PLATFORM_ID will match both key1.value3 and key3.value1 at this time
    simData::CategoryFilter filter(&ds);
    rv += SDK_ASSERT(filter.deserialize("key1(1)~value3(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key1(1)~value3(1)");

    rv += SDK_ASSERT(filter.deserialize("key3(1)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key3(1)~value1(1)");

    // Flipping the bit on category value will break the match
    rv += SDK_ASSERT(filter.deserialize("key1(1)~value3(0)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));

    // This is a rule that will match nothing
    rv += SDK_ASSERT(filter.deserialize("key3(1)~value1(0)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));

    // We've shown that key1 and key3 both independently match, now show they match together.
    rv += SDK_ASSERT(filter.deserialize("key1(1)~Unlisted Value(0)~value3(1)`key3(1)~Unlisted Value(0)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key1(1)~value3(1)`key3(1)~value1(1)");
  }

  // Test rule 3: empty string matches all entities
  {
    simData::CategoryFilter filter(&ds);
    rv += SDK_ASSERT(filter.deserialize(" "));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == " ");

    rv += SDK_ASSERT(filter.deserialize(""));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == " ");
  }

  // Test rule 4: Unchecked categories don't matter
  {
    simData::CategoryFilter filter(&ds);

    // Precondition: key1.value3 is set
    rv += SDK_ASSERT(filter.deserialize("key1(1)~Unlisted Value(0)~value3(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    // Precondition: key2.value2 is set
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(0)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    // Precondition: key3.value1 is set
    rv += SDK_ASSERT(filter.deserialize("key3(1)~Unlisted Value(0)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));

    // Test first example
    rv += SDK_ASSERT(filter.deserialize("key3(0)~value1(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == " ");

    // Test second example
    rv += SDK_ASSERT(filter.deserialize("key3(0)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == " ");

    // Test third example
    rv += SDK_ASSERT(filter.deserialize("key3(0)~Unlisted Value(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == " ");

    // Test fourth example
    rv += SDK_ASSERT(filter.deserialize("key3(0)~value1(0)`key2(1)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value2(1)");

    rv += SDK_ASSERT(filter.deserialize("key3(0)~value1(0)`key2(1)~value2(0)~value3(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value3(1)");

    rv += SDK_ASSERT(filter.deserialize("key3(0)~value1(0)`key2(1)~value3(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value3(1)");
  }

  // Test rule 5: unlisted values are unchecked by default
  {
    simData::CategoryFilter filter(&ds);

    // Test first example: unspecified values are unchecked by default
    rv += SDK_ASSERT(filter.deserialize("key2(1)~value3(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value3(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~value3(1)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value2(1)~value3(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value2(1)");

    // Test second and third example: Unlisted Value(0) does not add value
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(0)~value3(0)~value4(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value4(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(0)~value2(0)~value4(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value4(1)");

    // Test fourth example: Unlisted Value(1) with explicit off value
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value3(0)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value2(0)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value2(0)");

    // Test fifth example: Unlisted Value(1) with explicit on value
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value3(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)");

    // Test sixth example: Combining Unlisted Value(1) with an on and an off
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value2(0)~value3(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value2(0)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value3(0)");
  }

  // Test rule 6: No Value
  {
    simData::CategoryFilter filter(&ds);

    // Test first example: No Value(0) does not match when there's no value for the category
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~No Value(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // key2 has a value, so we do match
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)");

    rv += SDK_ASSERT(filter.deserialize("key4(1)~Unlisted Value(1)~No Value(0)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));  // key4 has no value, so we do not match
    rv += SDK_ASSERT(filter.serialize(true) == "key4(1)~Unlisted Value(1)");

    // Test simplification with first example
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~No Value(0)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // match due to explicit (1) on value2
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value3(0)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~No Value(1)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // match due to explicit (1) on value2
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~No Value(1)~value3(0)");

    // Test equivalency of second example: No Value(0) does not need to be explicitly mentioned
    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // key2 has a value, so we do match
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)");

    rv += SDK_ASSERT(filter.deserialize("key4(1)~Unlisted Value(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));  // key4 has no value, so we do not match
    rv += SDK_ASSERT(filter.serialize(true) == "key4(1)~Unlisted Value(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // match due to explicit (1) on value2
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~value3(0)");

    // Test third example: No Value(1) only matches when there's no value for the category
    rv += SDK_ASSERT(filter.deserialize("key2(1)~No Value(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));  // key2 has a value, so we don't match
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~No Value(1)");

    rv += SDK_ASSERT(filter.deserialize("key4(1)~No Value(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // key4 has no value, so we do match
    rv += SDK_ASSERT(filter.serialize(true) == "key4(1)~No Value(1)");

    // Test simplification with third example
    rv += SDK_ASSERT(filter.deserialize("key2(1)~No Value(1)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // match due to explicit (1) on value2
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~No Value(1)~value2(1)");

    rv += SDK_ASSERT(filter.deserialize("key2(1)~Unlisted Value(1)~No Value(1)~value2(1)~value3(0)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));  // match due to explicit (1) on value2
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~Unlisted Value(1)~No Value(1)~value3(0)");

    rv += SDK_ASSERT(filter.deserialize("key4(1)~Unlisted Value(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));  // No match due to implicit No Value(0)
    rv += SDK_ASSERT(filter.serialize(true) == "key4(1)~Unlisted Value(1)");
  }

  // Test rule 7: AND logic for categories
  {
    simData::CategoryFilter filter(&ds);

    // Precondition tests on key2=value2 and key3=value1
    rv += SDK_ASSERT(filter.deserialize("key2(1)~value2(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.deserialize("key3(1)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));

    // Simple match
    rv += SDK_ASSERT(filter.deserialize("key2(1)~value2(1)`key3(1)~value1(1)"));
    rv += SDK_ASSERT(filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value2(1)`key3(1)~value1(1)");

    // Break right side
    rv += SDK_ASSERT(filter.deserialize("key2(1)~value2(1)`key3(1)~value1(0)~value2(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value2(1)`key3(1)~value2(1)");

    // Break left side
    rv += SDK_ASSERT(filter.deserialize("key2(1)~value2(0)~value1(1)`key3(1)~value1(1)"));
    rv += SDK_ASSERT(!filter.match(PLATFORM_ID));
    rv += SDK_ASSERT(filter.serialize(true) == "key2(1)~value1(1)`key3(1)~value1(1)");
  }

  return rv;
}

class CDListener : public simData::DataStore::DefaultListener
{
public:
  explicit CDListener(unsigned int& cdChangeCounter) : cdChangeCounter_(cdChangeCounter)
  {}

  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    ++cdChangeCounter_;
  }
private:
  unsigned int& cdChangeCounter_;
};

int testIsDuplicateValue()
{
  int rv = 0;

  simUtil::DataStoreTestHelper dsHelper;
  uint64_t id = dsHelper.addPlatform();
  simData::DataStore& ds = *dsHelper.dataStore();

  ds.setDataLimiting(false); // Needs to be false to avoid triggering duplicate values
  const simData::MemoryCategoryDataSlice* cd = dynamic_cast<const simData::MemoryCategoryDataSlice*>(ds.categoryDataSlice(id));
  rv += SDK_ASSERT(cd != NULL);
  rv += SDK_ASSERT(cd->numItems() == 0);
  // No items, no duplicates
  rv += SDK_ASSERT(!cd->isDuplicateValue(10, "key", "value"));

  // Add an unrelated key, still no duplicates
  dsHelper.addCategoryData(id, "key2", "value", 10);
  rv += SDK_ASSERT(!cd->isDuplicateValue(9, "key2", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(10, "key2", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key2", "value"));

  // Add key+value, detect duplicates
  dsHelper.addCategoryData(id, "key", "value", 10);
  rv += SDK_ASSERT(!cd->isDuplicateValue(9, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(10, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key", "value"));

  // Add another key+value (same one) at time 20.  10=value, 20=value
  dsHelper.addCategoryData(id, "key", "value", 20);
  rv += SDK_ASSERT(!cd->isDuplicateValue(9, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(10, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(20, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(21, "key", "value"));

  // Add another key with a new value BEFORE time 10.  5=v5, 10=value, 20=value
  dsHelper.addCategoryData(id, "key", "v5", 5);
  rv += SDK_ASSERT(!cd->isDuplicateValue(4, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(5, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(9, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(10, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(20, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(21, "key", "value"));
  // Should still dupe on v5 though
  rv += SDK_ASSERT(!cd->isDuplicateValue(4, "key", "v5"));
  rv += SDK_ASSERT(cd->isDuplicateValue(5, "key", "v5"));
  rv += SDK_ASSERT(cd->isDuplicateValue(9, "key", "v5"));

  // test that datastore reports CategoryDataChange when it is supposed to
  {
    unsigned int cdChangeCounter = 0;
    simData::DataStore::ListenerPtr cdListener = simData::DataStore::ListenerPtr(new CDListener(cdChangeCounter));
    ds.addListener(cdListener);
    // test that update to time 5 flags a change in CD - new value
    cdChangeCounter = 0;
    ds.update(5.0);
    rv += SDK_ASSERT(1 == cdChangeCounter);
    // test that update to time 10 flags a change in CD - change in value
    cdChangeCounter = 0;
    ds.update(10.0);
    rv += SDK_ASSERT(1 == cdChangeCounter);

    // test that update to time 20 does not flag a change in CD - duplicate
    cdChangeCounter = 0;
    ds.update(20.0);
    rv += SDK_ASSERT(0 == cdChangeCounter);

    ds.removeListener(cdListener);
  }

  // Add another at the end.  5=v5, 10=value, 20=value, 25=v25
  dsHelper.addCategoryData(id, "key", "v25", 25);
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(20, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(21, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(25, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(26, "key", "value"));
  // Should still dupe on v5 though
  rv += SDK_ASSERT(!cd->isDuplicateValue(21, "key", "v25"));
  rv += SDK_ASSERT(cd->isDuplicateValue(25, "key", "v25"));
  rv += SDK_ASSERT(cd->isDuplicateValue(26, "key", "v25"));

  // Now add in the middle.  5=v5, 10=value, 15=v15, 20=value, 25=v25
  dsHelper.addCategoryData(id, "key", "v15", 15);
  rv += SDK_ASSERT(!cd->isDuplicateValue(9, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(10, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(11, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(15, "key", "value"));
  rv += SDK_ASSERT(!cd->isDuplicateValue(16, "key", "value"));
  rv += SDK_ASSERT(cd->isDuplicateValue(20, "key", "value"));
  // And test the v15 duplicates
  rv += SDK_ASSERT(!cd->isDuplicateValue(11, "key", "v15"));
  rv += SDK_ASSERT(cd->isDuplicateValue(15, "key", "v15"));
  rv += SDK_ASSERT(cd->isDuplicateValue(16, "key", "v15"));

  // 2nd test that datastore reports CategoryDataChange when it is supposed to
  {
    // test that change from something to nothing is reported as a CategoryDataChange
    dsHelper.addCategoryData(id, "key", "", 21);

    unsigned int cdChangeCounter = 0;
    simData::DataStore::ListenerPtr cdListener = simData::DataStore::ListenerPtr(new CDListener(cdChangeCounter));
    ds.addListener(cdListener);

    cdChangeCounter = 0;
    ds.update(21.0);
    rv += SDK_ASSERT(id == cdChangeCounter);

    ds.removeListener(cdListener);
  }

  return rv;
}

int testDeserializeFailures()
{
  simData::MemoryDataStore ds;
  loadCategoryData(ds);
  simData::CategoryFilter filter(&ds);

  int rv = 0;

  // Test successful strings
  rv += SDK_ASSERT(filter.deserialize("TestCategory(1)~TestValue(1)"));
  rv += SDK_ASSERT(filter.deserialize("TestCategory(1)~TestValue(1)`T2(1)~TV1(1)~TV2(1)"));
  rv += SDK_ASSERT(filter.deserialize("TestCategory(1)~TestValue(1)`T2(1)~TV1(1)~TV2(1)~T3(1)~TV1(1)"));

  // Start to break strings and test for failures

  // Bad value parens
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue()"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue)"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue1)"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue(1"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue[1]"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue"));
  // Bad value #
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue(2)"));

  // Short value names with invalid parens
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~Test"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~Tes"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~Te"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~T"));

  // Missing values
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~~"));

  // Bad category parens
  rv += SDK_ASSERT(!filter.deserialize("TestCategory()~TestValue(1)"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1~TestValue(1)"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory1)~TestValue(1)"));
  rv += SDK_ASSERT(!filter.deserialize("TestCategory~TestValue(1)"));
  // Bad category #
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(2)~TestValue(1)"));

  // Bad leading characters
  rv += SDK_ASSERT(!filter.deserialize("~TestValue(1)"));
  rv += SDK_ASSERT(!filter.deserialize("`TestValue(1)"));
  rv += SDK_ASSERT(!filter.deserialize("`TestCategory(1)~TestValue(1)"));

  // Illegal ~
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~~TestValue(1)"));

  // Second category name has no values
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue(1)`T2"));

  // Double backtick, missing a category
  rv += SDK_ASSERT(!filter.deserialize("TestCategory(1)~TestValue(1)``T2(1)~TV1(1)"));

  return rv;
}

}

int CategoryDataTest(int argc, char *argv[])
{
  simData::MemoryDataStore ds;
  loadCategoryData(ds);

  // start test
  int rv = 0;
  rv += testIterator(ds);
  rv += testFlush(ds);
  rv += testCatMan(ds);
  rv += testDeleteEntity(ds);

  rv += testFilterSerialize();
  rv += testIsDuplicateValue();
  rv += testCategoryFilterRules();
  rv += testDeserializeFailures();

  return rv;
}
