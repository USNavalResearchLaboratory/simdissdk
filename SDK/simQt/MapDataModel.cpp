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
#include <algorithm>
#include <cassert>
#include <limits>
#include <QIcon>
#include "osgEarth/Map"
#include "simCore/Calc/Math.h"

#ifdef HAVE_SIMUTIL
#include "simUtil/VelocityParticleLayer.h"
#endif

#include "simQt/MapDataModel.h"

namespace simQt {

//----------------------------------------------------------------------------
namespace {

/** Given container type V and container item T, returns index of T item in V vec; returns MapReindexer::INVALID_INDEX on not-found */
template <typename V, typename T>
unsigned int indexOf(const V& vec, const T& item)
{
  const auto iter = std::find(vec.begin(), vec.end(), item);
  if (iter == vec.end())
    return MapReindexer::INVALID_INDEX;
  return std::distance(vec.begin(), iter);
}

/** Returns true if the given status is "OK", or false if not OK, used for coloration of nodes. */
bool isStatusOk(const osgEarth::Status& status)
{
  if (status.isOK())
    return true;
  // Might be false, but for our purposes we don't show an error when the layer is closed, since that's an intentional setting
  return (status.code() == osgEarth::Status::ResourceUnavailable && status.message() == "Layer closed");
}

} // anon namespace

//----------------------------------------------------------------------------
const unsigned int MapReindexer::INVALID_INDEX = std::numeric_limits<unsigned int>::max();

MapReindexer::MapReindexer(osgEarth::Map* map)
  : map_(map)
{
}

MapReindexer::~MapReindexer()
{
}

void MapReindexer::getLayers(osgEarth::Map* map, osgEarth::ImageLayerVector& imageLayers)
{
  if (map != nullptr)
  {
    map->getLayers(imageLayers);
#ifdef HAVE_SIMUTIL
    // Remove Velocity Particle layers
    imageLayers.erase(std::remove_if(imageLayers.begin(), imageLayers.end(), [](const osg::ref_ptr<osgEarth::ImageLayer>& imgLayer) {
      return dynamic_cast<const simUtil::VelocityParticleLayer*>(imgLayer.get()) != nullptr;
    }), imageLayers.end());
#endif
  }
}

void MapReindexer::getLayers(osgEarth::Map* map, osgEarth::ElevationLayerVector& elevationLayers)
{
  if (map != nullptr)
    map->getLayers(elevationLayers);
}

void MapReindexer::getLayers(osgEarth::Map* map, FeatureModelLayerVector& modelLayers)
{
  if (map != nullptr)
    map->getLayers(modelLayers);
}

void MapReindexer::getLayers(osgEarth::Map* map, VelocityParticleLayerVector& velocityLayers)
{
#ifdef HAVE_SIMUTIL
  if (map != nullptr)
    map->getLayers(velocityLayers);
#endif
}

void MapReindexer::getOtherLayers(osgEarth::Map* map, osgEarth::VisibleLayerVector& otherLayers)
{
  if (map == nullptr)
    return;
  osgEarth::VisibleLayerVector allLayers;
  map->getLayers(allLayers);
  // pass along only layers that are not image, elevation, or model
  for (auto iter = allLayers.begin(); iter != allLayers.end(); ++iter)
  {
    const osgEarth::VisibleLayer* layer = (*iter).get();
    if (dynamic_cast<const osgEarth::ImageLayer*>(layer) != nullptr)
      continue;
    if (dynamic_cast<const osgEarth::ElevationLayer*>(layer) != nullptr)
      continue;
    if (dynamic_cast<const osgEarth::FeatureModelLayer*>(layer) != nullptr)
      continue;
    // No need to explicitly test for simUtil::VelocityParticleLayer; it's an ImageLayer
    otherLayers.push_back(*iter);
  }
}

unsigned int MapReindexer::layerTypeIndex(osgEarth::ImageLayer* layer) const
{
  // Must have a valid map
  assert(map_.valid());
  if (!map_.valid())
    return INVALID_INDEX;
  osgEarth::ImageLayerVector layers;
  MapReindexer::getLayers(map_.get(), layers);
  return indexOf(layers, layer);
}

unsigned int MapReindexer::layerTypeIndex(osgEarth::ElevationLayer* layer) const
{
  // Must have a valid map
  assert(map_.valid());
  if (!map_.valid())
    return INVALID_INDEX;
  osgEarth::ElevationLayerVector layers;
  MapReindexer::getLayers(map_.get(), layers);
  return indexOf(layers, layer);
}

unsigned int MapReindexer::layerTypeIndex(osgEarth::FeatureModelLayer* layer) const
{
  // Must have a valid map
  assert(map_.valid());
  if (!map_.valid())
    return INVALID_INDEX;
  FeatureModelLayerVector layers;
  MapReindexer::getLayers(map_.get(), layers);
  return indexOf(layers, layer);
}

unsigned int MapReindexer::layerTypeIndex(simUtil::VelocityParticleLayer* layer) const
{
#ifndef HAVE_SIMUTIL
  return INVALID_INDEX;
#else
  // Must have a valid map
  assert(map_.valid());
  if (!map_.valid())
    return INVALID_INDEX;
  VelocityParticleLayerVector layers;
  MapReindexer::getLayers(map_.get(), layers);
  return indexOf(layers, layer);
#endif
}

unsigned int MapReindexer::otherLayerTypeIndex(osgEarth::VisibleLayer* layer) const
{
  // Must have a valid map
  assert(map_.valid());
  if (!map_.valid())
    return INVALID_INDEX;
  osgEarth::VisibleLayerVector layers;
  MapReindexer::getOtherLayers(map_.get(), layers);
  return indexOf(layers, layer);
}

//----------------------------------------------------------------------------
/// Base class for the different item types
class MapDataModel::Item
{
public:
  virtual ~Item() {}

  /// return the name shown in the list
  virtual QString name() const = 0;

  /// Return the text color of the entry depending on status of item
  virtual QVariant color() const = 0;

  /// return a MapChildren value appropriate for this item
  virtual MapChildren layerTypeRole() const = 0;

  /// return the layer pointer (where applicable)
  virtual QVariant layerPtr() const = 0;

  /// return the flags for this
  virtual Qt::ItemFlags flags() const = 0;

