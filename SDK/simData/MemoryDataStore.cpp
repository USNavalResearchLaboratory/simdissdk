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
#include <cassert>
#include <algorithm>
#include <functional>
#include <float.h>
#include <limits>
#include "simNotify/Notify.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Time/Clock.h"
#include "simData/MemoryDataStore.h"
#include "simData/DataEntry.h"
#include "simData/DataTypes.h"
#include "simData/DataTable.h"
#include "simData/DataStoreHelpers.h"
#include "simData/EntityNameCache.h"
#include "simData/CategoryData/MemoryCategoryDataSlice.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/MemoryTable/DataLimitsProvider.h"
#include "simData/MemoryTable/TableManager.h"

namespace simData
{

//----------------------------------------------------------------------------
// Functions local to compilation unit, for implementation of common operations
namespace
{
/** helper function for MemoryDataStore::removeEntity()
 *
 * Look for a key of 'id' and, if found, remove it from 'map'
 * if 'deepDelete' then delete the value object
 *
 * @param[inout] map the map to delete from
 * @param[in   ] id id to delete
 * @param[in   ] deepDelete when true, also delete the object in the map
 */
template<typename T>
bool deleteFromMap(std::map<ObjectId, T*> &map, ObjectId id, bool deepDelete = true)
{
  typename std::map<ObjectId, T*>::iterator i = map.find(id);
  if (i != map.end())
  {
    if (deepDelete)
      delete i->second;

    map.erase(i);

    return true;
  }

  return false;
}

/**
 * @param Unique ID (retrieved from MemoryDataStore::genUniqueId_()
 * @param Container (std::map keyed by ID for Platform, Beam, or Gate)
 * @param memory data store
 * @param Pointer to Transaction object (MemoryDataStore::transaction_)
 */
template <typename EntryType,            // PlatformEntry, BeamEntry, GateEntry, LaserEntry, ProjectorEntry
          typename PropertiesType,       // PlatformProperties, BeamProperties, GateProperties, LaserProperties, ProjectorProperties
          typename TransactionImplType,  // Properties transaction implementation type
          typename ListenerListType,     // Type for list of "entry added" observer callbacks (such as the private MemoryDataStore::ListenerList)
          typename PrefType>             // Type for the adding the default pref values
PropertiesType* addEntry(ObjectId id, std::map<ObjectId, EntryType*> *entries, MemoryDataStore *store, DataStore::Transaction *transaction, ListenerListType *listeners, PrefType *defaultPrefs)
{
  assert(transaction);

  EntryType *entry = new EntryType();

  entry->mutable_properties()->set_id(id);

  // Setup transaction
  *transaction = DataStore::Transaction(new TransactionImplType(entry, entries, store, listeners, defaultPrefs, id));

  return entry->mutable_properties();
}

/// Retrieve a pointer to a properties object from different lists
template <typename EntryType,            // PlatformEntry, BeamEntry, GateEntry, const PlatformEntry, const BeamEntry, const GateEntry
          typename EntryMapType>         // Platforms, Beams, Gates, const Platforms, const Beams, const Gates
EntryType* getEntry(ObjectId id, const EntryMapType *store)
{
  // Retrieve the properties object
  typename EntryMapType::const_iterator iter = store->find(id);

  if (iter != store->end())
  {
    return iter->second;
  }

  return NULL;
}

/**
 * Retrieve a constant pointer to a properties object from different lists;
 * requires:
 * Entry type (PlatformEntry, BeamEntry, GateEntry, const PlatformEntry, const BeamEntry, const GateEntry)
 * map type (Platforms, Beams, Gates, const Platforms, const Beams, const Gates)
 * iterator type (const_iterator, or iterator)
 * Transaction type
 */
template <typename EntryType,
          typename EntryMapType,
          typename TransactionImplType>
EntryType* getEntry(ObjectId id, const EntryMapType *store, DataStore::Transaction *transaction)
{
  assert(transaction);
  *transaction = DataStore::Transaction(new TransactionImplType());

  return getEntry<EntryType, EntryMapType>(id, store);
}

/**
 * Update sparse data set slices (GenericData and CategoryData)
 */
template <typename EntryListType>
void updateSparseSlices(EntryListType& entries, double time)
{
  //for each entry
  for (typename EntryListType::const_iterator i = entries.begin(); i != entries.end(); ++i)
  {
    i->second->update(time);
  }
}

/**
* Calls flush on any entries found for the specified id in the entity map, as well as the category and generic data maps
*/
template <typename EntityMap>
void flushEntityData(EntityMap& map, ObjectId id, MemoryDataStore::CategoryDataMap& catMap, MemoryDataStore::GenericDataMap& genMap, bool keepTspiStatic = true)
{
  typename EntityMap::const_iterator i = map.find(id);
  if (i != map.end())
  {
    (*i).second->updates()->flush(keepTspiStatic);
    (*i).second->commands()->flush();
  }
  typename MemoryDataStore::CategoryDataMap::const_iterator ci = catMap.find(id);
  if (ci != catMap.end())
    (*ci).second->flush();
  typename MemoryDataStore::GenericDataMap::const_iterator gi = genMap.find(id);
  if (gi != genMap.end())
    (*gi).second->flush();
}

/** Data limit provider that pulls values out of the data store */
class DataStoreLimits : public MemoryTable::DataLimitsProvider
{
public:
  explicit DataStoreLimits(MemoryDataStore& dataStore)
    : dataStore_(dataStore)
  {
  }

  /** Retrieves limit values for the table (see parent docs) */
  virtual TableStatus getLimits(const simData::DataTable& table, size_t& pointsLimit, double& secondsLimit) const
  {
    // Only provide limits if limiting is enabled
    if (!dataStore_.dataLimiting())
    {
      pointsLimit = 0;
      secondsLimit = 0.0;
      return TableStatus::Success();
    }

    simData::DataStore::Transaction txn;
    simData::ObjectId owner = table.ownerId();

    // Figure out the limit values to use
    if (owner == 0)
      return setLimitValues_(dataStore_.scenarioProperties(&txn), pointsLimit, secondsLimit);
    return setLimitValues_(dataStore_.commonPrefs(owner, &txn), pointsLimit, secondsLimit);
  }

private:
  /** Given T (ScenarioProperties or CommonPrefs), set the pointsLimit and secondsLimit value */
  template <typename T>
  TableStatus setLimitValues_(const T* prefs, size_t& pointsLimit, double& secondsLimit) const
  {
    if (prefs == NULL)
      return TableStatus::Error("No preferences for table's owner entity ID.");
    // Limit by points
    pointsLimit = prefs->datalimitpoints();
    // Limit by seconds
    secondsLimit = prefs->datalimittime();
    // Successful
    return TableStatus::Success();
  }

  simData::DataStore& dataStore_;
};

} // End of anonymous namespace

//----------------------------------------------------------------------------

/** InternalsMemento implementation for MemoryDataStore */
class MemoryDataStore::MemoryInternalsMemento : public InternalsMemento
{
public:
  /** Constructor for a memento for the MemoryDataStore */
  explicit MemoryInternalsMemento(const MemoryDataStore &ds)
  {
    // fill in everything
    interpolator_ = ds.interpolator_;
    interpolationEnabled_ = ds.interpolationEnabled_;

    listeners_ = ds.listeners_;
    scenarioListeners_ = ds.scenarioListeners_;
    ds.dataTableManager().getObservers(dtObservers_);
    ds.categoryNameManager().getListeners(catListeners_);
    defaultPlatformPrefs_.CopyFrom(ds.defaultPlatformPrefs_);
    defaultBeamPrefs_.CopyFrom(ds.defaultBeamPrefs_);
    defaultGatePrefs_.CopyFrom(ds.defaultGatePrefs_);
    defaultLaserPrefs_.CopyFrom(ds.defaultLaserPrefs_);
    defaultLobGroupPrefs_.CopyFrom(ds.defaultLobGroupPrefs_);
    defaultProjectorPrefs_.CopyFrom(ds.defaultProjectorPrefs_);
    defaultCustomRenderingPrefs_.CopyFrom(ds.defaultCustomRenderingPrefs_);
    boundClock_ = ds.boundClock_;
  }

  virtual ~MemoryInternalsMemento()
  {
  }

  /** Sets the values for the incoming data store to match the values saved in this memento */
  virtual void apply(DataStore &ds)
  {
    ds.setInterpolator(interpolator_);
    ds.enableInterpolation(interpolationEnabled_);

    // Add back all listeners
    for (ListenerList::const_iterator iter = listeners_.begin(); iter != listeners_.end(); ++iter)
      ds.addListener(*iter);

    for (ScenarioListenerList::const_iterator iter2 = scenarioListeners_.begin(); iter2 != scenarioListeners_.end(); ++iter2)
      ds.addScenarioListener(*iter2);

    for (std::vector<DataTableManager::ManagerObserverPtr>::const_iterator iter = dtObservers_.begin(); iter != dtObservers_.end(); ++iter)
      ds.dataTableManager().addObserver(*iter);

    for (std::vector<CategoryNameManager::ListenerPtr>::const_iterator iter = catListeners_.begin(); iter != catListeners_.end(); ++iter)
      ds.categoryNameManager().addListener(*iter);

    ds.setDefaultPrefs(defaultPlatformPrefs_, defaultBeamPrefs_, defaultGatePrefs_, defaultLaserPrefs_, defaultLobGroupPrefs_, defaultProjectorPrefs_);
    ds.bindToClock(boundClock_);
  }

private: // data
  Interpolator *interpolator_;
  bool          interpolationEnabled_;

  DataStore::ListenerList listeners_;
  DataStore::ScenarioListenerList scenarioListeners_;
  std::vector<DataTableManager::ManagerObserverPtr> dtObservers_;
  std::vector<CategoryNameManager::ListenerPtr> catListeners_;

