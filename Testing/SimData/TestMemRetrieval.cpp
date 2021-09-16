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
#include <iostream>
#include <float.h>
#include <limits>
#include "simCore/Time/Utils.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simData/LinearInterpolator.h"
#include "simUtil/DataStoreTestHelper.h"

using namespace std;

namespace
{

const size_t NUM_PLATS = 100;
const size_t NUM_POINTS = 100;
const size_t NUM_COMMANDS = 6;
const size_t NUM_GD_POINTS = 1+2+3+4+5+6; // 21 -- SUM(1..6)
const std::string ICON_NAME = "unit_sphere";
const double DELTA = 0.000001;


std::string expectedValue(const std::string& text, uint64_t id)
{
  std::stringstream ss;
  ss << text << id;
  return ss.str();
}

bool isValidCategory(const simData::CategoryData* catData, size_t index)
{
  if (!catData->has_time())
    return false;

  if (catData->time() != static_cast<double>(index))
    return false;

  if (catData->entry().size() != static_cast<int>(index+1))
    return false;

  for (size_t ii = 0; ii < (index+1); ii++)
  {
    if (catData->entry().Get(static_cast<int>(ii)).key() != expectedValue("Some Key ", ii))
      return false;

    if (catData->entry().Get(static_cast<int>(ii)).value() != expectedValue("Some Value ", ii))
      return false;
  }

  return true;
}

void addPlatformCommand(simData::DataStore* dataStore, uint64_t id, size_t index)
{
  simData::DataStore::Transaction transaction;
  simData::PlatformCommand* cmd = dataStore->addPlatformCommand(id, &transaction);
  cmd->mutable_updateprefs()->mutable_commonprefs()->set_datadraw((index % 2) == 1);
  cmd->set_time(static_cast<double>(index));
  transaction.complete(&cmd);
}

void addPlatformGenericData(simData::DataStore* dataStore, uint64_t id, size_t index)
{
  simData::DataStore::Transaction transaction;
  simData::GenericData* genData = dataStore->addGenericData(id, &transaction);
  SDK_ASSERT(genData != nullptr);
  if (genData == nullptr)
    return;

  genData->set_time(static_cast<double>(index));
  genData->set_duration(-1.0);
  for (size_t ii = 0; ii < (index+1); ii++)
  {
    simData::GenericData_Entry* entry = genData->mutable_entry()->Add();
    entry->set_key(expectedValue("Some Tag ", ii));
    entry->set_value(expectedValue("Some Data ", ii));
  }

  transaction.complete(&genData);
}

void addPlatformCategoryData(simData::DataStore* dataStore, uint64_t id, size_t index)
{
  simData::DataStore::Transaction transaction;
  simData::CategoryData* catData = dataStore->addCategoryData(id, &transaction);
  SDK_ASSERT(catData != nullptr);
  if (catData == nullptr)
    return;

  catData->set_time(static_cast<double>(index));
  for (size_t ii = 0; ii < (index+1); ii++)
  {
    simData::CategoryData_Entry* entry = catData->add_entry();
    entry->set_key(expectedValue("Some Key ", ii));
    entry->set_value(expectedValue("Some Value ", ii));
  }
  transaction.complete(&catData);
}

void addPlatform(simUtil::DataStoreTestHelper& testHelper, bool inOrder)
{
  uint64_t id = testHelper.addPlatform();
  simData::DataStore* dataStore = testHelper.dataStore();
  simData::DataStore::Transaction t;
  simData::PlatformPrefs *pp = dataStore->mutable_platformPrefs(id, &t);
  pp->set_icon(ICON_NAME);
  t.commit();
  // Add a few data points
  if (inOrder)
  {
    for (size_t k = 0; k < NUM_POINTS; ++k)
      testHelper.addPlatformUpdate(static_cast<double>(k), id);

    for (size_t k = 0; k < NUM_COMMANDS; ++k)
    {
      addPlatformCommand(dataStore, id, k);
      addPlatformGenericData(dataStore, id, k);
      addPlatformCategoryData(dataStore, id, k);
    }
  }
  else
  {
    for (size_t k = NUM_POINTS; k > 0; --k)
      testHelper.addPlatformUpdate(static_cast<double>(k-1), id);

    for (size_t k = NUM_COMMANDS; k > 0; --k)
    {
      addPlatformCommand(dataStore, id, k-1);
      addPlatformGenericData(dataStore, id, k-1);
      addPlatformCategoryData(dataStore, id, k-1);
    }
  }


}

/** Helper class to visit and check data points in an update slice */
class TestVisit : public simData::PlatformUpdateSlice::Visitor
{
public:
  int numVisits;
  int numErrors;
  TestVisit() : numVisits(0), numErrors(0) {}
  virtual void operator()(const simData::PlatformUpdate *update)
  {
    // The following line will trip if the time is out-of-order, or in case of duplicates
    numErrors += SDK_ASSERT(update->time() == numVisits);
    // Test the positions to verify data persists over time
    numErrors += SDK_ASSERT(update->x() == 0.0 + numVisits);
    numErrors += SDK_ASSERT(update->y() == 1.0 + numVisits);
    numErrors += SDK_ASSERT(update->z() == 2.0 + numVisits);
    numVisits++;
  }
};

// Validation that data was correctly added
int sanityCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  rv += SDK_ASSERT(dataStore != nullptr);
  if (dataStore == nullptr) return rv;

