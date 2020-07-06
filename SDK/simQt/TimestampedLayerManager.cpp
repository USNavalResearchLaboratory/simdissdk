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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include <cassert>
#include "osg/ValueObject"
#include "osgEarth/Map"
#include "osgEarth/ImageLayer"
#include "simCore/Time/Clock.h"
#include "simQt/TimestampedLayerManager.h"

namespace simQt {

const std::string TimestampedLayerManager::DEFAULT_LAYER_TIME_GROUP = "DEFAULT_TIME_GROUP_KEY";
/// The xml tag used to identify the time group in the .earth file
static const std::string TIME_GROUP_TAG = "time_group";

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
  virtual void onLayerAdded(osgEarth::Layer *layer, unsigned int index)
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer == NULL)
      return;
    parent_.addLayerWithTime_(imageLayer);
    parent_.setTime_(parent_.currTime_);
  }

  virtual void onLayerRemoved(osgEarth::Layer *layer, unsigned int index)
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer == NULL)
      return;
    if (!parent_.layerIsTimed(imageLayer))
      return;

    std::string timeGroup = parent_.getLayerTimeGroup(imageLayer);
    parent_.originalVisibility_.erase(imageLayer);

    // If the layer is timed, it needs to be part of a group.  At the very least, we should get DEFAULT_LAYER_TIME_GROUP here
    assert(!timeGroup.empty());
    if (timeGroup.empty())
      return;

    std::map<std::string, TimeGroup*>& groups = parent_.groups_;
    auto groupIter = groups.find(timeGroup);

    // If there's at least one timed layer in this group, the group needs to exist
    assert(groupIter != groups.end());
    if (groupIter == groups.end())
      return;

    auto& layers = groupIter->second->layers;
    bool removedGroup = false;

    // Since layers are value of layers map, not key, need to iterate through the group's layers manually to find the removed layer
    for (auto layerIter = layers.begin(); layerIter != layers.end(); layerIter++)
    {
      if (layerIter->second == imageLayer)
      {
        layers.erase(layerIter);
        // If that was the last layer in its group, remove the group
        if (layers.empty())
        {
          delete groupIter->second;
          groups.erase(groupIter);
          removedGroup = true;
        }

        // Only one entry per layer
        break;
      }

    }

    // Don't need to recalculate current layers unless a current layer was removed and there are other layers left in its group
    if (removedGroup || groupIter->second->currentLayer != imageLayer)
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
    setName("Timestamped Layer CB");
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

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simQt"; }
  /** Return the class name */
  virtual const char* className() const { return "TimestampedLayerManager::MapChangeObserver"; }

protected:
  /* Descendants of osg::referenced need protected destructor */
  virtual ~MapChangeObserver() {}

private:
  TimestampedLayerManager& parent_;
  osg::observer_ptr<osgEarth::MapNode> map_;
};

/////////////////////////////////////////////////////////////////////////////////////////////

TimestampedLayerManager::TimeGroup::TimeGroup()
{
}

TimestampedLayerManager::TimeGroup::~TimeGroup()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

TimestampedLayerManager::TimestampedLayerManager(simCore::Clock& clock, osg::Group* attachPoint, QObject* parent)
  : QObject(parent),
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
    mapNode->getMap()->removeMapCallback(mapListener_.get());
  restoreOriginalVisibility_();
  for (auto groupIter = groups_.begin(); groupIter != groups_.end(); groupIter++)
    delete groupIter->second;
}

void TimestampedLayerManager::setTime_(const simCore::TimeStamp& stamp)
{
  // If inactive, keep track of time so that it's accurate on reactivate
  currTime_ = stamp;
  if (!timingActive_)
    return;

  osgEarth::ImageLayer* oldLayer = NULL;

  // Update the current layer for each group
  for (auto groupIter = groups_.begin(); groupIter != groups_.end(); groupIter++)
  {
    auto& layers = groupIter->second->layers;
    auto& currentLayer = groupIter->second->currentLayer;
    auto i = layers.upper_bound(currTime_);
    // If we have a layer to show
    if (i != layers.begin())
    {
      i--;
      if (currentLayer != i->second)
      {
        if (currentLayer.valid())
        {
          originalVisibility_[currentLayer.get()] = currentLayer->getVisible();
          currentLayer->setVisible(false);
          oldLayer = currentLayer.get();
        }

        currentLayer = i->second;
        emit currentTimedLayerChanged(currentLayer.get(), oldLayer);
        if (currentLayer.valid())
        {
          auto iter = originalVisibility_.find(currentLayer.get());
          if (iter != originalVisibility_.end())
            currentLayer->setVisible(iter->second);
        }
      }
    }
    // Current time is before first layer starts.  Show none of them.
    else
    {
      if (currentLayer.valid())
      {
        originalVisibility_[currentLayer.get()] = currentLayer->getVisible();
        currentLayer->setVisible(false);
        oldLayer = currentLayer.get();
      }
      currentLayer = NULL;
      emit currentTimedLayerChanged(NULL, oldLayer);
    }
  }
}

