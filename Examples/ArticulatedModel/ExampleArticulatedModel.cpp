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

/**
 * Articulated Model Example.
 *
 * Shows two tanks next to one another in scene, with Controls widgets to manipulate
 * the DOF nodes for the Turret and Gun.
 *
 * Note that changing the DOF affects the model itself, so changing one model changes
 * the articulation on both tanks.
 */

#include "osgSim/DOFTransform"
#include "osgEarth/NodeUtils"

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simData/MemoryDataStore.h"
#include "simVis/Registry.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------

// first line, describe the program
static const std::string s_title = "Articulated Model Example";
// From http://trac.openscenegraph.org/projects/osg//attachment/wiki/Support/Tutorials/NPS_Tutorials_src.rar
static const std::string s_modelName = EXAMPLE_TANK_ICON;

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, double lat, double lon, double alt)
{
  // create the platform:
  simData::ObjectId platformId;
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties *newProps = dataStore.addPlatform(&xaction);
    platformId = newProps->id();
    xaction.complete(&newProps);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->set_icon(s_modelName);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // now place it somewhere
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformUpdate* update = dataStore.addPlatformUpdate(platformId, &xaction);

    simCore::Coordinate lla(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat, simCore::DEG2RAD * lon, alt),
      simCore::Vec3(0, 0, 0));

    simCore::CoordinateConverter conv;
    simCore::Coordinate ecef;
    conv.convert(lla, ecef, simCore::COORD_SYS_ECEF);

    update->set_time(0.0);
    update->set_x(ecef.x());
    update->set_y(ecef.y());
    update->set_z(ecef.z());
    update->set_psi(ecef.psi());
    update->set_theta(ecef.theta());
    update->set_phi(ecef.phi());

    xaction.complete(&update);
  }

  return platformId;
}

//----------------------------------------------------------------------------

/** Visitor to find the first node with the given name */
class FindNodeByName : public osg::NodeVisitor
{
public:
  explicit FindNodeByName(const std::string& name)
    : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
      searchFor_(name),
      node_(nullptr)
  {
  }

  void apply(osg::Node& node)
  {
    if (node.getName() == searchFor_)
      node_ = &node;
    else
      traverse(node);
  }

  /** Retrieves the node from visitation, possibly nullptr */
  osg::Node* node() const
  {
    return node_.get();
  }

private:
  std::string searchFor_;
  osg::observer_ptr<osg::Node> node_;
};

//----------------------------------------------------------------------------

/** Encapsulates a tank model, providing functions to manipulate the turret and gun articulations */
class TankNode
{
public:
  TankNode()
    : gun_(nullptr),
      turret_(nullptr)
  {
  }
  virtual ~TankNode()
  {
  }

  bool needsSetup() const
  {
    return !gun_.valid() || !turret_.valid();
  }

  void setup(osg::Node* node)
  {
    if (!gun_.valid())
    {
      FindNodeByName gunFinder("gun");
      node->accept(gunFinder);
      gun_ = dynamic_cast<osgSim::DOFTransform*>(gunFinder.node());
    }

    if (!turret_.valid())
    {
      FindNodeByName turretFinder("turret");
      node->accept(turretFinder);
      turret_ = dynamic_cast<osgSim::DOFTransform*>(turretFinder.node());
    }
  }

  void setGunPitch(double pitchDeg)
  {
    if (gun_.valid())
      gun_->updateCurrentHPR(osg::Vec3f(0.0f, simCore::DEG2RAD * pitchDeg, 0.0f));
  }
  float gunPitch() const
  {
    if (gun_.valid())
      return simCore::RAD2DEG * gun_->getCurrentHPR().y();
    return 0.0f;
  }
  float gunMinimumPitch() const
  {
    if (gun_.valid())
      return simCore::RAD2DEG * gun_->getMinHPR().y();
    return 0.0f;
  }
  float gunMaximumPitch() const
  {
    if (gun_.valid())
      return simCore::RAD2DEG * gun_->getMaxHPR().y();
    return 0.0f;
  }

  void setTurretYaw(double yawDeg)
  {
    if (turret_.valid())
      turret_->updateCurrentHPR(osg::Vec3f(simCore::DEG2RAD * yawDeg, 0.0f, 0.0f));
  }
  float turretYaw() const
  {
    if (turret_.valid())
      return simCore::RAD2DEG * turret_->getCurrentHPR().x();
    return 0.0f;
  }
  float turretMinimumYaw() const
  {
    if (turret_.valid())
      return simCore::RAD2DEG * turret_->getMinHPR().x();
    return 0.0f;
  }
  float turretMaximumYaw() const
  {
    if (turret_.valid())
      return simCore::RAD2DEG * turret_->getMaxHPR().x();
    return 0.0f;
  }

private:
  osg::observer_ptr<osgSim::DOFTransform> gun_;
  osg::observer_ptr<osgSim::DOFTransform> turret_;
};

//----------------------------------------------------------------------------

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(simVis::EntityNode* node)
    : simExamples::SimExamplesGui("Articulated Model Example"),
    node_(node)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (tank_.needsSetup())
    {
      osg::ref_ptr<osg::Node> node;
      if (node_.lock(node))
        tank_.setup(node.get());
      return;
    }

    if (!isVisible())
      return;

    if (firstDraw_)
    {
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }

    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTable("Table", 2))
    {
      // Turret yaw
      float yaw = tank_.turretYaw();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Turret", &yaw, tank_.turretMinimumYaw(), tank_.turretMaximumYaw(), "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (yaw != tank_.turretYaw())
        tank_.setTurretYaw(yaw);

      // Gun pitch
      float pitch = tank_.gunPitch();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Gun", &pitch, tank_.gunMinimumPitch(), tank_.gunMaximumPitch(), "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (pitch != tank_.gunPitch())
        tank_.setGunPitch(pitch);

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  TankNode tank_;
  osg::observer_ptr<osg::Node> node_;
};

#endif

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // Set up the data:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  simData::ObjectId platform1 = addPlatform(dataStore, 21.3, -158, 0);
  simData::ObjectId platform2 = addPlatform(dataStore, 21.3001, -158.0001, 0);

  // Look up the platform models:
  osg::ref_ptr<simVis::EntityNode> node1 = scene->getScenario()->find(platform1);
  osg::ref_ptr<simVis::EntityNode> node2 = scene->getScenario()->find(platform2);

  // tick the sim
  dataStore.update(0);

  // zoom the camera
  viewer->getMainView()->tetherCamera(node1.get());
  viewer->getMainView()->setFocalOffsets(135, -8, 30);

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(node2.get()));
#endif

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
