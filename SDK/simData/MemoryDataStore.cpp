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
#include <algorithm>
#include <cassert>
#include <float.h>
#include <functional>
#include <limits>
#include <optional>
#ifdef HAVE_ENTT
#include "entt/container/dense_map.hpp"
#endif
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/Interpolation.h"
#include "simCore/Calc/MultiFrameCoordinate.h"
#include "simCore/Common/Common.h"
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

/// If there are more than USE_THREAD_FOR_GENERIC_DATA entities, then use a worker thread for the update
constexpr size_t USE_THREAD_FOR_GENERIC_DATA = 1000;

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

  return nullptr;
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
* Calls flush on any entries found for the specified id in the entity map
* @param map Entity map
* @param id The entity to flush
* @param flushUpdates If true remove updates
* @param flushCommands If true remove commands
* @param startTime The start time of the data to flush
* @param endTime The end time of the data to flush (non-inclusive)
*/
template <typename EntityMap>
void flushEntityData(EntityMap& map, ObjectId id, bool flushUpdates, bool flushCommands, double startTime, double endTime)
{
  typename EntityMap::const_iterator i = map.find(id);
  if (i != map.end())
  {
    if (flushUpdates)
      (*i).second->updates()->flush(startTime, endTime);
    if (flushCommands)
      (*i).second->commands()->flush(startTime, endTime);
  }
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
    if (prefs == nullptr)
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

/** Look for transitions from Live mode to File mode to force an update to hide expired platforms */
class MemoryDataStore::ClockModeMonitor : public simCore::Clock::ModeChangeObserver
{
public:
  explicit ClockModeMonitor(MemoryDataStore& parent)
    : parent_(parent)
  {
  }

  virtual void onModeChange(simCore::Clock::Mode newMode) override
  {
    // When switching to File mode force an update so that the lifespans of platforms are calculated
    if ((newMode == simCore::Clock::MODE_STEP) || (newMode == simCore::Clock::MODE_REALTIME))
      parent_.hasChanged_ = true;
  }

  virtual void onDirectionChange(simCore::TimeDirection newDirection) override
  {
  }

  virtual void onScaleChange(double newValue) override
  {
  }

  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) override
  {
  }

  virtual void onCanLoopChange(bool newVal) override
  {
  }

  virtual void onUserEditableChanged(bool userCanEdit) override
  {
  }

private:
  MemoryDataStore& parent_;
};


/** Group multiple callbacks into one callback */
class CompositeDataStoreListener : public simData::DataStore::DefaultListener
{
public:
  CompositeDataStoreListener()
  {
  }

  virtual ~CompositeDataStoreListener()
  {
  }

  void add(simData::DataStore::ListenerPtr listener)
  {
    listeners_.push_back(listener);
  }

  virtual void onAddEntity(DataStore* source, ObjectId newId, simData::ObjectType ot) override
  {
    for (const auto& listener : listeners_)
      listener->onAddEntity(source, newId, ot);
  }

  virtual void onRemoveEntity(DataStore* source, ObjectId removedId, simData::ObjectType ot) override
  {
    for (const auto& listener : listeners_)
      listener->onRemoveEntity(source, removedId, ot);
  }

  virtual void onPropertiesChange(simData::DataStore* source, simData::ObjectId id) override
  {
    for (const auto& listener : listeners_)
      listener->onPropertiesChange(source, id);
  }

  virtual void onPrefsChange(DataStore* source, ObjectId id) override
  {
    for (const auto& listener : listeners_)
      listener->onPrefsChange(source, id);
  }

  virtual void onScenarioDelete(DataStore* source) override
  {
    for (const auto& listener : listeners_)
      listener->onScenarioDelete(source);
  }

private:
  simData::DataStore::ListenerList listeners_;
};

/** Maintains a cache with host child relationship */
class MemoryDataStore::HostChildCache : public simData::DataStore::DefaultListener
{
public:
  explicit HostChildCache(std::multimap<IdAndTypeKey, ObjectId>& hostToChildren)
    : hostToChildren_(hostToChildren)
  {
  }

  virtual ~HostChildCache()
  {
    // Should be empty
    assert(hostToChildren_.empty());
  }

  virtual void onAddEntity(DataStore* source, ObjectId newId, simData::ObjectType ot) override
  {
    // ignore non-children
    if ((ot == simData::PLATFORM) || (ot == simData::NONE) || (ot == simData::ALL))
      return;

    auto hostId = source->entityHostId(newId);
    hostToChildren_.insert(std::pair<IdAndTypeKey, ObjectId>(IdAndTypeKey(hostId, ot), newId));
  }

  virtual void onRemoveEntity(DataStore* source, ObjectId removedId, simData::ObjectType ot) override
  {
    // ignore non-children
    if ((ot == simData::PLATFORM) || (ot == simData::NONE) || (ot == simData::ALL))
      return;

    auto hostId = source->entityHostId(removedId);
    auto pair = hostToChildren_.equal_range(IdAndTypeKey(hostId, ot));

    for (auto iter = pair.first; iter != pair.second; ++iter)
    {
      if (iter->second == removedId)
      {
        hostToChildren_.erase(iter);
        return;
      }
    }

    // Have a child with no parent
    assert(false);
  }

  virtual void onScenarioDelete(DataStore* source) override
  {
    // Do not optimize, must individually remove entities for correct clean up
  }

private:
  std::multimap<IdAndTypeKey, ObjectId>& hostToChildren_;
};

/** Maintains a cache of Original IDs */
class MemoryDataStore::OriginalIdCache : public simData::DataStore::DefaultListener
{
public:
  OriginalIdCache()
  {
  }

  virtual ~OriginalIdCache()
  {
  }

  void idListByOriginalId(IdList* ids, uint64_t originalId, simData::ObjectType type) const
  {
    if (ids == nullptr)
      return;

    auto range = originalIdToUniqueIds_.equal_range(originalId);
    ids->reserve(ids->size() + std::distance(range.first, range.second));
    for (auto rangeIt = range.first; rangeIt != range.second; ++rangeIt)
    {
      if ((rangeIt->second.type & type) != 0)
        ids->emplace_back(rangeIt->second.id);
    }
  }

  virtual void onAddEntity(DataStore* source, ObjectId newId, simData::ObjectType ot) override
  {
    auto originalId = simData::DataStoreHelpers::originalIdFromId(newId, source);
    uniqueIdToOriginalId_[newId] = originalId;
    originalIdToUniqueIds_.insert(std::make_pair(originalId, Entry(newId, ot)));
  }

  virtual void onRemoveEntity(DataStore* source, ObjectId removedId, simData::ObjectType ot) override
  {
    auto it = uniqueIdToOriginalId_.find(removedId);
    if (it == uniqueIdToOriginalId_.end())
      return;

    auto range = originalIdToUniqueIds_.equal_range(it->second);

    for (auto rangeIt = range.first; rangeIt != range.second; ++rangeIt)
    {
      if (rangeIt->second.id == removedId)
      {
        originalIdToUniqueIds_.erase(rangeIt);
        break;
      }
    }

    uniqueIdToOriginalId_.erase(it);
  }

  virtual void onPropertiesChange(simData::DataStore* source, simData::ObjectId id) override
  {
    auto it = uniqueIdToOriginalId_.find(id);
    if (it == uniqueIdToOriginalId_.end())
      return;

    auto originalId = simData::DataStoreHelpers::originalIdFromId(id, source);
    if (originalId == it->second)
      return;

    auto range = originalIdToUniqueIds_.equal_range(it->second);

    for (auto rangeIt = range.first; rangeIt != range.second; ++rangeIt)
    {
      if (rangeIt->second.id == id)
      {
        it->second = originalId;
        auto entry = rangeIt->second;
        originalIdToUniqueIds_.erase(rangeIt);
        originalIdToUniqueIds_.insert(std::make_pair(it->second, entry));
        return;
      }
    }

    // data structures out of sync
    assert(false);
  }

  virtual void onScenarioDelete(DataStore* source) override
  {
    uniqueIdToOriginalId_.clear();
    originalIdToUniqueIds_.clear();
  }

private:
  std::map<ObjectId, uint64_t> uniqueIdToOriginalId_;

  struct Entry
  {
    ObjectId id = 0;
    ObjectType type = simData::NONE;
    Entry(ObjectId inId, ObjectType inType)
      : id(inId),
        type(inType)
    {}
  };

  std::multimap<uint64_t, Entry> originalIdToUniqueIds_;
};

/** Add caches to various data slices */
class MemoryDataStore::SliceCacheObserver : public simData::DataStore::DefaultListener
{
public:
  explicit SliceCacheObserver(MemoryDataStore& mds)
    : mds_(mds)
  {
  }

  SDK_DISABLE_COPY_MOVE(SliceCacheObserver);

  virtual ~SliceCacheObserver() = default;

