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
#ifndef SIMVIS_SIMDIS_RANGE_TOOL_STATE_H
#define SIMVIS_SIMDIS_RANGE_TOOL_STATE_H

#include "simVis/RangeToolState.h"

namespace simRF { class RFPropagationFacade; }

namespace simVis
{
class EntityNode;
class PlatformNode;
class ScenarioManager;

/**
* Additional information for SIMDIS specific Range calculations
*/
struct SDKVIS_EXPORT SimdisEntityState : public EntityState
{
  osg::ref_ptr<const simVis::EntityNode> node_; ///< The node of the entity
  osg::ref_ptr<const simVis::PlatformNode> platformHostNode_; ///< The node of the host platform; for platforms platformHostNode_ == node_
  simRF::RFPropagationFacade* rfPropagation_;  ///< If the entity is a beam this MAY BE set

  SimdisEntityState();
  virtual ~SimdisEntityState();
};


/**
* A SIMDIS specific version
*/
struct SDKVIS_EXPORT SimdisRangeToolState : public RangeToolState
{
  SimdisRangeToolState(SimdisEntityState* beginEntity, SimdisEntityState* endEntity);

  /**
  * Fills in a entity state based on the given scenario an entity node
  * @param scenario The scenario for getting the host platform of node
  * @param node The node to extract information from
  * @param state Range Tool state information needed to do the calculations
  * @return zero on success and non-zero on failure
  */
  int populateEntityState(const simVis::ScenarioManager& scenario, const simVis::EntityNode* node, EntityState* state);

  /**
  * Calculates and caches the requested values
  * @param coord the type value to calculate and cache
  * @return the requested values, the type of values detailed in Coord
  */
  virtual osg::Vec3d coord(Coord coord);
};

}

#endif