  /// return the parent item
  virtual Item* parent() const = 0;

  /// return the number of children
  virtual int rowCount() const = 0;

  /// return the child which is 'row' positions under this
  virtual Item* childAt(int row) = 0;

  /// return the row for 'c'
  virtual int rowOfChild(Item *c) const = 0;

  /// add 'c' as a child
  virtual void insertChild(Item *c, int position) = 0;

  /// remove 'c' from children; does not delete 'c'
  virtual void removeChild(Item *c) = 0;
};

//----------------------------------------------------------------------------
/// visible top of the hierarchy
class MapItem : public MapDataModel::Item
{
public:
  /** Constructor */
  explicit MapItem(Item *parent);

  virtual ~MapItem()
  {
    qDeleteAll(children_);
  }

  /** @copydoc  MapDataModel::Item::name */
  virtual QString name() const override
  {
    return QObject::tr("Map");
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::layerTypeRole */
  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_NONE;
  }

  /** @copydoc  MapDataModel::Item::layerPtr */
  virtual QVariant layerPtr() const override
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual Qt::ItemFlags flags() const override
  {
    return Qt::ItemIsEnabled;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual Item* parent() const override
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const override
  {
    return children_.count();
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row) override
  {
    return children_.value(row);
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const override
  {
    return children_.indexOf(c);
  }

  /** @copydoc  MapDataModel::Item::insertChild */
  virtual void insertChild(Item *c, int position) override
  {
    children_.insert(position, c);
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c) override
  {
    assert(false); // should not remove top level groups
  }

private:
  Item *parent_;
  QList<Item*> children_; ///< all the items under this item
};

//----------------------------------------------------------------------------
/**
 * Second level of the hierarchy - container for layers
 * (should be one of: Image, Elevation, or Model)
 */
class GroupItem : public MapDataModel::Item
{
public:
  /** Constructor */
  GroupItem(const QString &name, MapDataModel::Item *parent)
  : name_(name),
    parent_(parent)
  {
  }

  virtual ~GroupItem()
  {
    qDeleteAll(children_);
  }

  /** @copydoc  MapDataModel::Item::name */
  virtual QString name() const override
  {
    return name_;
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::layerTypeRole */
  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_NONE;
  }

  /** @copydoc  MapDataModel::Item::layerPtr */
  virtual QVariant layerPtr() const override
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual Qt::ItemFlags flags() const override
  {
    // not selectable
    return Qt::ItemIsEnabled;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual MapDataModel::Item* parent() const override
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const override
  {
    return children_.count();
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row) override
  {
    return children_.value(row);
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const override
  {
    return children_.indexOf(c);
  }

  /** @copydoc  MapDataModel::Item::insertChild */
  virtual void insertChild(Item *c, int position) override
  {
    children_.insert(position, c);
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c) override
  {
    children_.removeOne(c);
  }

  /** Searches children for one that has the layer provided, returning a row or nullptr on failure */
  MapDataModel::Item* itemByLayer(const osgEarth::Layer* layer) const
  {
    for (auto it = children_.begin(); it != children_.end(); ++it)
    {
      if ((*it)->layerPtr().value<void*>() == layer)
        return (*it);
    }
    return nullptr;
  }

private:
  QString name_;
  MapDataModel::Item *parent_;
  QList<MapDataModel::Item*> children_; ///< all the items under this item
};

MapItem::MapItem(MapDataModel::Item *parent)
: parent_(parent)
{
  insertChild(new GroupItem(QObject::tr("Image"), this), MapDataModel::CHILD_IMAGE);
  insertChild(new GroupItem(QObject::tr("Elevation"), this), MapDataModel::CHILD_ELEVATION);
  insertChild(new GroupItem(QObject::tr("Model"), this), MapDataModel::CHILD_FEATURE);
  insertChild(new GroupItem(QObject::tr("Velocity"), this), MapDataModel::CHILD_VELOCITY);
  insertChild(new GroupItem(QObject::tr("Other"), this), MapDataModel::CHILD_OTHER);
}

//----------------------------------------------------------------------------
/// Less abstract class (base class for leaf nodes)
class LayerItem : public MapDataModel::Item
{
public:
  /** Constructor */
  explicit LayerItem(Item *parent)
  : parent_(parent)
  {
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual Qt::ItemFlags flags() const override
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual MapDataModel::Item* parent() const override
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const override
  {
    // no children
    return 0;
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row) override
  {
    return nullptr;
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const override
  {
    return -1;
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual void insertChild(Item *c, int position) override
  {
    assert(false); // no children
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c) override
  {
    assert(false); // no children
  }

private:
  Item *parent_;
};

/// An Image layer
class ImageLayerItem : public LayerItem
{
public:
  /** Constructor */
  ImageLayerItem(Item *parent, osgEarth::ImageLayer *layer)
  : LayerItem(parent),
    layer_(layer)
  {
  }

  virtual QString name() const override
  {
    if (layer_.valid())
      return QString::fromStdString(layer_->getName());
    return "";
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    if (layer_.valid() && isStatusOk(layer_->getStatus()))
      return QVariant();
    return QColor(Qt::gray);
  }

  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_IMAGE;
  }

  virtual QVariant layerPtr() const override
  {
    return QVariant::fromValue<void*>(layer_.get());
  }

private:
  osg::observer_ptr<osgEarth::ImageLayer> layer_;
};

/// An Elevation layer
class ElevationLayerItem : public LayerItem
{
public:
  /** Constructor */
  ElevationLayerItem(Item *parent, osgEarth::ElevationLayer *layer)
  : LayerItem(parent),
    layer_(layer)
  {
  }

  virtual QString name() const override
  {
    if (layer_.valid())
      return QString::fromStdString(layer_->getName());
    return "";
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    if (layer_.valid() && isStatusOk(layer_->getStatus()))
      return QVariant();
    return QColor(Qt::gray);
  }

  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_ELEVATION;
  }

  virtual QVariant layerPtr() const override
  {
    return QVariant::fromValue<void*>(layer_.get());
  }

private:
  osg::observer_ptr<osgEarth::ElevationLayer> layer_;
};

/// A Model layer
class FeatureModelLayerItem : public LayerItem
{
public:
  /** Constructor */
  FeatureModelLayerItem(Item *parent, osgEarth::FeatureModelLayer *layer)
  : LayerItem(parent),
    layer_(layer)
  {
  }

  virtual QString name() const override
  {
    if (layer_.valid())
      return QString::fromStdString(layer_->getName());
    return "";
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    if (layer_.valid() && isStatusOk(layer_->getStatus()))
      return QVariant();
    return QColor(Qt::gray);
  }

  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_FEATURE;
  }

  virtual QVariant layerPtr() const override
  {
    return QVariant::fromValue<void*>(layer_.get());
  }

private:
  osg::observer_ptr<osgEarth::FeatureModelLayer> layer_;
};

#if HAVE_SIMUTIL
/// A Velocity Particle layer
class VelocityParticleLayerItem : public LayerItem
{
public:
  /** Constructor */
  VelocityParticleLayerItem(Item* parent, simUtil::VelocityParticleLayer* layer)
    : LayerItem(parent),
    layer_(layer)
  {
  }

  virtual QString name() const override
  {
    return QString::fromStdString(layer_->getName());
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    if (!layer_.valid() || !isStatusOk(layer_->getStatus()))
      return QColor(Qt::gray);
    return QVariant();
  }

  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_VELOCITY;
  }

  virtual QVariant layerPtr() const override
  {
    return QVariant::fromValue<void*>(layer_.get());
  }

private:
  osg::observer_ptr<simUtil::VelocityParticleLayer> layer_;
};
#endif

/// Other layer
class OtherLayerItem : public LayerItem
{
public:
  /** Constructor */
  OtherLayerItem(Item *parent, osgEarth::VisibleLayer *layer)
    : LayerItem(parent),
    layer_(layer)
  {
  }

  virtual QString name() const override
  {
    if (layer_.valid())
      return QString::fromStdString(layer_->getName());
    return "";
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const override
  {
    if (layer_.valid() && isStatusOk(layer_->getStatus()))
      return QVariant();
    return QColor(Qt::gray);
  }

  virtual MapDataModel::MapChildren layerTypeRole() const override
  {
    return MapDataModel::CHILD_OTHER;
  }

  virtual QVariant layerPtr() const override
  {
    return QVariant::fromValue<void*>(layer_.get());
  }

private:
  osg::observer_ptr<osgEarth::VisibleLayer> layer_;
};

//----------------------------------------------------------------------------
/**
* Class for listening to the osgEarth::Map callbacks
*/
class MapDataModel::MapListener : public osgEarth::MapCallback
{
public:
  /** Constructor */
  explicit MapListener(MapDataModel &parent)
  : dataModel_(parent)
  {
  }

  /* Layer Added */
  virtual void onLayerAdded(osgEarth::Layer* layer, unsigned int index) override
  {
#ifdef HAVE_SIMUTIL
    // Need to test Velocity Layer first since it is-a ImageLayer
    simUtil::VelocityParticleLayer* velocityLayer = dynamic_cast<simUtil::VelocityParticleLayer*>(layer);
    if (velocityLayer)
    {
      MapReindexer reindex(dataModel_.map());
      unsigned int newIndex = reindex.layerTypeIndex(velocityLayer);
      // Means we got a layer that wasn't found in the vector. osgEarth or MapReindexer error.
      assert(newIndex != MapReindexer::INVALID_INDEX);
      if (newIndex != MapReindexer::INVALID_INDEX)
        dataModel_.addVelocityLayer_(velocityLayer, newIndex);
      return;
    }
#endif

    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
    {
      MapReindexer reindex(dataModel_.map());
      unsigned int newIndex = reindex.layerTypeIndex(imageLayer);
      // Means we got a layer that wasn't found in the vector. osgEarth or MapReindexer error.
      assert(newIndex != MapReindexer::INVALID_INDEX);
      if (newIndex != MapReindexer::INVALID_INDEX)
        dataModel_.addImageLayer_(imageLayer, newIndex);
      return;
    }

    osgEarth::ElevationLayer* elevationLayer = dynamic_cast<osgEarth::ElevationLayer*>(layer);
    if (elevationLayer)
    {
      MapReindexer reindex(dataModel_.map());
      unsigned int newIndex = reindex.layerTypeIndex(elevationLayer);
      // Means we got a layer that wasn't found in the vector. osgEarth or MapReindexer error.
      assert(newIndex != MapReindexer::INVALID_INDEX);
      if (newIndex != MapReindexer::INVALID_INDEX)
        dataModel_.addElevationLayer_(elevationLayer, newIndex);
      return;
    }

    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      MapReindexer reindex(dataModel_.map());
      unsigned int newIndex = reindex.layerTypeIndex(modelLayer);
      // Means we got a layer that wasn't found in the vector. osgEarth or MapReindexer error.
      assert(newIndex != MapReindexer::INVALID_INDEX);
      if (newIndex != MapReindexer::INVALID_INDEX)
        dataModel_.addFeatureLayer_(modelLayer, newIndex);
      return;
    }

    osgEarth::VisibleLayer* otherLayer = dynamic_cast<osgEarth::VisibleLayer*>(layer);
    if (otherLayer)
    {
      MapReindexer reindex(dataModel_.map());
      unsigned int newIndex = reindex.otherLayerTypeIndex(otherLayer);
      // Means we got a layer that wasn't found in the vector. osgEarth or MapReindexer error.
      assert(newIndex != MapReindexer::INVALID_INDEX);
      if (newIndex != MapReindexer::INVALID_INDEX)
        dataModel_.addOtherLayer_(otherLayer, newIndex);
      return;
    }
  }

  virtual void onLayerMoved(osgEarth::Layer* layer, unsigned int oldIndex, unsigned int newIndex) override
  {
#ifdef HAVE_SIMUTIL
    // Need to test Velocity Particle Layer first since it is-a ImageLayer
    simUtil::VelocityParticleLayer* velocityLayer = dynamic_cast<simUtil::VelocityParticleLayer*>(layer);
    if (velocityLayer)
    {
      moveLayer_(dataModel_.velocityGroup_(), layer, newIndex < oldIndex);
      return;
    }
#endif

    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
    {
      moveLayer_(dataModel_.imageGroup_(), layer, newIndex < oldIndex);
      return;
    }

    osgEarth::ElevationLayer* elevationLayer = dynamic_cast<osgEarth::ElevationLayer*>(layer);
    if (elevationLayer)
    {
      moveLayer_(dataModel_.elevationGroup_(), layer, newIndex < oldIndex);
      return;
    }

    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      moveLayer_(dataModel_.featureGroup_(), layer, newIndex < oldIndex);
      return;
    }

    osgEarth::VisibleLayer* otherLayer = dynamic_cast<osgEarth::VisibleLayer*>(layer);
    if (otherLayer)
    {
      moveLayer_(dataModel_.otherGroup_(), layer, newIndex < oldIndex);
      return;
    }
  }

  /** Layer Removed */
  virtual void onLayerRemoved(osgEarth::Layer* layer, unsigned int index) override
  {
#ifdef HAVE_SIMUTIL
    // Need to test Velocity Particle Layer first since it is-a ImageLayer
    simUtil::VelocityParticleLayer* velocityLayer = dynamic_cast<simUtil::VelocityParticleLayer*>(layer);
    if (velocityLayer)
    {
#if OSGEARTH_SOVERSION >= 152
      dataModel_.visibilityCallbacks_.erase(velocityLayer);
      dataModel_.opacityCallbacks_.erase(velocityLayer);
#else
      // We use image layer callbacks because it is an image layer
      dataModel_.imageCallbacks_.remove(velocityLayer);
#endif
      removeLayer_(dataModel_.velocityGroup_(), velocityLayer);
      return;
    }
#endif

    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
    {
#if OSGEARTH_SOVERSION >= 152
      dataModel_.visibilityCallbacks_.erase(imageLayer);
      dataModel_.opacityCallbacks_.erase(imageLayer);
#else
      dataModel_.imageCallbacks_.remove(imageLayer);
#endif
      removeLayer_(dataModel_.imageGroup_(), imageLayer);
      return;
    }

    osgEarth::ElevationLayer* elevationLayer = dynamic_cast<osgEarth::ElevationLayer*>(layer);
    if (elevationLayer)
    {
#if OSGEARTH_SOVERSION >= 152
      dataModel_.visibilityCallbacks_.erase(elevationLayer);
      dataModel_.opacityCallbacks_.erase(elevationLayer);
#else
      dataModel_.elevationCallbacks_.remove(elevationLayer);
#endif
      removeLayer_(dataModel_.elevationGroup_(), elevationLayer);
      return;
    }

    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
#if OSGEARTH_SOVERSION >= 152
      dataModel_.visibilityCallbacks_.erase(velocityLayer);
      dataModel_.opacityCallbacks_.erase(modelLayer);
#else
      dataModel_.featureCallbacks_.remove(modelLayer);
#endif
      removeLayer_(dataModel_.featureGroup_(), modelLayer);
      return;
    }

    osgEarth::VisibleLayer* visibleLayer = dynamic_cast<osgEarth::VisibleLayer*>(layer);
    if (visibleLayer)
    {
#if OSGEARTH_SOVERSION >= 152
      dataModel_.visibilityCallbacks_.erase(visibleLayer);
      dataModel_.opacityCallbacks_.erase(visibleLayer);
#else
      dataModel_.otherCallbacks_.remove(visibleLayer);
#endif
      removeLayer_(dataModel_.otherGroup_(), visibleLayer);
      return;
    }
  }

private:
  /** try to remove the given layer from the given group */
  void removeLayer_(Item *group, osgEarth::Layer *layer)
  {
    const Item *const groupParent = group->parent();
    // layer parent is a group, and the group parent is the root item
    assert(groupParent);

    const int rowOfParent = groupParent->rowOfChild(group);

    for (int row = 0; row < group->rowCount(); ++row)
    {
      Item *child = group->childAt(row);
      if (child->layerPtr().value<void*>() == layer)
      {
        QModelIndex parentIndex = dataModel_.createIndex(rowOfParent, 0, group);
        dataModel_.beginRemoveRows(parentIndex, row, row);
        group->removeChild(child);
        dataModel_.endRemoveRows();
        delete child;
        break;
      }
    }
  }

  /** process a layer moving up or down */
  void moveLayer_(Item *group, const osgEarth::Layer *layer, bool up)
  {
    Item *groupParent = group->parent();
    assert(groupParent); // layer parent is a group, and groups have parents
    const int rowOfParent = groupParent->rowOfChild(group);
    QModelIndex parentIndex = dataModel_.createIndex(rowOfParent, 0, group);

    for (int row = 0; row < group->rowCount(); ++row)
    {
      Item *child = group->childAt(row);
      if (child->layerPtr().value<void*>() == layer)
      {
        // model index is as if the item has not been removed (so is one greater when moving down)
        const int newRowIndex = up ? simCore::sdkMax(0, row - 1) :
            simCore::sdkMin(group->rowCount(), row + 2);

        if (dataModel_.beginMoveRows(parentIndex, row, row, parentIndex, newRowIndex))
        {
          group->removeChild(child);

          // tree index is after the item has been removed
          const int insertionPoint = up ? simCore::sdkMax(0, row - 1) :
              simCore::sdkMin(group->rowCount(), row + 1);

          group->insertChild(child, insertionPoint);
          dataModel_.endMoveRows();
        }
        break;
      }
    }
  }

  MapDataModel &dataModel_;
};

#if OSGEARTH_SOVERSION < 152
/** Watch for image layer changes */
class MapDataModel::ImageLayerListener : public osgEarth::TileLayerCallback
{
public:
  /** Constructor */
  explicit ImageLayerListener(MapDataModel& parent)
  : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer) override
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
#ifdef HAVE_SIMUTIL
    simUtil::VelocityParticleLayer* velocityLayer = dynamic_cast<simUtil::VelocityParticleLayer*>(imageLayer);
    if (velocityLayer)
      Q_EMIT dataModel_.velocityLayerVisibleChanged(velocityLayer);
    else
#endif
      if (imageLayer)
        Q_EMIT dataModel_.imageLayerVisibleChanged(imageLayer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer) override
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
#ifdef HAVE_SIMUTIL
    simUtil::VelocityParticleLayer* velocityLayer = dynamic_cast<simUtil::VelocityParticleLayer*>(imageLayer);
    if (velocityLayer)
      Q_EMIT dataModel_.velocityLayerOpacityChanged(velocityLayer);
    else
#endif
      if (imageLayer)
        Q_EMIT dataModel_.imageLayerOpacityChanged(imageLayer);
  }

private:
  MapDataModel& dataModel_;
};

