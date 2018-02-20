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
#ifndef SIMVIS_SCENE_MANAGER_H
#define SIMVIS_SCENE_MANAGER_H

#include "osg/ref_ptr"
#include "osg/Group"
#include "osgEarth/DrapeableNode"
#include "osgEarth/ImageLayer"
#include "osgEarth/MapNode"
#include "osgEarthUtil/Sky"
#include "osgEarthUtil/Ocean"
#include "simCore/Common/Common.h"
#include "simVis/Locator.h"
#include "simVis/Types.h"

namespace osgEarth { namespace Drivers
{
  namespace MPTerrainEngine { class MPTerrainEngineOptions; }
  namespace RexTerrainEngine { class RexTerrainEngineOptions; }
}}

namespace simVis
{
  class ProjectorManager;
  class ScenarioDisplayHints;
  class ScenarioManager;

  /**
   * @anchor SceneManagerLayout
   * The top-level content node for a scene. There is one scene per managed view.
   *
   * The scene manager is initialized using the following layout:
   *
   * <pre>
   *  this
   *    +-skyNode
   *       +-mapContainer
   *          +-mapNode
   *       +-scenarioManager
   *       +-projectorManager
   *       +-oceanNode
   * </pre>
   */
  class SDKVIS_EXPORT SceneManager : public osg::Group,
                                     public simVis::LocatorFactory
  {
  public:
    /**
     * Constructs a new scene manager.
     * You typically do not have to create a scene manager directly. The Viewer
     * will create one for you.
     */
    SceneManager();

    /**
    * The scene graph node that rendering the earth.
    * @return the MapNode
    */
    virtual osgEarth::MapNode* getMapNode() { return mapNode_.get(); }

    /**
     * Sets a new MapNode.
     * @param mapNode New map node to install in the scene manager
     */
    virtual void setMapNode(osgEarth::MapNode* mapNode);

    /**
    * The map displayed in this scene.
    * @return the Map
    */
    osgEarth::Map* getMap() { return mapNode_.valid() ? mapNode_->getMap() : NULL; }

    /**
    * Sets the Map. This method actually will copy the layers from the provided
    * map into the current map node (replacing any preexisting layers).
    * @param[in ] map the new map
    */
    void setMap(osgEarth::Map* map);

    /**
    * Interface to the scenario manager.
    * @return Scenario API
    */
    ScenarioManager* getScenario() { return scenarioManager_.get(); }

    /**
    * The scene graph node that renders the sky
    * @return osgEarth::Util::SkyNode*
    */
    osgEarth::Util::SkyNode* getSkyNode() { return skyNode_.get(); }

    /**
    * The scene graph node that renders the ocean surface
    * @return osgEarth::Drivers::OceanSurfaceNode*
    */
    osgEarth::Util::OceanNode* getOceanNode() { return oceanNode_.get(); }

    /**
    * Gets the node to which a camera manipulator should attach.
    * @return a Node
    */
    osg::Node* getManipulatorAttachPoint() const;

    /**
    * Gets or creates a new attach point for adding data to the scene graph
    * @param name Name of the attach point
    * @return     New osg group
    */
    osg::Group* getOrCreateAttachPoint(const std::string& name) const;

#ifdef USE_DEPRECATED_SIMDISSDK_API
    /**
    * Applies display hints to assist with large-scale visualizations
    * @param hints Display hints
    * @deprecated Use ScenarioManager::setEntityGraphStrategy(new ScenarioManager::GeoGraphEntityGraph(hints)) instead.
    */
    void setScenarioDisplayHints(const ScenarioDisplayHints& hints);
#endif /* USE_DEPRECATED_SIMDISSDK_API */

    /** Turns scenario draping on and off, for use with overhead mode. */
    void setScenarioDraping(bool value);

    /**
    * Set the SkyNode object for the scene.  Will automatically turn on or off
    * the depth writer node (setDepthWriterNodeMask()) based on whether the Sky
    * node is detected to be SilverLining.
    * @param skyNode the new sky node
    */
    void setSkyNode(osgEarth::Util::SkyNode* skyNode);

    /**
    * Set the OceanSurfaceNode object for the scene
    * @param oceanNode the new ocean surface node
    */
    void setOceanNode(osgEarth::Util::OceanNode* oceanNode);

    /** remove the current ocean node */
    void removeOceanNode();

    /** Changes the underlying globe color for when no image layers are shown */
    void setGlobeColor(const simVis::Color& color);

    /**
     * Toggles the depth writer, which is a second pass that will render the scene without
     * color, but with depth enabled.  This is useful for ending with a reasonable depth
     * buffer, even when transparent objects are in the scene that don't write to the
     * depth buffer like beams and gates.  By default, this is off for performance unless
     * setSkyNode() detects that SilverLining is the sky node being used.  If you want
     * to control this, call this method after your call to setSkyNode().
     * @param nodeMask Node mask for the second rendering of the scene that forces depth
     *   writes enabled.
     */
    void setDepthWriterNodeMask(osg::Node::NodeMask nodeMask);

    /** Returns true if there is an engine driver problem */
    bool hasEngineDriverProblem() const;

    /** Fills out an MPTerrainEngineOptions with good default values */
    static void initializeTerrainOptions(osgEarth::Drivers::MPTerrainEngine::MPTerrainEngineOptions& options);

    /** Fills out an RexTerrainEngineOptions with good default values */
    static void initializeTerrainOptions(osgEarth::Drivers::RexTerrainEngine::RexTerrainEngineOptions& options);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "SceneManager"; }

