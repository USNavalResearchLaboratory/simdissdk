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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_RADIAL_LOS_NODE_H
#define SIMVIS_RADIAL_LOS_NODE_H

#include "simVis/RadialLOS.h"

#include "osgEarth/MapNode"
#include "osgEarth/GeoData"
#include "osgEarth/GeoPositionNode"

#include "osg/Geode"
#include "osg/Geometry"

namespace osgEarth { class DrapeableNode; }

namespace simVis
{

/**
 * Radial line-of-sight node renders an LOS display corresponding to
 * the data in a RadialLOS structure.
 */
class SDKVIS_EXPORT RadialLOSNode : public osgEarth::GeoPositionNode
{
public:
  /**
   * Constructs a new LOS node.
   * @param[in ] mapNode MapNode to which to attach this object
   */
  RadialLOSNode(osgEarth::MapNode* mapNode);

  /**
   * Sets the center position of this object.
   * @param[in ] coord Origin of the LOS display
   */
  virtual bool setCoordinate(const simCore::Coordinate& coord);

  /**
   * Gets the center/origin coordinate.
   */
  const simCore::Coordinate& getCoordinate() const { return coord_; }

  /**
   * Sets the data model to visualize.
   * @param[in ] los LOS data model
   */
  void setDataModel(const RadialLOS& los);

  /**
   * Gets the data model this node is visualizing
   * @return Radial LOS data model
   */
  const RadialLOS& getDataModel() const { return los_; }

  /**@name Data Model Accessors
   * @note Setting a new data model will override values set by these functions
   * @{
   */
  void setMaxRange(const osgEarth::Distance& value);
  const osgEarth::Distance& getMaxRange() const { return los_.getMaxRange(); }

  void setCentralAzimuth(const osgEarth::Angle& value);
  const osgEarth::Angle& getCentralAzimuth() const { return los_.getCentralAzimuth(); }

  void setFieldOfView(const osgEarth::Angle& value);
  const osgEarth::Angle& getFieldOfView() const { return los_.getFieldOfView(); }

  void setRangeResolution(const osgEarth::Distance& value);
  const osgEarth::Distance& getRangeResolution() const { return los_.getRangeResolution(); }

  void setAzimuthalResolution(const osgEarth::Angle& value);
  const osgEarth::Angle& getAzimuthalResolution() const { return los_.getAzimuthalResolution(); }
  ///@}

  /**
   * Sets the "visible" color
   * param[in ] color for visible areas (rgba, [0..1])
   */
  void setVisibleColor(const osg::Vec4& color);

  /**
   * Gets the "visible" color
   */
  const osg::Vec4& getVisibleColor() const { return visibleColor_; }

  /**
   * Sets the "obstructed" color
   * @param[in ] color for obstructed areas (rgba, [0..1])
   */
  void setObstructedColor(const osg::Vec4& color);

  /**
   * Gets the "obstructed" color
   */
  const osg::Vec4& getObstructedColor() const { return obstructedColor_; }

  /** Set the node active or inactive.  Inactive node will not draw LOS or perform LOS calculations */
  void setActive(bool);

  /** Returns active state of node */
  bool getActive() const { return active_; }

public: // GeoPositionNode

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "RadialLOSNode"; }

public: // TerrainCallback

  /** Adjusts height of node based on new tiles */
  void onTileAdded(const osgEarth::TileKey& key, osg::Node* tile, osgEarth::TerrainCallbackContext& context);

public: // MapNodeObserver

  /** Sets the map node, used for positioning */
  virtual void setMapNode(osgEarth::MapNode* mapNode);

protected:
  /** dtor */
  virtual ~RadialLOSNode();

public:
  /** internal, updates the model of the LOS node */
  virtual void updateDataModel(
    const osgEarth::GeoExtent& extent,
    osg::Node*                 patch);

private:
  /** Not implemented */
  RadialLOSNode(const RadialLOSNode& rhs);

  /**
   * Central location to call los_.compute() to reduce spam on error.
   * Returning true means valid graphics were added to the scene.
   */
  bool updateLOS_(osgEarth::MapNode* mapNode, const simCore::Coordinate& coord);

  // callback hook.
  struct TerrainCallbackHook : public osgEarth::TerrainCallback
  {
    osg::observer_ptr<RadialLOSNode> node_;
    TerrainCallbackHook(RadialLOSNode* node) : node_(node) {}
    void onTileAdded(const osgEarth::TileKey& key, osg::Node* tile, osgEarth::TerrainCallbackContext&)
    {
      if (node_.valid()) node_->onTileAdded_(key, tile);
    }
  };

  RadialLOS los_;
  simCore::Coordinate coord_;
  osg::ref_ptr<osg::Geode> geode_;
  osg::ref_ptr<osgEarth::DrapeableNode> drapeable_;
  osg::Vec4 visibleColor_;
  osg::Vec4 obstructedColor_;
  osgEarth::GeoCircle bound_;
  osgEarth::optional<RadialLOS> losPrevious_;
  osg::ref_ptr<TerrainCallbackHook> callbackHook_;
  bool active_;
  bool isValid_;
  bool requireUpdateLOS_;

  /** Rebuilds the geometry if needed when parameters change. */
  void refreshGeometry_();

  // called by the terrain callback when a new tile enters the graph
  void onTileAdded_(const osgEarth::TileKey& key, osg::Node* tile);
};

} // namespace simVis

#endif // SIMVIS_RADIAL_LOS_NODE_H
