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
 * Range Tool Example
 *
 * Demonstrate how to use the RangeTool API to draw range calculation graphics.
 */

// Version check against the SDK DLL
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/MemoryDataStore.h"

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/TrackHistory.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Popup.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// the RangeTool API.
#include "simVis/RangeTool.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth::Util::Controls;


//----------------------------------------------------------------------------

// global association to manipulate
static osg::ref_ptr<simVis::RangeTool::Association> s_association = NULL;

// list of calculations to cycle through
static simVis::RangeTool::CalculationVector s_lineCalcs;
static simVis::RangeTool::CalculationVector s_angleCalcs;

// index of currently visible calculation
static int s_lineCalcIndex = -1, s_angleCalcIndex = -1;


//----------------------------------------------------------------------------
/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "Range Tool Example";

/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl = NULL;

/// label displaying the name of the current calculation
static osg::ref_ptr<LabelControl> s_lineCalcLabel = NULL;
static osg::ref_ptr<LabelControl> s_angleCalcLabel = NULL;


// callback to toggle depth testing flag
struct ToggleDepthTest : public ControlEventHandler
{
  void onValueChanged(Control*, bool value)
  {
    for (simVis::RangeTool::CalculationVector::iterator c = s_lineCalcs.begin(); c != s_lineCalcs.end(); ++c)
    {
      simVis::RangeTool::GraphicVector& graphics = c->get()->graphics();
      for (simVis::RangeTool::GraphicVector::iterator g = graphics.begin(); g !=graphics.end(); ++g)
      {
        g->get()->graphicOptions().useDepthTest_ = value;
        g->get()->setDirty();
      }
    }
  }
};