/** Watch for elevation layer changes */
class MapDataModel::ElevationLayerListener : public osgEarth::TileLayerCallback
{
public:
  /** Constructor */
  explicit ElevationLayerListener(MapDataModel& parent)
  : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer) override
  {
    Q_EMIT dataModel_.elevationLayerVisibleChanged(static_cast<osgEarth::ElevationLayer*>(layer));
  }

private:
  MapDataModel& dataModel_;
};

/** Watch for model layer changes */
class MapDataModel::FeatureModelLayerListener : public osgEarth::VisibleLayerCallback
{
public:
  /** Constructor */
  explicit FeatureModelLayerListener(MapDataModel& parent)
  : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer) override
  {
    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      Q_EMIT dataModel_.featureLayerVisibleChanged(modelLayer);
    }
  }

  /** Inherited from ModelLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer) override
  {
    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      Q_EMIT dataModel_.featureLayerOpacityChanged(modelLayer);
    }
  }

private:
  MapDataModel& dataModel_;
};

/** Watch for other layer changes */
class MapDataModel::OtherLayerListener : public osgEarth::VisibleLayerCallback
{
public:
  /** Constructor */
  explicit OtherLayerListener(MapDataModel& parent)
    : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer) override
  {
    Q_EMIT dataModel_.otherLayerVisibleChanged(layer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer) override
  {
    Q_EMIT dataModel_.otherLayerOpacityChanged(layer);
  }

private:
  MapDataModel& dataModel_;
};
#endif

