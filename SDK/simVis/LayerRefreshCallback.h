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
#ifndef SIMVIS_LAYERREFRESHCALLBACK_H
#define SIMVIS_LAYERREFRESHCALLBACK_H

#include "simCore/Common/Common.h"
#include "simVis/Utils.h"

namespace simVis {

/**
 * When attached to the scene and configured with a map node, this update callback
 * will monitor for layers that require refreshing and issue the invalidate and
 * dirty calls to refresh the map periodically as configured by the user.
 *
 * This gets enabled by setting the "refresh" tag on a layer to the number of minutes
 * between periodic refreshes.  For example:
 *
 * <map ...>
 *  <WMSImage>
 *   <refresh>10</refresh>
 * ...
 * </map>
 *
 * Because there is no way to refresh only a single layer, multiple layers with various
 * refresh timers will refresh on the shortest timer.
 */
class LayerRefreshCallback : public simVis::PeriodicUpdateCallback
{
public:
  LayerRefreshCallback();
  LayerRefreshCallback(const LayerRefreshCallback& rhs, const osg::CopyOp& copyop);
  META_Object(simVis, LayerRefreshCallback);

  /** Changes the map node.  This resets the refresh timer */
  void setMapNode(osgEarth::MapNode* mapNode);

  /** Given the map, search for minimum refresh and update the interval appropriately */
  void updateInterval(osgEarth::Map* map);

  /** When this triggers, the map needs a refresh.  Invalidate layers and dirty the map */
  virtual void runPeriodicEvent(osg::Object* object, osg::Object* data);

protected:
  /** osg::Referenced-derived */
  virtual ~LayerRefreshCallback();

private:
  class MapUpdatedCallback;

  /** Searches layers for a "refresh" tag, returning 0 if no layers have a refresh, else returns minimum refresh timeout in mintues. */
  int getRefreshInMinutes_(const osgEarth::Map& map) const;

  osg::ref_ptr<MapUpdatedCallback> mapUpdatedCallback_;
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
};

}

#endif /* SIMVIS_LAYERREFRESHCALLBACK_H */