Control* createHelp()
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);

  vbox->addControl(new LabelControl(s_title, 20, simVis::Color::Yellow));

  vbox->addControl(new LabelControl("1 : cycle through line calculations", 14, simVis::Color::Silver));
  s_lineCalcLabel = new LabelControl("currently viewing: none", 14, simVis::Color::Yellow);
  s_lineCalcLabel->setMargin(Gutter(0, 0, 10, 0));
  vbox->addControl(s_lineCalcLabel.get());

  vbox->addControl(new LabelControl("2 : cycle through angle calculations", 14, simVis::Color::Silver));
  s_angleCalcLabel = new LabelControl("currently viewing: none", 14, simVis::Color::Yellow);
  s_angleCalcLabel->setMargin(Gutter(0, 0, 10, 0));
  vbox->addControl(s_angleCalcLabel.get());

  vbox->addControl(new LabelControl("3 : zoom in", 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl("4 : rotate zoomed in view", 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl("5 : reset to main view", 14, simVis::Color::Silver));

  vbox->addControl(new LabelControl("t : toggle follow-platform", 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl("w,s : position offset north/south", 14, simVis::Color::Gray));
  vbox->addControl(new LabelControl("a,d : position offset west/east", 14, simVis::Color::Gray));
  vbox->addControl(new LabelControl("q,z : position offset up/down", 14, simVis::Color::Gray));
  vbox->addControl(new LabelControl("g : reset position offset", 14, simVis::Color::Gray));
  vbox->addControl(new LabelControl("Press \".\" to play/pause", 14, simVis::Color::Silver));

  osg::ref_ptr<HBox> hbox = vbox->addControl(new HBox());
  hbox->addControl(new CheckBoxControl(true, new ToggleDepthTest()));
  hbox->addControl(new LabelControl("depth testing"));

  s_helpControl = vbox;
  return vbox;
}

//----------------------------------------------------------------------------

void createLineCalculations(simVis::RangeTool::CalculationVector& calcs)
{
  osg::ref_ptr<simVis::RangeTool::Calculation> ground = new simVis::RangeTool::Calculation("Ground");
  ground->addGraphic(new simVis::RangeTool::GroundLineGraphic, true);
  ground->addGraphic(new simVis::RangeTool::BeginAltitudeLineGraphic);
  ground->addGraphic(new simVis::RangeTool::EndAltitudeLineGraphic);
  ground->setLabelMeasurement(new simVis::RangeTool::GroundDistanceMeasurement);
  ground->setLabelUnits(osgEarth::Units::KILOMETERS);
  calcs.push_back(ground);

  osg::ref_ptr<simVis::RangeTool::Calculation> slant = new simVis::RangeTool::Calculation("Slant");
  slant->addGraphic(new simVis::RangeTool::SlantLineGraphic, true);
  slant->setLabelMeasurement(new simVis::RangeTool::SlantDistanceMeasurement);
  calcs.push_back(slant);

  osg::ref_ptr<simVis::RangeTool::Calculation> alt = new simVis::RangeTool::Calculation("Altitude");
  alt->addGraphic(new simVis::RangeTool::BeginToEndLineAtBeginAltitudeGraphic, true);
  alt->addGraphic(new simVis::RangeTool::EndAltitudeLineToBeginAltitudeGraphic);
  alt->setLabelMeasurement(new simVis::RangeTool::AltitudeDeltaMeasurement);
  alt->textOptions().displayAssociationName_ = true;
  calcs.push_back(alt);

  osg::ref_ptr<simVis::RangeTool::Calculation> dr = new simVis::RangeTool::Calculation("Down Range");
  dr->addGraphic(new simVis::RangeTool::DownRangeLineGraphic, true);
  dr->addGraphic(new simVis::RangeTool::CrossRangeLineGraphic);
  dr->addGraphic(new simVis::RangeTool::DownRangeCrossRangeDownLineGraphic);
  dr->setLabelMeasurement(new simVis::RangeTool::DownRangeMeasurement);
  calcs.push_back(dr);

  osg::ref_ptr<simVis::RangeTool::Calculation> cr = new simVis::RangeTool::Calculation("Cross Range");
  cr->addGraphic(new simVis::RangeTool::DownRangeLineGraphic);
  cr->addGraphic(new simVis::RangeTool::CrossRangeLineGraphic, true);
  cr->addGraphic(new simVis::RangeTool::DownRangeCrossRangeDownLineGraphic);
  cr->setLabelMeasurement(new simVis::RangeTool::CrossRangeMeasurement);
  calcs.push_back(cr);

  osg::ref_ptr<simVis::RangeTool::Calculation> dv = new simVis::RangeTool::Calculation("Down Value");
  dv->addGraphic(new simVis::RangeTool::DownRangeLineGraphic);
  dv->addGraphic(new simVis::RangeTool::CrossRangeLineGraphic);
  dv->addGraphic(new simVis::RangeTool::DownRangeCrossRangeDownLineGraphic, true);
  dv->setLabelMeasurement(new simVis::RangeTool::DownRangeCrossRangeDownValueMeasurement);
  calcs.push_back(dv);

  osg::ref_ptr<simVis::RangeTool::Calculation> geodr = new simVis::RangeTool::Calculation("Geo Down Range");
  geodr->addGraphic(new simVis::RangeTool::DownRangeLineGraphic, true);
  geodr->addGraphic(new simVis::RangeTool::CrossRangeLineGraphic);
  geodr->setLabelMeasurement(new simVis::RangeTool::GeoDownRangeMeasurement);
  calcs.push_back(geodr);

  osg::ref_ptr<simVis::RangeTool::Calculation> geocr = new simVis::RangeTool::Calculation("Geo Cross Range");
  geocr->addGraphic(new simVis::RangeTool::DownRangeLineGraphic);
  geocr->addGraphic(new simVis::RangeTool::CrossRangeLineGraphic, true);
  geocr->setLabelMeasurement(new simVis::RangeTool::GeoCrossRangeMeasurement);
  calcs.push_back(geocr);

  osg::ref_ptr<simVis::RangeTool::Calculation> vc = new simVis::RangeTool::Calculation("Closing Velocity");
  vc->addGraphic(new simVis::RangeTool::SlantLineGraphic, true);
  vc->setLabelMeasurement(new simVis::RangeTool::ClosingVelocityMeasurement);
  calcs.push_back(vc);

  osg::ref_ptr<simVis::RangeTool::Calculation> vs = new simVis::RangeTool::Calculation("Separation Velocity");
  vs->addGraphic(new simVis::RangeTool::SlantLineGraphic, true);
  vs->setLabelMeasurement(new simVis::RangeTool::SeparationVelocityMeasurement);
  calcs.push_back(vs);

  osg::ref_ptr<simVis::RangeTool::Calculation> vd = new simVis::RangeTool::Calculation("Velocity Delta");
  vd->addGraphic(new simVis::RangeTool::SlantLineGraphic, true);
  vd->setLabelMeasurement(new simVis::RangeTool::VelocityDeltaMeasurement);
  calcs.push_back(vd);
}

void createAngleCalculations(simVis::RangeTool::CalculationVector& calcs)
{
  osg::ref_ptr<simVis::RangeTool::Calculation> trueAz = new simVis::RangeTool::Calculation("True Azimuth");
  trueAz->addGraphic(new simVis::RangeTool::TrueAzimuthPieSliceGraphic, true);
  trueAz->setLabelMeasurement(new simVis::RangeTool::TrueAzimuthMeasurement);
  trueAz->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(trueAz);

  osg::ref_ptr<simVis::RangeTool::Calculation> trueEl = new simVis::RangeTool::Calculation("True Elevation");
  trueEl->addGraphic(new simVis::RangeTool::TrueElevationPieSliceGraphic, true);
  trueEl->setLabelMeasurement(new simVis::RangeTool::TrueElevationMeasurement);
  trueEl->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(trueEl);

  osg::ref_ptr<simVis::RangeTool::Calculation> trueCmp = new simVis::RangeTool::Calculation("True Composite Angle");
  trueCmp->addGraphic(new simVis::RangeTool::TrueCompositeAnglePieSliceGraphic, true);
  trueCmp->setLabelMeasurement(new simVis::RangeTool::TrueCompositeAngleMeasurement);
  trueCmp->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(trueCmp);

  osg::ref_ptr<simVis::RangeTool::Calculation> relOriAz = new simVis::RangeTool::Calculation("Rel Ori Azimuth");
  relOriAz->addGraphic(new simVis::RangeTool::RelOriAzimuthPieSliceGraphic, true);
  relOriAz->setLabelMeasurement(new simVis::RangeTool::RelOriAzimuthMeasurement);
  relOriAz->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relOriAz);

  osg::ref_ptr<simVis::RangeTool::Calculation> relOriEl = new simVis::RangeTool::Calculation("Rel Ori Elevation");
  relOriEl->addGraphic(new simVis::RangeTool::RelOriElevationPieSliceGraphic, true);
  relOriEl->setLabelMeasurement(new simVis::RangeTool::RelOriElevationMeasurement);
  relOriEl->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relOriEl);

  osg::ref_ptr<simVis::RangeTool::Calculation> relOriCmp = new simVis::RangeTool::Calculation("Rel Ori Composite Angle");
  relOriCmp->addGraphic(new simVis::RangeTool::RelOriCompositeAnglePieSliceGraphic, true);
  relOriCmp->setLabelMeasurement(new simVis::RangeTool::RelOriCompositeAngleMeasurement);
  relOriCmp->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relOriCmp);

  osg::ref_ptr<simVis::RangeTool::Calculation> relVelAz = new simVis::RangeTool::Calculation("Rel Vel Azimuth");
  relVelAz->addGraphic(new simVis::RangeTool::RelVelAzimuthPieSliceGraphic, true);
  relVelAz->setLabelMeasurement(new simVis::RangeTool::RelVelAzimuthMeasurement);
  relVelAz->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relVelAz);

  osg::ref_ptr<simVis::RangeTool::Calculation> relVelEl = new simVis::RangeTool::Calculation("Rel Vel Elevation");
  relVelEl->addGraphic(new simVis::RangeTool::RelVelElevationPieSliceGraphic, true);
  relVelEl->setLabelMeasurement(new simVis::RangeTool::RelVelElevationMeasurement);
  relVelEl->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relVelEl);

  osg::ref_ptr<simVis::RangeTool::Calculation> relVelCmp = new simVis::RangeTool::Calculation("Rel Vel Composite Angle");
  relVelCmp->addGraphic(new simVis::RangeTool::RelVelCompositeAnglePieSliceGraphic, true);
  relVelCmp->setLabelMeasurement(new simVis::RangeTool::RelVelCompositeAngleMeasurement);
  relVelCmp->setLabelUnits(osgEarth::Units::DEGREES);
  calcs.push_back(relVelCmp);
}