  /// Default pref values
  PlatformPrefs defaultPlatformPrefs_;
  BeamPrefs defaultBeamPrefs_;
  GatePrefs defaultGatePrefs_;
  LaserPrefs defaultLaserPrefs_;
  LobGroupPrefs defaultLobGroupPrefs_;
  ProjectorPrefs defaultProjectorPrefs_;
  CustomRenderingPrefs defaultCustomRenderingPrefs_;
  simCore::Clock* boundClock_;
};

///constructor
MemoryDataStore::MemoryDataStore()
: baseId_(0),
  lastUpdateTime_(0.0),
  hasChanged_(false),
  interpolationEnabled_(false),
  interpolator_(NULL),
  timeBounds_(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()),
  dataLimiting_(false),
  categoryNameManager_(new CategoryNameManager),
  dataLimitsProvider_(NULL),
  dataTableManager_(NULL),
  boundClock_(NULL),
  entityNameCache_(new EntityNameCache())
{
  dataLimitsProvider_ = new DataStoreLimits(*this);
  dataTableManager_ = new MemoryTable::TableManager(dataLimitsProvider_);
  genericData_[0] = new MemoryGenericDataSlice();
}

///construct with properties
MemoryDataStore::MemoryDataStore(const ScenarioProperties &properties)
: baseId_(0),
  lastUpdateTime_(0.0),
  hasChanged_(false),
  interpolationEnabled_(false),
  interpolator_(NULL),
  timeBounds_(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()),
  dataLimiting_(false),
  categoryNameManager_(new CategoryNameManager),
  dataLimitsProvider_(NULL),
  dataTableManager_(NULL),
  boundClock_(NULL),
  entityNameCache_(new EntityNameCache())
{
  dataLimitsProvider_ = new DataStoreLimits(*this);
  dataTableManager_ = new MemoryTable::TableManager(dataLimitsProvider_);
  properties_.CopyFrom(properties);
  genericData_[0] = new MemoryGenericDataSlice();
}

///destructor
MemoryDataStore::~MemoryDataStore()
{
  clear();
  delete categoryNameManager_;
  categoryNameManager_ = NULL;
  delete dataTableManager_;
  dataTableManager_ = NULL;
  delete dataLimitsProvider_;
  dataLimitsProvider_ = NULL;
  delete entityNameCache_;
  entityNameCache_ = NULL;
}

void MemoryDataStore::clear()
{

  for (ListenerList::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
  {
    (**i).onScenarioDelete(this);
  }

  deleteEntries_<Platforms>(&platforms_);
  deleteEntries_<Beams>(&beams_);
  deleteEntries_<Gates>(&gates_);
  deleteEntries_<Lasers>(&lasers_);
  deleteEntries_<Projectors>(&projectors_);
  deleteEntries_<LobGroups>(&lobGroups_);
  deleteEntries_<CustomRenderings>(&customRenderings_);
  GenericDataMap::const_iterator it = genericData_.find(0);
  if (it != genericData_.end())
    delete it->second;
  genericData_.clear();
  categoryData_.clear();

  // clear out the category name manager, since categories are scenario specific data
  categoryNameManager_->clear();

  // dataTableManager_ will be cleared out by calls to deleteEntries_()
  // entityNameCache_ will be cleared out by calls to deleteEntries_()
}

DataStore::InternalsMemento* MemoryDataStore::createInternalsMemento() const
{
  return new MemoryInternalsMemento(*this);
}

///@return true if this supports interpolation for updates
bool MemoryDataStore::canInterpolate() const
{
  return true;
}

/**
 * Enable or disable interpolation, if supported
 * Will only succeed if the DataStore implementation supports interpolation and
 * contains a valid interpolator object
 * TODO(millman): this seems like an unnecessary interface, can we just use setInterpolator(NULL) to disable?
 * @return the state of interpolation
 */
bool MemoryDataStore::enableInterpolation(bool state)
{
  // interpolation can only be enabled if there is an interpolator
  if (state && interpolator_ != NULL)
  {
    if (!interpolationEnabled_)
    {
      hasChanged_ = true;
      interpolationEnabled_ = true;
    }
  }
  else
  {
    // no interpolator set, disable
    if (interpolationEnabled_)
    {
      interpolationEnabled_ = false;
      hasChanged_ = true;
    }
  }

  return interpolationEnabled_;
}

///Indicates that interpolation is either enabled or disabled
bool MemoryDataStore::isInterpolationEnabled() const
{
  return interpolationEnabled_ && interpolator_ != NULL;
}

///Specifies the interpolator
void MemoryDataStore::setInterpolator(Interpolator *interpolator)
{
  if (interpolator_ != interpolator)
  {
    interpolator_ = interpolator;
    hasChanged_ = true;
  }
}

/// Get the current interpolator (NULL if disabled)
Interpolator* MemoryDataStore::interpolator() const
{
  return (interpolationEnabled_) ? interpolator_ : NULL;
}

void MemoryDataStore::updatePlatforms_(double time)
{
  // determine if we are in "file mode"
  // treat file mode as the default if no clock has been bound
  const bool fileMode = (!boundClock_ || (boundClock_->mode()==simCore::Clock::MODE_STEP || boundClock_->mode() == simCore::Clock::MODE_REALTIME));

  for (Platforms::const_iterator iter = platforms_.begin(); iter != platforms_.end(); ++iter)
  {
    PlatformEntry* platform = iter->second;
    // apply commands
    platform->commands()->update(this, iter->first, time);

    if (!platform->preferences()->commonprefs().datadraw())
    {
      // until we have datadraw, send NULL; once we have datadraw, we'll immediately update with valid data
      platform->updates()->setCurrent(NULL);
      continue;
    }

    if (fileMode)
    {
      const PlatformUpdateSlice* slice = platform->updates();
      const double firstTime = slice->firstTime();
      const bool staticPlatform = (firstTime == -1.0);
      // do we need to expire a non-static platform?
      if (!staticPlatform && (time < firstTime || time > slice->lastTime()))
      {
        // platform is not valid/has expired
        platform->updates()->setCurrent(NULL);
        continue;
      }
    }

    if (isInterpolationEnabled() && platform->preferences()->interpolatepos())
      platform->updates()->update(time, interpolator_);
    else
      platform->updates()->update(time);
  }
}

void MemoryDataStore::updateTargetBeam_(ObjectId id, BeamEntry* beam, double time)
{
  // Get the two platforms, if available
  if (!beam->properties()->has_hostid())
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  if (!beam->preferences()->has_targetid())
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  Platforms::iterator plat = platforms_.find(beam->properties()->hostid());
  if ((plat == platforms_.end()) || (plat->second == NULL))
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  PlatformEntry* sourcePlatform = plat->second;
  const PlatformUpdate* sourceUpdate = sourcePlatform->updates()->current();
  if ((sourceUpdate == NULL) || (!sourceUpdate->has_position()))
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  plat = platforms_.find(beam->preferences()->targetid());
  if ((plat == platforms_.end()) || (plat->second == NULL))
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  PlatformEntry* destPlatform = plat->second;
  const PlatformUpdate* destUpdate = destPlatform->updates()->current();
  if ((destUpdate == NULL) || (!destUpdate->has_position()))
  {
    beam->updates()->setCurrent(NULL);
    return;
  }

  // target beam has no updates; it uses the currentInterpolated() to deliver info to simVis::Beam
  BeamUpdate* update = beam->updates()->currentInterpolated();

  // simVis::Beam will calculate the RAE whenever it gets a non-NULL update with hasChanged flag set

  // update only when there is a time change or this is a NULL->non-NULL transition
  if (beam->updates()->current() == NULL || update->time() != time)
  {
    update->set_time(time);
    update->set_azimuth(0.0);
    update->set_elevation(0.0);
    update->set_range(0.0);
    beam->updates()->setCurrent(update);
    // signal that this slice is updated, necessary for the time-change case
    beam->updates()->setChanged();
  }
  else
    beam->updates()->clearChanged();
}

void MemoryDataStore::updateBeams_(double time)
{
  for (Beams::iterator iter = beams_.begin(); iter != beams_.end(); ++iter)
  {
    BeamEntry* beamEntry = iter->second;
    // apply commands
    beamEntry->commands()->update(this, iter->first, time);

    // until we have datadraw, send NULL; once we have datadraw, we'll immediately update with valid data
    if (!beamEntry->preferences()->commonprefs().datadraw())
      beamEntry->updates()->setCurrent(NULL);
    else if (beamEntry->properties()->type() == BeamProperties_BeamType_TARGET)
      updateTargetBeam_(iter->first, beamEntry, time);
    else if (isInterpolationEnabled() && beamEntry->preferences()->interpolatebeampos())
      beamEntry->updates()->update(time, interpolator_);
    else
      beamEntry->updates()->update(time);
  }
}

simData::MemoryDataStore::BeamEntry* MemoryDataStore::getBeamForGate_(google::protobuf::uint64 gateID)
{
  Beams::iterator beamIt = beams_.find(gateID);
  if ((beamIt == beams_.end()) || (beamIt->second == NULL))
    return NULL;

  return beamIt->second;
}

void MemoryDataStore::updateTargetGate_(GateEntry* gate, double time)
{
  // this should only be called for target gates; if assert fails, check caller
  assert(gate->properties()->type() == GateProperties_GateType_TARGET);

  // Get the host beam for this gate
  if (!gate->properties()->has_hostid())
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  BeamEntry* beam = getBeamForGate_(gate->properties()->hostid());
  // target gates can only be hosted by target beams. if assert fails, run away.
  assert(beam->properties()->type() == BeamProperties_BeamType_TARGET);
  if (!beam || !beam->properties()->has_hostid() || beam->properties()->type() != BeamProperties_BeamType_TARGET || !beam->preferences()->has_targetid())
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  Platforms::iterator plat = platforms_.find(beam->properties()->hostid());
  if ((plat == platforms_.end()) || (plat->second == NULL))
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  PlatformEntry* sourcePlatform = plat->second;
  const PlatformUpdate* sourceUpdate = sourcePlatform->updates()->current();
  if ((sourceUpdate == NULL) || (!sourceUpdate->has_position()))
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  plat = platforms_.find(beam->preferences()->targetid());
  if ((plat == platforms_.end()) || (plat->second == NULL))
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  PlatformEntry* destPlatform = plat->second;
  const PlatformUpdate* destUpdate = destPlatform->updates()->current();
  if ((destUpdate == NULL) || (!destUpdate->has_position()))
  {
    gate->updates()->setCurrent(NULL);
    return;
  }

  const bool gateWasOff = (gate->updates()->current() == NULL);

  // target gates use the MemoryDataSlice::currentInterpolated_ to hold the modified update
  GateUpdate* update = gate->updates()->currentInterpolated();
  // at this point we have not updated, so the update can tell us the last time that this gate was updated
  const double lastUpdateTime = update->time();

  // target gates do have updates; they specify the minrange/maxrange/centroid for the gate, which are relative to the target beam az/el
  if (isInterpolationEnabled() && gate->preferences()->interpolategatepos())
    gate->updates()->update(time, interpolator_);
  else
    gate->updates()->update(time);
  const GateUpdate* currentUpdate = gate->updates()->current();
  if (currentUpdate == NULL)
    return;

  // update only when gate was off, there is a time change, or if we depend on beam for height/width
  if (gateWasOff || lastUpdateTime != time || gateUsesBeamBeamwidth_(gate))
  {
    // target gates use the az/el from the simVis::Beam target beam calc; this is done in simVis::Gate.
    // using the minrange/maxrange and centroid values from the currentUpdate that we are supplying here
    // minrange/maxrange and centroid are expected to be specified as -/+ offsets from the target range
    // az and el are ignored for target gate updates
    update->set_time(time);
    update->set_azimuth(0.0);
    update->set_elevation(0.0);
    update->set_minrange(currentUpdate->minrange());
    update->set_maxrange(currentUpdate->maxrange());
    // centroid is optional
    if (currentUpdate->has_centroid())
      update->set_centroid(currentUpdate->centroid());
    else
      update->clear_centroid();
    gate->updates()->setCurrent(update);
    // signal that this slice is updated, necessary for the time-change case and for using BeamBeamwidth case
    gate->updates()->setChanged();
  }
  else
    gate->updates()->clearChanged();
}

bool MemoryDataStore::gateUsesBeamBeamwidth_(GateEntry* gate) const
{
  const GateUpdate* currentUpdate = gate->updates()->current();
  return (currentUpdate != NULL && gate->properties()->has_hostid() &&
    (currentUpdate->height() <= 0.0 || currentUpdate->width() <= 0.0));
}

void MemoryDataStore::updateGates_(double time)
{
  for (Gates::iterator iter = gates_.begin(); iter != gates_.end(); ++iter)
  {
    GateEntry* gateEntry = iter->second;
    // apply commands
    gateEntry->commands()->update(this, iter->first, time);

    // until we have datadraw, send NULL; once we have datadraw, we'll immediately update with valid data
    if (!gateEntry->preferences()->commonprefs().datadraw())
      gateEntry->updates()->setCurrent(NULL);
    else if (gateEntry->properties()->type() == GateProperties_GateType_TARGET)
      updateTargetGate_(gateEntry, time);
    else
    {
      if (isInterpolationEnabled() && gateEntry->preferences()->interpolategatepos())
        gateEntry->updates()->update(time, interpolator_);
      else
        gateEntry->updates()->update(time);

      if (gateUsesBeamBeamwidth_(gateEntry))
      {
        // this gate depends on beam prefs; either
        //   force an update of the gate every iteration, or
        //   update gate when there is a change in beam pref height or width

        // force an update of the gate every iteration
        gateEntry->updates()->setChanged();
      }
    }
  }
}

void MemoryDataStore::updateLasers_(double time)
{
  for (Lasers::iterator iter = lasers_.begin(); iter != lasers_.end(); ++iter)
  {
    LaserEntry* laserEntry = iter->second;
    // apply commands
    laserEntry->commands()->update(this, iter->first, time);

    // until we have datadraw, send NULL; once we have datadraw, we'll immediately update with valid data
    if (!laserEntry->preferences()->commonprefs().datadraw())
      laserEntry->updates()->setCurrent(NULL);
    // laser interpolation is on, there is no preference; but off if we have no interpolator
    else if (isInterpolationEnabled())
      laserEntry->updates()->update(time, interpolator_);
    else
      laserEntry->updates()->update(time);
  }
}

void MemoryDataStore::updateProjectors_(double time)
{
  for (Projectors::iterator iter = projectors_.begin(); iter != projectors_.end(); ++iter)
  {
    ProjectorEntry* projectorEntry = iter->second;
    // apply commands
    projectorEntry->commands()->update(this, iter->first, time);

    if (isInterpolationEnabled() && projectorEntry->preferences()->interpolateprojectorfov())
      projectorEntry->updates()->update(time, interpolator_);
    else
      projectorEntry->updates()->update(time);
  }
}

void MemoryDataStore::updateLobGroups_(double time)
{
  //for each entry
  for (LobGroups::iterator iter = lobGroups_.begin(); iter != lobGroups_.end(); ++iter)
  {
    // apply commands
    iter->second->commands()->update(this, iter->first, time);

    // check for changes in maxdatapoints or maxdataseconds prefs, memoryDataSlice processes these.
    {
      DataStore::Transaction tn;
      const LobGroupPrefs* lobPrefs = lobGroupPrefs(iter->first, &tn);
      if (lobPrefs)
      {
        iter->second->updates()->setMaxDataPoints(static_cast<size_t>(lobPrefs->maxdatapoints()));
        iter->second->updates()->setMaxDataSeconds(lobPrefs->maxdataseconds());
      }
    }

    // update the slice
    iter->second->updates()->update(time);
  }
}

void MemoryDataStore::updateCustomRenderings_(double time)
{
  //for each entry
  for (auto iter = customRenderings_.begin(); iter != customRenderings_.end(); ++iter)
  {
    // apply commands
    iter->second->commands()->update(this, iter->first, time);
  }
}

void MemoryDataStore::flushEntity_(ObjectId flushId, simData::ObjectType type, FlushType flushType)
{
  bool recursive = (flushType == RECURSIVE);
  bool keepTspiStatic = !(flushType == NON_RECURSIVE_TSPI_STATIC);
  IdList ids;
  switch (type)
  {
  case PLATFORM:
    flushEntityData(platforms_, flushId, categoryData_, genericData_, keepTspiStatic);
    if (recursive)
    {
      beamIdListForHost(flushId, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::BEAM, flushType);
      ids.clear();
      laserIdListForHost(flushId, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::LASER, flushType);
      ids.clear();
      lobGroupIdListForHost(flushId, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::LOB_GROUP, flushType);
      ids.clear();
      projectorIdListForHost(flushId, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::PROJECTOR, flushType);
    }
    break;
  case BEAM:
    flushEntityData(beams_, flushId, categoryData_, genericData_);
    if (recursive)
    {
      gateIdListForHost(flushId, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::GATE, flushType);
    }
    break;
  case GATE:
    flushEntityData(gates_, flushId, categoryData_, genericData_);
    break;
  case LASER:
    flushEntityData(lasers_, flushId, categoryData_, genericData_);
    break;
  case LOB_GROUP:
    flushEntityData(lobGroups_, flushId, categoryData_, genericData_);
    break;
  case PROJECTOR:
    flushEntityData(projectors_, flushId, categoryData_, genericData_);
    break;
  case CUSTOM_RENDERING:
    flushEntityData(customRenderings_, flushId, categoryData_, genericData_);
    break;
  case ALL:
  case NONE:
    break;
  }

  // Only recursive flush deletes table data
  if (recursive)
    flushDataTables_(flushId);
}

void MemoryDataStore::flushDataTables_(ObjectId id)
{
  /// Defines a visitor function that flushes tables
  class FlushVisitor : public simData::TableList::Visitor
  {
  public:
    FlushVisitor() {}
    virtual void visit(simData::DataTable* table)
    {
      table->flush();
    }
  };

  // Visit all tables and flush them
  const simData::TableList* ownerTables = dataTableManager().tablesForOwner(id);
  if (ownerTables != NULL)
  {
    FlushVisitor flushVisitor;
    ownerTables->accept(flushVisitor);
  }
}

void MemoryDataStore::setDefaultPrefs(const PlatformPrefs& platformPrefs, const BeamPrefs& beamPrefs, const GatePrefs& gatePrefs, const LaserPrefs& laserPrefs, const LobGroupPrefs& lobPrefs, const ProjectorPrefs& projectorPrefs)
{
  defaultPlatformPrefs_.CopyFrom(platformPrefs);
  defaultBeamPrefs_.CopyFrom(beamPrefs);
  defaultGatePrefs_.CopyFrom(gatePrefs);
  defaultLaserPrefs_.CopyFrom(laserPrefs);
  defaultLobGroupPrefs_.CopyFrom(lobPrefs);
  defaultProjectorPrefs_.CopyFrom(projectorPrefs);
  defaultCustomRenderingPrefs_.CopyFrom(CustomRenderingPrefs());
}

void MemoryDataStore::setDefaultPrefs(const PlatformPrefs& platformPrefs)
{
  defaultPlatformPrefs_.CopyFrom(platformPrefs);
}

simData::PlatformPrefs MemoryDataStore::defaultPlatformPrefs() const
{
  return defaultPlatformPrefs_;
}

///Update internal data to show 'time' as current
void MemoryDataStore::update(double time)
{
  if (!hasChanged_ && time == lastUpdateTime_)
    return;

  updatePlatforms_(time);
  updateBeams_(time);
  updateGates_(time);

  updateSparseSlices(genericData_, time);

  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  // for each category data slice
  for (CategoryDataMap::const_iterator i = categoryData_.begin(); i != categoryData_.end(); ++i)
  {
    // if something changed
    if (i->second->update(time))
    {
      // send notification
      const simData::ObjectType ot = objectType(i->first);

      for (ListenerList::const_iterator j = localCopy.begin(); j != localCopy.end(); ++j)
      {
        if (*j != NULL)
        {
          (**j).onCategoryDataChange(this, i->first, ot);
          checkForRemoval_(localCopy);
        }
      }
    }
  }

  updateLasers_(time);
  updateProjectors_(time);
  updateLobGroups_(time);

  // After all the slice updates, set the new update time and notify observers
  lastUpdateTime_ = time;
  hasChanged_ = false;

  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != NULL)
    {
      (**i).onTimeChange(this);
      checkForRemoval_(localCopy);
    }
  }
}

