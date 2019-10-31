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
#include "osgEarth/TerrainEngineNode"
#include "simNotify/Notify.h"
#include "simVis/LayerRefreshCallback.h"

namespace simVis {

/** Custom osgEarth tag for a layer, to indicate it should trigger automatic refreshes */
static const std::string REFRESH_TAG = "refresh";

/** Update callback associated with current map, triggers updateInterval() */
class LayerRefreshCallback::MapUpdatedCallback : public osgEarth::MapCallback
{
public:
  explicit MapUpdatedCallback(LayerRefreshCallback& parent)
    : parent_(parent)
  {
  }

  /** Fire off updateInterval() on relevant changes */
  virtual void onLayerAdded(osgEarth::Layer* layer, unsigned index)
  {
    fireUpdateInterval_(layer);
  }

  virtual void onLayerRemoved(osgEarth::Layer* layer, unsigned index)
  {
    fireUpdateInterval_(layer);
  }

  /** Enabling or disabling a layer can change the interval */
  virtual void onLayerEnabled(osgEarth::Layer* layer)
  {
    fireUpdateInterval_(layer);
  }

  virtual void onLayerDisabled(osgEarth::Layer* layer)
  {
    fireUpdateInterval_(layer);
  }

private:
  /** Fires off an updateInterval() if the passed-in layer has the "refresh" property set */
  void fireUpdateInterval_(const osgEarth::Layer* layer)
  {
    if (hasRefreshProperty_(layer))
    {
      osg::ref_ptr<osgEarth::MapNode> mapNode;
      if (parent_.mapNode_.lock(mapNode) && mapNode->getMap())
        parent_.updateInterval(mapNode->getMap());
    }
  }

  /** Returns true if the layer has the "refresh" property set */
  bool hasRefreshProperty_(const osgEarth::Layer* layer) const
  {
    return layer != NULL && layer->getConfig().hasValue(REFRESH_TAG);
  }

  LayerRefreshCallback& parent_;
};

//--------------------------------------------------------------------------

LayerRefreshCallback::LayerRefreshCallback()
{
  mapUpdatedCallback_ = new MapUpdatedCallback(*this);
  // Needs a map to become enabled
  setEnabled(false);
}

LayerRefreshCallback::LayerRefreshCallback(const LayerRefreshCallback& rhs, const osg::CopyOp& copyop)
  : PeriodicUpdateCallback(rhs, copyop),
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

  // Add the map callback to the new node
  mapNode_ = mapNode;
  if (mapNode && mapNode->getMap())
    mapNode->getMap()->addMapCallback(mapUpdatedCallback_.get());

  // Presume that map is up to date
  resetTimer();
  // Pull out the minimum refresh of current layers
  if (mapNode)
    updateInterval(mapNode->getMap());
  else
    setInterval(0);
}

void LayerRefreshCallback::updateInterval(osgEarth::Map* map)
{
  if (map)
  {
    setInterval(60.0 * getRefreshInMinutes_(*map));
    setEnabled(true);
  }
  else
  {
    setEnabled(false);
  }
}

void LayerRefreshCallback::runPeriodicEvent(osg::Object* object, osg::Object* data)
{
  // Print debug text to the log.  This shouldn't run often, and users might need to figure out why map refreshes
  SIM_DEBUG_FP << "LayerRefreshCallback::runPeriodicEvent() attempting to refresh map due to refreshing layer.\n";

  // Pull out the terrain engine
  osg::ref_ptr<osgEarth::MapNode> mapNode;
  if (!mapNode_.lock(mapNode) || !mapNode->getTerrainEngine() || !mapNode->getMap())
    return;

  // Get all terrain layers; they are the ones with extents
  std::vector<osg::ref_ptr<osgEarth::TerrainLayer> > allLayers;
  mapNode->getMap()->getLayers(allLayers);

  bool refreshedOne = false;
  osgEarth::TerrainEngineNode* terrainEngine = mapNode->getTerrainEngine();
  // Loop through all layers
  for (const auto& layer : allLayers)
  {
    // Must be enabled and visible, else we skip this round of updates
    if (!layer.valid() || !layer->getVisible() || !layer->getEnabled())
      continue;

    // We cannot micromanage the refresh timer, so refresh all that have the tag
    auto cfg = layer->getConfig();
    if (!cfg.hasValue(REFRESH_TAG))
      continue;
    const auto extents = layer->getDataExtents();
    for (const auto& extent : extents)
    {
      terrainEngine->invalidateRegion(extent);
      refreshedOne = true;
    }
  }

  // Redoing the terrain can be expensive
  if (refreshedOne)
    terrainEngine->dirtyTerrain();
}

int LayerRefreshCallback::getRefreshInMinutes_(const osgEarth::Map& map) const
{
  int minRefresh = std::numeric_limits<int>::max();
  bool hasRefresh = false;
  std::vector<osg::ref_ptr<osgEarth::TerrainLayer> > layers;
  map.getLayers(layers);
  for (const auto& layer : layers)
  {
    if (layer == NULL)
      continue;
    const auto& cfg = layer->getConfig();
    int refreshValue = 0;
    if (cfg.get(REFRESH_TAG, refreshValue))
    {
      minRefresh = osg::maximum(1, osg::minimum(refreshValue, minRefresh));
      hasRefresh = true;
    }
  }

  // Return 0 if no layer has a refresh
  return hasRefresh ? minRefresh : 0;
}

}
