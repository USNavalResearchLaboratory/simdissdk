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
#include <cassert>
#include "osgEarthUtil/EarthManipulator"
#include "osgEarthUtil/ViewFitter"
#include "simVis/EarthManipulator.h"
#include "simVis/BoxGraphic.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/BoxZoomMouseHandler.h"

namespace simVis
{

BoxZoomMouseHandler::BoxZoomMouseHandler(const osgEarth::Util::EarthManipulator::ActionOptions& opts)
  : goToRangeFactor_(1.0),
    durationSec_(1.0),
    buttonMask_(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON),
    modKeyMask_(0),
    cancelDragKey_(osgGA::GUIEventAdapter::KEY_Escape)
{
  for (auto i = opts.begin(); i != opts.end(); ++i)
  {
    switch (i->option())
    {
    case osgEarth::Util::EarthManipulator::OPTION_GOTO_RANGE_FACTOR:
      goToRangeFactor_ = i->doubleValue();
      break;
    case osgEarth::Util::EarthManipulator::OPTION_DURATION:
      durationSec_ = i->doubleValue();
      break;
    }
  }
}

BoxZoomMouseHandler::~BoxZoomMouseHandler()
{
  // Just in case, remove any remnants of the box
  stopDrag_();
}

bool BoxZoomMouseHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  if (ea.getHandled())
  {
    // If something else intercepts the release, we should remove the box
    if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
      stopDrag_();
    return false;
  }

  // Several mouse events are intercepted when we are actively dragging.
  const bool currentlyDragging = box_.valid();
  // Assertion failure means a loss of sync internally and needs fixing
  assert(currentlyDragging == zoomView_.valid());

