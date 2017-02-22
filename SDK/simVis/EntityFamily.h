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
#ifndef SIMVIS_ENTITY_FAMILY_H
#define SIMVIS_ENTITY_FAMILY_H

#include "simData/DataStore.h"
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include <set>

namespace simVis
{

class EntityNode;
class ScenarioManager;

/**
 * Set of related entities, i.e. entities that are joined by
 * a hierarchical hosting relationship.
 */
class SDKVIS_EXPORT EntityFamily
{
public:
  /** Set of simData::ObjectId */
  typedef std::set<simData::ObjectId>         ObjectIdSet;
  /** Set of EntityNode ref_ptr */
  typedef std::set<osg::observer_ptr<EntityNode> > EntityObserverSet;

  EntityFamily();

  /** Clears the family. */
  void reset();

  /**
   * Adds all the entities in a scenario that are connected to
   * a host entity through hosting relationships (recursively).
   * @param[in ] scenario Scenario in which to search
   * @param[in ] hostId   ID of host at which to start searching
   */
  void add(ScenarioManager* scenario, const simData::ObjectId& hostId);

  /**
   * Adds an entity to the family if and only if it is hosted by one of the
   * entities already in the family.
   * @param[in ] entity Entity to consider adding to the family
   * @return TRUE if and only if the entity was related and joined the family.
   */
  bool invite(EntityNode* entity);

  /**
   * Removes an entity from the family.
   * @param[in ] entity Entity to remove from the family
   * @return TRUE if and only if the entity was dismissed
   */
  bool dismiss(EntityNode* entity);

  /**
   * Set of entities in the family.
   * @return Entity set.
   */
  EntityObserverSet& members() { return entities_; }

private:
  osg::observer_ptr<EntityNode>  host_;
  EntityObserverSet              entities_;
  ObjectIdSet                    entityIds_;
};

} // namespace simVis

#endif // SIMVIS_ENTITY_FAMILY_H
