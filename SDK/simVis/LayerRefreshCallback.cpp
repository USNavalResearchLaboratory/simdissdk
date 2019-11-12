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
#include "osgEarth/MapNode"
#include "osgEarth/TerrainEngineNode"
#include "simNotify/Notify.h"
#include "simVis/LayerRefreshCallback.h"

namespace simVis {

/** Custom osgEarth tag for a layer, to indicate it should trigger automatic refreshes */
static const std::string REFRESH_TAG = "refresh";

/** Callback that notifies its parent of when to watch or forget a layer. */
class LayerRefreshCallback::MapUpdatedCallback : public osgEarth::MapCallback
{
public:
  explicit MapUpdatedCallback(LayerRefreshCallback& parent)
    : parent_(parent)
  {
  }

  /** Watch a TerrainLayer when it's added */
  virtual void onLayerAdded(osgEarth::Layer* layer, unsigned index)
  {
    const osgEarth::TerrainLayer* terrainLayer = dynamic_cast<const osgEarth::TerrainLayer*>(layer);
    if (terrainLayer != NULL)
      parent_.watchLayer_(terrainLayer);
  }

  /** Forget a TerrainLayer when it's removed */
  virtual void onLayerRemoved(osgEarth::Layer* layer, unsigned index)
  {
    const osgEarth::TerrainLayer* terrainLayer = dynamic_cast<const osgEarth::TerrainLayer*>(layer);
    if (terrainLayer != NULL)
      parent_.forgetLayer_(terrainLayer);
  }

  /** Watch a TerrainLayer when it's enabled */
  virtual void onLayerEnabled(osgEarth::Layer* layer)
  {
    const osgEarth::TerrainLayer* terrainLayer = dynamic_cast<const osgEarth::TerrainLayer*>(layer);
    if (terrainLayer != NULL)
      parent_.watchLayer_(terrainLayer);
  }

  /** Forget a TerrainLayer when it's disabled */
  virtual void onLayerDisabled(osgEarth::Layer* layer)
  {
    const osgEarth::TerrainLayer* terrainLayer = dynamic_cast<const osgEarth::TerrainLayer*>(layer);
    if (terrainLayer != NULL)
      parent_.forgetLayer_(terrainLayer);
  }

private:
  LayerRefreshCallback& parent_;
};

//--------------------------------------------------------------------------

LayerRefreshCallback::LayerRefreshCallback()
  : enabled_(false)
{
  mapUpdatedCallback_ = new MapUpdatedCallback(*this);
}

LayerRefreshCallback::LayerRefreshCallback(const LayerRefreshCallback& rhs, const osg::CopyOp& copyop)
  : osg::Callback(rhs, copyop),
  enabled_(rhs.enabled_),
  mapNode_(rhs.mapNode_)
{
}

LayerRefreshCallback::~LayerRefreshCallback()
{
  setMapNode(NULL);
}

void LayerRefreshCallback::setMapNode(osgEarth::MapNode* mapNode)
{
  if (mapNode_.get() == mapNode)
    return;

  // Remove the callback from the old map node
  osg::ref_ptr<osgEarth::MapNode> oldMap;
  if (mapNode_.lock(oldMap) && oldMap->getMap())
    oldMap->getMap()->removeMapCallback(mapUpdatedCallback_.get());

  // Forget any previously watched layers
  watchedLayers_.clear();

  // Add the map callback to the new node
  mapNode_ = mapNode;
  if (mapNode_.valid() && mapNode_->getMap())
    mapNode_->getMap()->addMapCallback(mapUpdatedCallback_.get());

  enabled_ = (mapNode_ != NULL);
}

bool LayerRefreshCallback::run(osg::Object* object, osg::Object* data)
{
  runImpl_();
  return traverse(object, data);
}

void LayerRefreshCallback::runImpl_()
{
  if (!enabled_ || watchedLayers_.empty())
    return;

  // Pull out the terrain engine
  osg::ref_ptr<osgEarth::MapNode> mapNode;
  if (!mapNode_.lock(mapNode) || !mapNode->getTerrainEngine() || !mapNode->getMap())
    return;

  osgEarth::TerrainEngineNode* terrainEngine = mapNode->getTerrainEngine();
  // Loop through all watched layers
  for (auto it = watchedLayers_.begin(); it != watchedLayers_.end(); ++it)
  {
    osg::observer_ptr<const osgEarth::TerrainLayer> layer = (*it).layer;
    if (!layer.valid() || !layer->getEnabled())
    {
      assert(0); // Should not be watching a NULL or disabled layer
      continue;
    }

    // Ignore not visible layers
    if (!layer->getVisible())
      continue;

    double interval = getIntervalForLayer_(layer.get());

    // Skip layers that don't need refreshing
    if (interval == 0. || it->elapsedTime.elapsedTime() <= interval)
      continue;

    // Print debug text to the log.  This shouldn't run often, and users might need to figure out why map refreshes
    SIM_DEBUG_FP << "simVis::LayerRefreshCallback::run() attempting to refresh layer \"" << layer->getName() << "\".\n";

    const auto extents = layer->getDataExtents();
    for (const auto& extent : extents)
      terrainEngine->invalidateLayerRegion(layer.get(), extent);

    // Reset the timer for this layer
    it->elapsedTime.reset();
  }

  // NOTE: A call to terrainEngine->dirtyTerrain() is NOT required here
}

void LayerRefreshCallback::watchLayer_(const osgEarth::TerrainLayer* layer)
{
  if (layer == NULL)
    return;

  LayerInfo info;
  info.layer = layer;
  // No need to initialize info.elapsedTime (default constructor is sufficient)
  watchedLayers_.push_back(info);
}

void LayerRefreshCallback::forgetLayer_(const osgEarth::TerrainLayer* layer)
{
  if (layer == NULL)
    return;

  for (auto it = watchedLayers_.begin(); it != watchedLayers_.end(); ++it)
  {
    if (layer != it->layer.get())
      continue;
    watchedLayers_.erase(it);
    return;
  }
}

double LayerRefreshCallback::getIntervalForLayer_(const osgEarth::Layer* layer) const
{
  int refreshValue = 0;
  const auto& cfg = layer->getConfig();
  if (!cfg.get(REFRESH_TAG, refreshValue))
    return 0.0;
  return refreshValue * 60.;
}

}
