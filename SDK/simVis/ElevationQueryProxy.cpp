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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarth/MapNodeObserver"
#include "osgEarth/ElevationPool"
#include "osgEarth/ElevationQuery"
#include "simVis/osgEarthVersion.h"
#include "simVis/ElevationQueryProxy.h"

#ifdef HAVE_OSGEARTH_THREADING
#include "osgEarth/Threading"
#else
#include "osgEarth/ThreadingUtils"
#endif

namespace
{
bool getElevationFromSample(const osgEarth::ElevationSample& sample,
                            double& out_elevation,
                            double* out_actualResolution)
{
  if (sample.hasData())
  {
    out_elevation = sample.elevation().as(osgEarth::Units::METERS);
    if (out_elevation == NO_DATA_VALUE)
      out_elevation = 0.0;

    if (out_actualResolution)
      *out_actualResolution = sample.resolution().getValue();

    return true;
  }

  out_elevation = 0.0;
  return false;
}
}

namespace simVis
{

/// Wrapper around the osgEarth::Future class
struct ElevationQueryProxy::PrivateData
{
 /// Future object that monitors the status of the elevation query result
#if OSGEARTH_SOVERSION > 100
  osgEarth::Threading::Future<osgEarth::ElevationSample> elevationResult_;
#else
  osgEarth::Threading::Future<osgEarth::RefElevationSample> elevationResult_;
#endif
};

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
    setName("Elevation Query Proxy CB");
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

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const { return "ElevationQueryProxy::MapChangeListener"; }

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
  : lastElevation_(NO_DATA_VALUE),
    lastResolution_(NO_DATA_VALUE),
    query_(nullptr),
    map_(map),
    scene_(scene)
{
  data_ = new PrivateData();
  query_ = new osgEarth::Util::ElevationQuery(map);
  asyncSampler_ = new osgEarth::AsyncElevationSampler(map);

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
  query_ = nullptr;
  delete data_;
  data_ = nullptr;

  if (asyncSampler_)
  {
    delete asyncSampler_;
    asyncSampler_ = nullptr;
  }
}

osgEarth::Util::ElevationQuery* ElevationQueryProxy::q() const
{
  return query_;
}

bool ElevationQueryProxy::getPendingElevation(double& out_elevation, double* out_actualResolution)
{
  // if result hasn't returned yet, return early
  if (!data_->elevationResult_.isAvailable())
    return false;

#if OSGEARTH_SOVERSION > 100
  const osgEarth::ElevationSample& sample = data_->elevationResult_.get();
#else
  osg::ref_ptr<osgEarth::RefElevationSample> samplePtr = data_->elevationResult_.release();
  const osgEarth::ElevationSample& sample = *samplePtr.get();
#endif

  getElevationFromSample(sample, out_elevation, out_actualResolution);

  // cache values
  lastElevation_ = out_elevation;
  lastResolution_ = sample.resolution().getValue();

  return true;
}

bool ElevationQueryProxy::getElevationFromPool_(const osgEarth::GeoPoint& point, double& out_elevation, double desiredResolution, double* out_actualResolution, bool blocking)
{
  osg::ref_ptr<const osgEarth::Map> map;
  if (!map_.lock(map))
    return false;

  // Assume the caller expressed the desired resolution in map units.
  // A resolution or zero means "maximum available".
  osgEarth::Distance resolution(desiredResolution, map->getSRS()->getUnits());

  if (blocking)
  {
    // synchronous query - will not return until an answer is generated
    osgEarth::ElevationSample sample = map->getElevationPool()->getSample(
      point,
      resolution,
      &workingSet_);

    if (sample.hasData())
    {
      lastElevation_ = sample.elevation().as(osgEarth::Units::METERS);
      lastResolution_ = sample.resolution().getValue();
    }
  }
  else // (!blocking)
  {
    // Start a new background query.
    // Returns immediately but result not available until later.
    data_->elevationResult_ = asyncSampler_->getSample(point, resolution);
  }

  // if non-blocking, return last recorded values while waiting for result
  out_elevation = lastElevation_;
  if (out_actualResolution)
    *out_actualResolution = lastResolution_;

  return out_elevation == NO_DATA_VALUE ? false : true;
}

bool ElevationQueryProxy::getElevation(const osgEarth::GeoPoint& point, double& out_elevation, double desiredResolution, double* out_actualResolution, bool blocking)
{
  return getElevationFromPool_(point, out_elevation, desiredResolution, out_actualResolution, blocking);
}

void ElevationQueryProxy::setMap(const osgEarth::Map* map)
{
  // Avoid expensive operations on re-do of same map
  if (map == map_.get())
    return;

  delete query_;
  query_ = new osgEarth::Util::ElevationQuery(map);

  if (asyncSampler_)
  {
    delete asyncSampler_;
  }
  asyncSampler_ = new osgEarth::AsyncElevationSampler(map);

  map_ = map;

}

void ElevationQueryProxy::setMapNode(const osgEarth::MapNode* mapNode)
{
  if (mapNode == nullptr)
    setMap(nullptr);
  else
    setMap(mapNode->getMap());
}

}
