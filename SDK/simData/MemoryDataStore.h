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
#ifndef SIMDATA_MEMORYDATASTORE_H
#define SIMDATA_MEMORYDATASTORE_H

#include <map>
#include <string>
#include "simData/MemoryDataEntry.h"
#include "simData/DataStore.h"

namespace simCore { class Clock; }

namespace simData {

class EntityNameCache;
class GenericDataSlice;
class MemoryCategoryDataSlice;
class NewRowDataToNewUpdatesAdapter;
namespace MemoryTable { class DataLimitsProvider; }

/** @brief Implementation of DataStore using plain memory
 *
 * MemoryDataStore is an implementation of the DataStore interface that stores
 * its information within traditional data structures resident in RAM.
 */
class SDKDATA_EXPORT MemoryDataStore : public DataStore
{
public:
  MemoryDataStore();
  explicit MemoryDataStore(const ScenarioProperties &properties);

  virtual ~MemoryDataStore();

  /**@name Interface from DataStore
   * @{
   */
  /// allocate a new memento for this internals (caller deletes)
  virtual InternalsMemento* createInternalsMemento() const;

  /// update all data slices to reflect current 'time'
  virtual void update(double time);

  /// returns the last value sent to update(double), relative to current reference year
  virtual double updateTime() const;

  /// data store reference year (without transaction cost); intended to be cached locally for performance.
  virtual int referenceYear() const;

  /// store a reference to current clock, for time/data mode
  virtual void bindToClock(simCore::Clock* clock);

  /// fetches a currently bound clock
  virtual simCore::Clock* getBoundClock() const;

  /**
  * set data limiting in the data store.  This sets a flag in the
  * DataStore object, which is checked by each NewUpdateTransactionImpl object
  * after the DataSlice insert call is made, and then makes the call to
  * DataSlice::limitByPrefs, passing in the CommonPrefs record
  * @param[in] dataLimiting
  */
  virtual void setDataLimiting(bool dataLimiting);

  /// returns flag indicating if data limiting is set
  virtual bool dataLimiting() const;

  /// flush all the updates, command, category data and generic data for the specified id,
  /// if 0 is passed in flushes the entire scenario, except for static entities
  virtual void flush(ObjectId flushId, FlushType type = NON_RECURSIVE);

  /** Removes all the specified data */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields);