  virtual void onAddEntity(DataStore* source, ObjectId newId, simData::ObjectType ot) override
  {
    auto categoryIt = mds_.categoryData_.find(newId);
    if (categoryIt == mds_.categoryData_.end())
    {
      // All entities have category data
      assert(false);
      return;
    }

    categoryCache_[newId] = CategoryCache(categoryIt->second);

    auto genericIt = mds_.genericData_.find(newId);
    if (genericIt == mds_.genericData_.end())
    {
      // All entities have generic data
      assert(false);
      return;
    }

    genericIt->second->setTimeGetter([this]() { return mds_.updateTime(); });

    if (ot == simData::PLATFORM)
    {
      auto it = mds_.platforms_.find(newId);
      if (it == mds_.platforms_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      platformCache_[newId] = PlatformCache(it->second);
      platformCommandCache_[newId] = CommandCache(it->second->commands(), newId);
    }
    else if (ot == simData::CUSTOM_RENDERING)
    {
      auto it = mds_.customRenderings_.find(newId);
      if (it == mds_.customRenderings_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      customRenderingCommandCache_[newId] = CommandCache(it->second->commands(), newId);
    }
    else if (ot == simData::BEAM)
    {
      auto it = mds_.beams_.find(newId);
      if (it == mds_.beams_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      beamCommandCache_[newId] = CommandCache<MemoryCommandSlice<BeamCommand, BeamPrefs>>(it->second->commands(), newId);
    }
    else if (ot == simData::GATE)
    {
      auto it = mds_.gates_.find(newId);
      if (it == mds_.gates_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      gateCommandCache_[newId] = CommandCache<MemoryCommandSlice<GateCommand, GatePrefs>>(it->second->commands(), newId);
    }
    else if (ot == simData::LASER)
    {
      auto it = mds_.lasers_.find(newId);
      if (it == mds_.lasers_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      laserCommandCache_[newId] = CommandCache<MemoryCommandSlice<LaserCommand, LaserPrefs>>(it->second->commands(), newId);
    }
    else if (ot == simData::LOB_GROUP)
    {
      auto it = mds_.lobGroups_.find(newId);
      if (it == mds_.lobGroups_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      lobCommandCache_[newId] = CommandCache<MemoryCommandSlice<LobGroupCommand, LobGroupPrefs>>(it->second->commands(), newId);
    }
    else if (ot == simData::PROJECTOR)
    {
      auto it = mds_.projectors_.find(newId);
      if (it == mds_.projectors_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      projectorCommandCache_[newId] = CommandCache<MemoryCommandSlice<ProjectorCommand, ProjectorPrefs>>(it->second->commands(), newId);
    }
  }

  virtual void onRemoveEntity(DataStore* source, ObjectId removedId, simData::ObjectType ot) override
  {
    categoryCache_.erase(removedId);
    if (platformCache_.erase(removedId) == 1)
    {
      platformCommandCache_.erase(removedId);
      return;
    }

    if (customRenderingCommandCache_.erase(removedId) == 1)
      return;

    if (beamCommandCache_.erase(removedId) == 1)
      return;

    if (gateCommandCache_.erase(removedId) == 1)
      return;

    if (laserCommandCache_.erase(removedId) == 1)
      return;

    if (lobCommandCache_.erase(removedId) == 1)
      return;

    if (projectorCommandCache_.erase(removedId) == 1)
      return;
  }

  virtual void onPrefsChange(DataStore* source, ObjectId id) override
  {
    // A preference change can affect how a TSPI point is process, so reset
    auto platformIt = platformCache_.find(id);
    if (platformIt != platformCache_.end())
      platformIt->second.resetPreferences();
  }

  virtual void onScenarioDelete(DataStore* source) override
  {
    categoryCache_.clear();
    platformCache_.clear();
    platformCommandCache_.clear();
    customRenderingCommandCache_.clear();
    beamCommandCache_.clear();
    gateCommandCache_.clear();
    laserCommandCache_.clear();
    lobCommandCache_.clear();
    projectorCommandCache_.clear();
  }

  void installSliceTimeRangeMonitor(ObjectId id, std::function<void(double startTime, double endTime)> fn)
  {
    auto it = platformCache_.find(id);
    if (it != platformCache_.end())
      it->second.installSliceTimeRangeMonitor(fn);
  }
  /// Update category slices to the give time and return the ids slices that changed due to the update
  void updateCategoryData_(double time, std::vector<simData::ObjectId>& ids)
  {
#ifdef HAVE_ENTT
    for (const auto& [id, entry] : categoryCache_)
#else
    for (auto& [id, entry] : categoryCache_)
#endif
    {
      if (entry.update(time))
        ids.push_back(id);
    }
  }

  void updateCommands(double time, std::map<simData::ObjectId, CommitResult>& allResults)
  {
#ifdef HAVE_ENTT
    for (const auto& [id, entry] : platformCommandCache_)
#else
    for (auto& [id, entry] : platformCommandCache_)
#endif
    {
      auto results = entry.update(&mds_, id, time);

      if (results != CommitResult::NO_CHANGE)
        allResults[id] = results;

      if (entry.getAndClearDirty_() && (results == CommitResult::NO_CHANGE))
      {
        auto it = platformCache_.find(id);
        if (it != platformCache_.end())
          it->second.resetPreferences();
      }
    }

    updateCommands_(customRenderingCommandCache_, time, allResults);
    updateCommands_(beamCommandCache_, time, allResults);
    updateCommands_(gateCommandCache_, time, allResults);
    updateCommands_(laserCommandCache_, time, allResults);
    updateCommands_(lobCommandCache_, time, allResults);
    updateCommands_(projectorCommandCache_, time, allResults);
  }

  /// Update platforms to the given time and return the IDs of platforms with changes due to command processing
  void updatePlatforms_(double time)
  {
    auto interpolateEnabled = mds_.interpolatorState();
    if ((interpolateEnabled == InterpolatorState::EXTERNAL) && !mds_.interpolator())
      interpolateEnabled = InterpolatorState::OFF;

    const bool fileMode = isFileMode_();

#ifdef HAVE_ENTT
    for (const auto& [id, entry] : platformCache_)
#else
    for (auto& [id, entry] : platformCache_)
#endif
      entry.update(&mds_, id, interpolateEnabled, fileMode, time);
  }

  void resetPlatforms()
  {
#ifdef HAVE_ENTT
    for (const auto& [id, entry] : platformCache_)
#else
    for (auto& [id, entry] : platformCache_)
#endif
      entry.reset();
  }

private:
  /** Maintains the time range for a category data slice state and only updates the slice state if a new time is outside the time range. */
   class CategoryCache
  {
  public:
    explicit CategoryCache(MemoryCategoryDataSlice* slice = nullptr)
      : slice_(slice)
    {
      reset();
      if (slice_)
        slice_->installNotifier([this] { reset(); });
    }

    ~CategoryCache() = default;

    SDK_DISABLE_COPY(CategoryCache);

    CategoryCache(CategoryCache&& other) noexcept
      : startTime_(std::move(other.startTime_)),
        endTime_(std::move(other.endTime_)),
        slice_(std::move(other.slice_))
    {
      if (slice_)
        slice_->installNotifier([this] { reset(); });
    }

    CategoryCache& operator=(CategoryCache&& other) noexcept
    {
      startTime_ = std::move(other.startTime_);
      endTime_ = std::move(other.endTime_);
      slice_ = std::move(other.slice_);

      if (slice_)
        slice_->installNotifier([this] { reset(); });

      return *this;
    }

    bool update(double time)
    {
      // Kick out early if the time is with in the last time range
      if ((time >= startTime_.value_or(std::numeric_limits<double>::max())) && (time < endTime_.value_or(std::numeric_limits<double>::lowest())))
        return false;

      if (slice_)
        return slice_->update(time, startTime_, endTime_);

      return false;
    }

    /// Called when the slice is modified so that the next call to update will not kick out early
    void reset()
    {
      startTime_.reset();
      endTime_.reset();
    }

  private:
    std::optional<double> startTime_;
    std::optional<double> endTime_;
    MemoryCategoryDataSlice* slice_ = nullptr;
  };

  /** Maintains the time range for a command slice state and only updates the slice state if a new time is outside the time range. */
  template <typename SliceType>
  class CommandCache
  {
  public:
    explicit CommandCache(SliceType* slice = nullptr, simData::ObjectId id = 0)
      : slice_(slice),
        id_(id)
    {
      reset();
      if (slice_)
        slice_->installNotifier([this] { reset(); });
    }

    ~CommandCache() = default;

    SDK_DISABLE_COPY(CommandCache);

    CommandCache(CommandCache&& other) noexcept
      : startTime_(std::move(other.startTime_)),
        endTime_(std::move(other.endTime_)),
        slice_(std::move(other.slice_)),
        dirty_(std::move(other.dirty_))
    {
      if (slice_)
        slice_->installNotifier([this] { reset(); });
    }

    CommandCache& operator=(CommandCache&& other) noexcept
    {
      startTime_ = std::move(other.startTime_);
      endTime_ = std::move(other.endTime_);
      slice_ = std::move(other.slice_);
      dirty_ = std::move(other.dirty_);

      if (slice_)
        slice_->installNotifier([this] { reset(); });

      return *this;
    }

    CommitResult update(simData::DataStore* ds, simData::ObjectId id, double time)
    {
      CommitResult rv = CommitResult::NO_CHANGE;

      // Kick out early if the time is with in the last time range
      if ((time >= startTime_.value_or(std::numeric_limits<double>::max())) && (time < endTime_.value_or(std::numeric_limits<double>::lowest())))
        return rv;

      slice_->update(ds, id, time, rv, startTime_, endTime_);

      return rv;
    }

    /// Called when the slice is modified so that the next call to update will not kick out early
    void reset()
    {
      startTime_.reset();
      endTime_.reset();
      dirty_ = true;
    }

    bool getAndClearDirty_()
    {
      bool rv = dirty_;
      dirty_ = false;
      return rv;
    }

  private:
    std::optional<double> startTime_;
    std::optional<double> endTime_;
    SliceType* slice_ = nullptr;
    simData::ObjectId id_ = 0;
    bool dirty_ = true;
  };

  /** Cache various data to make the platform update more efficient */
  class PlatformCache
  {
  public:
    explicit PlatformCache(PlatformEntry* entry = nullptr)
      : entry_(entry)
    {
      resetPreferences();
      reset();
      if (entry_)
        entry_->updates()->installNotifier([this] { reset(); });
    }

    ~PlatformCache() = default;

    SDK_DISABLE_COPY(PlatformCache);

    PlatformCache(PlatformCache&& other) noexcept
      : updateStartTime_(std::move(other.updateStartTime_)),
        updateEndTime_(std::move(other.updateEndTime_)),
        sliceStartTime_(std::move(other.sliceStartTime_)),
        sliceEndTime_(std::move(other.sliceEndTime_)),
        entry_(std::move(other.entry_)),
        sliceSize_(std::move(other.sliceSize_)),
        dataDraw_(std::move(other.dataDraw_)),
        interpolatePos_(std::move(other.interpolatePos_)),
        needToClear_(std::move(other.needToClear_)),
        needToSetToNull_(std::move(other.needToSetToNull_)),
        lifeSpanMode_(std::move(other.lifeSpanMode_)),
        entry1_(std::move(other.entry1_)),
        entry2_(std::move(other.entry2_)),
        timeRangeMonitorFn_(std::move(other.timeRangeMonitorFn_))
    {
      if (entry_)
        entry_->updates()->installNotifier([this] { reset(); });
    }

    PlatformCache& operator=(PlatformCache&& other) noexcept
    {
      updateStartTime_ = std::move(other.updateStartTime_);
      updateEndTime_ = std::move(other.updateEndTime_);
      sliceStartTime_ = std::move(other.sliceStartTime_);
      sliceEndTime_ = std::move(other.sliceEndTime_);
      entry_ = std::move(other.entry_);
      sliceSize_ = std::move(other.sliceSize_);
      dataDraw_ = std::move(other.dataDraw_);
      interpolatePos_ = std::move(other.interpolatePos_);
      needToClear_ = std::move(other.needToClear_);
      needToSetToNull_ = std::move(other.needToSetToNull_);
      lifeSpanMode_ = std::move(other.lifeSpanMode_);
      entry1_ = std::move(other.entry1_);
      entry2_ = std::move(other.entry2_);
      timeRangeMonitorFn_ = std::move(other.timeRangeMonitorFn_);

      if (entry_)
        entry_->updates()->installNotifier([this] { reset(); });

      return *this;
    }

    /**
     * Updates the platform command slice and update slices. Uses cache data to improve performance
     * @param ds The data store
     * @param id The unique id of the platform
     * @param interpolateState Type of interpolation, if any
     * @param fileMode True if the data store is in file mode
     * @param time The scenario time to update the slices to
     */
    void update(simData::DataStore* ds, simData::ObjectId id, DataStore::InterpolatorState interpolateState, bool fileMode, double time)
    {
      // Set the slice time range
      if (!sliceStartTime_.has_value())
      {
        sliceStartTime_ = entry_->updates()->firstTime();
        sliceEndTime_ = entry_->updates()->lastTime();
        sliceSize_ = entry_->updates()->numItems();
        if (timeRangeMonitorFn_)
          timeRangeMonitorFn_(*sliceStartTime_, *sliceEndTime_);
      }

      // Return early if not drawing
      if (!dataDraw_)
      {
        // until we have datadraw, send nullptr; once we have datadraw, we'll immediately update with valid data
        entry_->updates()->setCurrent(nullptr);
        return;
      }

      const bool isInterpolated = ((interpolateState != InterpolatorState::OFF) && interpolatePos_);

      // Kick out early if the time is with in the last time range
      if (!isInterpolated && (time >= updateStartTime_.value_or(std::numeric_limits<double>::max())) && (time < updateEndTime_.value_or(std::numeric_limits<double>::lowest())))
      {
        if (needToClear_)
        {
          entry_->updates()->clearChanged();
          needToClear_ = false;
        }

        return;
      }

      needToClear_ = true;

      // If file mode, might need to extend a platform with one TSPI point
      if (fileMode)
      {
        if (!isFileModePlatformActive_(time))
        {
          // Platform is not valid/off
          if (needToSetToNull_)
          {
            entry_->updates()->setCurrent(nullptr);
            updateStartTime_.reset();
            updateEndTime_.reset();
            needToSetToNull_ = false;
          }
          return;
        }
      }

      needToSetToNull_ = true;

      if (!isInterpolated)
      {
        entry_->updates()->update(time, updateStartTime_, updateEndTime_);
        // update returns the extended time which might need to be truncated back to the original slice end time
        if (fileMode && !isExtendedPlatform() && (updateEndTime_ > sliceEndTime_))
          updateEndTime_ = sliceEndTime_;
      }
      else if (interpolateState == InterpolatorState::INTERNAL)
        update_(time);
      else
        entry_->updates()->update(time, ds->interpolator());
    }

    /** Called when the slice is modified so that the next call to update will not kick out early */
    void reset()
    {
      updateStartTime_.reset();
      updateEndTime_.reset();
      sliceStartTime_.reset();
      sliceEndTime_.reset();
      if (timeRangeMonitorFn_)
        timeRangeMonitorFn_(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
      sliceSize_ = 0;
      entry1_.reset();
      entry2_.reset();
    }

    void resetPreferences()
    {
      if (!entry_ || !entry_->preferences())
        return;

      bool resetTimes = false;

      if (dataDraw_ != entry_->preferences()->commonprefs().datadraw())
      {
        dataDraw_ = entry_->preferences()->commonprefs().datadraw();
        resetTimes = true;
      }
      if (interpolatePos_ != entry_->preferences()->interpolatepos())
      {
        interpolatePos_ = entry_->preferences()->interpolatepos();
        resetTimes = true;
      }

      if (lifeSpanMode_ != entry_->preferences()->lifespanmode())
      {
        lifeSpanMode_ = entry_->preferences()->lifespanmode();
        resetTimes = true;
      }

      if (resetTimes)
        reset();
    }

    void installSliceTimeRangeMonitor(std::function<void(double startTime, double endTime)> fn)
    {
      timeRangeMonitorFn_ = fn;
    }

  private:
    /** Returns true if platform is active per the life span mode */
    bool isFileModePlatformActive_(double time) const
    {
      double startTime = std::numeric_limits<double>::lowest();
      double endTime = std::numeric_limits<double>::max();
      if (sliceStartTime_ != -1.0)
      {
        startTime = sliceStartTime_.value_or(std::numeric_limits<double>::lowest());
        endTime = sliceEndTime_.value_or(std::numeric_limits<double>::max());

        if (isExtendedPlatform())
          endTime = std::numeric_limits<double>::max();
      }

      return time >= startTime && time <= endTime;
    }

    /** Returns true if the platform is extended */
    bool isExtendedPlatform() const
    {
      if (lifeSpanMode_ == LifespanMode::LIFE_FIRST_LAST_POINT)
        return false;

      if (sliceStartTime_ == -1.0)
        return false;

      return (sliceSize_ == 1);
    }

    /**
     * Updates the interpolated value.
     * Maps to void MemoryDataSlice<T>::update(double time, Interpolator *interpolator)
     */
    void update_(double time)
    {
      const auto slice = entry_->updates();
      const auto current = slice->current();

      slice->clearChanged();

      // early out when there are no changes to this slice
      if ((current != nullptr) && ((current->time() == time) || (current->time() == -1.0)))
        return;

      // If necessary calculate a new range
      if ((time < updateStartTime_.value_or(std::numeric_limits<double>::max())) || (time > updateEndTime_.value_or(std::numeric_limits<double>::lowest())))
      {
        auto it = slice->upper_bound(time);

        if (slice->numItems() == 0)
        {
          entry1_.reset();
          entry2_.reset();
          updateStartTime_.reset();
          updateEndTime_.reset();
        }
        else if (!it.hasPrevious())  // First point
        {
          entry1_ = Entry(it.peekNext(), convert_(it.peekNext()));
          entry2_.reset();
          updateStartTime_ = 0;
          updateEndTime_ = it.peekNext()->time();
        }
        else if (!it.hasNext()) // Last point
        {
          entry1_.reset();
          entry2_ = Entry(it.peekPrevious(), convert_(it.peekPrevious()));
          updateStartTime_ = it.peekPrevious()->time();
          updateEndTime_ = std::numeric_limits<double>::max();
        }
        else // Time in between points
        {
          entry1_ = Entry(it.peekPrevious(), convert_(it.peekPrevious()));
          entry2_ = Entry(it.peekNext(), convert_(it.peekNext()));
          updateStartTime_ = it.peekPrevious()->time();
          updateEndTime_ = it.peekNext()->time();
        }
      }

      DataSlice<simData::PlatformUpdate>::Bounds bounds;
      bool isBounded = false;

      // note that computeTimeUpdate can return a ptr to a real update, or pointer to currentInterpolated_
      slice->setCurrent(computeTimeUpdate_(time, isBounded, slice->currentInterpolated(), bounds));
      slice->setInterpolated(isBounded, bounds);
    }

    /**
     * Returns the update for the given time; can return null.
     * Maps to T *computeTimeUpdate(ForwardIterator begin, ForwardIterator& currentIt, ForwardIterator end, double time, Interpolator *interpolator, bool *isInterpolated, T *interpolatedPoint, B *bounds)
     */
    simData::PlatformUpdate* computeTimeUpdate_(double time, bool& isInterpolated, simData::PlatformUpdate* interpolatedPoint, DataSlice<simData::PlatformUpdate>::Bounds& bounds)
    {
      isInterpolated = false;
      bounds = { nullptr, nullptr };

      if (sliceSize_ == 0)
        return nullptr;

      // Closest update is the last point
      if (!entry1_.has_value() && entry2_.has_value())
        return const_cast<simData::PlatformUpdate*>(entry2_->update);

      // time is at or before the first point
      if (entry1_.has_value() && !entry2_.has_value())
      {
        // updateEndTime_ is correct, because the compare needs the end time.
        if (simCore::areEqual(time, updateEndTime_.value()))
          return const_cast<simData::PlatformUpdate*>(entry1_->update);

        return nullptr;
      }

      // time is between points, but first check to see if the time is on the boundary
      if (simCore::areEqual(time, updateStartTime_.value()))
        return const_cast<simData::PlatformUpdate*>(entry1_->update);

      // Check end boundary
      if (simCore::areEqual(time, updateEndTime_.value()))
        return const_cast<simData::PlatformUpdate*>(entry2_->update);

      // If gotten this far, then it must be an interpolation
      isInterpolated = true;
      bounds = { const_cast<simData::PlatformUpdate*>(entry1_->update), const_cast<simData::PlatformUpdate*>(entry2_->update) };
      interpolate_(time, *entry1_->mfc, *entry2_->mfc, *interpolatedPoint);
      return interpolatedPoint;
    }

    /**
     * Returns the interpolated TSPI point between mfc1 and mfc2 as specified by the time
     * Maps to bool LinearInterpolator::interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result)
     */
    void interpolate_(double time, simCore::MultiFrameCoordinate& mfc1, simCore::MultiFrameCoordinate& mfc2, PlatformUpdate& result) const
    {
      // time must be within bounds for interpolation to work
      assert(updateStartTime_.value() <= time && time <= updateEndTime_.value());

      // compute time ratio
      const double factor = simCore::getFactor(updateStartTime_.value(), time, updateEndTime_.value());

      const auto& prev = mfc1.ecefCoordinate();
      const auto& prevLla = mfc1.llaCoordinate();
      const auto& next = mfc2.ecefCoordinate();
      const auto& nextLla = mfc2.llaCoordinate();

      // do the interpolation in geocentric, this way the
       // interpolation is correct at N/S and E/W transitions
      simCore::Vec3 xyz(simCore::linearInterpolate(prev.x(), next.x(), factor),
        simCore::linearInterpolate(prev.y(), next.y(), factor),
        simCore::linearInterpolate(prev.z(), next.z(), factor));

      simCore::Vec3 lla;
      simCore::CoordinateConverter::convertEcefToGeodeticPos(xyz, lla);

      // Use interpolated geodetic altitude to prevent short cuts through the earth
      simCore::Coordinate resultsLla;
      resultsLla.setCoordinateSystem(simCore::COORD_SYS_LLA);
      resultsLla.setPositionLLA(lla.lat(), lla.lon(), simCore::linearInterpolate(prevLla.z(), nextLla.z(), factor));

      if (prev.hasOrientation() && next.hasOrientation())
      {
        const double l_yaw = simCore::angFix2PI(prevLla.yaw());
        const double l_pitch = simCore::angFix2PI(prevLla.pitch());
        const double l_roll = simCore::angFix2PI(prevLla.roll());
        const double h_yaw = simCore::angFix2PI(nextLla.yaw());
        const double h_pitch = simCore::angFix2PI(nextLla.pitch());
        const double h_roll = simCore::angFix2PI(nextLla.roll());

        // orientations assumed to be between 0 and 360
        const double delta_yaw = (h_yaw - l_yaw);
        const double delta_pitch = (h_pitch - l_pitch);
        const double delta_roll = (h_roll - l_roll);

        double yaw = 0.0;
        double pitch = 0.0;
        double roll = 0.0;

        if (delta_yaw == 0.)
          yaw = l_yaw;
        else if (std::abs(delta_yaw) < M_PI)
          yaw = (l_yaw + factor * delta_yaw);
        else
        {
          if (delta_yaw > 0)
            yaw = (l_yaw - factor * (M_TWOPI - delta_yaw));
          else
            yaw = (l_yaw + factor * (M_TWOPI + delta_yaw));
        }

        if (delta_pitch == 0.)
          pitch = l_pitch;
        else if (std::abs(delta_pitch) < M_PI)
          pitch = (l_pitch + factor * delta_pitch);
        else
        {
          if (delta_pitch > 0)
            pitch = (l_pitch - factor * (M_TWOPI - delta_pitch));
          else
            pitch = (l_pitch + factor * (M_TWOPI + delta_pitch));
        }

        if (delta_roll == 0.)
          roll = l_roll;
        else if (std::abs(delta_roll) < M_PI)
          roll = (l_roll + factor * delta_roll);
        else
        {
          if (delta_roll > 0)
            roll = (l_roll - factor * (M_TWOPI - delta_roll));
          else
            roll = (l_roll + factor * (M_TWOPI + delta_roll));
        }

        resultsLla.setOrientation(yaw, pitch, roll);
      }

      if (prev.hasVelocity() && next.hasVelocity())
      {
        resultsLla.setVelocity(simCore::linearInterpolate(prevLla.vx(), nextLla.vx(), factor),
          simCore::linearInterpolate(prevLla.vy(), nextLla.vy(), factor),
          simCore::linearInterpolate(prevLla.vz(), nextLla.vz(), factor));
      }

      simCore::Coordinate resultsEcef;
      simCore::CoordinateConverter::convertGeodeticToEcef(resultsLla, resultsEcef);

      result.set_time(time);

      result.set_x(resultsEcef.x());
      result.set_y(resultsEcef.y());
      result.set_z(resultsEcef.z());

      if (resultsEcef.hasVelocity())
      {
        result.set_vx(resultsEcef.vx());
        result.set_vy(resultsEcef.vy());
        result.set_vz(resultsEcef.vz());
      }

      if (resultsEcef.hasOrientation())
      {
        result.set_psi(resultsEcef.psi());
        result.set_theta(resultsEcef.theta());
        result.set_phi(resultsEcef.phi());
      }
    }

    /** Convert a simData::PlatformUpdate into a simCore::MultiFrameCoordinate */
    simCore::MultiFrameCoordinate convert_(const simData::PlatformUpdate* update) const
    {
      simCore::Vec3 pos;
      update->position(pos);

      simCore::MultiFrameCoordinate rv;
      simCore::Coordinate coord(simCore::COORD_SYS_ECEF, pos);

      if (update->has_orientation())
      {
        simCore::Vec3 ori;
        update->orientation(ori);
        coord.setOrientation(ori);
      }

      if (update->has_velocity())
      {
        simCore::Vec3 vec;
        update->velocity(vec);
        coord.setVelocity(vec);
      }

      rv.setCoordinate(coord);
      return rv;
    }

    std::optional<double> updateStartTime_;
    std::optional<double> updateEndTime_;
    std::optional<double> sliceStartTime_;
    std::optional<double> sliceEndTime_;
    PlatformEntry* entry_ = nullptr;
    size_t sliceSize_ = 0;
    bool dataDraw_ = false;
    bool interpolatePos_ = false;
    bool needToClear_ = true;
    bool needToSetToNull_ = true;
    LifespanMode lifeSpanMode_ = LifespanMode::LIFE_FIRST_LAST_POINT;

    /** Keep track of a platform update / MultiFrameCoordinate pair */
    struct Entry
    {
      const simData::PlatformUpdate* update = nullptr;
      std::optional<simCore::MultiFrameCoordinate> mfc;

      Entry(const simData::PlatformUpdate* inUpdate, std::optional<simCore::MultiFrameCoordinate> inMfc)
        : update(inUpdate),
          mfc(inMfc)
      {
      }

      ~Entry() = default;

      SDK_DISABLE_COPY(Entry);

      Entry(Entry&& other) noexcept = default;
      Entry& operator=(Entry&& other) noexcept = default;
    };

    std::optional<Entry> entry1_;
    std::optional<Entry> entry2_;
    std::function<void(double startTime, double endTime)> timeRangeMonitorFn_;
  };

  /** Returns true if the memory data store is in file mode */
  bool isFileMode_() const
  {
    return mds_.getBoundClock()
      ? !mds_.getBoundClock()->isLiveMode()
      : !mds_.dataLimiting();
  }

  /** Generic routine for updating command slices to the given time */
  template <typename Cache>
  void updateCommands_(Cache& cache, double time, std::map<simData::ObjectId, CommitResult>& allResults) const
  {
#ifdef HAVE_ENTT
    for (const auto& [id, entry] : cache)
#else
    for (auto& [id, entry] : cache)
#endif
    {
      auto results = entry.update(&mds_, id, time);

      if (results != CommitResult::NO_CHANGE)
        allResults[id] = results;
    }
  }

  MemoryDataStore& mds_;
#ifdef HAVE_ENTT
  entt::dense_map<simData::ObjectId, CategoryCache> categoryCache_;
  entt::dense_map<simData::ObjectId, PlatformCache> platformCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<PlatformCommand, PlatformPrefs>>> platformCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<CustomRenderingCommand, CustomRenderingPrefs>>> customRenderingCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<BeamCommand, BeamPrefs>>> beamCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<GateCommand, GatePrefs>>> gateCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<LaserCommand, LaserPrefs>>> laserCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<LobGroupCommand, LobGroupPrefs>>> lobCommandCache_;
  entt::dense_map<simData::ObjectId, CommandCache<MemoryCommandSlice<ProjectorCommand, ProjectorPrefs>>> projectorCommandCache_;
#else
  std::map<simData::ObjectId, CategoryCache> categoryCache_;
  std::map<simData::ObjectId, PlatformCache> platformCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<PlatformCommand, PlatformPrefs>>> platformCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<CustomRenderingCommand, CustomRenderingPrefs>>> customRenderingCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<BeamCommand, BeamPrefs>>> beamCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<GateCommand, GatePrefs>>> gateCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<LaserCommand, LaserPrefs>>> laserCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<LobGroupCommand, LobGroupPrefs>>> lobCommandCache_;
  std::map<simData::ObjectId, CommandCache<MemoryCommandSlice<ProjectorCommand, ProjectorPrefs>>> projectorCommandCache_;
#endif
};

//----------------------------------------------------------------------------

/** Add a reflector to a command slice */
class MemoryDataStore::ReflectionObserver : public simData::DataStore::DefaultListener
{
public:
  explicit ReflectionObserver(MemoryDataStore& mds)
    : mds_(mds),
      beamPreferences_(simData::Reflection::makePlatformPreferences()),
      customRenderingPreferences_(simData::Reflection::makeCustomRenderingPreferences()),
      gatePreferences_(simData::Reflection::makeGatePreferences()),
      laserPreferences_(simData::Reflection::makeLaserPreferences()),
      lobGroupPreferences_(simData::Reflection::makeLobGroupPreferences()),
      platformPreferences_(simData::Reflection::makePlatformPreferences()),
      projectorPreferences_(simData::Reflection::makeProjectorPreferences())
  {
  }

  SDK_DISABLE_COPY_MOVE(ReflectionObserver);

  virtual ~ReflectionObserver() = default;

  virtual void onAddEntity(DataStore* source, ObjectId newId, simData::ObjectType ot) override
  {
    if (ot == simData::PLATFORM)
    {
      auto it = mds_.platforms_.find(newId);
      if (it == mds_.platforms_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }

      it->second->commands()->setReflection(platformPreferences_);
    }
    else if (ot == simData::CUSTOM_RENDERING)
    {
      auto it = mds_.customRenderings_.find(newId);
      if (it == mds_.customRenderings_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(customRenderingPreferences_);
    }
    else if (ot == simData::BEAM)
    {
      auto it = mds_.beams_.find(newId);
      if (it == mds_.beams_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(beamPreferences_);
    }
    else if (ot == simData::GATE)
    {
      auto it = mds_.gates_.find(newId);
      if (it == mds_.gates_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(gatePreferences_);
    }
    else if (ot == simData::LASER)
    {
      auto it = mds_.lasers_.find(newId);
      if (it == mds_.lasers_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(laserPreferences_);
    }
    else if (ot == simData::LOB_GROUP)
    {
      auto it = mds_.lobGroups_.find(newId);
      if (it == mds_.lobGroups_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(lobGroupPreferences_);
    }
    else if (ot == simData::PROJECTOR)
    {
      auto it = mds_.projectors_.find(newId);
      if (it == mds_.projectors_.end())
      {
        // The data store does not match what was passed in
        assert(false);
        return;
      }
      it->second->commands()->setReflection(projectorPreferences_);
    }
  }

  private:
    MemoryDataStore& mds_;
    std::shared_ptr<simData::Reflection> beamPreferences_;
    std::shared_ptr<simData::Reflection> customRenderingPreferences_;
    std::shared_ptr<simData::Reflection> gatePreferences_;
    std::shared_ptr<simData::Reflection> laserPreferences_;
    std::shared_ptr<simData::Reflection> lobGroupPreferences_;
    std::shared_ptr<simData::Reflection> platformPreferences_;
    std::shared_ptr<simData::Reflection> projectorPreferences_;
};

//----------------------------------------------------------------------------

/** Adapts NewRowDataListener to MemoryDataStore's newUpdatesListener_ */
class MemoryDataStore::NewRowDataToNewUpdatesAdapter : public MemoryTable::TableManager::NewRowDataListener
{
public:
  explicit NewRowDataToNewUpdatesAdapter(simData::MemoryDataStore& dataStore)
    : dataStore_(dataStore)
  {
  }

  virtual void onNewRowData(simData::DataTable& table, simData::ObjectId id, double dataTime)
  {
    for (const auto& listenerPtr : dataStore_.newUpdatesListeners_)
      listenerPtr->onNewRowData(&dataStore_, table, id, dataTime);
  }

private:
  simData::MemoryDataStore& dataStore_;
};

//----------------------------------------------------------------------------

/** InternalsMemento implementation for MemoryDataStore */
class MemoryDataStore::MemoryInternalsMemento : public InternalsMemento
{
public:
  /** Constructor for a memento for the MemoryDataStore */
  explicit MemoryInternalsMemento(const MemoryDataStore &ds)
    : interpolator_(ds.interpolator_),
      interpolationEnabled_(ds.interpolationEnabled_),
      listeners_(ds.listeners_),
      scenarioListeners_(ds.scenarioListeners_),
      newUpdatesListeners_(ds.newUpdatesListeners_),
      boundClock_(ds.boundClock_)
  {
    // fill in everything

    ds.dataTableManager().getObservers(dtObservers_);
    ds.categoryNameManager().getListeners(catListeners_);
    defaultPlatformPrefs_.CopyFrom(ds.defaultPlatformPrefs_);
    defaultBeamPrefs_.CopyFrom(ds.defaultBeamPrefs_);
    defaultGatePrefs_.CopyFrom(ds.defaultGatePrefs_);
    defaultLaserPrefs_.CopyFrom(ds.defaultLaserPrefs_);
    defaultLobGroupPrefs_.CopyFrom(ds.defaultLobGroupPrefs_);
    defaultProjectorPrefs_.CopyFrom(ds.defaultProjectorPrefs_);
    defaultCustomRenderingPrefs_.CopyFrom(ds.defaultCustomRenderingPrefs_);
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
    {
      // Do not copy the internal one
      if (dynamic_cast<CompositeDataStoreListener*>((*iter).get()) == nullptr)
        ds.addListener(*iter);
    }
    for (ScenarioListenerList::const_iterator iter2 = scenarioListeners_.begin(); iter2 != scenarioListeners_.end(); ++iter2)
      ds.addScenarioListener(*iter2);

    for (auto listener : newUpdatesListeners_)
      ds.addNewUpdatesListener(listener);

    for (std::vector<DataTableManager::ManagerObserverPtr>::const_iterator iter = dtObservers_.begin(); iter != dtObservers_.end(); ++iter)
      ds.dataTableManager().addObserver(*iter);

    for (std::vector<CategoryNameManager::ListenerPtr>::const_iterator iter = catListeners_.begin(); iter != catListeners_.end(); ++iter)
      ds.categoryNameManager().addListener(*iter);

    ds.setDefaultPrefs(defaultPlatformPrefs_, defaultBeamPrefs_, defaultGatePrefs_, defaultLaserPrefs_, defaultLobGroupPrefs_, defaultProjectorPrefs_);
    ds.bindToClock(boundClock_);
  }

private: // data
  Interpolator *interpolator_;
  InterpolatorState interpolationEnabled_ = InterpolatorState::OFF;

  DataStore::ListenerList listeners_;
  DataStore::ScenarioListenerList scenarioListeners_;
  std::vector<DataStore::NewUpdatesListenerPtr> newUpdatesListeners_;
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

//----------------------------------------------------------------------------

///constructor
MemoryDataStore::MemoryDataStore()
: baseId_(0),
  lastUpdateTime_(0.0),
  hasChanged_(false),
  interpolationEnabled_(InterpolatorState::OFF),
  interpolator_(nullptr),
  dataLimiting_(false),
  categoryNameManager_(new CategoryNameManager),
  dataLimitsProvider_(nullptr),
  dataTableManager_(nullptr),
  boundClock_(nullptr),
  entityNameCache_(new EntityNameCache())
{
  initCompositeListener_();
  dataLimitsProvider_ = new DataStoreLimits(*this);
  dataTableManager_ = new MemoryTable::TableManager(dataLimitsProvider_);
  newRowDataListener_.reset(new NewRowDataToNewUpdatesAdapter(*this));
  auto data = new MemoryGenericDataSlice();
  data->setTimeGetter([this]() { return updateTime(); });
  genericData_[0] = data;
  clockModeMonitor_ = std::make_unique<ClockModeMonitor>(*this);
}

///construct with properties
MemoryDataStore::MemoryDataStore(const ScenarioProperties &properties)
: baseId_(0),
  lastUpdateTime_(0.0),
  hasChanged_(false),
  interpolationEnabled_(InterpolatorState::OFF),
  interpolator_(nullptr),
  dataLimiting_(false),
  categoryNameManager_(new CategoryNameManager),
  dataLimitsProvider_(nullptr),
  dataTableManager_(nullptr),
  boundClock_(nullptr),
  entityNameCache_(new EntityNameCache())
{
  initCompositeListener_();
  dataLimitsProvider_ = new DataStoreLimits(*this);
  dataTableManager_ = new MemoryTable::TableManager(dataLimitsProvider_);
  newRowDataListener_.reset(new NewRowDataToNewUpdatesAdapter(*this));
  properties_.CopyFrom(properties);
  auto data = new MemoryGenericDataSlice();
  data->setTimeGetter([this]() { return updateTime(); });
  genericData_[0] = data;
  clockModeMonitor_ = std::make_shared<ClockModeMonitor>(*this);
}

///destructor
MemoryDataStore::~MemoryDataStore()
{
  if (boundClock_)
    boundClock_->removeModeChangeCallback(clockModeMonitor_);

  clearMemory_();
  delete categoryNameManager_;
  categoryNameManager_ = nullptr;
  delete dataTableManager_;
  dataTableManager_ = nullptr;
  delete dataLimitsProvider_;
  dataLimitsProvider_ = nullptr;
  delete entityNameCache_;
  entityNameCache_ = nullptr;
}

void MemoryDataStore::initCompositeListener_()
{
  auto local = std::make_shared<CompositeDataStoreListener>();
  local->add(std::make_shared<HostChildCache>(hostToChildren_));
  originalIdCache_ = std::make_shared<OriginalIdCache>();
  local->add(originalIdCache_);
  sliceCacheObserver_ = std::make_shared<SliceCacheObserver>(*this);
  local->add(sliceCacheObserver_);
  local->add(std::make_shared<ReflectionObserver>(*this));
  addListener(local);
}

void MemoryDataStore::clear()
{
  for (ListenerList::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
    (**i).onScenarioDelete(this);

  clearMemory_();
}

void MemoryDataStore::clearMemory_()
{
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
 * @return the state of interpolation
 */
bool MemoryDataStore::enableInterpolation(bool state)
{
  // interpolation can only be enabled if there is an interpolator
  if (state && interpolator_ != nullptr)
  {
    if (interpolationEnabled_ == InterpolatorState::OFF)
    {
      hasChanged_ = true;
      interpolationEnabled_ = InterpolatorState::EXTERNAL;
      sliceCacheObserver_->resetPlatforms();
    }
  }
  else
  {
    // no interpolator set, disable
    if (interpolationEnabled_ != InterpolatorState::OFF)
    {
      hasChanged_ = true;
      interpolationEnabled_ = InterpolatorState::OFF;
      sliceCacheObserver_->resetPlatforms();
    }

  }

  return interpolationEnabled_ != InterpolatorState::OFF;
}

///Indicates that interpolation is either enabled or disabled
bool MemoryDataStore::isInterpolationEnabled() const
{
  return (interpolationEnabled_ != InterpolatorState::OFF) && interpolator_ != nullptr;
}

///Specifies the interpolator
void MemoryDataStore::setInterpolator(Interpolator *interpolator)
{
  if (interpolator_ != interpolator)
  {
    interpolator_ = interpolator;
    hasChanged_ = true;
    sliceCacheObserver_->resetPlatforms();
  }
}

/// Get the current interpolator (nullptr if disabled)
Interpolator* MemoryDataStore::interpolator() const
{
  return (interpolationEnabled_ != InterpolatorState::OFF) ? interpolator_ : nullptr;
}

bool MemoryDataStore::enableInterpolation(InterpolatorState state)
{
  if (interpolationEnabled_ == state)
    return interpolationEnabled_ != InterpolatorState::OFF;

  hasChanged_ = true;
  interpolationEnabled_ = state;
  sliceCacheObserver_->resetPlatforms();

  return interpolationEnabled_ != InterpolatorState::OFF;
}

DataStore::InterpolatorState MemoryDataStore::interpolatorState() const
{
  return interpolationEnabled_;
}

void MemoryDataStore::updateTargetBeam_(ObjectId id, BeamEntry* beam, double time)
{
  // Get the two platforms, if available
  if (!beam->properties()->has_hostid())
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  if (!beam->preferences()->has_targetid())
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  Platforms::iterator plat = platforms_.find(beam->properties()->hostid());
  if ((plat == platforms_.end()) || (plat->second == nullptr))
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  PlatformEntry* sourcePlatform = plat->second;
  const PlatformUpdate* sourceUpdate = sourcePlatform->updates()->current();
  if ((sourceUpdate == nullptr) || (!sourceUpdate->has_position()))
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  plat = platforms_.find(beam->preferences()->targetid());
  if ((plat == platforms_.end()) || (plat->second == nullptr))
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  PlatformEntry* destPlatform = plat->second;
  const PlatformUpdate* destUpdate = destPlatform->updates()->current();
  if ((destUpdate == nullptr) || (!destUpdate->has_position()))
  {
    beam->updates()->setCurrent(nullptr);
    return;
  }

  // target beam has no updates; it uses the currentInterpolated() to deliver info to simVis::Beam
  BeamUpdate* update = beam->updates()->currentInterpolated();

  // simVis::Beam will calculate the RAE whenever it gets a non-nullptr update with hasChanged flag set

  // update only when there is a time change or this is a null->non-null transition
  if (beam->updates()->current() == nullptr || update->time() != time)
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

    // until we have datadraw, send nullptr; once we have datadraw, we'll immediately update with valid data
    if (!beamEntry->preferences()->commonprefs().datadraw())
      beamEntry->updates()->setCurrent(nullptr);
    else if (beamEntry->properties()->type() == BeamProperties::Type::TARGET)
      updateTargetBeam_(iter->first, beamEntry, time);
    else if (isInterpolationEnabled() && beamEntry->preferences()->interpolatebeampos())
      beamEntry->updates()->update(time, interpolator_);
    else
      beamEntry->updates()->update(time);
  }
}

simData::MemoryDataStore::BeamEntry* MemoryDataStore::getBeamForGate_(uint64_t gateID)
{
  Beams::iterator beamIt = beams_.find(gateID);
  if ((beamIt == beams_.end()) || (beamIt->second == nullptr))
    return nullptr;

  return beamIt->second;
}

void MemoryDataStore::updateTargetGate_(GateEntry* gate, double time)
{
  // this should only be called for target gates; if assert fails, check caller
  assert(gate->properties()->type() == GateProperties::Type::TARGET);

  // Get the host beam for this gate
  if (!gate->properties()->has_hostid())
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  BeamEntry* beam = getBeamForGate_(gate->properties()->hostid());
  // target gates can only be hosted by target beams. if assert fails, run away.
  assert(beam && beam->properties()->type() == BeamProperties::Type::TARGET);
  if (!beam || !beam->properties()->has_hostid() || beam->properties()->type() != BeamProperties::Type::TARGET || !beam->preferences()->has_targetid())
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  Platforms::iterator plat = platforms_.find(beam->properties()->hostid());
  if ((plat == platforms_.end()) || (plat->second == nullptr))
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  PlatformEntry* sourcePlatform = plat->second;
  const PlatformUpdate* sourceUpdate = sourcePlatform->updates()->current();
  if ((sourceUpdate == nullptr) || (!sourceUpdate->has_position()))
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  plat = platforms_.find(beam->preferences()->targetid());
  if ((plat == platforms_.end()) || (plat->second == nullptr))
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  PlatformEntry* destPlatform = plat->second;
  const PlatformUpdate* destUpdate = destPlatform->updates()->current();
  if ((destUpdate == nullptr) || (!destUpdate->has_position()))
  {
    gate->updates()->setCurrent(nullptr);
    return;
  }

  const bool gateWasOff = (gate->updates()->current() == nullptr);

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
  if (currentUpdate == nullptr)
    return;

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

  if (gateUsesBeamBeamwidth_(gate))
  {
    update->set_width(0.0);
    update->set_height(0.0);
  }
  else
  {
    update->set_width(currentUpdate->width());
    update->set_height(currentUpdate->height());
  }

  gate->updates()->setCurrent(update);

  // signal that this slice is updated, necessary for the time-change case and for using BeamBeamwidth case
  gate->updates()->setChanged();

}

bool MemoryDataStore::gateUsesBeamBeamwidth_(GateEntry* gate) const
{
  const GateUpdate* currentUpdate = gate->updates()->current();
  return (currentUpdate != nullptr && gate->properties()->has_hostid() &&
    (currentUpdate->height() <= 0.0 || currentUpdate->width() <= 0.0));
}

void MemoryDataStore::updateGates_(double time)
{
  for (Gates::iterator iter = gates_.begin(); iter != gates_.end(); ++iter)
  {
    GateEntry* gateEntry = iter->second;

    // until we have datadraw, send nullptr; once we have datadraw, we'll immediately update with valid data
    if (!gateEntry->preferences()->commonprefs().datadraw())
      gateEntry->updates()->setCurrent(nullptr);
    else if (gateEntry->properties()->type() == GateProperties::Type::TARGET)
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

    // until we have datadraw, send nullptr; once we have datadraw, we'll immediately update with valid data
    if (!laserEntry->preferences()->commonprefs().datadraw())
      laserEntry->updates()->setCurrent(nullptr);
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

void MemoryDataStore::flushEntity_(ObjectId id, simData::ObjectType type, FlushScope flushScope, FlushFields flushFields, double startTime, double endTime)
{
  const bool recursive = (flushScope == FLUSH_RECURSIVE);
  const bool flushUpdates = ((flushFields & FLUSH_UPDATES) != 0);
  const bool flushCommands = ((flushFields & FLUSH_COMMANDS) != 0);
  IdList ids;
  switch (type)
  {
  case PLATFORM:
    flushEntityData(platforms_, id, flushUpdates, flushCommands, startTime, endTime);
    if (recursive)
    {
      beamIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::BEAM, flushScope, flushFields, startTime, endTime);
      ids.clear();
      laserIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::LASER, flushScope, flushFields, startTime, endTime);
      ids.clear();
      lobGroupIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::LOB_GROUP, flushScope, flushFields, startTime, endTime);
      ids.clear();
      projectorIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::PROJECTOR, flushScope, flushFields, startTime, endTime);
      ids.clear();
      customRenderingIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::CUSTOM_RENDERING, flushScope, flushFields, startTime, endTime);
    }
    break;
  case BEAM:
    flushEntityData(beams_, id, flushUpdates, flushCommands, startTime, endTime);
    if (recursive)
    {
      gateIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::GATE, flushScope, flushFields, startTime, endTime);
      ids.clear();
      projectorIdListForHost(id, &ids);
      for (IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
        flushEntity_(*iter, simData::PROJECTOR, flushScope, flushFields, startTime, endTime);
    }
    break;
  case GATE:
    flushEntityData(gates_, id, flushUpdates, flushCommands, startTime, endTime);
    break;
  case LASER:
    flushEntityData(lasers_, id, flushUpdates, flushCommands, startTime, endTime);
    break;
  case LOB_GROUP:
    flushEntityData(lobGroups_, id, flushUpdates, flushCommands, startTime, endTime);
    break;
  case PROJECTOR:
    flushEntityData(projectors_, id, flushUpdates, flushCommands, startTime, endTime);
    break;
  case CUSTOM_RENDERING:
    flushEntityData(customRenderings_, id, flushUpdates, flushCommands, startTime, endTime);
    break;
  case ALL:
  case NONE:
    break;
  }

  if ((flushFields & FLUSH_CATEGORY_DATA) != 0)
  {
    auto it = categoryData_.find(id);
    if (it != categoryData_.end())
    {
      if ((startTime == 0.0) && (endTime == std::numeric_limits<double>::max()) && ((flushFields & FLUSH_EXCLUDE_MINUS_ONE) != 0))
        it->second->flush();
      else
        it->second->flush(startTime, endTime);
    }
  }

  if ((flushFields & FLUSH_GENERIC_DATA) != 0)
  {
    const auto it = genericData_.find(id);
    if (it != genericData_.end())
    {
      if ((startTime <= 0.0) && (endTime == std::numeric_limits<double>::max()))
        it->second->flush();
      else
        it->second->flush(startTime, endTime);
    }
  }

  if ((flushFields & FLUSH_DATA_TABLES) != 0)
    flushDataTables_(id, startTime, endTime);
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
  if (ownerTables != nullptr)
  {
    FlushVisitor flushVisitor;
    ownerTables->accept(flushVisitor);
  }
}

void MemoryDataStore::flushDataTables_(ObjectId id, double startTime, double endTime)
{
  /// Defines a visitor function that flushes tables
  class FlushVisitor : public simData::TableList::Visitor
  {
  public:
    FlushVisitor(double startTime, double endTime)
    : startTime_(startTime),
    endTime_(endTime)
    {
    }
    virtual void visit(simData::DataTable* table)
    {
      if ((startTime_ <= 0.0) && (endTime_ == std::numeric_limits<double>::max()))
        table->flush();
      else
        table->flush(startTime_, endTime_);
    }
  private:
    double startTime_;
    double endTime_;
  };

  // Visit all tables and flush them
  const simData::TableList* ownerTables = dataTableManager().tablesForOwner(id);
  if (ownerTables != nullptr)
  {
    FlushVisitor flushVisitor(startTime, endTime);
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

  std::map<simData::ObjectId, CommitResult> results;
  sliceCacheObserver_->updateCommands(time, results);
  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  invokePreferenceChangeCallback_(results, localCopy);

  std::vector<simData::ObjectId> ids;
  sliceCacheObserver_->updateCategoryData_(time, ids);

  sliceCacheObserver_->updatePlatforms_(time);
  updateBeams_(time);
  updateGates_(time);
  updateLasers_(time);
  updateProjectors_(time);
  updateLobGroups_(time);

  for (auto id : ids)
  {
    // send notification
    const simData::ObjectType ot = objectType(id);

    for (ListenerList::const_iterator j = localCopy.begin(); j != localCopy.end(); ++j)
    {
      if (*j != nullptr)
      {
        (**j).onCategoryDataChange(this, id, ot);
        checkForRemoval_(localCopy);
      }
    }
  }

  // After all the slice updates, set the new update time and notify observers
  lastUpdateTime_ = time;
  hasChanged_ = false;

  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != nullptr)
    {
      (**i).onChange(this);
      checkForRemoval_(localCopy);
    }
  }
}

void MemoryDataStore::invokePreferenceChangeCallback_(const std::map<simData::ObjectId, CommitResult>& results, ListenerList& localCopy)
{
  for (const auto& [id, result] : results)
  {
    for (const auto& localListener : localCopy)
    {
      if (localListener == nullptr)
        continue;

      localListener->onPrefsChange(this, id);
      checkForRemoval_(localCopy);
      if (result == CommitResult::NAME_CHANGED)
      {
        localListener->onNameChange(this, id);
        checkForRemoval_(localCopy);
      }
    }
  }
}

void MemoryDataStore::bindToClock(simCore::Clock* clock)
{
  if (boundClock_)
    boundClock_->removeModeChangeCallback(clockModeMonitor_);

  boundClock_ = clock;

  if (boundClock_)
    boundClock_->registerModeChangeCallback(clockModeMonitor_);
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
  if (flushId == 0)
    flushType = RECURSIVE;

  switch (flushType)
  {
  case NON_RECURSIVE:
    flush(flushId, FLUSH_NONRECURSIVE, static_cast<FlushFields>(FLUSH_ALL_EXCLUDE_MINUS_ONE & ~FLUSH_DATA_TABLES));
    break;
  case NON_RECURSIVE_TSPI_STATIC:
    flush(flushId, FLUSH_NONRECURSIVE, static_cast<FlushFields>(FLUSH_ALL & ~FLUSH_DATA_TABLES));
    break;
  case RECURSIVE:
    flush(flushId, FLUSH_RECURSIVE, FLUSH_ALL_EXCLUDE_MINUS_ONE);
    break;
  case NON_RECURSIVE_TSPI_ONLY:
    flush(flushId, FLUSH_NONRECURSIVE, FLUSH_UPDATES);
    break;
  case NON_RECURSIVE_DATA:
    flush(flushId, FLUSH_NONRECURSIVE, static_cast<FlushFields>(FLUSH_UPDATES | FLUSH_COMMANDS));
    break;
  }
}

int MemoryDataStore::flush(ObjectId id, FlushScope scope, FlushFields fields)
{
  double startTime = ((fields & FLUSH_EXCLUDE_MINUS_ONE) != 0) ? 0.0 : -1.0;
  return flush(id, scope, fields, startTime, std::numeric_limits<double>::max());
}

int MemoryDataStore::flush(ObjectId id, FlushScope scope, FlushFields fields, double startTime, double endTime)
{
  if (id == 0)
  {
    if (scope == FLUSH_RECURSIVE)
    {
      for (Platforms::const_iterator iter = platforms_.begin(); iter != platforms_.end(); ++iter)
        flushEntity_(iter->first, simData::PLATFORM, scope, fields, startTime, endTime);
      for (auto iter = customRenderings_.begin(); iter != customRenderings_.end(); ++iter)
        flushEntity_(iter->first, simData::CUSTOM_RENDERING, scope, fields, startTime, endTime);
    }

    if (fields & FLUSH_DATA_TABLES)
      flushDataTables_(id, startTime, endTime);

    if (fields & FLUSH_GENERIC_DATA)
    {
      GenericDataMap::const_iterator it = genericData_.find(0);
      if (it != genericData_.end())
      {
        if ((startTime <= 0.0) && (endTime == std::numeric_limits<double>::max()))
          it->second->flush();
        else
          it->second->flush(startTime, endTime);
      }
    }
  }
  else
  {
    auto type = objectType(id);
    if (type == simData::NONE)
      return 1;

    flushEntity_(id, type, scope, fields, startTime, endTime);
  }

  hasChanged_ = true;

  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  // now send out notification to listeners
  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != nullptr)
    {
      (*i)->onFlush(this, id);
      checkForRemoval_(localCopy);
    }
  }
  // Send out notification to the new-updates listener
  for (const auto& listenerPtr : newUpdatesListeners_)
    listenerPtr->onFlush(this, id);

  return 0;
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

///Retrieves the number of objects of 'type' contained by the DataStore
size_t MemoryDataStore::idCount(simData::ObjectType type) const
{
  size_t count = 0;
  if (type & PLATFORM)
    count += platforms_.size();

  if (type & BEAM)
    count += beams_.size();

  if (type & GATE)
    count += gates_.size();

  if (type & LASER)
    count += lasers_.size();

  if (type & PROJECTOR)
    count += projectors_.size();

  if (type & LOB_GROUP)
    count += lobGroups_.size();

  if (type & CUSTOM_RENDERING)
    count += customRenderings_.size();

  return count;
}

///Retrieve a list of IDs for objects contained by the DataStore
void MemoryDataStore::idList(IdList* ids, simData::ObjectType type) const
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
  if (ids == nullptr)
    return;
  ids->clear();

  // If null someone is call this routine before entityNameCache_ is made in the constructor
  assert(entityNameCache_ != nullptr);
  if (entityNameCache_ == nullptr)
    return;

  std::vector<const EntityNameEntry*> entries;
  entityNameCache_->getEntries(name, type, entries);
  for (std::vector<const EntityNameEntry*>::const_iterator it = entries.begin(); it != entries.end(); ++it)
    ids->push_back((*it)->id());
}


/// Retrieve a list of IDs for objects with the given original id
void MemoryDataStore::idListByOriginalId(IdList *ids, uint64_t originalId, simData::ObjectType type) const
{
  if (originalIdCache_ == nullptr)
    return;

  originalIdCache_->idListByOriginalId(ids, originalId, type);
}

///Retrieve a list of IDs for all beams associated with a platform
void MemoryDataStore::beamIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::BEAM, ids);
}

///Retrieve a list of IDs for all gates associated with a beam
void MemoryDataStore::gateIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::GATE, ids);
}

///Retrieve a list of IDs for all lasers associated with a platform
void MemoryDataStore::laserIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::LASER, ids);
}

///Retrieve a list of IDs for all projectors associated with a platform
void MemoryDataStore::projectorIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::PROJECTOR, ids);
}

///Retrieve a list of IDs for all lobGroups associated with a platform
void MemoryDataStore::lobGroupIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::LOB_GROUP, ids);
}

