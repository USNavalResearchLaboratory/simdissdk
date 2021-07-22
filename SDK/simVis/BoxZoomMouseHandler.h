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
#ifndef SIMVIS_BOX_ZOOM_MOUSE_HANDLER
#define SIMVIS_BOX_ZOOM_MOUSE_HANDLER

#include "osg/ref_ptr"
#include "osgGA/GUIEventHandler"
#include "osgEarth/EarthManipulator"
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
 * Base class that uses mouse events to draw a simple box graphic for selecting an area in screen
 * pixels. Allows for specifying keys for canceling the drag, and specifying the mouse button mask
 * and modifier key mask for starting the box drawing. Implement a derived class and override the
 * processGeometry_() method to handle the selected area on mouse button release.
 */
class SDKVIS_EXPORT BoxMouseHandler : public osgGA::GUIEventHandler
{
public:
  BoxMouseHandler();

  /** Handle mouse events to apply selecting area on click and drag, then applying area to view on mouse release */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  /** Button Mask to test against.  Default is osgGA::GUIEVentAdapter::LEFT_MOUSE_BUTTON. */
  void setButtonMask(int buttonMask);
  /** Sets mask of modifier keys, such as osgGA::GUIEventAdapter::MODKEY_ALT.  Default is 0. */
  void setModKeyMask(int modKeyMask);
  /** Sets a key that can be used for canceling the operation while dragging.  Default is osgGA::GUIEventAdapter::KEY_Escape. */
  void setCancelDragKey(int cancelDragKey);

protected:
  /// osg::Referenced-derived
  virtual ~BoxMouseHandler();
  /** Stops the drag without processing the geometry */
  void stopDrag_();

  /** Return true if the given view is suitable for this mouse handler's use. */
  virtual bool validateView_(const simVis::View& view) const = 0;
  /** Called on a mouse release event. Process the box's geometry. */
  virtual void processGeometry_(double originX, double originY, double widthPixels, double heightPixels) = 0;

  /// View used by the mouse handler
  osg::observer_ptr<simVis::View> view_;
  /// Starting X screen coordinate of the box
  double originX_;
  /// Starting Y screen coordinate of the box
  double originY_;
  /// Box graphic
  osg::ref_ptr<BoxGraphic> box_;

  /// Button mask for activation
  int buttonMask_;
  /// Handles mod key mask
  ModKeyHandler* modKeys_;
  /// Keyboard key for canceling the drag
  int cancelDragKey_;
};

/////////////////////////////////////////////////////////////////

/** BoxMouseHandler implementation that selects a zoom area and zooms in on the selected extents within a view. */
class SDKVIS_EXPORT BoxZoomMouseHandler : public BoxMouseHandler
{
public:
  explicit BoxZoomMouseHandler(const osgEarth::Util::EarthManipulator::ActionOptions& opts);

protected:
  /// osg::Referenced-derived
  virtual ~BoxZoomMouseHandler();

  /** Overrides from BoxMouseHandler */
  virtual bool validateView_(const simVis::View& view) const;
  virtual void processGeometry_(double originX, double originY, double widthPixels, double heightPixels);

private:
  /** Calculates an LLA GeoPoint based on the screen x and y coordinates provided. If the resulting GeoPoint is valid, it gets added to the provided points vector */
  void calculateGeoPointFromScreenXY_(double x, double y, simVis::View& view, osgEarth::SpatialReference* srs, std::vector<osgEarth::GeoPoint>& points) const;
  /** Retrieves the map node given a view */
  osgEarth::MapNode* mapNodeForView_(const simVis::View& view) const;

  /// OPTION_GOTO_RANGE_FACTOR value from the options
  double goToRangeFactor_;
  /// OPTION_DURATION from the options
  double durationSec_;
};

}

#endif
