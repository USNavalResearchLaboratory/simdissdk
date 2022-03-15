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
#include "osg/TexGenNode"
#include "osg/TextureRectangle"
#include "osg/Image"
#include "osg/Depth"
#include "osg/LightModel"
#include "osg/PolygonOffset"
#include "osg/ClipNode"
#include "osg/ClipPlane"
#include "osgDB/ReadFile"
#include "osgEarth/CullingUtils"
#include "osgEarth/Horizon"
#include "osgEarth/HorizonClipPlane"
#include "osgEarth/ModelLayer"
#include "osgEarth/NodeUtils"
#include "osgEarth/ObjectIndex"
#include "osgEarth/ScreenSpaceLayout"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/TerrainOptions"
#include "osgEarth/Version"
#include "osgEarth/VirtualProgram"

#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simVis/AlphaTest.h"
#include "simVis/CentroidManager.h"
#include "simVis/Constants.h"
#include "simVis/LayerRefreshCallback.h"
#include "simVis/ModelCache.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/ProjectorManager.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/SceneManager.h"

#undef LC
#define LC "[SceneManager] "

//------------------------------------------------------------------------
namespace
{
/** Default map background color, when no terrain/imagery loaded; note: cannot currently be changed in osgEarth at runtime */
static const osg::Vec4f MAP_COLOR(0.01f, 0.01f, 0.01f, 1.f); // off-black

/** setUserData() tag for the scenario's object ID */
static const std::string SCENARIO_OBJECT_ID = "scenid";

/** Debugging callback that will dump the culling results each frame -- useful for debugging render order */
struct DebugCallback : public osg::NodeCallback
{
  void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    traverse(node, nv);
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (cv)
    {
      osgEarth::Config c = osgEarth::CullDebugger().dumpRenderBin(cv->getRenderStage());
      OE_INFO << "FRAME " << cv->getFrameStamp()->getFrameNumber() << "-----------------------------------" << std::endl
        << c.toJSON(true) << std::endl;
    }
  }
};
}

namespace simVis {

SceneManager::SceneManager()
  : hasEngineDriverProblem_(false)
{
  init_();

  // Uncomment this to activate the rendering debugger that will
  // print the cull results each frame
  //addCullCallback(new DebugCallback());
}

SceneManager::~SceneManager()
{
}

void SceneManager::detectTerrainEngineDriverProblems_()
{
  // Try to detect the osgearth_engine_rex driver; if not present, we will likely fail to render anything useful
  osgDB::Registry* registry = osgDB::Registry::instance();
  const std::string engineDriverExtension = "osgearth_engine_rex";
  if (registry->getReaderWriterForExtension(engineDriverExtension) != nullptr)
  {
    hasEngineDriverProblem_ = false;
    return;
  }

  // Construct a user message
  std::stringstream ss;
  const std::string libName = registry->createLibraryNameForExtension(engineDriverExtension);
  ss << "osgEarth REX engine driver (" << libName << ") not found on file system.  Tried search paths:\n";
  const osgDB::FilePathList& libList = registry->getLibraryFilePathList();
  for (osgDB::FilePathList::const_iterator i = libList.begin(); i != libList.end(); ++i)
    ss << "  " << simCore::toNativeSeparators(osgDB::getRealPath(*i)) << "\n";
  ss << "SceneManager may not be able to start.\n";

  SIM_FATAL << ss.str();

  // Remember for future queries
  hasEngineDriverProblem_ = true;
}

bool SceneManager::hasEngineDriverProblem() const
{
  return hasEngineDriverProblem_;
}

void SceneManager::init_()
{
  detectTerrainEngineDriverProblems_();

  // Create a default material for the scene (fixes NVidia bug where unset material defaults to white)
  osg::ref_ptr<osg::Material> material = new osg::Material;
  material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3f, 0.3f, 0.3f, 1.f));
  material->setDiffuse(osg::Material::FRONT_AND_BACK, simVis::Color::White);
  material->setSpecular(osg::Material::FRONT_AND_BACK, simVis::Color::White);
  material->setShininess(osg::Material::FRONT_AND_BACK, 10.f);
  getOrCreateStateSet()->setAttributeAndModes(material, osg::StateAttribute::ON);

#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
  // Set a decent ambient intensity
  osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
  lightModel->setAmbientIntensity(osg::Vec4(0.3f, 0.3f, 0.3f, 1.f));
  getOrCreateStateSet()->setAttributeAndModes(lightModel, osg::StateAttribute::ON);