  /** Removes a range of data from startTime up to but not including the endTime */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields, double startTime, double endTime);

  /**
  * clear out the data store of all scenario specific data, including all entities and category data names.
  */
  virtual void clear();

  /**@name Interpolation
   *@{
   */
  /// implementation supports interpolation for updates
  virtual bool canInterpolate() const;

  /** Enable or disable interpolation, if supported
   *
   * @note (Will only succeed if implementation supports interpolation
   * and contains a valid interpolator object)
   */
  virtual bool enableInterpolation(bool state);

  /// interpolation is enabled
  virtual bool isInterpolationEnabled() const;

  /// Specify the interpolator to use
  virtual void setInterpolator(Interpolator *interpolator);

  /// Get the current interpolator (nullptr if disabled)
  virtual Interpolator* interpolator() const;
  ///@}

  /**@name ID Lists
   * @{
   */
  /// Retrieve a list of IDs for objects of 'type'
  virtual void idList(IdList *ids, simData::ObjectType type = simData::ALL) const;

  /// Retrieve a list of IDs for objects of 'type' with the given name
  virtual void idListByName(const std::string& name, IdList* ids, simData::ObjectType type = simData::ALL) const;

  /// Retrieve a list of IDs for objects with the given original id
  virtual void idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type = simData::ALL) const;

  /// Retrieve a list of IDs for all beams associated with a platform
  virtual void beamIdListForHost(ObjectId hostid, IdList *ids) const;

  /// Retrieve a list of IDs for all gates associated with a beam
  virtual void gateIdListForHost(ObjectId hostid, IdList *ids) const;

  /// Retrieve a list of IDs for all lasers associated with a platform
  virtual void laserIdListForHost(ObjectId hostid, IdList *ids) const;

  /// Retrieve a list of IDs for all projectors associated with a platform
  virtual void projectorIdListForHost(ObjectId hostid, IdList *ids) const;

  /// Retrieve a list of IDs for all lobGroups associated with a platform
  virtual void lobGroupIdListForHost(ObjectId hostid, IdList *ids) const;

  /// Retrieve a list of IDs for all customs associated with a platform
  virtual void customRenderingIdListForHost(ObjectId hostid, IdList *ids) const;

  ///Retrieves the ObjectType for a particular ID
  virtual simData::ObjectType objectType(ObjectId id) const;

  /// Retrieves the host entity ID for a particular ID (i.e. a beam, given a gate ID; a platform, given a LOB ID)
  virtual ObjectId entityHostId(ObjectId childId) const;

  /// Retrieves the time bounds for a particular entity ID (first point, last point)
  virtual std::pair<double, double> timeBounds(ObjectId entityId) const;
  /// Retrieves the scenario time bounds (first point, last point)
  virtual std::pair<double, double> timeBounds() const;
  ///@}

  /**@name Properties
   * @note should always return a valid object (never nullptr)
   * @{
   */
  virtual const ScenarioProperties *scenarioProperties(Transaction *transaction) const;
  virtual const PlatformProperties *platformProperties(ObjectId id, Transaction *transaction) const;
  virtual const BeamProperties *beamProperties(ObjectId id, Transaction *transaction) const;
  virtual const GateProperties *gateProperties(ObjectId id, Transaction *transaction) const;
  virtual const LaserProperties *laserProperties(ObjectId id, Transaction *transaction) const;
  virtual const ProjectorProperties *projectorProperties(ObjectId id, Transaction *transaction) const;
  virtual const LobGroupProperties *lobGroupProperties(ObjectId id, Transaction *transaction) const;
  virtual const CustomRenderingProperties* customRenderingProperties(ObjectId id, Transaction *transaction) const;

  virtual ScenarioProperties *mutable_scenarioProperties(Transaction *transaction);
  virtual PlatformProperties *mutable_platformProperties(ObjectId id, Transaction *transaction);
  virtual BeamProperties *mutable_beamProperties(ObjectId id, Transaction *transaction);
  virtual GateProperties *mutable_gateProperties(ObjectId id, Transaction *transaction);
  virtual LaserProperties *mutable_laserProperties(ObjectId id, Transaction *transaction);
  virtual ProjectorProperties *mutable_projectorProperties(ObjectId id, Transaction *transaction);
  virtual LobGroupProperties *mutable_lobGroupProperties(ObjectId id, Transaction *transaction);
  virtual CustomRenderingProperties* mutable_customRenderingProperties(ObjectId id, Transaction *transaction);
  ///@}

  /**@name Object Preferences
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const PlatformPrefs *platformPrefs(ObjectId id, Transaction *transaction) const;
  virtual const BeamPrefs *beamPrefs(ObjectId id, Transaction *transaction) const;
  virtual const GatePrefs *gatePrefs(ObjectId id, Transaction *transaction) const;
  virtual const LaserPrefs *laserPrefs(ObjectId id, Transaction *transaction) const;
  virtual const ProjectorPrefs *projectorPrefs(ObjectId id, Transaction *transaction) const;
  virtual const LobGroupPrefs *lobGroupPrefs(ObjectId id, Transaction *transaction) const;
  virtual const CommonPrefs *commonPrefs(ObjectId id, Transaction* transaction) const;
  virtual const CustomRenderingPrefs *customRenderingPrefs(ObjectId id, Transaction *transaction) const;

  virtual PlatformPrefs *mutable_platformPrefs(ObjectId id, Transaction *transaction);
  virtual BeamPrefs *mutable_beamPrefs(ObjectId id, Transaction *transaction);
  virtual GatePrefs *mutable_gatePrefs(ObjectId id, Transaction *transaction);
  virtual LaserPrefs *mutable_laserPrefs(ObjectId id, Transaction *transaction);
  virtual ProjectorPrefs *mutable_projectorPrefs(ObjectId id, Transaction *transaction);
  virtual LobGroupPrefs *mutable_lobGroupPrefs(ObjectId id, Transaction *transaction);
  virtual CustomRenderingPrefs *mutable_customRenderingPrefs(ObjectId id, Transaction *transaction);
  virtual CommonPrefs *mutable_commonPrefs(ObjectId id, Transaction* transaction);
  ///@}

  /**@name Add a platform, beam, gate, laser, projector, or lobGroup
   * @note Returns properties object to be initialized.
   *  A unique id is generated internally and should not be changed.
   *  The original id field should be used for any user generated ids.
   * @{
   */
  virtual PlatformProperties *addPlatform(Transaction *transaction);
  virtual BeamProperties *addBeam(Transaction *transaction);
  virtual GateProperties *addGate(Transaction *transaction);
  virtual LaserProperties *addLaser(Transaction *transaction);
  virtual ProjectorProperties *addProjector(Transaction *transaction);
  virtual LobGroupProperties *addLobGroup(Transaction *transaction);
  virtual CustomRenderingProperties *addCustomRendering(Transaction *transaction);
  ///@}

  virtual void removeEntity(ObjectId id);

  /**
   * remove a category data point
   * @param id entity the data is associated with
   * @param time timestamp the data was originally given
   * @param catNameInt integer id of the category name string
   * @param valueInt integer id of the category value string
   * @return 0 if a point was actually removed
   */
  virtual int removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt);

  /**
   * remove all the generic data associated with a tag
   * @param id entity the data is associated with
   * @param tag The generic data to remove
   * @return 0 if a tag was actually removed
   */
  virtual int removeGenericDataTag(ObjectId id, const std::string& tag);

  /**@name Add data update, command, generic data, or category data
   *@note Returns nullptr if platform for specified ID does not exist
   * @{
   */
  virtual PlatformUpdate *addPlatformUpdate(ObjectId id, Transaction *transaction);
  virtual BeamUpdate *addBeamUpdate(ObjectId id, Transaction *transaction);
  virtual BeamCommand *addBeamCommand(ObjectId id, Transaction *transaction);
  virtual GateUpdate  *addGateUpdate(ObjectId id, Transaction *transaction);
  virtual GateCommand *addGateCommand(ObjectId id, Transaction *transaction);
  virtual LaserUpdate *addLaserUpdate(ObjectId id, Transaction *transaction);
  virtual LaserCommand *addLaserCommand(ObjectId id, Transaction *transaction);
  virtual PlatformCommand *addPlatformCommand(ObjectId id, Transaction *transaction);
  virtual ProjectorUpdate *addProjectorUpdate(ObjectId id, Transaction *transaction);
  virtual ProjectorCommand *addProjectorCommand(ObjectId id, Transaction *transaction);
  virtual LobGroupUpdate *addLobGroupUpdate(ObjectId id, Transaction *transaction);
  virtual LobGroupCommand *addLobGroupCommand(ObjectId id, Transaction *transaction);
  virtual CustomRenderingCommand *addCustomRenderingCommand(ObjectId id, Transaction *transaction);
  virtual GenericData *addGenericData(ObjectId id, Transaction *transaction);
  virtual CategoryData *addCategoryData(ObjectId id, Transaction *transaction);

  //virtual TableData *addTableData(ObjectId id, Transaction *transaction);
  ///@}

  /**@name Retrieving read-only data slices
   * @note No locking performed for read-only update slice objects
   * @{
   */
  virtual const PlatformUpdateSlice *platformUpdateSlice(ObjectId id) const;
  virtual const PlatformCommandSlice *platformCommandSlice(ObjectId id) const;

  virtual const BeamUpdateSlice *beamUpdateSlice(ObjectId id) const;
  virtual const BeamCommandSlice *beamCommandSlice(ObjectId id) const;

  virtual const GateUpdateSlice *gateUpdateSlice(ObjectId id) const;
  virtual const GateCommandSlice *gateCommandSlice(ObjectId id) const;

  virtual const LaserUpdateSlice *laserUpdateSlice(ObjectId id) const;
  virtual const LaserCommandSlice *laserCommandSlice(ObjectId id) const;

  virtual const ProjectorUpdateSlice *projectorUpdateSlice(ObjectId id) const;
  virtual const ProjectorCommandSlice *projectorCommandSlice(ObjectId id) const;

  virtual const LobGroupUpdateSlice *lobGroupUpdateSlice(ObjectId id) const;
  virtual const LobGroupCommandSlice *lobGroupCommandSlice(ObjectId id) const;

  virtual const CustomRenderingCommandSlice *customRenderingCommandSlice(ObjectId id) const;

  virtual const GenericDataSlice *genericDataSlice(ObjectId id) const;

  virtual const CategoryDataSlice *categoryDataSlice(ObjectId id) const;
  ///@}

  /// @copydoc simData::DataStore::modifyPlatformCommandSlice
  virtual int modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier);

  /// @copydoc simData::DataStore::modifyCustomRenderingCommandSlice
  virtual int modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier);

  /**@name Listeners
   * @{
   */
  /// Add or remove a listener for event messages
  virtual void addListener(ListenerPtr callback);
  virtual void removeListener(ListenerPtr callback);
  ///@}

  /**@name ScenarioListeners
   * @{
   */
  /// Add or remove a listener for scenario event messages
  virtual void addScenarioListener(ScenarioListenerPtr callback);
  virtual void removeScenarioListener(ScenarioListenerPtr callback);
  ///@}

  /**@name NewUpdatesListener
  * @{
  */
  /// Sets a listener for when entity updates are added; use nullptr to remove.
  virtual void setNewUpdatesListener(NewUpdatesListenerPtr callback);
  /// Retrieves the listener for new updates (internal)
  NewUpdatesListener& newUpdatesListener() const;
  ///@}

  /**@name Get a handle to the CategoryNameManager
   * @{
   */
  virtual CategoryNameManager& categoryNameManager() const;
  ///@}

  /**
   * Retrieves a reference to the data table manager associated with this data store.
   * The data table manager can be used to create data tables associated with entities,
   * iterate through tables, and add data to existing tables.
   * @return Pointer to the data table manager.
   */
  virtual DataTableManager& dataTableManager() const;