//----------------------------------------------------------------------------
MapDataModel::MapDataModel(QObject* parent)
  : QAbstractItemModel(parent),
    rootItem_(new MapItem(nullptr)),
    imageIcon_(":/simQt/images/Globe.png"),
    elevationIcon_(":/simQt/images/Image.png"),
    featureIcon_(":/simQt/images/Building Corporation.png"),
    velocityIcon_(":/simQt/images/WindLayer.png")
{
  mapListener_ = new MapListener(*this);
}

MapDataModel::~MapDataModel()
{
  removeAllCallbacks_(map());
  delete rootItem_;
}

osgEarth::Map* MapDataModel::map() const
{
  return map_.get();
}

void MapDataModel::bindTo(osgEarth::Map* map)
{
  // Refuse to do any work if binding to the same map (performance optimization)
  if (map == map_.get())
    return;

  // Remove the old callbacks
  removeAllCallbacks_(map_.get());

  // Swap out the internal state
  beginResetModel();
  removeAllItems_(imageGroup_());
  removeAllItems_(elevationGroup_());
  removeAllItems_(featureGroup_());
  removeAllItems_(velocityGroup_());
  removeAllItems_(otherGroup_());
  map_ = map;
  fillModel_(map);
  endResetModel();

  // Add the callback back in
  if (map_.valid())
    map_->addMapCallback(mapListener_.get());
}

