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
#include <algorithm>
#include "osg/ValueObject"
#include "osgEarth/GeoData"
#include "osgEarth/Horizon"
#include "osgEarth/NodeUtils"
#include "osgEarth/Registry"
#include "osgEarth/Utils"

#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Time/String.h"
#include "simData/DataStore.h"

#include "simVis/AlphaTest.h"
#include "simVis/Beam.h"
#include "simVis/BeamPulse.h"
#include "simVis/DisableDepthOnAlpha.h"
#include "simVis/DynamicScaleTransform.h"
#include "simVis/Entity.h"
#include "simVis/Gate.h"
#include "simVis/CustomRendering.h"
#include "simVis/LabelContentManager.h"
#include "simVis/Laser.h"
#include "simVis/LobGroup.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/OverrideColor.h"
#include "simVis/PlatformFilter.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/PolygonStipple.h"
#include "simVis/Projector.h"
#include "simVis/ProjectorManager.h"
#include "simVis/RadialLOSNode.h"
#include "simVis/RFProp/RFPropagationManager.h"
#include "simVis/Tool.h"
#include "simVis/TrackHistory.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/Scenario.h"

#undef LC
#define LC "[Scenario] "

/// The highest available Level of Detail from ElevationPool
static const unsigned int MAX_LOD = 23;

namespace
{
/**
 * Cull callback that installs a Horizon object with the proper eyepoint
 * in the NodeVisitor. (requires OSG 3.4+)
 */
struct SetHorizonCullCallback : public osg::NodeCallback
{
  osg::ref_ptr<osgEarth::Horizon> _horizonProto;

  explicit SetHorizonCullCallback(osgEarth::Horizon* horizon)
    : _horizonProto(horizon)
  {
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    // Do not move this declaration inside the if() statement.  The osgEarth::ObjectStorage::set()
    // solution stores the pointer in an osg::observer_ptr, so when horizon falls out of scope it
    // gets set to null.  See SIM-12601 for details.
    osg::ref_ptr<osgEarth::Horizon> horizon;
    if (_horizonProto.valid())
    {
      horizon = osg::clone(_horizonProto.get(), osg::CopyOp::DEEP_COPY_ALL);
      horizon->setEye(nv->getViewPoint());
      horizon->setName("simVis.ScenarioManager.SetHorizonCullCallback");
#if OSGEARTH_SOVERSION >= 105
      osgEarth::ObjectStorage::set(nv, horizon.get());
#else
      horizon->put(*nv);
#endif
    }
    traverse(node, nv);
  }
};

/** Calls ScenarioManager::notifyBeamsOfNewHostSize() when model node gets a bounds update. */
class BeamNoseFixer : public simVis::PlatformModelNode::Callback
{
public:
  explicit BeamNoseFixer(simVis::ScenarioManager* scenarioManager)
    : scenarioManager_(scenarioManager)
  {
  }

  virtual void operator()(simVis::PlatformModelNode* model, Callback::EventType eventType)
  {
    if (eventType == Callback::BOUNDS_CHANGED && model && model->getNumParents() > 0)
    {
      // First parent should be the simVis::PlatformNode
      const simVis::PlatformNode* platform = dynamic_cast<const simVis::PlatformNode*>(model->getParent(0));
      // Failure means layout changed.  We could try to use osgEarth::findFirstParentOfType() but it fails when parent has nodemask of 0
      assert(platform);
      osg::ref_ptr<simVis::ScenarioManager> refScenario;
      if (platform && scenarioManager_.lock(refScenario))
        refScenario->notifyBeamsOfNewHostSize(*platform);
    }
  }

private:
  osg::observer_ptr<simVis::ScenarioManager> scenarioManager_;
};

}


