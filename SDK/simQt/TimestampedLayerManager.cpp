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

#include "osgEarth/Map"
#include "osgEarth/ImageLayer"
#include "simCore/Time/Clock.h"
#include "simQt/TimestampedLayerManager.h"

namespace simQt {

/**
* Class for listening to the osgEarth::Map callbacks
*/
class TimestampedLayerManager::MapListener : public osgEarth::MapCallback
{
public:
  explicit MapListener(TimestampedLayerManager& parent)
    : parent_(parent)
  {
  }

  // Check for time values in config, add to watched layers if any are found
  virtual void onImageLayerAdded(osgEarth::ImageLayer *layer, unsigned int index)
  {
    parent_.addLayerWithTime_(layer);
    parent_.setTime_(parent_.currTime_);
  }

  virtual void onImageLayerRemoved(osgEarth::ImageLayer *layer, unsigned int index)
  {
    if (!parent_.layerIsTimed(layer))
      return;

    parent_.originalVisibility_.erase(layer);

    // Since layers are value of layers_ map, not key, need to iterate manually to find the removed layer
    for (auto iter = parent_.layers_.begin(); iter != parent_.layers_.end(); iter++)
    {
      if (iter->second == layer)
      {
        parent_.layers_.erase(iter);
        // Only one entry per layer
        break;
      }

    }

    // Current layer won't change unless layer being removed was current layer
    if (parent_.currentLayer_ != layer)
      return;
    // Reset current time to refresh current layer
    parent_.setTime_(parent_.currTime_);
  }

private:
  TimestampedLayerManager& parent_;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class TimestampedLayerManager::ClockListener : public simCore::Clock::TimeObserver
{
public:
  explicit ClockListener(TimestampedLayerManager& parent)
    : parent_(parent)
  {
  }

  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    parent_.setTime_(t);
  }

  virtual void onTimeLoop()
  {
    // No-op
  }

