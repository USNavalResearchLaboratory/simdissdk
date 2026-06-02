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
   * no other modes (like global opt-in) are keeping it alive. Returns 0 on successful toggle.
   */
  int toggleExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);
  /** Sets the explicit edit state on the GOG; can fail if GOG cannot be edited. Returns 0 on success */
  int setExplicitEdit(std::shared_ptr<simVis::GOG::GogNodeInterface> gog, bool edit);

  /**
   * Forcefully removes a GOG from the controller.
   * Call this when a GOG is deleted from the scenario to prevent dangling pointers.
   */
  void removeGog(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);

  /** Returns true if the specific GOG currently has an active manipulator attached. */
  bool isEditing(const simVis::GOG::GogNodeInterface& gog) const;
  /** Returns true if the specific GOG has editing explicitly enabled */
  bool isExplicitlyEditing(const simVis::GOG::GogNodeInterface& gog) const;

  /** Turns global opt-in editing on or off. */
  void setGlobalEditMode(bool active);
  /** Returns the current state of the global edit mode. */
  bool isGlobalEditMode() const;

  /** Callback fired when the availability of globally editable shapes changes. */
  using GlobalEditAvailabilityCallback = std::function<void(bool isAvailable)>;
  /** Provide a callback to be notified when globally editable shape availability changes, e.g. to drive enable/disable buttons. */
  void setGlobalEditAvailabilityCallback(GlobalEditAvailabilityCallback cb);
  /** Returns true if there is at least one globally editable shape known to the controller. */
  bool hasGloballyEditableShapes() const;

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

  /** Returns true if the GOG is editable, and opted in to global edit */
  bool isGloballyEditable_(const simVis::GOG::GogNodeInterface& gog) const;

  /** Removes the iterator from activeManipulators_ and the scene if it's no longer needed */
  void removeIfUnused_(std::map<const simVis::GOG::GogNodeInterface*, EditState>::iterator it);
  /** Turns on edit mode, if global editing, for a single shape that just got created */
  void applyGlobalStateToShape_(std::shared_ptr<simVis::GOG::GogNodeInterface> gog);

  /** Helper to safely update the count and fire the callback if needed */
  void updateGloballyEditableCount_(int delta);

  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  osg::observer_ptr<osg::Group> manipulatorRoot_;

  /** List of all GOG pointers this class knows about. */
  std::vector<std::shared_ptr<simVis::GOG::GogNodeInterface>> editableShapes_;
  /** Maps the raw pointer of the shape to its UI edit state */
  std::map<const simVis::GOG::GogNodeInterface*, EditState> activeManipulators_;

  /** Turns global editing on and off */
  bool globalEditMode_ = false;

  /** User-supplied callback when globally editable becomes 0 or non-zero */
  GlobalEditAvailabilityCallback availabilityCallback_;
  /** Count of globally editable items */
  size_t globallyEditableCount_ = 0;
};

}