void MemoryDataStore::bindToClock(simCore::Clock* clock)
{
  boundClock_ = clock;
}

simCore::Clock* MemoryDataStore::getBoundClock() const
{
  return boundClock_;
}

/// @return last value of update(double)
double MemoryDataStore::updateTime() const
{
  return lastUpdateTime_;
}

int MemoryDataStore::referenceYear() const
{
  return static_cast<int>(properties_.referenceyear());
}

void MemoryDataStore::setDataLimiting(bool dataLimiting)
{
  dataLimiting_ = dataLimiting;
}

bool MemoryDataStore::dataLimiting() const
{
  return dataLimiting_;
}

void MemoryDataStore::flush(ObjectId flushId, FlushType flushType)
{
  hasChanged_ = true;

  simData::ObjectType objType = simData::ALL;
  if (flushId > 0)
    objType = objectType(flushId);

  if (objType == ALL)
  {
    for (Platforms::const_iterator iter = platforms_.begin(); iter != platforms_.end(); ++iter)
      flushEntity_(iter->first, simData::PLATFORM, RECURSIVE);
    flushDataTables_(0);
    GenericDataMap::const_iterator it = genericData_.find(0);
    if (it != genericData_.end())
      it->second->flush();
    for (auto iter = customRenderings_.begin(); iter != customRenderings_.end(); ++iter)
      flushEntity_(iter->first, simData::CUSTOM_RENDERING, RECURSIVE);
  }
  else
    flushEntity_(flushId, objType, flushType);

  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  // now send out notification to listeners
  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != NULL)
    {
      (*i)->onFlush(this, flushId);
      checkForRemoval_(localCopy);
    }
  }
}