  simData::DataStore::IdList ids;
  dataStore->idList(&ids);
  rv += SDK_ASSERT(ids.size() == 100);
  // Iterate through all IDs
  simData::DataStore::Transaction transaction;
  for (simData::DataStore::IdList::const_iterator idIter = ids.begin(); idIter != ids.end(); ++idIter)
  {
    rv += SDK_ASSERT(*idIter > 0 && *idIter <= NUM_PLATS);
    const simData::PlatformPrefs* prefs = dataStore->platformPrefs(*idIter, &transaction);
    rv += SDK_ASSERT(prefs != nullptr);
    if (prefs == nullptr) continue;
    // Check two prefs that were set
    rv += SDK_ASSERT(prefs->commonprefs().has_name());
    rv += SDK_ASSERT(prefs->has_icon());
    rv += SDK_ASSERT(prefs->commonprefs().name() == expectedValue("platform", *idIter));
    rv += SDK_ASSERT(prefs->icon() == ICON_NAME);
  }
  // Validate the data for various IDs
  assert(NUM_POINTS % 2 == 0); // Note: Need even number of points for time update checks
  double timeValue = NUM_POINTS / 2;
  dataStore->update(timeValue);
  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr) return rv;
  // Get the current data (for timeValue)
  const simData::PlatformUpdate* update = slice->current();
  rv += SDK_ASSERT(update != nullptr);
  if (update == nullptr) return rv;
  rv += SDK_ASSERT(update->time() == timeValue);

  // Verify the time bounds; note that this might change depending on data store implementation
  simData::PlatformUpdateSlice::Bounds bounds = slice->interpolationBounds();
  // TODO: Are bounds only set when update() is interpolated?
  // rv += SDK_ASSERT(bounds.first != nullptr);
  // rv += SDK_ASSERT(bounds.second != nullptr);
  if (bounds.first != nullptr && bounds.second != nullptr)
  {
    rv += SDK_ASSERT(bounds.first->time() == 0);
    rv += SDK_ASSERT(bounds.second->time() == NUM_POINTS - 1);
  }
  // Iterate through points, validating values; note this might change depending on implementation
  TestVisit testVisit;
  slice->visit(&testVisit);
  rv += SDK_ASSERT(testVisit.numVisits == 100);
  rv += SDK_ASSERT(testVisit.numErrors == 0);
  return rv;
}

// Helper function to test upper/lower bound returns; <0 is a special flag for end()
template <class T>
int upperLowerTest(const simData::DataSlice<T>* slice, double searchTime, double expectedLower, double expectedUpper)
{
  typename simData::DataSlice<T>::Iterator ui = slice->upper_bound(searchTime);
  typename simData::DataSlice<T>::Iterator li = slice->lower_bound(searchTime);
  // Error if we expect end-of-list, and don't get it
  if (expectedLower < 0 && li.hasNext())
    return 1;
  if (expectedUpper < 0 && ui.hasNext())
    return 1;
  // If at end of list, return whether we expected it
  if (!li.hasNext())
    return (expectedLower < 0) ? 0 : 1;
  if (!ui.hasNext())
    return (expectedUpper < 0) ? 0 : 1;
  // If we're here, check the time values
  if (li.peekNext()->time() != expectedLower)
    return 1;
  if (ui.peekNext()->time() != expectedUpper)
    return 1;
  return 0;
}