#endif

  // Set up blending to get rid of most jaggies.  Line smoothing is not enabled
  // by default, as it can cause problems when multisampling is enabled.
  getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

  // Turn on cull face by default.  Back side faces will not be visible.
  getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

  // Set a default Object ID uniform for the Object Index for the ObjectIndex picking highlight
  getOrCreateStateSet()->addUniform(new osg::Uniform(osgEarth::Registry::objectIndex()->getObjectIDUniformName().c_str(), 0u));

  // a container group so we always have a manipulator attach point:
  mapContainer_ = new osg::Group();
  mapContainer_->setName("Map Container");
  addChild(mapContainer_.get());
  globeColor_ = new osg::Uniform("oe_terrain_color", MAP_COLOR);
  mapContainer_->getOrCreateStateSet()->addUniform(globeColor_, osg::StateAttribute::OVERRIDE);

  // this will assist in z-fighting of overlaid lines, sometimes
  mapContainer_->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1, -1), osg::StateAttribute::ON);

  // handles centroids
  centroidManager_ = new CentroidManager();
  centroidManager_->setName("Centroid Manager");
  addChild(centroidManager_.get());

  // handles projected textures/videos
  projectorManager_ = new ProjectorManager();
  projectorManager_->setName("Projector Manager");
  addChild(projectorManager_.get());

  drapeableNode_ = new osgEarth::DrapeableNode();
  drapeableNode_->setName("Drapeable Scene Objects");
  drapeableNode_->setDrapingEnabled(false);
  addChild(drapeableNode_.get());

  // updates scenario objects
  scenarioManager_ = new ScenarioManager(projectorManager_.get());
  scenarioManager_->setName("Scenario");
  drapeableNode_->addChild(scenarioManager_.get());

  // Add the Model Cache's asynchronous loader node.  This is needed for asynchronous loading, which
  // requires access to the database pager mechanisms of OSG that are available during the cull traversal.
  const char* noAsyncLoad = ::getenv("SIMVIS_NO_ASYNC_LOAD");
  // Allow end user to force synchronous load
  if (!noAsyncLoad || strncmp(noAsyncLoad, "0", 1) == 0)
    addChild(simVis::Registry::instance()->modelCache()->asyncLoaderNode());

  osgEarth::MapNode* mapNode = new osgEarth::MapNode();
  SceneManager::initializeTerrainOptions(mapNode);
  setMapNode(mapNode);

  // TODO: Re-evaluate
  // getOrCreateStateSet()->setDefine("OE_TERRAIN_RENDER_NORMAL_MAP", osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  // an empty map for starters
  osgEarth::Map* map = getMap();
  if (map)
    map->setMapName("Empty Map");

  setName("simVis::SceneManager");

  // Install a clip node. This will activate and maintain our visible-horizon
  // clip plane for geometry (or whatever else we want clipped). Then, to activate
  // clipping on a graph, just enable the GL_CLIP_DISTANCE0+CLIPPLANE_VISIBLE_HORIZON
  // mode on its stateset; or you can use osgEarth symbology and use
  // RenderSymbol::clipPlane() = CLIPPLANE_VISIBLE_HORIZON in conjunction with
  // RenderSymbol::depthTest() = false.
  osgEarth::HorizonClipPlane* hcp = new osgEarth::HorizonClipPlane();
  hcp->setClipPlaneNumber(CLIPPLANE_VISIBLE_HORIZON);
  addCullCallback(hcp);

  // Use the labeling render bin for our labels
  osgEarth::ScreenSpaceLayoutOptions screenOptions;
  screenOptions.renderOrder() = BIN_SCREEN_SPACE_LABEL;
  osgEarth::ScreenSpaceLayout::setOptions(screenOptions);

  // Turn off declutter
  osgEarth::ScreenSpaceLayout::setDeclutteringEnabled(false);

  // Run the shader generator on this stateset
  osgEarth::Registry::shaderGenerator().run(this);

  // Add the callback that manages the "refresh" tag in layers
  layerRefreshCallback_ = new LayerRefreshCallback;
  layerRefreshCallback_->setMapNode(mapNode_.get());
  addUpdateCallback(layerRefreshCallback_.get());
}

