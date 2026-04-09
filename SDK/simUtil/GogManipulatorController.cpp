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
#include "simVis/GOG/GogNodeInterface.h"
#include "simUtil/GogManipulatorController.h"

namespace simUtil {

GogManipulatorController::GogManipulatorController(osgEarth::MapNode* mapNode, osg::Group* manipulatorRoot)
  : mapNode_(mapNode),
    manipulatorRoot_(manipulatorRoot)
{
}

GogManipulatorController::~GogManipulatorController()
{
  // Clean up any remaining manipulators on destruction
  for (auto& pair : activeManipulators_)
  {
    if (pair.second.manipulator.valid() && manipulatorRoot_.valid())
    {
      pair.second.manipulator->clearTarget();
      manipulatorRoot_->removeChild(pair.second.manipulator.get());
    }
  }
  activeManipulators_.clear();
}

void GogManipulatorController::toggleExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (!gog || !mapNode_.valid() || !manipulatorRoot_.valid())
    return;

  // Never allow editing if the GOG inherently denies it
  if (!GogManipulator::canEdit(*gog))
    return;

  auto it = activeManipulators_.find(gog.get());
  if (it == activeManipulators_.end())
  {
    // Create new manipulator
    EditState state;
    state.explicitRequest = true;

    state.manipulator = new GogManipulator(mapNode_.get());
    state.manipulator->setTarget(gog);

    // state.manipulator->setEditFinishedCallback(...) can be hooked up here for undo capabilities

    manipulatorRoot_->addChild(state.manipulator.get());
    activeManipulators_[gog.get()] = state;
  }
  else
  {
    // Toggle the explicit state
    it->second.explicitRequest = !it->second.explicitRequest;
    removeIfUnused_(it);
  }
}

void GogManipulatorController::removeGog(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (!gog)
    return;

  auto it = activeManipulators_.find(gog.get());
  if (it != activeManipulators_.end())
  {
    // Clear flags to trigger cleanup in evaluateState_()
    it->second.explicitRequest = false;
    it->second.globalRequest = false;
    removeIfUnused_(it);
  }
}

bool GogManipulatorController::isEditing(const simVis::GOG::GogNodeInterface* gog) const
{
  return activeManipulators_.find(gog) != activeManipulators_.end();
}

void GogManipulatorController::removeIfUnused_(std::map<const simVis::GOG::GogNodeInterface*, EditState>::iterator it)
{
  // Destroy manipulator if no longer requested
  if (!it->second.explicitRequest && !it->second.globalRequest)
  {
    if (it->second.manipulator.valid() && manipulatorRoot_.valid())
    {
      it->second.manipulator->clearTarget();
      manipulatorRoot_->removeChild(it->second.manipulator.get());
    }
    activeManipulators_.erase(it);
  }
}

void GogManipulatorController::setGlobalEditMode(bool active, const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& availableGogs)
{
  globalEditMode_ = active;
  for (const auto& gog : availableGogs)
    applyGlobalStateToShape_(gog);
}

void GogManipulatorController::applyGlobalStateToShape_(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (!gog || !mapNode_.valid() || !manipulatorRoot_.valid())
    return;

  // We only care about GOGs that explicitly opt-in and are safely editable
  if (!GogManipulator::isOptInForGlobalEditing(*gog) || !GogManipulator::canEdit(*gog))
    return;

  auto it = activeManipulators_.find(gog.get());

  if (globalEditMode_)
  {
    if (it == activeManipulators_.end())
    {
      EditState state;
      state.globalRequest = true;
      state.manipulator = new GogManipulator(mapNode_.get());
      state.manipulator->setTarget(gog);
      if (manipulatorRoot_.valid()) manipulatorRoot_->addChild(state.manipulator.get());
      activeManipulators_[gog.get()] = state;
    }
    else
      it->second.globalRequest = true;
  }
  else if (it != activeManipulators_.end())
  {
    it->second.globalRequest = false;
    removeIfUnused_(it);
  }
}

bool GogManipulatorController::isGlobalEditMode() const
{
  return globalEditMode_;
}

void GogManipulatorController::notifyShapesAdded(const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& addedShapes)
{
  if (!globalEditMode_)
    return;
  for (const auto& shape : addedShapes)
    applyGlobalStateToShape_(shape);
}

void GogManipulatorController::notifyShapesRemoved(const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& removedShapes)
{
  for (const auto& shape : removedShapes)
    removeGog(shape);
}

}