// Tests the Update DataSlice::Iterator class
int updateIterateTest(simData::DataStore* dataStore)
{
  int rv = 0;
  // First, update the data store to get a valid data slice
  dataStore->update(2.0);
  // Get a pointer to the data slice
  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(1);
  simData::PlatformUpdateSlice::Iterator iter(slice);
  // Validate first entry with peek
  rv += SDK_ASSERT(iter.peekNext() != nullptr);
  rv += SDK_ASSERT(iter.peekNext()->time() == 0.0);
  // Validate copy constructor
  simData::PlatformUpdateSlice::Iterator copy(iter);
  rv += SDK_ASSERT(iter.peekNext() == copy.next());
  // Validate toFront
  rv += SDK_ASSERT(iter.peekNext() != copy.peekNext());
  copy.toFront();
  rv += SDK_ASSERT(iter.peekNext() == copy.peekNext());
  rv += SDK_ASSERT(!copy.hasPrevious());
  // Validate the last item with a peek
  copy.toBack();
  rv += SDK_ASSERT(copy.hasPrevious());
  rv += SDK_ASSERT(!copy.hasNext());
  rv += SDK_ASSERT(copy.peekPrevious() != nullptr);
  rv += SDK_ASSERT(copy.peekPrevious()->time() == NUM_POINTS - 1);
  copy.toFront();
  // Validate copy constructor
  int numSeen = 0;
  while (iter.hasNext())
  {
    // Validate peek functions too
    const simData::PlatformUpdate* peek = iter.peekNext();
    const simData::PlatformUpdate* update = iter.next();
    rv += SDK_ASSERT(peek == update);
    rv += SDK_ASSERT(peek == iter.peekPrevious());
    rv += SDK_ASSERT(update != nullptr);
    // Confirm time
    rv += SDK_ASSERT(update->time() == numSeen);
    numSeen++;
  }
  rv += SDK_ASSERT(static_cast<size_t>(numSeen) == NUM_POINTS);
  // Iterate backwards, using the copy from earlier
  copy.toBack();
  rv += SDK_ASSERT(!copy.hasNext());
  while (iter.hasPrevious())
  {
    // This time just do a simple time comparison
    --numSeen;
    const simData::PlatformUpdate* update = iter.previous();
    rv += SDK_ASSERT(update->time() == numSeen);
  }
  // Should have numSeen at 0 now
  rv += SDK_ASSERT(numSeen == 0);

  // Test upper_bound and lower_bound
  rv += SDK_ASSERT(0 == upperLowerTest(slice, 0.0, 0.0, 1.0));
  rv += SDK_ASSERT(0 == upperLowerTest(slice, 0.5, 1.0, 1.0));
  rv += SDK_ASSERT(0 == upperLowerTest(slice, 1.0, 1.0, 2.0));
  rv += SDK_ASSERT(0 == upperLowerTest(slice, -1.0, 0.0, 0.0));
  rv += SDK_ASSERT(0 == upperLowerTest(slice, NUM_POINTS - 1.0, NUM_POINTS - 1, -1));
  rv += SDK_ASSERT(0 == upperLowerTest(slice, NUM_POINTS + 0.0, -1, -1));

  return rv;
}

// Tests the Command DataSlice::Iterator class
int commandIterateTest(simData::DataStore* dataStore)
{
  int rv = 0;

  const simData::PlatformCommandSlice* slice = dataStore->platformCommandSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr)
    return rv;

  // check size
  rv += SDK_ASSERT(slice->numItems() == NUM_COMMANDS);
  if (rv)
    return rv;

  // go forward
  simData::PlatformCommandSlice::Iterator iter = slice->lower_bound(0);
  size_t index = 0;
  while (iter.hasNext())
  {
    rv += SDK_ASSERT(iter.peekNext()->time() == static_cast<double>(index));
    rv += SDK_ASSERT(iter.peekNext()->updateprefs().commonprefs().datadraw() == ((index % 2) == 1));
    const simData::PlatformCommand* command = iter.next();
    rv += SDK_ASSERT(command->time() == static_cast<double>(index));
    rv += SDK_ASSERT(command->updateprefs().commonprefs().datadraw() == ((index % 2) == 1));
    index++;
  }

  rv += SDK_ASSERT(index == NUM_COMMANDS);

  // go backwards
  iter = slice->lower_bound(static_cast<double>(NUM_COMMANDS));
  index = NUM_COMMANDS;
  while (iter.hasPrevious())
  {
    rv += SDK_ASSERT(iter.peekPrevious()->time() == static_cast<double>(index-1));
    rv += SDK_ASSERT(iter.peekPrevious()->updateprefs().commonprefs().datadraw() == (((index-1) % 2) == 1));
    const simData::PlatformCommand* command = iter.previous();
    rv += SDK_ASSERT(command->time() == static_cast<double>(index-1));
    rv += SDK_ASSERT(command->updateprefs().commonprefs().datadraw() == (((index-1) % 2) == 1));
    index--;
  }

  rv += SDK_ASSERT(index == 0);

  // test lower_bound and upper_bound
  iter = slice->lower_bound(0.0);
  index = 0;
  while (iter.hasNext())
  {
    const simData::PlatformCommand* command = iter.next();
    rv += SDK_ASSERT(slice->upper_bound(command->time() - DELTA).next()->time() == command->time());
    rv += SDK_ASSERT(slice->lower_bound(command->time()).next()->time() == command->time());
    rv += SDK_ASSERT(slice->lower_bound(command->time() - DELTA).next()->time() == command->time());

    if (index != (NUM_COMMANDS-1))  // do not walk off the end
    {
      rv += SDK_ASSERT(slice->upper_bound(command->time()).next()->time() == (command->time()+1.0));
    }

    index++;
  }

  rv += SDK_ASSERT(index == NUM_COMMANDS);

  // test visitor
  class CommandSliceTest : public simData::DataSlice<simData::PlatformCommand>::Visitor
  {
  public:
    explicit CommandSliceTest(int* returnValue)
    {
      rv_ = returnValue;
      index_ = 0;
    }
    virtual void operator()(const simData::PlatformCommand *command)
    {
      *rv_ += SDK_ASSERT(command->time() == static_cast<double>(index_));
      *rv_ += SDK_ASSERT(command->updateprefs().commonprefs().datadraw() == ((index_ % 2) == 1));
      ++index_;
    }

  protected:
    size_t index_;
    int* rv_;
  };

  CommandSliceTest sc(&rv);
  slice->visit(&sc);

  return rv;
}

