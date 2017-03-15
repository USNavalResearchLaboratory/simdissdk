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
#include <algorithm>
#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/Calc/Angle.h"
#include "simData/DataStore.h"
#include "simVis/Scenario.h"
#include "simVis/LobGroup.h"
#include "simVis/Platform.h"
#include "simVis/Beam.h"
#include "simVis/Entity.h"
#include "simVis/Gate.h"
#include "simVis/Laser.h"
#include "simVis/Projector.h"
#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simVis/OverheadMode.h"
#include "simVis/OverrideColor.h"
#include "simVis/LabelContentManager.h"
#include "simVis/PlatformFilter.h"
#include "simVis/Shaders.h"
#include "simVis/TrackHistory.h"
#include "simVis/RFProp/RFPropagationManager.h"

#include "osgUtil/IntersectionVisitor"
#include "osgUtil/LineSegmentIntersector"

#include "osgEarth/AutoScale"
#include "osgEarth/GeoData"
#include "osgEarth/NodeUtils"
#include "osgEarth/Registry"
#include "osgEarth/ElevationQuery"
#include "osgEarth/Horizon"

#include <iomanip>

#define LC "[Scenario] "

using namespace simVis;
using namespace osgEarth;
using namespace osgEarth::Util;

namespace
{

/**
 * Cull callback that installs a Horizon object with the proper eyepoint
 * in the NodeVisitor. (requires OSG 3.4+)
 */
struct SetHorizonCullCallback : public osg::NodeCallback
{
  osg::ref_ptr<Horizon> _horizonProto;

  explicit SetHorizonCullCallback(Horizon* horizon)
    : _horizonProto(horizon)
  {
  }

  void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    if (_horizonProto.valid())
    {
      osg::ref_ptr<Horizon> horizon = osg::clone(_horizonProto.get(), osg::CopyOp::DEEP_COPY_ALL);
      horizon->setEye(nv->getViewPoint());
      horizon->put(*nv);
    }
    traverse(node, nv);
  }
};

}

// -----------------------------------------------------------------------

ScenarioManager::EntityRecord::EntityRecord(EntityNode* node, const simData::DataSliceBase* updateSlice, simData::DataStore* dataStore)
  : node_(node),
    updateSlice_(updateSlice),
    dataStore_(dataStore)
{
}

EntityNode* ScenarioManager::EntityRecord::getEntityNode() const
{ // Convenience method for us
  return node_;
}

osg::Node* ScenarioManager::EntityRecord::getNode() const
{ // GeoObject interface
  return node_;
}

bool ScenarioManager::EntityRecord::getLocation(osg::Vec3d& output) const
{
  // Check for NULL
  if (!node_.valid() || !node_->getLocator())
    return false;
  simCore::Vec3 outPos;

  // Retrieve position and error out if needed
  const bool rv = node_->getLocator()->getLocatorPosition(&outPos, simCore::COORD_SYS_LLA);
  if (!rv)
    return false;
  // Convert to a Vec3d for LLA; note osgEarth expects Lon, Lat, Alt (XYZ)
  output = osg::Vec3d(outPos.y() * simCore::RAD2DEG, outPos.x() * simCore::RAD2DEG, outPos.z());
  return rv;
}

bool ScenarioManager::EntityRecord::dataStoreMatches(const simData::DataStore* dataStore) const
{
  return dataStore == dataStore_;
}

bool ScenarioManager::EntityRecord::updateFromDataStore(bool force) const
{
  return (updateSlice_ && node_.valid() && node_->updateFromDataStore(updateSlice_, force));
}

// -----------------------------------------------------------------------

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
  return group_;
}

