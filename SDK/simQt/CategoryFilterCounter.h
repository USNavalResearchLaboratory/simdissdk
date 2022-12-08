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
#ifndef SIMQT_CATEGORYFILTERCOUNTER_H
#define SIMQT_CATEGORYFILTERCOUNTER_H

#include <map>
#include <memory>
#include <vector>
#include <QObject>
#include "simCore/Common/Export.h"
#include "simData/ObjectId.h"
#include "simData/CategoryData/CategoryFilter.h"

namespace simQt {

/** Structure that contains the results of a category counter operation. */
struct CategoryCountResults
{
  /** Map of integer category value to number of values in the category. */
  typedef std::map<int, size_t> ValueToCountMap;
  /** Map of integer category name to the value-to-counts map. */
  typedef std::map<int, ValueToCountMap> AllCategories;

  /** Maps all known categories to the resulting values. */
  AllCategories allCategories;
};

/**
 * Algorithm that will count the number of objects from a given ID list that are impacted by a
 * given filter.  This is intended to give a runtime count of the number of entities that will
 * be impacted by clicking a category value line in a category tree widget.
 *
 * Note that this algorithm is O(m * n), scaling both on the number of entities (m) and the
 * total number of category values (n).
 */
class SDKQT_EXPORT CategoryFilterCounter : public QObject
{
  Q_OBJECT;
public:
  /** Default constructor */
  explicit CategoryFilterCounter(QObject* parent = nullptr);

  /** Sets the category filter to use */
  void setFilter(const simData::CategoryFilter& filter);
  /** Sets the entity filter, restricting the counts. Useful for only listing PLATFORMS, for example, in a platform-only list. */
  void setObjectTypes(simData::ObjectType objectTypes);

  /** Retrieves the most recent results set */
  const CategoryCountResults& results() const;

  /**
   * Prepares to run testAllCategories().  This method calls non-thread-safe methods on the filter's
   * data store.  In a single threaded context, this method does not need to be explicitly called.
   * However, for testAllCategories() to function in another thread than the data store, this
   * method must be called in the same thread as the data store.
   */
  void prepare();

public Q_SLOTS:
  /**
   * Performs the testing.  When done, results() will be valid, and resultsReady() will be emitted.
   * Though this method is not threaded, it is safe to call this in another thread with respect to
   * the category filter and its data store, so long as prepare() is called first in the same thread
   * as the data store.
   * In typical usage, prepare() is optional.  In threaded usage, prepare() is required in the main
   * thread.
   */
  void testAllCategories();

Q_SIGNALS:
  /** Called when testAllCategories() is completed. */
  void resultsReady(const simQt::CategoryCountResults& results);

private:
  /** Local storage structure for current category data.  Filled out by prepare(). */
  struct IdAndCategories
  {
    simData::ObjectId id;
    simData::CategoryFilter::CurrentCategoryValues categories;
  };

  /**
   * Retrieves the list of IDs out of the data store.  This is called in prepare() and is not thread
   * safe with regards to interactions with the data store.
   */
  void idList_(std::vector<simData::ObjectId>& ids) const;

  /** Returns a list of ID+Categories that matches filter_.  Thread safe. */
  void filteredIds_(int ignoreNameInt, std::vector<const IdAndCategories*>& ids) const;
  /** Tests an individual category and sets the counts for that category */
  void testCategory_(int nameInt, CategoryCountResults::ValueToCountMap& countMap);

  /** Stores all entity IDs and their current category values. */
  std::vector<IdAndCategories> allEntities_;
  /** Map of category name, to map of category value to count. */
  CategoryCountResults results_;
  /** Current filter supplied by end user. */
  std::unique_ptr<simData::CategoryFilter> filter_;
  /** Is set true when filter changes, until prepare() is called. */
  bool dirtyFlag_;
  /** Filter entity results by object type */
  simData::ObjectType objectTypes_;
};

/**
 * Asynchronous implementation of a category counter.  Since CategoryFilterCounter is potentially
 * expensive, it can be advantageous to perform the calculations in the background.  This
 * implementation ensures that the counter only runs one at a time, and additional calls are
 * queued up for execution once the first execution finishes.
 */
class SDKQT_EXPORT AsyncCategoryCounter : public QObject
{
  Q_OBJECT;
public:
  /** Default constructor */
  explicit AsyncCategoryCounter(QObject* parent = nullptr);

  virtual ~AsyncCategoryCounter();

  /** Retrieves the last fully executed results. */
  const simQt::CategoryCountResults& lastResults() const;

  /** Sets the entity filter, restricting the counts. Useful for only listing PLATFORMS, for example, in a platform-only list. */
  void setObjectTypes(simData::ObjectType objectTypes);

public Q_SLOTS:
  /**
   * Sets the category filter to use.  Immediately calls testAsync().  If a count is already
   * queued, then it is dropped and this new filter is used instead.  Only one count occurs
   * asynchronously at a time.
   */
  void setFilter(const simData::CategoryFilter& filter);

  /**
   * Tests the filter against all known entities.  This function will query the data store for the
   * list of all entities and their category data, then prepare a CategoryFilterCounter.  It
   * executes the count in the background.  Once the count is complete, the resultsReady() signal
   * is emitted.  If this is called while a count is ongoing in the background, another count will
   * start once the first one finishes.  Only one count is queued at a time.
   */
  void asyncCountEntities();

Q_SIGNALS:
  /** Indicates that the asynchronous operation from testAsync() has completed. */
  void resultsReady(const simQt::CategoryCountResults& results);

private Q_SLOTS:
  /** Captures the results from counter_, clears the future watcher, emits results, and restarts if needed. */
  void emitResults_();

private:
  simQt::CategoryCountResults lastResults_;
  CategoryFilterCounter* counter_;
  std::unique_ptr<simData::CategoryFilter> nextFilter_;
  bool retestPending_;
  simData::ObjectType objectTypes_;
};

}

#endif /* SIMQT_CATEGORYFILTERCOUNTER_H */