// Tests the Generic Data DataSlice::Iterator class
int genericIterateTest(simData::DataStore* dataStore)
{
  int rv = 0;

  const simData::GenericDataSlice* slice = dataStore->genericDataSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr)
    return rv;

  // check size
  rv += SDK_ASSERT(slice->numItems() == NUM_GD_POINTS);

  // test visitor
  class GenericSliceTest : public simData::DataSlice<simData::GenericData>::Visitor
  {
  public:
    GenericSliceTest()
      : index_(0),
        numErrors_(0)
    {
    }
    virtual void operator()(const simData::GenericData *command)
    {
      index_ += command->entry_size();
    }
    size_t numVisits() const
    {
      return index_;
    }
    int numErrors() const
    {
      return numErrors_;
    }

  private:
    size_t index_;
    int numErrors_;
  };

  GenericSliceTest sc;
  slice->visit(&sc);
  rv += SDK_ASSERT(sc.numVisits() == NUM_GD_POINTS);
  rv += SDK_ASSERT(sc.numErrors() == 0);

  return rv;
}

// Tests the Category Data DataSlice::Iterator class
int categoryIterateTest(simData::DataStore* dataStore)
{
  int rv = 0;

  const simData::CategoryDataSlice* slice = dataStore->categoryDataSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr)
    return rv;

  // check size
  /* TODO, fix based on new implementation
  rv += SDK_ASSERT(slice->numItems() == NUM_COMMANDS);
  if (rv)
    return rv;

  // go forward
  simData::CategoryDataSlice::Iterator iter = slice->lower_bound(0);
  size_t index = 0;
  while (iter.hasNext())
  {
    rv += SDK_ASSERT(isValidCategory(iter.peekNext(), index));
    const simData::CategoryData* command = iter.next();
    rv += SDK_ASSERT(isValidCategory(command, index));
    index++;
  }

  rv += SDK_ASSERT(index == NUM_COMMANDS);

  // go backwards
  iter = slice->lower_bound(static_cast<double>(NUM_COMMANDS));
  index = NUM_COMMANDS;
  while (iter.hasPrevious())
  {
    rv += SDK_ASSERT(isValidCategory(iter.peekPrevious(), index-1));
    const simData::CategoryData* command = iter.previous();
    rv += SDK_ASSERT(isValidCategory(command, index-1));
    index--;
  }

  rv += SDK_ASSERT(index == 0);

  // test lower_bound and upper_bound
  iter = slice->lower_bound(0.0);
  index = 0;
  while (iter.hasNext())
  {
    const simData::CategoryData* command = iter.next();
    rv += SDK_ASSERT(slice->upper_bound(command->time() - DELTA).next()->time() == command->time());
    rv += SDK_ASSERT(slice->lower_bound(command->time()).next()->time() == command->time());
    rv += SDK_ASSERT(slice->lower_bound(command->time() - DELTA).next()->time() == command->time());

    if (index != (NUM_COMMANDS-1))  // do not walk off the end
    {
      rv += SDK_ASSERT(slice->upper_bound(command->time()).next()->time() == (command->time()+1.0));
    }

    index++;
  }

  rv += SDK_ASSERT(index == NUM_COMMANDS);

  // test visitor
  class CategorySliceTest : public simData::DataSlice<simData::CategoryData>::Visitor
  {
  public:
    CategorySliceTest(int* returnValue)
    {
      rv_ = returnValue;
      index_ = 0;
    }
    virtual void operator()(const simData::CategoryData *command)
    {
      *rv_ += SDK_ASSERT(isValidCategory(command, index_));
      ++index_;
    }

  protected:
    size_t index_;
    int* rv_;
  };

  CategorySliceTest sc(&rv);
  slice->visit(&sc);*/

  return rv;
}

