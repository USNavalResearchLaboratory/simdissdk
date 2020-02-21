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
#include "osg/Depth"
#include "osg/BlendFunc"
#include "osgUtil/CullVisitor"
#include "osgEarth/StringUtils"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/VirtualProgram"
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
  UpdateProjMatrix(ProjectorNode* node) : proj_(node)
  {
    //nop
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv) const
  {
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();
    osg::Matrixf projMat = cv->getCurrentCamera()->getInverseViewMatrix() * proj_->getTexGenMatrix();
    ss->addUniform(new osg::Uniform("simProjTexGenMat", projMat));
    cv->pushStateSet(ss.get());
    traverse(node, nv);
    cv->popStateSet();
  }

private:
  osg::ref_ptr<ProjectorNode> proj_;
};

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

ProjectorManager::ProjectorManager()
  : needReorderProjectorLayers_(false)
{
  setCullingActive(false);
  mapListener_ = new MapListener(*this);
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
      // Get existing layers in the new map
      osgEarth::LayerVector currentLayers;
      osgEarth::Map* map = mapNode_->getMap();
      if (!map)
        return;
      map->getLayers(currentLayers);

      for (ProjectorLayerVector::const_iterator piter = projectorLayers_.begin(); piter != projectorLayers_.end(); ++piter)
      {
        // Check if projector layer already exists
        bool found = false;
        for (osgEarth::LayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
        {
          ProjectorLayer* pLayer = dynamic_cast<ProjectorLayer*>((*iter).get());
          if ((*piter) == pLayer)
          {
            found = true;
            break;
          }
        }

        // If not found, add this layer to the map
        if (!found)
          map->addLayer(piter->get());
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
  osg::StateSet* projStateSet = layer->getOrCreateStateSet();
  projectorLayers_.push_back(layer);

  mapNode_->getMap()->addLayer(layer);

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

  projStateSet->addUniform(proj->projectorActive_.get());
  projStateSet->addUniform(proj->projectorAlpha_.get());
  projStateSet->addUniform(proj->texProjDirUniform_.get());
  projStateSet->addUniform(proj->texProjPosUniform_.get());
  projStateSet->addUniform(proj->useColorOverrideUniform_.get());
  projStateSet->addUniform(proj->colorOverrideUniform_.get());
}

void ProjectorManager::unregisterProjector(const ProjectorNode* proj)
{
  for (ProjectorLayerVector::iterator i = projectorLayers_.begin(); i != projectorLayers_.end(); ++i)
  {
    ProjectorLayer* layer = i->get();
    if (layer->id() == proj->getId())
    {
      // Remove it from the map:
      osg::ref_ptr<osgEarth::MapNode> mapNode;
      if (mapNode_.lock(mapNode))
        mapNode->getMap()->removeLayer(layer);

      // Remove it from the local vector:
      projectorLayers_.erase(i);
      break;
    }
  }

  // Remove projector node
  projectors_.erase(std::remove(projectors_.begin(), projectors_.end(), proj), projectors_.end());
}

void ProjectorManager::clear()
{
  // Remove it from the map:
  osg::ref_ptr<osgEarth::MapNode> mapNode;
  if (mapNode_.lock(mapNode))
  {
    for (ProjectorLayerVector::const_iterator i = projectorLayers_.begin(); i != projectorLayers_.end(); ++i)
    {
      mapNode->getMap()->removeLayer(i->get());
    }
  }
  projectorLayers_.clear();

  projectors_.clear();
}

void ProjectorManager::traverse(osg::NodeVisitor& nv)
{
  // cull only. the terrain was already traversed my osgearth so there's no need for
  // update/event travs again. (It would be nice to make this more efficient by restricting
  // the culling frustum to the projector's frustum -gw)
  if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
  {
    osg::Group::traverse(nv);
  }

  if (needReorderProjectorLayers_)
    reorderProjectorLayers_();
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
  for (auto iter = projectorLayers_.begin(); iter != projectorLayers_.end(); ++iter)
  {
    unsigned int projIndex = map->getIndexOfLayer(iter->get());
    // Check that the projector layer is in the map
    if (projIndex < numLayers)
      map->moveLayer(iter->get(), numLayers - 1);
  }
}

}