protected:
  /// generate a unique id
  ObjectId genUniqueId_();
  ///@}

private:
  /// delete all entries in a map
  template <typename EntryMapType>
  void deleteEntries_(EntryMapType *entries);

  /// apply data limiting for this entity
  void applyDataLimiting_(ObjectId id);

  template <typename EntryMapType>
  void dataLimit_(std::map<ObjectId, EntryMapType*>& entryMap, ObjectId id, const CommonPrefs* prefs);
  ///@}

  /// Execute the onPostRemoveEntity callback
  void fireOnPostRemoveEntity_(ObjectId id, ObjectType ot);

  /// Check to see if a Listener got removed during a callback
  void checkForRemoval_(ListenerList& list);
  /// The Listener, if any, that got removed during the last callback
  ListenerList justRemoved_;

public:
  // Types for SIMDIS

  /// PlatformEntry uses its own PlatformMemoryCommandSlice instead of a template MemoryCommandSlice
  typedef MemoryDataEntry<PlatformProperties, PlatformPrefs, MemoryDataSlice<PlatformUpdate>,    MemoryCommandSlice<PlatformCommand, PlatformPrefs> >  PlatformEntry;
  /// BeamEntry;  note that it uses a BeamMemoryCommandSlice instead of a template MemoryCommandSlice
  typedef MemoryDataEntry<BeamProperties,      BeamPrefs,      MemoryDataSlice<BeamUpdate>,      BeamMemoryCommandSlice >      BeamEntry;
  /// GateEntry
  typedef MemoryDataEntry<GateProperties,      GatePrefs,      MemoryDataSlice<GateUpdate>,      GateMemoryCommandSlice >      GateEntry;
  /// LaserEntry
  typedef MemoryDataEntry<LaserProperties,     LaserPrefs,     MemoryDataSlice<LaserUpdate>,     MemoryCommandSlice<LaserCommand, LaserPrefs> >     LaserEntry;
  /// ProjectEntry
  typedef MemoryDataEntry<ProjectorProperties, ProjectorPrefs, MemoryDataSlice<ProjectorUpdate>, MemoryCommandSlice<ProjectorCommand, ProjectorPrefs> > ProjectorEntry;
  /// LobGroupEntry
  typedef MemoryDataEntry<LobGroupProperties,  LobGroupPrefs,  LobGroupMemoryDataSlice,          MemoryCommandSlice<LobGroupCommand, LobGroupPrefs> >   LobGroupEntry;
  /// CustomRenderingEntry
  typedef MemoryDataEntry<CustomRenderingProperties,       CustomRenderingPrefs,       MemoryDataSlice<CustomRenderingUpdate>,       MemoryCommandSlice<CustomRenderingCommand, CustomRenderingPrefs> >     CustomRenderingEntry;

  /// Map of entity IDs to platform entries
  typedef std::map<ObjectId, PlatformEntry*>           Platforms;
  /// Map of entity IDs to beam entries
  typedef std::map<ObjectId, BeamEntry*>               Beams;
  /// Map of entity IDs to gate entries
  typedef std::map<ObjectId, GateEntry*>               Gates;
  /// Map of entity IDs to laser entries
  typedef std::map<ObjectId, LaserEntry*>              Lasers;
  /// Map of entity IDs to projector entries
  typedef std::map<ObjectId, ProjectorEntry*>          Projectors;
  /// Map of entity IDs to LOB Group entries
  typedef std::map<ObjectId, LobGroupEntry*>           LobGroups;
  /// Map of entity IDs to custom entries
  typedef std::map<ObjectId, CustomRenderingEntry*>    CustomRenderings;
  /// Map of entity IDs to generic data entries
  typedef std::map<ObjectId, MemoryGenericDataSlice*>  GenericDataMap;
  /// Map of entity IDs to category data entries
  typedef std::map<ObjectId, MemoryCategoryDataSlice*> CategoryDataMap;


