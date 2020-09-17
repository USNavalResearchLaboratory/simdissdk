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
#ifndef SIMVIS_SCENARIO_H
#define SIMVIS_SCENARIO_H

#include <limits>
#include <string>
#include <map>
#include <set>
#include "osg/Group"
#include "osg/ref_ptr"
#include "osg/View"
#include "osgEarth/CullingUtils"
#include "osgEarth/Revisioning"
#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/Types.h"
#include "simVis/RFProp/RFPropagationManager.h"

namespace osgEarth { class MapNode; }
namespace simCore { class Clock; }
namespace simData { class DataStore; }

namespace simVis
{

class BeamNode;
class CoordSurfaceClamping;
class GateNode;
class CustomRenderingNode;
class LabelContentManager;
class LaserNode;
class LobGroupNode;
class Locator;
class PlatformNode;
class PlatformTspiFilterManager;
class ProjectorManager;
class ProjectorNode;
class ScenarioTool;

//----------------------------------------------------------------------------
/// Interface for an object that can create a new Locator
class LocatorFactory
{
public:
  virtual ~LocatorFactory() {}

  /// create a new locator
  virtual SDK_DEPRECATE(Locator* createLocator() const, "LocatorFactory is deprecated.") = 0;

  /// create a new platform locator
  virtual SDK_DEPRECATE(Locator* createEciLocator() const, "LocatorFactory is deprecated.") = 0;
};

//----------------------------------------------------------------------------

/**
* Manages all scenario objects (platforms, beams, gates, etc) and their
* visualization within the scene
*/
class SDKVIS_EXPORT ScenarioManager : public osgEarth::LODScaleGroup
{
  friend class SceneManager;
public:

  /**
   * Binds this scenario manager to a DataStore.
   * @param[in ] dataStore Datastore to bind.
   */
  void bind(simData::DataStore* dataStore);

  /**
   * Unbinds this scenario manager from a DataStore.
   * @param[in ] dataStore Datastore to unbind.
   * @param[in ] clearAll  Whether to remove all entities that originated from this DataStore (default=false)
   */
  void unbind(simData::DataStore* dataStore, bool clearAll = false);

  /**
   * Sets the manager for label content for all entity types
   * @param[in ] manager Manager for the content
   */
  void setLabelContentManager(LabelContentManager* manager);

  /**
  * Sets the manager for the RF Propagation
  * @param[in ] manager Manager for the RF Propagation
  */
  void setRFPropagationManager(simRF::RFPropagationManagerPtr manager);

  /** Returns the RFPropagationManager */
  simRF::RFPropagationManagerPtr rfPropagationManager() const;

  /**
  * Add a new platform to the scenario and bind it to the data store.
  * @param props     Platform initialization properties
  * @param dataStore Datastore to which to bind the new node
  * @return          New platform node
  */
  PlatformNode* addPlatform(
    const simData::PlatformProperties& props,
    simData::DataStore&                dataStore);

  /**
  * Add a new beam to the scenario, bind it to the data store, and associate it
  * with its host platform (it if exists).
  * @param props     Beam initialization properties
  * @param dataStore Datastore to which to bind this beam
  * @return          New beam node
  */
  BeamNode* addBeam(
    const simData::BeamProperties& props,
    simData::DataStore&            dataStore);

  /**
  * Add a new gate to the scenario, bind it to the data store, and associate it
  * with its host platform (it if exists).
  * @param props     Gate initialization properties
  * @param dataStore Datastore to which to bind this gate
  * @return          New gate node
  */
  GateNode* addGate(
    const simData::GateProperties& props,
    simData::DataStore&            dataStore);

  /**
  * Add a new projector to the scenario, binds it to the data store.
  * @return New projector node.
  */
  ProjectorNode* addProjector(
    const simData::ProjectorProperties& props,
    simData::DataStore&                 dataStore);

  /**
  * Add a new laser to the scenario, binds it to the data store.
  * @param props     Laser initialization properties
  * @param dataStore Datastore to which to bind this laser
  * @return New laser node.
  */
  LaserNode* addLaser(
    const simData::LaserProperties& props,
    simData::DataStore&             dataStore);

  /**
  * Add a new LobGroup to the scenario, binds it to the data store.
  * @param props LOB initialization properties
  * @param dataStore Datastore to which to bind this LOB group
  * @return New LOB node.
  */
  LobGroupNode* addLobGroup(
    const simData::LobGroupProperties& props,
    simData::DataStore&                dataStore);

  CustomRenderingNode* addCustomRendering(
    const simData::CustomRenderingProperties& props,
    simData::DataStore&            dataStore);

