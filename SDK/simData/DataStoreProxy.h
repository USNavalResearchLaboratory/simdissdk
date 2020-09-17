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
  virtual InternalsMemento* createInternalsMemento() const;

  /// update all data slices to reflect current 'time'
  virtual void update(double time) {dataStore_->update(time);}

  /// returns the last value sent to update(double), relative to current reference year
  virtual double updateTime() const {return dataStore_->updateTime();}

  /// data store reference year (without transaction cost); intended to be cached locally for performance.
  virtual int referenceYear() const {return dataStore_->referenceYear();}

  /// set data limiting in the data store
  virtual void setDataLimiting(bool dataLimiting) {dataStore_->setDataLimiting(dataLimiting);}

  /// returns flag indicating if data limiting is set
  virtual bool dataLimiting() const {return dataStore_->dataLimiting();}

  /// store a reference to current clock, for time/data mode
  virtual void bindToClock(simCore::Clock* clock);

  /// fetches a currently bound clock
  virtual simCore::Clock* getBoundClock() const;

  /**
   * flush all the updates, command, category data and generic data for the specified id,
   * if 0 is passed in flushes the entire scenario, except for static entities
   */
  virtual void flush(ObjectId flushId, FlushType type = NON_RECURSIVE) {dataStore_->flush(flushId, type);}

  /**
  * clear out the data store of all scenario specific data, including all entities and category data names.
  */
  virtual void clear() {dataStore_->clear();}

  /**@name Interpolation
   *@{
   */
  /// implementation supports interpolation for updates
  virtual bool canInterpolate() const {return dataStore_->canInterpolate();}

  /** Enable or disable interpolation, if supported
   *
   * @note (Will only succeed if implementation supports interpolation
   * and contains a valid interpolator object)
   * @return Flag indicating isInterpolationEnabled()
   */
  virtual bool enableInterpolation(bool state) {return dataStore_->enableInterpolation(state);}

  /// interpolation is enabled
  virtual bool isInterpolationEnabled() const {return dataStore_->isInterpolationEnabled();}

  /// Specify the interpolator to use
  virtual void setInterpolator(Interpolator *interpolator);

  /// Get the current interpolator (nullptr if disabled)
  virtual Interpolator* interpolator() const;
  ///@}

  /**@name ID Lists
   * @{
   */
  /// Retrieve a list of IDs for objects of 'type'
  virtual void idList(IdList *ids, simData::ObjectType type = simData::ALL) const {dataStore_->idList(ids, type);}

  /// Retrieve a list of IDs for objects of 'type' with the given name
  virtual void idListByName(const std::string& name, IdList* ids, simData::ObjectType type = simData::ALL) const {dataStore_->idListByName(name, ids, type);}

  /// Retrieve a list of IDs for objects with the given original id
  virtual void idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type = simData::ALL) const {dataStore_->idListByOriginalId(ids, originalId, type);}

  /// Retrieve a list of IDs for all beams associated with a platform
  virtual void beamIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->beamIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all gates associated with a beam
  virtual void gateIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->gateIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all lasers associated with a platform
  virtual void laserIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->laserIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all projectors associated with a platform
  virtual void projectorIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->projectorIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all LobGroups associated with a platform
  virtual void lobGroupIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->lobGroupIdListForHost(hostid, ids);}

  /// Retrieve a list of IDs for all custom renderings associated with a platform
  virtual void customRenderingIdListForHost(ObjectId hostid, IdList *ids) const {dataStore_->customRenderingIdListForHost(hostid, ids);}

  /// Retrieves the ObjectType for a particular ID
  virtual simData::ObjectType objectType(ObjectId id) const {return dataStore_->objectType(id);}

  /// Retrieves the host entity ID for a particular ID (i.e. a beam, given a gate ID; a platform, given a LOB ID)
  virtual ObjectId entityHostId(ObjectId childId) const {return dataStore_->entityHostId(childId);}

  /// Retrieves the time bounds for a particular entity ID (first point, last point)
  virtual std::pair<double, double> timeBounds(ObjectId entityId) const {return dataStore_->timeBounds(entityId);}
  ///@}

  /**@name Scenario Properties
   * @note should always return a valid object (never nullptr)
   * @{
   */
  virtual const  ScenarioProperties*          scenarioProperties(Transaction *transaction) const {return dataStore_->scenarioProperties(transaction);}
  virtual        ScenarioProperties*  mutable_scenarioProperties(Transaction *transaction) {return dataStore_->mutable_scenarioProperties(transaction);}

  /**@name Object Properties
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformProperties*          platformProperties(ObjectId id, Transaction *transaction) const {return dataStore_->platformProperties(id, transaction);}
  virtual const      BeamProperties*              beamProperties(ObjectId id, Transaction *transaction) const {return dataStore_->beamProperties(id, transaction);}
  virtual const      GateProperties*              gateProperties(ObjectId id, Transaction *transaction) const {return dataStore_->gateProperties(id, transaction);}
  virtual const     LaserProperties*             laserProperties(ObjectId id, Transaction *transaction) const {return dataStore_->laserProperties(id, transaction);}
  virtual const ProjectorProperties*         projectorProperties(ObjectId id, Transaction *transaction) const {return dataStore_->projectorProperties(id, transaction);}
  virtual const  LobGroupProperties*          lobGroupProperties(ObjectId id, Transaction *transaction) const {return dataStore_->lobGroupProperties(id, transaction);}
  virtual const CustomRenderingProperties* customRenderingProperties(ObjectId id, Transaction *transaction) const { return dataStore_->customRenderingProperties(id, transaction); }
  virtual        PlatformProperties*  mutable_platformProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_platformProperties(id, transaction);}
  virtual            BeamProperties*      mutable_beamProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_beamProperties(id, transaction);}
  virtual            GateProperties*      mutable_gateProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_gateProperties(id, transaction);}
  virtual           LaserProperties*     mutable_laserProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_laserProperties(id, transaction);}
  virtual       ProjectorProperties* mutable_projectorProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_projectorProperties(id, transaction);}
  virtual        LobGroupProperties*  mutable_lobGroupProperties(ObjectId id, Transaction *transaction) {return dataStore_->mutable_lobGroupProperties(id, transaction);}
  virtual CustomRenderingProperties* mutable_customRenderingProperties(ObjectId id, Transaction *transaction) { return dataStore_->mutable_customRenderingProperties(id, transaction); }
  ///@}

  /**@name Object Preferences
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformPrefs*          platformPrefs(ObjectId id, Transaction *transaction) const {return dataStore_->platformPrefs(id, transaction);}
  virtual const      BeamPrefs*              beamPrefs(ObjectId id, Transaction *transaction) const {return dataStore_->beamPrefs(id, transaction);}
  virtual const      GatePrefs*              gatePrefs(ObjectId id, Transaction *transaction) const {return dataStore_->gatePrefs(id, transaction);}
  virtual const     LaserPrefs*             laserPrefs(ObjectId id, Transaction *transaction) const {return dataStore_->laserPrefs(id, transaction);}
  virtual const ProjectorPrefs*         projectorPrefs(ObjectId id, Transaction *transaction) const {return dataStore_->projectorPrefs(id, transaction);}
  virtual const  LobGroupPrefs*          lobGroupPrefs(ObjectId id, Transaction *transaction) const {return dataStore_->lobGroupPrefs(id, transaction);}
  virtual const    CommonPrefs*            commonPrefs(ObjectId id, Transaction* transaction) const {return dataStore_->commonPrefs(id, transaction);}
  virtual        PlatformPrefs*  mutable_platformPrefs(ObjectId id, Transaction *transaction) {return dataStore_->mutable_platformPrefs(id, transaction);}
  virtual            BeamPrefs*      mutable_beamPrefs(ObjectId id, Transaction *transaction) {return dataStore_->mutable_beamPrefs(id, transaction);}
  virtual            GatePrefs*      mutable_gatePrefs(ObjectId id, Transaction *transaction) {return dataStore_->mutable_gatePrefs(id, transaction);}
  virtual           LaserPrefs*     mutable_laserPrefs(ObjectId id, Transaction *transaction) {return dataStore_->mutable_laserPrefs(id, transaction);}
  virtual       ProjectorPrefs* mutable_projectorPrefs(ObjectId id, Transaction *transaction) {return dataStore_->mutable_projectorPrefs(id, transaction);}
  virtual        LobGroupPrefs*  mutable_lobGroupPrefs(ObjectId id, Transaction *transaction) { return dataStore_->mutable_lobGroupPrefs(id, transaction); }
  virtual const CustomRenderingPrefs* customRenderingPrefs(ObjectId id, Transaction *transaction) const { return dataStore_->customRenderingPrefs(id, transaction); }
  virtual       CustomRenderingPrefs* mutable_customRenderingPrefs(ObjectId id, Transaction *transaction) { return dataStore_->mutable_customRenderingPrefs(id, transaction); }
  virtual          CommonPrefs*    mutable_commonPrefs(ObjectId id, Transaction* transaction) { return dataStore_->mutable_commonPrefs(id, transaction); }
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
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs) { dataStore_->setDefaultPrefs(platformPrefs); }

  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual PlatformPrefs defaultPlatformPrefs() const { return dataStore_->defaultPlatformPrefs(); }

  ///@}

  /**@name Add a platform, beam, gate, or laser
   * @note Returns properties object to be initialized.
   *  A unique id is generated internally and should not be changed.
   *  The original id field should be used for any user generated ids.
   * @{
   */
  virtual  PlatformProperties* addPlatform(Transaction *transaction) {return dataStore_->addPlatform(transaction);}
  virtual      BeamProperties* addBeam(Transaction *transaction) {return dataStore_->addBeam(transaction);}
  virtual      GateProperties* addGate(Transaction *transaction) {return dataStore_->addGate(transaction);}
  virtual     LaserProperties* addLaser(Transaction *transaction) {return dataStore_->addLaser(transaction);}
  virtual ProjectorProperties* addProjector(Transaction *transaction) {return dataStore_->addProjector(transaction);}
  virtual  LobGroupProperties* addLobGroup(Transaction *transaction) {return dataStore_->addLobGroup(transaction);}
  virtual CustomRenderingProperties* addCustomRendering(Transaction *transaction) {return dataStore_->addCustomRendering(transaction);}
  ///@}

  /// remove an entity from the data store
  virtual void removeEntity(ObjectId id) {dataStore_->removeEntity(id);}

  /**
   * remove a category data point
   * @param id entity the data is associated with
   * @param time timestamp the data was originally given
   * @param catNameInt integer id of the category name string
   * @param valueInt integer id of the category value string
   * @return 0 if a point was actually removed
   */
  virtual int removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt) { return dataStore_->removeCategoryDataPoint(id, time, catNameInt, valueInt); }

  /// @copydoc simData::DataStore::removeGenericDataTag
  virtual int removeGenericDataTag(ObjectId id, const std::string& tag) { return dataStore_->removeGenericDataTag(id, tag); }

  /**@name Add data update, command, generic data, or category data
   *@note Returns nullptr if platform for specified ID does not exist
   * @{
   */
  virtual  PlatformUpdate *   addPlatformUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addPlatformUpdate(id, transaction);}
  virtual      BeamUpdate *   addBeamUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addBeamUpdate(id, transaction);}
  virtual      BeamCommand*   addBeamCommand(ObjectId id, Transaction *transaction) {return dataStore_->addBeamCommand(id, transaction);}
  virtual      GateUpdate *   addGateUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addGateUpdate(id, transaction);}
  virtual      GateCommand*   addGateCommand(ObjectId id, Transaction *transaction) {return dataStore_->addGateCommand(id, transaction);}
  virtual     LaserUpdate *   addLaserUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addLaserUpdate(id, transaction);}
  virtual     LaserCommand*   addLaserCommand(ObjectId id, Transaction *transaction) {return dataStore_->addLaserCommand(id, transaction);}
  virtual  PlatformCommand*   addPlatformCommand(ObjectId id, Transaction *transaction) {return dataStore_->addPlatformCommand(id, transaction);}
  virtual  ProjectorUpdate*   addProjectorUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addProjectorUpdate(id, transaction);}
  virtual ProjectorCommand*   addProjectorCommand(ObjectId id, Transaction *transaction) {return dataStore_->addProjectorCommand(id, transaction);}
  virtual   LobGroupUpdate*   addLobGroupUpdate(ObjectId id, Transaction *transaction) {return dataStore_->addLobGroupUpdate(id, transaction);}
  virtual  LobGroupCommand*   addLobGroupCommand(ObjectId id, Transaction *transaction) {return dataStore_->addLobGroupCommand(id, transaction);}
  virtual CustomRenderingCommand* addCustomRenderingCommand(ObjectId id, Transaction *transaction) { return dataStore_->addCustomRenderingCommand(id, transaction); }
  virtual      GenericData*   addGenericData(ObjectId id, Transaction *transaction) {return dataStore_->addGenericData(id, transaction);}
  virtual     CategoryData*   addCategoryData(ObjectId id, Transaction *transaction) {return dataStore_->addCategoryData(id, transaction);}
  //virtual        TableData*        addTableData(ObjectId id, Transaction *transaction) = 0;
  ///@}

  /**@name Retrieving read-only data slices
   * @note No locking performed for read-only update slice objects
   * @{
   */
  virtual const PlatformUpdateSlice*   platformUpdateSlice(ObjectId id) const {return dataStore_->platformUpdateSlice(id);}
  virtual const PlatformCommandSlice*  platformCommandSlice(ObjectId id) const {return dataStore_->platformCommandSlice(id);}
  virtual const BeamUpdateSlice*       beamUpdateSlice(ObjectId id) const {return dataStore_->beamUpdateSlice(id);}
  virtual const BeamCommandSlice*      beamCommandSlice(ObjectId id) const {return dataStore_->beamCommandSlice(id);}
  virtual const GateUpdateSlice*       gateUpdateSlice(ObjectId id) const {return dataStore_->gateUpdateSlice(id);}
  virtual const GateCommandSlice*      gateCommandSlice(ObjectId id) const {return dataStore_->gateCommandSlice(id);}
  virtual const LaserUpdateSlice*      laserUpdateSlice(ObjectId id) const {return dataStore_->laserUpdateSlice(id);}
  virtual const LaserCommandSlice*     laserCommandSlice(ObjectId id) const {return dataStore_->laserCommandSlice(id);}
  virtual const ProjectorUpdateSlice*  projectorUpdateSlice(ObjectId id) const {return dataStore_->projectorUpdateSlice(id);}
  virtual const ProjectorCommandSlice* projectorCommandSlice(ObjectId id) const {return dataStore_->projectorCommandSlice(id);}
  virtual const LobGroupUpdateSlice*   lobGroupUpdateSlice(ObjectId id) const {return dataStore_->lobGroupUpdateSlice(id);}
  virtual const LobGroupCommandSlice*  lobGroupCommandSlice(ObjectId id) const {return dataStore_->lobGroupCommandSlice(id);}
  virtual const CustomRenderingCommandSlice* customRenderingCommandSlice(ObjectId id) const { return dataStore_->customRenderingCommandSlice(id); }
  virtual const GenericDataSlice*      genericDataSlice(ObjectId id) const {return dataStore_->genericDataSlice(id);}
  virtual const CategoryDataSlice*     categoryDataSlice(ObjectId id) const {return dataStore_->categoryDataSlice(id);}
  ///@}

  /// @copydoc simData::DataStore::modifyPlatformCommandSlice
  virtual int modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier) { return dataStore_->modifyPlatformCommandSlice(id, modifier); }

  /// @copydoc simData::DataStore::modifyCustomRenderingCommandSlice
  virtual int modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier) { return dataStore_->modifyCustomRenderingCommandSlice(id, modifier); }

  /**@name Listeners
   * @{
   */
  /// Add or remove a listener for event messages
  virtual void addListener(DataStore::ListenerPtr callback);
  virtual void removeListener(DataStore::ListenerPtr callback);
  ///@}

  /**@name ScenarioListeners
   * @{
   */
  /// Add or remove a listener for scenario event messages
  virtual void addScenarioListener(DataStore::ScenarioListenerPtr callback);
  virtual void removeScenarioListener(DataStore::ScenarioListenerPtr callback);
  ///@}

  /**@name NewUpdatesListener
  * @{
  */
  /// Sets a listener for when entity updates are added; use nullptr to remove.
  virtual void setNewUpdatesListener(NewUpdatesListenerPtr callback);
  ///@}

  /**@name Get a handle to the CategoryNameManager
   * @{
   */
  virtual CategoryNameManager& categoryNameManager() const { return dataStore_->categoryNameManager(); }
  ///@}

  /**
   * Retrieves a reference to the data table manager associated with this data store.
   * The data table manager can be used to create data tables associated with entities,
   * iterate through tables, and add data to existing tables.
   * @return Reference to the data table manager.
   */
  virtual DataTableManager& dataTableManager() const { return dataStore_->dataTableManager(); }

protected: // data
  DataStore* dataStore_;                      ///< Pointer to the actual DataStore class
  Interpolator *interpolator_;                ///< Stores the interpolator

}; // End of DataStoreProxy

} // End of namespace simData

#endif // SIMDATA_DATASTOREPROXY_H