//----------------------------------------------------------------------------
/// event handler for keyboard commands to alter symbology at runtime

struct MenuHandler : public osgGA::GUIEventHandler
{
  /// constructor grabs all the state it needs for updating
  MenuHandler(simVis::Viewer* viewer, simVis::ScenarioManager* scenario, osg::Node* tetherNode)
    : viewer_(viewer),
      scenario_(scenario),
      tetherNode_(tetherNode)
  {
    //nop
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    bool handled = false;
    osgEarth::Util::EarthManipulator* manip = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer_->getMainView()->getCameraManipulator());
    simVis::Viewpoint vp;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case '?' : // toggle help
        s_helpControl->setVisible(!s_helpControl->visible());
        handled = true;
        break;

      case '1' : // cycle line calculations
        if (s_lineCalcIndex >= 0)
          s_association->remove(s_lineCalcs[s_lineCalcIndex]);

        s_lineCalcIndex++;
        if (s_lineCalcIndex >= (int)s_lineCalcs.size())
          s_lineCalcIndex = -1;

        if (s_lineCalcIndex >= 0)
        {
          s_lineCalcLabel->setText("Currently viewing: " + s_lineCalcs[s_lineCalcIndex]->name());
          s_association->add(s_lineCalcs[s_lineCalcIndex]);
        }
        else
          s_lineCalcLabel->setText("Currently viewing: none");
        break;

