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
#ifndef SIMDATA_DATASTOREPROXY_H
#define SIMDATA_DATASTOREPROXY_H

#include <vector>
#include <map>
#include "DataStore.h"

namespace simData
{

/** @brief Proxy for DataStores
 *
 *  The subject of this proxy is the DataStore class. This proxy helps in
 *  managing DataStore deletion by managing the observers, listeners, and
 *  interpolators to ensure all these elements are maintained during
 *  deletion and re-creation of a data store.
 *
 *  The proxy forwards all DataStore functions to DataStore subject inside
 *  the proxy without any extra actions with the exception of the functions
 *  used to add/remove observers, listeners, and interpolators. In these
 *  functions, the proxy first adds/removes the observers, listeners, and
 *  interpolators to its own vectors/maps before forwarding the call to the
 *  subject.
 *
 */
class SDKDATA_EXPORT DataStoreProxy : public DataStore
{
public: // methods
  /** Constructor for the proxy with a pointer to the subject passed in
   *
   * @note ownership of dataStore is given to the proxy
   */
  DataStoreProxy(simData::DataStore* dataStore);

  /// Returns a pointer to the RealSubject
  const DataStore* dataStore() const {return dataStore_;}

  /** Deletes the current dataStore_ and sets the new one to dataStore.
   *
   * @note ownership of dataStore is given to the proxy
   */
  void reset(DataStore* dateStore);

  virtual ~DataStoreProxy();

  /// allocate a new memento for this internals (caller deletes)
  virtual InternalsMemento* createInternalsMemento() const override;

  /// update all data slices to reflect current 'time'
  virtual void update(double time) override {dataStore_->update(time);}

  /// returns the last value sent to update(double), relative to current reference year
  virtual double updateTime() const override {return dataStore_->updateTime();}

  /// data store reference year (without transaction cost); intended to be cached locally for performance.
  virtual int referenceYear() const override {return dataStore_->referenceYear();}

  /// set data limiting in the data store
  virtual void setDataLimiting(bool dataLimiting) override {dataStore_->setDataLimiting(dataLimiting);}

  /// returns flag indicating if data limiting is set
  virtual bool dataLimiting() const override {return dataStore_->dataLimiting();}

  /// store a reference to current clock, for time/data mode
  virtual void bindToClock(simCore::Clock* clock) override;

  /// fetches a currently bound clock
  virtual simCore::Clock* getBoundClock() const override;

  /**
   * flush all the updates, command, category data and generic data for the specified id,
   * if 0 is passed in flushes the entire scenario, except for static entities
   */
  [[deprecated("Use flush(ObjectId, FlushScope, FlushFields) instead.")]]
  virtual void flush(ObjectId flushId, FlushType flushType = NON_RECURSIVE) override;

  /** Removes all the specified data */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields) override {return dataStore_->flush(id, scope, fields);}

