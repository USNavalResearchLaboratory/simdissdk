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
#include "simCore/Common/Common.h"
#include "simVis/BoxGraphic.h"

namespace osgEarth
{
  class GeoPoint;
  class MapNode;
  class SpatialReference;
}

namespace simVis
{
class View;

/** Uses mouse events to draw a simple box graphic for selecting a zoom area, and zooms in on the selected extents within a view */
class SDKVIS_EXPORT BoxZoomMouseHandler : public osgGA::GUIEventHandler
{
public:
  explicit BoxZoomMouseHandler(osgEarth::MapNode* mapNode);

  /** Handle mouse events to apply selecting zoom area on click and drag, then applying zoom area to view on mouse release */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
  /// osg::Referenced-derived
  virtual ~BoxZoomMouseHandler();

private:
  /** Calculates an LLA GeoPoint based on the screen x and y coordinates provided. If the resulting GeoPoint is valid, it gets added to the provided points vector */
  void calculateGeoPointFromScreenXY_(double x, double y, simVis::View& view, osgEarth::SpatialReference* srs, std::vector<osgEarth::GeoPoint>& points) const;
  /** Set the zoom area to the current zoom view, based on the provided extents */
  void setZoom_(double originX, double originY, double widthPixels, double heightPixels) const;

  /// view where zooming occurs
  osg::observer_ptr<simVis::View> zoomView_;
  /// map node for calculating LLA GeoPoints
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  /// starting screen coordinate x of the zoom area
  double originX_;
  /// starting screen coordinate y of the zoom area
  double originY_;
  /// box graphics for highlighting zoom area selection
  osg::ref_ptr<BoxGraphic> box_;
};

}

#endif
