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
#include "simNotify/Notify.h"
#include "simCore/Time/Clock.h"
#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/LobGroup.h"
#include "simVis/Scenario.h"

using namespace simVis;

#define LC "[SimDataStoreAdapter] "

// -----------------------------------------------------------------------

namespace
{

/// handle notifications from the data store which include more information than the simple Observer
class MyListener : public simData::DataStore::Listener
{
public:
  explicit MyListener(ScenarioManager *parent)
    : scenarioManager_(parent)
  {
  }

  /// new entity has been added, with the given id and type
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    switch (ot)
    {
    case simData::PLATFORM: addPlatform_(*source, newId); break;
    case simData::BEAM: addBeam_(*source, newId); break;
    case simData::GATE: addGate_(*source, newId); break;
    case simData::PROJECTOR: addProjector_(*source, newId); break;
    case simData::LASER: addLaser_(*source, newId); break;
    case simData::LOB_GROUP: addLobGroup_(*source, newId); break;

    case simData::ALL: // shouldn't see these
    case simData::NONE:
      assert(false);
    }
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    scenarioManager_->removeEntity(removedId);
  }

  /// prefs for the given entity have been changed
  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id)
  {
    switch (source->objectType(id))
    {
    case simData::PLATFORM: changePlatformPrefs_(*source, id); break;
    case simData::BEAM: changeBeamPrefs_(*source, id); break;
    case simData::GATE: changeGatePrefs_(*source, id); break;
    case simData::PROJECTOR: changeProjectorPrefs_(*source, id); break;
    case simData::LASER: changeLaserPrefs_(*source, id); break;
    case simData::LOB_GROUP: changeLobGroupPrefs_(*source, id); break;

    case simData::ALL: // shouldn't see these
    case simData::NONE:
      assert(false);
    }
  }

  /// current time has been changed
  virtual void onTimeChange(simData::DataStore *source)
  {
    scenarioManager_->update(source);
  }

  /// something has changed in the entity category data
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    // category data has no effect on visualization
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    // already handled by the prefs change notification
  }

  /// entity's data was flushed, 0 means entire scenario was flushed
  virtual void onFlush(simData::DataStore *source, simData::ObjectId flushedId)
  {
    scenarioManager_->flush(flushedId);
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source)
  {
    // no-op
  }

private: // methods
  void addPlatform_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::PlatformProperties props;
    simData::DataStore::Transaction xaction;
    const simData::PlatformProperties *liveProps = ds.platformProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps;
    xaction.release(&liveProps);

    scenarioManager_->addPlatform(props, ds);
  }

  void addBeam_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::BeamProperties props;

    simData::DataStore::Transaction xaction;
    const simData::BeamProperties *liveProps = ds.beamProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps; // makes a copy inside the xaction
    xaction.release(&liveProps);

    scenarioManager_->addBeam(props, ds);
  }

  void addGate_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::GateProperties props;
    simData::DataStore::Transaction xaction;
    const simData::GateProperties *liveProps = ds.gateProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps;
    xaction.release(&liveProps);

    scenarioManager_->addGate(props, ds);
  }

  void addProjector_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::ProjectorProperties props;
    simData::DataStore::Transaction xaction;
    const simData::ProjectorProperties *liveProps = ds.projectorProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps;
    xaction.release(&liveProps);

    scenarioManager_->addProjector(props, ds);
  }

  void addLaser_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::LaserProperties props;
    simData::DataStore::Transaction xaction;
    const simData::LaserProperties *liveProps = ds.laserProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps;
    xaction.release(&liveProps);

    scenarioManager_->addLaser(props, ds);
  }

  void addLobGroup_(simData::DataStore &ds, simData::ObjectId newId) const
  {
    simData::LobGroupProperties props;
    simData::DataStore::Transaction xaction;
    const simData::LobGroupProperties *liveProps = ds.lobGroupProperties(newId, &xaction);
    if (liveProps)
      props = *liveProps;
    xaction.release(&liveProps);

    scenarioManager_->addLobGroup(props, ds);
  }

  void changePlatformPrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::PlatformPrefs          prefs;
    simData::DataStore::Transaction xaction;
    const simData::PlatformPrefs* livePrefs = ds.platformPrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setPlatformPrefs(id, prefs);
  }

  void changeBeamPrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::BeamPrefs              prefs;
    simData::DataStore::Transaction xaction;
    const simData::BeamPrefs* livePrefs = ds.beamPrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setBeamPrefs(id, prefs);
  }

  void changeGatePrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::GatePrefs              prefs;
    simData::DataStore::Transaction xaction;
    const simData::GatePrefs* livePrefs = ds.gatePrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setGatePrefs(id, prefs);
  }

  void changeProjectorPrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::ProjectorPrefs         prefs;
    simData::DataStore::Transaction xaction;
    const simData::ProjectorPrefs* livePrefs = ds.projectorPrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setProjectorPrefs(id, prefs);
  }

  void changeLaserPrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::LaserPrefs             prefs;
    simData::DataStore::Transaction xaction;
    const simData::LaserPrefs* livePrefs = ds.laserPrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setLaserPrefs(id, prefs);
  }

  void changeLobGroupPrefs_(simData::DataStore &ds, simData::ObjectId id)
  {
    simData::LobGroupPrefs            prefs;
    simData::DataStore::Transaction xaction;
    const simData::LobGroupPrefs* livePrefs = ds.lobGroupPrefs(id, &xaction);
    prefs = *livePrefs;
    xaction.complete(&livePrefs);

    scenarioManager_->setLobGroupPrefs(id, prefs);
  }