namespace simVis
{

ScenarioManager::EntityRecord::EntityRecord(EntityNode* node, const simData::DataSliceBase* updateSlice, simData::DataStore* dataStore)
  : node_(node),
    updateSlice_(updateSlice),
    dataStore_(dataStore)
{
}

EntityNode* ScenarioManager::EntityRecord::getEntityNode() const
{ // Convenience method for us
  return node_.get();
}

osg::Node* ScenarioManager::EntityRecord::getNode() const
{ // GeoObject interface
  return node_.get();
}

bool ScenarioManager::EntityRecord::getLocation(osg::Vec3d& output) const
{
  // Check for nullptr
  if (!node_.valid() || !node_->getLocator())
    return false;
  simCore::Vec3 outPos;

  // Retrieve position and error out if needed
  if (0 != node_->getPosition(&outPos, simCore::COORD_SYS_LLA))
    return false;
  // Convert to a Vec3d for LLA; note osgEarth expects Lon, Lat, Alt (XYZ)
  output = osg::Vec3d(outPos.y() * simCore::RAD2DEG, outPos.x() * simCore::RAD2DEG, outPos.z());
  return true;
}

bool ScenarioManager::EntityRecord::dataStoreMatches(const simData::DataStore* dataStore) const
{
  return dataStore == dataStore_;
}

bool ScenarioManager::EntityRecord::updateFromDataStore(bool force) const
{
  return (node_.valid() && node_->updateFromDataStore(updateSlice_, force));
}

// -----------------------------------------------------------------------

/** Entity group that stores all nodes in a flat osg::Group */
class ScenarioManager::SimpleEntityGraph : public osg::Referenced
{
public:
  SimpleEntityGraph();
  virtual osg::Group* node() const;
  virtual int addOrUpdate(EntityRecord* record);
  virtual int removeEntity(EntityRecord* record);
  virtual int clear();

protected:
  virtual ~SimpleEntityGraph();

private:
  osg::ref_ptr<osg::Group> group_;
};

ScenarioManager::SimpleEntityGraph::SimpleEntityGraph()
  : group_(new osg::Group)
{
  group_->setName("Entity Group");
}

ScenarioManager::SimpleEntityGraph::~SimpleEntityGraph()
{
}

osg::Group* ScenarioManager::SimpleEntityGraph::node() const
{
  return group_.get();
}

int ScenarioManager::SimpleEntityGraph::addOrUpdate(EntityRecord* record)
{
  // Assertion failure means ScenarioManager error
  assert(record != nullptr && record->getEntityNode() != nullptr);

  // add the entity to the scenegraph by adding the entity to the Group, but only if: not already in the group and not a CR that is hosted (into the scenegraph) by its host platform.
  const auto node = record->getEntityNode();
  const unsigned int numParents = node->getNumParents();
  for (unsigned int k = 0; k < numParents; ++k)
  {
    // This is an update -- don't need to do anything
    if (node->getParent(k) == group_)
      return 0;

    // custom rendering nodes hosted by platforms are attached to the scenegraph by their host; see ScenarioManager::addCustomRendering
    simData::ObjectId hostId;
    if ((node->type() == simData::CUSTOM_RENDERING) && (node->getHostId(hostId) != 0) && dynamic_cast<CustomRenderingNode*>(node))
      return 0;
  }

  // Is not in the group -- will need to add the entity
  return group_->addChild(record->getEntityNode()) ? 0 : 1;
}

int ScenarioManager::SimpleEntityGraph::removeEntity(EntityRecord* record)
{
  // Assertion failure means the entity is in multiple parents and this removal won't work
  assert(record->getEntityNode()->getNumParents() <= 1);
  if (record->getEntityNode()->getNumParents() > 0)
  {
    osg::Group* parent = record->getEntityNode()->getParent(0);
    // Assertion failure means the parent is different than what we expect,
    // so we can't use group_->removeChild(record->getEntityNode())
    return parent->removeChild(record->getEntityNode()) ? 0 : 1;
  }
  return 1;
}

int ScenarioManager::SimpleEntityGraph::clear()
{
  group_->removeChildren(0, group_->getNumChildren());
  return 0;
}

// -----------------------------------------------------------------------

/// Clamps a platform to the surface (terrain). Expects coordinates to be in LLA
class ScenarioManager::SurfaceClamping : public PlatformTspiFilter
{
public:
  /** Constructor */
  SurfaceClamping()
    : PlatformTspiFilter(),
    coordSurfaceClamping_()
  {
  }

  virtual ~SurfaceClamping()
  {
  }

  /** Returns true if surface clamping should be applied */
  virtual bool isApplicable(const simData::PlatformPrefs& prefs) const
  {
    return prefs.surfaceclamping() && coordSurfaceClamping_.isValid();
  }

  /** Applies coordinate surface clamping to the LLA coordinate */
  virtual PlatformTspiFilterManager::FilterResponse filter(simCore::Coordinate& llaCoord, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props)
  {
    if (!prefs.surfaceclamping() || !coordSurfaceClamping_.isValid())
      return PlatformTspiFilterManager::POINT_UNCHANGED;

    osgEarth::ElevationPool::WorkingSet& ws = lut_[props.id()];
    coordSurfaceClamping_.clampCoordToMapSurface(llaCoord, ws);

    return PlatformTspiFilterManager::POINT_CHANGED;
  }

  /** Sets the map pointer, required for proper clamping */
  void setMapNode(const osgEarth::MapNode* map)
  {
    coordSurfaceClamping_.setMapNode(map);
  }

  /** Changes the flag for using maximum elevation precision */
  void setUseMaxElevPrec(bool useMaxElev)
  {
    coordSurfaceClamping_.setUseMaxElevPrec(useMaxElev);
  }

  /** Removes an entity from the optimization look-up table */
  void removeEntity(simData::ObjectId id)
  {
    lut_.erase(id);
  }

private:
  CoordSurfaceClamping coordSurfaceClamping_;
  std::map<simData::ObjectId, osgEarth::ElevationPool::WorkingSet> lut_;
};


/// Prevents a platform from going below the surface (terrain). Expects coordinates to be in LLA
class ScenarioManager::AboveSurfaceClamping : public PlatformTspiFilter
{
public:
  /** Constructor */
  AboveSurfaceClamping()
    : PlatformTspiFilter(),
    useMaxElevPrec_(false)
  {
  }

  virtual ~AboveSurfaceClamping()
  {
  }

  /** Returns true if surface clamping should be applied */
  virtual bool isApplicable(const simData::PlatformPrefs& prefs) const
  {
    return prefs.abovesurfaceclamping() && mapNode_.valid();
  }

