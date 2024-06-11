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
#ifndef SIMUTIL_DYNAMICSELECTIONPICKER_H
#define SIMUTIL_DYNAMICSELECTIONPICKER_H

#include "simCore/Common/Export.h"
#include "simVis/Picker.h"
#include "simVis/Types.h"

namespace simVis {
  class BeamNode;
  class CustomRenderingNode;
  class LaserNode;
  class LobGroupNode;
}

namespace simUtil
{

class ScreenCoordinateCalculator;

/**
 * Implementation of the advanced selection algorithm, sometimes referred to as the advanced
 * hooking algorithm (AHA), identified in U.S. Patent 5,757,358.  This algorithm was developed
 * in a Navy research laboratory and was patented, but can be used without restriction.
 *
 * This algorithm improves selection ability by allowing the mouse to select if it is close to
 * an item, dynamically adjusting the pickable range of the item relative to items around it.
 * This improves accuracy when the display is cluttered or when a target is obscured.
 *
 * This picker supports picking of only platforms and gates at this time.  The gate picking is
 * based off gate locator, which is at the centroid node.  Gate picking is disabled by default.
 * Use the setPickMask() method to change this behavior.
 */
class SDKUTIL_EXPORT DynamicSelectionPicker : public simVis::Picker
{
public:
  DynamicSelectionPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager);

  /**
   * Changes the range (from center of object) in pixels that you can do selection.  Increasing
   * this range will make objects pickable from farther away.
   */
  void setRange(double pixelsFromCenter);

  /** Changes the pick mask.  Use this to pick only on certain entity types. */
  void setPickMask(osg::Node::NodeMask pickMask);
  /** Retrieves the current pick mask. */
  osg::Node::NodeMask pickMask() const;

  /**
   * Sets a platform advantage in terms of ratio of the range.  A value of 0.0 indicates that platforms
   * have no preference in picking.  A value closer to 1.0 indicates that platforms are more likely to
   * get picked than other entity types.  As other entity types are introduced, such as LOB, they have a
   * large picking surface and are easier to pick.  This conversely makes platforms harder to pick.
   * Adjusting this value helps to give a slight advantage to platform picking.  The default is 0.7,
   * indicating a 70%-of-range advantage to platforms over other entity types.
   */
  void setPlatformAdvantagePct(double platformAdvantage);

  enum class PickBehavior {
    /// Retrieve all entities at the "closest" range
    Closest,
    /// Retrieve all entities within the range
    AllInRange
  };

  /**
   * Pick using an arbitrary mouse coordinate. The inset under the mouse is used. While this class is
   * configured for tying into a simVis::Picker interface that automatically updates each frame, this
   * function provides a mechanism to pick entities on demand. As such, this function will not change
   * the globally picked variable using simVis::Picker::setPicked().
   * @param nodes Output of nodes picked.
   * @param mouseXy Mouse coordinates in OSG coordinate system (e.g. GUIActionAdapter::getX() and getY())
   * @param behavior Defines behavior for picking nodes.
   */
  void pickToVector(simVis::EntityVector& nodes, const osg::Vec2d& mouseXy, PickBehavior behavior);

protected:
  /** Derived from osg::Referenced, protect destructor */
  virtual ~DynamicSelectionPicker();

private:
  /** Performs the actual intersection pick. */
  void pickThisFrame_();

  /** Picks into a vector. */
  void pickToVector_(simVis::EntityVector& nodes, PickBehavior behavior, double& mouseRangeSquaredPx) const;

  /** Returns true if the entity type is pickable. */
  bool isPickable_(const simVis::EntityNode* entityNode) const;
  /** Calculates the squared range from the mouse for the given entity, returning 0 on success */
  int calculateSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::EntityNode& entityNode, double& rangeSquared) const;
  /** Special case calculation for LOBs, called by calculateSquaredRange_() automatically, returning 0 on success */
  int calculateLobSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::LobGroupNode& lobNode, double& rangeSquared) const;
  /** Special case calculation for CustomRenderings, called by calculateSquaredRange_() automatically, returning 0 on success */
  int calculateCustomRenderRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::CustomRenderingNode& customNode, double& rangeSquared) const;
  /** Special case calculation for Lasers, called by calculateSquaredRange_() automatically, returning 0 on success */
  int calculateLaserRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::LaserNode& laserNode, double& rangeSquared) const;
  /** Special case calculation for Beams, called by calculateSquaredRange_() automatically, returning 0 on success */
  int calculateBeamRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::BeamNode& beamNode, double& rangeSquared) const;

  /** Convenience method to find the squared range from the cursor to the closest point within ecefVec, returning 0 on success */
  int calculateScreenRangePoints_(simUtil::ScreenCoordinateCalculator& calc, const std::vector<osg::Vec3d>& ecefVec, double& rangeSquared) const;
  /** Convenience method to find the squared range from the cursor to the line segments formed by treating ecefVec as successive end points, returning 0 on success */
  int calculateScreenRangeSegments_(simUtil::ScreenCoordinateCalculator& calc, const std::vector<osg::Vec3d>& ecefVec, double& rangeSquared) const;
  /** Finds the squared distance between point p and the closest point on the line described by a and b */
  double lineSegmentDistanceSquared_(const osg::Vec2d& a, const osg::Vec2d& b, const osg::Vec2d& p) const;

  class RepickEventHandler;

  /** View that the mouse was last over from a MOVE/DRAG */
  osg::observer_ptr<simVis::View> lastMouseView_;
  /** Mouse X and Y coordinates */
  osg::Vec2d mouseXy_;

  /** Callback that is used to add the picker to SDK views. */
  osg::ref_ptr<simVis::AddEventHandlerToViews> addHandlerToViews_;
  /** Event handler for requesting re-pick operation */
  osg::ref_ptr<osgGA::GUIEventHandler> guiEventHandler_;

  /** Retain a pointer to the view manager to clean up callbacks. */
  osg::observer_ptr<simVis::ViewManager> viewManager_;
  /** Pointer to the scenario manager */
  osg::observer_ptr<simVis::ScenarioManager> scenario_;

  /** Maximum valid range in pixels. */
  double maximumValidRange_;
  /** Picking mask */
  osg::Node::NodeMask pickMask_;
  /** Percentage [0,1] of advantage given to platforms over other entity types. */
  double platformAdvantagePct_;
};

}

#endif /* SIMUTIL_DYNAMICSELECTIONPICKER_H */
