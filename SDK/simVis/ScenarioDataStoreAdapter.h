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
#ifndef SIMVIS_SCENARIO_DATASTORE_ADAPTER_H
#define SIMVIS_SCENARIO_DATASTORE_ADAPTER_H

#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include <map>
#include <vector>
#include <set>

namespace simVis
{
  class ScenarioManager;

  /**
  * Binds a DataStore to a ScenarioManager. This class is used internally by ScenarioManager
  * to automatically update the visual scenario based on DataStore notifications
  */
  class SDKVIS_EXPORT ScenarioDataStoreAdapter
  {
  public:

    /**
    * Constructs a new data store adapter. The adapter won't do anything until
    * you bind it with a call to bind().
    */
    ScenarioDataStoreAdapter() { }

    /**
    * Destructor
    */
    virtual ~ScenarioDataStoreAdapter() {}

    /**
    * Constructs a new data store adapter and creates a binding.
    * @param dataStore Data store to which to bind the scenario
    * @param scenario  Scenario to which to bind the data store
    */
    ScenarioDataStoreAdapter(simData::DataStore* dataStore, ScenarioManager* scenario);

    /**
    * Binds the specified data store to a scenario manager.
    * @param dataStore Data store to which to bind the scenario
    * @param scenario  Scenario to which to bind the data store
    */
    void bind(simData::DataStore* dataStore, ScenarioManager* scenario);

    /**
    * Removes the binding associated with a data store.
    * @param dataStore Data store to unbind from its scenario
    */
    void unbind(simData::DataStore* dataStore);

    /**
    * Fetches a list of bound DataStores.
    * @param[out] output Set of bound datastores.
    */
    void getBindings(std::set<simData::DataStore*>& output) const;

  private:
    std::map<simData::DataStore*, simData::DataStore::ListenerPtr> listeners_;
  };

} // namespace simVis

#endif // SIMVIS_SCENARIO_DATASTORE_ADAPTER_H