  /**
  * Set new preferences for a platform.
  * @param id    ID of the platform
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setPlatformPrefs(
    simData::ObjectId             id,
    const simData::PlatformPrefs& prefs);

  /**
  * Set new preferences for a beam.
  * @param id    Object id of the beam
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setBeamPrefs(
    simData::ObjectId         id,
    const simData::BeamPrefs& prefs);

  /**
  * Set new preferences for a gate.
  * @param id    Object id of the gate
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setGatePrefs(
    simData::ObjectId          id,
    const simData::GatePrefs& prefs);

  /**
  * Set new preferences for a projector.
  * @param id    Object id of the projector
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setProjectorPrefs(
    simData::ObjectId              id,
    const simData::ProjectorPrefs& prefs);

  /**
  * Set new preferences for a laser.
  * @param id    Object id of the laser
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setLaserPrefs(
    simData::ObjectId          id,
    const simData::LaserPrefs& prefs);

  /**
  * Set new preferences for a LobGroup.
  * @param id    Object id
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setLobGroupPrefs(
    simData::ObjectId          id,
    const simData::LobGroupPrefs& prefs);

  /**
  * Set new preferences for a LobGroup.
  * @param id    Object id
  * @param prefs New preferences to set
  * @return      True upon success; false if the object could not be found.
  */
  bool setCustomRenderingPrefs(
    simData::ObjectId          id,
    const simData::CustomRenderingPrefs& prefs);

  /**
  * Find an entity by its unique ID.
  * @param id Unique entity ID
  * @return   Entity node, or nullptr if not found
  */
  EntityNode* find(const simData::ObjectId &id) const;

  /**
  * Returns the host platform for the given entity
  * If entity is a platform it will return itself
  * @param entity Need its host platform
  * @return the host platform for the given entity, or nullptr if not found (orphan)
  */
  const EntityNode* getHostPlatform(const EntityNode* entity) const;

  /**
  * Find a node and casts it to the requested type (convenience function)
  * @param id Unique entity ID
  * @return   Entity node, cast to the requested type, or nullptr if not found.
  */
  template<typename T>
  T* find(const simData::ObjectId &id) const
  {
    return dynamic_cast<T*>(find(id));
  }

  /**
  * Find an entity by intersecting the scene under the provided mouse coordinates.
  * @param view     View within to search
  * @param x        X mouse coordinate
  * @param y        Y mouse coordinate
  * @param typeMask Traversal mask of node type to find, or ~0 to find anything
  * @return         Entity node, or nullptr if nothing was hit
  */
  EntityNode* find(osg::View *view, float x, float y, int typeMask = ~0) const;

  /// Convenience function - calls find(view,x,y,mask) and casts the result
  template<typename T>
  T* find(osg::View *view, float x, float y, int mask = ~0) const
  {
    return dynamic_cast<T*>(find(view, x, y, mask));
  }

  /**
  * Flush the entity data of the specified entity.  0 indicates flush all entities
  * @param[in ] flushedId
  */
  void flush(simData::ObjectId flushedId);

  /**
   * Remove entities from the scenario.
   * @param[in ] dataStore Remove entities that originated from this data store.
   *             Pass in nullptr to remove all entities regardless of origin.
   */
  void clearEntities(simData::DataStore* dataStore = nullptr);

  /**
   * Remove the entity referenced by 'id' from the entity list and from
   * the scene graph
   * @param[in ] id Entity ID
   */
  void removeEntity(simData::ObjectId id);

  /**
  * Adds a new scenario tool to the manager
  */
  void addTool(ScenarioTool* tool);

  /**
  * Removes a scenario tool from the manager
  */
  void removeTool(ScenarioTool* tool);

  /**
   * Retrieve a list of all tools
   */
  void getTools(std::vector< osg::ref_ptr<ScenarioTool> >& tools) const;

  /**
  * Removes all scenario tools from the manager
  */
  void removeAllTools_();

  /**
  * Accesses the DataStore adapter bound to this scenario.
  */
  const ScenarioDataStoreAdapter& getDataStoreAdapter() const { return dataStoreAdapter_; }

  /**
  * Finds a list of object IDs that point to the input object ID
  * as their host.
  */
  void getObjectsHostedBy(
    const simData::ObjectId&     hostId,
    std::set<simData::ObjectId>& output) const;

  /**
   * Gets a collection of all the entities currently active
   */
  void getAllEntities(EntityVector& out_vector) const;

  /**
   * Gets or creates a new attach point for adding data to the scene graph, not subject to horizon culling
   * @param name Name of the attach point
   * @return     New osg group
   */
  osg::Group* getOrCreateAttachPoint(const std::string& name);

  /** Called internally when the platform size changes, to notify the beam so it can adjust to actual/visual size */
  void notifyBeamsOfNewHostSize(const PlatformNode& platform) const;

  /** Set whether to use the most precise elevation sampling method for platform clamping.  Using max precision may cause performance hits. */
  void setUseMaxElevClampPrec(bool useMaxPrec);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "ScenarioManager"; }

public: // package protected

  /** Creates a new ScenarioManager with the given projector manager */
  explicit ScenarioManager(ProjectorManager* projMan);