  public: // simVis::LocatorFactory

    // internal - override
    Locator* createLocator() const;
    // internal - override
    CachingLocator* createCachingLocator() const;

  protected:
    /// osg::Referenced-derived
    virtual ~SceneManager();

    /// Override and protect addChild().  Most children should be going to the scenario.
    virtual bool addChild(osg::Node *child);

  private:
    /** Copy constructor, not implemented or available. */
    SceneManager(const SceneManager&);

    /** Returns true if the sky node is SilverLining. */
    bool isSilverLining_(const osgEarth::Util::SkyNode* skyNode) const;

    /** Contains the map node, child of the sky node.  See also @ref SceneManagerLayout */
    osg::ref_ptr<osg::Group> mapContainer_;
    /** Contains a second reference to scenario manager for rendering depth. */
    osg::ref_ptr<osg::Group> depthRenderContainer_;
    /** Child of the map container, holds the map.  See also @ref SceneManagerLayout */
    osg::ref_ptr<osgEarth::MapNode> mapNode_;
    /** Contains the scenario entities and tools, child of the sky node.  See also @ref SceneManagerLayout */
    osg::ref_ptr<ScenarioManager> scenarioManager_;
    /** Contains the scene projectors, child of the sky node.  See also @ref SceneManagerLayout */
    osg::ref_ptr<ProjectorManager> projectorManager_;
    /** Child of the top level root, contains most of the scene because it applies various shading to scene elements.  See also @ref SceneManagerLayout */
    osg::ref_ptr<osgEarth::Util::SkyNode> skyNode_;
    /** Contains ocean surface graphics, child of the sky node.  See also @ref SceneManagerLayout */
    osg::ref_ptr<osgEarth::Util::OceanNode> oceanNode_;
    /** Uniform shader variable that changes the globe color where there is no opaque image layer */
    osg::ref_ptr<osg::Uniform> globeColor_;
    /** Parent node that permits draping of geometry */
    osg::ref_ptr<osgEarth::DrapeableNode> drapeableNode_;
    /** Flags true if there are problems starting the map engine */
    bool hasEngineDriverProblem_;

    /** Applies the display settings from the sourceLayer to the destLayer provided */
    void applyImageLayerDisplaySettings_(const osgEarth::ImageLayer& sourceLayer, osgEarth::ImageLayer* destLayer) const;
    /** Creates map container and sets initial map node*/
    void init_();
    /** Detects engine driver problems and sets internal state appropriately */
    void detectTerrainEngineDriverProblems_();
    /** Returns the unique hash identifying the layer. Taken from osgEarth::TerrainLayer::setCache method */
    std::string getLayerHash_(osgEarth::TerrainLayer* layer) const;
    /** Replace image layers in currentMap with image layers in newMap, unless the layer already exists in currentMap. Removes old layers not in newMap */
    void updateImageLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap);
    /** Replace elevation layers in currentMap with image layers in newMap, unless the layer already exists in currentMap. Removes old layers not in newMap */
    void updateElevationLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap);
    /** Replace model layers in currentMap with model layers in newMap */
    void updateModelLayers_(const osgEarth::Map& newMap, osgEarth::Map* currentMap);
  };

} // namespace simVis

#endif // SIMVIS_SCENE_MANAGER_H
