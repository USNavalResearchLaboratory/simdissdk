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
#ifndef SIMQT_MAPDATAMODEL_H
#define SIMQT_MAPDATAMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osgEarth/ImageLayer"
#include "osgEarth/ElevationLayer"
#include "osgEarth/FeatureModelLayer"
#include "simCore/Common/Export.h"

namespace osgEarth {
  class Layer;
  class Map;
}

typedef std::vector<osg::ref_ptr<osgEarth::FeatureModelLayer> > FeatureModelLayerVector;

namespace simQt {

/**
 * Helper class to maintain list of layer indices for a map.  Post-2.8, indexing of map
 * layers changed from per-layer-type to per-map.  While a reasonable change, it breaks
 * a lot of functionality in Map Data Model and in code that works with map layers where
 * we treat layers differently.  This class helps map from global (map-based) indexing
 * to local (layer-type-based) indexing.
 */
class SDKQT_EXPORT MapReindexer
{
public:
  /** Initialize map reindexer with given map */
  MapReindexer(osgEarth::Map* map);
  /** Destroys the map reindexer */
  virtual ~MapReindexer();

  /** Retrieves the map image layers using a consistent interface */
  static void getLayers(osgEarth::Map* map, osgEarth::ImageLayerVector& imageLayers);
  /** Retrieves the map elevation layers using a consistent interface */
  static void getLayers(osgEarth::Map* map, osgEarth::ElevationLayerVector& elevationLayers);
  /** Retrieves the map model layers using a consistent interface */
  static void getLayers(osgEarth::Map* map, FeatureModelLayerVector& modelLayers);
  /** Retrieves all other map layers that aren't explicitly of type ImageLayer, ElevationLayer, or ModelLayer */
  static void getOtherLayers(osgEarth::Map* map, osgEarth::VisibleLayerVector& otherLayers);

  /** Sentinel value return for invalid index (layer not found) */
  static const unsigned int INVALID_INDEX;

  /** Returns the layer index relative to other layers in getLayers(ImageVector&) */
  unsigned int layerTypeIndex(osgEarth::ImageLayer* layer) const;
  /** Returns the layer index relative to other layers in getLayers(ElevationVector&) */
  unsigned int layerTypeIndex(osgEarth::ElevationLayer* layer) const;
  /** Returns the layer index relative to other layers in getLayers(FeatureModelVector&) */
  unsigned int layerTypeIndex(osgEarth::FeatureModelLayer* layer) const;
  /** Returns the layer index relative to other layers in getOtherLayers(VisibleLayerVector&) */
  unsigned int otherLayerTypeIndex(osgEarth::VisibleLayer* layer) const;

private:
  osg::observer_ptr<osgEarth::Map> map_;
};

/**
 * Abstract item model representing an osgEarth::Map.  This is a hierarchical model that
 * has three levels of hierarchy.  The top level is the Map itself.  The next level breaks
 * out the layer type into Image, Elevation, and Feature.  The final level is the individual
 * layers that are loaded in the map.
 *
 * There is only a single column, representing the name of the item.  Mid-tier layer types
 * are decorated with an icon for quick recognition by end users.
 */
class SDKQT_EXPORT MapDataModel : public QAbstractItemModel
{
  Q_OBJECT;
public:
  /** Constructor */
  explicit MapDataModel(QObject* parent=0);
  virtual ~MapDataModel();

  /// Changes the underlying Map pointer (nullptr is tolerated)
  void bindTo(osgEarth::Map* map);
  /// Retrieve the underlying map pointer
  osgEarth::Map* map() const;

  /// Retrieve the model index associated with the given map layer
  QModelIndex layerIndex(const osgEarth::Layer* layer) const;

  ///@return the index for the given row and column
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  ///@return the index of the parent of the item given by index
  virtual QModelIndex parent(const QModelIndex& child) const;
  ///@return the number of rows in the data
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  ///@return number of columns needed to hold data
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  ///@return data for given item
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  ///@return the header data for given section
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  ///@return the flags on the given item
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /// Map is the top level node; it has 4 children: Image, Elevation, Feature, and Other
  enum MapChildren
  {
    CHILD_IMAGE = 0,
    CHILD_ELEVATION,
    CHILD_FEATURE,
    CHILD_OTHER,
    CHILD_NONE
  };

  /** data() returns the pointer to the layer, or nullptr */
  static const int LAYER_POINTER_ROLE = Qt::UserRole + 0;
  /** data() returns the type of node: image, elevation, feature, or none for top level MAP selection */
  static const int LAYER_TYPE_ROLE = Qt::UserRole + 1;
  /** data() returns the 'global' map index for the layer type */
  static const int LAYER_MAP_INDEX_ROLE = Qt::UserRole + 2;

