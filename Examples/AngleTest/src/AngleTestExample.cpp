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
 * Angle Test Example
 *
 * Unit test verifying that Earth Centered Earth Fixed (ECEF) angle composition
 * is rendered correctly in OSG.
 */
/// the simulator provides time/space data for our platform
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"

#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osgEarth/StringUtils"
#include "osgEarthSymbology/Style"
#include "osgEarthUtil/LatLongFormatter"
#include "osgEarthUtil/MGRSFormatter"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

/** Test program for update angle computations. */

/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl;

static simData::DataStore* s_dataStore;

static simData::ObjectId s_id;

static HSliderControl* yaw, * pitch, * roll, * lat, * lon;

static double s_time = 0.0;


struct SetUpdate : public ControlEventHandler
{
  void onValueChanged(Control*, float value)
  {
    s_time += 1.0;

    simCore::Coordinate lla(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat->getValue(), simCore::DEG2RAD * lon->getValue(), 10000.0),
      simCore::Vec3(simCore::DEG2RAD * yaw->getValue(), pitch->getValue() * simCore::DEG2RAD, roll->getValue() * simCore::DEG2RAD));
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);

    simData::DataStore::Transaction t;
    simData::PlatformUpdate* u = s_dataStore->addPlatformUpdate(s_id, &t);
    u->set_time(s_time);
    u->set_x(ecef.x());
    u->set_y(ecef.y());
    u->set_z(ecef.z());
    u->set_psi(ecef.psi());
    u->set_theta(ecef.theta());
    u->set_phi(ecef.phi());
    t.complete(&u);

    s_dataStore->update(s_time);
  }
};

static Control* createHelp()
{
  osg::ref_ptr<Grid> g = new Grid();
  g->setChildSpacing(5);

  g->setControl(0, 0, new LabelControl("Yaw:"));
  yaw = g->setControl(1, 0, new HSliderControl(-180, 180, 0.0));
  yaw->setSize(300, 35);
  yaw->addEventHandler(new SetUpdate());

  g->setControl(0, 1, new LabelControl("Pitch:"));
  pitch = g->setControl(1, 1, new HSliderControl(-90, 90, 0.0));
  pitch->setSize(300, 35);
  pitch->addEventHandler(new SetUpdate());

  g->setControl(0, 2, new LabelControl("Roll:"));
  roll = g->setControl(1, 2, new HSliderControl(-90, 90, 0.0));
  roll->setSize(300, 35);
  roll->addEventHandler(new SetUpdate());

  g->setControl(0, 3, new LabelControl("Lat:"));
  lat = g->setControl(1, 3, new HSliderControl(-89, 89, 0.0));
  lat->setSize(300, 35);
  lat->addEventHandler(new SetUpdate());

  g->setControl(0, 4, new LabelControl("Long:"));
  lon = g->setControl(1, 4, new HSliderControl(-180, 180, 0.0));
  lon->setSize(300, 35);
  lon->addEventHandler(new SetUpdate());

  s_helpControl = g.get();
  return g.release();
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  simCore::checkVersionThrow();

  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  viewer->setMap(map.get());

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  s_dataStore = &dataStore;

  /// bind dataStore to the scenario manager
  // TODO (perhaps this should be automatic, or a scene->getScenario()->bind(dataStore) instead -gw)
  //simVis::ScenarioDataStoreAdapter adapter(&dataStore, scene->getScenario());

  osg::ref_ptr<simVis::ScenarioManager> scenario = viewer->getSceneManager()->getScenario();

  scenario->bind(&dataStore);

  {
    simData::DataStore::Transaction transaction;
    simData::PlatformProperties* newProps = s_dataStore->addPlatform(&transaction);
    s_id = newProps->id();
    transaction.complete(&newProps);
  }

  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = s_dataStore->mutable_platformPrefs(s_id, &xaction);
    prefs->mutable_commonprefs()->set_name("Simulated Platform");
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(20.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  /// Tether camera to platform
  osg::observer_ptr<simVis::PlatformNode> platformNode = scenario->find<simVis::PlatformNode>(s_id);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 4e5);

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());

  /// Prime it
  SetUpdate().onValueChanged(NULL, 0.0);

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

