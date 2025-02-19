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
  SDK_DISABLE_COPY(MemoryDataStore);

  virtual ~MemoryDataStore();

  /**@name Interface from DataStore
   * @{
   */
  /// allocate a new memento for this internals (caller deletes)
  virtual InternalsMemento* createInternalsMemento() const override;

  /// update all data slices to reflect current 'time'
  virtual void update(double time) override;

  /// returns the last value sent to update(double), relative to current reference year
  virtual double updateTime() const override;

  /// data store reference year (without transaction cost); intended to be cached locally for performance.
  virtual int referenceYear() const override;

  /// store a reference to current clock, for time/data mode
  virtual void bindToClock(simCore::Clock* clock) override;

  /// fetches a currently bound clock
  virtual simCore::Clock* getBoundClock() const override;

  /**
  * set data limiting in the data store.  This sets a flag in the
  * DataStore object, which is checked by each NewUpdateTransactionImpl object
  * after the DataSlice insert call is made, and then makes the call to
  * DataSlice::limitByPrefs, passing in the CommonPrefs record
  * @param[in] dataLimiting
  */
  virtual void setDataLimiting(bool dataLimiting) override;

  /// returns flag indicating if data limiting is set
  virtual bool dataLimiting() const override;

  /// flush all the updates, command, category data and generic data for the specified id,
  /// if 0 is passed in flushes the entire scenario, except for static entities
  virtual void flush(ObjectId flushId, FlushType type = NON_RECURSIVE) override;

  /** Removes all the specified data */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields) override;

  /** Removes a range of data from startTime up to but not including the endTime */
  virtual int flush(ObjectId id, FlushScope scope, FlushFields fields, double startTime, double endTime) override;

  /** clear out the data store of all scenario specific data, including all entities and category data names. */
  virtual void clear() override;

  /**@name Interpolation
   *@{
   */
  /// implementation supports interpolation for updates
  virtual bool canInterpolate() const override;

  /** Enable or disable interpolation, if supported
   *
   * @note (Will only succeed if implementation supports interpolation
   * and contains a valid interpolator object)
   */
  virtual bool enableInterpolation(bool state) override;

  /// interpolation is enabled
  virtual bool isInterpolationEnabled() const override;

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
  virtual size_t idCount(simData::ObjectType type = simData::ALL) const override;

  /// Retrieve a list of IDs for objects of 'type'
  virtual void idList(IdList* ids, simData::ObjectType type = simData::ALL) const override;

  /// Retrieve a list of IDs for objects of 'type' with the given name
  virtual void idListByName(const std::string& name, IdList* ids, simData::ObjectType type = simData::ALL) const override;

  /// Retrieve a list of IDs for objects with the given original id
  virtual void idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type = simData::ALL) const override;

  /// Retrieve a list of IDs for all beams associated with a platform
  virtual void beamIdListForHost(ObjectId hostid, IdList *ids) const override;

  /// Retrieve a list of IDs for all gates associated with a beam
  virtual void gateIdListForHost(ObjectId hostid, IdList *ids) const override;

  /// Retrieve a list of IDs for all lasers associated with a platform
  virtual void laserIdListForHost(ObjectId hostid, IdList *ids) const override;

  /// Retrieve a list of IDs for all projectors associated with a platform
  virtual void projectorIdListForHost(ObjectId hostid, IdList *ids) const override;

  /// Retrieve a list of IDs for all lobGroups associated with a platform
  virtual void lobGroupIdListForHost(ObjectId hostid, IdList *ids) const override;

  /// Retrieve a list of IDs for all customs associated with a platform
  virtual void customRenderingIdListForHost(ObjectId hostid, IdList *ids) const override;

  ///Retrieves the ObjectType for a particular ID
  virtual simData::ObjectType objectType(ObjectId id) const override;

  /// Retrieves the host entity ID for a particular ID (i.e. a beam, given a gate ID; a platform, given a LOB ID)
  virtual ObjectId entityHostId(ObjectId childId) const override;

  /// Retrieves the time bounds for a particular entity ID (first point, last point)
  virtual std::pair<double, double> timeBounds(ObjectId entityId) const override;
  /// Retrieves the scenario time bounds (first point, last point); scenario (ID 0) implementation
  std::pair<double, double> timeBounds() const;
  ///@}

  /**@name Properties
   * @note should always return a valid object (never nullptr)
   * @{
   */
  virtual const ScenarioProperties *scenarioProperties(Transaction *transaction) const override;
  virtual const PlatformProperties *platformProperties(ObjectId id, Transaction *transaction) const override;
  virtual const BeamProperties *beamProperties(ObjectId id, Transaction *transaction) const override;
  virtual const GateProperties *gateProperties(ObjectId id, Transaction *transaction) const override;
  virtual const LaserProperties *laserProperties(ObjectId id, Transaction *transaction) const override;
  virtual const ProjectorProperties *projectorProperties(ObjectId id, Transaction *transaction) const override;
  virtual const LobGroupProperties *lobGroupProperties(ObjectId id, Transaction *transaction) const override;
  virtual const CustomRenderingProperties* customRenderingProperties(ObjectId id, Transaction *transaction) const override;

  virtual ScenarioProperties *mutable_scenarioProperties(Transaction *transaction) override;
  virtual PlatformProperties *mutable_platformProperties(ObjectId id, Transaction *transaction) override;
  virtual BeamProperties *mutable_beamProperties(ObjectId id, Transaction *transaction) override;
  virtual GateProperties *mutable_gateProperties(ObjectId id, Transaction *transaction) override;
  virtual LaserProperties *mutable_laserProperties(ObjectId id, Transaction *transaction) override;
  virtual ProjectorProperties *mutable_projectorProperties(ObjectId id, Transaction *transaction) override;
  virtual LobGroupProperties *mutable_lobGroupProperties(ObjectId id, Transaction *transaction) override;
  virtual CustomRenderingProperties* mutable_customRenderingProperties(ObjectId id, Transaction *transaction) override;
  ///@}

  /**@name Object Preferences
   * @note will return nullptr if no object is associated with the specified id
   * @{
   */
  virtual const PlatformPrefs *platformPrefs(ObjectId id, Transaction *transaction) const override;
  virtual const BeamPrefs *beamPrefs(ObjectId id, Transaction *transaction) const override;
  virtual const GatePrefs *gatePrefs(ObjectId id, Transaction *transaction) const override;
  virtual const LaserPrefs *laserPrefs(ObjectId id, Transaction *transaction) const override;
  virtual const ProjectorPrefs *projectorPrefs(ObjectId id, Transaction *transaction) const override;
  virtual const LobGroupPrefs *lobGroupPrefs(ObjectId id, Transaction *transaction) const override;
  virtual const CommonPrefs *commonPrefs(ObjectId id, Transaction* transaction) const override;
  virtual const CustomRenderingPrefs *customRenderingPrefs(ObjectId id, Transaction *transaction) const override;

  /**
   * The mutable_* routines have two modes of operation, one for external users and one for internal users.  External users should
   * always set the results argument to nullptr.  The mutable_* routines will generate a callback(s) in the commit() routine if the
   * preference changed and if the name changed.  As always, the routine must be called from the main thread.   An internal user can
   * set the results argument that will disable the callback(s) and return if preference has changed due to the commit() routine.
   * This allows an internal user to use worker threads to update preferences and accumulate the results for eventual callbacks in
   * the main thread.  The design of the code made it not practical to hide the argument from the public interface.  External users
   * should always set results to nullptr.
   */

  virtual PlatformPrefs *mutable_platformPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual BeamPrefs *mutable_beamPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual GatePrefs *mutable_gatePrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual LaserPrefs *mutable_laserPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual ProjectorPrefs *mutable_projectorPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual LobGroupPrefs *mutable_lobGroupPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual CustomRenderingPrefs *mutable_customRenderingPrefs(ObjectId id, Transaction *transaction, CommitResult* results = nullptr) override;
  virtual CommonPrefs *mutable_commonPrefs(ObjectId id, Transaction* transaction) override;
  ///@}

  /**@name Add a platform, beam, gate, laser, projector, or lobGroup
   * @note Returns properties object to be initialized.
   *  A unique id is generated internally and should not be changed.
   *  The original id field should be used for any user generated ids.
   * @{
   */
  virtual PlatformProperties *addPlatform(Transaction *transaction) override;
  virtual BeamProperties *addBeam(Transaction *transaction) override;
  virtual GateProperties *addGate(Transaction *transaction) override;
  virtual LaserProperties *addLaser(Transaction *transaction) override;
  virtual ProjectorProperties *addProjector(Transaction *transaction) override;
  virtual LobGroupProperties *addLobGroup(Transaction *transaction) override;
  virtual CustomRenderingProperties *addCustomRendering(Transaction *transaction) override;
  ///@}

  virtual void removeEntity(ObjectId id) override;

  /**
   * remove a category data point
   * @param id entity the data is associated with
   * @param time timestamp the data was originally given
   * @param catNameInt integer id of the category name string
   * @param valueInt integer id of the category value string
   * @return 0 if a point was actually removed
   */
  virtual int removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt) override;

  /**
   * remove all the generic data associated with a tag
   * @param id entity the data is associated with
   * @param tag The generic data to remove
   * @return 0 if a tag was actually removed
   */
  virtual int removeGenericDataTag(ObjectId id, const std::string& tag) override;

  /**@name Add data update, command, generic data, or category data
   *@note Returns nullptr if platform for specified ID does not exist
   * @{
   */
  virtual PlatformUpdate *addPlatformUpdate(ObjectId id, Transaction *transaction) override;
  virtual BeamUpdate *addBeamUpdate(ObjectId id, Transaction *transaction) override;
  virtual BeamCommand *addBeamCommand(ObjectId id, Transaction *transaction) override;
  virtual GateUpdate  *addGateUpdate(ObjectId id, Transaction *transaction) override;
  virtual GateCommand *addGateCommand(ObjectId id, Transaction *transaction) override;
  virtual LaserUpdate *addLaserUpdate(ObjectId id, Transaction *transaction) override;
  virtual LaserCommand *addLaserCommand(ObjectId id, Transaction *transaction) override;
  virtual PlatformCommand *addPlatformCommand(ObjectId id, Transaction *transaction) override;
  virtual ProjectorUpdate *addProjectorUpdate(ObjectId id, Transaction *transaction) override;
  virtual ProjectorCommand *addProjectorCommand(ObjectId id, Transaction *transaction) override;
  virtual LobGroupUpdate *addLobGroupUpdate(ObjectId id, Transaction *transaction) override;
  virtual LobGroupCommand *addLobGroupCommand(ObjectId id, Transaction *transaction) override;
  virtual CustomRenderingCommand *addCustomRenderingCommand(ObjectId id, Transaction *transaction) override;
  virtual GenericData *addGenericData(ObjectId id, Transaction *transaction) override;
  virtual CategoryData *addCategoryData(ObjectId id, Transaction *transaction) override;
  ///@}

  /**@name Retrieving read-only data slices
   * @note No locking performed for read-only update slice objects
   * @{
   */
  virtual const PlatformUpdateSlice *platformUpdateSlice(ObjectId id) const override;
  virtual const PlatformCommandSlice *platformCommandSlice(ObjectId id) const override;

  virtual const BeamUpdateSlice *beamUpdateSlice(ObjectId id) const override;
  virtual const BeamCommandSlice *beamCommandSlice(ObjectId id) const override;

  virtual const GateUpdateSlice *gateUpdateSlice(ObjectId id) const override;
  virtual const GateCommandSlice *gateCommandSlice(ObjectId id) const override;

  virtual const LaserUpdateSlice *laserUpdateSlice(ObjectId id) const override;
  virtual const LaserCommandSlice *laserCommandSlice(ObjectId id) const override;

  virtual const ProjectorUpdateSlice *projectorUpdateSlice(ObjectId id) const override;
  virtual const ProjectorCommandSlice *projectorCommandSlice(ObjectId id) const override;

  virtual const LobGroupUpdateSlice *lobGroupUpdateSlice(ObjectId id) const override;
  virtual const LobGroupCommandSlice *lobGroupCommandSlice(ObjectId id) const override;

  virtual const CustomRenderingCommandSlice *customRenderingCommandSlice(ObjectId id) const override;

  virtual const GenericDataSlice *genericDataSlice(ObjectId id) const override;

  virtual const CategoryDataSlice *categoryDataSlice(ObjectId id) const override;
  ///@}

  /// @copydoc simData::DataStore::installSliceTimeRangeMonitor
  virtual void installSliceTimeRangeMonitor(ObjectId id, std::function<void(double startTime, double endTime)> fn) override;

  /// @copydoc simData::DataStore::modifyPlatformCommandSlice
  virtual int modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier) override;

  /// @copydoc simData::DataStore::modifyProjectorCommandSlice
  virtual int modifyProjectorCommandSlice(ObjectId id, VisitableDataSlice<ProjectorCommand>::Modifier* modifier) override;

  /// @copydoc simData::DataStore::modifyCustomRenderingCommandSlice
  virtual int modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier) override;

  /**@name Listeners
   * @{
   */
  /// Add or remove a listener for event messages
  virtual void addListener(ListenerPtr callback) override;
  virtual void removeListener(ListenerPtr callback) override;
  ///@}

  /**@name ScenarioListeners
   * @{
   */
  /// Add or remove a listener for scenario event messages
  virtual void addScenarioListener(ScenarioListenerPtr callback) override;
  virtual void removeScenarioListener(ScenarioListenerPtr callback) override;
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
  virtual CategoryNameManager& categoryNameManager() const override;
  ///@}

  /**
   * Retrieves a reference to the data table manager associated with this data store.
   * The data table manager can be used to create data tables associated with entities,
   * iterate through tables, and add data to existing tables.
   * @return Pointer to the data table manager.
   */
  virtual DataTableManager& dataTableManager() const override;

