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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_TOOL_H
#define SIMVIS_TOOL_H

#include "osg/Node"
#include "osgEarth/Revisioning"
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

namespace simCore { class TimeStamp; }
namespace simVis
{
  class ScenarioManager;
  class EntityNode;

  /**
  * Generic tool interface.
  */
  class /* SDKVIS_EXPORT */ Tool : public osg::Referenced, public osgEarth::DirtyNotifier
  {
  public:
    /// get the root of the visualization
    virtual osg::Node* getNode() const =0;

  protected:
    /// osg::Referenced-derived
    virtual ~Tool() {}
  };


  /**
  * Interface for a tool that you can attach to the ScenarioManager.
  * A ScenarioTool is intended to add Scenario-related visualizations to the scenegraph;
  * ScenarioManager will add/remove the ScenarioTool's getNode() to the scenegraph on install/uninstall
  * A ScenarioTool should not expect to do anything unless installed onto a scenario;
  * A ScenarioTool receives its updates from the ScenarioManager; an uninstalled ScenarioTool will not receive updates.
  */
  class /* SDKVIS_EXPORT */ ScenarioTool : public Tool
  {
  protected:
    ScenarioTool() { }

    /// osg::Referenced-derived
    virtual ~ScenarioTool() {}

  public:

    /**
    * Called when this tool is installed onto the scenario.
    * Tool should initialize scenario-related data and prepare for updates.
    * Tool's root node will be added to the scenegraph immediately after this call.
    * @param scenario the scenarioManager that is installing this tool
    */
    virtual void onInstall(const ScenarioManager& scenario) = 0;

    /**
    * Called when this tool is removed from the scenario.
    * Tool should clear anything related to the scenario, prepare for deletion or installation of another scenario.
    * Tool's root node has already been removed from the scenegraph before this call
    * @param scenario the scenarioManager that is uninstalling this tool
    */
    virtual void onUninstall(const ScenarioManager& scenario) = 0;

    /**
    * Called when a new entity is added.
    * @param scenario the scenarioManager that is managing this tool
    * @param entity the entity that has been added to the scenario
    */
    virtual void onEntityAdd(const ScenarioManager& scenario, EntityNode* entity) { }

    /**
    * Called when an entity is removed.
    * @param scenario the scenarioManager that is managing this tool
    * @param entity the entity that has been removed from the scenario
    */
    virtual void onEntityRemove(const ScenarioManager& scenario, EntityNode* entity) { }

    /**
    * Called when scenario time changes.
    * @param scenario the scenarioManager that is managing this tool.
    * @param timeStamp the update time.
    * @param updates the changes to the scenario's entities for the given update time
    */
    virtual void onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates) { }

    /**
    * Called when scenario or an entity is about to be flushed.
    * @param scenario  the scenarioManager that is managing this tool.
    * @param flushedId  0 if scenario flush, otherwise the ID of the entity that will be flushed
    */
    virtual void onFlush(const ScenarioManager& scenario, simData::ObjectId flushedId) { }
  };

} // namespace simVis

#endif // SIMVIS_CAMERA_H

