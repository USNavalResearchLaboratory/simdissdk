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
#ifndef SIMUTIL_IDMAPPER_H
#define SIMUTIL_IDMAPPER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "simCore/Common/Common.h"

namespace simData { class DataStore; }

namespace simUtil {

/** Pure virtual class that is responsible for being able to map from one ID scheme to another */
class SDKUTIL_EXPORT IdMapper
{
public:
  virtual ~IdMapper() {}

  /**
   * Map ID value to local system; returns 0 on not-found.
   *
   * Note that method is non-const to permit internal caching in derived instances.
   * @param id Foreign ID to map into local system
   * @return Locally mapped ID value.  Uses 0 for not-found.
   */
  virtual uint64_t map(uint64_t id) = 0;
};

/**
 * Responsible for mapping IDs from one data store to another.  Two different instances of a
 * data store (e.g. remotely connected computers, or serialized data files) can use a class like
 * this to match IDs from the remote, secondary, or foreign data store to the local data store.
 *
 * IDs are matched by a variety of data that should reasonably be considered identifying,
 * including name, original ID, and host ID.
 */
class SDKUTIL_EXPORT DataStoreIdMapper : public IdMapper
{
public:
  explicit DataStoreIdMapper(simData::DataStore& dataStore);
  virtual ~DataStoreIdMapper();

  /** Contains identifying information from a remote or secondary source. */
  struct EntityIdData
  {
    /** ID on the remote system */
    uint64_t id;

    /** Original ID of the entity */
    uint64_t originalId;
    /** Name of the entity */
    std::string entityName;
    /** Host platform's ID on remote system, if not a platform.  Should match "id" for platforms. */
    uint64_t hostPlatformId;
  };

  /** Adds a mapping to a remote entry */
  int addMapping(const EntityIdData& mapping);
  /** Adds a mapping to a remote entry; convenience method that combines into an EntityIdData. */
  int addMapping(uint64_t id, uint64_t originalId, const std::string& entityName, uint64_t hostPlatformId);

  /** Removes a foreign remote ID from our list */
  int removeId(uint64_t id);
  /** Clear out all foreign ID mappings */
  void clearMappings();

  /** Provides an ID mapping from Remote IDs to local ID */
  virtual uint64_t map(uint64_t id);

private:
  /** Attempts to resolve the ID to a known ID on our side, returns 0 on not-found */
  uint64_t resolve_(const EntityIdData& fromIdData);
  /** Filter the ID list to only contain those with the given host ID */
  void filterToHostPlatform_(uint64_t localPlatformHost, std::vector<uint64_t>& idList) const;
  /** Filter the ID list to only contain those with the given name */
  void filterToName_(const std::string& entityName, std::vector<uint64_t>& idList) const;

  /** Removes a local ID from our list */
  int removeLocalId_(uint64_t localId);
  /** Clear out the IDs but keep the mappings */
  void clearResolvedIds_();

  /** Our data store */
  simData::DataStore& dataStore_;
  /** Listens for events like entity removal to clear out IDs */
  class DataStoreListener;
  /** Listens for events like entity removal to clear out IDs */
  std::shared_ptr<DataStoreListener> dataStoreListener_;

  /** Maps a SERVER ID to a LOCAL ID */
  std::map<uint64_t, uint64_t> resolvedIds_;
  /** Reverse lookup from LOCAL ID to SERVER ID; required for speedy removal */
  std::map<uint64_t, uint64_t> reverseResolvedIds_;
  /** Includes all mappings; useful for when resolved IDs have been removed */
  std::map<uint64_t, EntityIdData> mappings_;
};

}

#endif /* SIMUTIL_IDMAPPER_H */