void MapDataModel::removeAllCallbacks_(osgEarth::Map* map)
{
#if OSGEARTH_SOVERSION >= 152
  if (!map)
  {
    // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
    assert(visibilityCallbacks_.empty() && opacityCallbacks_.empty());
    return;
  }

  osgEarth::VisibleLayerVector visibleLayers;
  map->getLayers(visibleLayers);

  // Remove all visibility and opacity callbacks
  for (const auto& visibleLayerPtr : visibleLayers)
  {
    auto vIter = visibilityCallbacks_.find(visibleLayerPtr.get());
    if (vIter != visibilityCallbacks_.end())
      visibleLayerPtr->onVisibleChanged.remove(vIter->second);

    auto oIter = opacityCallbacks_.find(visibleLayerPtr.get());
    if (oIter != opacityCallbacks_.end())
      visibleLayerPtr->onOpacityChanged.remove(oIter->second);
  }
  visibilityCallbacks_.clear();
  opacityCallbacks_.clear();

#else

  if (!map)
  {
    // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
    assert(imageCallbacks_.empty() && elevationCallbacks_.empty() && featureCallbacks_.empty());
    return;
  }

  // need to remove all image callbacks
  osgEarth::ImageLayerVector imageLayers;
  MapReindexer::getLayers(map, imageLayers);
  for (osgEarth::ImageLayerVector::const_iterator iter = imageLayers.begin(); iter != imageLayers.end(); ++iter)
  {
    osgEarth::ImageLayer* imageLayer = iter->get();
    if (imageCallbacks_.contains(imageLayer))
    {
      osgEarth::TileLayer* tileLayer = iter->get();
      tileLayer->removeCallback(imageCallbacks_.find(iter->get())->get());
    }
  }

#ifdef HAVE_SIMUTIL
  // Need to also remove velocity layer callbacks from the image callbacks
  VelocityParticleLayerVector velocityLayers;
  MapReindexer::getLayers(map, velocityLayers);
  for (VelocityParticleLayerVector::const_iterator iter = velocityLayers.begin(); iter != velocityLayers.end(); ++iter)
  {
    if (imageCallbacks_.contains(iter->get()))
    {
      osgEarth::TileLayer* tileLayer = iter->get();
      tileLayer->removeCallback(imageCallbacks_.find(iter->get())->get());
    }
  }
  // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
  assert(imageCallbacks_.size() == static_cast<int>(imageLayers.size() + velocityLayers.size()));
#endif

  imageCallbacks_.clear();

  // need to remove all elevation callbacks
  osgEarth::ElevationLayerVector elevationLayers;
  MapReindexer::getLayers(map, elevationLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = elevationLayers.begin(); iter != elevationLayers.end(); ++iter)
  {
    if (elevationCallbacks_.contains(iter->get()))
    {
      osgEarth::TileLayer* tileLayer = iter->get();
      tileLayer->removeCallback(elevationCallbacks_.find(iter->get())->get());
    }
  }
  // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
  assert(elevationCallbacks_.size() == static_cast<int>(elevationLayers.size()));
  elevationCallbacks_.clear();

  // need to remove all model callbacks
  FeatureModelLayerVector modelLayers;
  MapReindexer::getLayers(map, modelLayers);
  for (FeatureModelLayerVector::const_iterator iter = modelLayers.begin(); iter != modelLayers.end(); ++iter)
  {
    if (featureCallbacks_.contains(iter->get()))
      iter->get()->removeCallback(featureCallbacks_.find(iter->get())->get());
  }
  // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
  assert(featureCallbacks_.size() == static_cast<int>(modelLayers.size()));
  featureCallbacks_.clear();

  // need to remove all callbacks for other layers
  osgEarth::VisibleLayerVector otherLayers;
  MapReindexer::getOtherLayers(map, otherLayers);
  for (osgEarth::VisibleLayerVector::const_iterator iter = otherLayers.begin(); iter != otherLayers.end(); ++iter)
  {
    if (otherCallbacks_.contains(iter->get()))
      iter->get()->removeCallback(otherCallbacks_.find(iter->get())->get());
  }
  // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
  assert(otherCallbacks_.size() == static_cast<int>(otherLayers.size()));
  otherCallbacks_.clear();
#endif

  // Remove the map callback itself
  map->removeMapCallback(mapListener_.get());
}

