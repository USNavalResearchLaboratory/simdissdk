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
#include <limits>
#include <QIcon>
#include "osgEarth/Map"
#include "simCore/Calc/Math.h"
#include "simVis/osgEarthVersion.h"
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
  if (map != NULL)
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->getLayers(imageLayers);
#else
    map->getImageLayers(imageLayers);
#endif
}

void MapReindexer::getLayers(osgEarth::Map* map, osgEarth::ElevationLayerVector& elevationLayers)
{
  if (map != NULL)
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->getLayers(elevationLayers);
#else
    map->getElevationLayers(elevationLayers);
#endif
}

void MapReindexer::getLayers(osgEarth::Map* map, FeatureModelLayerVector& modelLayers)
{
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  if (map != NULL)
    map->getLayers(modelLayers);
#endif
}

void MapReindexer::getOtherLayers(osgEarth::Map* map, osgEarth::VisibleLayerVector& otherLayers)
{
  if (map == NULL)
    return;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  osgEarth::VisibleLayerVector allLayers;
  map->getLayers(allLayers);
  // pass along only layers that are not image, elevation, or model
  for (auto iter = allLayers.begin(); iter != allLayers.end(); ++iter)
  {
    const osgEarth::VisibleLayer* layer = (*iter).get();
    if (dynamic_cast<const osgEarth::ImageLayer*>(layer) != NULL)
      continue;
    if (dynamic_cast<const osgEarth::ElevationLayer*>(layer) != NULL)
      continue;
    if (dynamic_cast<const osgEarth::FeatureModelLayer*>(layer) != NULL)
      continue;
    otherLayers.push_back(*iter);
  }

#endif
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
  virtual QString name() const
  {
    return QObject::tr("Map");
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::layerTypeRole */
  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_NONE;
  }

  /** @copydoc  MapDataModel::Item::layerPtr */
  virtual QVariant layerPtr() const
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual Qt::ItemFlags flags() const
  {
    return Qt::ItemIsEnabled;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual Item* parent() const
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const
  {
    return children_.count();
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row)
  {
    return children_.value(row);
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const
  {
    return children_.indexOf(c);
  }

  /** @copydoc  MapDataModel::Item::insertChild */
  virtual void insertChild(Item *c, int position)
  {
    children_.insert(position, c);
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c)
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
  virtual QString name() const
  {
    return name_;
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::layerTypeRole */
  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_NONE;
  }

  /** @copydoc  MapDataModel::Item::layerPtr */
  virtual QVariant layerPtr() const
  {
    return QVariant();
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual Qt::ItemFlags flags() const
  {
    // not selectable
    return Qt::ItemIsEnabled;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual MapDataModel::Item* parent() const
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const
  {
    return children_.count();
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row)
  {
    return children_.value(row);
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const
  {
    return children_.indexOf(c);
  }

  /** @copydoc  MapDataModel::Item::insertChild */
  virtual void insertChild(Item *c, int position)
  {
    children_.insert(position, c);
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c)
  {
    children_.removeOne(c);
  }

  /** Searches children for one that has the layer provided, returning a row or NULL on failure */
  MapDataModel::Item* itemByLayer(const osgEarth::Layer* layer) const
  {
    for (auto it = children_.begin(); it != children_.end(); ++it)
    {
      if ((*it)->layerPtr().value<void*>() == layer)
        return (*it);
    }
    return NULL;
  }

private:
  QString name_;
  MapDataModel::Item *parent_;
  QList<MapDataModel::Item*> children_; ///< all the items under this item
};

MapItem::MapItem(MapDataModel::Item *parent)
: parent_(parent)
{
  insertChild(new GroupItem(QObject::tr("Image"), this), 0);
  insertChild(new GroupItem(QObject::tr("Elevation"), this), 1);
  insertChild(new GroupItem(QObject::tr("Model"), this), 2);
  insertChild(new GroupItem(QObject::tr("Other"), this), 3);
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
  Qt::ItemFlags flags() const
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  /** @copydoc  MapDataModel::Item::parent */
  virtual MapDataModel::Item* parent() const
  {
    return parent_;
  }

  /** @copydoc  MapDataModel::Item::rowCount */
  virtual int rowCount() const
  {
    // no children
    return 0;
  }

  /** @copydoc  MapDataModel::Item::childAt */
  virtual Item* childAt(int row)
  {
    return NULL;
  }

  /** @copydoc  MapDataModel::Item::rowOfChild */
  virtual int rowOfChild(Item *c) const
  {
    return -1;
  }

  /** @copydoc  MapDataModel::Item::flags */
  virtual void insertChild(Item *c, int position)
  {
    assert(false); // no children
  }

  /** @copydoc  MapDataModel::Item::removeChild */
  virtual void removeChild(Item *c)
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

  virtual QString name() const
  {
    return QString::fromStdString(layer_->getName());
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    if (!layer_->getStatus().isOK())
      return QColor(Qt::gray);
    return QVariant();
  }

  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_IMAGE;
  }

  virtual QVariant layerPtr() const
  {
    return QVariant::fromValue<void*>(layer_);
  }

private:
  osgEarth::ImageLayer *layer_;
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

  virtual QString name() const
  {
    return QString::fromStdString(layer_->getName());
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    if (!layer_->getStatus().isOK())
      return QColor(Qt::gray);
    return QVariant();
  }

  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_ELEVATION;
  }

  virtual QVariant layerPtr() const
  {
    return QVariant::fromValue<void*>(layer_);
  }

private:
  osgEarth::ElevationLayer *layer_;
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

  virtual QString name() const
  {
    return QString::fromStdString(layer_->getName());
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    if (!layer_->getStatus().isOK())
      return QColor(Qt::gray);
    return QVariant();
  }

  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_FEATURE;
  }

  virtual QVariant layerPtr() const
  {
    return QVariant::fromValue<void*>(layer_);
  }

private:
  osgEarth::FeatureModelLayer *layer_;
};

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

  virtual QString name() const
  {
    return QString::fromStdString(layer_->getName());
  }

  /** @copydoc  MapDataModel::Item::color */
  virtual QVariant color() const
  {
    if (!layer_->getStatus().isOK())
      return QColor(Qt::gray);
    return QVariant();
  }

  virtual MapDataModel::MapChildren layerTypeRole() const
  {
    return MapDataModel::CHILD_OTHER;
  }

  virtual QVariant layerPtr() const
  {
    return QVariant::fromValue<void*>(layer_);
  }

private:
  osgEarth::VisibleLayer *layer_;
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
  virtual void onLayerAdded(osgEarth::Layer* layer, unsigned int index)
  {
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

  virtual void onLayerMoved(osgEarth::Layer* layer, unsigned int oldIndex, unsigned int newIndex)
  {
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
  virtual void onLayerRemoved(osgEarth::Layer* layer, unsigned int index)
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
    {
      dataModel_.imageCallbacks_.remove(imageLayer);
      removeLayer_(dataModel_.imageGroup_(), imageLayer);
      return;
    }

    osgEarth::ElevationLayer* elevationLayer = dynamic_cast<osgEarth::ElevationLayer*>(layer);
    if (elevationLayer)
    {
      dataModel_.elevationCallbacks_.remove(elevationLayer);
      removeLayer_(dataModel_.elevationGroup_(), elevationLayer);
      return;
    }

    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      dataModel_.featureCallbacks_.remove(modelLayer);
      removeLayer_(dataModel_.featureGroup_(), modelLayer);
      return;
    }

    osgEarth::VisibleLayer* visibleLayer = dynamic_cast<osgEarth::VisibleLayer*>(layer);
    if (visibleLayer)
    {
      dataModel_.otherCallbacks_.remove(visibleLayer);
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

/** Watch for image layer changes */
class MapDataModel::ImageLayerListener : public osgEarth::ImageLayerCallback
{
public:
  /** Constructor */
  explicit ImageLayerListener(MapDataModel& parent)
  : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer)
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
      emit dataModel_.imageLayerVisibleChanged(imageLayer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer)
  {
    osgEarth::ImageLayer* imageLayer = dynamic_cast<osgEarth::ImageLayer*>(layer);
    if (imageLayer)
      emit dataModel_.imageLayerOpacityChanged(imageLayer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleRangeChanged(osgEarth::ImageLayer *layer)
  {
    emit dataModel_.imageLayerVisibleRangeChanged(layer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onColorFiltersChanged(osgEarth::ImageLayer *layer)
  {
    emit dataModel_.imageLayerColorFilterChanged(layer);
  }

  /** Inherited from ImageLayerCallback */
  virtual void onAltitudeChanged(class osgEarth::ImageLayer *layer)
  {
    emit dataModel_.imageLayerAltitudeChanged(layer);
  }

private:
  MapDataModel& dataModel_;
};

/** Watch for elevation layer changes */
class MapDataModel::ElevationLayerListener : public osgEarth::ElevationLayerCallback
{
public:
  /** Constructor */
  explicit ElevationLayerListener(MapDataModel& parent)
  : dataModel_(parent)
  {}

  /** Inherited from VisibleLayerCallback */
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer)
  {
    emit dataModel_.elevationLayerVisibleChanged(static_cast<osgEarth::ElevationLayer*>(layer));
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
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer)
  {
    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      emit dataModel_.featureLayerVisibleChanged(modelLayer);
#ifdef USE_DEPRECATED_SIMDISSDK_API
      emit dataModel_.modelLayerVisibleChanged(modelLayer);
#endif
    }
  }

  /** Inherited from ModelLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer)
  {
    osgEarth::FeatureModelLayer* modelLayer = dynamic_cast<osgEarth::FeatureModelLayer*>(layer);
    if (modelLayer)
    {
      emit dataModel_.featureLayerOpacityChanged(modelLayer);
#ifdef USE_DEPRECATED_SIMDISSDK_API
      emit dataModel_.modelLayerOpacityChanged(modelLayer);
#endif
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
  virtual void onVisibleChanged(osgEarth::VisibleLayer *layer)
  {
    emit dataModel_.otherLayerVisibleChanged(layer);
  }

  /** Inherited from VisibleLayerCallback */
  virtual void onOpacityChanged(osgEarth::VisibleLayer *layer)
  {
    emit dataModel_.otherLayerOpacityChanged(layer);
  }

private:
  MapDataModel& dataModel_;
};

//----------------------------------------------------------------------------
MapDataModel::MapDataModel(QObject* parent)
  : QAbstractItemModel(parent),
    rootItem_(new MapItem(NULL)),
    imageIcon_(":/simQt/images/Globe.png"),
    elevationIcon_(":/simQt/images/Image.png"),
    featureIcon_(":/simQt/images/Building Corporation.png")
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
    if (imageCallbacks_.contains(iter->get()))
      iter->get()->removeCallback(imageCallbacks_.find(iter->get())->get());
  }
  // Assertion failure means that we were out of sync with map; not a one-to-one with callback-to-layer
  assert(imageCallbacks_.size() == static_cast<int>(imageLayers.size()));

  imageCallbacks_.clear();

  // need to remove all elevation callbacks
  osgEarth::ElevationLayerVector elevationLayers;
  MapReindexer::getLayers(map, elevationLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = elevationLayers.begin(); iter != elevationLayers.end(); ++iter)
  {
    if (elevationCallbacks_.contains(iter->get()))
      iter->get()->removeCallback(elevationCallbacks_.find(iter->get())->get());
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

  // Remove the map callback itself
  map->removeMapCallback(mapListener_.get());
}

void MapDataModel::fillModel_(osgEarth::Map *map)
{
  // assume begin/end reset model surrounds this function
  if (map == NULL)
    return;

  osgEarth::ImageLayerVector imageLayers;
  MapReindexer::getLayers(map, imageLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::ImageLayerVector::const_reverse_iterator iter = imageLayers.rbegin(); iter != imageLayers.rend(); ++iter)
  {
    imageGroup_()->insertChild(new ImageLayerItem(imageGroup_(), iter->get()), 0);
    osg::ref_ptr<osgEarth::ImageLayerCallback> cb = new ImageLayerListener(*this);
    imageCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
  }

  osgEarth::ElevationLayerVector elevationLayers;
  MapReindexer::getLayers(map, elevationLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::ElevationLayerVector::const_reverse_iterator iter = elevationLayers.rbegin(); iter != elevationLayers.rend(); ++iter)
  {
    elevationGroup_()->insertChild(new ElevationLayerItem(elevationGroup_(), iter->get()), 0);
    osg::ref_ptr<osgEarth::ElevationLayerCallback> cb = new ElevationLayerListener(*this);
    elevationCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
  }

  FeatureModelLayerVector featureLayers;
  MapReindexer::getLayers(map, featureLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (FeatureModelLayerVector::const_reverse_iterator iter = featureLayers.rbegin(); iter != featureLayers.rend(); ++iter)
  {
    featureGroup_()->insertChild(new FeatureModelLayerItem(featureGroup_(), iter->get()), 0);
    osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new FeatureModelLayerListener(*this);
    featureCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
  }

  osgEarth::VisibleLayerVector otherLayers;
  MapReindexer::getOtherLayers(map, otherLayers);
  // need to reverse iterate, because we are inserting at row 0
  for (osgEarth::VisibleLayerVector::const_reverse_iterator iter = otherLayers.rbegin(); iter != otherLayers.rend(); ++iter)
  {
    otherGroup_()->insertChild(new OtherLayerItem(otherGroup_(), iter->get()), 0);
    osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new OtherLayerListener(*this);
    otherCallbacks_[iter->get()] = cb.get();
    (*iter)->addCallback(cb.get());
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
  return rootItem_->childAt(0);
}

MapDataModel::Item* MapDataModel::elevationGroup_() const
{
  return rootItem_->childAt(1);
}

MapDataModel::Item* MapDataModel::featureGroup_() const
{
  return rootItem_->childAt(2);
}

MapDataModel::Item* MapDataModel::otherGroup_() const
{
  return rootItem_->childAt(3);
}

void MapDataModel::addImageLayer_(osgEarth::ImageLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(imageGroup_()), 0, imageGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  imageGroup_()->insertChild(new ImageLayerItem(imageGroup_(), layer), index);
  endInsertRows();

  osg::ref_ptr<osgEarth::ImageLayerCallback> cb = new ImageLayerListener(*this);
  imageCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
  emit imageLayerAdded(layer);
}

void MapDataModel::addElevationLayer_(osgEarth::ElevationLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(elevationGroup_()), 0, elevationGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  elevationGroup_()->insertChild(new ElevationLayerItem(elevationGroup_(), layer), index);
  endInsertRows();

  osg::ref_ptr<osgEarth::ElevationLayerCallback> cb = new ElevationLayerListener(*this);
  elevationCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
  emit elevationLayerAdded(layer);
}

void MapDataModel::addFeatureLayer_(osgEarth::FeatureModelLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(featureGroup_()), 0, featureGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  featureGroup_()->insertChild(new FeatureModelLayerItem(featureGroup_(), layer), index);
  endInsertRows();

  osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new FeatureModelLayerListener(*this);
  featureCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
  emit featureLayerAdded(layer);
#ifdef USE_DEPRECATED_SIMDISSDK_API
  emit modelLayerAdded(layer);
#endif
}

void MapDataModel::addOtherLayer_(osgEarth::VisibleLayer *layer, unsigned int index)
{
  const QModelIndex parentIndex = createIndex(rootItem_->rowOfChild(otherGroup_()), 0, otherGroup_());
  beginInsertRows(parentIndex, index, index);
  insertRow(index, parentIndex);
  otherGroup_()->insertChild(new OtherLayerItem(otherGroup_(), layer), index);
  endInsertRows();

  osg::ref_ptr<osgEarth::VisibleLayerCallback> cb = new OtherLayerListener(*this);
  otherCallbacks_[layer] = cb.get();
  layer->addCallback(cb.get());
  emit otherLayerAdded(layer);
}

MapDataModel::Item* MapDataModel::itemAt_(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

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
  if (parentsParent == NULL)
  {
    // This means that the parent's item is NULL yet we're not dealing with the Map
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

      case CHILD_OTHER:
        return otherIcon_;

      default:
        return QVariant();
      }
    }

  case Qt::TextColorRole:
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

  const Item* imageGroup = imageGroup_();
  // Only need to emit data changed for the image group if there are images
  if (imageGroup != NULL && imageGroup->rowCount() > 0)
  {
    const QModelIndex imageItem = index(0, 0, mapItem);
    // Assertion failure means the tree structure changed and this wasn't updated
    assert(imageItem.isValid());
    emit dataChanged(index(0, 0, imageItem), index(imageGroup->rowCount() - 1, 0, imageItem));
  }

  const Item* elevationGroup = elevationGroup_();
  // Only need to emit data changed for the elevation group if there are elevation layers
  if (elevationGroup != NULL && elevationGroup->rowCount() > 0)
  {
    const QModelIndex elevItem = index(1, 0, mapItem);
    // Assertion failure means the tree structure changed and this wasn't updated
    assert(elevItem.isValid());
    emit dataChanged(index(0, 0, elevItem), index(elevationGroup->rowCount() - 1, 0, elevItem));
  }

  const Item* featureGroup = featureGroup_();
  // Only need to emit data changed for the model group if there are model layers
  if (featureGroup != NULL && featureGroup->rowCount() > 0)
  {
    const QModelIndex featureItem = index(2, 0, mapItem);
    // Assertion failure means the tree structure changed and this wasn't updated
    assert(featureItem.isValid());
    emit dataChanged(index(0, 0, featureItem), index(featureGroup->rowCount() - 1, 0, featureItem));
  }

  const Item* otherGroup = otherGroup_();
  // Only need to emit data changed for the other group if there are other layers
  if (otherGroup != NULL && otherGroup->rowCount() > 0)
  {
    const QModelIndex otherItem = index(3, 0, mapItem);
    // Assertion failure means the tree structure changed and this wasn't updated
    assert(otherItem.isValid());
    emit dataChanged(index(0, 0, otherItem), index(otherGroup->rowCount() - 1, 0, otherItem));
  }
}

QModelIndex MapDataModel::layerIndex(const osgEarth::Layer* layer) const
{
  if (layer == NULL)
    return QModelIndex();

  // Find the appropriate group item based on the item type
  GroupItem* group = NULL;
  QModelIndex parentIndex;
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
  if (itemByLayer == NULL)
    return QModelIndex();
  return createIndex(group->rowOfChild(itemByLayer), 0, itemByLayer);
}

QVariant MapDataModel::layerMapIndex_(osgEarth::Layer* layer) const
{
  if (layer == NULL || !map_.valid())
    return QVariant();

  unsigned int index = MapReindexer::INVALID_INDEX;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  osgEarth::LayerVector layers;
  map_->getLayers(layers);
  index = indexOf(layers, layer);
#else
  if (dynamic_cast<osgEarth::ImageLayer*>(layer))
  {
    osgEarth::ImageLayerVector images;
    MapReindexer::getLayers(map_.get(), images);
    index = indexOf(images, static_cast<osgEarth::ImageLayer*>(layer));
  }
  else if (dynamic_cast<osgEarth::ElevationLayer*>(layer))
  {
    osgEarth::ElevationLayerVector elevs;
    MapReindexer::getLayers(map_.get(), elevs);
    index = indexOf(elevs, static_cast<osgEarth::ElevationLayer*>(layer));
  }
  else if (dynamic_cast<osgEarth::FeatureModelLayer*>(layer))
  {
    FeatureModelLayerVector models;
    MapReindexer::getLayers(map_.get(), models);
    index = indexOf(models, static_cast<osgEarth::FeatureModelLayer*>(layer));
  }
#endif

  if (index != MapReindexer::INVALID_INDEX)
    return index;
  return QVariant();
}

}