  /** Applies coordinate surface clamping to the LLA coordinate */
  virtual PlatformTspiFilterManager::FilterResponse filter(simCore::Coordinate& llaCoord, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props)
  {
    if (!prefs.abovesurfaceclamping() || !mapNode_.valid())
      return PlatformTspiFilterManager::POINT_UNCHANGED;

    // Both methods for getting terrain elevation have drawbacks that make them undesirable in certain situations. SIM-10423
    // getHeight() can give inaccurate results depending on how much map data is loaded into the scene graph, while ElevationEnvelope can be prohibitively slow if there are many clamped entities
    double elevation = 0;

    if (useMaxElevPrec_)
    {
      osgEarth::GeoPoint point(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, 0, osgEarth::ALTMODE_ABSOLUTE);
      osgEarth::ElevationSample sample = mapNode_->getMap()->getElevationPool()->getSample(point, osgEarth::Distance(1.0, osgEarth::Units::METERS), nullptr);
      if (sample.hasData())
        elevation = sample.elevation().as(osgEarth::Units::METERS);
    }
    else
    {
      double hamsl;  // Not used
      double terrainHeightHae = 0.0; // height above ellipsoid, the rough elevation
      if (mapNode_->getTerrain()->getHeight(mapNode_->getMapSRS(), llaCoord.lon()*simCore::RAD2DEG, llaCoord.lat()*simCore::RAD2DEG, &hamsl, &terrainHeightHae))
        elevation = terrainHeightHae;
    }

    if (llaCoord.alt() < elevation)
    {
      llaCoord.setPositionLLA(llaCoord.lat(), llaCoord.lon(), elevation);
      return PlatformTspiFilterManager::POINT_CHANGED;
    }

    return PlatformTspiFilterManager::POINT_UNCHANGED;
  }

  /** Sets the map pointer, required for proper clamping */
  void setMapNode(const osgEarth::MapNode* map)
  {
    mapNode_ = map;
  }

  void setUseMaxElevPrec(bool useMaxElevPrec)
  {
    if (useMaxElevPrec_ == useMaxElevPrec)
      return;

    useMaxElevPrec_ = useMaxElevPrec;
  }

private:
  osg::observer_ptr<const osgEarth::MapNode> mapNode_;
  bool useMaxElevPrec_;
};

// -----------------------------------------------------------------------

class ScenarioManager::ScenarioLosCreator : public LosCreator
{
public:
  ScenarioLosCreator()
  {
  }

  virtual ~ScenarioLosCreator()
  {
  }

  void setMapNode(osgEarth::MapNode* map)
  {
    map_ = map;
  }

  virtual RadialLOSNode* newLosNode()
  {
    if (map_.valid())
      return new RadialLOSNode(map_.get());
    return nullptr;
  }

private:
  osg::observer_ptr<osgEarth::MapNode> map_;
};

// -----------------------------------------------------------------------

/**
 * Cull callback that supplies a reference year in the NodeVisitor
 * for time based culling. (requires OSG 3.4+)
 */
class ScenarioManager::SetRefYearCullCallback : public osg::NodeCallback
{
public:
  SetRefYearCullCallback()
  {
  }

  void setCurrTime(simCore::TimeStamp currTime)
  {
    currTime_ = currTime;
  }

  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) override
  {
    // simCore::Timestamp can't be stored directly.  Separate it into constituent elements and recombine where needed
    nv->setUserValue("simVis.ScenarioManager.RefYear", currTime_.referenceYear());
    nv->setUserValue("simVis.ScenarioManager.Seconds", currTime_.secondsSinceRefYear().Double());
    traverse(node, nv);
  }

protected:
  ~SetRefYearCullCallback() {}

private:
  simCore::TimeStamp currTime_;
};

// -----------------------------------------------------------------------

ScenarioManager::ScenarioManager(ProjectorManager* projMan)
  : platformTspiFilterManager_(new PlatformTspiFilterManager()),
  surfaceClamping_(nullptr),
  aboveSurfaceClamping_(nullptr),
  lobSurfaceClamping_(nullptr),
  root_(new osg::Group),
  entityGraph_(new SimpleEntityGraph),
  projectorManager_(projMan),
  labelContentManager_(new NullLabelContentManager()),
  rfManager_(new simRF::NullRFPropagationManager()),
  losCreator_(new ScenarioLosCreator())
{
  root_->setName("root");
  root_->addChild(entityGraph_->node());
  addChild(root_.get());

  // Install a callback that will convey the Horizon info
#if OSGEARTH_SOVERSION >= 110
  osgEarth::Ellipsoid em;
  // 11km is rough depth of Mariana Trench; decrease radius to help horizon culling work underwater
  em.setSemiMajorAxis(em.getRadiusEquator() - 11000.0);
  em.setSemiMinorAxis(em.getRadiusPolar() - 11000.0);
#else
  osg::EllipsoidModel em;
  // 11km is rough depth of Mariana Trench; decrease radius to help horizon culling work underwater
  em.setRadiusEquator(em.getRadiusEquator() - 11000.0);
  em.setRadiusPolar(em.getRadiusPolar() - 11000.0);
#endif
  SetHorizonCullCallback* setHorizon = new SetHorizonCullCallback(new osgEarth::Horizon(em));
  addCullCallback(setHorizon);

  refYearCallback_ = new SetRefYearCullCallback();
  addCullCallback(refYearCallback_);

  // Clamping requires a Group for MapNode changes
  surfaceClamping_ = new SurfaceClamping();
  aboveSurfaceClamping_ = new AboveSurfaceClamping();
  lobSurfaceClamping_ = new CoordSurfaceClamping();

  // set normal rescaling so that dynamically-scaled platforms have
  // proper lighting. Note: once we move to using shaders we don't
  // need this anymore
  osg::StateSet* stateSet = getOrCreateStateSet();
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
  // GL_RESCALE_NORMAL is deprecated in GL CORE builds
  stateSet->setMode(GL_RESCALE_NORMAL, 1);
#endif
  // Lighting will be off for all objects under the Scenario,
  // unless explicitly turned on further down the scene graph
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);

  setName("simVis::ScenarioManager");

  platformTspiFilterManager_->addFilter(surfaceClamping_);
  platformTspiFilterManager_->addFilter(aboveSurfaceClamping_);

  // Install shaders used by multiple entities at the scenario level
  AlphaTest::installShaderProgram(stateSet);
  BeamPulse::installShaderProgram(stateSet);
  DisableDepthOnAlpha::installShaderProgram(stateSet);
  LobGroupNode::installShaderProgram(stateSet);
  OverrideColor::installShaderProgram(stateSet);
  PolygonStipple::installShaderProgram(stateSet);
  TrackHistoryNode::installShaderProgram(stateSet);

  scenarioEciLocator_ = new Locator();
}

