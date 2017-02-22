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
#include <algorithm>
#include "simData/DataTable.h"
#include "simData/DataStoreProxy.h"

namespace simData
{

DataStoreProxy::DataStoreProxy(DataStore* dataStore)
  : dataStore_(dataStore),
    interpolator_(NULL)
{
  assert(dataStore_);
}

void DataStoreProxy::reset(DataStore* newDataStore)
{
  if (dataStore_ == newDataStore)
    return;

  // only capture old internals if there is somewhere new to put them
  InternalsMemento *oldInternals = newDataStore ? dataStore_->createInternalsMemento() : NULL;

  DataStore *const oldDataStore = dataStore_;
  dataStore_ = newDataStore;

  if (oldInternals)
  {
    oldInternals->apply(*dataStore_);
    delete oldInternals;
  }

  delete oldDataStore;
}

DataStoreProxy::~DataStoreProxy()
{
  delete dataStore_;
}

DataStore::InternalsMemento* DataStoreProxy::createInternalsMemento() const
{
  return dataStore_->createInternalsMemento();
}

void DataStoreProxy::setInterpolator(Interpolator *interpolator)
{
  interpolator_ = interpolator;
  dataStore_->setInterpolator(interpolator);
}

Interpolator* DataStoreProxy::interpolator() const
{
  return isInterpolationEnabled() ? interpolator_ : NULL;
}

void DataStoreProxy::addListener(DataStore::ListenerPtr callback)
{
  dataStore_->addListener(callback);
}

void DataStoreProxy::removeListener(DataStore::ListenerPtr callback)
{
  dataStore_->removeListener(callback);
}

void DataStoreProxy::addScenarioListener(DataStore::ScenarioListenerPtr callback)
{
  dataStore_->addScenarioListener(callback);
}

void DataStoreProxy::removeScenarioListener(DataStore::ScenarioListenerPtr callback)
{
  dataStore_->removeScenarioListener(callback);
}

void DataStoreProxy::bindToClock(simCore::Clock* clock)
{
  dataStore_->bindToClock(clock);
}

simCore::Clock* DataStoreProxy::getBoundClock() const
{
  return dataStore_->getBoundClock();
}

} // End of namespace simData