int ScenarioManager::SimpleEntityGraph::addOrUpdate(EntityRecord* record)
{
  // Assertion failure means ScenarioManager error
  assert(record != NULL && record->getEntityNode() != NULL);

  // Only need to insert in Group, and only if we're not on parents list
  const auto node = record->getNode();
  const int numParents = node->getNumParents();
  for (int k = 0; k < numParents; ++k)
  {
    // This is an update -- don't need to do anything
    if (node->getParent(k) == group_)
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

ScenarioManager::GeoGraphEntityGraph::GeoGraphEntityGraph(const ScenarioDisplayHints& hints)
  : hints_(hints),
    group_(new osg::Group),
    graph_(NULL)
{
  group_->setName("Entity Group");
  // clear() will instantiate the graph
  clear();
}

ScenarioManager::GeoGraphEntityGraph::~GeoGraphEntityGraph()
{
}

osg::Group* ScenarioManager::GeoGraphEntityGraph::node() const
{
  return group_;
}

int ScenarioManager::GeoGraphEntityGraph::addOrUpdate(EntityRecord* record)
{
  const bool inGraph = (record->getGeoCell() != NULL);
  if (inGraph)
    return graph_->reindexObject(record) ? 0 : 1;
  return graph_->insertObject(record) ? 0 : 1;
}

int ScenarioManager::GeoGraphEntityGraph::removeEntity(EntityRecord* record)
{
  return graph_->removeObject(record) ? 0 : 1;
}

int ScenarioManager::GeoGraphEntityGraph::clear()
{
  // NOTE: No way to clear out the GeoGraph, so we create a new one that's empty
  if (graph_)
    group_->removeChild(graph_);
  // Reallocate graph_, destroying the old one in the process
  graph_ = new osgEarth::Util::GeoGraph(
    osgEarth::Registry::instance()->getGlobalGeodeticProfile()->getExtent(),
    hints_.maxRange_, hints_.maxPerCell_, 2, 0.5f, hints_.cellsX_, hints_.cellsY_);
  graph_->setName("GeoGraphEntityGraph GeoGraph");
  group_->addChild(graph_);
  return 0;
}

// -----------------------------------------------------------------------

/// Clamps a platform to the surface (terrain). Expects coordinates to be in LLA
class ScenarioManager::SurfaceClamping : public PlatformTspiFilter
{
public:
  /** Constructor */
  explicit SurfaceClamping(osg::Group* scene)
    : PlatformTspiFilter(),
      coordSurfaceClamping_(scene)
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

    coordSurfaceClamping_.clampCoordToMapSurface(llaCoord);

    return PlatformTspiFilterManager::POINT_CHANGED;
  }

  /** Sets the map pointer, required for proper clamping */
  void setMap(const osgEarth::Map* map)
  {
    coordSurfaceClamping_.setMap(map);
  }

private:
  CoordSurfaceClamping coordSurfaceClamping_;
};


// -----------------------------------------------------------------------

ScenarioManager::ScenarioManager(LocatorFactory* factory, ProjectorManager* projMan)
  : locatorFactory_(factory),
    platformTspiFilterManager_(new PlatformTspiFilterManager()),
    surfaceClamping_(NULL),
    lobSurfaceClamping_(NULL),
    root_(new osg::Group),
    entityGraph_(new SimpleEntityGraph),
    projectorManager_(projMan),
    labelContentManager_(new NullLabelContentManager()),
    rfManager_(new simRF::NullRFPropagationManager())
{
  root_->setName("root");
  root_->addChild(entityGraph_->node());
  addChild(root_.get());

  // Install a callback that will convey the Horizon info
  osg::EllipsoidModel em;
  // 11km is rough depth of Mariana Trench; decrease radius to help horizon culling work underwater
  em.setRadiusEquator(em.getRadiusEquator() - 11000.0);
  em.setRadiusPolar(em.getRadiusPolar() - 11000.0);
  SetHorizonCullCallback* setHorizon = new SetHorizonCullCallback(new Horizon(em));
  root_->addCullCallback(setHorizon);

  // Clamping requires a Group for MapNode changes
  surfaceClamping_ = new SurfaceClamping(root_);
  lobSurfaceClamping_ = new CoordSurfaceClamping(root_);

  // set normal rescaling so that dynamically-scaled platforms have
  // proper lighting. Note: once we move to using shaders we don't
  // need this anymore
  osg::StateSet* stateSet = getOrCreateStateSet();
  stateSet->setMode(GL_RESCALE_NORMAL, 1);
  // Lighting will be off for all objects under the Scenario,
  // unless explicitly turned on further down the scene graph
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);

  // set a default render bin to propagate down to child nodes
  stateSet->setRenderBinDetails(BIN_POST_TERRAIN, BIN_GLOBAL_SIMSDK);

  setName("simVis::ScenarioManager");

  platformTspiFilterManager_->addFilter(surfaceClamping_);

  // Install shaders used by multiple entities at the scenario level
  OverrideColor::installShaderProgram(stateSet);
  TrackHistoryNode::installShaderProgram(stateSet);
}

