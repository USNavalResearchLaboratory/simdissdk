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
#ifndef SIMDATA_DATASTORE_H
#define SIMDATA_DATASTORE_H

#include <cassert>
#include <vector>

#include "simData/DataSlice.h"
#include "simData/Interpolator.h"
#include "simCore/Common/Memory.h"
#include "simCore/Common/Export.h"

// forward declare
namespace simCore { class Clock; }

namespace simData
{
/// uniquely identify an object
typedef uint64_t ObjectId;
class CategoryDataSlice;
class CategoryNameManager;
class GenericDataSlice;
class DataTableManager;

/** @brief Interface for storing and retrieving scenario data
 *
 *  DataStore provides an interface for the SIMDIS SDK data storage component.
 *  The data storage component is responsible for storing the position and state
 *  information for objects whose position and state change relative to time
 *  within a scenario. Objects with data that is stored in the data store
 *  include Platforms, Beams, Gates, Lasers, Projectors, and LobGroups.
 *
 *  Access to data within the data store for retrieval and/or update is managed
 *  by a transaction based system. The transaction system allows the use of
 *  relational database systems, such as MySQL or Oracle, for back-end storage.
 *  It also provides a mechanism for safe concurrent access to the data store.
 *  More information about transactions can be found in the
 *  DataStore::Transaction documentation.
 *
 *  The data store is capable of retrieving position and state information
 *  that is relative to a specific time. The time is specified to the data store
 *  with the DataStore::update(double) function. Position and state data for
 *  the specified time is accessed with an DataSlice object. Each object
 *  stored in the data store has an associated data slice.
 *
 *  The data slice provides a read-only handle to information contained within the
 *  data store, and can be held outside of a transaction.  It is the only
 *  data store item that can be safely held outside of a transaction. It is safe
 *  for an application to hold onto a data slice as long as the following
 *  conditions are met:@n
 *    1. The application does not access data provided by the data slice
 *       while the DataStore::update(double) function is active; update
 *       modifies the information that is accessible to the data slice, and
 *       accessing that information during an update can result in retrieval
 *       of incorrect information.@n
 *    2. The application registers an observer with the data store to be notified
 *       when the data slice's object is removed. The application must release
 *       the pointer to the data slice when its object is removed, or it will
 *       be holding onto a dangling pointer.
 *
 *  The data store manages all of the memory for the objects that it contains,
 *  and is responsible for allocating and deallocating that memory.  When it
 *  allocates memory for new objects, it assigns a unique ID to that object.
 *  This ID is stored in the object's Properties record, and should never be
 *  changed by the application.
 */
class SDKDATA_EXPORT DataStore
{
protected:
  class TransactionImpl;

public: // types
  /// Bitmask representing the types of objects that are assigned IDs
  enum ObjectType
  {
    NONE      = 0x00,
    PLATFORM  = 0x01,
    BEAM      = 0x02,
    GATE      = 0x04,
    LASER     = 0x08,
    PROJECTOR = 0x10,
    LOB_GROUP = 0x20,
    ALL = (PLATFORM|BEAM|GATE|LASER|PROJECTOR|LOB_GROUP)
  };

  /** DataStore transaction handle
   *
   *  The primary functions of the DataStore transaction are:@n
   *  1. Allow safe concurrent access to scenario data@n
   *  2. Allow modification to data residing in a separate memory, such as a
   *     database, to be synchronously committed after modification@n
   *
   * Currently works as a scoped transaction that is canceled if not committed
   * prior to exiting scope.  The pointer to the mutable object associated
   * with the transaction should not be accessed after the transaction is
   * committed or has gone out of scope.  An attempt to enforce this behavior
   * has been made by requiring the reference to the object associated with the
   * transaction to be provided as an argument when releasing or canceling a
   * transaction, so that it may be set to NULL.
   */
  class SDKDATA_EXPORT Transaction
  {
  public:
    Transaction();

    /// construct using a given implementation
    Transaction(TransactionImpl *actual);

    /// copy constructor
    Transaction(const Transaction &transaction);

    virtual ~Transaction();

    /// allow assignment
    Transaction &operator=(const Transaction &rhs);

    /// commit changes to the object associated with this transaction handle
    void commit();

    /// release the transaction and its resources; effectively cancels an uncommitted transaction
    template <typename T>
    void release(T **operand)
    {
      assert(transaction_.get() != NULL);
      transaction_->release();
      *operand = NULL;
    }

