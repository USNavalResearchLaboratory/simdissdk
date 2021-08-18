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
#include <cassert>
#include "simData/DataStore.h"
#include "simData/DataStoreHelpers.h"
#include "simUtil/IdMapper.h"

namespace simUtil {

class DataStoreIdMapper::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  explicit DataStoreListener(DataStoreIdMapper& mapper)
    : mapper_(mapper)
  {
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    mapper_.removeLocalId_(removedId);
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source)
  {
    mapper_.clearResolvedIds_();
  }

private:
  DataStoreIdMapper& mapper_;
};

///////////////////////////////////////////////////////////////////////

DataStoreIdMapper::DataStoreIdMapper(simData::DataStore& dataStore)
  : dataStore_(dataStore)
{
  dataStoreListener_.reset(new DataStoreListener(*this));
  dataStore_.addListener(dataStoreListener_);
}

DataStoreIdMapper::~DataStoreIdMapper()
{
  dataStore_.removeListener(dataStoreListener_);
}

uint64_t DataStoreIdMapper::map(uint64_t id)
{
  // Try to find it in resolved list first
  std::map<uint64_t, uint64_t>::const_iterator rid = resolvedIds_.find(id);
  if (rid != resolvedIds_.end())
    return rid->second;

  // Try to find the server mapping data for this ID
  std::map<uint64_t, EntityIdData>::const_iterator dataIter = mappings_.find(id);
  if (dataIter == mappings_.end()) // fail?
    return 0;

  // Attempt to resolve the ID value
  uint64_t resolved = resolve_(dataIter->second);
  if (resolved != 0)
  {
    // Record the resolution
    resolvedIds_[id] = resolved;
    // Assertion failure means one-to-many problem
    assert(reverseResolvedIds_.find(resolved) == reverseResolvedIds_.end());
    reverseResolvedIds_[resolved] = id;
  }
  return resolved;
}

int DataStoreIdMapper::addMapping(const EntityIdData& mapping)
{
  // Save for later searches; note that already-existing IDs are possible and not an error.
  mappings_[mapping.id] = mapping;
  return 0;
}

int DataStoreIdMapper::addMapping(uint64_t id, uint64_t originalId, const std::string& entityName, uint64_t hostPlatformId)
{
  EntityIdData mapping;
  mapping.id = id;
  mapping.originalId = originalId;
  mapping.entityName = entityName;
  mapping.hostPlatformId = hostPlatformId;
  return addMapping(mapping);
}

int DataStoreIdMapper::removeId(uint64_t remoteId)
{
  auto iter = mappings_.find(remoteId);
  if (iter == mappings_.end())
    return 1;
  mappings_.erase(iter);
  std::map<uint64_t, uint64_t>::iterator remoteIter = resolvedIds_.find(remoteId);
  if (remoteIter != resolvedIds_.end())
  {
    // Assertion failure means we didn't clean up somewhere properly
    assert(reverseResolvedIds_.find(remoteIter->second) != reverseResolvedIds_.end());
    reverseResolvedIds_.erase(remoteIter->second);
    resolvedIds_.erase(remoteIter);
  }
  return 0;
}

void DataStoreIdMapper::clearMappings()
{
  clearResolvedIds_();
  mappings_.clear();
}

uint64_t DataStoreIdMapper::resolve_(const EntityIdData& fromIdData)
{
  // Get the entity type -- either platform or all-but-platforms
  const bool isPlatform = (fromIdData.id == fromIdData.hostPlatformId);
  simData::ObjectType entityTypeFilter = simData::ALL;
  if (isPlatform)
    entityTypeFilter = simData::PLATFORM;
  else
    entityTypeFilter = static_cast<simData::ObjectType>(entityTypeFilter ^ simData::PLATFORM);

  // Find original IDs matching this list
  simData::DataStore::IdList ids;
  dataStore_.idListByOriginalId(&ids, fromIdData.originalId, entityTypeFilter);

  // If it's an empty list, we return; server has an ID we don't have
  if (ids.empty())
    return 0;
  // If it's a list of size 1, then we return; presume exact match
  if (ids.size() == 1)
    return ids[0];

  // Try to narrow down by host ID
  if (!isPlatform)
    filterToHostPlatform_(map(fromIdData.hostPlatformId), ids);
  // Try to return results
  if (ids.empty())
    return 0;
  if (ids.size() == 1)
    return ids[0];

  // Else we narrow it down by name.  Note that name is the most unreliable method for ID
  // matching, because in live scenarios (e.g. ReadSCORE with Legend Server) the names can
  // easily change at runtime, through automatic means (Legend Server) or manual means
  // (operator applying legend updates manually).  Because of this, we only use name as a
  // discriminator on a set of matched IDs only, and not as a primary matching parameter.
  filterToName_(fromIdData.entityName, ids);
  if (ids.size() == 1)
    return ids[0];
  // Not found
  return 0;
}

void DataStoreIdMapper::filterToHostPlatform_(uint64_t localPlatformHost, std::vector<uint64_t>& idList) const
{
  std::vector<uint64_t> matches;
  if (localPlatformHost != 0)
  {
    for (std::vector<uint64_t>::const_iterator i = idList.begin(); i != idList.end(); ++i)
    {
      if (simData::DataStoreHelpers::getPlatformHostId(*i, &dataStore_) == localPlatformHost)
        matches.push_back(*i);
    }
  }
  idList.swap(matches);
}

void DataStoreIdMapper::filterToName_(const std::string& entityName, std::vector<uint64_t>& idList) const
{
  std::vector<uint64_t> matches;
  for (std::vector<uint64_t>::const_iterator i = idList.begin(); i != idList.end(); ++i)
  {
    if (simData::DataStoreHelpers::nameFromId(*i, &dataStore_) == entityName)
      matches.push_back(*i);
  }
  idList.swap(matches);
}

int DataStoreIdMapper::removeLocalId_(uint64_t localId)
{
  std::map<uint64_t, uint64_t>::iterator localIter = reverseResolvedIds_.find(localId);
  if (localIter != reverseResolvedIds_.end())
  {
    const uint64_t remoteId = localIter->second;

    // Assertion failure means we didn't clean up somewhere properly
    assert(resolvedIds_.find(remoteId) != resolvedIds_.end());
    resolvedIds_.erase(remoteId);

    // Note that we do not remove from mappings_ here; mappings_ is a match of what the
    // server thinks we have, so it's possible that our data is just out of sync.

    reverseResolvedIds_.erase(localIter);
  }
  return 0;
}

void DataStoreIdMapper::clearResolvedIds_()
{
  resolvedIds_.clear();
  reverseResolvedIds_.clear();
}


}