protected:
  /// generate a unique id
  ObjectId genUniqueId_();
  ///@}

private:
  class NewRowDataToNewUpdatesAdapter;

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

  /// Adds the children of hostid of type inType to the ids list
  void idListForHost_(ObjectId hostid, simData::ObjectType inType, IdList* ids) const;

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
    virtual void commit() override {}

    virtual void release() override {}
  };

  /// Perform transactions that modify preferences
  /// Notification of changes are sent to observers on transaction release if the results argument is a nullptr.
  /// If the results argument is defined the observers are not called and results argument is set to the outcome of the commit.
  template<typename T>
  class MutableSettingsTransactionImpl : public TransactionImpl
  {
  public:
    MutableSettingsTransactionImpl(ObjectId id, T *settings, MemoryDataStore *store, ListenerList *observers, CommitResult* results = nullptr);

    /// Retrieve the settings object to be modified during the transaction
    T *settings() { return modifiedSettings_; }

    /// Check for changes to preference object and copy them
    /// to the internal data structure
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
    T *currentSettings_ = nullptr;                ///< Pointer to current settings object stored by DataStore; Will not be modified until the transaction is committed
    T *modifiedSettings_ = nullptr;               ///< The mutable settings object provided to the transaction initiator for modification
    MemoryDataStore *store_ = nullptr;            ///< Memory data store
    ListenerList *observers_ = nullptr;           ///< Observers to call if the results argument is a nullptr
    CommitResult* results_ = nullptr;            ///< If not a nullptr, the outcome of the commit and the observers are not called
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
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
    virtual void commit() override;

    /// No resources to be released here (resource locks/DB handles/etc)
    virtual void release() override;

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
  /// Look for transitions from Live mode to File mode to force an update to hide expired platforms
  class ClockModeMonitor;

  /// Invokes the appropriate callbacks for the given entities
  void invokePreferenceChangeCallback_(const std::map<simData::ObjectId, CommitResult>& results, ListenerList& localCopy);

  /// Clean up memory
  void clearMemory_();

  /// Updates a target beam
  void updateTargetBeam_(ObjectId id, BeamEntry* beam, double time);
  /// Updates all the beams
  void updateBeams_(double time);
  /// Gets the beam that corresponds to specified gate
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
  /// Flushes an entity based on the given scope, fields and time ranges
  void flushEntity_(ObjectId id, simData::ObjectType type, FlushScope flushScope, FlushFields flushFields, double startTime, double endTime);
  /// Flushes an entity's data tables
  void flushDataTables_(ObjectId id);
  /// Flushes an entity's data tables for the given time range; up to but not including endTime
  void flushDataTables_(ObjectId id, double startTime, double endTime);

  /// Configure local listeners
  void initCompositeListener_();

  /// Initialize the default prefs objects
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs,
    const BeamPrefs& beamPrefs,
    const GatePrefs& gatePrefs,
    const LaserPrefs& laserPrefs,
    const LobGroupPrefs& lobPrefs,
    const ProjectorPrefs& projectorPrefs) override;
  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs) override;
  /** @see simData::DataStore::defaultPlatformPrefs() */
  virtual PlatformPrefs defaultPlatformPrefs() const override;

  ObjectId baseId_;          // Used for unique ID generation
  double   lastUpdateTime_;  // Last time sent to update(double)
  bool     hasChanged_; // has something changed since last update

  // interpolation
  InterpolatorState interpolationEnabled_ = InterpolatorState::OFF;
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

  /// To improve performance keep track of children entities by host
  class HostChildCache;
  /// To improve performance keep track of Original IDs
  class OriginalIdCache;
  /// Improve performance by caching the slice state
  class SliceCacheObserver;

  /// Key by host id and child type
  struct IdAndTypeKey {
    ObjectId id;
    simData::ObjectType type;
    IdAndTypeKey(ObjectId inId, simData::ObjectType inType)
      : id(inId),
        type(inType)
    {
    }
    bool operator<(const IdAndTypeKey& rhs) const
    {
      if (id < rhs.id)
        return true;

      if (id > rhs.id)
        return false;

      return type < rhs.type;
    }
  };
  /// A secondary map to track children id by host id
  std::multimap<IdAndTypeKey, ObjectId> hostToChildren_;

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
  /// Observers for new updates
  std::vector<NewUpdatesListenerPtr> newUpdatesListeners_;
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

  /// Improve performance by caching the original Ids
  std::shared_ptr<OriginalIdCache> originalIdCache_;

  /// Improve performance by caching the slice state
  std::shared_ptr<SliceCacheObserver> sliceCacheObserver_;

  /// Links together the TableManager::NewRowDataListener to our newUpdatesListener_
  std::shared_ptr<NewRowDataToNewUpdatesAdapter> newRowDataListener_;

  /// Look for transitions from Live mode to File mode to force an update to hide expired platforms
  std::shared_ptr<ClockModeMonitor> clockModeMonitor_;

}; // End of class MemoryDataStore

} // End of namespace simData

#endif // SIMDATA_MEMORYDATASTORE_H