int iterateTest(simData::DataStore* dataStore)
{
  int rv = 0;

  rv += updateIterateTest(dataStore);
  rv += commandIterateTest(dataStore);
  rv += genericIterateTest(dataStore);
  rv += categoryIterateTest(dataStore);

  return rv;
}
// Demonstrates that duplicate time values are not permitted
int duplicateTimesCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  // Update the data store and get the update-slice for platform 1
  dataStore->update(1.0); // update to time 1.0
  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr) return rv;
  // Validate that the platform #1 has no duplicates and has data
  TestVisit testVisit;
  slice->visit(&testVisit);
  rv += SDK_ASSERT(testVisit.numVisits == 100);
  rv += SDK_ASSERT(testVisit.numErrors == 0);
  // TODO: Duplicate points
  //// Add a duplicate time to platform #1
  //addPlatformPoint(dataStore, 1, 15);
  // Re-validate
  testVisit.numVisits = testVisit.numErrors = 0;
  slice->visit(&testVisit);
  rv += SDK_ASSERT(testVisit.numVisits == 100);
  rv += SDK_ASSERT(testVisit.numErrors == 0);
  // Re-update, and re-validate
  dataStore->update(1.0); // update to time 1.0
  slice = dataStore->platformUpdateSlice(1);
  testVisit.numVisits = testVisit.numErrors = 0;
  slice->visit(&testVisit);
  rv += SDK_ASSERT(testVisit.numVisits == 100);
  rv += SDK_ASSERT(testVisit.numErrors == 0);
  return rv;
}

// Demonstration of iterating through all data points
int superformIteration(simData::DataStore* dataStore)
{
  int rv = 0;
  // Prime the data store with an update
  dataStore->update(0.0);
  // Iterate through platforms
  simData::DataStore::IdList idList;
  dataStore->idList(&idList, simData::PLATFORM);
  for (simData::DataStore::IdList::const_iterator i = idList.begin(); i != idList.end(); ++i)
  {
    rv += SDK_ASSERT(dataStore->objectType(*i) == simData::PLATFORM);
    const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(*i);
    rv += SDK_ASSERT(slice != nullptr);
    if (slice == nullptr)
      continue;
    // Iterate through the slice
    simData::PlatformUpdateSlice::Iterator iter(slice);
    int loopNum = 0;
    while (iter.hasNext())
    {
      const simData::PlatformUpdate* update = iter.next();
      // Make sure the data looks good on the update
      // Expected values based on addPlatformPoint() above
      rv += SDK_ASSERT(update->time() == loopNum);
      // Test the positions to verify data persists over time
      rv += SDK_ASSERT(update->x() == 0.0 + loopNum);
      rv += SDK_ASSERT(update->y() == 1.0 + loopNum);
      rv += SDK_ASSERT(update->z() == 2.0 + loopNum);
      loopNum++;
    }
  }
  return rv;
}

// Demonstration of data store time bounds retrieval and update; should also check num points
int timeBoundsCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  rv += SDK_ASSERT(dataStore->timeBounds(0).first == 0);
  rv += SDK_ASSERT(dataStore->timeBounds(0).second == NUM_POINTS - 1);
  rv += SDK_ASSERT(dataStore->timeBounds(1).first == 0);
  rv += SDK_ASSERT(dataStore->timeBounds(1).second == NUM_POINTS - 1);
  rv += SDK_ASSERT(dataStore->timeBounds(NUM_PLATS).first == 0);
  rv += SDK_ASSERT(dataStore->timeBounds(NUM_PLATS).second == NUM_POINTS - 1);
  return rv;
}

// Demonstration of getting individual historical data points (get value by time)
int historicalDataCheck(simData::DataStore* dataStore)
{
  // Update the data store and get the update-slice for platform 1
  dataStore->update(50); // update to time
  int rv = 0;
  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(1);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr) return rv;
  const simData::PlatformUpdate* update = slice->current();
  rv += SDK_ASSERT(update != nullptr);
  if (update == nullptr) return rv;

  // TODO

  return rv;
}