bool TimestampedLayerManager::layerIsTimed(const osgEarth::ImageLayer* layer) const
{
  // Any layer we're keeping track of is timed if timing is active
  return (timingActive_ && originalVisibility_.find(layer) != originalVisibility_.end());
}

const osgEarth::ImageLayer* TimestampedLayerManager::getCurrentTimedLayer(const std::string& timeGroup) const
{
  auto groupIter = groups_.find(timeGroup);
  if (groupIter != groups_.end())
    return groupIter->second->currentLayer.get();
  return NULL;
}

void TimestampedLayerManager::addLayerWithTime_(osgEarth::ImageLayer* newLayer)
{
  if (!newLayer)
    return;
  const simCore::TimeStamp& simTime = getLayerTime(newLayer);
  // If layer has no time, nothing to do with it
  if (simTime == simCore::INFINITE_TIME_STAMP)
    return;

  originalVisibility_[newLayer] = newLayer->getVisible();
  std::string groupName = getLayerTimeGroup(newLayer);
  // If the group doesn't exist yet, create it and put it in the map
  if (groups_.find(groupName) == groups_.end())
    groups_[groupName] = new TimeGroup();

  groups_[groupName]->layers[simTime] = newLayer;
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
    castObserver->getMapNode()->getMap()->removeMapCallback(mapListener_.get());

  // Attempt to restore visibility settings to current image layers before clearing them for the new map
  restoreOriginalVisibility_();

  for (auto groupIter = groups_.begin(); groupIter != groups_.end(); groupIter++)
    delete groupIter->second;
  groups_.clear();

  originalVisibility_.clear();
  if (mapNode && mapNode->getMap())
  {
    osgEarth::Map* map = mapNode->getMap();
    map->addMapCallback(mapListener_.get());

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
  // First iterate through all groups
  for (auto groupIter = groups_.begin(); groupIter != groups_.end(); groupIter++)
  {
    // Then iterate through the layers within the groups
    for (auto layerIter = groupIter->second->layers.begin(); layerIter != groupIter->second->layers.end(); layerIter++)
    {
      if (layerIter->second.valid() && layerIter->second != groupIter->second->currentLayer)
      {
        auto originVisIter = originalVisibility_.find(layerIter->second.get());
        // Don't restore original visibility to current layer, since user may have changed it since it became current
        if (originVisIter != originalVisibility_.end())
          layerIter->second->setVisible(originVisIter->second);
      }
    }

    // Unset the group's current layer.  Prevents bad starting state when timed visibility is reactiviated
    groupIter->second->currentLayer = NULL;
  }
}

void TimestampedLayerManager::useTimedVisibility_()
{
  // Set all layers invisible as a base, then let setTime handle which (if any) should be visible
  for (auto groupIter = groups_.begin(); groupIter != groups_.end(); groupIter++)
  {
    for (auto layerIter = groupIter->second->layers.begin(); layerIter != groupIter->second->layers.end(); layerIter++)
    {
      if (layerIter->second.valid())
      {
        originalVisibility_[layerIter->second.get()] = layerIter->second->getVisible();
        layerIter->second->setVisible(false);
      }
    }
  }

  setTime_(currTime_);
}

std::string TimestampedLayerManager::getLayerTimeGroup(const osgEarth::ImageLayer* layer) const
{
  if (!layer || getLayerTime(layer) == simCore::INFINITE_TIME_STAMP)
    return "";
  const osgEarth::Config& conf = layer->getConfig();
  std::string rv = conf.value(TIME_GROUP_TAG);
  if (rv.empty())
    rv = DEFAULT_LAYER_TIME_GROUP;
  return rv;
}

simCore::TimeStamp TimestampedLayerManager::getLayerTime(const osgEarth::ImageLayer* layer) const
{
  if (!layer)
    return simCore::INFINITE_TIME_STAMP;

  const osgEarth::Config& conf = layer->getConfig();
  std::string iso8601 = conf.value("time");
  // Fall back to "times" if possible
  if (iso8601.empty())
    iso8601 = conf.value("times");

  // Some image layer file types can have time values (e.g. db files).  Config values can't be
  // changed at time of file read, so time is set as a user value of the layer in these cases.
  // Config values take precedence over user values.
  if (iso8601.empty())
    layer->getUserValue("time", iso8601);
  // If layer has no time, nothing to do with it
  if (iso8601.empty())
    return simCore::INFINITE_TIME_STAMP;

  const osgEarth::DateTime osgTime(iso8601);
  return simCore::TimeStamp(1970, osgTime.asTimeStamp());
}

}
