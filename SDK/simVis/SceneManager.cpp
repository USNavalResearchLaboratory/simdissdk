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
#include "osg/TexGenNode"
#include "osg/TextureRectangle"
#include "osg/Image"
#include "osg/Depth"
#include "osg/LightModel"
#include "osg/PolygonOffset"
#include "osg/ClipNode"
#include "osg/ClipPlane"
#include "osgDB/ReadFile"
#include "osgEarth/Version"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/NodeUtils"
#include "osgEarth/Horizon"
#include "osgEarth/VirtualProgram"
#include "osgEarth/ModelLayer"
#include "osgEarth/ObjectIndex"
#include "osgEarth/ScreenSpaceLayout"
#include "osgEarthDrivers/engine_rex/RexTerrainEngineOptions"
#include "osgEarthDrivers/engine_mp/MPTerrainEngineOptions"
#include "osgEarthUtil/LODBlending"

#if OSGEARTH_MIN_VERSION_REQUIRED(2,10,0)
#include "osgEarthUtil/HorizonClipPlane"
#else
#include "osgEarth/CullingUtils" // for ClipToGeocentricHorizon
#endif

#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simVis/AlphaTest.h"
#include "simVis/CentroidManager.h"
#include "simVis/Constants.h"
#include "simVis/ModelCache.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/ProjectorManager.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/SceneManager.h"

#include "osgEarth/CullingUtils"

#define LC "[SceneManager] "

using namespace simVis;

//------------------------------------------------------------------------
namespace
{

  /** Default map background color, when no terrain/imagery loaded; note: cannot currently be changed in osgEarth at runtime */
  static const osg::Vec4f MAP_COLOR(0.01f, 0.01f, 0.01f, 1.f); // off-black

  /** setUserData() tag for the scenario's object ID */
  static const std::string SCENARIO_OBJECT_ID = "scenid";

  /** Debugging callback that will dump the culling results each frame --
      useful for debugging render order */
  struct DebugCallback : public osg::NodeCallback {
    void operator()(osg::Node* node, osg::NodeVisitor* nv) {
      traverse(node, nv);
      osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
      if (cv) {
        osgEarth::Config c = osgEarth::CullDebugger().dumpRenderBin(cv->getRenderStage());
        OE_INFO << "FRAME " << cv->getFrameStamp()->getFrameNumber() << "-----------------------------------" << std::endl
            << c.toJSON(true) << std::endl;
      }
    }
  };
}

SceneManager::SceneManager()
  : hasEngineDriverProblem_(false)
{
  init_();

  // Uncomment this to activate the rendering debugger that will
  // print the cull results each frame
  //this->addCullCallback(new DebugCallback());
}

SceneManager::~SceneManager()
{
}

void SceneManager::detectTerrainEngineDriverProblems_()
{
  // Try to detect the osgearth_engine_mp driver; if not present, we will likely fail to render anything useful
  osgDB::Registry* registry = osgDB::Registry::instance();
  const std::string engineDriverExtension = "osgearth_engine_mp";
  if (registry->getReaderWriterForExtension(engineDriverExtension) != NULL)
  {
    hasEngineDriverProblem_ = false;
    return;
  }

  // Construct a user message
  std::stringstream ss;
  const std::string libName = registry->createLibraryNameForExtension(engineDriverExtension);
  ss << "osgEarth MP engine driver (" << libName << ") not found on file system.  Tried search paths:\n";
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
  material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 1.f, 1.f, 1.f));
  material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 1.f, 1.f, 1.f));
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
  addChild(mapContainer_.get());
  globeColor_ = new osg::Uniform("oe_terrain_color", MAP_COLOR);
  mapContainer_->getOrCreateStateSet()->addUniform(globeColor_, osg::StateAttribute::OVERRIDE);

  // this will assist in z-fighting of overlaid lines, sometimes
  mapContainer_->getOrCreateStateSet()->setAttributeAndModes(new osg::PolygonOffset(1, -1), osg::StateAttribute::ON);

  // handles centroids
  centroidManager_ = new CentroidManager();
  addChild(centroidManager_.get());

  // handles projected textures/videos
  projectorManager_ = new ProjectorManager();
  addChild(projectorManager_.get());

  drapeableNode_ = new osgEarth::DrapeableNode();
  drapeableNode_->setDrapingEnabled(false);
  addChild(drapeableNode_.get());

  // updates scenario objects
  scenarioManager_ = new ScenarioManager(this, projectorManager_.get());
  drapeableNode_->addChild(scenarioManager_.get());

  // Add the Model Cache's asynchronous loader node.  This is needed for asynchronous loading, which
  // requires access to the database pager mechanisms of OSG that are available during the cull traversal.
  const char* noAsyncLoad = ::getenv("SIMVIS_NO_ASYNC_LOAD");
  // Allow end user to force synchronous load
  if (!noAsyncLoad || strncmp(noAsyncLoad, "0", 1) == 0)
    addChild(simVis::Registry::instance()->modelCache()->asyncLoaderNode());

  // Configure the default terrain options
  if (simVis::useRexEngine())
  {
    osgEarth::Drivers::RexTerrainEngine::RexTerrainEngineOptions options;
    SceneManager::initializeTerrainOptions(options);
    setMapNode(new osgEarth::MapNode(options));
  }

  else // MP engine
  {
    osgEarth::Drivers::MPTerrainEngine::MPTerrainEngineOptions options;
    SceneManager::initializeTerrainOptions(options);
    setMapNode(new osgEarth::MapNode(options));
  }

  // TODO: Re-evaluate
  // getOrCreateStateSet()->setDefine("OE_TERRAIN_RENDER_NORMAL_MAP", osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  // an empty map for starters
  osgEarth::Map* map = getMap();
  if (map)
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    map->setMapName("Empty Map");
#else
    map->setName("Empty Map");
