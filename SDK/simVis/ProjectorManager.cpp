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
#include "osg/Depth"
#include "osg/BlendFunc"
#include "osgUtil/CullVisitor"
#include "osgEarth/EllipsoidIntersector"
#include "osgEarth/StringUtils"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/VirtualProgram"
#include "osgEarth/NodeUtils"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/Projector.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/ProjectorManager.h"

#undef LC
#define LC "simVis::ProjectorManager "

namespace simVis
{
/// Projector texture unit for shader and projector state sets
static const int PROJECTOR_TEXTURE_UNIT = 5;
/// Projector shadowmap unit for shader
static const int PROJECTOR_SHADOWMAP_UNIT = 6;

ProjectorManager::ProjectorLayer::ProjectorLayer(simData::ObjectId id)
  : osgEarth::Layer(),
    id_(id)
{
  setRenderType(RENDERTYPE_TERRAIN_SURFACE);
}

simData::ObjectId ProjectorManager::ProjectorLayer::id() const
{
  return id_;
}

//-------------------------------------------------------------------------
/**
 * Cull callback for a projector layer that will update the
 * texture projection matrix. Since we need the inverse view
 * matrix to properly transform from view coords to texture
 * coords, we have to install this each frame. Doing it in
 * the shader would cause precision loss and jittering.
 */
class UpdateProjMatrix : public osgEarth::Layer::TraversalCallback
{
public:
  explicit UpdateProjMatrix(ProjectorNode* node) : proj_(node)
  {
    //nop
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv) const
  {
    osg::ref_ptr<ProjectorNode> proj;
    if (!proj_.lock(proj) || !proj.valid())
      return;
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();
    const osg::Matrixd view_to_world = cv->getCurrentCamera()->getInverseViewMatrix();
    const osg::Matrixf texgen = view_to_world * proj->getTexGenMatrix();
    ss->addUniform(new osg::Uniform("simProjTexGenMat", texgen));
    const osg::Matrixf shadow = view_to_world * proj->getShadowMapMatrix();
    ss->addUniform(new osg::Uniform("simProjShadowMapMat", shadow));
    cv->pushStateSet(ss.get());
    traverse(node, nv);
    cv->popStateSet();
  }

private:
  osg::observer_ptr<ProjectorNode> proj_;
};

//-------------------------------------------------------------------------
/**
 * A class to listen to the map for new layers being added
 */
class ProjectorManager::MapListener : public osgEarth::MapCallback
{
public:
  explicit MapListener(ProjectorManager& manager)
    : manager_(manager)
  {}

