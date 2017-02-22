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

#include "simData/EntityNameCache.h"

namespace simData {

EntityNameEntry::EntityNameEntry(simData::ObjectId id, simData::DataStore::ObjectType type)
  : id_(id),
    type_(type)
{
}

EntityNameEntry::~EntityNameEntry()
{
}

simData::ObjectId EntityNameEntry::id() const
{
  return id_;
}

simData::DataStore::ObjectType EntityNameEntry::type() const
{
  return type_;
}

//---------------------------------------------------------------------------------------------------------------------------

EntityNameCache::EntityNameCache()
{
}

EntityNameCache::~EntityNameCache()
{
  for (EntityMap::iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
    delete iter->second;
}

void EntityNameCache::getEntries(const std::string& name, simData::DataStore::ObjectType type, std::vector<const EntityNameEntry*>& entries) const
{
  const std::pair<EntityMap::const_iterator, EntityMap::const_iterator> range = entries_.equal_range(name);
  for (EntityMap::const_iterator iter = range.first; iter != range.second; ++iter)
  {
    if (iter->second->type() & type)
      entries.push_back(iter->second);
  }
}

void EntityNameCache::addEntity(const std::string& name, simData::ObjectId newId, simData::DataStore::ObjectType ot)
{
  entries_.insert(std::pair<std::string, EntityNameEntry*>(name, new EntityNameEntry(newId, ot)));
}

void EntityNameCache::removeEntity(const std::string& name, simData::ObjectId removedId, simData::DataStore::ObjectType ot)
{
  const std::pair<EntityMap::iterator, EntityMap::iterator> range = entries_.equal_range(name);
  for (EntityMap::iterator iter = range.first; iter != range.second; ++iter)
  {
    if (iter->second->id() == removedId)
    {
      delete iter->second;
      entries_.erase(iter);
      return;
    }
  }

  // The map entries_ is not consistent with the datastore
  assert(false);
}

void EntityNameCache::nameChange(const std::string& newName, const std::string& oldName, simData::ObjectId changeId)
{
  const std::pair<EntityMap::iterator, EntityMap::iterator> range = entries_.equal_range(oldName);
  for (EntityMap::iterator iter = range.first; iter != range.second; ++iter)
  {
    if (iter->second->id() == changeId)
    {
      // Make sure name actually changed; onNameChanged gets call when switching between name and alias
      if (iter->first != newName)
      {
        EntityNameEntry* entry = iter->second;
        entries_.erase(iter);
        entries_.insert(std::pair<std::string, EntityNameEntry*>(newName, entry));
      }

      return;
    }
  }

  // The map entries_ is not consistent with the datastore
  assert(false);
}


}
