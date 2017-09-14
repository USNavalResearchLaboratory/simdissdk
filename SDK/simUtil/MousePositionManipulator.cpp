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
#include <algorithm>
#include <limits>
#include "osgViewer/Viewer"
#include "osgEarth/MapNode"
#include "osgEarth/MapNodeObserver"
#include "osgEarth/TerrainEngineNode"
#include "simVis/ElevationQueryProxy.h"
#include "simUtil/MousePositionManipulator.h"

namespace simUtil {

// Sentinel value
const double MousePositionManipulator::INVALID_POSITION_VALUE = -std::numeric_limits<double>::max();

////////////////////////////////////////////////////////////////////////////////

/** Connects a MousePositionManipulator to a osgEarth::MapNodeObserver */
class MousePositionManipulator::MapChangeListener : public osg::Node, public osgEarth::MapNodeObserver
{
public:
  /** Constructor */
  explicit MapChangeListener(MousePositionManipulator& manip)
    : manip_(manip)
  {
  }

  /** Override setMapNode() from MapNodeObserver to inform the MousePositionManipulator. */
  virtual void setMapNode(osgEarth::MapNode* mapNode)
  {
    manip_.setMapNode(mapNode);
  }

  /** Override getMapNode() from MapNodeObserver. */
  virtual osgEarth::MapNode* getMapNode()
  {
    return manip_.mapNode_.get();
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "MousePositionManipulator::MapChangeListener"; }

protected:
  /** osg::Referenced-derived */
  virtual ~MapChangeListener()
  {
  }

private:
  MousePositionManipulator& manip_;
};

////////////////////////////////////////////////////////////////////////////////

MousePositionManipulator::MousePositionManipulator(osgEarth::MapNode* mapNode, osg::Group* scene)
  : mapNode_(mapNode),
    lastView_(NULL),
    lastMouseX_(0.0f),
    lastMouseY_(0.0f),
    terrainResolution_(0.00001),
    scene_(scene)
{
  assert(mapNode != NULL);
  terrainEngineNode_ = mapNode_->getTerrainEngine();
  mapNodePath_.push_back(terrainEngineNode_);
  elevationQuery_ = new simVis::ElevationQueryProxy(mapNode_->getMap(), scene);

  if (scene_.valid())
  {
    mapChangeListener_ = new MapChangeListener(*this);
    scene_->addChild(mapChangeListener_);
  }
}

MousePositionManipulator::~MousePositionManipulator()
{
  if (scene_.valid())
    scene_->removeChild(mapChangeListener_);
  delete elevationQuery_;
}

void MousePositionManipulator::setMapNode(osgEarth::MapNode* mapNode)
{
  // Avoid expensive recalculation for no gain
  if (mapNode == mapNode_.get())
    return;
  mapNode_ = mapNode;
  mapNodePath_.clear();

  // If we don't have a valid map node, then try to gracefully deal with it
  if (mapNode == NULL)
  {
    terrainEngineNode_ = NULL;
    return;
  }

  terrainEngineNode_ = mapNode_->getTerrainEngine();
  mapNodePath_.push_back(terrainEngineNode_);
  // Note that the elevation query proxy will take care of itself for updating map.
  // Elevation query proxy has a MapNodeObserver and should not be deleted.
}

osgEarth::GeoPoint MousePositionManipulator::lastLLA() const
{
  return lastLLA_;
}

void MousePositionManipulator::getLastXY(float& lastX, float& lastY) const
{
  lastX = lastMouseX_;
  lastY = lastMouseY_;
}

osgEarth::GeoPoint MousePositionManipulator::getLLA(float mx, float my, bool queryElevation) const
{
  return getLLA_(mx, my, queryElevation, true);
}

osgEarth::GeoPoint MousePositionManipulator::getLLA_(float mx, float my, bool queryElevation, bool blocking) const
{
  osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");
  osgEarth::GeoPoint lonLatAlt(srs, INVALID_POSITION_VALUE, INVALID_POSITION_VALUE, INVALID_POSITION_VALUE, osgEarth::ALTMODE_ABSOLUTE);
  if (lastView_ == NULL)
    return lonLatAlt;

  // do not display an elevation unless it is valid
  double elevation = INVALID_POSITION_VALUE;

  osgUtil::LineSegmentIntersector::Intersections results;
  if (lastView_->computeIntersections(mx, my, mapNodePath_, results))
  {
    // find the first hit under the mouse:
    osgUtil::LineSegmentIntersector::Intersection first = *(results.begin());
    osg::Vec3d point = first.getWorldIntersectPoint();
    lonLatAlt.fromWorld(srs, point);

    // Do not query altitude, if the lat and lon were invalid
    if (queryElevation && lonLatAlt.x() != INVALID_POSITION_VALUE && lonLatAlt.y() != INVALID_POSITION_VALUE)
    {
      if (0 != getElevation_(lonLatAlt, elevation, blocking))
        elevation = INVALID_POSITION_VALUE;
    }
  }

  // fromWorld returns longitude, latitude, altitude, translate to latitude, longitude, altitude
  osgEarth::GeoPoint latLonAlt(srs, lonLatAlt.y(), lonLatAlt.x(), elevation, osgEarth::ALTMODE_ABSOLUTE);
  return latLonAlt;
}

osgEarth::GeoPoint MousePositionManipulator::getLLA(bool queryElevation) const
{
  return getLLA(lastMouseX_, lastMouseY_, queryElevation);
}

void MousePositionManipulator::addListener(Listener* listener, bool queryElevation)
{
  if (queryElevation)
    llaListeners_.push_back(listener);
  else
    llListeners_.push_back(listener);
}

void MousePositionManipulator::removeListener(Listener* listener)
{
  llaListeners_.erase(std::remove(llaListeners_.begin(), llaListeners_.end(), listener), llaListeners_.end());
  llListeners_.erase(std::remove(llListeners_.begin(), llListeners_.end(), listener), llListeners_.end());
}

int MousePositionManipulator::drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // Drag and move are treated the same by this manipulator
  return move(ea, aa);
}

