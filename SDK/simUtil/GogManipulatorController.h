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
#pragma once

#include <map>
#include <memory>
#include "osg/Group"
#include "osg/observer_ptr"
#include "osgEarth/MapNode"
#include "simCore/Common/Export.h"
#include "simUtil/GogManipulator.h"

namespace simVis::GOG { class GogNodeInterface; }

namespace simUtil {

/** Centralized manager coordinating multiple GOG Manipulators on multiple GOGs */
class SDKUTIL_EXPORT GogManipulatorController
{
public:
  /**
   * @param mapNode Used to initialize the draggers' geographic positions.
   * @param manipulatorRoot The scene graph node where active GogManipulators will be attached.
   */
  GogManipulatorController(osgEarth::MapNode* mapNode, osg::Group* manipulatorRoot);
  virtual ~GogManipulatorController();

  /**
   * Toggles the explicit (user-requested) edit state for a specific GOG shape.
   * Will allocate and attach a GogManipulator if one does not exist, or destroy it if
   * no other modes (like global opt-in) are keeping it alive.
   */
  void toggleExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);

  /**
   * Forcefully removes a GOG from the controller.
   * Call this when a GOG is deleted from the scenario to prevent dangling pointers.
   */
  void removeGog(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);

  /** Returns true if the specific GOG currently has an active manipulator attached. */
  bool isEditing(const simVis::GOG::GogNodeInterface* gog) const;

  /** Turns global opt-in editing on or off for the provided list of GOGs. */
  void setGlobalEditMode(bool active, const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& availableGogs);
  /** Returns the current state of the global edit mode. */
  bool isGlobalEditMode() const;

  /** Call this when new shapes are loaded into the scene so the controller can apply global rules. */
  void notifyShapesAdded(const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& addedShapes);
  /** Call this before shapes are removed from memory to safely destroy active draggers. */
  void notifyShapesRemoved(const std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>>& removedShapes);

private:
  struct EditState
  {
    bool explicitRequest = false;
    bool globalRequest = false;
    osg::ref_ptr<GogManipulator> manipulator;
  };

  /** Removes the iterator from activeManipulators_ and the scene if it's no longer needed */
  void removeIfUnused_(std::map<const simVis::GOG::GogNodeInterface*, EditState>::iterator it);
  /** Turns on edit mode, if global editing, for a single shape that just got created */
  void applyGlobalStateToShape_(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);

  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  osg::observer_ptr<osg::Group> manipulatorRoot_;

  /** Maps the raw pointer of the shape to its UI edit state */
  std::map<const simVis::GOG::GogNodeInterface*, EditState> activeManipulators_;

  /** Turns global editing on and off */
  bool globalEditMode_ = false;
};

}
