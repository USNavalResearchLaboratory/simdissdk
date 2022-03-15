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
#ifndef SIMVIS_LAYERREFRESHCALLBACK_H
#define SIMVIS_LAYERREFRESHCALLBACK_H

#include "osg/Callback"
#include "simCore/Common/Common.h"

namespace osgEarth { class MapNode; }

namespace simVis {

/**
 * When attached to the scene and configured with a map node, this update callback
 * will monitor for layers that require refreshing and issue the invalidate and
 * dirty calls to refresh each layer periodically as configured by the user.
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
 * Note that all TerrainLayers (including those without refresh intervals) are monitored.
 * This is to catch cases where a layer is given a refresh interval after being added.
 */
class LayerRefreshCallback : public osg::Callback
{
public:
  LayerRefreshCallback();
  LayerRefreshCallback(const LayerRefreshCallback& rhs, const osg::CopyOp& copyop);
  META_Object(simVis, LayerRefreshCallback);

  /** Changes the map node. Clears the list of watched layers */
  void setMapNode(osgEarth::MapNode* mapNode);

  /** Override osg::Callback::run() to check timers and refresh layers if needed */
  virtual bool run(osg::Object* object, osg::Object* data);

protected:
  /** osg::Referenced-derived */
  virtual ~LayerRefreshCallback();

private:
  class MapUpdatedCallback;

  /** Groups a TerrainLayer pointer and the elapsed time since last its refresh */
  struct LayerInfo
  {
    osg::observer_ptr<osgEarth::TileLayer> layer;
    osg::ElapsedTime elapsedTime;
  };

  /** Implementation of run() */
  void runImpl_();

  /** Watch the given layer and refresh it when required a refresh is due */
  void watchLayer_(osgEarth::TileLayer* layer);
  /** Stop watching the given layer */
  void forgetLayer_(osgEarth::TileLayer* layer);

  /** Get the interval for the given layer in seconds */
  double getIntervalForLayer_(osgEarth::Layer* layer) const;

  bool enabled_;
  osg::ref_ptr<MapUpdatedCallback> mapUpdatedCallback_;
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  /** Info about all enabled terrain layers */
  std::vector<LayerInfo> watchedLayers_;
};

}

#endif /* SIMVIS_LAYERREFRESHCALLBACK_H */