///Retrieve a list of IDs for all customs associated with a platform
void MemoryDataStore::customRenderingIdListForHost(ObjectId hostid, IdList *ids) const
{
  idListForHost_(hostid, simData::CUSTOM_RENDERING, ids);
}

///Adds the children of hostid of type inType to the ids list
void MemoryDataStore::idListForHost_(ObjectId hostid, simData::ObjectType inType, IdList* ids) const
{
  auto pair = hostToChildren_.equal_range(IdAndTypeKey(hostid, inType));
  for (auto iter = pair.first; iter != pair.second; ++iter)
    ids->push_back(iter->second);
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
    if (*i != nullptr)
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
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  Beams::iterator bi = beams_.find(id);
  if (bi != beams_.end())
  {
    // also delete any gates or projectors; projectorIdListForHost adds to the list
    gateIdListForHost(id, &ids);
    projectorIdListForHost(id, &ids);
    // we will need to send notifications and recurse on them as well...
    for (IdList::const_iterator i = ids.begin(); i != ids.end(); ++i)
      removeEntity(*i);

    delete bi->second;
    beams_.erase(bi);
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  if (deleteFromMap(gates_, id))
  {
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  if (deleteFromMap(lasers_, id))
  {
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  if (deleteFromMap(projectors_, id))
  {
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  if (deleteFromMap(lobGroups_, id))
  {
    fireOnPostRemoveEntity_(id, ot);
    return;
  }

  if (deleteFromMap(customRenderings_, id))
  {
    fireOnPostRemoveEntity_(id, ot);
    return;
  }
}

void MemoryDataStore::fireOnPostRemoveEntity_(ObjectId id, ObjectType ot)
{
  // Need to handle recursion so make a local copy
  ListenerList localCopy = listeners_;
  justRemoved_.clear();
  for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
  {
    if (*i != nullptr)
    {
      (**i).onPostRemoveEntity(this, id, ot);
      checkForRemoval_(localCopy);
    }
  }
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
  return entry ? entry->properties() : nullptr;
}

/// mutable version
PlatformProperties* MemoryDataStore::mutable_platformProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<PlatformProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

///@return const properties of beam with 'id'
const BeamProperties *MemoryDataStore::beamProperties(ObjectId id, Transaction *transaction) const
{
  const BeamEntry *entry = getEntry<const BeamEntry, Beams, NullTransactionImpl>(id, &beams_, transaction);
  return entry ? entry->properties() : nullptr;
}

/// mutable version
BeamProperties *MemoryDataStore::mutable_beamProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<BeamProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

///@return const properties of gate with 'id'
const GateProperties *MemoryDataStore::gateProperties(ObjectId id, Transaction *transaction) const
{
  const GateEntry *entry = getEntry<const GateEntry, Gates, NullTransactionImpl>(id, &gates_, transaction);
  return entry ? entry->properties() : nullptr;
}

/// mutable version
GateProperties *MemoryDataStore::mutable_gateProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<GateProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

///@return const properties of laser with 'id'
const LaserProperties* MemoryDataStore::laserProperties(ObjectId id, Transaction *transaction) const
{
  const LaserEntry *entry = getEntry<const LaserEntry, Lasers, NullTransactionImpl>(id, &lasers_, transaction);
  return entry ? entry->properties() : nullptr;
}

/// mutable version
LaserProperties* MemoryDataStore::mutable_laserProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<LaserProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

///@return const properties of projector with 'id'
const ProjectorProperties* MemoryDataStore::projectorProperties(ObjectId id, Transaction *transaction) const
{
  const ProjectorEntry *entry = getEntry<const ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  return entry ? entry->properties() : nullptr;
}

/// mutable version
ProjectorProperties* MemoryDataStore::mutable_projectorProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<ProjectorProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

///@return const properties of lobGroup with 'id'
const LobGroupProperties* MemoryDataStore::lobGroupProperties(ObjectId id, Transaction *transaction) const
{
  const LobGroupEntry *entry = getEntry<const LobGroupEntry, LobGroups, NullTransactionImpl>(id, &lobGroups_, transaction);
  return entry ? entry->properties() : nullptr;
}

/// mutable version
LobGroupProperties* MemoryDataStore::mutable_lobGroupProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<LobGroupProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

const CustomRenderingProperties* MemoryDataStore::customRenderingProperties(ObjectId id, Transaction *transaction) const
{
  const CustomRenderingEntry *entry = getEntry<const CustomRenderingEntry, CustomRenderings, NullTransactionImpl>(id, &customRenderings_, transaction);
  return entry ? entry->properties() : nullptr;
}

CustomRenderingProperties* MemoryDataStore::mutable_customRenderingProperties(ObjectId id, Transaction *transaction)
{
  if (!transaction)
    return nullptr;

  auto *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  if (!entry)
    return nullptr;

  auto *impl = new MutablePropertyTransactionImpl<CustomRenderingProperties>(id, entry->mutable_properties(), this, &listeners_);
  *transaction = Transaction(impl);
  return impl->properties();
}

const PlatformPrefs* MemoryDataStore::platformPrefs(ObjectId id, Transaction *transaction) const
{
  const PlatformEntry *entry = getEntry<const PlatformEntry, Platforms, NullTransactionImpl>(id, &platforms_, transaction);
  return entry ? entry->preferences() : nullptr;
}

PlatformPrefs* MemoryDataStore::mutable_platformPrefs(ObjectId id, Transaction* transaction, CommitResult* results)
{
  assert(transaction);
  PlatformEntry* entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (entry)
  {
    MutableSettingsTransactionImpl<PlatformPrefs>* impl =
      new MutableSettingsTransactionImpl<PlatformPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Platform associated with specified id was not found
  return nullptr;
}

const BeamPrefs* MemoryDataStore::beamPrefs(ObjectId id, Transaction *transaction) const
{
  const BeamEntry *entry = getEntry<const BeamEntry, Beams, NullTransactionImpl>(id, &beams_, transaction);
  return entry ? entry->preferences() : nullptr;
}

BeamPrefs* MemoryDataStore::mutable_beamPrefs(ObjectId id, Transaction* transaction, CommitResult* results)
{
  assert(transaction);
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (entry)
  {
    MutableSettingsTransactionImpl<BeamPrefs> *impl =
      new MutableSettingsTransactionImpl<BeamPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Beam associated with specified id was not found
  return nullptr;
}

const GatePrefs* MemoryDataStore::gatePrefs(ObjectId id, Transaction *transaction) const
{
  const GateEntry *entry = getEntry<const GateEntry, Gates, NullTransactionImpl>(id, &gates_, transaction);
  return entry ? entry->preferences() : nullptr;
}

GatePrefs* MemoryDataStore::mutable_gatePrefs(ObjectId id, Transaction* transaction, CommitResult* results)
{
  assert(transaction);
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (entry)
  {
    MutableSettingsTransactionImpl<GatePrefs> *impl =
      new MutableSettingsTransactionImpl<GatePrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Gate associated with specified id was not found
  return nullptr;
}

const LaserPrefs* MemoryDataStore::laserPrefs(ObjectId id, Transaction *transaction) const
{
  const LaserEntry *entry = getEntry<const LaserEntry, Lasers, NullTransactionImpl>(id, &lasers_, transaction);
  return entry ? entry->preferences() : nullptr;
}

LaserPrefs* MemoryDataStore::mutable_laserPrefs(ObjectId id, Transaction *transaction, CommitResult* results)
{
  assert(transaction);
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (entry)
  {
    MutableSettingsTransactionImpl<LaserPrefs> *impl =
      new MutableSettingsTransactionImpl<LaserPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Laser associated with specified id was not found
  return nullptr;
}

const ProjectorPrefs* MemoryDataStore::projectorPrefs(ObjectId id, Transaction *transaction) const
{
  const ProjectorEntry *entry = getEntry<const ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  return entry ? entry->preferences() : nullptr;
}

ProjectorPrefs* MemoryDataStore:: mutable_projectorPrefs(ObjectId id, Transaction *transaction, CommitResult* results)
{
  assert(transaction);
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors, NullTransactionImpl>(id, &projectors_, transaction);
  if (entry)
  {
    MutableSettingsTransactionImpl<ProjectorPrefs> *impl =
      new MutableSettingsTransactionImpl<ProjectorPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // Projector associated with specified id was not found
  return nullptr;
}

const LobGroupPrefs* MemoryDataStore::lobGroupPrefs(ObjectId id, Transaction *transaction) const
{
  const LobGroupEntry *entry = getEntry<const LobGroupEntry, LobGroups, NullTransactionImpl>(id, &lobGroups_, transaction);
  return entry ? entry->preferences() : nullptr;
}

LobGroupPrefs* MemoryDataStore::mutable_lobGroupPrefs(ObjectId id, Transaction *transaction, CommitResult* results)
{
  assert(transaction);
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (entry)
  {
    MutableSettingsTransactionImpl<LobGroupPrefs> *impl =
      new MutableSettingsTransactionImpl<LobGroupPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  // LobGroup associated with specified id was not found
  return nullptr;
}

const CustomRenderingPrefs* MemoryDataStore::customRenderingPrefs(ObjectId id, Transaction *transaction) const
{
  const CustomRenderingEntry *entry = getEntry<const CustomRenderingEntry, CustomRenderings, NullTransactionImpl>(id, &customRenderings_, transaction);
  return entry ? entry->preferences() : nullptr;
}

CustomRenderingPrefs* MemoryDataStore::mutable_customRenderingPrefs(ObjectId id, Transaction *transaction, CommitResult* results)
{
  assert(transaction);
  CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  if (entry)
  {
    MutableSettingsTransactionImpl<CustomRenderingPrefs> *impl =
      new MutableSettingsTransactionImpl<CustomRenderingPrefs>(entry->mutable_properties()->id(), entry->mutable_preferences(), this, &listeners_, results);
    *transaction = Transaction(impl);
    return impl->settings();
  }

  return nullptr;
}

const CommonPrefs* MemoryDataStore::commonPrefs(ObjectId id, Transaction* transaction) const
{
  const PlatformPrefs* plat = platformPrefs(id, transaction);
  if (plat != nullptr)
    return &plat->commonprefs();
  const BeamPrefs* beam = beamPrefs(id, transaction);
  if (beam != nullptr)
    return &beam->commonprefs();
  const GatePrefs* gate = gatePrefs(id, transaction);
  if (gate != nullptr)
    return &gate->commonprefs();
  const LaserPrefs* laser = laserPrefs(id, transaction);
  if (laser != nullptr)
    return &laser->commonprefs();
  const LobGroupPrefs* lobGroup = lobGroupPrefs(id, transaction);
  if (lobGroup != nullptr)
    return &lobGroup->commonprefs();
  const ProjectorPrefs* proj = projectorPrefs(id, transaction);
  if (proj != nullptr)
    return &proj->commonprefs();
  const CustomRenderingPrefs* custom = customRenderingPrefs(id, transaction);
  if (custom != nullptr)
    return &custom->commonprefs();

  return nullptr;
}

CommonPrefs* MemoryDataStore::mutable_commonPrefs(ObjectId id, Transaction* transaction)
{
  PlatformPrefs* plat = mutable_platformPrefs(id, transaction);
  if (plat != nullptr)
    return plat->mutable_commonprefs();
  BeamPrefs* beam = mutable_beamPrefs(id, transaction);
  if (beam != nullptr)
    return beam->mutable_commonprefs();
  GatePrefs* gate = mutable_gatePrefs(id, transaction);
  if (gate != nullptr)
    return gate->mutable_commonprefs();
  LaserPrefs* laser = mutable_laserPrefs(id, transaction);
  if (laser != nullptr)
    return laser->mutable_commonprefs();
  LobGroupPrefs* lobGroup = mutable_lobGroupPrefs(id, transaction);
  if (lobGroup != nullptr)
    return lobGroup->mutable_commonprefs();
  ProjectorPrefs* proj = mutable_projectorPrefs(id, transaction);
  if (proj != nullptr)
    return proj->mutable_commonprefs();
  CustomRenderingPrefs* custom = mutable_customRenderingPrefs(id, transaction);
  if (custom != nullptr)
    return custom->mutable_commonprefs();
  return nullptr;
}

///@return nullptr if platform for specified 'id' does not exist
PlatformUpdate* MemoryDataStore::addPlatformUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (!entry)
  {
    return nullptr;
  }

  PlatformUpdate *update = new PlatformUpdate();

  // Setup transaction
  MemoryDataSlice<PlatformUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<PlatformUpdate, MemoryDataSlice<PlatformUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if platform for specified 'id' does not exist
PlatformCommand *MemoryDataStore::addPlatformCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  if (!entry)
  {
    return nullptr;
  }

  PlatformCommand *command = new PlatformCommand();

  // Setup transaction
  MemoryCommandSlice<PlatformCommand, PlatformPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<PlatformCommand, MemoryCommandSlice<PlatformCommand, PlatformPrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if beam for specified 'id' does not exist
BeamUpdate* MemoryDataStore::addBeamUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (!entry)
  {
    return nullptr;
  }

  BeamUpdate *update = new BeamUpdate();

  // Setup transaction
  MemoryDataSlice<BeamUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<BeamUpdate, MemoryDataSlice<BeamUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if beam for specified 'id' does not exist
BeamCommand *MemoryDataStore::addBeamCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  if (!entry)
  {
    return nullptr;
  }

  BeamCommand *command = new BeamCommand();

  // Setup transaction
  MemoryCommandSlice<BeamCommand, BeamPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<BeamCommand, MemoryCommandSlice<BeamCommand, BeamPrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if gate for specified 'id' does not exist
GateUpdate* MemoryDataStore::addGateUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (!entry)
  {
    return nullptr;
  }

  GateUpdate *update = new GateUpdate();

  // Setup transaction
  MemoryDataSlice<GateUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<GateUpdate, MemoryDataSlice<GateUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if gate for specified 'id' does not exist
GateCommand *MemoryDataStore::addGateCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  if (!entry)
  {
    return nullptr;
  }

  GateCommand *command = new GateCommand();

  // Setup transaction
  MemoryCommandSlice<GateCommand, GatePrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<GateCommand, MemoryCommandSlice<GateCommand, GatePrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if laser for specified 'id' does not exist
LaserUpdate* MemoryDataStore::addLaserUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (!entry)
  {
    return nullptr;
  }

  LaserUpdate *update = new LaserUpdate();

  // Setup transaction
  MemoryDataSlice<LaserUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<LaserUpdate, MemoryDataSlice<LaserUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if laser for specified 'id' does not exist
LaserCommand *MemoryDataStore::addLaserCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  if (!entry)
  {
    return nullptr;
  }

  LaserCommand *command = new LaserCommand();

  // Setup transaction
  MemoryCommandSlice<LaserCommand, LaserPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<LaserCommand, MemoryCommandSlice<LaserCommand, LaserPrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if projector for specified 'id' does not exist
ProjectorUpdate* MemoryDataStore::addProjectorUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  if (!entry)
  {
    return nullptr;
  }

  ProjectorUpdate *update = new ProjectorUpdate();

  // Setup transaction
  MemoryDataSlice<ProjectorUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<ProjectorUpdate, MemoryDataSlice<ProjectorUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if projector for specified 'id' does not exist
ProjectorCommand *MemoryDataStore::addProjectorCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  if (!entry)
  {
    return nullptr;
  }

  ProjectorCommand *command = new ProjectorCommand();

  // Setup transaction
  MemoryCommandSlice<ProjectorCommand, ProjectorPrefs> *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<ProjectorCommand, MemoryCommandSlice<ProjectorCommand, ProjectorPrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if lobGroup for specified 'id' does not exist
LobGroupUpdate* MemoryDataStore::addLobGroupUpdate(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (!entry)
  {
    return nullptr;
  }

  LobGroupUpdate *update = new LobGroupUpdate();

  // Setup transaction
  MemoryDataSlice<LobGroupUpdate> *slice = entry->updates();
  *transaction = Transaction(new NewUpdateTransactionImpl<LobGroupUpdate, MemoryDataSlice<LobGroupUpdate> >(update, slice, this, id, true));

  return update;
}

///@return nullptr if lobGroup for specified 'id' does not exist
LobGroupCommand *MemoryDataStore::addLobGroupCommand(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  if (!entry)
  {
    return nullptr;
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
    return nullptr;
  }

  auto *command = new CustomRenderingCommand();

  // Setup transaction
  auto *slice = entry->commands();
  // Note that Command doesn't change the time bounds for this data store
  *transaction = Transaction(new NewUpdateTransactionImpl<CustomRenderingCommand, MemoryCommandSlice<CustomRenderingCommand, CustomRenderingPrefs> >(command, slice, this, id, false));

  return command;
}

///@return nullptr if generic data for specified 'id' does not exist
GenericData* MemoryDataStore::addGenericData(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  MemoryGenericDataSlice *slice = getEntry<MemoryGenericDataSlice, GenericDataMap>(id, &genericData_);
  if (!slice)
  {
    return nullptr;
  }

  GenericData *data = new GenericData();

  // Setup transaction
  if (id == 0)
    *transaction = Transaction(new NewScenarioGenericUpdateTransactionImpl<GenericData, MemoryGenericDataSlice>(data, slice, this, id, false));
  else
    *transaction = Transaction(new NewUpdateTransactionImpl<GenericData, MemoryGenericDataSlice>(data, slice, this, id, false));

  return data;
}

///@return nullptr if category data for specified 'id' does not exist
CategoryData* MemoryDataStore::addCategoryData(ObjectId id, Transaction *transaction)
{
  assert(transaction);

  MemoryCategoryDataSlice *slice = getEntry<MemoryCategoryDataSlice, CategoryDataMap> (id, &categoryData_);
  if (!slice)
  {
    return nullptr;
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
  return entry ? entry->updates() : nullptr;
}

const PlatformCommandSlice* MemoryDataStore::platformCommandSlice(ObjectId id) const
{
  PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
  return entry ? entry->commands() : nullptr;
}

const BeamUpdateSlice* MemoryDataStore::beamUpdateSlice(ObjectId id) const
{
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  return entry ? entry->updates() : nullptr;
}

const BeamCommandSlice* MemoryDataStore::beamCommandSlice(ObjectId id) const
{
  BeamEntry *entry = getEntry<BeamEntry, Beams>(id, &beams_);
  return entry ? entry->commands() : nullptr;
}

const GateUpdateSlice* MemoryDataStore::gateUpdateSlice(ObjectId id) const
{
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  return entry ? entry->updates() : nullptr;
}

const GateCommandSlice* MemoryDataStore::gateCommandSlice(ObjectId id) const
{
  GateEntry *entry = getEntry<GateEntry, Gates>(id, &gates_);
  return entry ? entry->commands() : nullptr;
}

const LaserUpdateSlice* MemoryDataStore::laserUpdateSlice(ObjectId id) const
{
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  return entry ? entry->updates() : nullptr;
}

const LaserCommandSlice* MemoryDataStore::laserCommandSlice(ObjectId id) const
{
  LaserEntry *entry = getEntry<LaserEntry, Lasers>(id, &lasers_);
  return entry ? entry->commands() : nullptr;
}

const ProjectorUpdateSlice* MemoryDataStore::projectorUpdateSlice(ObjectId id) const
{
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  return entry ? entry->updates() : nullptr;
}

const ProjectorCommandSlice* MemoryDataStore::projectorCommandSlice(ObjectId id) const
{
  ProjectorEntry *entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
  return entry ? entry->commands() : nullptr;
}

const LobGroupUpdateSlice* MemoryDataStore::lobGroupUpdateSlice(ObjectId id) const
{
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  return entry ? entry->updates() : nullptr;
}

const LobGroupCommandSlice* MemoryDataStore::lobGroupCommandSlice(ObjectId id) const
{
  LobGroupEntry *entry = getEntry<LobGroupEntry, LobGroups>(id, &lobGroups_);
  return entry ? entry->commands() : nullptr;
}

const CustomRenderingCommandSlice* MemoryDataStore::customRenderingCommandSlice(ObjectId id) const
{
  CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
  return entry ? entry->commands() : nullptr;
}

const GenericDataSlice* MemoryDataStore::genericDataSlice(ObjectId id) const
{
  return getEntry<GenericDataSlice, GenericDataMap>(id, &genericData_);
}

const CategoryDataSlice* MemoryDataStore::categoryDataSlice(ObjectId id) const
{
  return getEntry<CategoryDataSlice, CategoryDataMap>(id, &categoryData_);
}

void MemoryDataStore::installSliceTimeRangeMonitor(ObjectId id, std::function<void(double startTime, double endTime)> fn)
{
  sliceCacheObserver_->installSliceTimeRangeMonitor(id, fn);
}

int MemoryDataStore::modifyPlatformCommandSlice(ObjectId id, VisitableDataSlice<PlatformCommand>::Modifier* modifier)
{
  if (objectType(id) == simData::PLATFORM)
  {
    PlatformEntry *entry = getEntry<PlatformEntry, Platforms>(id, &platforms_);
    if (entry)
    {
      entry->commands()->modify(modifier);
      hasChanged_ = true;
      return 0;
    }
  }
  return 1;
}

int MemoryDataStore::modifyProjectorCommandSlice(ObjectId id, VisitableDataSlice<ProjectorCommand>::Modifier* modifier)
{
  if (objectType(id) == simData::PROJECTOR)
  {
    ProjectorEntry* entry = getEntry<ProjectorEntry, Projectors>(id, &projectors_);
    if (entry)
    {
      entry->commands()->modify(modifier);
      hasChanged_ = true;
      return 0;
    }
  }
  return 1;
}

int MemoryDataStore::modifyCustomRenderingCommandSlice(ObjectId id, VisitableDataSlice<CustomRenderingCommand>::Modifier* modifier)
{
  if (objectType(id) == simData::CUSTOM_RENDERING)
  {
    CustomRenderingEntry *entry = getEntry<CustomRenderingEntry, CustomRenderings>(id, &customRenderings_);
    if (entry)
    {
      entry->commands()->modify(modifier);
      hasChanged_ = true;
      return 0;
    }
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
  auto it = std::find_if(listeners_.begin(), listeners_.end(), [&callback](const ListenerPtr& ptr) { return callback->weight() < ptr->weight(); });
  listeners_.insert(it, callback);
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

void MemoryDataStore::addNewUpdatesListener(NewUpdatesListenerPtr callback)
{
  if (!callback)
    return;

  newUpdatesListeners_.push_back(callback);
  // Update table manager if going from empty to non-empty, so it starts sending us updates
  if (newUpdatesListeners_.size() == 1)
    static_cast<MemoryTable::TableManager*>(dataTableManager_)->setNewRowDataListener(newRowDataListener_);
}

void MemoryDataStore::removeNewUpdatesListener(NewUpdatesListenerPtr callback)
{
  if (!callback)
    return;
  auto iter = std::find(newUpdatesListeners_.begin(), newUpdatesListeners_.end(), callback);
  if (iter == newUpdatesListeners_.end())
    return;
  newUpdatesListeners_.erase(iter);

  // If clearing out the updates listener, then also clear out the memory table's listener for performance
  if (newUpdatesListeners_.empty())
    static_cast<MemoryTable::TableManager*>(dataTableManager_)->setNewRowDataListener({});
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
MemoryDataStore::MutableSettingsTransactionImpl<T>::MutableSettingsTransactionImpl(ObjectId id, T *settings, MemoryDataStore *store, ListenerList *observers, CommitResult* results)
: id_(id),
  committed_(false),
  notified_(false),
  nameChange_(false),
  currentSettings_(settings),
  store_(store),
  observers_(observers),
  results_(results)
{
  // create a copy of currentSettings_ for the user to experiment with
  modifiedSettings_ = currentSettings_->New();
  modifiedSettings_->CopyFrom(*currentSettings_);
  if (results_)
    *results_ = CommitResult::NO_CHANGE;
}

template<typename T>
void MemoryDataStore::MutableSettingsTransactionImpl<T>::commit()
{
  // performance: skip if there are no changes
  if (*modifiedSettings_ != *currentSettings_)
  {
    committed_ = true; // transaction is valid

    // Check for name change. It will be considered changed if the alias changes and alias is on, if the alias setting toggles,
    // or if the name switches, regardless of alias setting. This all ensures that EntityNameCache is updated properly (which
    // only ever tracks name()), and also ensures onNameChanged() observer fires properly.
    const bool useAlias = modifiedSettings_->commonprefs().usealias();
    const bool useAliasChanged = (useAlias != currentSettings_->commonprefs().usealias());
    const bool nameChanged = modifiedSettings_->commonprefs().name() != currentSettings_->commonprefs().name();
    const bool aliasChanged = useAlias && (modifiedSettings_->commonprefs().alias() != currentSettings_->commonprefs().alias());
    if (nameChanged || aliasChanged || useAliasChanged)
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

    if (results_)
    {
      if (nameChange_)
        *results_ = CommitResult::NAME_CHANGED;
      else
        *results_ = CommitResult::PREFERENCE_CHANGED;
    }
    else
    {
      // Need to handle recursion so make a local copy
      ListenerList localCopy = *observers_;
      store_->justRemoved_.clear();
      // Raise notifications for settings changes after internal data structures are updated
      for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
      {
        if (*i != nullptr)
        {
          (*i)->onPrefsChange(store_, id_);
          store_->checkForRemoval_(localCopy);
          if (nameChange_)
          {
            (*i)->onNameChange(store_, id_);
            store_->checkForRemoval_(localCopy);
          }
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
  modifiedSettings_ = nullptr;
}

//----------------------------------------------------------------------------
template<typename T>
MemoryDataStore::MutablePropertyTransactionImpl<T>::MutablePropertyTransactionImpl(ObjectId id, T *properties, MemoryDataStore *store, ListenerList *observers)
  : id_(id),
  committed_(false),
  notified_(false),
  currentProperties_(properties),
  store_(store),
  observers_(observers)
{
  // create a copy of currentProperties_ for the user to experiment with
  modifiedProperties_ = currentProperties_->New();
  modifiedProperties_->CopyFrom(*currentProperties_);
}

template<typename T>
void MemoryDataStore::MutablePropertyTransactionImpl<T>::commit()
{
  // performance: skip if there are no changes
  if (*modifiedProperties_ != *currentProperties_)
  {
    committed_ = true; // transaction is valid

    // copy the settings modified by the user into the entity settings
    currentProperties_->CopyFrom(*modifiedProperties_);
    store_->hasChanged_ = true;
  }
}

// Notification occurs on release
template<typename T>
void MemoryDataStore::MutablePropertyTransactionImpl<T>::release()
{
  // Raise the notification if changes were committed (one time only)
  if (committed_ && !notified_)
  {
    notified_ = true;

    // Need to handle recursion so make a local copy
    ListenerList localCopy = *observers_;
    store_->justRemoved_.clear();
    // Raise notifications for settings changes after internal data structures are updated
    for (ListenerList::const_iterator i = localCopy.begin(); i != localCopy.end(); ++i)
    {
      if (*i != nullptr)
      {
        (*i)->onPropertiesChange(store_, id_);
        store_->checkForRemoval_(localCopy);
      }
    }
  }
}

template<typename T>
MemoryDataStore::MutablePropertyTransactionImpl<T>::~MutablePropertyTransactionImpl()
{
  release();
  delete modifiedProperties_;
  modifiedProperties_ = nullptr;
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
  if (*modifiedSettings_ != *currentSettings_)
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
  modifiedSettings_ = nullptr;
}

//----------------------------------------------------------------------------
template <typename T, typename P>
void MemoryDataStore::NewEntryTransactionImpl<T, P>::commit()
{
  // Only need to add the entries to the container once
  if (!committed_)
  {
    committed_ = true;

    assert(entries_ != nullptr);
    assert(entry_ != nullptr);
    assert(entry_->properties() != nullptr);
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
      assert(i->second != nullptr);
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
    entry_ = nullptr;
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
        if (*i != nullptr)
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
    dataStore_->hasChanged_ = true;
    if (isEntityUpdate_)
    {
      // Notify the data store's new-update callback
      for (const auto& listenerPtr : dataStore_->newUpdatesListeners_)
        listenerPtr->onEntityUpdate(dataStore_, id_, updateTime);
    }
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
    update_ = nullptr;
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
    update_ = nullptr;
  }
}

template <typename T, typename SliceType>
MemoryDataStore::NewScenarioGenericUpdateTransactionImpl<T, SliceType>::~NewScenarioGenericUpdateTransactionImpl()
{
  // Make sure any un-stored entries are deallocated
  release();
}

//----------------------------------------------------------------------------

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
  double min = std::numeric_limits<double>::max();
  double max = -std::numeric_limits<double>::max();

  for (auto it = platforms_.begin(); it != platforms_.end(); ++it)
  {
    const auto updates = it->second->updates();
    if ((updates->numItems() == 0) || (updates->firstTime() < 0.0))
      continue;

    min = simCore::sdkMin(min, updates->firstTime());
    max = simCore::sdkMax(max, updates->lastTime());
  }

  return std::pair<double, double>(min, max);
}

}