  /** Removes a range of data from startTime up to but not including the endTime */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields, double startTime, double endTime) override {return dataStore_->flush(id, scope, fields, startTime, endTime);}

  /** Clear out the data store of all scenario specific data, including all entities and category data names. */
  virtual void clear() override {dataStore_->clear();}

  /**@name Interpolation
   *@{
   */
  /// implementation supports interpolation for updates
  virtual bool canInterpolate() const override {return dataStore_->canInterpolate();}

  /** Enable or disable interpolation, if supported
   *
   * @note (Will only succeed if implementation supports interpolation
   * and contains a valid interpolator object)
   * @return Flag indicating isInterpolationEnabled()
   */
  virtual bool enableInterpolation(bool state) override {return dataStore_->enableInterpolation(state);}

  /// interpolation is enabled
  virtual bool isInterpolationEnabled() const override {return dataStore_->isInterpolationEnabled();}

  /// Specify the interpolator to use
  virtual void setInterpolator(Interpolator *interpolator) override;

  /// Get the current interpolator (nullptr if disabled)
  virtual Interpolator* interpolator() const override;
  ///@}

  /// Sets the interpolation state; returns true if interpolation is enabled
  virtual bool enableInterpolation(InterpolatorState state) override;

  /// Returns the interpolation state
  virtual InterpolatorState interpolatorState() const override;

  /**@name ID Lists
   * @{
   */
   /// Retrieves the number of objects of 'type'
  virtual size_t idCount(simData::ObjectType type = simData::ALL) const override { return dataStore_->idCount(type); }

  /// Retrieve a list of IDs for objects of 'type'
  virtual void idList(IdList* ids, simData::ObjectType type = simData::ALL) const override { dataStore_->idList(ids, type); }

  /// Retrieve a list of IDs for objects of 'type' with the given name
  virtual void idListByName(const std::string& name, IdList* ids, simData::ObjectType type = simData::ALL) const override {dataStore_->idListByName(name, ids, type);}

  /// Retrieve a list of IDs for objects with the given original id
  virtual void idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type = simData::ALL) const override {dataStore_->idListByOriginalId(ids, originalId, type);}

  /// Retrieve a list of IDs for all beams associated with a platform
  virtual void beamIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->beamIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all gates associated with a beam
  virtual void gateIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->gateIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all lasers associated with a platform
  virtual void laserIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->laserIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all projectors associated with a platform
  virtual void projectorIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->projectorIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all LobGroups associated with a platform
  virtual void lobGroupIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->lobGroupIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all custom renderings associated with a platform
  virtual void customRenderingIdListForHost(ObjectId hostid, IdList *ids) const override {dataStore_->customRenderingIdListForHost(hostid, ids);}

  /// Retrieves the ObjectType for a particular ID
  virtual simData::ObjectType objectType(ObjectId id) const override {return dataStore_->objectType(id);}

  /// Retrieves the host entity ID for a particular ID (i.e. a beam, given a gate ID; a platform, given a LOB ID)
  virtual ObjectId entityHostId(ObjectId childId) const override {return dataStore_->entityHostId(childId);}

  /// Retrieves the time bounds for a particular entity ID (first point, last point)
  virtual std::pair<double, double> timeBounds(ObjectId entityId) const override {return dataStore_->timeBounds(entityId);}
  ///@}

  /**@name Scenario Properties
   * @note should always return a valid object (never nullptr)
   * @{
   */
  virtual const  ScenarioProperties*          scenarioProperties(Transaction *transaction) const override {return dataStore_->scenarioProperties(transaction);}
  virtual        ScenarioProperties*  mutable_scenarioProperties(Transaction *transaction) override {return dataStore_->mutable_scenarioProperties(transaction);}

  /**@name Object Properties
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformProperties*          platformProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->platformProperties(id, transaction);}
  virtual const      BeamProperties*              beamProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->beamProperties(id, transaction);}
  virtual const      GateProperties*              gateProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->gateProperties(id, transaction);}
  virtual const     LaserProperties*             laserProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->laserProperties(id, transaction);}
  virtual const ProjectorProperties*         projectorProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->projectorProperties(id, transaction);}
  virtual const  LobGroupProperties*          lobGroupProperties(ObjectId id, Transaction *transaction) const override {return dataStore_->lobGroupProperties(id, transaction);}
  virtual const CustomRenderingProperties* customRenderingProperties(ObjectId id, Transaction *transaction) const override { return dataStore_->customRenderingProperties(id, transaction); }
  virtual        PlatformProperties*  mutable_platformProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_platformProperties(id, transaction);}
  virtual            BeamProperties*      mutable_beamProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_beamProperties(id, transaction);}
  virtual            GateProperties*      mutable_gateProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_gateProperties(id, transaction);}
  virtual           LaserProperties*     mutable_laserProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_laserProperties(id, transaction);}
  virtual       ProjectorProperties* mutable_projectorProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_projectorProperties(id, transaction);}
  virtual        LobGroupProperties*  mutable_lobGroupProperties(ObjectId id, Transaction *transaction) override {return dataStore_->mutable_lobGroupProperties(id, transaction);}
  virtual CustomRenderingProperties* mutable_customRenderingProperties(ObjectId id, Transaction *transaction) override { return dataStore_->mutable_customRenderingProperties(id, transaction); }
  ///@}

  /**@name Object Preferences
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformPrefs*          platformPrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->platformPrefs(id, transaction);}
  virtual const      BeamPrefs*              beamPrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->beamPrefs(id, transaction);}
  virtual const      GatePrefs*              gatePrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->gatePrefs(id, transaction);}
  virtual const     LaserPrefs*             laserPrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->laserPrefs(id, transaction);}
  virtual const ProjectorPrefs*         projectorPrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->projectorPrefs(id, transaction);}
  virtual const  LobGroupPrefs*          lobGroupPrefs(ObjectId id, Transaction *transaction) const override {return dataStore_->lobGroupPrefs(id, transaction);}
  virtual const    CommonPrefs*            commonPrefs(ObjectId id, Transaction* transaction) const override {return dataStore_->commonPrefs(id, transaction);}

  /**
   * The mutable_* routines have two modes of operation, one for external users and one for internal users.  External users should always set the results argument
   * to nullptr.  Since this interface is for external users, the results argument is ignored and always treated as nullptr.
   */
  virtual        PlatformPrefs*  mutable_platformPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override {return dataStore_->mutable_platformPrefs(id, transaction, nullptr);}
  virtual            BeamPrefs*      mutable_beamPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override {return dataStore_->mutable_beamPrefs(id, transaction, nullptr);}
  virtual            GatePrefs*      mutable_gatePrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override {return dataStore_->mutable_gatePrefs(id, transaction, nullptr);}
  virtual           LaserPrefs*     mutable_laserPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override {return dataStore_->mutable_laserPrefs(id, transaction, nullptr);}
  virtual       ProjectorPrefs* mutable_projectorPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override {return dataStore_->mutable_projectorPrefs(id, transaction, nullptr);}
  virtual        LobGroupPrefs*  mutable_lobGroupPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override { return dataStore_->mutable_lobGroupPrefs(id, transaction, nullptr); }
  virtual const CustomRenderingPrefs* customRenderingPrefs(ObjectId id, Transaction *transaction) const override { return dataStore_->customRenderingPrefs(id, transaction); }
  virtual       CustomRenderingPrefs* mutable_customRenderingPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override { return dataStore_->mutable_customRenderingPrefs(id, transaction, nullptr); }
  virtual          CommonPrefs*    mutable_commonPrefs(ObjectId id, Transaction* transaction) override { return dataStore_->mutable_commonPrefs(id, transaction); }
  ///@}

 /**@name Set default pref values
   * @note provide default values for each pref type
   * @{
   */
  virtual void setDefaultPrefs(
    const PlatformPrefs& platformPrefs,
    const BeamPrefs& beamPrefs,
    const GatePrefs& gatePrefs,
    const LaserPrefs& laserPrefs,
    const LobGroupPrefs& lobPrefs,
    const ProjectorPrefs& projectorPrefs)
  {
    dataStore_->setDefaultPrefs(platformPrefs, beamPrefs, gatePrefs, laserPrefs, lobPrefs, projectorPrefs);
  }

  /** @see simData::DataStore::setDefaultPrefs() */
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs) override { dataStore_->setDefaultPrefs(platformPrefs); }

  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual PlatformPrefs defaultPlatformPrefs() const override { return dataStore_->defaultPlatformPrefs(); }

  ///@}

  /**@name Add a platform, beam, gate, or laser
   * @note Returns properties object to be initialized.
   *  A unique id is generated internally and should not be changed.
   *  The original id field should be used for any user generated ids.
   * @{
   */
  virtual  PlatformProperties* addPlatform(Transaction *transaction) override {return dataStore_->addPlatform(transaction);}
  virtual      BeamProperties* addBeam(Transaction *transaction) override {return dataStore_->addBeam(transaction);}
  virtual      GateProperties* addGate(Transaction *transaction) override {return dataStore_->addGate(transaction);}
  virtual     LaserProperties* addLaser(Transaction *transaction) override {return dataStore_->addLaser(transaction);}
  virtual ProjectorProperties* addProjector(Transaction *transaction) override {return dataStore_->addProjector(transaction);}
  virtual  LobGroupProperties* addLobGroup(Transaction *transaction) override {return dataStore_->addLobGroup(transaction);}
  virtual CustomRenderingProperties* addCustomRendering(Transaction *transaction) override {return dataStore_->addCustomRendering(transaction);}
  ///@}

  /// remove an entity from the data store
  virtual void removeEntity(ObjectId id) override {dataStore_->removeEntity(id);}

  /**
   * remove a category data point
   * @param id entity the data is associated with
   * @param time timestamp the data was originally given
   * @param catNameInt integer id of the category name string
   * @param valueInt integer id of the category value string
   * @return 0 if a point was actually removed
   */
  virtual int removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt) override { return dataStore_->removeCategoryDataPoint(id, time, catNameInt, valueInt); }

  /// @copydoc simData::DataStore::removeGenericDataTag
  virtual int removeGenericDataTag(ObjectId id, const std::string& tag) override { return dataStore_->removeGenericDataTag(id, tag); }

  /**@name Add data update, command, generic data, or category data
   *@note Returns nullptr if platform for specified ID does not exist
   * @{
   */
  virtual  PlatformUpdate *   addPlatformUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addPlatformUpdate(id, transaction);}
  virtual      BeamUpdate *   addBeamUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addBeamUpdate(id, transaction);}
  virtual      BeamCommand*   addBeamCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addBeamCommand(id, transaction);}
  virtual      GateUpdate *   addGateUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addGateUpdate(id, transaction);}
  virtual      GateCommand*   addGateCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addGateCommand(id, transaction);}
  virtual     LaserUpdate *   addLaserUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addLaserUpdate(id, transaction);}
  virtual     LaserCommand*   addLaserCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addLaserCommand(id, transaction);}
  virtual  PlatformCommand*   addPlatformCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addPlatformCommand(id, transaction);}
  virtual  ProjectorUpdate*   addProjectorUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addProjectorUpdate(id, transaction);}
  virtual ProjectorCommand*   addProjectorCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addProjectorCommand(id, transaction);}
  virtual   LobGroupUpdate*   addLobGroupUpdate(ObjectId id, Transaction *transaction) override {return dataStore_->addLobGroupUpdate(id, transaction);}
  virtual  LobGroupCommand*   addLobGroupCommand(ObjectId id, Transaction *transaction) override {return dataStore_->addLobGroupCommand(id, transaction);}
  virtual CustomRenderingCommand* addCustomRenderingCommand(ObjectId id, Transaction *transaction) override { return dataStore_->addCustomRenderingCommand(id, transaction); }
  virtual      GenericData*   addGenericData(ObjectId id, Transaction *transaction) override {return dataStore_->addGenericData(id, transaction);}
  virtual     CategoryData*   addCategoryData(ObjectId id, Transaction *transaction) override {return dataStore_->addCategoryData(id, transaction);}
  ///@}

  /**@name Retrieving read-only data slices
   * @note No locking performed for read-only update slice objects
   * @{
   */
  virtual const PlatformUpdateSlice*   platformUpdateSlice(ObjectId id) const override {return dataStore_->platformUpdateSlice(id);}
  virtual const PlatformCommandSlice*  platformCommandSlice(ObjectId id) const override {return dataStore_->platformCommandSlice(id);}
  virtual const BeamUpdateSlice*       beamUpdateSlice(ObjectId id) const override {return dataStore_->beamUpdateSlice(id);}
  virtual const BeamCommandSlice*      beamCommandSlice(ObjectId id) const override {return dataStore_->beamCommandSlice(id);}
  virtual const GateUpdateSlice*       gateUpdateSlice(ObjectId id) const override {return dataStore_->gateUpdateSlice(id);}
  virtual const GateCommandSlice*      gateCommandSlice(ObjectId id) const override {return dataStore_->gateCommandSlice(id);}
  virtual const LaserUpdateSlice*      laserUpdateSlice(ObjectId id) const override {return dataStore_->laserUpdateSlice(id);}
  virtual const LaserCommandSlice*     laserCommandSlice(ObjectId id) const override {return dataStore_->laserCommandSlice(id);}
  virtual const ProjectorUpdateSlice*  projectorUpdateSlice(ObjectId id) const override {return dataStore_->projectorUpdateSlice(id);}
  virtual const ProjectorCommandSlice* projectorCommandSlice(ObjectId id) const override {return dataStore_->projectorCommandSlice(id);}
  virtual const LobGroupUpdateSlice*   lobGroupUpdateSlice(ObjectId id) const override {return dataStore_->lobGroupUpdateSlice(id);}
  virtual const LobGroupCommandSlice*  lobGroupCommandSlice(ObjectId id) const override {return dataStore_->lobGroupCommandSlice(id);}
  virtual const CustomRenderingCommandSlice* customRenderingCommandSlice(ObjectId id) const override { return dataStore_->customRenderingCommandSlice(id); }
  virtual const GenericDataSlice*      genericDataSlice(ObjectId id) const override {return dataStore_->genericDataSlice(id);}
  virtual const CategoryDataSlice*     categoryDataSlice(ObjectId id) const override {return dataStore_->categoryDataSlice(id);}
  ///@}

  /// @copydoc simData::DataStore::installSliceTimeRangeMonitor
  virtual void installSliceTimeRangeMonitor(ObjectId id, std::function<void(double startTime, double endTime)> fn) override { dataStore_->installSliceTimeRangeMonitor(id, fn); }

  /// @copydoc simData::DataStore::modifyPlatformCommandSlice
  virtual int modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier) override { return dataStore_->modifyPlatformCommandSlice(id, modifier); }

  /// @copydoc simData::DataStore::modifyProjectorCommandSlice
  virtual int modifyProjectorCommandSlice(ObjectId id, VisitableDataSlice<ProjectorCommand>::Modifier* modifier) override { return dataStore_->modifyProjectorCommandSlice(id, modifier); }

  /// @copydoc simData::DataStore::modifyCustomRenderingCommandSlice
  virtual int modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier) override { return dataStore_->modifyCustomRenderingCommandSlice(id, modifier); }

  /**@name Listeners
   * @{
   */
  /// Add or remove a listener for event messages
  virtual void addListener(DataStore::ListenerPtr callback) override;
  virtual void removeListener(DataStore::ListenerPtr callback) override;
  ///@}

  /**@name ScenarioListeners
   * @{
   */
  /// Add or remove a listener for scenario event messages
  virtual void addScenarioListener(DataStore::ScenarioListenerPtr callback) override;
  virtual void removeScenarioListener(DataStore::ScenarioListenerPtr callback) override;
  ///@}

  /**@name NewUpdatesListener
  * @{
  */
  /// Add or remove a listener for when entity updates are added
  virtual void addNewUpdatesListener(NewUpdatesListenerPtr callback) override;
  virtual void removeNewUpdatesListener(NewUpdatesListenerPtr callback) override;
  ///@}

  /**@name Get a handle to the CategoryNameManager
   * @{
   */
  virtual CategoryNameManager& categoryNameManager() const override { return dataStore_->categoryNameManager(); }
  ///@}

  /**
   * Retrieves a reference to the data table manager associated with this data store.
   * The data table manager can be used to create data tables associated with entities,
   * iterate through tables, and add data to existing tables.
   * @return Reference to the data table manager.
   */
  virtual DataTableManager& dataTableManager() const override { return dataStore_->dataTableManager(); }

protected: // data
  DataStore* dataStore_;                      ///< Pointer to the actual DataStore class
  Interpolator *interpolator_;                ///< Stores the interpolator

}; // End of DataStoreProxy

} // End of namespace simData

#endif // SIMDATA_DATASTOREPROXY_H

