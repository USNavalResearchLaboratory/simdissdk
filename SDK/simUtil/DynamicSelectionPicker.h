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
#ifndef SIMUTIL_DYNAMICSELECTIONPICKER_H
#define SIMUTIL_DYNAMICSELECTIONPICKER_H

#include "simCore/Common/Export.h"
#include "simVis/Picker.h"

namespace simUtil
{

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

protected:
  /** Derived from osg::Referenced, protect destructor */
  virtual ~DynamicSelectionPicker();

private:
  /** Performs the actual intersection pick. */
  void pickThisFrame_();
  /** Returns true if the entity type is pickable. */
  bool isPickable_(const simVis::EntityNode* entityNode) const;

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

  /** Maximum valid range */
  double maximumValidRange_;
  /** Picking mask */
  osg::Node::NodeMask pickMask_;
};

}

#endif /* SIMUTIL_DYNAMICSELECTIONPICKER_H */
