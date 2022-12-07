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
#ifndef SIMVIS_PROJECTOR_MANAGER_H
#define SIMVIS_PROJECTOR_MANAGER_H

#include "osgEarth/MapNode"
#include "osgEarth/MapNodeObserver"
#include "simVis/Projector.h"
#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"

namespace simVis
{
class ProjectorNode;

//-------------------------------------------------------------------------

/** Responsible for managing projectors in the scene */
class ProjectorManager : public osg::Group, public osgEarth::MapNodeObserver
{
public:
  /** Default constructor */
  ProjectorManager();

  /**
  * Registers a projector with the manager, so it will be included in the texture projection
  * calculations.
  * @param proj Projector to register.
  */
  void registerProjector(ProjectorNode* proj);

  /**
  * Unregisters a projector.
  * @param proj Projector to remove.
  */
  void unregisterProjector(const ProjectorNode* proj);

  /**
  * Clear all projector nodes and group nodes from manager
  */
  void clear();

  /** Texture image unit used by projectors */
  static const int getTextureImageUnit();

  /// Texture image unit for shadow map raster
  static const int getShadowMapImageUnit();

  /// Set the base (starting) texture image unit for projector textures.
  static void setBaseTextureImageUnit(int unit);

public: // MapNodeObserver

  /** Gets the map node */
  osgEarth::MapNode* getMapNode() { return mapNode_.get(); }

  /** Sets the map node */
  void setMapNode(osgEarth::MapNode* mapNode);

public: // osg::Node

  /** Override in order to limit node traversals to cull visitor only */
  void traverse(osg::NodeVisitor& nv);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "ProjectorManager"; }

protected:
  /// osg::Referenced-derived; destructor body needs to be in the .cpp
  virtual ~ProjectorManager();

private:

  /// Move projector layers to the bottom of the layer stack to ensure visibility
  void reorderProjectorLayers_();

  /// Current osgEarth MapNode
  osg::observer_ptr<osgEarth::MapNode> mapNode_;

  /// Vector for holding projector nodes created by the scenario
  std::vector< osg::ref_ptr<ProjectorNode> > projectors_;

  /// Map for associating projector node Ids to projector state sets
  typedef std::map<simData::ObjectId, osg::ref_ptr<osg::Group> > GroupMap;
  GroupMap groupMap_;

  /// REX Layer for a projector (REX engine only)
  class ProjectorLayer : public osgEarth::Layer
  {
  public:
    ProjectorLayer(simData::ObjectId id);
    /// Return owner entity id
    simData::ObjectId id() const;
  private:
    /// Owner entity id
    simData::ObjectId id_;
  };

  /// Vector of projectorLayers that have been added to the mapNode
  typedef std::unordered_map<simData::ObjectId, osg::ref_ptr<ProjectorLayer> > ProjectorLayerTable;
  ProjectorLayerTable projectorLayers_;

  /// A listener to detect new image layers and force projectors to be visible over them
  class MapListener;
  osg::ref_ptr<MapListener> mapListener_;

  /// A class that provides calculation services to projectors
  std::shared_ptr<osgEarth::Util::EllipsoidIntersector> ellipsoidIntersector_;

  /// A flag to mark when projector layers need to be moved to ensure visibility
  bool needReorderProjectorLayers_;

  /// Base (starting) texture image unit for projector textures
  static int baseTextureImageUnit_;
};
}

#endif // SIMVIS_PROJECTOR_MANAGER_H