void MemoryDataStore::applyDataLimiting_(ObjectId id)
{
  if (!dataLimiting_)
    return;
  // first get common prefs object
  Transaction t;
  const CommonPrefs* prefs = commonPrefs(id, &t);

  switch (objectType(id))
  {
  case PLATFORM:
    dataLimit_(platforms_, id, prefs);
    break;
  case BEAM:
    dataLimit_(beams_, id, prefs);
    break;
  case GATE:
    dataLimit_(gates_, id, prefs);
    break;
  case LASER:
    dataLimit_(lasers_, id, prefs);
    break;
  case LOB_GROUP:
    dataLimit_(lobGroups_, id, prefs);
    break;
  case PROJECTOR:
    dataLimit_(projectors_, id, prefs);
    break;
  case CUSTOM_RENDERING:
    dataLimit_(customRenderings_, id, prefs);
    break;
  case ALL:
  case NONE:
    break;
  }
  // now limit generic and category data
  GenericDataMap::const_iterator genIter = genericData_.find(id);
  if (genIter != genericData_.end())
    genIter->second->limitByPrefs(*prefs);

  CategoryDataMap::const_iterator catIter = categoryData_.find(id);
  if (catIter != categoryData_.end())
    catIter->second->limitByPrefs(*prefs);
}

