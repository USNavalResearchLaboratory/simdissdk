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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simVis/EntityFamily.h"
#include "simVis/Entity.h"
#include "simVis/Scenario.h"

#undef LC
#define LC "[EntityFamily] "

namespace simVis {

EntityFamily::EntityFamily()
{
  // nop
}


bool EntityFamily::invite(EntityNode* entity)
{
  bool joined = false;

  // check whether the new entity belongs to an entity already in our list:
  simData::ObjectId hostId;
  if (entity->getHostId(hostId))
  {
    ObjectIdSet::const_iterator i = entityIds_.find(hostId);
    if (i != entityIds_.end())
    {
      // found the host, so add this object to our list as well
      entities_.insert(entity);
      entityIds_.insert(entity->getId());
      joined = true;
    }
  }

  return joined;
}


bool EntityFamily::dismiss(EntityNode* entity)
{
  bool related = false;

  simData::ObjectId id = entity->getId();

  // remove from the ID cache:
  entityIds_.erase(id);

  // remove from the node cache. We don't need to "unconfigure" the entity
  // since it's being removed from the scenario anyway.
  EntityObserverSet::iterator i = entities_.find(entity);
  if (i != entities_.end())
  {
    if (i->valid())
    {
      related = true;
    }
    entities_.erase(i);
  }

  return related;
}

bool EntityFamily::isMember(simData::ObjectId id) const
{
  return (entityIds_.find(id) != entityIds_.end());
}


// Finds all the entities in the scenario "related" to the host,
// searching recursively.
void EntityFamily::add(const ScenarioManager& scenario, const simData::ObjectId& hostId)
{
  entityIds_.insert(hostId);
  std::set<simData::ObjectId> hostees;
  scenario.getObjectsHostedBy(hostId, hostees);
  for (std::set<simData::ObjectId>::iterator i = hostees.begin(); i != hostees.end(); ++i)
  {
    EntityNode* entity = scenario.find(*i);
    if (entity)
    {
      entities_.insert(entity);
      add(scenario, *i);
    }
  }
}

void EntityFamily::reset()
{
  entities_.clear();
  entityIds_.clear();
}

}
