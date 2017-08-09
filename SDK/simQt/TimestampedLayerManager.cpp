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

#include "osg/ValueObject"
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
    currTime_(clock_.currentTime()),
    timingActive_(true)
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
  restoreOriginalVisibility_();
}

void TimestampedLayerManager::setTime_(const simCore::TimeStamp& stamp)
{
  // If inactive, keep track of time so that it's accurate on reactivate
  currTime_ = stamp;
  if (!timingActive_)
    return;

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
  // Any layer we're keeping track of is timed if timing is active
  return (timingActive_ && originalVisibility_.find(layer) != originalVisibility_.end());
}

const osgEarth::ImageLayer* TimestampedLayerManager::currentTimedLayer() const
{
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
  /*
   * Some image layer file types can have time values (e.g. db files).  Config values can't be
   * changed at time of file read, so time is set as a user value of the tile source in these cases.
   * Config values take precedence of user values.
   */
  if (iso8601.empty())
    newLayer->getTileSource()->getUserValue("time", iso8601);
  // If layer has no time, nothing to do with it
  if (iso8601.empty())
    return;

  osgEarth::DateTime osgTime(iso8601);
  simCore::TimeStamp simTime = simCore::TimeStamp(1970, osgTime.asTimeStamp());
  layers_[simTime] = newLayer;
  originalVisibility_[newLayer] = newLayer->getVisible();
  if (timingActive_)
    newLayer->setVisible(false);
  setTime_(currTime_);
}

void TimestampedLayerManager::setMapNode(osgEarth::MapNode* mapNode)
{
  MapChangeObserver* castObserver = dynamic_cast<MapChangeObserver*>(mapChangeObserver_.get());
  if (castObserver && mapNode != castObserver->getMapNode())
    castObserver->setMapNode(mapNode);
}

void TimestampedLayerManager::setMapNode_(osgEarth::MapNode* mapNode)
{
  MapChangeObserver* castObserver = dynamic_cast<MapChangeObserver*>(mapChangeObserver_.get());
  if (castObserver && castObserver->getMapNode() && castObserver->getMapNode()->getMap())
    castObserver->getMapNode()->getMap()->removeMapCallback(mapListener_);

  // Attempt to restore visibility settings to current image layers before clearing them for the new map
  restoreOriginalVisibility_();

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

void TimestampedLayerManager::setTimingActive(bool active)
{
  if (active == timingActive_)
    return;
  timingActive_ = active;

  if (timingActive_)
    useTimedVisibility_();
  else
    restoreOriginalVisibility_();
}

bool TimestampedLayerManager::timingActive() const
{
  return timingActive_;
}

void TimestampedLayerManager::restoreOriginalVisibility_()
{
  for (auto iter = layers_.begin(); iter != layers_.end(); iter++)
  {
    if (iter->second.valid())
    {
      auto originVisIter = originalVisibility_.find(iter->second.get());
      // Don't restore original visibility to current layer, since user may have changed it since it became current
      if (originVisIter != originalVisibility_.end() && iter->second != currentLayer_)
        iter->second->setVisible(originVisIter->second);
    }
  }

  // No concept of a current layer if layers aren't being treated as timed
  currentLayer_ = NULL;
}

void TimestampedLayerManager::useTimedVisibility_()
{
  // Set all layers invisible as a base, then let setTime handle which (if any) should be visible
  for (auto iter = layers_.begin(); iter != layers_.end(); iter++)
  {
    if (iter->second.valid())
    {
      originalVisibility_[iter->second.get()] = iter->second->getVisible();
      iter->second->setVisible(false);
    }
  }

  setTime_(currTime_);
}

}