int MousePositionManipulator::frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // NOTE: always return 0, since we don't need to capture the frame event

  // need to fire off mouseOverLatLon on listeners if elevation query still has a pending elevation query that is finished
  if (llaListeners_.empty() || elevationQuery_ == NULL)
    return 0;
  double outElevation = 0.0;
  // this call does not block, will return false if no pending elevation query available
  if (!elevationQuery_->getPendingElevation(outElevation))
    return 0;
  lastLLA_.alt() = outElevation;
  for (std::vector<Listener*>::const_iterator iter = llaListeners_.begin(); iter != llaListeners_.end(); ++iter)
    (*iter)->mouseOverLatLon(lastLLA_.x(), lastLLA_.y(), lastLLA_.z());
  return 0;
}

int MousePositionManipulator::release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  // treat the release same as a move, which simply grabs the last lla. Need to call here in case move() is not called before the next press/release
  return move(ea, aa);
}

int MousePositionManipulator::move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
  lastView_ = static_cast<osgViewer::View*>(aa.asView());
  lastMouseX_ = ea.getX();
  lastMouseY_ = ea.getY();

  // Only query LLA if someone cares about it
  if (llaListeners_.empty() && llListeners_.empty())
  {
    lastLLA_.set(lastLLA_.getSRS(), INVALID_POSITION_VALUE, INVALID_POSITION_VALUE, INVALID_POSITION_VALUE, osgEarth::ALTMODE_ABSOLUTE);
  }
  else
  {
    // Only query altitude if someone cares
    lastLLA_ = getLLA_(lastMouseX_, lastMouseY_, !llaListeners_.empty(), false);
    for (std::vector<Listener*>::const_iterator iter = llListeners_.begin(); iter != llListeners_.end(); ++iter)
      (*iter)->mouseOverLatLon(lastLLA_.x(), lastLLA_.y(), INVALID_POSITION_VALUE);
    for (std::vector<Listener*>::const_iterator iter = llaListeners_.begin(); iter != llaListeners_.end(); ++iter)
      (*iter)->mouseOverLatLon(lastLLA_.x(), lastLLA_.y(), lastLLA_.z());
  }

  // Don't need to stop it from being processed, we just listen
  return 0;
}

int MousePositionManipulator::getElevation(const osgEarth::GeoPoint& lonLatAlt, double& elevationMeters) const
{
  return getElevation_(lonLatAlt, elevationMeters, true);
}

int MousePositionManipulator::getElevation_(const osgEarth::GeoPoint& lonLatAlt, double& elevationMeters, bool blocking) const
{
  // It's possible that elevation query is NULL for NULL maps
  if (elevationQuery_ == NULL)
    return 1;

  // The 3rd argument control how far down the angular resolution to get an answer
  return elevationQuery_->getElevation(lonLatAlt, elevationMeters, terrainResolution_, NULL, blocking) ? 0 : 1;
}
void MousePositionManipulator::setTerrainResolution(double resolutionRadians)
{
  terrainResolution_ = resolutionRadians;
}

double MousePositionManipulator::getTerrainResolution() const
{
  return terrainResolution_;
}

}
