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
#ifndef SIMVIS_ELEVATIONQUERYPROXY_H
#define SIMVIS_ELEVATIONQUERYPROXY_H

#include "simCore/Common/Common.h"
#include "osg/observer_ptr"
#include "osg/ref_ptr"

namespace osg {
  class Group;
  class Node;
}
namespace osgEarth {
  class ElevationQuery;
  class GeoPoint;
  class Map;
  class MapNode;
}

namespace simVis
{

/**
 * Proxy to an ElevationQuery that provides synchronization / protection for MapNode changes.
 *
 * osgEarth::ElevationQuery depends on MapNode, but it is possible that the MapNode can change.
 * Because of this, whenever the MapNode changes, the ElevationQuery needs to be re-instantiated.
 * Rather than make each user of ElevationQuery deal with this problem, the ElevationQueryProxy
 * monitors for MapNode changes and re-instantiates a personal ElevationQuery.
 *
 * It relies upon the osgEarth::MapNodeObserver tag to update the MapNode.
 */
class SDKVIS_EXPORT ElevationQueryProxy
{
public:
  /**
   * Creates a new elevation query proxy, passing the Map to the subject and configuring an observer in the Scene.
   * @param map Pointer to the Map object used by the underlying ElevationQuery subject.
   * @param scene Points to an attachment location in the scene.  This instance will add a Node
   *   under there to listen for MapNode changes using the osgEarth::MapNodeObserver tag.
   */
  ElevationQueryProxy(const osgEarth::Map* map, osg::Group* scene);
  virtual ~ElevationQueryProxy();

  /**
   * Returns the subject of the proxy.  Note that this pointer may become invalid
   * at any point when the Map changes; avoid caching the return value.
   */
  osgEarth::ElevationQuery* q() const;

  /**
   * Gets the terrain elevation at a point, given a terrain resolution.
   * Convenience method that forwards to the ElevationQuery.
   *
   * @param point
   *      Coordinates for which to query elevation.
   * @param out_elevation
   *      Stores the elevation result in this variable upon success.
   * @param desiredResolution
   *      Optimal resolution of elevation data to use for the query (if available).
   *      Pass in 0 (zero) to use the best available resolution.
   * @param out_actualResolution
   *      Resolution of the resulting elevation value (if the method returns true).
   *
   * @return True if the query succeeded, false upon failure.
   */
  bool getElevation(const osgEarth::GeoPoint& point, double& out_elevation, double desiredResolution = 0.0, double* out_actualResolution = 0L);

  /**
   * @deprecated
   * Sets the maximum cache size for elevation tiles.  Forwards to ElevationQuery::setMaxTilesToCache().
   * Newer versions of osgEarth do not implement this method, as the ElevationQuery will instead use
   * the pool from the Map.  If you really need to increase the maximum tiles to cache, call the
   * Map method map->getElevationPool()->setMaxEntries().
   * @param value Number of tiles to cache
   */
  SDK_DEPRECATE(void setMaxTilesToCache(int value), "Method is removed in newer osgEarth.");

  /** Changes the MapNode that is associated with the query. */
  void setMap(const osgEarth::Map* map);
  /** Changes the MapNode that is associated with the query.  Calls setMap(osgEarth::Map*) appropriately. */
  void setMapNode(const osgEarth::MapNode* mapNode);

private:
  osgEarth::ElevationQuery* query_;
  osg::observer_ptr<const osgEarth::Map> map_;
  osg::observer_ptr<osg::Group> scene_;

  class MapChangeListener;
  osg::ref_ptr<osg::Node> mapChangeListener_;
};

}

#endif /* SIMVIS_ELEVATIONQUERYPROXY_H */