    /// complete the transaction by committing it and releasing it; equivalent to commit followed by release
    template <typename T>
    void complete(T **operand)
    {
      commit();
      release(operand);
    }

  private:
    std::tr1::shared_ptr<TransactionImpl> transaction_; /// underlying implementation
  };

  /// similar to Observer, but provides more info to the listener
  class Listener
  {
  public: // methods
    virtual ~Listener() {}

    /// new entity has been added, with the given id and type
    virtual void onAddEntity(DataStore *source, ObjectId newId, ObjectType ot) = 0;

    /// entity with the given id and type will be removed after all notifications are processed
    virtual void onRemoveEntity(DataStore *source, ObjectId removedId, ObjectType ot) = 0;

    /// prefs for the given entity have been changed
    virtual void onPrefsChange(DataStore *source, ObjectId id) = 0;

    /// current time has been changed
    virtual void onTimeChange(DataStore *source) = 0;

    /// something has changed in the entity category data
    virtual void onCategoryDataChange(DataStore *source, ObjectId changedId, ObjectType ot) = 0;

    /// entity name has changed
    virtual void onNameChange(DataStore *source, ObjectId changeId) = 0;

    /// entity's data was flushed, 0 means entire scenario was flushed
    virtual void onFlush(DataStore* source, ObjectId id) = 0;

    /// The scenario is about to be deleted
    virtual void onScenarioDelete(DataStore* source) = 0;
  };

  /// default Listener - does nothing
  /// (useful for consumers who only want one or two events)
  class DefaultListener : public Listener
  {
  public: // methods
    virtual ~DefaultListener() {}

    /// new entity has been added, with the given id and type
    virtual void onAddEntity(DataStore *source, ObjectId newId, ObjectType ot) {}

    /// entity with the given id and type will be removed after all notifications are processed
    virtual void onRemoveEntity(DataStore *source, ObjectId removedId, ObjectType ot) {}

    /// prefs for the given entity have been changed
    virtual void onPrefsChange(DataStore *source, ObjectId id) {}

    /// current time has been changed
    virtual void onTimeChange(DataStore *source) {}

    /// something has changed in the entity category data
    virtual void onCategoryDataChange(DataStore *source, ObjectId changedId, ObjectType ot) {}

    /// entity name has changed
    virtual void onNameChange(DataStore *source, ObjectId changeId) {}

    /// entity's data was flushed, 0 means entire scenario was flushed
    virtual void onFlush(DataStore *source, ObjectId flushedId) {}

    /// The scenario is about to be deleted
    virtual void onScenarioDelete(DataStore* source) {}
  };

  /// Managed pointer to be used when holding a pointer to a Listener object.
  /// Memory for the Listener object is deleted automatically when the last managed pointer is released.
  typedef std::tr1::shared_ptr<Listener> ListenerPtr;

  /// Observer for scenario events
  class ScenarioListener
  {
  public: // methods
    virtual ~ScenarioListener() {}

    /// Scenario Property changed
    virtual void onScenarioPropertiesChange(DataStore *source) = 0;

  };

  /// opaque class used to store internals when swapping data stores
  class InternalsMemento
  {
  public:
    virtual ~InternalsMemento();

    /// add these internals to the given data store
    virtual void apply(DataStore &ds) = 0;
  };

  /// Managed pointer to be used when holding a pointer to a ScenarioListener object.
  /// Memory for the ScenarioListener object is deleted automatically when the last managed pointer is released.
  typedef std::tr1::shared_ptr<ScenarioListener> ScenarioListenerPtr;

  /// List of listeners
  typedef std::vector<ListenerPtr> ListenerList;
  /// List of listeners
  typedef std::vector<ScenarioListenerPtr> ScenarioListenerList;

  /// List of IDs for objects contained by the DataStore
  typedef std::vector<ObjectId> IdList;

public: // methods
  virtual ~DataStore();

  /// allocate a new memento for this internals (caller deletes)
  virtual InternalsMemento* createInternalsMemento() const = 0;

  /// update all data slices to reflect current 'time'
  virtual void update(double time) = 0;

  /// returns the last value sent to update(double), relative to current reference year
  virtual double updateTime() const = 0;

  /// data store reference year (without transaction cost); intended to be cached locally for performance.
  virtual int referenceYear() const = 0;