void MapDataModel::fillModel_(osgEarth::Map *map)
{
  // assume begin/end reset model surrounds this function
  if (map == nullptr)
    return;

  osgEarth::ImageLayerVector imageLayers;
  MapReindexer::getLayers(map, imageLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::ImageLayerVector::const_reverse_iterator iter = imageLayers.rbegin(); iter != imageLayers.rend(); ++iter)
  {
    imageGroup_()->insertChild(new ImageLayerItem(imageGroup_(), iter->get()), 0);
#if OSGEARTH_SOVERSION >= 152
    registerLayerCallbacks_(*iter->get());
#else
    osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ImageLayerListener(*this);
    imageCallbacks_[iter->get()] = cb.get();
    static_cast<osgEarth::TileLayer*>(*iter)->addCallback(cb.get());
#endif
  }

  osgEarth::ElevationLayerVector elevationLayers;
  MapReindexer::getLayers(map, elevationLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::ElevationLayerVector::const_reverse_iterator iter = elevationLayers.rbegin(); iter != elevationLayers.rend(); ++iter)
  {
    elevationGroup_()->insertChild(new ElevationLayerItem(elevationGroup_(), iter->get()), 0);
#if OSGEARTH_SOVERSION >= 152
    registerLayerCallbacks_(*iter->get());
#else
    osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ElevationLayerListener(*this);
    elevationCallbacks_[iter->get()] = cb.get();
    static_cast<osgEarth::TileLayer*>(*iter)->addCallback(cb.get());
#endif
  }

  FeatureModelLayerVector featureLayers;
  MapReindexer::getLayers(map, featureLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (FeatureModelLayerVector::const_reverse_iterator iter = featureLayers.rbegin(); iter != featureLayers.rend(); ++iter)
  {
    featureGroup_()->insertChild(new FeatureModelLayerItem(featureGroup_(), iter->get()), 0);
#if OSGEARTH_SOVERSION >= 152
    registerLayerCallbacks_(*iter->get());
#else
    osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new FeatureModelLayerListener(*this);
    featureCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
#endif
  }

#ifdef HAVE_SIMUTIL
  VelocityParticleLayerVector velocityLayers;
  MapReindexer::getLayers(map, velocityLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (VelocityParticleLayerVector::const_reverse_iterator iter = velocityLayers.rbegin(); iter != velocityLayers.rend(); ++iter)
  {
    velocityGroup_()->insertChild(new VelocityParticleLayerItem(velocityGroup_(), iter->get()), 0);
#if OSGEARTH_SOVERSION >= 152
    registerLayerCallbacks_(*iter->get());
#else
    // Velocity layers are image layers, so use an image layer listener
    osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ImageLayerListener(*this);
    imageCallbacks_[iter->get()] = cb.get();
    static_cast<osgEarth::TileLayer*>(*iter)->addCallback(cb.get());
#endif
  }
#endif

  osgEarth::VisibleLayerVector otherLayers;
  MapReindexer::getOtherLayers(map, otherLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::VisibleLayerVector::const_reverse_iterator iter = otherLayers.rbegin(); iter != otherLayers.rend(); ++iter)
  {
    otherGroup_()->insertChild(new OtherLayerItem(otherGroup_(), iter->get()), 0);
#if OSGEARTH_SOVERSION >= 152
    registerLayerCallbacks_(*iter->get());
#else
    osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new OtherLayerListener(*this);
    otherCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
#endif
  }
}

void MapDataModel::removeAllItems_(Item *group)
{
  // assume begin/end reset model surrounds this function
  while (group->rowCount())
  {
    Item *child = group->childAt(0);
    group->removeChild(child);
    delete child;
  }
}

MapDataModel::Item* MapDataModel::imageGroup_() const
{
  return rootItem_->childAt(CHILD_IMAGE);
}

MapDataModel::Item* MapDataModel::elevationGroup_() const
{
  return rootItem_->childAt(CHILD_ELEVATION);
}

MapDataModel::Item* MapDataModel::featureGroup_() const
{
  return rootItem_->childAt(CHILD_FEATURE);
}

MapDataModel::Item* MapDataModel::velocityGroup_() const
{
  return rootItem_->childAt(CHILD_VELOCITY);
}

MapDataModel::Item* MapDataModel::otherGroup_() const
{
  return rootItem_->childAt(CHILD_OTHER);
}

void MapDataModel::addImageLayer_(osgEarth::ImageLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(imageGroup_()), 0, imageGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  imageGroup_()->insertChild(new ImageLayerItem(imageGroup_(), layer), index);
  endInsertRows();

#if OSGEARTH_SOVERSION >= 152
  registerLayerCallbacks_(*layer);
#else
  osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ImageLayerListener(*this);
  imageCallbacks_[layer] = cb.get();
  static_cast<osgEarth::TileLayer*>(layer)->addCallback(cb.get());
#endif
  Q_EMIT imageLayerAdded(layer);
}

void MapDataModel::addElevationLayer_(osgEarth::ElevationLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(elevationGroup_()), 0, elevationGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  elevationGroup_()->insertChild(new ElevationLayerItem(elevationGroup_(), layer), index);
  endInsertRows();

#if OSGEARTH_SOVERSION >= 152
  registerLayerCallbacks_(*layer);
#else
  osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ElevationLayerListener(*this);
  elevationCallbacks_[layer] = cb.get();
  static_cast<osgEarth::TileLayer*>(layer)->addCallback(cb.get());
#endif
  Q_EMIT elevationLayerAdded(layer);
}

void MapDataModel::addFeatureLayer_(osgEarth::FeatureModelLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(featureGroup_()), 0, featureGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  featureGroup_()->insertChild(new FeatureModelLayerItem(featureGroup_(), layer), index);
  endInsertRows();

#if OSGEARTH_SOVERSION >= 152
  registerLayerCallbacks_(*layer);
#else
  osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new FeatureModelLayerListener(*this);
  featureCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
#endif
  Q_EMIT featureLayerAdded(layer);
}

void MapDataModel::addVelocityLayer_(simUtil::VelocityParticleLayer* layer, unsigned int index)
{
#ifdef HAVE_SIMUTIL
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(velocityGroup_()), 0, velocityGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  velocityGroup_()->insertChild(new VelocityParticleLayerItem(velocityGroup_(), layer), index);
  endInsertRows();

#if OSGEARTH_SOVERSION >= 152
  registerLayerCallbacks_(*layer);
#else
  osg::ref_ptr<osgEarth::TileLayerCallback> cb = new ImageLayerListener(*this);
  imageCallbacks_[layer] = cb.get();
  static_cast<osgEarth::TileLayer*>(layer)->addCallback(cb.get());
#endif
  Q_EMIT velocityLayerAdded(layer);
#endif
}

void MapDataModel::addOtherLayer_(osgEarth::VisibleLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(otherGroup_()), 0, otherGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  otherGroup_()->insertChild(new OtherLayerItem(otherGroup_(), layer), index);
  endInsertRows();

#if OSGEARTH_SOVERSION >= 152
  registerLayerCallbacks_(*layer);
#else
  osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new OtherLayerListener(*this);
  otherCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
#endif
  Q_EMIT otherLayerAdded(layer);
}

MapDataModel::Item* MapDataModel::itemAt_(const QModelIndex &index) const
{
  if (!index.isValid())
    return nullptr;

  Item *const ret = static_cast<Item*>(index.internalPointer());
  assert(ret); // internal pointer should always be valid
  return ret;
}

QModelIndex MapDataModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  Item *const parentItem = itemAt_(parent);
  if (!parentItem)
  {
    // only the root item has no parent
    return createIndex(row, column, rootItem_);
  }

  Item *const childItem = parentItem->childAt(row);
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

QModelIndex MapDataModel::parent(const QModelIndex &child) const
{
  Item *const childItem = itemAt_(child);
  if (!childItem)
    return QModelIndex();

  Item *const parentItem = childItem->parent();
  if (!parentItem)
    return QModelIndex();

  Item *const parentsParent = parentItem->parent();
  if (parentsParent == nullptr)
  {
    // This means that the parent's item is nullptr yet we're not dealing with the Map
    assert(parentItem == rootItem_);
    return createIndex(0, 0, rootItem_);
  }

  const int row = parentsParent->rowOfChild(parentItem);
  return createIndex(row, 0, parentItem);
}

int MapDataModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return 1; // just the visible root, "Map"

  // only column 0 has children
  if (parent.column() > 0)
    return 0;

  const Item *const parentItem = itemAt_(parent);
  return parentItem->rowCount();
}

int MapDataModel::columnCount(const QModelIndex &parent) const
{
  // only one column of information
  return 1;
}

QVariant MapDataModel::data(const QModelIndex &index, int role) const
{
  Item *const item = itemAt_(index);
  if (!item)
    return QVariant();

  switch (role)
  {
  case Qt::DisplayRole:
    return item->name();

  case LAYER_TYPE_ROLE:
    return item->layerTypeRole();

  case LAYER_POINTER_ROLE:
    return item->layerPtr();

  case LAYER_MAP_INDEX_ROLE:
    return layerMapIndex_(static_cast<osgEarth::Layer*>(item->layerPtr().value<void*>()));

  case Qt::DecorationRole:
    {
      switch (item->layerTypeRole())
      {
      case CHILD_IMAGE:
        return imageIcon_;

      case CHILD_ELEVATION:
        return elevationIcon_;

      case CHILD_FEATURE:
        return featureIcon_;

      case CHILD_VELOCITY:
        return velocityIcon_;

      case CHILD_OTHER:
        return otherIcon_;

      default:
        return QVariant();
      }
    }

  case Qt::ForegroundRole:
    return item->color();
  }

  return QVariant();
}

QVariant MapDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    return tr("Map Data");

  return QVariant();
}