private:
  class MemoryInternalsMemento;

  // Implementation of transactions for this data store

  /// Transaction that does nothing
  class NullTransactionImpl : public TransactionImpl
  {
  public:
    virtual void commit() {}

    virtual void release() {}
  };

  /// Perform transactions that modify preferences
  /// Notification of changes are sent to observers on transaction release
  template<typename T>
  class MutableSettingsTransactionImpl : public TransactionImpl
  {
  public:
    MutableSettingsTransactionImpl(ObjectId id, T *settings, MemoryDataStore *store, ListenerList *observers);

    /// Retrieve the settings object to be modified during the transaction
    T *settings() { return modifiedSettings_; }

    /// Check for changes to preference object and copy them
    /// to the internal data structure
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate the properties
    /// object which has not been transferred
    virtual ~MutableSettingsTransactionImpl();

  private:
    ObjectId id_;                                 ///< Id of entry associated with the transaction (to be provided to observers on entry change notification)
    bool committed_;                              ///< The changes have been committed to the data structure
    bool notified_;                               ///< Observers have been notified of the entry's modification
    bool nameChange_;                             ///< Determine if a name change has occurred
    std::string oldName_;                         ///< Old entity name
    std::string newName_;                         ///< New entity name
    T *currentSettings_;                          ///< Pointer to current settings object stored by DataStore; Will not be modified until the transaction is committed
    T *modifiedSettings_;                         ///< The mutable settings object provided to the transaction initiator for modification
    MemoryDataStore *store_;
    ListenerList *observers_;
  };

  /// Perform transactions that modify properties
  /// Notification of changes are sent to observers on transaction release
  template<typename T>
  class MutablePropertyTransactionImpl : public TransactionImpl
  {
  public:
    MutablePropertyTransactionImpl(ObjectId id, T *properties, MemoryDataStore *store, ListenerList *observers);

    /// Retrieve the settings object to be modified during the transaction
    T *properties() { return modifiedProperties_; }

    /// Check for changes to property object and copy them
    /// to the internal data structure
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate the properties
    /// object which has not been transferred
    virtual ~MutablePropertyTransactionImpl();

  private:
    ObjectId id_;                                 ///< Id of entry associated with the transaction (to be provided to observers on entry change notification)
    bool committed_;                              ///< The changes have been committed to the data structure
    bool notified_;                               ///< Observers have been notified of the entry's modification
    T *currentProperties_;                        ///< Pointer to current properties object stored by DataStore; Will not be modified until the transaction is committed
    T *modifiedProperties_;                       ///< The mutable properties object provided to the transaction initiator for modification
    MemoryDataStore *store_;
    ListenerList *observers_;
  };

  /// Perform transactions that modify scenario properties
  /// Notification of changes are sent to observers on transaction release
  class ScenarioSettingsTransactionImpl : public TransactionImpl
  {
  public:
    ScenarioSettingsTransactionImpl(ScenarioProperties *settings, MemoryDataStore *store, ScenarioListenerList *observers);

    /// Retrieve the settings object to be modified during the transaction
    ScenarioProperties *settings() { return modifiedSettings_; }

    /// Check for changes to preference object and copy them
    /// to the internal data structure
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate the properties
    /// object which has not been transferred
    virtual ~ScenarioSettingsTransactionImpl();

  private:
    bool committed_;                              // The changes have been committed to the data structure
    bool notified_;                               // Observers have been notified of the entry's modification
    ScenarioProperties *currentSettings_;         // Pointer to current settings object stored by DataStore; Will not be modified until the transaction is committed
    ScenarioProperties *modifiedSettings_;        // The mutable settings object provided to the transaction initiator for modification
    MemoryDataStore *store_;
    ScenarioListenerList *observers_;
  };

  /// Perform transactions to add new object entry to the MemoryDataStore
  /// Notification of new entries are sent to observers on transaction release
  template<typename T, typename P>
  class NewEntryTransactionImpl : public TransactionImpl
  {
  public:
    NewEntryTransactionImpl(T *entry, std::map<ObjectId, T *> *entries, MemoryDataStore *store, ListenerList *listeners, const P *defaultPrefs, uint64_t initialId)
      : committed_(false),
        notified_(false),
        entry_(entry),
        entries_(entries),
        store_(store),
        listeners_(listeners),
        defaultPrefs_(defaultPrefs),
        initialId_(initialId)
    {
    }

    /// Transfers ownership of the entry object to the MemoryDataStore
    /// internal data structures
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate memory for the
    /// entry that was not added to the memoryDataStore internal data structures
    virtual ~NewEntryTransactionImpl();

  private:
    bool committed_;                              // The entry has been added to the data structure
    bool notified_;                               // Observers have been notified for the new entry's commit
    T *entry_;                                    // Type such as PlatformEntry, BeamEntry, GateEntry, LaserEntry, ProjectorEntry, LobGroupEntry
    std::map<ObjectId, T *> *entries_;            // Matches Platforms, Beams, Gates, Lasers, Projectors, or LobGroups internal typedef
    MemoryDataStore *store_;                      // Data store which will receive the new entity on commit
    ListenerList *listeners_;                     // Listeners from the data store which need to be notified
    const P *defaultPrefs_;                       // Default prefs values for initializing prefs on entity creation
    uint64_t initialId_;                          // Used to make sure the developer does not change the ID
  };

  /// Perform transactions on new updates
  template <typename T, typename SliceType>
  class NewUpdateTransactionImpl : public TransactionImpl
  {
  public:
    NewUpdateTransactionImpl(T *update, SliceType *slice, MemoryDataStore* dataStore, ObjectId id, bool isEntityUpdate)
      : committed_(false),
        update_(update),
        slice_(slice),
        dataStore_(dataStore),
        id_(id),
        isEntityUpdate_(isEntityUpdate)
    { }

    /// Transfers ownership of the update object to the MemoryDataStore
    /// internal data structures
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate the properties
    /// object which has not been transferred
    virtual ~NewUpdateTransactionImpl();

  private:
    /// Responsible for performing the actual insert(), prior to data limiting.
    void insert_();

    bool       committed_; // The update has been added to the data structure
    T         *update_;
    SliceType *slice_;     // Type such as PlatformEntry, BeamEntry, GateEntry, LaserEntry, ProjectorEntry, or LobGroupEntry
    MemoryDataStore* dataStore_; // Pointer back to the data store
    ObjectId id_; // entity id for applying data limiting
    bool isEntityUpdate_; // flag to indicate if transaction is entity update; if so, update time should apply to time bounds, and applies to new-updates listener
  };

  /** Perform transactions on new scenario generic updates
    * Cannot use NewUpdateTransactionImpl since a scenario does not have preferences, just property
    */
  template <typename T, typename SliceType>
  class NewScenarioGenericUpdateTransactionImpl : public TransactionImpl
  {
  public:
    NewScenarioGenericUpdateTransactionImpl(T *update, SliceType *slice, MemoryDataStore* dataStore, ObjectId id, bool applyTimeBound = true)
      : committed_(false),
        update_(update),
        slice_(slice),
        dataStore_(dataStore),
        id_(id),
        applyTimeBound_(applyTimeBound)
    { }

    /// Transfers ownership of the update object to the MemoryDataStore
    /// internal data structures
    virtual void commit();

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release();

    /// If the transaction was not committed, will deallocate the properties
    /// object which has not been transferred
    virtual ~NewScenarioGenericUpdateTransactionImpl();

  private:
    bool       committed_; // The update has been added to the data structure
    T         *update_;
    SliceType *slice_;     // Type such as PlatformEntry, BeamEntry, GateEntry, LaserEntry, ProjectorEntry, or LobGroupEntry
    MemoryDataStore* dataStore_; // Pointer back to the data store
    ObjectId id_; // entity id for applying data limiting
    bool applyTimeBound_; // flag to indicate if update time should apply to time bounds
  };