void SceneManager::setSkyNode(osgEarth::SkyNode* skyNode)
{
  // don't load sky model to minimize memory usage when checking memory
  if (simVis::Registry::instance()->isMemoryCheck())
    return;

  // remove an old one:
  if (skyNode_.valid() && skyNode_->getNumParents() > 0)
  {
    osg::Group* skyNodeParent = skyNode_->getParent(0);
    if (skyNodeParent)
    {
      for (unsigned int i = 0; i < skyNode_->getNumChildren(); ++i)
      {
        skyNodeParent->addChild(skyNode_->getChild(i));
      }
      skyNodeParent->removeChild(skyNode_.get());
    }
  }
  skyNode_ = nullptr;

  // install a new one.
  if (skyNode != nullptr)
  {
    // insert the sky between this node and its children.
    skyNode_ = skyNode;
    osgEarth::insertGroup(skyNode, this);
  }
}

bool SceneManager::isSilverLining_(const osgEarth::SkyNode* skyNode) const
{
  if (skyNode == nullptr)
    return false;
  try
  {
    // Attempt to use RTTI to determine if sky node is from SilverLining.  While not
    // ideal, there is no other way to automatically determine.
    const char* typeName = typeid(*skyNode).name();
    if (typeName == nullptr)
      return false;

    // Search for the name SilverLining in the type ID
    std::string sTypeName = typeName;
    if (sTypeName.find("SilverLining") != std::string::npos)
      return true;
  }
  catch (const std::bad_typeid&)
  {
    // Shouldn't occur -- maybe RTTI isn't available?
    assert(0);
  }
  return false;
}

void SceneManager::setScenarioDraping(bool value)
{
  drapeableNode_->setDrapingEnabled(value);
}

void SceneManager::setMapNode(osgEarth::MapNode* mapNode)
{
  // Note that if this method is called directly, you may need to update
  // any views in the scene with a new manipulator attach point.  See
  // for example simVis::Viewer::setMapNode().
  osg::ref_ptr<osgEarth::MapNode> oldMapNode = getMapNode();

  if (oldMapNode != mapNode)
  {
    osg::Group* parent = mapContainer_.get();
    if (oldMapNode)
    {
      parent = oldMapNode->getParent(0);
      parent->removeChild(oldMapNode);
    }

    mapNode_ = mapNode;

    if (mapNode_.valid())
    {
      mapNode_->open();
      parent->addChild(mapNode_.get());
      scenarioManager_->setMapNode(mapNode_.get());

      // By default, the lighting on the terrain is enabled.  This can be changed
      // after calling setMapNode() by calling something like:
      // simVis::setLighting(mapNode_->getTerrainEngine()->getOrCreateStateSet(), 0);
    }

    // traverse the graph and replace any MapNode references
    // (i.e. any objects implementing MapNodeObserver)
    osgEarth::MapNodeReplacer replacer(mapNode);
    this->accept(replacer);
  }

  // Update the callback explicitly, since it's not a node that gets hit by MapNodeReplacer.
  if (layerRefreshCallback_.valid())
    layerRefreshCallback_->setMapNode(mapNode_.get());
}

void SceneManager::setMap(osgEarth::Map* map)
{
  // Swaps out the layers of underlying mapNode_ with layers in map.
  if (map == nullptr)
    return;

  if (mapNode_.valid())
  {
    osgEarth::Map* currentMap = mapNode_->getMap();
    currentMap->setMapName(map->getMapName());
    updateImageLayers_(*map, currentMap);
    updateElevationLayers_(*map, currentMap);
    updateModelLayers_(*map, currentMap);
  }
  else
  {
    osgEarth::MapNode* mapNode = new osgEarth::MapNode();
    SceneManager::initializeTerrainOptions(mapNode);
    setMapNode(mapNode);
  }
}

void SceneManager::updateImageLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, figure out what layers we already have
  std::map<std::string, osgEarth::ImageLayer*> loadedLayerHash;
  osgEarth::ImageLayerVector currentLayers;
  currentMap->getLayers(currentLayers);
  for (osgEarth::ImageLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    loadedLayerHash[layerHash] = iter->get();
  }

  // now figure out which layers we need to add
  osgEarth::ImageLayerVector newLayers;
  newMap.getLayers(newLayers);
  for (osgEarth::ImageLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    std::map<std::string, osgEarth::ImageLayer*>::iterator loadedLayerIter = loadedLayerHash.find(layerHash);

    if (loadedLayerIter == loadedLayerHash.end())
    {
      if ((*iter)->getStatus().isOK())
        currentMap->addLayer(iter->get());
      else
      {
        SIM_ERROR << "Image Layer " << (*iter)->getName() << " could not be loaded" << std::endl;
      }
    }
    else
    {
      // layer is already loaded, but update its display settings
      applyImageLayerDisplaySettings_(*(*iter), loadedLayerIter->second);
      // now remove from our loaded hash, since it's been found
      loadedLayerHash.erase(loadedLayerIter);
    }
  }

  // remove any layers leftover from currentMap not in the newMap
  for (std::map<std::string, osgEarth::ImageLayer*>::const_iterator iter = loadedLayerHash.begin(); iter != loadedLayerHash.end(); ++iter)
    currentMap->removeLayer(iter->second);
}