  switch (ea.getEventType())
  {
  case osgGA::GUIEventAdapter::PUSH:
  {
    // Ignore all button presses after we start out first drag
    if (currentlyDragging)
      return true;
    // only handle defined button press, and only if the defined mod key is pressed
    if (ea.getButtonMask() != buttonMask_ || ea.getModKeyMask() != modKeyMask_)
      return false;
    // Shouldn't happen, but make sure we don't leave the box laying around in an old view
    if (box_.valid() && zoomView_.valid())
      zoomView_->getOrCreateHUD()->removeChild(box_);

    // Only proceed is there is a current focused view
    simVis::View* view = dynamic_cast<simVis::View*>(aa.asView());
    if (view == NULL)
      return false;
    // Don't do anything if we don't have a map node, because the zoom won't be able to work
    if (!mapNodeForView_(*view))
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

  case osgGA::GUIEventAdapter::DRAG:
  {
    // Do not care about drag unless we are dragging
    if (!currentlyDragging)
      return false;
    // Button mask and mod keys don't matter; they get locked in at click

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
        return true;
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

    // Clamp the current X values
    const double curX = osg::clampBetween(static_cast<double>(ea.getX()), viewX, viewX + viewWidth);
    const double curY = osg::clampBetween(static_cast<double>(ea.getY()), viewY, viewY + viewHeight);

    // now calculate the new width and height and update the box geometry
    const double width = curX - originX_;
    const double height = curY - originY_;
    box_->setGeometry(startX, startY, width, height);

    return true;
  }

  case osgGA::GUIEventAdapter::RELEASE:
    // Only care if we are dragging
    if (!currentlyDragging)
      return false;
    if (!box_.valid() || !zoomView_.valid())
    {
      // Need to reset values here to satisfy post-conditions of both being NULL when not dragging
      box_ = NULL;
      zoomView_ = NULL;
      return false;
    }

    setZoom_(originX_, originY_, box_->width(), box_->height());

    // done drawing the box, remove it
    stopDrag_();
    return true;

  case osgGA::GUIEventAdapter::SCROLL:
    // Do not let scroll go through if we are dragging
    if (currentlyDragging)
      return true;
    break;

  case osgGA::GUIEventAdapter::KEYDOWN:
    // Only intercept the cancel-drag key
    if (currentlyDragging && ea.getKey() == cancelDragKey_)
    {
      stopDrag_();
      return true;
    }
    break;

  default:
    break;
  }
  return false;
}

void BoxZoomMouseHandler::stopDrag_()
{
  // Remove the box
  if (zoomView_.valid() && box_.valid())
    zoomView_->getOrCreateHUD()->removeChild(box_);
  box_ = NULL;
  zoomView_ = NULL;
}

void BoxZoomMouseHandler::calculateGeoPointFromScreenXY_(double x, double y, simVis::View& view, osgEarth::SpatialReference* srs, std::vector<osgEarth::GeoPoint>& points) const
{
  osg::ref_ptr<osgGA::GUIEventAdapter> ea = new osgGA::GUIEventAdapter();
  ea->setX(x);
  ea->setY(y);

  osgEarth::GeoPoint lonLatAlt(srs, 0, 0, 0, osgEarth::ALTMODE_ABSOLUTE);
  osgUtil::LineSegmentIntersector::Intersections results;
  osg::NodePath mapNodePath;
  mapNodePath.push_back(mapNodeForView_(*zoomView_));
  if (view.computeIntersections(*ea.get(), mapNodePath, results))
  {
    // find the first hit under the mouse:
    osgUtil::LineSegmentIntersector::Intersection first = *(results.begin());
    osg::Vec3d point = first.getWorldIntersectPoint();
    lonLatAlt.fromWorld(srs, point);
    points.push_back(lonLatAlt);
  }
}

void BoxZoomMouseHandler::setZoom_(double originX, double originY, double widthPixels, double heightPixels) const
{
  if (!zoomView_.valid() || !mapNodeForView_(*zoomView_))
    return;

  // if box is too small, treat as a single click and center on new position
  if (abs(widthPixels) < 2.0 || abs(heightPixels) < 2.0)
  {
    // Set up the SRS
    osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");

    // calculate the GeoPoint from the screen coords
    std::vector<osgEarth::GeoPoint> points;
    calculateGeoPointFromScreenXY_(originX, originY, *zoomView_, srs.get(), points);
    if (points.empty())
      return;
    simVis::Viewpoint vp = zoomView_->getViewpoint();
    vp.focalPoint()->vec3d() = osg::Vec3d(points.front().x(), points.front().y(), 0.);
    // Adjust the range by the factor provided
    if (goToRangeFactor_ != 1.0)
    {
      double newRange = vp.range()->as(osgEarth::Units::METERS) * goToRangeFactor_;
      if (newRange < 0.0)
        newRange = 1.0;
      vp.range()->set(newRange, osgEarth::Units::METERS);
    }
    // Break tether
    vp.setNode(NULL);
    zoomView_->setViewpoint(vp, durationSec_);
    return;
  }

  // Set up the SRS
  osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");

  // calculate the 4 corner GeoPoints from the screen coords
  std::vector<osgEarth::GeoPoint> points;
  calculateGeoPointFromScreenXY_(originX, originY, *zoomView_, srs.get(), points);
  calculateGeoPointFromScreenXY_(originX + widthPixels, originY, *zoomView_, srs.get(), points);
  calculateGeoPointFromScreenXY_(originX + widthPixels, originY + heightPixels, *zoomView_, srs.get(), points);
  calculateGeoPointFromScreenXY_(originX, originY + heightPixels, *zoomView_, srs.get(), points);

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
  zoomView_->setViewpoint(vp, durationSec_);
}

void BoxZoomMouseHandler::setButtonMask(int buttonMask)
{
  buttonMask_ = buttonMask;
}

void BoxZoomMouseHandler::setModKeyMask(int modKeyMask)
{
  modKeyMask_ = modKeyMask;
}

void BoxZoomMouseHandler::setCancelDragKey(int key)
{
  cancelDragKey_ = key;
}

osgEarth::MapNode* BoxZoomMouseHandler::mapNodeForView_(const simVis::View& view) const
{
  simVis::SceneManager* sceneManager = view.getSceneManager();
  if (sceneManager)
    return sceneManager->getMapNode();
  return NULL;
}

}