private:
  /// Updates all the platforms
  void updatePlatforms_(double time);
  /// Updates a target beam
  void updateTargetBeam_(ObjectId id, BeamEntry* beam, double time);
  /// Updates all the beams
  void updateBeams_(double time);
  ///Gets the beam that corresponds to specified gate
  BeamEntry* getBeamForGate_(google::protobuf::uint64 gateID);
  /// Updates a target gate
  void updateTargetGate_(GateEntry* gate, double time);

  /**
  * Determine if a gate depends on beam prefs for height/width
  * @param gate gate entry to test for beam dependency
  * @return true if gate is using host beam height or width parameter, false otherwise
  */
  bool gateUsesBeamBeamwidth_(GateEntry* gate) const;

  /// Updates all the gates
  void updateGates_(double time);
  /// Updates all the lasers
  void updateLasers_(double time);
  /// Updates all the projectors
  void updateProjectors_(double time);
  /// Updates all the LobGroups
  void updateLobGroups_(double time);
  ///Updates all the CustomRenderings
  void updateCustomRenderings_(double time);
  /// Flushes an entity based on the given scope, fields and time ranges
  void flushEntity_(ObjectId id, simData::ObjectType type, FlushScope flushScope, FlushFields flushFields, double startTime, double endTime);
  /// Flushes an entity's data tables
  void flushDataTables_(ObjectId id);
  /// Flushes an entity's data tables for the given time range; up to but not including endTime
  void flushDataTables_(ObjectId id, double startTime, double endTime);

  /// Initialize the default prefs objects
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs,
    const BeamPrefs& beamPrefs,
    const GatePrefs& gatePrefs,
    const LaserPrefs& laserPrefs,
    const LobGroupPrefs& lobPrefs,
    const ProjectorPrefs& projectorPrefs);
  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs);
  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual PlatformPrefs defaultPlatformPrefs() const;

  ObjectId baseId_;          // Used for unique ID generation
  double   lastUpdateTime_;  // Last time sent to update(double)
  bool     hasChanged_; // has something changed since last update

  // interpolation
  bool          interpolationEnabled_;
  Interpolator *interpolator_;

  // all the data
  ScenarioProperties properties_;
  Platforms          platforms_;
  Beams              beams_;
  Gates              gates_;
  Lasers             lasers_;
  Projectors         projectors_;
  LobGroups          lobGroups_;
  CustomRenderings   customRenderings_;
  GenericDataMap     genericData_;  // Map to hold references for GenericData update slice contained by the DataEntry object with the associated id
  CategoryDataMap    categoryData_; // Map to hold references for CategoryData update slice contained by the DataEntry object with the associated id

  // default prefs objects
  PlatformPrefs  defaultPlatformPrefs_;
  BeamPrefs      defaultBeamPrefs_;
  GatePrefs      defaultGatePrefs_;
  LaserPrefs     defaultLaserPrefs_;
  LobGroupPrefs  defaultLobGroupPrefs_;
  ProjectorPrefs defaultProjectorPrefs_;
  CustomRenderingPrefs defaultCustomRenderingPrefs_;

  /// Observers to receive notifications when things change
  ListenerList listeners_;
  /// Observers to receive notifications when things change
  ScenarioListenerList scenarioListeners_;
  /// Observer for new updates
  NewUpdatesListenerPtr newUpdatesListener_;
  /// Flag indicating if data limiting is set
  bool dataLimiting_;
  /// The CategoryNameManager coordinates string/int values
  CategoryNameManager* categoryNameManager_;
  /// Correlates data store preferences to limit values for the table manager
  MemoryTable::DataLimitsProvider* dataLimitsProvider_;
  /// Data Table Manager pointer contains all data tables
  DataTableManager* dataTableManager_;
  /// Clock pointer is bound to application's Clock, allows datastore to determine current application's data mode
  simCore::Clock* boundClock_;

  /// Improves performance of by-name searches in the data store
  EntityNameCache* entityNameCache_;

  /// Links together the TableManager::NewRowDataListener to our newUpdatesListener_
  std::shared_ptr<NewRowDataToNewUpdatesAdapter> newRowDataListener_;

}; // End of class MemoryDataStore

} // End of namespace simData

#endif // SIMDATA_MEMORYSCENARIO_H