  virtual void adjustTime(const simCore::TimeStamp &oldTime, simCore::TimeStamp& newTime)
  {
    // No-op
  }

private:
  TimestampedLayerManager& parent_;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class TimestampedLayerManager::MapChangeObserver : public osgEarth::MapNodeObserver, public osg::Node
{
public:
  explicit MapChangeObserver(TimestampedLayerManager& parent)
    : parent_(parent)
  {
  }

  /* Reimplemented from MapNodeObserver */
  virtual void setMapNode(osgEarth::MapNode* mapNode)
  {
    if (map_ == mapNode)
      return;
    map_ = mapNode;
    parent_.setMapNode_(mapNode);
  }

  /* Reimplemented from MapNodeObserver */
  virtual osgEarth::MapNode* getMapNode()
  {
    return (map_.valid() ? map_.get() : NULL);
  }

protected:
  /* Descendants of osg::referenced need protected destructor */
  virtual ~MapChangeObserver() {}

private:
  TimestampedLayerManager& parent_;
  osg::observer_ptr<osgEarth::MapNode> map_;
};

/////////////////////////////////////////////////////////////////////////////////////////////

TimestampedLayerManager::TimestampedLayerManager(simCore::Clock& clock, osg::Group* attachPoint, QObject* parent)
  : QObject(parent),
    currentLayer_(NULL),
    clock_(clock),
    currTime_(clock_.currentTime())
{
  mapListener_ = new MapListener(*this);
  clockListener_.reset(new ClockListener(*this));
  clock_.registerTimeCallback(clockListener_);
  attachPoint_ = attachPoint;
  if (attachPoint_.valid())
  {
    mapChangeObserver_ = new MapChangeObserver(*this);
    attachPoint_->addChild(mapChangeObserver_);
  }
}

TimestampedLayerManager::~TimestampedLayerManager()
{
  if (attachPoint_.valid())
    attachPoint_->removeChild(mapChangeObserver_);
  clock_.removeTimeCallback(clockListener_);
  clockListener_.reset();
  osgEarth::MapNode* mapNode = dynamic_cast<MapChangeObserver*>(mapChangeObserver_.get())->getMapNode();
  if (mapNode && mapNode->getMap())
    mapNode->getMap()->removeMapCallback(mapListener_);
}

void TimestampedLayerManager::setTime_(const simCore::TimeStamp& stamp)
{
  currTime_ = stamp;
  osgEarth::ImageLayer* oldLayer = NULL;

  auto i = layers_.upper_bound(currTime_);
  if (i != layers_.begin())
  {
    i--;
    if (currentLayer_ != i->second)
    {
      if (currentLayer_.valid())
      {
        originalVisibility_[currentLayer_.get()] = currentLayer_->getVisible();
        currentLayer_->setVisible(false);
        oldLayer = currentLayer_.get();
      }

      currentLayer_ = i->second;
      emit currentTimedLayerChanged(currentLayer_.get(), oldLayer);
      if (currentLayer_.valid())
      {
        currentLayer_->setVisible(originalVisibility_[currentLayer_.get()]);
      }
    }
  }
  else
  {
    if (currentLayer_.valid())
    {
      currentLayer_->setVisible(false);
      oldLayer = currentLayer_.get();
    }
    currentLayer_ = NULL;
    emit currentTimedLayerChanged(NULL, oldLayer);
  }
}

bool TimestampedLayerManager::layerIsTimed(const osgEarth::ImageLayer* layer) const
{
  // If layer is in the originalVisibility_ map, it's timed
  return (originalVisibility_.find(layer) != originalVisibility_.end());
}

const osgEarth::ImageLayer* TimestampedLayerManager::currentTimedLayer() const
{
  // Any layer being tracked is always considered current
  return currentLayer_.get();
}

void TimestampedLayerManager::addLayerWithTime_(osgEarth::ImageLayer* newLayer)
{
  if (!newLayer)
    return;
  const osgEarth::Config& conf = newLayer->getConfig();
  std::string iso8601 = conf.value("time");
  // Fall back to "times" if possible
  if (iso8601.empty())
    iso8601 = conf.value("times");
  // If layer has no time, nothing to do with it
  if (iso8601.empty())
    return;

  osgEarth::DateTime osgTime(iso8601);
  simCore::TimeStamp simTime = simCore::TimeStamp(1970, osgTime.asTimeStamp());
  layers_[simTime] = newLayer;
  originalVisibility_[newLayer] = newLayer->getVisible();
  newLayer->setVisible(false);
}

void TimestampedLayerManager::setMapNode(osgEarth::MapNode* mapNode)
{
  MapChangeObserver* castObserver = dynamic_cast<MapChangeObserver*>(mapChangeObserver_.get());
  if (castObserver && mapNode != castObserver->getMapNode())
    castObserver->setMapNode(mapNode);
}

void TimestampedLayerManager::setMapNode_(osgEarth::MapNode* mapNode)
{
  osgEarth::Map* prevMap = NULL;
  MapChangeObserver* castObserver = dynamic_cast<MapChangeObserver*>(mapChangeObserver_.get());
  if (castObserver && castObserver->getMapNode() && castObserver->getMapNode()->getMap())
    prevMap = castObserver->getMapNode()->getMap();

  prevMap->removeMapCallback(mapListener_);

  // Attempt to restore visibility settings to current image layers before clearing them for the new map
  for (auto iter = layers_.begin(); iter != layers_.end(); iter++)
  {
    if (iter->second.valid())
    {
      auto originVisIter = originalVisibility_.find(iter->second.get());
      if (originVisIter != originalVisibility_.end())
        iter->second->setVisible(originVisIter->second);
    }
  }

  layers_.clear();
  originalVisibility_.clear();
  currentLayer_ = NULL;
  if (mapNode && mapNode->getMap())
  {
    osgEarth::Map* map = mapNode->getMap();
    map->addMapCallback(mapListener_);

    // Rebuild the layers_ map by going through all layers in the map to find all image layers with time
    osgEarth::LayerVector vec;
    map->getLayers(vec);
    for (auto i = vec.begin(); i != vec.end(); i++)
    {
      osgEarth::ImageLayer* newLayer = NULL;
      if (i->valid())
        newLayer = dynamic_cast<osgEarth::ImageLayer*>(i->get());
      addLayerWithTime_(newLayer);
    }
  }

  setTime_(currTime_);
}

}
