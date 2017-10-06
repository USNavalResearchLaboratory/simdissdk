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
#ifndef SIMVIS_TOOL_H
#define SIMVIS_TOOL_H

#include <vector>
#include "osg/Node"
#include "osgEarth/Revisioning"
#include "simCore/Common/Common.h"

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
  */
  class /* SDKVIS_EXPORT */ ScenarioTool : public Tool
  {
  protected:
    ScenarioTool() { }

    /// osg::Referenced-derived
    virtual ~ScenarioTool() {}

  public:

    /// called when this tool is installed onto the scenario.
    virtual void onInstall(ScenarioManager* scenario) { }

    /// called when this tool is removed from the scenario.
    virtual void onUninstall(ScenarioManager* scenario) { }

    /// called when a new entity is added
    virtual void onEntityAdd(ScenarioManager* scenario, EntityNode* entity) { }

    /// called when a new entity is removed
    virtual void onEntityRemove(ScenarioManager* scenario, EntityNode* entity) { }

    /// called when scenario time changes
    virtual void onUpdate(ScenarioManager* scenario, const simCore::TimeStamp& timeStamp, const std::vector<osg::ref_ptr<EntityNode> >& updates) { }
  };

  /** Vector of ScenarioTool ref_ptr */
  typedef std::vector< osg::ref_ptr<ScenarioTool> > ScenarioToolVector;

} // namespace simVis

#endif // SIMVIS_CAMERA_H