Qt::ItemFlags MapDataModel::flags(const QModelIndex& index) const
{
  Item *const item = itemAt_(index);
  if (!item)
    return Qt::NoItemFlags;

  return item->flags();
}

void MapDataModel::refreshText()
{
  const QModelIndex mapItem = index(0, 0);
  // Assertion failure means the tree structure changed and this wasn't updated
  assert(mapItem.isValid());

  // Create a lambda that emits dataChanged for each group type
  auto emitDataChanged = [mapItem, this](const Item* group, MapChildren childType) {
    if (group && group->rowCount() > 0)
    {
      const QModelIndex childItem = index(childType, 0, mapItem);
      // Assertion failure means the tree structure changed and this wasn't updated
      assert(childItem.isValid());
      Q_EMIT dataChanged(index(0, 0, childItem), index(group->rowCount() - 1, 0, childItem));
    }
  };

  emitDataChanged(imageGroup_(), CHILD_IMAGE);
  emitDataChanged(elevationGroup_(), CHILD_ELEVATION);
  emitDataChanged(featureGroup_(), CHILD_FEATURE);
  emitDataChanged(velocityGroup_(), CHILD_VELOCITY);
  emitDataChanged(otherGroup_(), CHILD_OTHER);
}

QModelIndex MapDataModel::layerIndex(const osgEarth::Layer* layer) const
{
  if (layer == nullptr)
    return QModelIndex();

  // Find the appropriate group item based on the item type
  GroupItem* group = nullptr;
  QModelIndex parentIndex;

#ifdef HAVE_SIMUTIL
  // Order matters because VelocityParticleLayer is-a ImageLayer
  if (dynamic_cast<const simUtil::VelocityParticleLayer*>(layer))
    group = static_cast<GroupItem*>(velocityGroup_());
  else
#endif
     if (dynamic_cast<const osgEarth::ImageLayer*>(layer))
      group = static_cast<GroupItem*>(imageGroup_());
  else if (dynamic_cast<const osgEarth::ElevationLayer*>(layer))
    group = static_cast<GroupItem*>(elevationGroup_());
  else if (dynamic_cast<const osgEarth::FeatureModelLayer*>(layer))
    group = static_cast<GroupItem*>(featureGroup_());
  else if (dynamic_cast<const osgEarth::VisibleLayer*>(layer))
    group = static_cast<GroupItem*>(otherGroup_());

  // Might be a new layer type we don't handle
  assert(group && group->parent());
  if (!group || !group->parent())
    return QModelIndex();

  // Return by calling createIndex with valid values, if the item exists
  Item* itemByLayer = group->itemByLayer(layer);
  if (itemByLayer == nullptr)
    return QModelIndex();
  return createIndex(group->rowOfChild(itemByLayer), 0, itemByLayer);
}