  class Item;

public slots:
  /** Refreshes the data on the Map model.  Useful when names change (which aren't signaled by osgEarth) */
  void refreshText();

signals:
  /** Qt signal as described by the signal name */
  void imageLayerVisibleChanged(osgEarth::ImageLayer* layer);
  /** Qt signal as described by the signal name */
  void imageLayerOpacityChanged(osgEarth::ImageLayer* layer);
  /** Qt signal as described by the signal name */
  void imageLayerAdded(osgEarth::ImageLayer* layer);
  /** Qt signal as described by the signal name */
  void elevationLayerVisibleChanged(osgEarth::ElevationLayer* layer);
  /** Qt signal as described by the signal name */
  void elevationLayerAdded(osgEarth::ElevationLayer* layer);
  /** Qt signal as described by the signal name */
  void featureLayerVisibleChanged(osgEarth::FeatureModelLayer* layer);
  /** Qt signal as described by the signal name */
  void featureLayerOpacityChanged(osgEarth::FeatureModelLayer* layer);
  /** Qt signal as described by the signal name */
  void featureLayerAdded(osgEarth::FeatureModelLayer* layer);

  /** Qt signal as described by the signal name */
  void otherLayerVisibleChanged(osgEarth::VisibleLayer* layer);
  /** Qt signal as described by the signal name */
  void otherLayerOpacityChanged(osgEarth::VisibleLayer* layer);
  /** Qt signal as described by the signal name */
  void otherLayerAdded(osgEarth::VisibleLayer* layer);

private: // methods
  /**
   * create all the layer items for the current map state
   * (must be called from within begin/end reset model)
   */
  void fillModel_(osgEarth::Map* map);

  /** Removes all callbacks associated with the given map. */
  void removeAllCallbacks_(osgEarth::Map* map);

  /**
   * remove all the items in a group
   * (must be called from within begin/end reset model)
   */
  void removeAllItems_(Item* group);

  /** add an image layer */
  void addImageLayer_(osgEarth::ImageLayer* layer, unsigned int index);
  /** add an elevation layer */
  void addElevationLayer_(osgEarth::ElevationLayer* layer, unsigned int index);
  /** add a feature layer */
  void addFeatureLayer_(osgEarth::FeatureModelLayer* layer, unsigned int index);
  /** add a layer other than image, elevation, or feature */
  void addOtherLayer_(osgEarth::VisibleLayer* layer, unsigned int index);

  /** return the Item for the given index (nullptr if it can't be represented) */
  Item* itemAt_(const QModelIndex &index) const;

  /** return the Item for the imagery group */
  Item* imageGroup_() const;
  /** return the Item for the elevation group */
  Item* elevationGroup_() const;
  /** return the Item for the feature group */
  Item* featureGroup_() const;
  /** return the Item for the other group */
  Item* otherGroup_() const;

  /** Retrieves the QVariant for LAYER_MAP_INDEX_ROLE for a layer.  Gives a global index on layer. */
  QVariant layerMapIndex_(osgEarth::Layer* layer) const;

  class MapListener;
  class ImageLayerListener;
  class ElevationLayerListener;
  class FeatureModelLayerListener;
  class OtherLayerListener;

  /** holds the invisible root item */
  Item* rootItem_;

  /** Icon for image layer */
  QIcon imageIcon_;
  /** Icon for elevation layer */
  QIcon elevationIcon_;
  /** Icon for feature layer */
  QIcon featureIcon_;
  /** Icon for other layer */
  QIcon otherIcon_;

  /** Maps of terrain layer callbacks */
  QMap<osgEarth::ImageLayer*, osg::ref_ptr<osgEarth::TileLayerCallback> > imageCallbacks_;
  QMap<osgEarth::ElevationLayer*, osg::ref_ptr<osgEarth::TileLayerCallback> > elevationCallbacks_;
  QMap<osgEarth::FeatureModelLayer*, osg::ref_ptr<osgEarth::VisibleLayerCallback> > featureCallbacks_;
  QMap<osgEarth::VisibleLayer*, osg::ref_ptr<osgEarth::VisibleLayerCallback> > otherCallbacks_;

  /** Weak pointer back to the map */
  osg::observer_ptr<osgEarth::Map> map_;
  osg::ref_ptr<MapListener> mapListener_;
};

}

#endif /* SIMQT_MAPDATAMODEL_H */