  /// store a reference to current clock, for time/data mode
  virtual void bindToClock(simCore::Clock* clock) = 0;

  /// fetches a currently bound clock
  virtual simCore::Clock* getBoundClock() const = 0;

  /// set data limiting in the data store
  virtual void setDataLimiting(bool dataLimiting) = 0;

  /// returns flag indicating if data limiting is set
  virtual bool dataLimiting() const = 0;

  /// Types of flushes supported by the flush method
  enum FlushType
  {
    NON_RECURSIVE,   ///< Flush only the supplied entity and keep any static point
    NON_RECURSIVE_TSPI_STATIC, ///< Flush only the supplied PLATFORM and flush any static TSPI point
    RECURSIVE ///< Flush the supplied entity and any children and keep any static point
  };

  /**
   * flush all the updates, command, category data and generic data for the specified id,
   * if 0 is passed in flushes the entire scenario, except for static entities
   */
  virtual void flush(ObjectId flushId, FlushType type = NON_RECURSIVE) = 0;

  /**
  * clear out the data store of all scenario specific data, including all entities and category data names.
  */
  virtual void clear() = 0;

  /**@name Interpolation
   *@{
   */
  /// implementation supports interpolation for updates
  virtual bool canInterpolate() const = 0;

  /** Enable or disable interpolation, if supported
   *
   * @note (Will only succeed if implementation supports interpolation
   * and contains a valid interpolator object)
   * @return Flag indicating isInterpolationEnabled()
   */
  virtual bool enableInterpolation(bool state) = 0;

  /// interpolation is enabled
  virtual bool isInterpolationEnabled() const = 0;

  /// Specify the interpolator to use
  virtual void setInterpolator(Interpolator *interpolator) = 0;

  /// Get the current interpolator (NULL if disabled)
  virtual Interpolator* interpolator() const = 0;
  ///@}

  /**@name ID Lists
   * @{
   */
  /// Retrieve a list of IDs for objects of 'type'
  virtual void idList(IdList *ids, ObjectType type = ALL) const = 0;

  /// Retrieve a list of IDs for objects of 'type' with the given name
  virtual void idListByName(const std::string& name, IdList* ids, ObjectType type = ALL) const = 0;

  /// Retrieve a list of IDs for objects with the given original id
  virtual void idListByOriginalId(IdList *ids, uint64_t originalId, ObjectType type = ALL) const = 0;

  /// Retrieve a list of IDs for all beams associated with a platform
  virtual void beamIdListForHost(ObjectId hostid, IdList *ids) const = 0;

  /// Retrieve a list of IDs for all gates associated with a beam
  virtual void gateIdListForHost(ObjectId hostid, IdList *ids) const = 0;

  /// Retrieve a list of IDs for all lasers associated with a platform
  virtual void laserIdListForHost(ObjectId hostid, IdList *ids) const = 0;

  /// Retrieve a list of IDs for all projectors associated with a platform
  virtual void projectorIdListForHost(ObjectId hostid, IdList *ids) const = 0;

  /// Retrieve a list of IDs for all LobGroups associated with a platform
  virtual void lobGroupIdListForHost(ObjectId hostid, IdList *ids) const = 0;

  /// Retrieves the ObjectType for a particular ID
  virtual ObjectType objectType(ObjectId id) const = 0;

  /// Retrieves the host entity ID for a particular ID (i.e. a beam, given a gate ID; a platform, given a LOB ID)
  virtual ObjectId entityHostId(ObjectId childId) const = 0;

  /// Retrieves the time bounds for a particular entity ID (first point, last point)
  virtual std::pair<double, double> timeBounds(ObjectId entityId) const = 0;
  ///@}

  /**@name Scenario Properties
   * @note should always return a valid object (never NULL)
   * @{
   */
  virtual const  ScenarioProperties*          scenarioProperties(Transaction *transaction) const = 0;
  virtual        ScenarioProperties*  mutable_scenarioProperties(Transaction *transaction) = 0;

