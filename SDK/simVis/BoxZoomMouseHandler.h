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
#ifndef SIMVIS_BOX_ZOOM_MOUSE_HANDLER
#define SIMVIS_BOX_ZOOM_MOUSE_HANDLER

#include "osg/ref_ptr"
#include "osgGA/GUIEventHandler"
#include "osgEarthUtil/EarthManipulator"
#include "simCore/Common/Common.h"
#include "simVis/BoxGraphic.h"

namespace osgEarth {
  class GeoPoint;
  class MapNode;
  class SpatialReference;
}

namespace simVis
{

class ModKeyHandler;
class View;

/**
 * Uses mouse events to draw a simple box graphic for selecting a zoom area, and zooms in on the selected extents
 * within a view.  Allows for specifying keys for canceling the drag, and specifying the mouse button mask and
 * modifier key mask for starting the zoom mode.
 */
class SDKVIS_EXPORT BoxZoomMouseHandler : public osgGA::GUIEventHandler
{
public:
  explicit BoxZoomMouseHandler(const osgEarth::Util::EarthManipulator::ActionOptions& opts);

  /** Handle mouse events to apply selecting zoom area on click and drag, then applying zoom area to view on mouse release */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  /** Button Mask to test against.  Default is osgGA::GUIEVentAdapter::LEFT_MOUSE_BUTTON. */
  void setButtonMask(int buttonMask);
  /** Sets mask of modifier keys, such as osgGA::GUIEventAdapter::MODKEY_ALT.  Default is 0. */
  void setModKeyMask(int modKeyMask);
  /** Sets a key that can be used for canceling the operation while dragging.  Default is osgGA::GUIEventAdapter::KEY_Escape. */
  void setCancelDragKey(int cancelDragKey);

protected:
  /// osg::Referenced-derived
  virtual ~BoxZoomMouseHandler();

private:
  /** Calculates an LLA GeoPoint based on the screen x and y coordinates provided. If the resulting GeoPoint is valid, it gets added to the provided points vector */
  void calculateGeoPointFromScreenXY_(double x, double y, simVis::View& view, osgEarth::SpatialReference* srs, std::vector<osgEarth::GeoPoint>& points) const;
  /** Set the zoom area to the current zoom view, based on the provided extents */
  void setZoom_(double originX, double originY, double widthPixels, double heightPixels) const;
  /** Stops the drag without zooming */
  void stopDrag_();
  /** Retrieves the map node given a view */
  osgEarth::MapNode* mapNodeForView_(const simVis::View& view) const;

  /// view where zooming occurs
  osg::observer_ptr<simVis::View> zoomView_;
  /// starting screen coordinate x of the zoom area
  double originX_;
  /// starting screen coordinate y of the zoom area
  double originY_;
  /// box graphics for highlighting zoom area selection
  osg::ref_ptr<BoxGraphic> box_;

  /// OPTION_GOTO_RANGE_FACTOR value from the options
  double goToRangeFactor_;
  /// OPTION_DURATION from the options
  double durationSec_;

  /// Button mask for activation
  int buttonMask_;
  /// Handles mod key mask
  ModKeyHandler* modKeys_;
  /// Keyboard key for canceling the drag
  int cancelDragKey_;
};

}

#endif