ScenarioManager::~ScenarioManager()
{
  // Do not delete surfaceClamping_ or surfaceLimiting_
  delete platformTspiFilterManager_;
  platformTspiFilterManager_ = nullptr;
  delete lobSurfaceClamping_;
  lobSurfaceClamping_ = nullptr;
  delete losCreator_;
  losCreator_ = nullptr;
  // guarantee that ScenarioTools receive OnUninstall() calls
  removeAllTools_();
}

void ScenarioManager::bind(simData::DataStore* dataStore)
{
  assert(dataStore != nullptr);

  // sets up notifications so that changes to the datastore will
  // create objects in the scene graph:
  dataStoreAdapter_.bind(dataStore, this);
}

void ScenarioManager::unbind(simData::DataStore* dataStore, bool clearAll)
{
  dataStoreAdapter_.unbind(dataStore);

  if (clearAll)
  {
    clearEntities(dataStore);
  }
}

void ScenarioManager::setLabelContentManager(LabelContentManager* manager)
{
  if (manager == nullptr)
    labelContentManager_ = new NullLabelContentManager();
  else
    labelContentManager_ = manager;
}

void ScenarioManager::setRFPropagationManager(simRF::RFPropagationManagerPtr manager)
{
  if (manager == nullptr)
    rfManager_.reset(new simRF::NullRFPropagationManager());
  else
    rfManager_ = manager;
}

simRF::RFPropagationManagerPtr ScenarioManager::rfPropagationManager() const
{
  return rfManager_;
}

void ScenarioManager::flush(simData::ObjectId flushedId)
{
  SAFETRYBEGIN;
  notifyToolsOfFlush_(flushedId);
  // if id 0, flush entire scenario
  if (flushedId == 0)
  {
    for (EntityRepo::const_iterator i = entities_.begin(); i != entities_.end(); ++i)
    {
      const EntityRecord* record = i->second.get();
      static_cast<EntityNode*>(record->getNode())->flush();
    }
  }
  else // flush individual entity
  {
    EntityNode* entity = find(flushedId);
    if (entity)
      entity->flush();
  }
  SAFETRYEND("flushing scenario entities");
}

void ScenarioManager::clearEntities(simData::DataStore* dataStore)
{
  SAFETRYBEGIN;

  if (dataStore)
  {
    // remove all data associated with a particular datastore.
    for (EntityRepo::iterator i = entities_.begin(); i != entities_.end();)
    {
      EntityRecord* record = i->second.get();
      if (record)
      {
        if (record->dataStoreMatches(dataStore))
        {
          notifyToolsOfRemove_(record->getEntityNode());

          if (record->getEntityNode()->type() == simData::PROJECTOR)
          {
            const ProjectorNode* projectorNode = dynamic_cast<const ProjectorNode*>(record->getEntityNode());
            if (projectorNode)
              projectorManager_->unregisterProjector(projectorNode);
          }

          // remove it from the scene graph:
          entityGraph_->removeEntity(record);

          // remove it from the entities list (works because EntityRepo is a map, will not work for vector)
          entities_.erase(i++);
        }
        else
        {
          ++i;
        }
      }
    }
    // All entities have been removed, forget about any hosting relationships
    hosterTable_.clear();
  }
  else
  {
    // just remove everything.
    entityGraph_->clear();
    entities_.clear();
    projectorManager_->clear();
    hosterTable_.clear();
  }
  SAFETRYEND("clearing scenario entities");
}

