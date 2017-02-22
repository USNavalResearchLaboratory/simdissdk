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
#include "osgEarth/MapNodeObserver"
#include "osgEarth/ElevationQuery"
#include "simVis/ElevationQueryProxy.h"

namespace simVis
{

/**
 * Empty node that implements the MapNodeVisitor interface.  As it gets notifications
 * that the MapNode changes, it passes those notifications to its parent Elevation
 * Query Proxy instance.
 */
class ElevationQueryProxy::MapChangeListener : public osg::Node, public osgEarth::MapNodeObserver
{
public:
  /** Constructor that takes an Elevation Query Proxy reference */
  explicit MapChangeListener(ElevationQueryProxy& queryProxy)
    : queryProxy_(queryProxy)
  {
  }

  /** Override setMapNode() from MapNodeObserver to inform the MousePositionManipulator. */
  virtual void setMapNode(osgEarth::MapNode* mapNode)
  {
    mapNode_ = mapNode;
    queryProxy_.setMapNode(mapNode);
  }

  /** Override getMapNode() from MapNodeObserver. */
  virtual osgEarth::MapNode* getMapNode()
  {
    return mapNode_.get();
  }

protected:
  /** osg::Referenced-derived */
  virtual ~MapChangeListener()
  {
  }

private:
  ElevationQueryProxy& queryProxy_;
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
};

///////////////////////////////////////////////////////////////////////////////////

ElevationQueryProxy::ElevationQueryProxy(const osgEarth::Map* map, osg::Group* scene)
  : query_(NULL),
    map_(map),
    scene_(scene)
{
  query_ = new osgEarth::ElevationQuery(map);

#ifndef HAVE_ELEVQUERY_1016API
  // Prior to the 10/2016 API, this had to be set to true; now that's the default
  query_->setFallBackOnNoData(true);
#endif

  if (scene_.valid())
  {
    mapChangeListener_ = new MapChangeListener(*this);
    scene_->addChild(mapChangeListener_);
  }
}

ElevationQueryProxy::~ElevationQueryProxy()
{
  if (scene_.valid())
    scene_->removeChild(mapChangeListener_);
  delete query_;
  query_ = NULL;
}

osgEarth::ElevationQuery* ElevationQueryProxy::q() const
{
  return query_;
}

bool ElevationQueryProxy::getElevation(const osgEarth::GeoPoint& point, double& out_elevation, double desiredResolution, double* out_actualResolution)
{
  if (query_)
  {
#ifndef HAVE_ELEVQUERY_1016API
    // Older API used a double value and returned a boolean, like our method
    return query_->getElevation(point, out_elevation, desiredResolution, out_actualResolution);
#else
    const float value = query_->getElevation(point, desiredResolution, out_actualResolution);
    if (value ==  NO_DATA_VALUE)
    {
      out_elevation = 0.0;
      return false; // fail
    }
    out_elevation = value;
    return true;
#endif
  }
  return false; // failure
}

void ElevationQueryProxy::setMaxTilesToCache(int value)
{
#ifndef HAVE_ELEVQUERY_1016API
  // After 10/2016 API, now uses map's elevation tile pool and this method is gone.
  // If you really want to change it from default of 128, you can do so by
  // calling map->getElevationPool()->setMaxEntries().
  if (query_)
    query_->setMaxTilesToCache(value);
#endif
}

void ElevationQueryProxy::setMap(const osgEarth::Map* map)
{
  // Avoid expensive operations on re-do of same map
  if (map == map_.get())
    return;

  delete query_;
  query_ = new osgEarth::ElevationQuery(map);
}

void ElevationQueryProxy::setMapNode(const osgEarth::MapNode* mapNode)
{
  if (mapNode == NULL)
    setMap(NULL);
  else
    setMap(mapNode->getMap());
}

}