QVariant MapDataModel::layerMapIndex_(osgEarth::Layer* layer) const
{
  if (layer == nullptr || !map_.valid())
    return QVariant();

  osgEarth::LayerVector layers;
  map_->getLayers(layers);
  unsigned int index = indexOf(layers, layer);

  if (index != MapReindexer::INVALID_INDEX)
    return index;
  return QVariant();
}

void MapDataModel::registerLayerCallbacks_(osgEarth::VisibleLayer& layer)
{
#if OSGEARTH_SOVERSION >= 152
  const auto visibilityUid = layer.onVisibleChanged([this](const osgEarth::VisibleLayer* layer) {
    fireVisibilityChange_(layer);
    });
  // Failure here implies that we have a double add
  assert(visibilityCallbacks_.find(&layer) == visibilityCallbacks_.end());
  // Failure here implies osgEarth is reusing UIDs
  assert(std::find_if(visibilityCallbacks_.begin(), visibilityCallbacks_.end(),
    [visibilityUid](const auto& p) { return p.second == visibilityUid; })
    == visibilityCallbacks_.end());
  visibilityCallbacks_[&layer] = visibilityUid;

  const auto opacityUid = layer.onOpacityChanged([this](const osgEarth::VisibleLayer* layer) {
    fireOpacityChange_(layer);
    });
  // Failure here implies that we have a double add
  assert(opacityCallbacks_.find(&layer) == opacityCallbacks_.end());
  // Failure here implies osgEarth is reusing UIDs
  assert(std::find_if(opacityCallbacks_.begin(), opacityCallbacks_.end(),
    [opacityUid](const auto& p) { return p.second == opacityUid; })
    == opacityCallbacks_.end());
  opacityCallbacks_[&layer] = opacityUid;
#else
  // Not supported
  assert(0);
#endif
}

void MapDataModel::fireVisibilityChange_(const osgEarth::Layer* layer)
{
  const auto* featureModel = dynamic_cast<const osgEarth::FeatureModelLayer*>(layer);
  if (featureModel)
  {
    Q_EMIT featureLayerVisibleChanged(const_cast<osgEarth::FeatureModelLayer*>(featureModel));
    return;
  }

  const auto* elevation = dynamic_cast<const osgEarth::ElevationLayer*>(layer);
  if (elevation)
  {
    Q_EMIT elevationLayerVisibleChanged(const_cast<osgEarth::ElevationLayer*>(elevation));
    return;
  }

  const auto* image = dynamic_cast<const osgEarth::ImageLayer*>(layer);
  if (image)
  {
#ifdef HAVE_SIMUTIL
    const auto* velocity = dynamic_cast<const simUtil::VelocityParticleLayer*>(image);
    if (velocity)
    {
      Q_EMIT velocityLayerVisibleChanged(const_cast<simUtil::VelocityParticleLayer*>(velocity));
      return;
    }
#endif
    Q_EMIT imageLayerVisibleChanged(const_cast<osgEarth::ImageLayer*>(image));
    return;
  }

  const auto* visibleLayer = dynamic_cast<const osgEarth::VisibleLayer*>(layer);
  // We only add callbacks to visible layers. If this fails, either osgEarth is sending us null,
  // or we're getting unexpected failures in dynamic_cast (RTTI?), or a new edge case was added
  // to the code and not accounted for here.
  assert(visibleLayer);
  if (visibleLayer) [[likely]]
    Q_EMIT otherLayerVisibleChanged(const_cast<osgEarth::VisibleLayer*>(visibleLayer));
}

void MapDataModel::fireOpacityChange_(const osgEarth::Layer* layer)
{
  const auto* featureModel = dynamic_cast<const osgEarth::FeatureModelLayer*>(layer);
  if (featureModel)
  {
    Q_EMIT featureLayerOpacityChanged(const_cast<osgEarth::FeatureModelLayer*>(featureModel));
    return;
  }

  const auto* elevation = dynamic_cast<const osgEarth::ElevationLayer*>(layer);
  if (elevation)
  {
    // Elevation layers get the callback, but we do not support a signal
    return;
  }

  const auto* image = dynamic_cast<const osgEarth::ImageLayer*>(layer);
  if (image)
  {
#ifdef HAVE_SIMUTIL
    const auto* velocity = dynamic_cast<const simUtil::VelocityParticleLayer*>(image);
    if (velocity)
    {
      Q_EMIT velocityLayerOpacityChanged(const_cast<simUtil::VelocityParticleLayer*>(velocity));
      return;
    }
#endif
    Q_EMIT imageLayerOpacityChanged(const_cast<osgEarth::ImageLayer*>(image));
    return;
  }

  const auto* visibleLayer = dynamic_cast<const osgEarth::VisibleLayer*>(layer);
  // We only add callbacks to visible layers. If this fails, either osgEarth is sending us null,
  // or we're getting unexpected failures in dynamic_cast (RTTI?), or a new edge case was added
  // to the code and not accounted for here.
  assert(visibleLayer);
  if (visibleLayer) [[likely]]
    Q_EMIT otherLayerOpacityChanged(const_cast<osgEarth::VisibleLayer*>(visibleLayer));
}

}