  virtual void onLayerAdded(osgEarth::Layer *layer, unsigned int index)
  {
    // Can't reorder layers in the middle of an insert, so queue it instead
    manager_.needReorderProjectorLayers_ = true;
  }

private:
  ProjectorManager& manager_;
};

//-------------------------------------------------------------------------

ProjectorManager::ProjectorManager()
  : needReorderProjectorLayers_(false)
{
  setCullingActive(false);
  mapListener_ = new MapListener(*this);

  // using osg default WGS-84 ellipsoid
#if OSGEARTH_SOVERSION >= 110
  const osgEarth::Ellipsoid wgs84EllipsoidModel;
  ellipsoidIntersector_ = std::make_shared<osgEarth::Util::EllipsoidIntersector>(wgs84EllipsoidModel);
#else
  const osg::EllipsoidModel wgs84EllipsoidModel;
  // Passing address of a temporary, but it's OK because it's not retained by EllipsoidIntersector
  ellipsoidIntersector_ = std::make_shared<osgEarth::Util::EllipsoidIntersector>(&wgs84EllipsoidModel);
#endif

  // to handle state updates.
  ADJUST_UPDATE_TRAV_COUNT(this, +1);
}

ProjectorManager::~ProjectorManager()
{
  if (mapNode_.valid())
    mapNode_->getMap()->removeMapCallback(mapListener_.get());
}

const int ProjectorManager::getTextureImageUnit()
{
    return PROJECTOR_TEXTURE_UNIT;
}

const int ProjectorManager::getShadowMapImageUnit()
{
  return PROJECTOR_SHADOWMAP_UNIT;
}

void ProjectorManager::setMapNode(osgEarth::MapNode* mapNode)
{
  if (mapNode != mapNode_.get())
  {
    // Remove listener from old map
    if (mapNode_.valid() && mapNode_->getMap())
      mapNode_->getMap()->removeMapCallback(mapListener_.get());

    mapNode_ = mapNode;

    // reinitialize the projection system
    if (mapNode_.valid())
    {
      osgEarth::Map* map = mapNode_->getMap();
      if (!map)
        return;

      // Get existing layers in the new map
      std::vector<osg::ref_ptr<ProjectorLayer>> currentLayers;
      map->getLayers(currentLayers);

      for(auto& entry : projectorLayers_)
      {
        // Check if projector layer already exists in the map
        bool found = false;

        for (auto& currentLayer : currentLayers)
        {
          if (entry.second.get() == currentLayer.get())
          {
            found = true;
            break;
          }
        }

        // If not found, add this layer to the map
        if (!found)
          map->addLayer(entry.second.get());
      }
      map->addMapCallback(mapListener_.get());
    }
  }
}

void ProjectorManager::registerProjector(ProjectorNode* proj)
{
  // Check if this ProjectorNode already exists in the map and exit if so
  if (std::find(projectors_.begin(), projectors_.end(), proj) != projectors_.end())
    return;

  projectors_.push_back(proj);

  ProjectorLayer* layer = new ProjectorLayer(proj->getId());
  layer->setName("SIMSDK Projector");
  layer->setCullCallback(new UpdateProjMatrix(proj));
  projectorLayers_[proj->getId()] = layer;
  mapNode_->getMap()->addLayer(layer);

  osg::StateSet* projStateSet = layer->getOrCreateStateSet();

  // shader code to render the projectors
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(projStateSet);
  simVis::Shaders package;
  package.load(vp, package.projectorManagerVertex());
  package.load(vp, package.projectorManagerFragment());

  projStateSet->setDefine("SIMVIS_USE_REX");

  // tells the shader where to bind the sampler uniform
  projStateSet->addUniform(new osg::Uniform("simProjSampler", PROJECTOR_TEXTURE_UNIT));

  // Set texture from projector into state set
  projStateSet->setTextureAttribute(PROJECTOR_TEXTURE_UNIT, proj->getTexture());

  // tells the shader where to bind the shadow map sampler
  projStateSet->addUniform(new osg::Uniform("simProjShadowMap", ProjectorManager::getShadowMapImageUnit()));

  // Bind the shadow map texture to the shader
  projStateSet->setTextureAttribute(ProjectorManager::getShadowMapImageUnit(), proj->getShadowMap());

  // ask the projector to apply its particular values to the stateset
  proj->applyToStateSet(projStateSet);

  // provide the calculator to the projector so that the projector can calc its ellipsoid point
  proj->setCalculator(ellipsoidIntersector_);

  // attach the projector to the active map node
  proj->setMapNode(getMapNode());
}

void ProjectorManager::unregisterProjector(const ProjectorNode* proj)
{
  if (!proj) return;

  auto iter = projectorLayers_.find(proj->getId());
  if (iter != projectorLayers_.end())
  {
    ProjectorLayer* layer = iter->second.get();

    // Remove it from the map:
    osg::ref_ptr<osgEarth::MapNode> mapNode;
    if (mapNode_.lock(mapNode))
      mapNode->getMap()->removeLayer(layer);

    // Remove it from the local table:
    projectorLayers_.erase(iter);
  }

  // Remove projector node form the local collection as well
  projectors_.erase(std::remove(projectors_.begin(), projectors_.end(), proj), projectors_.end());
}

void ProjectorManager::clear()
{
  // Remove it from the map:
  osg::ref_ptr<osgEarth::MapNode> mapNode;
  if (mapNode_.lock(mapNode))
  {
    for (auto& iter : projectorLayers_)
    {
      mapNode->getMap()->removeLayer(iter.second.get());
    }
  }
  projectorLayers_.clear();

  projectors_.clear();
}

void ProjectorManager::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
  {
    if (needReorderProjectorLayers_)
      reorderProjectorLayers_();

    for (auto& projector : projectors_)
    {
      if (projector->isStateDirty())
      {
        auto iter = projectorLayers_.find(projector->getId());
        if (iter != projectorLayers_.end())
        {
          projector->applyToStateSet(iter->second->getOrCreateStateSet());
          projector->resetStateDirty();
        }
      }
    }
  }

  osg::Group::traverse(nv);
}

void ProjectorManager::reorderProjectorLayers_()
{
  needReorderProjectorLayers_ = false;

  if (!mapNode_.valid())
    return;
  osgEarth::Map* map = mapNode_->getMap();
  if (!map)
    return;

  // Force all projector layers to be at the bottom of the layer stack
  unsigned int numLayers = map->getNumLayers();

  for(auto& entry : projectorLayers_)
  {
    ProjectorLayer* layer = entry.second.get();

    unsigned int projIndex = map->getIndexOfLayer(layer);
    // Check that the projector layer is in the map
    if (projIndex < numLayers)
      map->moveLayer(layer, numLayers - 1);
  }
}

}