void ScenarioManager::removeEntity(simData::ObjectId id)
{
  SAFETRYBEGIN;
  const EntityRepo::iterator i = entities_.find(id);
  EntityRecord* record = (i != entities_.end()) ? i->second.get() : nullptr;
  if (record)
  {
    EntityNode* entity = record->getEntityNode();
    notifyToolsOfRemove_(entity);

    // Remove it from the surface clamping algorithm
    surfaceClamping_->removeEntity(id);

    // If this is a projector node, delete this from the projector manager
    if (entity->type() == simData::PROJECTOR)
    {
      const ProjectorNode* projectorNode = dynamic_cast<const ProjectorNode*>(entity);
      if (projectorNode)
        projectorManager_->unregisterProjector(projectorNode);
    }
    entityGraph_->removeEntity(record);

    // remove from the hoster table
    hosterTable_.erase(id);
    // if entity was hosted by another entity, remove the link to this entity from other entity
    for (auto it = hosterTable_.begin(); it != hosterTable_.end();)
    {
      auto erase = it++;
      if (erase->second == id)
        hosterTable_.erase(erase);
    }

    // remove it from the entities list
    entities_.erase(i);
  }
  SAFETRYEND("removing entity from scenario");
}

void ScenarioManager::setMapNode(osgEarth::MapNode* map)
{
  SAFETRYBEGIN;
  mapNode_ = map;

  losCreator_->setMapNode(mapNode_.get());
  surfaceClamping_->setMapNode(mapNode_.get());
  aboveSurfaceClamping_->setMapNode(mapNode_.get());
  lobSurfaceClamping_->setMapNode(mapNode_.get());
  SAFETRYEND("setting map in scenario");
}

PlatformNode* ScenarioManager::addPlatform(const simData::PlatformProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // create the OSG node representing this entity
  PlatformNode* node = new PlatformNode(props,
    dataStore,
    *platformTspiFilterManager_,
    root_.get(), // for expire mode group attachment
    new Locator(scenarioEciLocator_.get()),
    dataStore.referenceYear());
  node->getModel()->addCallback(new BeamNoseFixer(this));

  // put it in the vis database.
  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.platformUpdateSlice(node->getId()),
    &dataStore);

  node->setLosCreator(losCreator_);

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding platform");
  return nullptr;
}

BeamNode* ScenarioManager::addBeam(const simData::BeamProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the beam to its host platform:
  PlatformNode* host = nullptr;
  if (props.has_hostid())
    host = find<PlatformNode>(props.hostid());

  // make a locator, tying it to the host's locator if there is one
  Locator* locator = host ? host->getLocator() : new Locator();

  // put the beam into our entity db:
  BeamNode* node = new BeamNode(props, locator, host, dataStore.referenceYear());

  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.beamUpdateSlice(node->getId()),
    &dataStore);

  if (host)
  {
    hosterTable_.insert(std::make_pair(host->getId(), node->getId()));
    node->setHostMissileOffset(host->getFrontOffset());
  }

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding beam");
  return nullptr;
}

GateNode* ScenarioManager::addGate(const simData::GateProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the gate to its host beam or platform:
  EntityNode* host = nullptr;
  if (props.has_hostid())
    host = find(props.hostid());

  if ((props.type() == simData::GateProperties_GateType_TARGET) && (dynamic_cast<BeamNode*>(host) == nullptr))
  {
    // simVis gate will not update this gate - it will look just like an invisible zombie
    SIM_WARN << "ScenarioManager::addGate: a target gate requires a Beam host; gate will be ignored." << std::endl;
  }

  Locator* locator = host ? host->getLocator() : new Locator();

  GateNode* node = new GateNode(props, locator, host, dataStore.referenceYear());

  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.gateUpdateSlice(node->getId()),
    &dataStore);

  if (host)
    hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding gate");
  return nullptr;
}

LaserNode* ScenarioManager::addLaser(const simData::LaserProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the laser to its host platform:
  EntityNode* host = nullptr;
  if (props.has_hostid())
    host = find(props.hostid());

  Locator* locator = host ? host->getLocator() : new Locator();

  LaserNode* node = new LaserNode(props, locator, host, dataStore.referenceYear());

  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.laserUpdateSlice(node->getId()),
    &dataStore);

  if (host)
    hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding laser");
  return nullptr;
}

LobGroupNode* ScenarioManager::addLobGroup(const simData::LobGroupProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor to the host platform
  EntityNode* host = nullptr;
  if (props.has_hostid())
    host = find(props.hostid());

  // no host, no LOB group.
  if (!host)
    return nullptr;

  LobGroupNode* node = new LobGroupNode(props, host, lobSurfaceClamping_, dataStore);

  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.lobGroupUpdateSlice(node->getId()),
    &dataStore);

  hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding LOB group");
  return nullptr;
}