int interpTester(simData::DataStore* dataStore, uint64_t id, double timeVal)
{
  // Update the data store and get the update-slice for platform "id"
  dataStore->update(timeVal); // update to time
  int rv = 0;

  // If data store interpolation is not enabled, the data ought to look like the last integer's time
  if (!dataStore->isInterpolationEnabled())
    timeVal = floor(timeVal);

  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(id);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr) return rv;
  const simData::PlatformUpdate* update = slice->current();
  rv += SDK_ASSERT(update != nullptr);
  if (update == nullptr) return rv;
  // Test data now
  rv += SDK_ASSERT(update->time() == timeVal);
  rv += SDK_ASSERT(update->x() == timeVal);
  rv += SDK_ASSERT(update->y() == 1.0 + timeVal);
  rv += SDK_ASSERT(update->z() == 2.0 + timeVal);
  return rv;
}

// Demonstrates usage of interpolate class
int interpolateTest(simData::DataStore* dataStore)
{
  int rv = 0;
  simData::LinearInterpolator interp;
  dataStore->setInterpolator(&interp);
  // Test various flag combinations
  // Enable On: Can, and Is
  rv += SDK_ASSERT(true == dataStore->enableInterpolation(true));
  rv += SDK_ASSERT(true == dataStore->canInterpolate());
  rv += SDK_ASSERT(true == dataStore->isInterpolationEnabled());
  // Enable off: Can, and !Is
  rv += SDK_ASSERT(false == dataStore->enableInterpolation(false));
  rv += SDK_ASSERT(true == dataStore->canInterpolate());
  rv += SDK_ASSERT(false == dataStore->isInterpolationEnabled());
  // Remove the interpolator, then try to enable
  dataStore->setInterpolator(nullptr);
  rv += SDK_ASSERT(true == dataStore->canInterpolate());
  rv += SDK_ASSERT(false == dataStore->enableInterpolation(true));
  rv += SDK_ASSERT(false == dataStore->isInterpolationEnabled());
  // Re-add the interpolator and verify values Can and Is
  dataStore->setInterpolator(&interp);
  rv += SDK_ASSERT(true == dataStore->canInterpolate());
  // NOTE: enableInterpolation() MUST be called after setInterpolator for interpolation to be enabled
  rv += SDK_ASSERT(false == dataStore->isInterpolationEnabled());
  rv += SDK_ASSERT(true == dataStore->enableInterpolation(true));
  rv += SDK_ASSERT(true == dataStore->isInterpolationEnabled());

  // Now we test the actual interpolation through update()

  rv += SDK_ASSERT(interpTester(dataStore, 1, 0) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, NUM_POINTS - 1) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 5.5) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 50) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 50.5) == 0);
  // Try the same tests with interpolation off
  rv += SDK_ASSERT(false == dataStore->enableInterpolation(false));
  rv += SDK_ASSERT(interpTester(dataStore, 1, 0) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, NUM_POINTS - 1) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 5.5) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 50) == 0);
  rv += SDK_ASSERT(interpTester(dataStore, 1, 50.5) == 0);

  dataStore->setInterpolator(nullptr);
  return rv;
}

// Helper function to test that the time before a given time is what is expected
int testTimeBefore(const simData::PlatformUpdateSlice* slice, double time, double expected)
{
  simData::PlatformUpdateSlice::Iterator i(slice->lower_bound(time));
  double timeVal = std::numeric_limits<double>::max();
  if (i.hasPrevious())
    timeVal = i.previous()->time();
  return SDK_ASSERT(timeVal == expected);
}

// Helper function to test that the time after a given time is what is expected
int testTimeAfter(const simData::PlatformUpdateSlice* slice, double time, double expected)
{
  simData::PlatformUpdateSlice::Iterator i(slice->upper_bound(time));
  double timeVal = -std::numeric_limits<double>::max();
  if (i.hasNext())
    timeVal = i.next()->time();
  return SDK_ASSERT(timeVal == expected);
}

// Demonstration of getting time indices
int timeNextPreviousCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  // Prime the data store with an update
  dataStore->update(0.0);
  // Iterate through platforms
  simData::DataStore::IdList idList;
  dataStore->idList(&idList, simData::PLATFORM);
  rv += SDK_ASSERT(!idList.empty());
  if (idList.empty())
    return rv;
  // Get a slice
  const simData::PlatformUpdateSlice* slice = dataStore->platformUpdateSlice(*idList.begin());
  rv += SDK_ASSERT(slice != nullptr);
  if (!slice)
    return rv;
  // Test some arbitrary times
  rv += testTimeBefore(slice, 5.5, 5);
  rv += testTimeBefore(slice, 5, 4);
  rv += testTimeBefore(slice, 0, std::numeric_limits<double>::max());
  rv += testTimeBefore(slice, NUM_POINTS + 900, NUM_POINTS - 1);
  rv += testTimeBefore(slice, NUM_POINTS - 1, NUM_POINTS - 2);
  rv += testTimeAfter(slice, 5.5, 6);
  rv += testTimeAfter(slice, 6, 7);
  rv += testTimeAfter(slice, NUM_POINTS - 1, -std::numeric_limits<double>::max());
  rv += testTimeAfter(slice, -100, 0);
  rv += testTimeAfter(slice, 0, 1);
  return rv;
}

