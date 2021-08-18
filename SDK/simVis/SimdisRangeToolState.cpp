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
#include "simCore/Calc/Math.h"
#include "simVis/Beam.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SimdisRangeToolState.h"

namespace simVis
{

SimdisEntityState::SimdisEntityState()
  : node_(nullptr),
  platformHostNode_(nullptr),
  rfPropagation_(nullptr)
{
}

SimdisEntityState::~SimdisEntityState()
{
}

SimdisRangeToolState::SimdisRangeToolState(SimdisEntityState* beginEntity, SimdisEntityState* endEntity)
  : RangeToolState(beginEntity, endEntity)
{
}

osg::Vec3d SimdisRangeToolState::coord(Coord which)
{
  if (coord_[which].isSet())
    return *coord_[which];

  if ((which != COORD_BEAM_LLA_0) && (which != COORD_BEAM_LLA_1))
    return RangeToolState::coord(which);

  if (beginEntity_->type_ == simData::BEAM)
  {
    auto simdisState = dynamic_cast<SimdisEntityState*>(beginEntity_);
    if (simdisState == nullptr)
      return osg::Vec3d();

    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(simdisState->node_.get());
    // Node not defined correctly; type() and pointer should match)
    assert(beam != nullptr);
    if (beam != nullptr)
    {
      simCore::Vec3 from;
      beam->getClosestPoint(endEntity_->lla_, from);
      coord_[COORD_BEAM_LLA_0] = simCore2osg(from);
      coord_[COORD_BEAM_LLA_1] = simCore2osg(endEntity_->lla_);
    }
  }
  else
  {
    // at least one side must be a beam.  Check willAccept for errors
    assert(endEntity_->type_ == simData::BEAM);

    auto simdisState = dynamic_cast<SimdisEntityState*>(endEntity_);
    if (simdisState == nullptr)
      return osg::Vec3d();

    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(simdisState->node_.get());
    // Node not defined correctly; type() and pointer should match)
    assert(beam != nullptr);
    if (beam != nullptr)
    {
      simCore::Vec3 to;
      beam->getClosestPoint(beginEntity_->lla_, to);
      coord_[COORD_BEAM_LLA_0] = simCore2osg(beginEntity_->lla_);
      coord_[COORD_BEAM_LLA_1] = simCore2osg(to);
    }
  }

  return *coord_[which];
}

int SimdisRangeToolState::populateEntityState(const simVis::ScenarioManager& scenario, const simVis::EntityNode* node, EntityState* state)
{
  if ((node == nullptr) || (state == nullptr))
    return 1;

  auto hostNode = dynamic_cast<const simVis::PlatformNode*>(scenario.getHostPlatform(node));
  if (hostNode == nullptr)
    return 1;

  state->id_ = node->getId();
  state->type_ = node->type();
  state->hostId_ = hostNode->getId();
  if (state->type_ == simData::CUSTOM_RENDERING)
    state->hostId_ = state->id_;
  else
    state->hostId_ = hostNode->getId();

  auto simdisState = dynamic_cast<SimdisEntityState*>(state);
  if (simdisState != nullptr)
  {
    simdisState->node_ = node;
    simdisState->platformHostNode_ = hostNode;
  }

  // Kick out only after setting non-location information
  if (!node->isActive())
    return 1;

  if (0 != node->getPositionOrientation(&state->lla_, &state->ypr_, simCore::COORD_SYS_LLA))
    return 1;

  if (state->type_ == simData::PLATFORM)
  {
    // Platforms need velocity which is not available from getPositionOrientation, so add it in
    const simVis::PlatformNode* platform = dynamic_cast<const simVis::PlatformNode*>(node);
    if (platform == nullptr)
      return 1;

    const simData::PlatformUpdate* update = platform->update();
    if (update == nullptr)
      return 1;

    simCore::Coordinate ecef(simCore::COORD_SYS_ECEF,
      simCore::Vec3(update->x(), update->y(), update->z()),
      simCore::Vec3(update->psi(), update->theta(), update->phi()),
      simCore::Vec3(update->vx(), update->vy(), update->vz()));
    simCore::Coordinate needVelocity;
    simCore::CoordinateConverter::convertEcefToGeodetic(ecef, needVelocity);
    // Take only the velocity since the other values have not gone been modified by any preferences
    state->vel_ = needVelocity.velocity();
  }

  if ((simdisState != nullptr) && (simdisState->type_ == simData::BEAM))
  {
    simRF::RFPropagationManagerPtr manager = scenario.rfPropagationManager();
    simdisState->rfPropagation_ = manager->getRFPropagation(node->getId());
  }

  return 0;
}

}
