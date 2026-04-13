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

int GogManipulatorController::toggleExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (!gog || !mapNode_.valid() || !manipulatorRoot_.valid())
    return 1;

  // Never allow editing if the GOG inherently denies it
  if (!GogManipulator::canEdit(*gog))
    return 1;

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
  return 0;
}

int GogManipulatorController::setExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog, bool edit)
{
  if (!gog)
    return 1;
  if (isExplicitlyEditing(*gog) == edit)
    return 0;
  return toggleExplicitEdit(gog);
}

void GogManipulatorController::removeGog(std::shared_ptr<simVis::GOG::GogNodeInterface> gog)
{
  if (!gog)
    return;

  auto manipIt = activeManipulators_.find(gog.get());
  if (manipIt != activeManipulators_.end())
  {
    // Clear flags to trigger cleanup in evaluateState_()
    manipIt->second.explicitRequest = false;
    manipIt->second.globalRequest = false;
    removeIfUnused_(manipIt);
  }

  // Find it in the list of editableShapes_ - we do this to ensure we don't fall out of sync
  auto shapeIt = std::find(editableShapes_.begin(), editableShapes_.end(), gog);
  if (shapeIt != editableShapes_.end())
  {
    // Sync up the editable count
    if (GogManipulator::isOptInForGlobalEditing(*gog))
      updateGloballyEditableCount_(-1);
    std::erase(editableShapes_, gog);
  }
}

bool GogManipulatorController::isEditing(const simVis::GOG::GogNodeInterface& gog) const
{
  return activeManipulators_.find(&gog) != activeManipulators_.end();
}

bool GogManipulatorController::isExplicitlyEditing(const simVis::GOG::GogNodeInterface& gog) const
{
  auto it = activeManipulators_.find(&gog);
  return (it != activeManipulators_.end()) && it->second.explicitRequest;
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

void GogManipulatorController::setGlobalEditMode(bool active)
{
  globalEditMode_ = active;
  for (const auto& gog : editableShapes_)
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
  int newGlobalEdits = 0;

  // Save GOGs for later toggle of global edit
  for (const auto& shape : addedShapes)
  {
    if (shape && GogManipulator::canEdit(*shape))
    {
      editableShapes_.push_back(shape);
      if (GogManipulator::isOptInForGlobalEditing(*shape))
        ++newGlobalEdits;
    }
  }

  // Increment the global edit count with new shapes
  if (newGlobalEdits > 0)
    updateGloballyEditableCount_(newGlobalEdits);

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

void GogManipulatorController::setGlobalEditAvailabilityCallback(GlobalEditAvailabilityCallback cb)
{
  availabilityCallback_ = std::move(cb);
}

bool GogManipulatorController::hasGloballyEditableShapes() const
{
  return globallyEditableCount_ > 0;
}

void GogManipulatorController::updateGloballyEditableCount_(int delta)
{
  const bool wasAvailable = (globallyEditableCount_ > 0);
  // Avoid underflow
  if (delta < 0 && static_cast<int>(globallyEditableCount_) < -delta)
  {
    assert(0); // lost track; shouldn't happen
    globallyEditableCount_ = 0;
  }
  else
    globallyEditableCount_ += delta;
  const bool isAvailable = (globallyEditableCount_ > 0);

  // Only fire the callback if the state actually crossed the 0/1 threshold
  if (wasAvailable != isAvailable && availabilityCallback_)
    availabilityCallback_(isAvailable);
}

}
