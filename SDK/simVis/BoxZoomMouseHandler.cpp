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

#include "osgEarthUtil/EarthManipulator"
#include "osgEarthUtil/ViewFitter"

#include "simVis/BoxGraphic.h"
#include "simVis/View.h"
#include "simVis/BoxZoomMouseHandler.h"

namespace simVis
{

BoxZoomMouseHandler::BoxZoomMouseHandler(osgEarth::MapNode* mapNode)
  : mapNode_(mapNode)
{}

BoxZoomMouseHandler::~BoxZoomMouseHandler()
{}

bool BoxZoomMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::PUSH:
  {
    // only handle left mouse click, and only if there is a current focused view
    if (ea.getButtonMask() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
      return false;

    if (box_.valid() && zoomView_.valid())
      zoomView_->getOrCreateHUD()->removeChild(box_);

    simVis::View* view = dynamic_cast<simVis::View*>(aa.asView());
    if (view == NULL)
      return false;

    zoomView_ = view;

    // view is unable to maintain watch or cockpit modes when box zooming
    zoomView_->enableWatchMode(NULL, NULL);
    zoomView_->enableCockpitMode(NULL);

    originX_ = ea.getX();
    originY_ = ea.getY();
    box_ = new BoxGraphic();
    zoomView_->getOrCreateHUD()->addChild(box_);

    return true;
  }
  break;
  case osgGA::GUIEventAdapter::DRAG:
  {
    if (ea.getButtonMask() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON || !box_.valid())
      return 0;
    double curX = ea.getX();
    double curY = ea.getY();

    // may need to update the origin for a view that has a host
    double startX = originX_;
    double startY = originY_;

    // limit drawing to within the current zoom view
    const simVis::View::Extents& ext = zoomView_->getExtents();
    double viewX = ext.x_;
    double viewY = ext.y_;
    double viewWidth = ext.width_;
    double viewHeight = ext.height_;

    // if view extents are a ratio of the host, convert to global extents
    if (ext.isRatio_)
    {
      simVis::View* hostView = zoomView_->getHostView();
      if (hostView == NULL)
      {
        // There's a view that defines its extents as ratio of host, but has no host.
        // This can happen while creating a view, but should not happen by the time box zooming occurs.
        assert(0);
        return 0;
      }
      viewX = hostView->getExtents().width_ * viewX + hostView->getExtents().x_;
      viewY = hostView->getExtents().height_ * viewY + hostView->getExtents().y_;
      viewWidth = hostView->getExtents().width_ * viewWidth;
      viewHeight = hostView->getExtents().height_ * viewHeight;

      // update the start x/y based on inset extents
      startX = originX_ - viewX;
      startY = originY_ - viewY;
    }

    // add some padding to keep box inside view borders
    double padding = 2.0;
    viewX += padding;
    viewY += padding;
    viewWidth -= 3 * padding;
    viewHeight -= 3 * padding;

    if (curX < viewX)
      curX = viewX;
    if (curX > viewX + viewWidth)
      curX = viewX + viewWidth;
    if (curY < viewY)
      curY = viewY;
    if (curY > viewY + viewHeight)
      curY = viewY + viewHeight;

    // now calculate the new width and height and update the box geometry
    double width = curX - originX_;
    double height = curY - originY_;

    box_->setGeometry(startX, startY, width, height);

    return true;
  }
  break;
  case osgGA::GUIEventAdapter::RELEASE:
  {
    if (!box_.valid() || !zoomView_.valid())
      return false;

    setZoom_(originX_, originY_, box_->width(), box_->height());

    // done drawing the box, remove it
    zoomView_->getOrCreateHUD()->removeChild(box_);
    zoomView_ = NULL;
    box_ = NULL;

    return false;
  }
    break;
  default:
    break;
  }
  return false;
}

void BoxZoomMouseHandler::calculateGeoPointFromScreenXY_(double x, double y, simVis::View& view, osgEarth::SpatialReference* srs, std::vector<osgEarth::GeoPoint>& points) const
{
  osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter();
  ea->setX(x);
  ea->setY(y);

  osgEarth::GeoPoint lonLatAlt(srs, 0, 0, 0, osgEarth::ALTMODE_ABSOLUTE);
  osgUtil::LineSegmentIntersector::Intersections results;
  osg::NodePath mapNodePath;
  mapNodePath.push_back(mapNode_.get());
  if (view.computeIntersections(*ea.get(), mapNodePath, results))
  {
    // find the first hit under the mouse:
    osgUtil::LineSegmentIntersector::Intersection first = *(results.begin());
    osg::Vec3d point = first.getWorldIntersectPoint();
    lonLatAlt.fromWorld(srs, point);
    points.push_back(lonLatAlt);
  }
}

void BoxZoomMouseHandler::setZoom_(double originX, double originY, double width, double height) const
{
  if (!zoomView_.valid() || !mapNode_.valid())
    return;
  // Set up the SRS
  osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");

  // calculate the 4 corner GeoPoints from the screen coords
  std::vector<osgEarth::GeoPoint> points;
  calculateGeoPointFromScreenXY_(originX, originY, *zoomView_, srs, points);
  calculateGeoPointFromScreenXY_(originX + width, originY, *zoomView_, srs, points);
  calculateGeoPointFromScreenXY_(originX + width, originY + height, *zoomView_, srs, points);
  calculateGeoPointFromScreenXY_(originX, originY + height, *zoomView_, srs, points);

  // not enough points found for a reasonable zoom
  if (points.size() < 2)
    return;

  // use osgEarthUtil ViewFitter to create a viewpoint that encompasses the 4 corner points
  osg::Camera* cam = zoomView_->getCamera();
  osgEarth::Util::ViewFitter vf(srs.get(), cam);

  osgEarth::Viewpoint vp;
  vf.createViewpoint(points, vp);

  // prevent a case where the viewpoint can end up going under the earth
  if (vp.range().value().getValue() < 0.)
    vp.range()->set(1.0, osgEarth::Units::METERS);
  zoomView_->setViewpoint(vp);
}

}