private: // data
  ScenarioManager *scenarioManager_;
};

// Observer for time clock mode changes
class MyClockModeChangeObserver : public simCore::Clock::ModeChangeObserver
{
public:
  MyClockModeChangeObserver(ScenarioManager* scenarioManager, simCore::Clock* clock)
    : scenarioManager_(scenarioManager), clock_(clock)
  {
    //nop
  }

public: // ModeChangeObserver interface

  virtual void onModeChange(simCore::Clock::Mode newMode)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

  virtual void onDirectionChange(simCore::TimeDirection newDirection)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

  virtual void onScaleChange(double newValue)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

  virtual void onCanLoopChange(bool newVal)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

  virtual void onUserEditableChanged(bool userCanEdit)
  {
    scenarioManager_->notifyOfClockChange(clock_);
  }

private: // data
  ScenarioManager *scenarioManager_;
  simCore::Clock  *clock_;
};
}

// -----------------------------------------------------------------------
ScenarioDataStoreAdapter::ScenarioDataStoreAdapter(simData::DataStore* dataStore, ScenarioManager* scenario)
{
  bind(dataStore, scenario);
}

void ScenarioDataStoreAdapter::bind(simData::DataStore* dataStore, ScenarioManager* scenario)
{
  if (dataStore == NULL)
  {
    unbind(dataStore);
  }
  else if (scenario)
  {
    // first ensure that this datastore isn't already bound
    if (listeners_.find(dataStore) == listeners_.end())
    {
      // set up notifications so we can react to data store actions:
      // the listener allows us to receive multiple notifications with a single object
      simData::DataStore::ListenerPtr listener(new MyListener(scenario));
      dataStore->addListener(listener);
      listeners_[dataStore]= listener;

      // find any data already in the data store and activate it:
      simData::DataStore::IdList ids;
      dataStore->idList(&ids);
      for (simData::DataStore::IdList::const_iterator p = ids.begin(); p != ids.end(); ++p)
      {
        listener->onAddEntity(dataStore, *p, dataStore->objectType(*p));
        listener->onPrefsChange(dataStore, *p);
      }

      // force a complete update of all entities
      scenario->update(dataStore, true);

      // If the datastore has a bound clock, listen for clock changes.
      if (dataStore->getBoundClock())
      {
          simCore::Clock::ModeChangeObserverPtr callback(
              new MyClockModeChangeObserver(scenario, dataStore->getBoundClock()));

          dataStore->getBoundClock()->registerModeChangeCallback(callback);

          // immediately notify.
          scenario->notifyOfClockChange(dataStore->getBoundClock());
      }
    }
  }
}

void ScenarioDataStoreAdapter::unbind(simData::DataStore* dataStore)
{
  if (dataStore != NULL)
  {
    // remove data store listeners
    std::map<simData::DataStore*, simData::DataStore::ListenerPtr>::iterator li = listeners_.find(dataStore);
    if (li != listeners_.end())
    {
      dataStore->removeListener(li->second);
      listeners_.erase(li);
    }
  }
}

void ScenarioDataStoreAdapter::getBindings(std::set<simData::DataStore*>& output) const
{
  output.clear();

  for (std::map<simData::DataStore*, simData::DataStore::ListenerPtr>::const_iterator i = listeners_.begin();
      i != listeners_.end();
      ++i)
  {
    output.insert(i->first);
  }
}