#endif

  setName("simVis::SceneManager");

#if OSGEARTH_MIN_VERSION_REQUIRED(2,10,0)
  // Install a clip node. This will activate and maintain our visible-horizon
  // clip plane for geometry (or whatever else we want clipped). Then, to activate
  // clipping on a graph, just enable the GL_CLIP_DISTANCE0+CLIPPLANE_VISIBLE_HORIZON
  // mode on its stateset; or you can use osgEarth symbology and use
  // RenderSymbol::clipPlane() = CLIPPLANE_VISIBLE_HORIZON in conjunction with
  // RenderSymbol::depthTest() = false.
  osgEarth::Util::HorizonClipPlane* hcp = new osgEarth::Util::HorizonClipPlane();
  hcp->setClipPlaneNumber(CLIPPLANE_VISIBLE_HORIZON);
  hcp->installShaders(this->getOrCreateStateSet());
  this->addCullCallback(hcp);
#else // osgEarth 2.9 or older, use ClipToGeocentricHorizon object
  // Install a clip node. This will activate and maintain our visible-horizon
  // clip plane for geometry (or whatever else we want clipped). Then, to activate
  // clipping on a graph, just enable the GL_CLIP_PLANE0 mode on its stateset; or
  // you can use osgEarth symbology and use RenderSymbol::clipPlane() = 0 in
  // conjunction with RenderSymbol::depthTest() = false.
  osg::ClipNode*  clipNode = new osg::ClipNode();
  osg::ClipPlane* horizonClipPlane = new osg::ClipPlane(CLIPPLANE_VISIBLE_HORIZON);
  clipNode->addClipPlane(horizonClipPlane);
  clipNode->addCullCallback(new osgEarth::ClipToGeocentricHorizon(getMap()->getSRS(), horizonClipPlane));
  addChild(clipNode);
  // Install shader snippet to activate clip planes in the shader
  osgEarth::VirtualProgram* clipVp = osgEarth::VirtualProgram::getOrCreate(this->getOrCreateStateSet());
  simVis::Shaders package;
  package.load(clipVp, package.setClipVertex());
#endif

  // Use the labeling render bin for our labels
  osgEarth::ScreenSpaceLayoutOptions screenOptions;
  screenOptions.renderOrder() = BIN_SCREEN_SPACE_LABEL;
  osgEarth::ScreenSpaceLayout::setOptions(screenOptions);
  
  // Turn off declutter
  osgEarth::ScreenSpaceLayout::setDeclutteringEnabled(false);

  // Run the shader generator on this stateset
  osgEarth::Registry::shaderGenerator().run(this);
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
void SceneManager::setScenarioDisplayHints(const ScenarioDisplayHints& hints)
{
  // Deprecated.  Instead use ScenarioManager::setEntityGraphStrategy(new ScenarioManager::GeoGraphEntityGraph(hints)).
  // Note that this will use the GeoGraph entity grouping strategy instead of a flat osg::Group, which has
  // typically positive performance ramifications, but may also have correctness ramifications.  EG:
  // scenarioManager_->setEntityGraphStrategy(new ScenarioManager::GeoGraphEntityGraph(hints));

  // noop -- This SceneManager method does nothing, to match previous behavior.

  // Assert to catch the problem of using this method, which should not be used.
  assert(0);
}
#endif /* USE_DEPRECATED_SIMDISSDK_API */

