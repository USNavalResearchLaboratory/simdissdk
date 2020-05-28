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

#ifndef SIMDATA_ENTITY_NAME_CACHE_H
#define SIMDATA_ENTITY_NAME_CACHE_H

#include <map>
#include <string>
#include <vector>

#include "simData/ObjectId.h"

namespace simData {

/// Information that is stored per entity name
class SDKDATA_EXPORT EntityNameEntry
{
public:
  /** Constructs a new EntityNameCache for the given ID and type */
  EntityNameEntry(simData::ObjectId id, simData::ObjectType type);
  virtual ~EntityNameEntry();

  /// Returns the unique id of the entity
  simData::ObjectId id() const;
  /// Returns the type of the entity
  simData::ObjectType type() const;

private:
  simData::ObjectId id_;
  simData::ObjectType type_;
};

/// Manages a multimap keyed on entity name
class SDKDATA_EXPORT EntityNameCache
{
public:
  EntityNameCache();
  virtual ~EntityNameCache();

  /// Adds the given entity to the multimap
  void addEntity(const std::string& name, simData::ObjectId newId, simData::ObjectType ot);
  /// Removes the given entity from the multimap
  void removeEntity(const std::string& name, simData::ObjectId removedId, simData::ObjectType ot);
  /// Changes the name of the given entity
  void nameChange(const std::string& newName, const std::string& oldName, simData::ObjectId changeId);
  /// Returns a vector of EntityNameEntry for the given name and given type
  void getEntries(const std::string& name, simData::ObjectType type, std::vector<const EntityNameEntry*>& entries) const;

private:
  typedef std::multimap<std::string, EntityNameEntry*> EntityMap;  /// Keyed off entity name
  EntityMap entries_;
};


}

#endif