      case '2' : // cycle angle calculations
        if (s_angleCalcIndex >= 0)
          s_association->remove(s_angleCalcs[s_angleCalcIndex]);

        s_angleCalcIndex++;
        if (s_angleCalcIndex >= (int)s_angleCalcs.size())
          s_angleCalcIndex = -1;

        if (s_angleCalcIndex >= 0)
        {
          s_angleCalcLabel->setText("Currently viewing: " + s_angleCalcs[s_angleCalcIndex]->name());
          s_association->add(s_angleCalcs[s_angleCalcIndex]);
        }
        else
          s_angleCalcLabel->setText("Currently viewing: none");
        break;

      case '3' : // zoom in
        viewer_->getMainView()->setFocalOffsets(0, -45, 5E1, 2);
        break;

      case '4' : // rotate zoomed in
        viewer_->getMainView()->setFocalOffsets(30, -45, 5E1, 2);
        break;

      case '5' : // reset view
        viewer_->getMainView()->tetherCamera(tetherNode_.get());
        viewer_->getMainView()->setFocalOffsets(0, -45, 5E5, 2);
        break;

      case 'w': // Position offset forward
        // NOTE: The position offsets fail when applied to a TETHER_CENTER_AND_ROTATION view.  The commented
        // sections below demonstrate an alternate way to set the position offsets using getViewpoint().
        // Both the primary and alternate methods of setting the viewpoint fail in the center-and-rotation mode.

        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(0, 1, 0);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 's': // Position offset back
        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(0, -1, 0);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 'a': // Position offset left
        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(-1, 0, 0);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 'd': // Position offset right
        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(1, 0, 0);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 'q': // Position offset up
        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(0, 0, 1);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 'z': // Position offset down
        vp.positionOffset() = (*viewer_->getMainView()->getViewpoint().positionOffset()) + osg::Vec3(0, 0, -1);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 'g': // Position offset reset
        vp.positionOffset() = osg::Vec3(0, 0, 0);
        viewer_->getMainView()->setViewpoint(vp);
        handled = true;
        break;

      case 't': // Toggle tether mode for following platform
        if (manip->getSettings()->getTetherMode() == osgEarth::Util::EarthManipulator::TETHER_CENTER)
          manip->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER_AND_ROTATION);
        else
          manip->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER);
        handled = true;
        break;
      }
    }

    return handled;
  }

protected: // data
  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::ref_ptr<simVis::ScenarioManager> scenario_;
  osg::observer_ptr<osg::Node> tetherNode_;
};

//----------------------------------------------------------------------------

simData::ObjectId createPlatform(simData::DataStore& dataStore)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
  simData::ObjectId id = props->id();
  xaction.complete(&props);
  return id;
}

//----------------------------------------------------------------------------