void SceneManager::setSkyNode(osgEarth::Util::SkyNode* skyNode)
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
  skyNode_ = NULL;

  // install a new one.
  if (skyNode != NULL)
  {
    // insert the sky between this node and its children.
    skyNode_ = skyNode;
    osgEarth::insertGroup(skyNode, this);
  }
}

bool SceneManager::isSilverLining_(const osgEarth::Util::SkyNode* skyNode) const
{
  if (skyNode == NULL)
    return false;
  try
  {
    // Attempt to use RTTI to determine if sky node is from SilverLining.  While not
    // ideal, there is no other way to automatically determine.
    const char* typeName = typeid(*skyNode).name();
    if (typeName == NULL)
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

void SceneManager::setOceanNode(osgEarth::Util::OceanNode* oceanNode)
{
  removeOceanNode();

  if (oceanNode != NULL)
  {
    oceanNode_ = oceanNode;
    osg::Group* oceanParent = skyNode_.valid() ? skyNode_->asGroup() : this->asGroup();
    oceanParent->addChild(oceanNode);
  }
}

void SceneManager::removeOceanNode()
{
  if (oceanNode_ != NULL)
  {
    oceanNode_->getParent(0)->removeChild(oceanNode_.get());
    oceanNode_ = NULL;
  }
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
}

void SceneManager::setMap(osgEarth::Map* map)
{
  // Swaps out the layers of underlying mapNode_ with layers in map.
  if (map == NULL)
    return;

  if (mapNode_.valid())
  {
    osgEarth::Map* currentMap = mapNode_->getMap();
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    currentMap->setMapName(map->getMapName());
#else
    currentMap->setName(map->getName());
#endif
    updateImageLayers_(*map, currentMap);
    updateElevationLayers_(*map, currentMap);
    updateModelLayers_(*map, currentMap);
  }
  else
  {
    if (simVis::useRexEngine())
    {
      osgEarth::Drivers::RexTerrainEngine::RexTerrainEngineOptions options;
      SceneManager::initializeTerrainOptions(options);
      setMapNode(new osgEarth::MapNode(map, options));
    }
    else // MP engine
    {
      osgEarth::Drivers::MPTerrainEngine::MPTerrainEngineOptions options;
      SceneManager::initializeTerrainOptions(options);
      setMapNode(new osgEarth::MapNode(map, options));
    }
  }
}

void SceneManager::updateImageLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, figure out what layers we already have
  std::map<std::string, osgEarth::ImageLayer*> loadedLayerHash;
  osgEarth::ImageLayerVector currentLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  currentMap->getLayers(currentLayers);
#else
  currentMap->getImageLayers(currentLayers);
#endif
  for (osgEarth::ImageLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    loadedLayerHash[layerHash] = iter->get();
  }

  // now figure out which layers we need to add
  osgEarth::ImageLayerVector newLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  newMap.getLayers(newLayers);
#else
  newMap.getImageLayers(newLayers);
#endif
  for (osgEarth::ImageLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    std::map<std::string, osgEarth::ImageLayer*>::iterator loadedLayerIter = loadedLayerHash.find(layerHash);

    if (loadedLayerIter == loadedLayerHash.end())
    {
      if ((*iter)->getTileSource() != NULL && (*iter)->getTileSource()->isOK())
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
        currentMap->addLayer(iter->get());
#else
        currentMap->addImageLayer(iter->get());
#endif
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
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    currentMap->removeLayer(iter->second);
#else
    currentMap->removeImageLayer(iter->second);
#endif
  }
}

void SceneManager::updateElevationLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, figure out what layers we already have
  std::map<std::string, osgEarth::ElevationLayer*> loadedLayerHash;
  osgEarth::ElevationLayerVector currentLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  currentMap->getLayers(currentLayers);
#else
  currentMap->getElevationLayers(currentLayers);
#endif
  for (osgEarth::ElevationLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    loadedLayerHash[layerHash] = iter->get();
  }

  // now figure out which layers we need to add
  osgEarth::ElevationLayerVector newLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  newMap.getLayers(newLayers);
#else
  newMap.getElevationLayers(newLayers);
#endif
  for (osgEarth::ElevationLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
  {
    std::string layerHash = getLayerHash_(iter->get());
    if (loadedLayerHash.find(layerHash) == loadedLayerHash.end())
    {
      if ((*iter)->getTileSource() != NULL && iter->get()->getTileSource()->isOK())
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
        currentMap->addLayer(iter->get());
#else
        currentMap->addElevationLayer(iter.get());
#endif
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
  {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    currentMap->removeLayer(iter->second);
#else
    currentMap->removeElevationLayer(iter->second);
#endif
  }
}

void SceneManager::updateModelLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap)
{
  // first, remove all current model layers
  osgEarth::ModelLayerVector currentLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  currentMap->getLayers(currentLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
    currentMap->removeLayer(iter->get());
#else
  currentMap->getModelLayers(currentLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = currentLayers.begin(); iter != currentLayers.end(); ++iter)
    currentMap->removeModelLayer(iter->get());
#endif

  // now add the new model layers
  osgEarth::ModelLayerVector newLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  newMap.getLayers(newLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
    currentMap->addLayer(iter->get());
#else
  newMap.getModelLayers(newLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = newLayers.begin(); iter != newLayers.end(); ++iter)
    currentMap->addModelLayer(iter->get());
#endif
}

void SceneManager::applyImageLayerDisplaySettings_(const osgEarth::ImageLayer& sourceLayer, osgEarth::ImageLayer* destLayer) const
{
  destLayer->setOpacity(sourceLayer.getOpacity());
  destLayer->setVisible(sourceLayer.getVisible());
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,8,0)
  destLayer->setEnabled(sourceLayer.getEnabled());
#endif
}

std::string SceneManager::getLayerHash_(osgEarth::TerrainLayer* layer) const
{
  // This method mimics the logic in osgEarth::TerrainLayer::setCache for generating a unique id for the layer

  // system will generate a cacheId. technically, this is not quite right, we need to remove everything that's
  // an image layer property and just use the tilesource properties.
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  const auto& layerOptions = layer->options();
#else
  const auto& layerOptions = layer->getTerrainLayerRuntimeOptions();
#endif
#if SDK_OSGEARTH_VERSION_LESS_OR_EQUAL(1,6,0)
  osgEarth::Config layerConf  = layerOptions.getConfig(true);
#else
  osgEarth::Config layerConf  = layerOptions.getConfig();
#endif
  osgEarth::Config driverConf = layerOptions.driver()->getConfig();
  // remove everything from driverConf that also appears in layerConf
  osgEarth::Config hashConf = driverConf - layerConf;
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
    mapNode_.valid() ? mapNode_->getTerrainEngine() :
    mapContainer_.get();
}

Locator* SceneManager::createLocator() const
{
  return
    mapNode_.valid() ? new Locator(mapNode_->getMap()->getProfile()->getSRS()) :
    NULL;
}

CachingLocator* SceneManager::createCachingLocator() const
{
  return
    mapNode_.valid() ? new CachingLocator(mapNode_->getMap()->getProfile()->getSRS()) :
    NULL;
}

osg::Group* SceneManager::getOrCreateAttachPoint(const std::string& name) const
{
  // Assertion failure means this method was called before init_()
  assert(scenarioManager_ != NULL);
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

void SceneManager::initializeTerrainOptions(osgEarth::Drivers::MPTerrainEngine::MPTerrainEngineOptions& options)
{
  // ensure sufficient tessellation for areas without hires data (e.g. ocean)
  options.minLOD() = 20;
  // Drop default tile size down to 7 by default to reduce tile subdivision (suggested by GW 5/13/15)
  options.tileSize() = 7;
  // Display should match the data more closely; don't smooth out elevation across large LOD
  options.elevationSmoothing() = false;
  // edges will be normalized, reducing seam stitch artifacts
  options.normalizeEdges() = true;
  // polar areas "take longer" to subdivide, reducing CPU thrash at poles
  options.adaptivePolarRangeFactor() = true;
}

void SceneManager::initializeTerrainOptions(osgEarth::Drivers::RexTerrainEngine::RexTerrainEngineOptions& options)
{
  // Drop default tile size down to 7 by default to reduce tile subdivision (suggested by GW 5/13/15)
  options.tileSize() = 7;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  // edges will be normalized, reducing seam stitch artifacts
  options.normalizeEdges() = true;
#endif
  // turn off normal maps
  options.normalMaps() = false;
}