ScenarioManager::~ScenarioManager()
{
  // Do not delete surfaceClamping_
  delete platformTspiFilterManager_;
  platformTspiFilterManager_ = NULL;
  delete lobSurfaceClamping_;
  lobSurfaceClamping_ = NULL;
}

void ScenarioManager::bind(simData::DataStore* dataStore)
{
  assert(dataStore != NULL);

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
  if (manager == NULL)
    labelContentManager_ = new NullLabelContentManager();
  else
    labelContentManager_ = manager;
}

void ScenarioManager::setRFPropagationManager(simRF::RFPropagationManagerPtr manager)
{
  if (manager == NULL)
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
          ProjectorNode* projectorNode = dynamic_cast<ProjectorNode*>(record->getEntityNode());
          if (projectorNode)
            projectorManager_->unregisterProjector(projectorNode);

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
  }

  else
  {
    // just remove everything.
    entityGraph_->clear();
    entities_.clear();
    projectorManager_->clear();
  }
  SAFETRYEND("clearing scenario entities");
}

void ScenarioManager::removeEntity(simData::ObjectId id)
{
  SAFETRYBEGIN;
  EntityRepo::iterator i = entities_.find(id);

  EntityRecord* record = (i != entities_.end()) ? i->second.get() : NULL;
  if (record)
  {
    notifyToolsOfRemove_(record->getEntityNode());

    // If this is a projector node, delete this from the projector manager
    ProjectorNode* projectorNode = dynamic_cast<ProjectorNode*>(record->getEntityNode());
    if (projectorNode)
    {
      projectorManager_->unregisterProjector(projectorNode);
    }
    entityGraph_->removeEntity(record);

    // remove it from the entities list
    entities_.erase(i);
  }
  SAFETRYEND("removing entity from scenario");
}

void ScenarioManager::setEntityGraphStrategy(AbstractEntityGraph* strategy)
{
  if (strategy == NULL || strategy == entityGraph_)
    return;
  // Hold onto the old strategy so it doesn't get removed until we've added all the entities
  osg::ref_ptr<AbstractEntityGraph> oldStrategy = entityGraph_;

  root_->removeChild(entityGraph_->node());
  entityGraph_ = strategy;
  // Make sure the graph is clear so that we don't add extra entities
  entityGraph_->clear();
  root_->addChild(entityGraph_->node());

  // Add each entity to the graph
  for (EntityRepo::const_iterator i = entities_.begin(); i != entities_.end(); ++i)
    entityGraph_->addOrUpdate(i->second);
}

void ScenarioManager::setMap(const osgEarth::Map* map)
{
  SAFETRYBEGIN;
  map_ = map;

  surfaceClamping_->setMap(map);
  lobSurfaceClamping_->setMap(map);
  if (map)
  {
    // update all the entity locators with the new SRS.
    for (EntityRepo::iterator i = entities_.begin(); i != entities_.end(); ++i)
    {
      EntityRecord* record = i->second.get();
      if (record)
      {
        EntityNode* node = record->getEntityNode();
        node->getLocator()->setMapSRS(map_->getSRS());
      }
    }
  }
  SAFETRYEND("setting map in scenario");
}

PlatformNode* ScenarioManager::addPlatform(const simData::PlatformProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // create the OSG node representing this entity
  PlatformNode* node = new PlatformNode(props, dataStore, *platformTspiFilterManager_, this, locatorFactory_->createLocator(), dataStore.referenceYear());

  // put it in the vis database.
  entities_[node->getId()] = new EntityRecord(
    node,
    dataStore.platformUpdateSlice(node->getId()),
    &dataStore);

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  return node;
  SAFETRYEND("adding platform");
  return NULL;
}