simVis::SimulatorEventHandler* createSimulation(simUtil::PlatformSimulatorManager& simMgr, simData::ObjectId obj1, simData::ObjectId obj2)
{
  osg::ref_ptr<simUtil::PlatformSimulator> sim1 = new simUtil::PlatformSimulator(obj1);
  sim1->addWaypoint(simUtil::Waypoint(51.5,   0.5, 40000, 200.0)); // London
  sim1->addWaypoint(simUtil::Waypoint(38.8, -77.0, 40000, 200.0)); // DC
  sim1->setSimulateRoll(true);
  sim1->setSimulatePitch(true);
  simMgr.addSimulator(sim1);

  osg::ref_ptr<simUtil::PlatformSimulator> sim2 = new simUtil::PlatformSimulator(obj2);
  sim2->addWaypoint(simUtil::Waypoint(51.0, 0.0, 20000, 200.0));
  sim2->addWaypoint(simUtil::Waypoint(38.0, -76.0, 20000, 200.0));
  simMgr.addSimulator(sim2);

  // Run the simulations:
  simMgr.simulate(0.0, 120.0, 60.0);

  return new simVis::SimulatorEventHandler(&simMgr, 0.0, 120.0);
}

//----------------------------------------------------------------------------

simVis::View* createInsetView(simVis::View& mainView, float xPosition)
{
  // Create an inset centered on first item
  simVis::View* inset = new simVis::View;
  inset->setExtents(simVis::View::Extents(xPosition, .75, .25, .25, true));
  inset->setSceneManager(mainView.getSceneManager());
  inset->applyManipulatorSettings(mainView);
  inset->setName("Centered_Inset");
  mainView.addInset(inset);
  return inset;
}

void centerInsetView(simVis::View& view, simData::ObjectId objectId)
{
  simVis::ScenarioManager* scenario = view.getSceneManager()->getScenario();

  // Set a view centered on platform 1
  simVis::Viewpoint viewpoint;
  viewpoint.setNode(view.getModelNodeForTether(scenario->find(objectId)));
  viewpoint.heading() = osgEarth::Angle(70, osgEarth::Units::DEGREES);
  viewpoint.pitch() = osgEarth::Angle(-15, osgEarth::Units::DEGREES);
  viewpoint.range() = osgEarth::Distance(15, osgEarth::Units::METERS);
  view.setViewpoint(viewpoint);
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// use the utility code to create a basic world map (terrain imagery and height)
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map);
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  /// (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;

  /// bind dataStore to the scenario manager
  // TODO (perhaps this should be automatic, or a scene->getScenario()->bind(dataStore) instead -gw)
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&dataStore);

  // Create two platforms to visualize:
  simData::ObjectId obj1 = createPlatform(dataStore);
  simData::ObjectId obj2 = createPlatform(dataStore);

  // Set up and apply prefs to platform 1
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(obj1, &txn);
    prefs->set_dynamicscale(true);
    prefs->set_scale(3.0);
    prefs->mutable_commonprefs()->set_name("First");
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsetx(50);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(10);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    txn.complete(&prefs);
  }

  // Set up and apply prefs to platform 2
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(obj2, &txn);
    prefs->set_dynamicscale(true);
    prefs->set_scale(3.0);
    prefs->mutable_commonprefs()->set_name("Second");
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsetx(50);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(-10);
    prefs->set_icon(EXAMPLE_MISSILE_ICON);
    txn.complete(&prefs);
  }

  // Set up a simulation for our two platforms.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  viewer->addEventHandler(createSimulation(*simMgr, obj1, obj2));

  // Set up the range tool.
  osg::ref_ptr<simVis::RangeTool> rangeTool = new simVis::RangeTool();
  s_association = rangeTool->add(obj1, obj2);
  createLineCalculations(s_lineCalcs);
  createAngleCalculations(s_angleCalcs);

  scene->getScenario()->addTool(rangeTool);

  /// Tether camera to platform
  osg::observer_ptr<simVis::EntityNode> obj1Node = scene->getScenario()->find(obj1);
  simVis::View* mainView = viewer->getMainView();
  mainView->tetherCamera(obj1Node.get());

  /// set the camera to look at the platform
  mainView->setFocalOffsets(0, -45, 5e5);

  /// handle keypress events
  viewer->addEventHandler(new MenuHandler(viewer, scene->getScenario(), obj1Node.get()));

  /// hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(scene));

  /// show the instructions overlay
  mainView->addOverlayControl(createHelp());

  // Create one inset centered on each object
  simVis::View* inset1 = createInsetView(*mainView, 0.50f);
  centerInsetView(*inset1, obj1);
  simVis::View* inset2 = createInsetView(*mainView, 0.75f);
  centerInsetView(*inset2, obj2);

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}