///Retrieve a list of IDs for objects contained by the DataStore
void MemoryDataStore::idList(IdList *ids, simData::ObjectType type) const
{
  if (type & PLATFORM)
  {
    for (Platforms::const_iterator iter = platforms_.begin(); iter != platforms_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & BEAM)
  {
    for (Beams::const_iterator iter = beams_.begin(); iter != beams_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & GATE)
  {
    for (Gates::const_iterator iter = gates_.begin(); iter != gates_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & LASER)
  {
    for (Lasers::const_iterator iter = lasers_.begin(); iter != lasers_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & PROJECTOR)
  {
    for (Projectors::const_iterator iter = projectors_.begin(); iter != projectors_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & LOB_GROUP)
  {
    for (LobGroups::const_iterator iter = lobGroups_.begin(); iter != lobGroups_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }

  if (type & CUSTOM_RENDERING)
  {
    for (auto iter = customRenderings_.begin(); iter != customRenderings_.end(); ++iter)
    {
      ids->push_back(iter->first);
    }
  }
}

/// Retrieve a list of IDs for objects of 'type' with the given name
void MemoryDataStore::idListByName(const std::string& name, IdList* ids, simData::ObjectType type) const
{
  // If null someone is call this routine before entityNameCache_ is made in the constructor
  assert(entityNameCache_ != NULL);
  if (entityNameCache_ == NULL)
    return;

  std::vector<const EntityNameEntry*> entries;
  entityNameCache_->getEntries(name, type, entries);
  for (std::vector<const EntityNameEntry*>::const_iterator it = entries.begin(); it != entries.end(); ++it)
    ids->push_back((*it)->id());
}

namespace
{
  /**
  * Template helper function to map original ID to unique ID
  * @param T Expected to be an associative container with an entity pointer as the value
  * @param entityList Associative container of entities
  * @param ids Container to hold the IDs that match up to the given original ID
  * @param originalId Entities in the list with the given originalId are added to ids
  */
  template <typename T>
  void idsByOriginalId(const T &entityList, DataStore::IdList *ids, uint64_t originalId)
  {
    for (typename T::const_iterator iter = entityList.begin(); iter != entityList.end(); ++iter)
    {
      if (iter->second->properties()->originalid() == originalId)
        ids->push_back(iter->second->properties()->id());
    }
  }
}

/// Retrieve a list of IDs for objects with the given original id
void MemoryDataStore::idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type) const
{
  // Retrieve entities with matching original ID
  if (type & PLATFORM)
    idsByOriginalId(platforms_, ids, originalId);
  if (type & BEAM)
    idsByOriginalId(beams_, ids, originalId);
  if (type & GATE)
    idsByOriginalId(gates_, ids, originalId);
  if (type & LASER)
    idsByOriginalId(lasers_, ids, originalId);
  if (type & PROJECTOR)
    idsByOriginalId(projectors_, ids, originalId);
  if (type & LOB_GROUP)
    idsByOriginalId(lobGroups_, ids, originalId);
  if (type & CUSTOM_RENDERING)
    idsByOriginalId(customRenderings_, ids, originalId);
}

///Retrieve a list of IDs for all beams associated with a platform
void MemoryDataStore::beamIdListForHost(ObjectId hostid, IdList *ids) const
{
  // inefficient for extracting beam ids for a platform host
  // A more efficient method (speed-wise, not memory-wise) is to keep a map of
  // beam ids keyed by host id
  for (Beams::const_iterator iter = beams_.begin(); iter != beams_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieve a list of IDs for all gates associated with a beam
void MemoryDataStore::gateIdListForHost(ObjectId hostid, IdList *ids) const
{
  // inefficient for extracting gate ids for a beam host
  // A more efficient method (speed-wise, not memory-wise) is to keep a map of
  // gate ids keyed by host id
  for (Gates::const_iterator iter = gates_.begin(); iter != gates_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieve a list of IDs for all lasers associated with a platform
void MemoryDataStore::laserIdListForHost(ObjectId hostid, IdList *ids) const
{
  // inefficient for extracting laser ids for a platform host
  // A more efficient method (speed-wise, not memory-wise) is to keep a map of
  // laser ids keyed by host id
  for (Lasers::const_iterator iter = lasers_.begin(); iter != lasers_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieve a list of IDs for all projectors associated with a platform
void MemoryDataStore::projectorIdListForHost(ObjectId hostid, IdList *ids) const
{
  // inefficient for extracting projector ids for a platform host
  // A more efficient method (speed-wise, not memory-wise) is to keep a map of
  // projector ids keyed by host id
  for (Projectors::const_iterator iter = projectors_.begin(); iter != projectors_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieve a list of IDs for all lobGroups associated with a platform
void MemoryDataStore::lobGroupIdListForHost(ObjectId hostid, IdList *ids) const
{
  // inefficient for extracting lobGroup ids for a platform host
  // A more efficient method (speed-wise, not memory-wise) is to keep a map of
  // lobGroup ids keyed by host id
  for (LobGroups::const_iterator iter = lobGroups_.begin(); iter != lobGroups_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieve a list of IDs for all customs associated with a platform
void MemoryDataStore::customRenderingIdListForHost(ObjectId hostid, IdList *ids) const
{
  for (auto iter = customRenderings_.begin(); iter != customRenderings_.end(); ++iter)
  {
    if (iter->second->properties()->hostid() == hostid)
    {
      ids->push_back(iter->first);
    }
  }
}

///Retrieves the ObjectType for a particular ID
simData::ObjectType MemoryDataStore::objectType(ObjectId id) const
{
  if (platforms_.find(id) != platforms_.end())
    return simData::PLATFORM;
  if (beams_.find(id) != beams_.end())
    return simData::BEAM;
  if (gates_.find(id) != gates_.end())
    return simData::GATE;
  if (lasers_.find(id) != lasers_.end())
    return simData::LASER;
  if (projectors_.find(id) != projectors_.end())
    return simData::PROJECTOR;
  if (lobGroups_.find(id) != lobGroups_.end())
    return simData::LOB_GROUP;
  if (customRenderings_.find(id) != customRenderings_.end())
    return simData::CUSTOM_RENDERING;
  return simData::NONE;
}

///Retrieves the host ID for an entity; returns 0 for platforms, or for not found
ObjectId MemoryDataStore::entityHostId(ObjectId childId) const
{
  simData::ObjectType objType = objectType(childId);
  Transaction t;
  switch (objType)
  {
  case simData::PLATFORM:
  case simData::NONE:
  case simData::ALL:
    break;
  case simData::BEAM:
    return beamProperties(childId, &t)->hostid();
  case simData::GATE:
    return gateProperties(childId, &t)->hostid();
  case simData::LASER:
    return laserProperties(childId, &t)->hostid();
  case simData::PROJECTOR:
    return projectorProperties(childId, &t)->hostid();
  case simData::LOB_GROUP:
    return lobGroupProperties(childId, &t)->hostid();
  case simData::CUSTOM_RENDERING:
    return customRenderingProperties(childId, &t)->hostid();
  }
  return 0;
}

///@return immutable ScenarioProperties object
const ScenarioProperties* MemoryDataStore::scenarioProperties(Transaction *transaction) const
{
  assert(transaction);
  *transaction = Transaction(new NullTransactionImpl());
  return &properties_;
}

/* mutable version */
ScenarioProperties* MemoryDataStore::mutable_scenarioProperties(Transaction *transaction)
{
  assert(transaction);
  ScenarioSettingsTransactionImpl* rv = new ScenarioSettingsTransactionImpl(&properties_, this, &scenarioListeners_);
  *transaction = Transaction(rv);
  return rv->settings();
}

/**
 * @return platform properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
PlatformProperties* MemoryDataStore::addPlatform(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  PlatformProperties* rv = addEntry<PlatformEntry,
                              PlatformProperties,
                              NewEntryTransactionImpl<PlatformEntry, PlatformPrefs>,
                              ListenerList>(id, &platforms_, this, transaction, &listeners_, &defaultPlatformPrefs_);
  entityNameCache_->addEntity(defaultPlatformPrefs_.commonprefs().name(), id, simData::PLATFORM);
  return rv;
}

/**
 * @return beam properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
BeamProperties* MemoryDataStore::addBeam(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  BeamProperties* rv = addEntry<BeamEntry,
                          BeamProperties,
                          NewEntryTransactionImpl<BeamEntry, BeamPrefs>,
                          ListenerList>(id, &beams_, this, transaction, &listeners_, &defaultBeamPrefs_);
  entityNameCache_->addEntity(defaultBeamPrefs_.commonprefs().name(), id, simData::BEAM);
  return rv;
}

/**
 * @return gate properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
GateProperties* MemoryDataStore::addGate(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  GateProperties* rv = addEntry<GateEntry,
                          GateProperties,
                          NewEntryTransactionImpl<GateEntry, GatePrefs>,
                          ListenerList>(id, &gates_, this, transaction, &listeners_, &defaultGatePrefs_);
  entityNameCache_->addEntity(defaultGatePrefs_.commonprefs().name(), id, simData::GATE);
  return rv;
}

/**
 * @return laser properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
LaserProperties* MemoryDataStore::addLaser(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  LaserProperties* rv = addEntry<LaserEntry,
                            LaserProperties,
                            NewEntryTransactionImpl<LaserEntry, LaserPrefs>,
                            ListenerList>(id, &lasers_, this, transaction, &listeners_, &defaultLaserPrefs_);
  entityNameCache_->addEntity(defaultLaserPrefs_.commonprefs().name(), id, simData::LASER);
  return rv;
}

/**
 * @return projector properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
ProjectorProperties* MemoryDataStore::addProjector(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  ProjectorProperties* rv = addEntry<ProjectorEntry,
                                ProjectorProperties,
                                NewEntryTransactionImpl<ProjectorEntry, ProjectorPrefs>,
                                ListenerList>(id, &projectors_, this, transaction, &listeners_, &defaultProjectorPrefs_);
  entityNameCache_->addEntity(defaultProjectorPrefs_.commonprefs().name(), id, simData::PROJECTOR);
  return rv;
}

/**
 * @return lobGroup properties object to be initialized. A unique id is generated
 * internally and should not be changed.  The original id field should be used
 * for any user generated ids.
 */
LobGroupProperties* MemoryDataStore::addLobGroup(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  LobGroupProperties* rv = addEntry<LobGroupEntry,
                              LobGroupProperties,
                              NewEntryTransactionImpl<LobGroupEntry, LobGroupPrefs>,
                              ListenerList>(id, &lobGroups_, this, transaction, &listeners_, &defaultLobGroupPrefs_);
  entityNameCache_->addEntity(defaultLobGroupPrefs_.commonprefs().name(), id, simData::LOB_GROUP);
  return rv;
}

CustomRenderingProperties* MemoryDataStore::addCustomRendering(Transaction *transaction)
{
  simData::ObjectId id = genUniqueId_();
  CustomRenderingProperties* rv = addEntry<CustomRenderingEntry,
    CustomRenderingProperties,
    NewEntryTransactionImpl<CustomRenderingEntry, CustomRenderingPrefs>,
    ListenerList>(id, &customRenderings_, this, transaction, &listeners_, &defaultCustomRenderingPrefs_);
  entityNameCache_->addEntity(defaultCustomRenderingPrefs_.commonprefs().name(), id, simData::CUSTOM_RENDERING);
  return rv;
}

void MemoryDataStore::removeEntity(ObjectId id)
{
  const simData::ObjectType ot = objectType(id);
  if (ot == NONE)
    return; // entity with given id not found

  hasChanged_ = true;

  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != NULL)
    {
      (**i).onRemoveEntity(this, id, ot);
      checkForRemoval_(localCopy);
    }
  }

  entityNameCache_->removeEntity(simData::DataStoreHelpers::nameFromId(id, this), id, ot);

  // do not delete the objects pointed to by the GD and CD maps
  // those pointers point into regions of the entity structure - not objects on the heap
  deleteFromMap(genericData_, id, false);
  deleteFromMap(categoryData_, id, false);
  dataTableManager().deleteTablesByOwner(id);

  IdList ids; // for things attached to this entity

  // once we've found the item in an entity-type list, we are done

  Platforms::iterator pi = platforms_.find(id);
  if (pi != platforms_.end())
  {
    // also delete everything attached to the platform
    // we will need to send notifications and recurse on them as well...
    beamIdListForHost(id, &ids);
    laserIdListForHost(id, &ids);
    projectorIdListForHost(id, &ids);
    lobGroupIdListForHost(id, &ids);
    customRenderingIdListForHost(id, &ids);

    for (IdList::const_iterator i = ids.begin(); i != ids.end(); ++i)
      removeEntity(*i);

    delete pi->second;
    platforms_.erase(pi);
    return;
  }

  Beams::iterator bi = beams_.find(id);
  if (bi != beams_.end())
  {
    // also delete any gates
    gateIdListForHost(id, &ids);
    // we will need to send notifications and recurse on them as well...
    for (IdList::const_iterator i = ids.begin(); i != ids.end(); ++i)
      removeEntity(*i);

    delete bi->second;
    beams_.erase(bi);
    return;
  }

  if (deleteFromMap(gates_, id))
    return;

  if (deleteFromMap(lasers_, id))
    return;

  if (deleteFromMap(projectors_, id))
    return;

  if (deleteFromMap(lobGroups_, id))
    return;

  if (deleteFromMap(customRenderings_, id))
    return;
}

int MemoryDataStore::removeCategoryDataPoint(ObjectId id, double time, int catNameInt, int valueInt)
{
  MemoryCategoryDataSlice *slice = getEntry<MemoryCategoryDataSlice, CategoryDataMap>(id, &categoryData_);
  if (!slice)
    return -1;

  hasChanged_ = true;
  return slice->removePoint(time, catNameInt, valueInt) ? 0 : 1;
}

int MemoryDataStore::removeGenericDataTag(ObjectId id, const std::string& tag)
{
  MemoryGenericDataSlice *slice = getEntry<MemoryGenericDataSlice, GenericDataMap>(id, &genericData_);
  if (!slice)
    return -1;

  hasChanged_ = true;
  return slice->removeTag(tag);
}

///@return const properties of platform corresponding to 'id'
const PlatformProperties* MemoryDataStore::platformProperties(ObjectId id, Transaction *transaction) const
{
  const PlatformEntry *entry = getEntry<const PlatformEntry, Platforms, NullTransactionImpl>(id, &platforms_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
PlatformProperties* MemoryDataStore::mutable_platformProperties(ObjectId id, Transaction *transaction)
{
  PlatformEntry *entry = getEntry<PlatformEntry, Platforms, NullTransactionImpl>(id, &platforms_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

///@return const properties of beam with 'id'
const BeamProperties *MemoryDataStore::beamProperties(ObjectId id, Transaction *transaction) const
{
  const BeamEntry *entry = getEntry<const BeamEntry, Beams, NullTransactionImpl>(id, &beams_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
BeamProperties *MemoryDataStore::mutable_beamProperties(ObjectId id, Transaction *transaction)
{
  BeamEntry *entry = getEntry<BeamEntry, Beams, NullTransactionImpl>(id, &beams_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

///@return const properties of gate with 'id'
const GateProperties *MemoryDataStore::gateProperties(ObjectId id, Transaction *transaction) const
{
  const GateEntry *entry = getEntry<const GateEntry, Gates, NullTransactionImpl>(id, &gates_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
GateProperties *MemoryDataStore::mutable_gateProperties(ObjectId id, Transaction *transaction)
{
  GateEntry *entry = getEntry<GateEntry, Gates, NullTransactionImpl>(id, &gates_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

///@return const properties of laser with 'id'
const LaserProperties* MemoryDataStore::laserProperties(ObjectId id, Transaction *transaction) const
{
  const LaserEntry *entry = getEntry<const LaserEntry, Lasers, NullTransactionImpl>(id, &lasers_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
LaserProperties* MemoryDataStore::mutable_laserProperties(ObjectId id, Transaction *transaction)
{
  LaserEntry *entry = getEntry<LaserEntry, Lasers, NullTransactionImpl>(id, &lasers_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

///@return const properties of projector with 'id'
const ProjectorProperties* MemoryDataStore::projectorProperties(ObjectId id, Transaction *transaction) const
{
  const ProjectorEntry *entry = getEntry<const ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
ProjectorProperties* MemoryDataStore::mutable_projectorProperties(ObjectId id, Transaction *transaction)
{
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

///@return const properties of lobGroup with 'id'
const LobGroupProperties* MemoryDataStore::lobGroupProperties(ObjectId id, Transaction *transaction) const
{
  const LobGroupEntry *entry = getEntry<const LobGroupEntry, LobGroups, NullTransactionImpl>(id, &lobGroups_, transaction);
  return entry ? entry->properties() : NULL;
}

/// mutable version
LobGroupProperties* MemoryDataStore::mutable_lobGroupProperties(ObjectId id, Transaction *transaction)
{
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups, NullTransactionImpl>(id, &lobGroups_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

const CustomRenderingProperties* MemoryDataStore::customRenderingProperties(ObjectId id, Transaction *transaction) const
{
  const CustomRenderingEntry *entry = getEntry<const CustomRenderingEntry, CustomRenderings, NullTransactionImpl>(id, &customRenderings_, transaction);
  return entry ? entry->properties() : NULL;
}

CustomRenderingProperties* MemoryDataStore::mutable_customRenderingProperties(ObjectId id, Transaction *transaction)
{
  CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings, NullTransactionImpl>(id, &customRenderings_, transaction);
  return entry ? entry->mutable_properties() : NULL;
}

const PlatformPrefs* MemoryDataStore::platformPrefs(ObjectId id, Transaction *transaction) const
{
  const PlatformEntry *entry = getEntry<const PlatformEntry, Platforms, NullTransactionImpl>(id, &platforms_, transaction);
  return entry ? entry->preferences() : NULL;
}

PlatformPrefs* MemoryDataStore::mutable_platformPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (entry)
  {
    MutableSettingsTransactionImpl<PlatformPrefs> *impl =
      new MutableSettingsTransactionImpl<PlatformPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Platform associated with specified id was not found
  return NULL;
}

const BeamPrefs* MemoryDataStore::beamPrefs(ObjectId id, Transaction *transaction) const
{
  const BeamEntry *entry = getEntry<const BeamEntry, Beams, NullTransactionImpl>(id, &beams_, transaction);
  return entry ? entry->preferences() : NULL;
}

BeamPrefs* MemoryDataStore::mutable_beamPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (entry)
  {
    MutableSettingsTransactionImpl<BeamPrefs> *impl =
      new MutableSettingsTransactionImpl<BeamPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Beam associated with specified id was not found
  return NULL;
}

const GatePrefs* MemoryDataStore::gatePrefs(ObjectId id, Transaction *transaction) const
{
  const GateEntry *entry = getEntry<const GateEntry, Gates, NullTransactionImpl>(id, &gates_, transaction);
  return entry ? entry->preferences() : NULL;
}

GatePrefs* MemoryDataStore::mutable_gatePrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (entry)
  {
    MutableSettingsTransactionImpl<GatePrefs> *impl =
      new MutableSettingsTransactionImpl<GatePrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Gate associated with specified id was not found
  return NULL;
}

const LaserPrefs* MemoryDataStore::laserPrefs(ObjectId id, Transaction *transaction) const
{
  const LaserEntry *entry = getEntry<const LaserEntry, Lasers, NullTransactionImpl>(id, &lasers_, transaction);
  return entry ? entry->preferences() : NULL;
}

LaserPrefs* MemoryDataStore::mutable_laserPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (entry)
  {
    MutableSettingsTransactionImpl<LaserPrefs> *impl =
      new MutableSettingsTransactionImpl<LaserPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Laser associated with specified id was not found
  return NULL;
}

const ProjectorPrefs* MemoryDataStore::projectorPrefs(ObjectId id, Transaction *transaction) const
{
  const ProjectorEntry *entry = getEntry<const ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  return entry ? entry->preferences() : NULL;
}

ProjectorPrefs* MemoryDataStore:: mutable_projectorPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  if (entry)
  {
    MutableSettingsTransactionImpl<ProjectorPrefs> *impl =
      new MutableSettingsTransactionImpl<ProjectorPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Projector associated with specified id was not found
  return NULL;
}

const LobGroupPrefs* MemoryDataStore::lobGroupPrefs(ObjectId id, Transaction *transaction) const
{
  const LobGroupEntry *entry = getEntry<const LobGroupEntry, LobGroups, NullTransactionImpl>(id, &lobGroups_, transaction);
  return entry ? entry->preferences() : NULL;
}

LobGroupPrefs* MemoryDataStore::mutable_lobGroupPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (entry)
  {
    MutableSettingsTransactionImpl<LobGroupPrefs> *impl =
      new MutableSettingsTransactionImpl<LobGroupPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // LobGroup associated with specified id was not found
  return NULL;
}

const CustomRenderingPrefs* MemoryDataStore::customRenderingPrefs(ObjectId id, Transaction *transaction) const
{
  const CustomRenderingEntry *entry = getEntry<const CustomRenderingEntry, CustomRenderings, NullTransactionImpl>(id, &customRenderings_, transaction);
  return entry ? entry->preferences() : NULL;
}

CustomRenderingPrefs* MemoryDataStore::mutable_customRenderingPrefs(ObjectId id, Transaction *transaction)
{
  assert(transaction);
  CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  if (entry)
  {
    MutableSettingsTransactionImpl<CustomRenderingPrefs> *impl =
      new MutableSettingsTransactionImpl<CustomRenderingPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  return NULL;
}

const CommonPrefs* MemoryDataStore::commonPrefs(ObjectId id, Transaction* transaction) const
{
  const PlatformPrefs* plat = platformPrefs(id, transaction);
  if (plat != NULL)
    return &plat->commonprefs();
  const BeamPrefs* beam = beamPrefs(id, transaction);
  if (beam != NULL)
    return &beam->commonprefs();
  const GatePrefs* gate = gatePrefs(id, transaction);
  if (gate != NULL)
    return &gate->commonprefs();
  const LaserPrefs* laser = laserPrefs(id, transaction);
  if (laser != NULL)
    return &laser->commonprefs();
  const LobGroupPrefs* lobGroup = lobGroupPrefs(id, transaction);
  if (lobGroup != NULL)
    return &lobGroup->commonprefs();
  const ProjectorPrefs* proj = projectorPrefs(id, transaction);
  if (proj != NULL)
    return &proj->commonprefs();
  const CustomRenderingPrefs* custom = customRenderingPrefs(id, transaction);
  if (custom != NULL)
    return &custom->commonprefs();

  return NULL;
}

CommonPrefs* MemoryDataStore::mutable_commonPrefs(ObjectId id, Transaction* transaction)
{
  PlatformPrefs* plat = mutable_platformPrefs(id, transaction);
  if (plat != NULL)
    return plat->mutable_commonprefs();
  BeamPrefs* beam = mutable_beamPrefs(id, transaction);
  if (beam != NULL)
    return beam->mutable_commonprefs();
  GatePrefs* gate = mutable_gatePrefs(id, transaction);
  if (gate != NULL)
    return gate->mutable_commonprefs();
  LaserPrefs* laser = mutable_laserPrefs(id, transaction);
  if (laser != NULL)
    return laser->mutable_commonprefs();
  LobGroupPrefs* lobGroup = mutable_lobGroupPrefs(id, transaction);
  if (lobGroup != NULL)
    return lobGroup->mutable_commonprefs();
  ProjectorPrefs* proj = mutable_projectorPrefs(id, transaction);
  if (proj != NULL)
    return proj->mutable_commonprefs();
  return NULL;
}

///@return NULL if platform for specified 'id' does not exist
PlatformUpdate* MemoryDataStore::addPlatformUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (!entry)
  {
    return NULL;
  }

  PlatformUpdate *update = new PlatformUpdate();

  // Setup transaction
  MemoryDataSlice<PlatformUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<PlatformUpdate, MemoryDataSlice<PlatformUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if platform for specified 'id' does not exist
PlatformCommand *MemoryDataStore::addPlatformCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (!entry)
  {
    return NULL;
  }

  PlatformCommand *command = new PlatformCommand();

  // Setup transaction
  MemoryCommandSlice<PlatformCommand, PlatformPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<PlatformCommand, MemoryCommandSlice<PlatformCommand, PlatformPrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if beam for specified 'id' does not exist
BeamUpdate* MemoryDataStore::addBeamUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (!entry)
  {
    return NULL;
  }

  BeamUpdate *update = new BeamUpdate();

  // Setup transaction
  MemoryDataSlice<BeamUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<BeamUpdate, MemoryDataSlice<BeamUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if beam for specified 'id' does not exist
BeamCommand *MemoryDataStore::addBeamCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (!entry)
  {
    return NULL;
  }

  BeamCommand *command = new BeamCommand();

  // Setup transaction
  MemoryCommandSlice<BeamCommand, BeamPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<BeamCommand, MemoryCommandSlice<BeamCommand, BeamPrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if gate for specified 'id' does not exist
GateUpdate* MemoryDataStore::addGateUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (!entry)
  {
    return NULL;
  }

  GateUpdate *update = new GateUpdate();

  // Setup transaction
  MemoryDataSlice<GateUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<GateUpdate, MemoryDataSlice<GateUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if gate for specified 'id' does not exist
GateCommand *MemoryDataStore::addGateCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (!entry)
  {
    return NULL;
  }

  GateCommand *command = new GateCommand();

  // Setup transaction
  MemoryCommandSlice<GateCommand, GatePrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<GateCommand, MemoryCommandSlice<GateCommand, GatePrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if laser for specified 'id' does not exist
LaserUpdate* MemoryDataStore::addLaserUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (!entry)
  {
    return NULL;
  }

  LaserUpdate *update = new LaserUpdate();

  // Setup transaction
  MemoryDataSlice<LaserUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<LaserUpdate, MemoryDataSlice<LaserUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if laser for specified 'id' does not exist
LaserCommand *MemoryDataStore::addLaserCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (!entry)
  {
    return NULL;
  }

  LaserCommand *command = new LaserCommand();

  // Setup transaction
  MemoryCommandSlice<LaserCommand, LaserPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<LaserCommand, MemoryCommandSlice<LaserCommand, LaserPrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if projector for specified 'id' does not exist
ProjectorUpdate* MemoryDataStore::addProjectorUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  if (!entry)
  {
    return NULL;
  }

  ProjectorUpdate *update = new ProjectorUpdate();

  // Setup transaction
  MemoryDataSlice<ProjectorUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<ProjectorUpdate, MemoryDataSlice<ProjectorUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if projector for specified 'id' does not exist
ProjectorCommand *MemoryDataStore::addProjectorCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  if (!entry)
  {
    return NULL;
  }

  ProjectorCommand *command = new ProjectorCommand();

  // Setup transaction
  MemoryCommandSlice<ProjectorCommand, ProjectorPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<ProjectorCommand, MemoryCommandSlice<ProjectorCommand, ProjectorPrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if lobGroup for specified 'id' does not exist
LobGroupUpdate* MemoryDataStore::addLobGroupUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (!entry)
  {
    return NULL;
  }

  LobGroupUpdate *update = new LobGroupUpdate();

  // Setup transaction
  MemoryDataSlice<LobGroupUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<LobGroupUpdate, MemoryDataSlice<LobGroupUpdate> >(update, slice, this, id));

  return update;
}

///@return NULL if lobGroup for specified 'id' does not exist
LobGroupCommand *MemoryDataStore::addLobGroupCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (!entry)
  {
    return NULL;
  }

  LobGroupCommand *command = new LobGroupCommand();

  // Setup transaction
  MemoryCommandSlice<LobGroupCommand, LobGroupPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<LobGroupCommand, MemoryCommandSlice<LobGroupCommand, LobGroupPrefs> >(command, slice, this, id, false));

  return command;
}

CustomRenderingCommand* MemoryDataStore::addCustomRenderingCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  auto *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  if (!entry)
  {
    return NULL;
  }

  auto *command = new CustomRenderingCommand();

  // Setup transaction
  auto *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<CustomRenderingCommand, MemoryCommandSlice<CustomRenderingCommand, CustomRenderingPrefs> >(command, slice, this, id, false));

  return command;
}

///@return NULL if generic data for specified 'id' does not exist
GenericData* MemoryDataStore::addGenericData(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  MemoryGenericDataSlice *slice = getEntry<MemoryGenericDataSlice, GenericDataMap>(id, &genericData_);
  if (!slice)
  {
    return NULL;
  }

  GenericData *data = new GenericData();

  // Setup transaction
  if (id == 0)
    *transaction = Transaction(new NewScenarioGenericUpdateTransactionImpl<GenericData, MemoryGenericDataSlice>(data, slice, this, id, false));
  else
    *transaction = Transaction(new NewUpdateTransactionImpl<GenericData, MemoryGenericDataSlice>(data, slice, this, id, false));

  return data;
}

///@return NULL if category data for specified 'id' does not exist
CategoryData* MemoryDataStore::addCategoryData(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  MemoryCategoryDataSlice *slice = getEntry<MemoryCategoryDataSlice, CategoryDataMap> (id, &categoryData_);
  if (!slice)
  {
    return NULL;
  }

  CategoryData *data = new CategoryData();

  // Setup transaction
  *transaction = Transaction(new NewUpdateTransactionImpl<CategoryData, MemoryCategoryDataSlice>(data, slice, this, id, false));

  return data;
}

// No locking performed for read-only update list objects
const PlatformUpdateSlice* MemoryDataStore::platformUpdateSlice(ObjectId id) const
{
  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  return entry ? entry->updates() : NULL;
}

const PlatformCommandSlice* MemoryDataStore::platformCommandSlice(ObjectId id) const
{
  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  return entry ? entry->commands() : NULL;
}

const BeamUpdateSlice* MemoryDataStore::beamUpdateSlice(ObjectId id) const
{
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  return entry ? entry->updates() : NULL;
}

const BeamCommandSlice* MemoryDataStore::beamCommandSlice(ObjectId id) const
{
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  return entry ? entry->commands() : NULL;
}

const GateUpdateSlice* MemoryDataStore::gateUpdateSlice(ObjectId id) const
{
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  return entry ? entry->updates() : NULL;
}

const GateCommandSlice* MemoryDataStore::gateCommandSlice(ObjectId id) const
{
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  return entry ? entry->commands() : NULL;
}

const LaserUpdateSlice* MemoryDataStore::laserUpdateSlice(ObjectId id) const
{
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  return entry ? entry->updates() : NULL;
}

const LaserCommandSlice* MemoryDataStore::laserCommandSlice(ObjectId id) const
{
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  return entry ? entry->commands() : NULL;
}

const ProjectorUpdateSlice* MemoryDataStore::projectorUpdateSlice(ObjectId id) const
{
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  return entry ? entry->updates() : NULL;
}

const ProjectorCommandSlice* MemoryDataStore::projectorCommandSlice(ObjectId id) const
{
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  return entry ? entry->commands() : NULL;
}

const LobGroupUpdateSlice* MemoryDataStore::lobGroupUpdateSlice(ObjectId id) const
{
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  return entry ? entry->updates() : NULL;
}

const LobGroupCommandSlice* MemoryDataStore::lobGroupCommandSlice(ObjectId id) const
{
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  return entry ? entry->commands() : NULL;
}

const CustomRenderingCommandSlice* MemoryDataStore::customRenderingCommandSlice(ObjectId id) const
{
  CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  return entry ? entry->commands() : NULL;
}

const GenericDataSlice* MemoryDataStore::genericDataSlice(ObjectId id) const
{
  return getEntry<GenericDataSlice, GenericDataMap>(id, &genericData_);
}

const CategoryDataSlice* MemoryDataStore::categoryDataSlice(ObjectId id) const
{
  return getEntry<CategoryDataSlice, CategoryDataMap>(id, &categoryData_);
}

int MemoryDataStore::modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier)
{
  switch (objectType(id))
  {
  case simData::PLATFORM:
  {
    PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
    if (entry == NULL)
      return 1;
    PlatformCommandSlice* commands = entry->commands();
    commands->modify(modifier);
    hasChanged_ = true;
    break;
  }
  default:
    break;
  }

  return 1;
}

int MemoryDataStore::modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier)
{
  switch (objectType(id))
  {
  case simData::CUSTOM_RENDERING:
  {
    CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
    if (entry == NULL)
      return 1;
    CustomRenderingCommandSlice* commands = entry->commands();
    commands->modify(modifier);
    hasChanged_ = true;
    break;
  }
  default:
    break;
  }

  return 1;
}

namespace
{
  // Template function to add an observer to a list of observers associated with a specific object type
  // Used for all object types: Platform, Beam, Gate, Laser, and Projector
  template<typename Container, typename Callback>
  void addObserver(Container *container, Callback callback)
  {
    // prevent duplicates
    if (find(container->begin(), container->end(), callback) == container->end())
    {
      container->push_back(callback);
    }
  }
}

void MemoryDataStore::addListener(ListenerPtr callback)
{
  listeners_.push_back(callback);
}

void MemoryDataStore::removeListener(ListenerPtr callback)
{
  ListenerList::iterator i = std::find(listeners_.begin(), listeners_.end(), callback);
  if (i != listeners_.end())
  {
    justRemoved_.push_back(callback);
    listeners_.erase(i);
  }
}

void MemoryDataStore::checkForRemoval_(ListenerList& list)
{
  // Should not need to ever call this on listeners_, only on copies of listeners_
  assert(&listeners_ != &list);

  if (justRemoved_.empty())
    return;

  for (ListenerList::iterator just = justRemoved_.begin(); just != justRemoved_.end(); ++just)
  {
    ListenerList::iterator it = std::find(list.begin(), list.end(), *just);
    if (it != list.end())
      (*it).reset();
  }

  justRemoved_.clear();
}

void MemoryDataStore::addScenarioListener(ScenarioListenerPtr callback)
{
  scenarioListeners_.push_back(callback);
}

void MemoryDataStore::removeScenarioListener(ScenarioListenerPtr callback)
{
  ScenarioListenerList::iterator i = std::find(scenarioListeners_.begin(), scenarioListeners_.end(), callback);
  if (i != scenarioListeners_.end())
    scenarioListeners_.erase(i);
}

CategoryNameManager& MemoryDataStore::categoryNameManager() const
{
  return *categoryNameManager_;
}

DataTableManager& MemoryDataStore::dataTableManager() const
{
  return *dataTableManager_;
}

ObjectId MemoryDataStore::genUniqueId_()
{
  return ++baseId_;
}

template <typename EntryMapType>
void MemoryDataStore::deleteEntries_(EntryMapType *entries)
{
  while (!entries->empty())
    removeEntity(entries->begin()->first);
  entries->clear();
}

template <typename EntryMapType>
void MemoryDataStore::dataLimit_(std::map<ObjectId, EntryMapType* >& entryMap, ObjectId id, const CommonPrefs* prefs)
{
  typename std::map<ObjectId, EntryMapType *>::const_iterator iter = entryMap.find(id);
  if (iter == entryMap.end())
    return;
  // limit updates and commands
  iter->second->updates()->limitByPrefs(*prefs);
  iter->second->commands()->limitByPrefs(*prefs);
}

//----------------------------------------------------------------------------
template<typename T>
MemoryDataStore::MutableSettingsTransactionImpl<T>::MutableSettingsTransactionImpl(ObjectId id, T *settings, MemoryDataStore *store, ListenerList *observers)
: id_(id),
  committed_(false),
  notified_(false),
  nameChange_(false),
  currentSettings_(settings),
  store_(store),
  observers_(observers)
{
  // create a copy of currentSettings_ for the user to experiment with
  modifiedSettings_ = currentSettings_->New();
  modifiedSettings_->CopyFrom(*currentSettings_);
}

template<typename T>
void MemoryDataStore::MutableSettingsTransactionImpl<T>::commit()
{
  // performance: skip if there are no changes
  if (modifiedSettings_->SerializeAsString() != currentSettings_->SerializeAsString())
  {
    committed_ = true; // transaction is valid

    // check for name change, if shown or alias change, if shown or change for show name to/from show alias
    if ((!modifiedSettings_->commonprefs().usealias() && modifiedSettings_->commonprefs().name() != currentSettings_->commonprefs().name()) ||
        (modifiedSettings_->commonprefs().usealias() && modifiedSettings_->commonprefs().alias() != currentSettings_->commonprefs().alias()) ||
        (modifiedSettings_->commonprefs().usealias() != currentSettings_->commonprefs().usealias()))
    {
      oldName_ = currentSettings_->commonprefs().name();
      newName_ = modifiedSettings_->commonprefs().name();
      // even if oldName and newName match a name change has occurred since displayed name can be switching between name and alias
      nameChange_ = true;
    }

    // copy the settings modified by the user into the entity settings
    currentSettings_->CopyFrom(*modifiedSettings_);
    // now apply data limiting.  Will apply for Prefs and Properties changes
    store_->applyDataLimiting_(id_);
    store_->hasChanged_ = true;
  }
}

// Notification occurs on release
template<typename T>
void MemoryDataStore::MutableSettingsTransactionImpl<T>::release()
{
  // Raise the notification if changes were committed (one time only)
  if (committed_ && !notified_)
  {
    notified_ = true;

    if (nameChange_ && (oldName_ != newName_))
      store_->entityNameCache_->nameChange(newName_, oldName_, id_);

    // Need to handle recursion so make a local copy
    ListenerList localCopy = *observers_;
    store_->justRemoved_.clear();
    // Raise notifications for settings changes after internal data structures are updated
    for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
    {
      if (*i != NULL)
      {
        (*i)->onPrefsChange(store_, id_);
        store_->checkForRemoval_(localCopy);
        if ((*i != NULL) && (nameChange_))
        {
          (*i)->onNameChange(store_, id_);
          store_->checkForRemoval_(localCopy);
        }
      }
    }
  }
}

template<typename T>
MemoryDataStore::MutableSettingsTransactionImpl<T>::~MutableSettingsTransactionImpl()
{
  release();
  delete modifiedSettings_;
  modifiedSettings_ = NULL;
}

MemoryDataStore::ScenarioSettingsTransactionImpl::ScenarioSettingsTransactionImpl(ScenarioProperties *settings, MemoryDataStore *store, ScenarioListenerList *observers)
  : committed_(false),
    notified_(false),
    currentSettings_(settings),
    store_(store),
    observers_(observers)
{
  // create a copy of currentSettings_ for the user to experiment with
  modifiedSettings_ = currentSettings_->New();
  modifiedSettings_->CopyFrom(*currentSettings_);
}

/// Check for changes to preference object and copy them
/// to the internal data structure
void MemoryDataStore::ScenarioSettingsTransactionImpl::commit()
{
  // performance: skip if there are no changes
  if (modifiedSettings_->SerializeAsString() != currentSettings_->SerializeAsString())
  {
    committed_ = true; // transaction is valid

    // copy the settings modified by the user into the entity settings
    currentSettings_->CopyFrom(*modifiedSettings_);
    store_->hasChanged_ = true;
  }
}

/// No resources to be released here (resource locks/DB handles/etc)
void MemoryDataStore::ScenarioSettingsTransactionImpl::release()
{
  // Raise the notification if changes were committed (one time only)
  if (committed_ && !notified_)
  {
    notified_ = true;

    // Raise notifications for settings changes
    for (ScenarioListenerList::const_iterator i = observers_->begin(); i != observers_->end(); ++i)
    {
      (*i)->onScenarioPropertiesChange(store_);
    }
  }
}

/// If the transaction was not committed, will deallocate the properties
/// object which has not been transferred
MemoryDataStore::ScenarioSettingsTransactionImpl::~ScenarioSettingsTransactionImpl()
{
  release();
  delete modifiedSettings_;
  modifiedSettings_ = NULL;
}

//----------------------------------------------------------------------------
template <typename T, typename P>
void MemoryDataStore::NewEntryTransactionImpl<T, P>::commit()
{
  // Only need to add the entries to the container once
  if (!committed_)
  {
    committed_ = true;

    assert(entries_ != NULL);
    assert(entry_ != NULL);
    assert(entry_->properties() != NULL);
    // Not allows to change the ID
    assert(initialId_ == entry_->properties()->id());

    // assign default pref values
    P* mutablePrefs = entry_->mutable_preferences();
    mutablePrefs->CopyFrom(*defaultPrefs_);

    typename std::map<ObjectId, T*>::iterator i = entries_->find(entry_->properties()->id());
    if (i == entries_->end())
      (*entries_)[entry_->properties()->id()] = entry_;
    else
    {
      // Attempting to create an entity with same ID
      SIM_DEBUG << "Replacing entity with ID " << entry_->properties()->id() << "\n";
      assert(i->second != NULL);
      delete i->second;
      i->second = entry_;
    }
    MemoryGenericDataSlice *genericData = dynamic_cast<MemoryGenericDataSlice *>(entry_->genericData());
    assert(genericData);
    store_->genericData_[entry_->properties()->id()] = genericData;

    MemoryCategoryDataSlice *categoryData = dynamic_cast<MemoryCategoryDataSlice *>(entry_->categoryData());
    assert(categoryData);
    // need to set the category name manager for this entry
    categoryData->setCategoryNameManager(store_->categoryNameManager_);
    store_->categoryData_[entry_->properties()->id()] = categoryData;
    store_->hasChanged_ = true;
  }
}

template <typename T, typename P>
void MemoryDataStore::NewEntryTransactionImpl<T, P>::release()
{
  if (!committed_)
  {
    // Delete the uncommitted entry
    delete entry_;
    entry_ = NULL;
  }
  else
  {
    // Raise the notification (one time only)
    if (!notified_)
    {
      notified_ = true;

      // Raise notifications for new entry
      const ObjectId id = entry_->properties()->id();
      const simData::ObjectType ot = store_->objectType(id);
      // Need to handle recursion so make a local copy
      ListenerList localCopy = *listeners_;
      store_->justRemoved_.clear();
      for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
      {
        if (*i != NULL)
        {
          (*i)->onAddEntity(store_, id, ot);
          store_->checkForRemoval_(localCopy);
        }
      }
    }
  }
}

template <typename T, typename P>
MemoryDataStore::NewEntryTransactionImpl<T, P>::~NewEntryTransactionImpl()
{
  // Make sure any un-stored entries are deallocated
  release();
}

//----------------------------------------------------------------------------
template <typename T, typename SliceType>
void MemoryDataStore::NewUpdateTransactionImpl<T, SliceType>::commit()
{
  // Only need to add the entries to the container once
  if (!committed_)
  {
    committed_ = true;
    // need to grab time here, since update_ object may be deleted in the following insert call
    double updateTime = update_->time();
    insert_();
    // this applies data limiting to all implementations of the DataSlice
    // e.g. MemoryDataSlice, MemoryCommandSlice, MemoryGenericDataSlice, MemoryCategoryDataSlice, etc.
    if (dataStore_->dataLimiting())
    {
      Transaction t;
      const CommonPrefs* prefs = dataStore_->commonPrefs(id_, &t);
      slice_->limitByPrefs(*prefs);
    }
    if (applyTimeBound_)
      dataStore_->newTimeBound_(updateTime);
    dataStore_->hasChanged_ = true;
  }
}

/// Responsible for inserting the update into the slice in the general case
template <typename T, typename SliceType>
void MemoryDataStore::NewUpdateTransactionImpl<T, SliceType>::insert_()
{
  // Sorted insert
  slice_->insert(update_);
}

/// Specialization for Generic Data to permit use of ignore-duplicate-generic-data flag on insert
template <>
void MemoryDataStore::NewUpdateTransactionImpl<simData::GenericData, simData::MemoryGenericDataSlice>::insert_()
{
  // Sorted insert, optionally ignoring/limiting duplicate values
  // Ignore only applies to live mode.  Determining live mode here based on dataLimiting() flag
  slice_->insert(update_, dataStore_->dataLimiting() && dataStore_->properties_.ignoreduplicategenericdata());
}

template <typename T, typename SliceType>
void MemoryDataStore::NewUpdateTransactionImpl<T, SliceType>::release()
{
  if (!committed_)
  {
    // Delete the uncommitted update
    delete update_;
    update_ = NULL;
  }
}

template <typename T, typename SliceType>
MemoryDataStore::NewUpdateTransactionImpl<T, SliceType>::~NewUpdateTransactionImpl()
{
  // Make sure any un-stored entries are deallocated
  release();
}

//----------------------------------------------------------------------------
template <typename T, typename SliceType>
void MemoryDataStore::NewScenarioGenericUpdateTransactionImpl<T, SliceType>::commit()
{
  // Only need to add the entries to the container once
  if (!committed_)
  {
    committed_ = true;
    // Sorted insert
    slice_->insert(update_, dataStore_->dataLimiting() && dataStore_->properties_.ignoreduplicategenericdata());
    if (dataStore_->dataLimiting())
    {
      Transaction t;
      const simData::ScenarioProperties *properties = dataStore_->scenarioProperties(&t);
      CommonPrefs prefs;
      prefs.set_datalimitpoints(properties->datalimitpoints());
      prefs.set_datalimittime(properties->datalimittime());
      slice_->limitByPrefs(prefs);
    }
    dataStore_->hasChanged_ = true;
  }
}

template <typename T, typename SliceType>
void MemoryDataStore::NewScenarioGenericUpdateTransactionImpl<T, SliceType>::release()
{
  if (!committed_)
  {
    // Delete the uncommitted update
    delete update_;
    update_ = NULL;
  }
}

template <typename T, typename SliceType>
MemoryDataStore::NewScenarioGenericUpdateTransactionImpl<T, SliceType>::~NewScenarioGenericUpdateTransactionImpl()
{
  // Make sure any un-stored entries are deallocated
  release();
}

//----------------------------------------------------------------------------
// Updates the scenario time bounds with a new time
void MemoryDataStore::newTimeBound_(double timeVal)
{
  if (timeVal < 0.0)
    return;

  timeBounds_.first = simCore::sdkMin(timeVal, timeBounds_.first);
  timeBounds_.second = simCore::sdkMax(timeVal, timeBounds_.second);
}

/// Helper function to set a pair<> to min/max bounds for an XyzEntry; returns 0 when minMax is changed
template <typename EntryType,            // PlatformEntry, BeamEntry, GateEntry, etc
          typename EntryMapType>         // Platforms, Beams, Gates, etc
int setTimeBounds(ObjectId entityId, const EntryMapType* entries, std::pair<double, double>* minMax)
{
  EntryType* entry = getEntry<EntryType, EntryMapType>(entityId, entries);
  if (entry)
  {
    *minMax = std::pair<double, double>(
      simCore::sdkMin(entry->updates()->firstTime(), entry->commands()->firstTime()),
      simCore::sdkMax(entry->updates()->lastTime(), entry->commands()->lastTime())
      );
    return 0;
  }
  return 1;
}

// Retrieves the time bounds for a particular entity ID (first point, last point)
std::pair<double, double> MemoryDataStore::timeBounds(ObjectId entityId) const
{
  if (entityId == 0)
    return timeBounds();
  std::pair<double, double> rv(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
  if (setTimeBounds<PlatformEntry, Platforms>(entityId, &platforms_, &rv) == 0)
    return rv;
  if (setTimeBounds<BeamEntry, Beams>(entityId, &beams_, &rv) == 0)
    return rv;
  if (setTimeBounds<GateEntry, Gates>(entityId, &gates_, &rv) == 0)
    return rv;
  if (setTimeBounds<LaserEntry, Lasers>(entityId, &lasers_, &rv) == 0)
    return rv;
  if (setTimeBounds<ProjectorEntry, Projectors>(entityId, &projectors_, &rv) == 0)
    return rv;
  if (setTimeBounds<LobGroupEntry, LobGroups>(entityId, &lobGroups_, &rv) == 0)
    return rv;
  return rv;
}

std::pair<double, double> MemoryDataStore::timeBounds() const
{
  return timeBounds_;
}

}