  /**
   * Creates a new ScenarioManager with the given locator factory and projector manager
   * @deprecated
   */
  SDK_DEPRECATE(ScenarioManager(LocatorFactory* factory, ProjectorManager* projMan), "Method will be removed in a future SDK release");

  /**
  * Check for scenario entity updates and applies them to the corresponding
  * scene graph nodes.
  * @param[in ] ds    DataStore driving the update
  * @param[in ] force Force an update even if the data store says nothing has changed
  */
  void update(simData::DataStore* ds, bool force = false);

  /**
  * Notify all entities of a change in a Clock Mode.
  * @param[in ] clock Clock to propagate to scenario objects.
  */
  void notifyOfClockChange(const simCore::Clock* clock);

  /**
   * Gets map information
   */
  osgEarth::MapNode* mapNode() const { return mapNode_.get(); }

  /**
   * Sets map information
   */
  void setMapNode(osgEarth::MapNode* map);

protected:
  /// osg::Referenced-derived
  virtual ~ScenarioManager();

protected:
  class AboveSurfaceClamping;
  class EntityRecord;
  class ScenarioLosCreator;
  class SimpleEntityGraph;
  class SurfaceClamping;

  /** Provides capability to process platform TSPI points */
  PlatformTspiFilterManager*   platformTspiFilterManager_;
  /** PlatformTspiFilter that provides surface clamping capabilities */
  SurfaceClamping*             surfaceClamping_;
  /** PlatformTspiFilter that provides surface limiting capabilities */
  AboveSurfaceClamping*        aboveSurfaceClamping_;
  /** Helps clamping for LOBs to map surface */
  CoordSurfaceClamping*        lobSurfaceClamping_;
  /** Root node for the scenario */
  osg::ref_ptr<osg::Group>     root_;
  /** Strategy for grouping up entities into the scene graph */
  osg::ref_ptr<SimpleEntityGraph> entityGraph_;
  /** Holds a map of all named attachment points added through getOrCreateAttachPoint(). */
  std::map<std::string, osg::observer_ptr<osg::Group> > customAttachPoints_;

  /** Observer to the current map */
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  /** Responsible for managing Projector entities */
  ProjectorManager*         projectorManager_;
  /** Responsible for linking a data store to this instance */
  ScenarioDataStoreAdapter  dataStoreAdapter_;
  /** Manages callbacks that are responsible for creating entity labels */
  osg::ref_ptr<LabelContentManager> labelContentManager_;
  /** Manages RF Propagation data */
  simRF::RFPropagationManagerPtr rfManager_;
  /** Responsible for creation of LOS nodes as needed by platforms */
  ScenarioLosCreator* losCreator_;

  /** Association between the EntityNode, the data store, and the entity's update slice */
  class EntityRecord : public osg::Group
  {
  public:
    /** Constructs a new entity record */
    EntityRecord(EntityNode* node, const simData::DataSliceBase* updateSlice, simData::DataStore* dataStore);

    /** Retrieves the entity node as an osg::Node, from GeoObject interface */
    virtual osg::Node* getNode() const;
    /** Retrieves the entity node, upcasted to entity node */
    EntityNode* getEntityNode() const;

    /** Returns node's LLA position, from GeoObject interface */
    virtual bool getLocation(osg::Vec3d& output) const;

    /** Returns true if the data store passed in is the same as the entity's data store */
    bool dataStoreMatches(const simData::DataStore* dataStore) const;
    /** Updates the entity from the data store.  Returns true if update was applied, false otherwise */
    bool updateFromDataStore(bool force) const;

  private:
    /** Node in scene graph representing entity */
    osg::ref_ptr<EntityNode> node_;

    /** Const pointer to the entity's data update slice */
    const simData::DataSliceBase* updateSlice_;
    /** Convenience pointer to the data store */
    simData::DataStore* dataStore_;
  };

  /** Typedef to map entity IDs to EntityRecord structs */
  typedef std::map< simData::ObjectId, osg::ref_ptr<EntityRecord> > EntityRepo;
  /** List of all scene graph entities to their ID */
  EntityRepo entities_;

  /** table that maps hoster ID's to hostee ID's */
  typedef std::multimap< simData::ObjectId, simData::ObjectId > HosterTable;
  /** Maps the hoster to the hostee, for hosted entity types */
  HosterTable hosterTable_;

  /** Maintains a list of scenario tools, like Range Tool */
  ScenarioToolVector scenarioTools_;
  /** Currently unused revision */
  osgEarth::Revision scenarioToolRev_;

  /// informs the scenario tools of an entity addition
  void notifyToolsOfAdd_(EntityNode* node);
  /// informs the scenario tools of an entity removal
  void notifyToolsOfRemove_(EntityNode* node);
  /// locator that tracks earth rotation linked to sim time
  osg::ref_ptr<Locator> scenarioEciLocator_;

private:
  /// Copy constructor, not implemented or available.
  ScenarioManager(const ScenarioManager&);
};

} // namespace simVis

#endif // SIMVIS_SCENARIO_H
