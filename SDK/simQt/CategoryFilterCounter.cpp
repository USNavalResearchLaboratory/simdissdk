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
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/DataStore.h"
#include "simQt/CategoryFilterCounter.h"

namespace simQt {

CategoryFilterCounter::CategoryFilterCounter(QObject* parent)
  : QObject(parent),
    dirtyFlag_(false)
{
}

void CategoryFilterCounter::prepare()
{
  // Turn off the dirty flag immediately so we don't have to check each return case
  if (!dirtyFlag_)
    return;
  dirtyFlag_ = false;

  // Set up initial state
  allEntities_.clear();
  results_.allCategories.clear();
  if (!filter_)
    return;
  const simData::DataStore* ds = filter_->getDataStore();
  if (!ds)
    return;

  // Make a copy of all the current category data
  std::vector<simData::ObjectId> ids;
  idList_(ids);
  for (auto i = ids.begin(); i != ids.end(); ++i)
  {
    IdAndCategories entry;
    entry.id = *i;
    simData::CategoryFilter::getCurrentCategoryValues(*ds, entry.id, entry.categories);
    allEntities_.push_back(entry);
  }

  // Initialize all filter entries based on state of filter
  const simData::CategoryNameManager& nameManager = ds->categoryNameManager();
  std::vector<int> names;
  nameManager.allCategoryNameInts(names);
  for (auto i = names.begin(); i != names.end(); ++i)
  {
    const int nameInt = *i;
    CategoryCountResults::ValueToCountMap countMap;

    // Mark all category values as 0
    std::vector<int> values;
    nameManager.allValueIntsInCategory(nameInt, values);
    for (auto i = values.begin(); i != values.end(); ++i)
      countMap[*i] = 0;
    // Also mark NO VALUE as 0
    countMap[simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME] = 0;

    // Save the count map
    results_.allCategories[nameInt] = countMap;
  }
}

void CategoryFilterCounter::setFilter(const simData::CategoryFilter& filter)
{
  // Avoid copy constructor, which could add a listener
  filter_.reset(new simData::CategoryFilter(filter.getDataStore()));
  filter_->assign(filter, false);
  dirtyFlag_ = true;
}

void CategoryFilterCounter::idList_(std::vector<simData::ObjectId>& ids) const
{
  ids.clear();
  const simData::DataStore* ds = filter_->getDataStore();
  if (!ds)
    return;
  ds->idList(&ids);
}

void CategoryFilterCounter::testAllCategories()
{
  if (dirtyFlag_)
    prepare();
  // prepare() should turn off the dirty flag
  assert(!dirtyFlag_);

  // Test every category we know about
  for (auto i = results_.allCategories.begin(); i != results_.allCategories.end(); ++i)
    testCategory_(i->first, i->second);
  emit resultsReady(results_);
}

const CategoryCountResults& CategoryFilterCounter::results() const
{
  return results_;
}

// Inside thread (protected)
void CategoryFilterCounter::filteredIds_(int ignoreNameInt, std::vector<const IdAndCategories*>& ids) const
{
  simData::CategoryFilter baseFilter(*filter_);
  baseFilter.removeName(ignoreNameInt);

  // Find all IDs that match the new filter without the name provided
  for (auto idIter = allEntities_.begin(); idIter != allEntities_.end(); ++idIter)
  {
    if (baseFilter.matchData(idIter->categories))
      ids.push_back(&(*idIter));
  }
}

// Inside thread (protected)
void CategoryFilterCounter::testCategory_(int nameInt, CategoryCountResults::ValueToCountMap& countMap)
{
  // Start out by not testing anything in this filter
  simData::CategoryFilter baseFilter(*filter_);
  baseFilter.removeName(nameInt);

  // Get all IDs that match the filter excluding this name
  std::vector<const IdAndCategories*> idDataVec;
  filteredIds_(nameInt, idDataVec);

  // Loop through each value in this category.  If the platform matches, add it
  for (auto vi = countMap.begin(); vi != countMap.end(); ++vi)
  {
    // Test what happens when this filter value is turned on
    baseFilter.setValue(nameInt, vi->first, true);
    size_t& numMatches = vi->second;
    // Assertion failure means the prepare() missed a category value
    assert(numMatches == 0);

    // Loop through each entity of interest
    for (auto idIter = idDataVec.begin(); idIter != idDataVec.end(); ++idIter)
    {
      const IdAndCategories& idData = **idIter;
      const simData::ObjectId& id = idData.id;

      // Test the value and increment if it matches
      const bool matches = baseFilter.matchData(idData.categories);
      if (matches)
        ++vi->second;
    }

    // Turn that value back off.  removeName()
    baseFilter.setValue(nameInt, vi->first, false);
  }
}

////////////////////////////////////////////////////

AsyncCategoryCounter::AsyncCategoryCounter(QObject* parent)
  : QObject(parent),
    counter_(NULL),
    retestPending_(false)
{
}

AsyncCategoryCounter::~AsyncCategoryCounter()
{
}

void AsyncCategoryCounter::setFilter(const simData::CategoryFilter& filter)
{
  nextFilter_.reset(new simData::CategoryFilter(filter));
  retestPending_ = true;
  asyncCountEntities();
}

void AsyncCategoryCounter::asyncCountEntities()
{
  if (counter_ != NULL)
  {
    retestPending_ = true;
    return;
  }

  // Turn off the retest flag
  retestPending_ = false;

  // Create a watcher that will tell us when the task is complete
  QFutureWatcher<void>* watcher = new QFutureWatcher<void>();

  counter_ = new CategoryFilterCounter(watcher);
  if (nextFilter_ != NULL)
    counter_->setFilter(*nextFilter_);
  counter_->prepare();

  // Be sure to set up a connect() before setFuture() to avoid race.
  connect(watcher, SIGNAL(finished()), this, SLOT(emitResults_()));
  // To prevent race conditions use deleteLater() instead of Qt parents to manage lifespan
  connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));

  watcher->setFuture(QtConcurrent::run(counter_, &CategoryFilterCounter::testAllCategories));
}

void AsyncCategoryCounter::emitResults_()
{
  // This call happens in the main thread and is the "join" for the job
  lastResults_ = counter_->results();
  emit resultsReady(lastResults_);

  // just set to NULL, the deleteLater() for watcher will do the actual delete
  counter_ = NULL;

  // Retest now that it's safe to do so
  if (retestPending_)
    asyncCountEntities();
}

const simQt::CategoryCountResults& AsyncCategoryCounter::lastResults() const
{
  return lastResults_;
}

}