BeamNode* ScenarioManager::addBeam(const simData::BeamProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the beam to its host platform:
  PlatformNode* host = NULL;
  if (props.has_hostid())
    host = find<PlatformNode>(props.hostid());

  // make a locator, tying it to the host's locator if there is one
  Locator* locator = host ? host->getLocator() : locatorFactory_->createLocator();

  // put the beam into our entity db:
  BeamNode* node = new BeamNode(this, props, locator, host, dataStore.referenceYear());

  entities_[ node->getId() ] = new EntityRecord(
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

  return node;
  SAFETRYEND("adding beam");
  return NULL;
}

GateNode* ScenarioManager::addGate(const simData::GateProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the gate to its host platform:
  EntityNode* beamHost = NULL;
  EntityNode* platformHost = NULL;
  if (props.has_hostid())
  {
    beamHost = find(props.hostid());
    if (beamHost)
    {
      simData::BeamProperties beamProps;
      simData::DataStore::Transaction xaction;
      const simData::BeamProperties *liveProps = dataStore.beamProperties(props.hostid(), &xaction);
      if (liveProps)
        beamProps = *liveProps;
      xaction.release(&liveProps);

      if (beamProps.has_hostid())
        platformHost = find(beamProps.hostid());
    }
  }

  Locator* locator = beamHost ? beamHost->getLocator() : locatorFactory_->createLocator();

  GateNode* node = new GateNode(props, locator, beamHost, dataStore.referenceYear());

  entities_[ node->getId() ] = new EntityRecord(
    node,
    dataStore.gateUpdateSlice(node->getId()),
    &dataStore);

  if (platformHost)
    hosterTable_.insert(std::make_pair(beamHost->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  return node;
  SAFETRYEND("adding gate");
  return NULL;
}

LaserNode* ScenarioManager::addLaser(const simData::LaserProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor the laser to its host platform:
  EntityNode* host = NULL;
  if (props.has_hostid())
    host = find(props.hostid());

  Locator* locator = host ? host->getLocator() : locatorFactory_->createLocator();

  LaserNode* node = new LaserNode(props, locator, host, dataStore.referenceYear());

  entities_[ node->getId() ] = new EntityRecord(
    node,
    dataStore.laserUpdateSlice(node->getId()),
    &dataStore);

  if (host)
    hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  return node;
  SAFETRYEND("adding laser");
  return NULL;
}

LobGroupNode* ScenarioManager::addLobGroup(const simData::LobGroupProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  // attempt to anchor to the host platform
  EntityNode* host = NULL;
  if (props.has_hostid())
    host = find(props.hostid());

  // no host, no LOB group.
  if (!host)
    return NULL;

  LobGroupNode* node = new LobGroupNode(props, host, lobSurfaceClamping_, dataStore);

  entities_[ node->getId() ] = new EntityRecord(
    node,
    dataStore.lobGroupUpdateSlice(node->getId()),
    &dataStore);

  hosterTable_.insert(std::make_pair(host->getId(), node->getId()));

  notifyToolsOfAdd_(node);

  node->setLabelContentCallback(labelContentManager_->createLabelContentCallback(node->getId()));

  return node;
  SAFETRYEND("adding LOB group");
  return NULL;
}

ProjectorNode* ScenarioManager::addProjector(const simData::ProjectorProperties& props, simData::DataStore& dataStore)
{
  SAFETRYBEGIN;
  EntityNode* host = NULL;
  if (props.has_hostid())
    host = find(props.hostid());

  Locator* locator = host ? host->getLocator() : locatorFactory_->createLocator();

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

  return node;
  SAFETRYEND("adding projector");
  return NULL;
}

bool ScenarioManager::setPlatformPrefs(simData::ObjectId id, const simData::PlatformPrefs& prefs)
{
  SAFETRYBEGIN;
  PlatformNode* platform = find<PlatformNode>(id);
  if (platform)
  {
    // Copy the original prefs to detect changes; make a pointer for easier use with PB_FIELD_CHANGED
    const simData::PlatformPrefs lastPrefs = platform->getPrefs();
    const simData::PlatformPrefs* pLastPrefs = &lastPrefs;

    platform->setPrefs(prefs);
    const simData::PlatformPrefs* pPrefs = &prefs;

    // if the model has changed, we need to let the Beams know.
    if (PB_FIELD_CHANGED(pLastPrefs, pPrefs, icon) ||
      PB_SUBFIELD_CHANGED(pLastPrefs, pPrefs, orientationoffset, yaw) ||
      PB_SUBFIELD_CHANGED(pLastPrefs, pPrefs, orientationoffset, pitch) ||
      PB_SUBFIELD_CHANGED(pLastPrefs, pPrefs, orientationoffset, roll) ||
      PB_FIELD_CHANGED(pLastPrefs, pPrefs, platpositionoffset) ||
      PB_FIELD_CHANGED(pLastPrefs, pPrefs, scale))
    {
      notifyBeamsOfNewHostSize_(platform);
    }

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

void ScenarioManager::notifyBeamsOfNewHostSize_(PlatformNode* platform)
{
  SAFETRYBEGIN;
  std::pair< HosterTable::const_iterator, HosterTable::const_iterator > range =
    hosterTable_.equal_range(platform->getId());

  for (HosterTable::const_iterator i = range.first; i != range.second; ++i)
  {
    BeamNode* beam = find<BeamNode>(i->second);
    if (beam)
      beam->setHostMissileOffset(platform->getFrontOffset());
  }
  SAFETRYEND("notifying beams of new host size");
}

EntityNode* ScenarioManager::find(const simData::ObjectId& id) const
{
  SAFETRYBEGIN;
  EntityRepo::const_iterator i = entities_.find(id);
  return i != entities_.end() ? static_cast<EntityNode*>(i->second->getNode()) : NULL;
  SAFETRYEND(std::string(osgEarth::Stringify() << "finding entity ID " << id));
  return NULL;
}

const EntityNode* ScenarioManager::getHostPlatform(const EntityNode* entity) const
{
  if (entity == NULL)
    return NULL;

  simData::ObjectId hostId;
  while (entity->getHostId(hostId))
  {
    entity = find(hostId);
    if (entity == NULL)
    {
      // An orphan entity without a host platform
      assert(false);
      return NULL;
    }
  }

  return entity;
}

namespace {

#ifdef DEBUG
/** Visitor that, in debug mode, asserts that the overhead mode hint is set to a certain value */
class AssertOverheadModeHint : public osg::NodeVisitor
{
public:
  AssertOverheadModeHint(bool expectedHint, TraversalMode tm=osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
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
    return NULL;
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
    b = osg::Vec4d(x, y,  1.0, 1.0) * toModel;
  }

  osg::Vec3d beg(a.x() / a.w(), a.y() / a.w(), a.z() / a.w());
  osg::Vec3d end(b.x() / b.w(), b.y() / b.w(), b.z() / b.w());

#ifdef DEBUG
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

  // configure the line segment intersector
  osgEarth::DPLineSegmentIntersector* lsi = new osgEarth::DPLineSegmentIntersector(beg, end);
  osgUtil::IntersectionVisitor iv(lsi);
  iv.setTraversalMask(typeMask);
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
    for (osgEarth::DPLineSegmentIntersector::Intersections::iterator i = lsi->getIntersections().begin();
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

  return NULL;
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
    tool->onInstall(this);
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
    tool->onUninstall(this);
    scenarioTools_.erase(i);
  }
  SAFETRYEND("removing scenario tool");
}

void ScenarioManager::getTools(std::vector<ScenarioTool*>& tools) const
{
  SAFETRYBEGIN;
  std::copy(scenarioTools_.begin(), scenarioTools_.end(), std::back_inserter(tools));
  SAFETRYEND("retrieving scenario tools")
}

void ScenarioManager::notifyToolsOfAdd_(EntityNode* node)
{
  for (ScenarioToolVector::iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    i->get()->onEntityAdd(this, node);
  }
}

void ScenarioManager::notifyToolsOfRemove_(EntityNode* node)
{
  for (ScenarioToolVector::iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    i->get()->onEntityRemove(this, node);
  }
}

void ScenarioManager::update(simData::DataStore* ds, bool force)
{
  EntityVector updates;

  SAFETRYBEGIN;
  for (EntityRepo::iterator i = entities_.begin(); i != entities_.end(); ++i)
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

  for (ScenarioToolVector::const_iterator i = scenarioTools_.begin(); i != scenarioTools_.end(); ++i)
  {
    SAFETRYBEGIN;
    ScenarioTool* tool = i->get();
    if (updates.size() > 0 || tool->isDirty())
    {
      tool->onUpdate(this, ds->updateTime(), updates);
      needsRedraw = true;
    }
    SAFETRYEND("updating scenario tools");
  }

  if (needsRedraw)
  {
    SAFETRYBEGIN;
    // "dirty" the scene graph
    ViewVisitor<RequestRedraw> visitor;
    this->accept(visitor);
    SAFETRYEND("requesting redraw on scenario");
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
  osg::Group* result = NULL;
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