// Demonstration of getting entity type from an ID
int getEntityTypeCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  // Test some wild values
  rv += SDK_ASSERT(simData::NONE == dataStore->objectType(0));
  rv += SDK_ASSERT(simData::NONE == dataStore->objectType(NUM_PLATS + 1));
  // Test inner values
  rv += SDK_ASSERT(simData::PLATFORM == dataStore->objectType(1));
  rv += SDK_ASSERT(simData::PLATFORM == dataStore->objectType(NUM_PLATS));
  assert(NUM_PLATS > 1); // Test fails if condition is not true due to next line
  rv += SDK_ASSERT(simData::PLATFORM == dataStore->objectType(NUM_PLATS / 2));
  // TODO: Add beams, gate, projector tests
  return rv;
}

// Demonstration of finding platform by name
int findEntityCheck(simData::DataStore* dataStore)
{
  int rv = 0;
  simData::DataStore::IdList ids;
  // Start by testing basics of idList()
  ids.clear();
  dataStore->idList(&ids, simData::ALL);
  rv += SDK_ASSERT(ids.size() == NUM_PLATS);
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(1)) != ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), NUM_PLATS) != ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(0)) == ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), NUM_PLATS+1) == ids.end());
  // Should have same results for platforms
  ids.clear();
  dataStore->idList(&ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.size() == NUM_PLATS);
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(1)) != ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), NUM_PLATS) != ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(0)) == ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), NUM_PLATS+1) == ids.end());
  // Should have no results for beams
  ids.clear();
  dataStore->idList(&ids, simData::ObjectType(simData::BEAM | simData::GATE));
  rv += SDK_ASSERT(ids.empty());
  // Find a platform in the middle
  assert(NUM_PLATS > 1); // This test assumes NUM_PLATS > 1
  ids.clear();
  std::string expectedName = expectedValue("platform", NUM_PLATS / 2);
  dataStore->idListByName(expectedName, &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.size() == 1);
  if (ids.size() == 1)
    rv += SDK_ASSERT(ids[0] == (NUM_PLATS / 2));
  // Search for nonexistent platform
  ids.clear();
  dataStore->idListByName(expectedValue("platform", NUM_PLATS+1), &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.empty());

  // Validate that platform 1 exists; this is important for next part where name changes
  ids.clear();
  dataStore->idListByName(expectedValue("platform", 1), &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.size() == 1 && ids[0] == 1);
  // Change a name and do the search over again
  simData::DataStore::Transaction transaction;
  simData::PlatformPrefs* prefs = dataStore->mutable_platformPrefs(1, &transaction);
  prefs->mutable_commonprefs()->set_name("Another name");
  transaction.complete(&prefs);
  // Validate that platform 1 exists; this is important for next part where name changes
  ids.clear();
  dataStore->idListByName(expectedValue("platform", 1), &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.empty());
  // Now search for the renamed value
  ids.clear();
  dataStore->idListByName("Another name", &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.size() == 1 && ids[0] == 1);
  // Make sure capitalization counts
  ids.clear();
  dataStore->idListByName("Another Name", &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.empty());

  // Next we'll test that we can get multiple platforms of the same name; start by renaming another plat
  prefs = dataStore->mutable_platformPrefs(2, &transaction);
  prefs->mutable_commonprefs()->set_name("Another name");
  transaction.complete(&prefs);
  // Make sure it took effect on ID 2...
  ids.clear();
  dataStore->idListByName(expectedValue("Platform ", 2), &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.empty());
  // Now search for renamed value; should have 2 entries!
  ids.clear();
  dataStore->idListByName("Another name", &ids, simData::PLATFORM);
  rv += SDK_ASSERT(ids.size() == 2);
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(1)) != ids.end());
  rv += SDK_ASSERT(std::find(ids.begin(), ids.end(), static_cast<uint64_t>(2)) != ids.end());

  return rv;
}