CustomRenderingNode* ScenarioManager::addCustomRendering(const simData::CustomRenderingProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor to the host
  EntityNode* host = nullptr;
  if (props.has_hostid())
    host = find(props.hostid());

  // put the custom into our entity db:
  auto node = new CustomRenderingNode(this, props, host, dataStore.referenceYear());
  if (host)
  {
    // host will attach the cr to the scenegraph; ScenarioManager::SimpleEntityGraph::addOrUpdate will understand not to attach to scenario's group
    host->addChild(node);
  }
  entities_[node->getId()] = new EntityRecord(node, nullptr, &dataStore);
  hosterTable_.insert(std::make_pair((host ? host->getId() : 0), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding custom");
  return nullptr;
}

ProjectorNode* ScenarioManager::addProjector(const simData::ProjectorProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  EntityNode* host = nullptr;
  if (props.has_hostid())
    host = find(props.hostid());

  Locator* locator = host ? host->getLocator() : new Locator();

  ProjectorNode* node = new ProjectorNode(props, locator, host);

  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.projectorUpdateSlice(node->getId()),
    &dataStore);

  if (host)
    hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  projectorManager_->registerProjector(node);

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  node->setNodeGetter(nodeGetter_);

  return node;
  SAFETRYEND("adding projector");
  return nullptr;
}

bool ScenarioManager::setPlatformPrefs(simData::ObjectId id, const simData::PlatformPrefs& prefs)
{
  SAFETRYBEGIN;
  PlatformNode* platform = find<PlatformNode>(id);
  if (platform)
  {
    // Note that this may trigger the Beam Nose Fixer indirectly
    platform->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting platform prefs of ID " << id));
  return false;
}

bool ScenarioManager::setBeamPrefs(simData::ObjectId id, const simData::BeamPrefs& prefs)
{
  SAFETRYBEGIN;
  BeamNode* beam = find<BeamNode>(id);
  if (beam)
  {
    beam->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting beam prefs of ID " << id));
  return false;
}

bool ScenarioManager::setGatePrefs(simData::ObjectId id, const simData::GatePrefs& prefs)
{
  SAFETRYBEGIN;
  GateNode* gate = find<GateNode>(id);
  if (gate)
  {
    gate->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting gate prefs of ID " << id));
  return false;
}

bool ScenarioManager::setProjectorPrefs(simData::ObjectId id, const simData::ProjectorPrefs& prefs)
{
  SAFETRYBEGIN;
  ProjectorNode* proj = find<ProjectorNode>(id);
  if (proj)
  {
    proj->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting projector prefs of ID " << id));
  return false;
}

bool ScenarioManager::setLaserPrefs(simData::ObjectId id, const simData::LaserPrefs& prefs)
{
  SAFETRYBEGIN;
  LaserNode* obj = find<LaserNode>(id);
  if (obj)
  {
    obj->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting laser prefs of ID " << id));
  return false;
}

bool ScenarioManager::setLobGroupPrefs(simData::ObjectId id, const simData::LobGroupPrefs& prefs)
{
  SAFETRYBEGIN;
  LobGroupNode* obj = find<LobGroupNode>(id);
  if (obj)
  {
    obj->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting LOB group prefs of ID " << id));
  return false;
}

bool ScenarioManager::setCustomRenderingPrefs(simData::ObjectId id, const simData::CustomRenderingPrefs& prefs)
{
  SAFETRYBEGIN;
  CustomRenderingNode* obj = find<CustomRenderingNode>(id);
  if (obj)
  {
    obj->setPrefs(prefs);
    return true;
  }
  SAFETRYEND(std::string(osgEarth::Stringify() << "setting custom prefs of ID " << id));
  return false;
}

void ScenarioManager::notifyBeamsOfNewHostSize(const PlatformNode& platform) const
{
  SAFETRYBEGIN;
  std::pair< HosterTable::const_iterator, HosterTable::const_iterator > range =
    hosterTable_.equal_range(platform.getId());

  for (HosterTable::const_iterator i = range.first; i != range.second; ++i)
  {
    BeamNode* beam = find<BeamNode>(i->second);
    if (beam)
      beam->setHostMissileOffset(platform.getFrontOffset());
  }
  SAFETRYEND("notifying beams of new host size");
}

void ScenarioManager::setUseMaxElevClampPrec(bool useMaxPrec)
{
  surfaceClamping_->setUseMaxElevPrec(useMaxPrec);
  aboveSurfaceClamping_->setUseMaxElevPrec(useMaxPrec);
  lobSurfaceClamping_->setUseMaxElevPrec(useMaxPrec);
}

EntityNode* ScenarioManager::find(const simData::ObjectId& id) const
{
  SAFETRYBEGIN;
  EntityRepo::const_iterator i = entities_.find(id);
  return i != entities_.end() ? static_cast<EntityNode*>(i->second->getNode()) : nullptr;
  SAFETRYEND(std::string(osgEarth::Stringify() << "finding entity ID " << id));
  return nullptr;
}

const EntityNode* ScenarioManager::getHostPlatform(const EntityNode* entity) const
{
  if (entity == nullptr)
    return nullptr;

  simData::ObjectId hostId;
  while (entity->getHostId(hostId))
  {
    entity = find(hostId);
    if (entity == nullptr)
    {
      // An orphan entity without a host platform
      assert(false);
      return nullptr;
    }
  }

  return entity;
}

namespace {

#ifndef NDEBUG
/** Visitor that, in debug mode, asserts that the overhead mode hint is set to a certain value */
class AssertOverheadModeHint : public osg::NodeVisitor
{
public:
  AssertOverheadModeHint(bool expectedHint, TraversalMode tm = osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    : NodeVisitor(tm),
    expectedHint_(expectedHint)
  {
  }

  /** Assert that the hint is set to what we expect */
  virtual void apply(osg::MatrixTransform& mx)
  {
    simVis::LocatorNode* node = dynamic_cast<simVis::LocatorNode*>(&mx);
    if (node)
    {
      assert(node->overheadModeHint() == expectedHint_);
    }
    traverse(mx);
  }

private:
  bool expectedHint_;
};
#endif

}

EntityNode* ScenarioManager::find(osg::View* _view, float x, float y, int typeMask) const
{
  View* view = dynamic_cast<View*>(_view);
  if (!view)
  {
    SIM_WARN << "ScenarioManager::findEntity: ILLEGAL: view is not a simVis::View" << std::endl;
    return nullptr;
  }

  osg::Camera* cam = _view->getCamera();

  osg::Vec4d a;
  osg::Vec4d b;

  if (cam->getViewport())
  {
    // Assume x and y are in window coords; transform to model:
    osg::Matrix toModel;
    toModel.invert(
      cam->getViewMatrix() *
      cam->getProjectionMatrix() *
      cam->getViewport()->computeWindowMatrix());

    a = osg::Vec4d(x, y, 0.0, 1.0) * toModel;
    b = osg::Vec4d(x, y, 1.0, 1.0) * toModel;
  }
  else
  {
    // No viewport, so assume x and y are in clip coords; transform to model:
    osg::Matrix toModel;
    toModel.invert(
      cam->getViewMatrix() *
      cam->getProjectionMatrix());

    a = osg::Vec4d(x, y, -1.0, 1.0) * toModel;
    b = osg::Vec4d(x, y, 1.0, 1.0) * toModel;
  }

  osg::Vec3d beg(a.x() / a.w(), a.y() / a.w(), a.z() / a.w());
  osg::Vec3d end(b.x() / b.w(), b.y() / b.w(), b.z() / b.w());

#ifndef NDEBUG
  // In debug mode, make sure the overhead hint is false, else a release mode
  // optimization that presumes hint is false will fail.
  AssertOverheadModeHint assertHintIsFalse(false);
  assertHintIsFalse.setTraversalMask(typeMask);
  // Assertion failure means that the overhead mode hint was true.  This means
  // someone set the hint and didn't reset it when done.  This will cause failures
  // in the code below.  Either fix the offender that set the flag and didn't
  // reset it, or forcibly set the flag to true/false unconditionally.
  cam->accept(assertHintIsFalse);
#endif

  // Turn on the overhead mode hint if the View is in overhead mode
  if (view->isOverheadEnabled())
  {
    // First set the overhead mode hint; this also dirties the bounds
    SetOverheadModeHintVisitor setOverheadMode(true);
    setOverheadMode.setTraversalMask(typeMask);
    cam->accept(setOverheadMode);
  }

  // Dynamic scale cache will be out of date and needs a visitation to fix
  DynamicScaleTransform::recalculateAllDynamicScaleBounds(*cam);

  // configure the line segment intersector
  osgUtil::LineSegmentIntersector* lsi = new osgUtil::LineSegmentIntersector(beg, end);
  osgUtil::IntersectionVisitor iv(lsi);
  iv.setTraversalMask(typeMask);
  iv.setReferenceEyePoint(osg::Vec3d(0, 0, 0) * view->getCamera()->getInverseViewMatrix());
  simVis::OverheadMode::prepareVisitor(view, &iv);
  cam->accept(iv);

  // Go back and turn off overhead mode if needed, so that bounds are correctly recomputed
  if (view->isOverheadEnabled())
  {
    SetOverheadModeHintVisitor setOverheadMode(false);
    setOverheadMode.setTraversalMask(typeMask);
    cam->accept(setOverheadMode);
  }

  if (lsi->containsIntersections())
  {
    for (osgUtil::LineSegmentIntersector::Intersections::iterator i = lsi->getIntersections().begin();
      i != lsi->getIntersections().end();
      ++i)
    {
      const osg::NodePath& path = i->nodePath;
      for (osg::NodePath::const_reverse_iterator p = path.rbegin(); p != path.rend(); ++p)
      {
        if (dynamic_cast<EntityNode*>(*p))
          return static_cast<EntityNode*>(*p);
      }
    }
  }

  return nullptr;
}

void ScenarioManager::addTool(ScenarioTool* tool)
{
  SAFETRYBEGIN;
  if (tool)
  {
    ScenarioToolVector::iterator i = std::find(scenarioTools_.begin(), scenarioTools_.end(), tool);
    if (i != scenarioTools_.end())
    {
      SIM_WARN << LC << "WARNING: adding a tool that is already installed!" << std::endl;
    }

    scenarioTools_.push_back(tool);
    tool->onInstall(*this);
    root_->addChild(tool->getNode());
  }
  SAFETRYEND("installing scenario tool");
}

void ScenarioManager::removeTool(ScenarioTool* tool)
{
  SAFETRYBEGIN;
  ScenarioToolVector::iterator i = std::find(scenarioTools_.begin(), scenarioTools_.end(), tool);
  if (i != scenarioTools_.end())
  {
    ScenarioTool* tool = i->get();
    root_->removeChild(tool->getNode());
    tool->onUninstall(*this);
    scenarioTools_.erase(i);
  }
  SAFETRYEND("removing scenario tool");
}

void ScenarioManager::getTools(std::vector< osg::ref_ptr<ScenarioTool> >& tools) const
{
  SAFETRYBEGIN;
  std::copy(scenarioTools_.begin(), scenarioTools_.end(), std::back_inserter(tools));
  SAFETRYEND("retrieving scenario tools")
}

void ScenarioManager::notifyToolsOfAdd_(EntityNode* node)
{
  for (ScenarioToolVector::iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    i->get()->onEntityAdd(*this, node);
  }
}

void ScenarioManager::notifyToolsOfRemove_(EntityNode* node)
{
  for (ScenarioToolVector::iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    i->get()->onEntityRemove(*this, node);
  }
}

void ScenarioManager::notifyToolsOfFlush_(simData::ObjectId flushedId)
{
  for (const auto& scenarioToolRefPtr : scenarioTools_)
    scenarioToolRefPtr->onFlush(*this, flushedId);
}

void ScenarioManager::update(simData::DataStore* ds, bool force)
{
  // update the base eci locator rotation
  if (scenarioEciLocator_.get())
    scenarioEciLocator_->setEciRotationTime(ds->updateTime(), ds->updateTime());

  EntityVector updates;

  SAFETRYBEGIN;
  for (EntityRepo::const_iterator i = entities_.begin(); i != entities_.end(); ++i)
  {
    EntityRecord* record = i->second.get();

    bool appliedUpdate = false;

    // Note that entity classes decide how to process 'force' and record->updateSlice_->hasChanged()
    if (record->updateFromDataStore(force))
    {
      updates.push_back(record->getEntityNode());
      appliedUpdate = true;
    }

    if (appliedUpdate)
      entityGraph_->addOrUpdate(record);
  }
  SAFETRYEND("checking scenario for updates");

  //if ( updated > 0 )
  //  SIM_INFO << LC << "Updated " << updated << std::endl;

  // next, update all the scenario tools
  bool needsRedraw = false;
  const simCore::TimeStamp updateTimeStamp(ds->referenceYear(), ds->updateTime());

  for (ScenarioToolVector::const_iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    SAFETRYBEGIN;
    ScenarioTool* tool = i->get();
    if (updates.size() > 0 || tool->isDirty())
    {
      tool->onUpdate(*this, updateTimeStamp, updates);
      needsRedraw = true;
    }
    SAFETRYEND("updating scenario tools");
  }

  if (ds->getBoundClock())
  {
    simCore::Clock* clock = ds->getBoundClock();
    // Set the reference year for time based culling.  If the clock doesn't have valid bounds and isn't in live mode,
    // set an invalid reference year to indicate no such culling should be done
    if (clock->startTime() == simCore::MIN_TIME_STAMP && clock->endTime() == simCore::INFINITE_TIME_STAMP && clock->isLiveMode())
      refYearCallback_->setCurrTime(simCore::INFINITE_TIME_STAMP);
    else
      refYearCallback_->setCurrTime(clock->currentTime());
  }
  else
    refYearCallback_->setCurrTime(simCore::INFINITE_TIME_STAMP);

  if (needsRedraw)
  {
    SAFETRYBEGIN;
    // "dirty" the scene graph
    osgEarth::ViewVisitor<osgEarth::RequestRedraw> visitor;
    this->accept(visitor);
    SAFETRYEND("requesting redraw on scenario");
  }
}

void ScenarioManager::removeAllTools_()
{
  std::vector< osg::ref_ptr<ScenarioTool> > scenarioTools;
  getTools(scenarioTools);
  for (std::vector< osg::ref_ptr<ScenarioTool> >::const_iterator i = scenarioTools.begin(); i != scenarioTools.end(); ++i)
  {
    removeTool(i->get());
  }
}

void ScenarioManager::notifyOfClockChange(const simCore::Clock* clock)
{
  for (EntityRepo::iterator i = entities_.begin(); i != entities_.end(); ++i)
  {
    EntityRecord* record = i->second.get();
    record->getEntityNode()->updateClockMode(clock);
  }
}

void ScenarioManager::getObjectsHostedBy(const simData::ObjectId& hostId, std::set<simData::ObjectId>& output) const
{
  output.clear();

  std::pair< HosterTable::const_iterator, HosterTable::const_iterator > range =
    hosterTable_.equal_range(hostId);

  for (HosterTable::const_iterator i = range.first; i != range.second; ++i)
  {
    output.insert(i->second);
  }
}

void ScenarioManager::getAllEntities(EntityVector& output) const
{
  output.reserve(entities_.size());

  for (EntityRepo::const_iterator i = entities_.begin(); i != entities_.end(); ++i)
  {
    output.push_back(i->second->getEntityNode());
  }
}

osg::Group* ScenarioManager::getOrCreateAttachPoint(const std::string& name)
{
  osg::Group* result = nullptr;
  std::map<std::string, osg::observer_ptr<osg::Group> >::const_iterator i = customAttachPoints_.find(name);
  if (i != customAttachPoints_.end() && i->second.valid())
  {
    result = i->second.get();
  }
  else
  {
    result = new osg::Group();
    result->setName(name);
    customAttachPoints_[name] = result;
    addChild(result); // Ownership through ref_ptr
  }
  return result;
}
}