  /**@name Object Properties
   * @note will return NULL if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformProperties*          platformProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual const      BeamProperties*              beamProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual const      GateProperties*              gateProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual const     LaserProperties*             laserProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual const ProjectorProperties*         projectorProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual const  LobGroupProperties*          lobGroupProperties(ObjectId id, Transaction *transaction) const = 0;
  virtual        PlatformProperties*  mutable_platformProperties(ObjectId id, Transaction *transaction) = 0;
  virtual            BeamProperties*      mutable_beamProperties(ObjectId id, Transaction *transaction) = 0;
  virtual            GateProperties*      mutable_gateProperties(ObjectId id, Transaction *transaction) = 0;
  virtual           LaserProperties*     mutable_laserProperties(ObjectId id, Transaction *transaction) = 0;
  virtual       ProjectorProperties* mutable_projectorProperties(ObjectId id, Transaction *transaction) = 0;
  virtual        LobGroupProperties*  mutable_lobGroupProperties(ObjectId id, Transaction *transaction) = 0;
  ///@}

  /**@name Object Preferences
   * @note will return NULL if no object is associated with the specified id
   * @{
   */
  virtual const  PlatformPrefs*          platformPrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const      BeamPrefs*              beamPrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const      GatePrefs*              gatePrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const     LaserPrefs*             laserPrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const ProjectorPrefs*         projectorPrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const  LobGroupPrefs*          lobGroupPrefs(ObjectId id, Transaction *transaction) const = 0;
  virtual const    CommonPrefs*            commonPrefs(ObjectId id, Transaction* transaction) const = 0;
  virtual        PlatformPrefs*  mutable_platformPrefs(ObjectId id, Transaction *transaction) = 0;
  virtual            BeamPrefs*      mutable_beamPrefs(ObjectId id, Transaction *transaction) = 0;
  virtual            GatePrefs*      mutable_gatePrefs(ObjectId id, Transaction *transaction) = 0;
  virtual           LaserPrefs*     mutable_laserPrefs(ObjectId id, Transaction *transaction) = 0;
  virtual       ProjectorPrefs* mutable_projectorPrefs(ObjectId id, Transaction *transaction) = 0;
  virtual        LobGroupPrefs*  mutable_lobGroupPrefs(ObjectId id, Transaction *transaction) = 0;
  virtual          CommonPrefs*    mutable_commonPrefs(ObjectId id, Transaction* transaction) = 0;
  ///@}

 /**@name Set default pref values
   * @note provide default values for each pref type
   * @{
   */

  /** Set all default prefs at one time. */
  virtual void setDefaultPrefs(
    const PlatformPrefs& platformPrefs,
    const BeamPrefs& beamPrefs,
    const GatePrefs& gatePrefs,
    const LaserPrefs& laserPrefs,
    const LobGroupPrefs& lobPrefs,
    const ProjectorPrefs& projectorPrefs) = 0;

  /**
   * Set the default prefs for newly created platforms.  New platforms will start
   * with these values.  This has no impact on already-created entities.
   */
  virtual void setDefaultPrefs(const PlatformPrefs& platformPrefs) = 0;

  /** Retrieves the default preferences used to initialize newly created platforms. */
  virtual PlatformPrefs defaultPlatformPrefs() const = 0;

  ///@}

  /**@name Add a platform, beam, gate, or laser
   * @note Returns properties object to be initialized.
   *  A unique id is generated internally and should not be changed.
   *  The original id field should be used for any user generated ids.
   * @{
   */
  virtual  PlatformProperties* addPlatform(Transaction *transaction) = 0;
  virtual      BeamProperties* addBeam(Transaction *transaction) = 0;
  virtual      GateProperties* addGate(Transaction *transaction) = 0;
  virtual     LaserProperties* addLaser(Transaction *transaction) = 0;
  virtual ProjectorProperties* addProjector(Transaction *transaction) = 0;
  virtual  LobGroupProperties* addLobGroup(Transaction *transaction) = 0;
  ///@}

  /// remove an entity from the data store
  virtual void removeEntity(ObjectId id) = 0;

  /**
   * remove a category data point
   * @param id entity the data is associated with
   * @param time timestamp the data was originally given
   * @param catNameInt integer id of the category name string
   * @param valueInt integer id of the category value string
   * @return 0 if a point was actually removed
   */
  virtual int removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt) = 0;

  /**
   * remove all the generic data associated with a tag
   * @param id entity the data is associated with
   * @param tag The generic data to remove
   * @return 0 if a tag was actually removed
   */
  virtual int removeGenericDataTag(ObjectId id, const std::string& tag) = 0;

