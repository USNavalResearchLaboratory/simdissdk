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
      node_(NULL)
  {
  }

  void apply(osg::Node& node)
  {
    if (node.getName() == searchFor_)
      node_ = &node;
    else
      traverse(node);
  }

  /** Retrieves the node from visitation, possibly NULL */
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
  explicit TankNode()
    : gun_(NULL),
      turret_(NULL)
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
    if (gun_ != NULL)
      gun_->updateCurrentHPR(osg::Vec3f(0.0f, simCore::DEG2RAD * pitchDeg, 0.0f));
  }
  float gunPitch() const
  {
    if (gun_ != NULL)
      return simCore::RAD2DEG * gun_->getCurrentHPR().y();
    return 0.0f;
  }
  float gunMinimumPitch() const
  {
    if (gun_ != NULL)
      return simCore::RAD2DEG * gun_->getMinHPR().y();
    return 0.0f;
  }
  float gunMaximumPitch() const
  {
    if (gun_ != NULL)
      return simCore::RAD2DEG * gun_->getMaxHPR().y();
    return 0.0f;
  }

  void setTurretYaw(double yawDeg)
  {
    if (turret_ != NULL)
      turret_->updateCurrentHPR(osg::Vec3f(simCore::DEG2RAD * yawDeg, 0.0f, 0.0f));
  }
  float turretYaw() const
  {
    if (turret_ != NULL)
      return simCore::RAD2DEG * turret_->getCurrentHPR().x();
    return 0.0f;
  }
  float turretMinimumYaw() const
  {
    if (turret_ != NULL)
      return simCore::RAD2DEG * turret_->getMinHPR().x();
    return 0.0f;
  }
  float turretMaximumYaw() const
  {
    if (turret_ != NULL)
      return simCore::RAD2DEG * turret_->getMaxHPR().x();
    return 0.0f;
  }

private:
  osg::observer_ptr<osgSim::DOFTransform> gun_;
  osg::observer_ptr<osgSim::DOFTransform> turret_;
};

//----------------------------------------------------------------------------

/** Application object that syncs the UI with the Tank data after async model load */
struct App : public osg::Node
{
  TankNode tank_;
  osg::observer_ptr<osg::Node> node_;
  osg::ref_ptr<HSliderControl> turretSlider_;
  osg::ref_ptr<HSliderControl> gunSlider_;

  void refresh();

  App(simVis::EntityNode* node) :
    node_(node)
  {
    setNumChildrenRequiringUpdateTraversal(1u);
  }

  void traverse(osg::NodeVisitor& nv)
  {
    if (nv.getVisitorType() == nv.UPDATE_VISITOR && tank_.needsSetup())
    {
      osg::ref_ptr<osg::Node> node;
      if (node_.lock(node))
      {
        tank_.setup(node.get());
        syncUI();
      }
    }
    osg::Node::traverse(nv);
  }

  void syncUI()
  {
    turretSlider_->setMin(tank_.turretMinimumYaw());
    turretSlider_->setMax(tank_.turretMaximumYaw());
    turretSlider_->setValue(tank_.turretYaw());
    
    gunSlider_->setMin(tank_.gunMinimumPitch());
    gunSlider_->setMax(tank_.gunMaximumPitch());
    gunSlider_->setValue(tank_.gunPitch());
  }
};

//----------------------------------------------------------------------------

/** Control handler to change the gun pitch */
class TankGunPitchChange : public ControlEventHandler
{
public:
  explicit TankGunPitchChange(TankNode* tank)
    : tank_(tank)
  {
  }
  void onValueChanged(Control*, float value)
  {
    tank_->setGunPitch(value);
  }
private:
  TankNode* tank_;
};

//----------------------------------------------------------------------------

/** Control handler to change the turret yaw */
class TankTurretYawChange : public ControlEventHandler
{
public:
  explicit TankTurretYawChange(TankNode* tank)
    : tank_(tank)
  {
  }
  void onValueChanged(Control*, float value)
  {
    tank_->setTurretYaw(value);
  }
private:
  TankNode* tank_;
};

//----------------------------------------------------------------------------

/** Control handler to update the label to the current value. */
class SetLabelValue : public ControlEventHandler
{
public:
  explicit SetLabelValue(LabelControl* label)
    : label_(label)
  {
  }
  void onValueChanged(Control*, float value)
  {
    std::stringstream ss;
    ss << value;
    label_->setText(ss.str());
  }
private:
  osg::ref_ptr<LabelControl> label_;
};

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

  // Set up the tank to manipulate the articulations
  App* app = new App(node2.get());
  viewer->getMainView()->getSceneData()->asGroup()->addChild(app);

  // Set up a grid for animation controls
  osg::ref_ptr<Grid> grid = new Grid;
  grid->setChildSpacing(5);

  // Turret widgets
  grid->setControl(0, 0, new LabelControl("Turret:"));
  app->turretSlider_ = grid->setControl(1, 0, new HSliderControl());
  app->turretSlider_->setSize(300, 35);
  app->turretSlider_->addEventHandler(new TankTurretYawChange(&app->tank_));
  app->turretSlider_->addEventHandler(new SetLabelValue(grid->setControl(2, 0, new LabelControl("0.0"))));

  // Gun widgets
  grid->setControl(0, 1, new LabelControl("Gun:"));
  app->gunSlider_ = grid->setControl(1, 1, new HSliderControl());
  app->gunSlider_->setSize(300, 35);
  app->gunSlider_->addEventHandler(new TankGunPitchChange(&app->tank_));
  app->gunSlider_->addEventHandler(new SetLabelValue(grid->setControl(2, 1, new LabelControl("0.0"))));

  app->syncUI();

  // Add grid to the main view
  viewer->getMainView()->addOverlayControl(grid.get());

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