void SceneManager::updateElevationLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, figure out what layers we already have
  std::map<std::string, osgEarth::ElevationLayer*> loadedLayerHash;
  osgEarth::ElevationLayerVector currentLayers;
  currentMap->getLayers(currentLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    loadedLayerHash[layerHash] = iter->get();
  }

  // now figure out which layers we need to add
  osgEarth::ElevationLayerVector newLayers;
  newMap.getLayers(newLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    if (loadedLayerHash.find(layerHash) == loadedLayerHash.end())
    {
      if ((*iter)->getStatus().isOK())
        currentMap->addLayer(iter->get());
      else
      {
        SIM_ERROR << "Elevation Layer " << (*iter)->getName() << " could not be loaded" << std::endl;
      }
    }
    else
      loadedLayerHash.erase(layerHash);
  }

  // remove any layers leftover from currentMap not in the newMap
  for (std::map<std::string, osgEarth::ElevationLayer*>::const_iterator iter = loadedLayerHash.begin(); iter != loadedLayerHash.end(); ++iter)
    currentMap->removeLayer(iter->second);
}

void SceneManager::updateModelLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, remove all current model layers
  osgEarth::ModelLayerVector currentLayers;
  currentMap->getLayers(currentLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
    currentMap->removeLayer(iter->get());

  // now add the new model layers
  osgEarth::ModelLayerVector newLayers;
  newMap.getLayers(newLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
    currentMap->addLayer(iter->get());
}

void SceneManager::applyImageLayerDisplaySettings_(const osgEarth::ImageLayer& sourceLayer, osgEarth::ImageLayer* destLayer) const
{
  destLayer->setOpacity(sourceLayer.getOpacity());
  destLayer->setVisible(sourceLayer.getVisible());
#if OSGEARTH_SOVERSION >= 127
  destLayer->setOpenAutomatically(sourceLayer.getOpenAutomatically());
#else
  destLayer->setEnabled(sourceLayer.getEnabled());
#endif
}

std::string SceneManager::getLayerHash_(osgEarth::TileLayer* layer) const
{
  // This method mimics the logic in osgEarth::TileLayer::setCache for generating a unique id for the layer

  // system will generate a cacheId. technically, this is not quite right, we need to remove everything that's
  // an image layer property and just use the tilesource properties.
  const auto& layerOptions = layer->options();
  osgEarth::Config hashConf = layerOptions.getConfig();
  // remove cache-control properties before hashing.
  hashConf.remove("cache_only");
  hashConf.remove("cache_enabled");
  hashConf.remove("cache_policy");
  hashConf.remove("cacheid");
  return osgEarth::Stringify() << std::hex << osgEarth::hashString(hashConf.toJSON());
}

osg::Node* SceneManager::getManipulatorAttachPoint() const
{
  return
#if OSGEARTH_SOVERSION >= 104
    (mapNode_.valid() && mapNode_->getTerrainEngine() && mapNode_->getTerrainEngine()->getNode()) ? mapNode_->getTerrainEngine()->getNode() :
#else
    mapNode_.valid() ? mapNode_->getTerrainEngine() :
#endif
    mapContainer_.get();
}

osg::Group* SceneManager::getOrCreateAttachPoint(const std::string& name) const
{
  // Assertion failure means this method was called before init_()
  assert(scenarioManager_ != nullptr);
  return scenarioManager_->getOrCreateAttachPoint(name);
}

bool SceneManager::addChild(osg::Node *child)
{
  // Note that this method is overridden in SceneManager and is protected instead of
  // public.  This is being done because most entities should be added to the Scenario
  // and not the Scene.  Otherwise, inherited defaults like lighting or render
  // bin will not carry forward.  The intent of protecting the method is to make it
  // a compile error to remove the child.  The only expected child is the sky node.
  //
  // You can create attachment nodes by calling getOrCreateAttachPoint(), which will
  // give you a node under the Scenario Manager.
  return osg::Group::addChild(child);
}

void SceneManager::setGlobeColor(const simVis::Color& color)
{
  globeColor_->set(color);
}

void SceneManager::initializeTerrainOptions(osgEarth::MapNode* mapNode)
{
  // Default options for the Rex engine can be initialized here.
  // These options apply to the default map loaded on initialization.
}

}
