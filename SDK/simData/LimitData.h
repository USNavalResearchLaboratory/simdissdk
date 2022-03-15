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
#ifndef SIMDATA_LIMIT_DATA_H
#define SIMDATA_LIMIT_DATA_H

/**
 * These functions are helpers for managing the size of time sorted data
 * structures.
 *
 * limitData - the basic functionality, called if you have the limit preferences
 * limitDataAndDelete - deleting version of limitData()
 *
 * limitEntityData - most likely the function you want, retrieves the
 * data limiting preferences from the DataStore, and applies them
 *
 * limitScenarioData - as above, but for entityId == 0
 *
 * There are also deleting versions for these two functions.
 *
 */

#include <cassert>
#include "simData/DataLimiter.h"
#include "simData/DataStore.h"

namespace simData
{
/// Limit data in a map according to time and points
template<typename DataType>
static void limitData(std::map<double, DataType> &data, double timeLimit, size_t pointsLimit)
{
  DataLimiter<std::map<double, DataType> > limiter;
  limiter.limitDataSeconds(data, timeLimit);
  limiter.limitDataPoints(data, pointsLimit);
}

/// Limit data in a map according to time and points (calls delete on the data)
template<typename DataType>
static void limitDataAndDelete(std::map<double, DataType*> &data, double timeLimit, size_t pointsLimit)
{
  DataLimiterDynamic<std::map<double, DataType*> > limiter;
  limiter.limitDataSeconds(data, timeLimit);
  limiter.limitDataPoints(data, pointsLimit);
}

/// Limit data in a map according to time and points
template<typename DataType>
static void limitDataPoints(std::map<double, DataType> &data, size_t pointsLimit)
{
  DataLimiter<std::map<double, DataType> > limiter;
  limiter.limitDataPoints(data, pointsLimit);
}

/// Limit data in a map according to time and points (calls delete on the data)
template<typename DataType>
static void limitDataPointsAndDelete(std::map<double, DataType*> &data, size_t pointsLimit)
{
  DataLimiterDynamic<std::map<double, DataType*> > limiter;
  limiter.limitDataPoints(data, pointsLimit);
}

/// Limit data in a map according to time and points
template<typename DataType>
static void limitDataTime(std::map<double, DataType> &data, double timeLimit)
{
  DataLimiter<std::map<double, DataType> > limiter;
  limiter.limitDataSeconds(data, timeLimit);
}

/// Limit data in a map according to time and points (calls delete on the data)
template<typename DataType>
static void limitDataTimeAndDelete(std::map<double, DataType*> &data, double timeLimit)
{
  DataLimiterDynamic<std::map<double, DataType*> > limiter;
  limiter.limitDataSeconds(data, timeLimit);
}

/// Retrieve the scenario data limit preferences, and use them to limit data.
template<typename DataType>
static void limitScenarioData(std::map<double, DataType> &data, const simData::DataStore &dataStore)
{
  // early out if the data store is not limited
  if (!dataStore.dataLimiting())
    return;

  simData::DataStore::Transaction txn;
  const simData::ScenarioProperties *props = dataStore.scenarioProperties(&txn);
  if (props)
  {
    limitDataTime(data, props->datalimittime());
    limitDataPoints(data, props->datalimitpoints());
  }
}

/**
 * Retrieve the data limiting preferences for the given entity, and use them
 * to limit data.
 */
template<typename DataType>
static void limitEntityData(std::map<double, DataType> &data, const simData::DataStore &dataStore, simData::ObjectId entityId)
{
  // early out if the data store is not limited
  if (!dataStore.dataLimiting())
    return;

  if (entityId == 0)
  {
    limitScenarioData(data, dataStore);
    return;
  }

  simData::DataStore::Transaction txn;
  const simData::CommonPrefs *commonPrefs = dataStore.commonPrefs(entityId, &txn);
  if (commonPrefs)
  {
    limitDataTime(data, commonPrefs->datalimittime());
    limitDataPoints(data, commonPrefs->datalimitpoints());
  }
}

//----------------------------------------------------------------------------
/// Deleting version of limitScenarioData()
template<typename DataType>
static void limitScenarioDataAndDelete(std::map<double, DataType*> &data, const simData::DataStore &dataStore)
{
  // early out if the data store is not limited
  if (!dataStore.dataLimiting())
    return;

  simData::DataStore::Transaction txn;
  const simData::ScenarioProperties *props = dataStore.scenarioProperties(&txn);
  if (props)
  {
    limitDataTimeAndDelete(data, props->datalimittime());
    limitDataPointsAndDelete(data, props->datalimitpoints());
  }
}

/// Deleting version of limitEntityData()
template<typename DataType>
static void limitEntityDataAndDelete(std::map<double, DataType*> &data, const simData::DataStore &dataStore, simData::ObjectId entityId)
{
  // early out if the data store is not limited
  if (!dataStore.dataLimiting())
    return;

  if (entityId == 0)
  {
    limitScenarioDataAndDelete(data, dataStore);
    return;
  }

  simData::DataStore::Transaction txn;
  const simData::CommonPrefs *commonPrefs = dataStore.commonPrefs(entityId, &txn);
  if (commonPrefs)
  {
    limitDataTimeAndDelete(data, commonPrefs->datalimittime());
    limitDataPointsAndDelete(data, commonPrefs->datalimitpoints());
  }
}

}

#endif