  /**@name Add data update, command, generic data, or category data
   *@note Returns NULL if platform for specified ID does not exist
   * @{
   */
  virtual  PlatformUpdate *   addPlatformUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual      BeamUpdate *   addBeamUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual      BeamCommand*   addBeamCommand(ObjectId id, Transaction *transaction) = 0;
  virtual      GateUpdate *   addGateUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual      GateCommand*   addGateCommand(ObjectId id, Transaction *transaction) = 0;
  virtual     LaserUpdate *   addLaserUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual     LaserCommand*   addLaserCommand(ObjectId id, Transaction *transaction) = 0;
  virtual  PlatformCommand*   addPlatformCommand(ObjectId id, Transaction *transaction) = 0;
  virtual  ProjectorUpdate*   addProjectorUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual ProjectorCommand*   addProjectorCommand(ObjectId id, Transaction *transaction) = 0;
  virtual   LobGroupUpdate*   addLobGroupUpdate(ObjectId id, Transaction *transaction) = 0;
  virtual  LobGroupCommand*   addLobGroupCommand(ObjectId id, Transaction *transaction) = 0;
  virtual      GenericData*   addGenericData(ObjectId id, Transaction *transaction) = 0;
  virtual     CategoryData*   addCategoryData(ObjectId id, Transaction *transaction) = 0;
  //virtual        TableData*        addTableData(ObjectId id, Transaction *transaction) = 0;
  ///@}

  /**@name Retrieving read-only data slices
   * @note No locking performed for read-only update slice objects
   * @{
   */
  virtual const PlatformUpdateSlice*   platformUpdateSlice(ObjectId id) const = 0;
  virtual const PlatformCommandSlice*  platformCommandSlice(ObjectId id) const = 0;
  virtual const BeamUpdateSlice*       beamUpdateSlice(ObjectId id) const = 0;
  virtual const BeamCommandSlice*      beamCommandSlice(ObjectId id) const = 0;
  virtual const GateUpdateSlice*       gateUpdateSlice(ObjectId id) const = 0;
  virtual const GateCommandSlice*      gateCommandSlice(ObjectId id) const = 0;
  virtual const LaserUpdateSlice*      laserUpdateSlice(ObjectId id) const = 0;
  virtual const LaserCommandSlice*     laserCommandSlice(ObjectId id) const = 0;
  virtual const ProjectorUpdateSlice*  projectorUpdateSlice(ObjectId id) const = 0;
  virtual const ProjectorCommandSlice* projectorCommandSlice(ObjectId id) const = 0;
  virtual const LobGroupUpdateSlice*   lobGroupUpdateSlice(ObjectId id) const = 0;
  virtual const LobGroupCommandSlice*  lobGroupCommandSlice(ObjectId id) const = 0;
  virtual const GenericDataSlice*      genericDataSlice(ObjectId id) const = 0;
  virtual const CategoryDataSlice*     categoryDataSlice(ObjectId id) const = 0;
  ///@}

  /**
   * Modify commands for a given platform
   * @param id Platform that needs commands modified
   * @param modifier The object to modify the commands
   * @return 0 on success
   */
  virtual int modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier) = 0;

  /**@name Listeners
   * @{
   */
  /// Add or remove a listener for event messages
  virtual void addListener(ListenerPtr callback) = 0;
  virtual void removeListener(ListenerPtr callback) = 0;
  ///@}

  /**@name ScenarioListeners
   * @{
   */
  /// Add or remove a listener for scenario event messages
  virtual void addScenarioListener(ScenarioListenerPtr callback) = 0;
  virtual void removeScenarioListener(ScenarioListenerPtr callback) = 0;
  ///@}

  /**@name Get a handle to the CategoryNameManager
   * @{
   */
  virtual CategoryNameManager& categoryNameManager() const = 0;
  ///@}

  /**
   * Retrieves a reference to the data table manager associated with this data store.
   * The data table manager can be used to create data tables associated with entities,
   * iterate through tables, and add data to existing tables.
   * @return Reference to the data table manager.
   */
  virtual DataTableManager& dataTableManager() const = 0;

protected:
  ///  Interface for all Transaction implementations
  class TransactionImpl
  {
  public:
    virtual ~TransactionImpl() {}

    virtual void  commit() = 0; ///< accept the updates connected to this transaction
    virtual void release() = 0; ///< reject the updates connected to this transaction
  };
}; // End of class DataStore

} // End of namespace simData

#endif // SIMDATA_SCENARIO_H