int commandCheck(simData::DataStore* dataStore)
{
  int rv = 0;

  // reset the datastore
  dataStore->update(0);

  // grab current name
  simData::DataStore::Transaction t;
  const uint64_t id = 1;
  const simData::PlatformPrefs* prefs = dataStore->platformPrefs(id, &t);
  const std::string origName = prefs->commonprefs().name();
  t.release(&prefs);

  // set a new name in the future
  simData::PlatformCommand *pcp = dataStore->addPlatformCommand(id, &t);
  pcp->mutable_updateprefs()->mutable_commonprefs()->set_name("test");
  pcp->set_time(10.0);
  t.commit();

  dataStore->update(9.5);
  prefs = dataStore->platformPrefs(id, &t);
  rv += SDK_ASSERT(prefs->commonprefs().name() == origName);
  t.release(&prefs);

  dataStore->update(10);
  prefs = dataStore->platformPrefs(id, &t);
  rv += SDK_ASSERT(prefs->commonprefs().name() == "test");
  t.release(&prefs);

  return rv;
}

int dataLimitCheck(simData::DataStore* dataStore)
{
  // assumes there are NUM_POINTS with times 0 to NUM_POINTS-1
  int rv = 0;
  simData::DataStore::Transaction t;
  const uint64_t id = 1;

  dataStore->setDataLimiting(true);
  simData::PlatformPrefs* prefs = dataStore->mutable_platformPrefs(id, &t);
  // drop first point using time
  const size_t numItems = dataStore->platformUpdateSlice(id)->numItems();
  prefs->mutable_commonprefs()->set_datalimittime(NUM_POINTS-1);
  t.commit();
  rv += SDK_ASSERT(dataStore->platformUpdateSlice(id)->numItems() == numItems - 1);

  // drop second point using points
  prefs = dataStore->mutable_platformPrefs(id, &t);
  prefs->mutable_commonprefs()->set_datalimitpoints(NUM_POINTS-2);
  t.commit();

  rv += SDK_ASSERT(dataStore->platformUpdateSlice(id)->numItems() == numItems - 2);

  // make sure time bound is correct
  rv += SDK_ASSERT(dataStore->platformUpdateSlice(id)->firstTime() == 2.0);

  // try a no-op
  prefs = dataStore->mutable_platformPrefs(id, &t);
  prefs->mutable_commonprefs()->set_datalimitpoints(NUM_POINTS-1);
  t.commit();

  rv += SDK_ASSERT(dataStore->platformUpdateSlice(id)->numItems() == numItems - 2);

  // delete everything
  prefs = dataStore->mutable_platformPrefs(id, &t);
  prefs->mutable_commonprefs()->set_datalimittime(0);
  t.commit();
  rv += SDK_ASSERT(dataStore->platformUpdateSlice(id)->numItems() == 1);

  return rv;
}

int testDataStoreRetrieval(bool inOrder)
{
  simUtil::DataStoreTestHelper testHelper;
  int rv = 0;
  simData::DataStore* dataStore = testHelper.dataStore();
  double t = simCore::systemTimeToSecsBgnYr();
  if (inOrder)
  {
    for (size_t k = 0; k < NUM_PLATS; ++k)
      addPlatform(testHelper, inOrder);
  }
  else
  {
    for (size_t k = NUM_PLATS; k > 0; --k)
      addPlatform(testHelper, inOrder);
  }
  double elapsed = simCore::systemTimeToSecsBgnYr() - t;
  cout << "Time to add " << NUM_PLATS << " platforms with " << NUM_POINTS << " points: " << elapsed << endl;

  rv += SDK_ASSERT(0 == sanityCheck(dataStore));
  rv += SDK_ASSERT(0 == iterateTest(dataStore));
  rv += SDK_ASSERT(0 == superformIteration(dataStore));
  rv += SDK_ASSERT(0 == timeBoundsCheck(dataStore));
  rv += SDK_ASSERT(0 == historicalDataCheck(dataStore));
  // TODO: SDK-61 Need a more realistic test rv += SDK_ASSERT(0 == interpolateTest(dataStore));
  rv += SDK_ASSERT(0 == timeNextPreviousCheck(dataStore));
  rv += SDK_ASSERT(0 == getEntityTypeCheck(dataStore));
  rv += SDK_ASSERT(0 == findEntityCheck(dataStore));
  rv += SDK_ASSERT(0 == duplicateTimesCheck(dataStore));
  rv += SDK_ASSERT(0 == commandCheck(dataStore));
  rv += SDK_ASSERT(0 == dataLimitCheck(dataStore));

  return rv;
}

}

int TestMemRetrieval(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv1 = testDataStoreRetrieval(true);
  cout << "TestMemRetrieval (Fwd): " << (rv1 == 0 ? "PASSED" : "FAILED") << endl;
  int rv2 = testDataStoreRetrieval(false);
  cout << "TestMemRetrieval (Rev): " << (rv2 == 0 ? "PASSED" : "FAILED") << endl;
  return rv1 + rv2;
}

