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
 * GOG READER EXAMPLE - SIMDIS SDK
 *
 * Demonstrates the loading and display of SIMDIS .gog format vector overlay data.
 */

#include "osgEarth/NodeUtils"
#include "osgEarthAnnotation/PlaceNode"
#include "osgEarthAnnotation/LabelNode"
#include "osgEarthUtil/Controls"
#include "osgEarthUtil/MouseCoordsTool"

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/GOG/GOG.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Parser.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"

namespace ui = osgEarth::Util::Controls;

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Annotation;

typedef std::shared_ptr<simVis::GOG::GogNodeInterface> GogNodeInterfacePtr;
static std::vector<GogNodeInterfacePtr> s_overlayNodes;

static std::string s_title = " \n \nGOG Example";
static std::string s_help =
"c : center on next GOG\n"
"a : change altitude mode for centered GOG\n"
"f : toggle fill state for centered GOG\n"
"n : toggle labels for all platforms\n"
"d : toggle dynamic scale for all platforms\n";

//----------------------------------------------------------------------------
/// A mouse position listener to update the elevation label with the current lat/lon/elevation value under the mouse
class LatLonElevListener : public simUtil::MousePositionManipulator::Listener
{
public:
  LatLonElevListener()
    : lastLat_(0.),
    lastLon_(0.),
    lastElev_(0.)
  {
  }

  // latitude in degrees
  double lat() const { return lastLat_; }
  // logintude in degrees
  double lon() const { return lastLon_; }
  // elevation in meters
  double elev() const { return lastElev_; }

  virtual void mouseOverLatLon(double lat, double lon, double elev)
  {
    lastLat_ = lat;
    lastLon_ = lon;
    lastElev_ = elev;
  }

private:
  double lastLat_; // degrees
  double lastLon_; // degrees
  double lastElev_; // meters
};

//------------------------------------------------------------------------------
/// An event handler to assist in testing the GOG dynamic update functionality.
class MouseAndMenuHandler : public osgGA::GUIEventHandler
{
public:
  MouseAndMenuHandler(simVis::Viewer* viewer,
    ui::LabelControl* status,
    simData::DataStore& dataStore,
    bool showElevation,
    simVis::PlatformNode* platform
    )
    : viewer_(viewer),
    statusLabel_(status),
    dataStore_(dataStore),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true),
    border_(0),
    centeredGogIndex_(-1),
    platform_(platform),
    altMode_(simVis::GOG::ALTITUDE_NONE)
  {
    mouseDispatcher_.reset(new simUtil::MouseDispatcher);
    mouseDispatcher_->setViewManager(NULL);
    latLonElevListener_.reset(new LatLonElevListener());
    setUpMouseManip_(viewer_.get());
    updateStatusAndLabel_();
  }

  virtual ~MouseAndMenuHandler()
  {
    mouseManip_->removeListener(latLonElevListener_.get());
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;


    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      handled = handleKeyPress_(ea.getKey());
    else if (ea.getEventType() == osgGA::GUIEventAdapter::DRAG)
    {
      // panning uncenters from GOG
      if (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
      {
        centeredGogIndex_ = -1;
        updateStatusAndLabel_();
      }
      // zooming updates camera distance label
      if (ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        updateStatusAndLabel_();
    }
    else if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
      updateStatusAndLabel_();
    // scroll zooming updates camera distance label
    else if (ea.getEventType() == osgGA::GUIEventAdapter::SCROLL)
      updateStatusAndLabel_();

    return handled;
  }

private:

  bool handleKeyPress_(int keyPress)
  {
    switch (keyPress)
    {
    case 'c': // center on next GOG
    {
      centeredGogIndex_++;
      if (centeredGogIndex_ >= s_overlayNodes.size())
        centeredGogIndex_ = 0;
      osg::Vec3d position;
      if (platform_.valid())
      {
        osgEarth::GeoPoint referencePosition;
        simCore::Coordinate coord;
        platform_->getLocator()->getCoordinate(&coord, simCore::COORD_SYS_LLA);
        referencePosition.x() = coord.lon() * simCore::RAD2DEG;
        referencePosition.y() = coord.lat() * simCore::RAD2DEG;
        referencePosition.z() = coord.alt();
        s_overlayNodes[centeredGogIndex_]->getPosition(position, &referencePosition);

      }
      else
        s_overlayNodes[centeredGogIndex_]->getPosition(position);

      simVis::GOG::AltitudeMode curMode;
      if (s_overlayNodes[centeredGogIndex_]->getAltitudeMode(curMode) == 0)
        altMode_ = curMode;

      simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();
      simVis::Viewpoint eyePos = focusedView->getViewpoint();

      // update the eye position's focal point
      focusedView->tetherCamera(NULL);
      eyePos.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("wgs84"), position);
      eyePos.setNode(NULL);

      focusedView->setViewpoint(eyePos);
      updateStatusAndLabel_();

      return true;
    }

    case 'a': // change altitude mode for centered GOG
    {
      if (centeredGogIndex_ < 0 || centeredGogIndex_ >= s_overlayNodes.size())
        return false;
      if (altMode_ == simVis::GOG::ALTITUDE_EXTRUDE)
        altMode_ = simVis::GOG::ALTITUDE_NONE;
      else
        altMode_ = static_cast<simVis::GOG::AltitudeMode>(altMode_ + 1);
      s_overlayNodes[centeredGogIndex_]->setAltitudeMode(altMode_);
      updateStatusAndLabel_();
      return true;
    }

    case 'f': // change fill state for centered GOG
    {
      if (centeredGogIndex_ < 0 || centeredGogIndex_ >= s_overlayNodes.size())
        return false;
      bool filled = false;
      osg::Vec4f fillColor;
      if (s_overlayNodes[centeredGogIndex_]->getFilledState(filled, fillColor) == 0)
        s_overlayNodes[centeredGogIndex_]->setFilledState(!filled);
      return true;
    }

    case 'd': // toggle dynamic scale
    {
      dynamicScaleOn_ = !dynamicScaleOn_;
      std::vector<simData::ObjectId>  ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (auto iter = ids.begin(); iter != ids.end(); ++iter)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
        prefs->set_dynamicscale(dynamicScaleOn_);
        tn.complete(&prefs);
      }
      updateStatusAndLabel_();
      return true;
    }

    case 'n': // toggle labels
    {
      labelsOn_ = !labelsOn_;
      std::vector<simData::ObjectId>  ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (auto iter = ids.begin(); iter != ids.end(); ++iter)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(labelsOn_);
        tn.complete(&prefs);
      }
      return true;
    }

    }
    return false;
  }

  void updateStatusAndLabel_()
  {
    std::string text;

    // get centered GOG name
    text += "Centered: ";
    if (centeredGogIndex_ >= 0 && centeredGogIndex_ < s_overlayNodes.size())
      text += s_overlayNodes[centeredGogIndex_]->osgNode()->getName() + "\n";
    else
      text += "None\n";

    text += "Altitude Mode: ";
    switch (altMode_)
    {
    case simVis::GOG::ALTITUDE_NONE:
      text += "NONE\n";
      break;
    case simVis::GOG::ALTITUDE_GROUND_RELATIVE:
      text += "GROUND RELATIVE\n";
      break;
    case simVis::GOG::ALTITUDE_GROUND_CLAMPED:
      text += "GROUND CLAMPED\n";
      break;
    case simVis::GOG::ALTITUDE_EXTRUDE:
      text += "EXTRUDE\n";
      break;
    }

    // indicate dynamic scale state
    text += "\nDynamic Scale: ";
    text += dynamicScaleOn_ ? "ON" : "OFF";
    text += "\n";

    const simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();

    // get camera distance
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m";
    text += os.str() + " \n";

    std::ostringstream mouseOs;
    mouseOs << "Mouse lat:" << latLonElevListener_->lat() << ", lon:" << latLonElevListener_->lon();
    if (showElevation_)
      mouseOs << ", elev:" << latLonElevListener_->elev();
    text += mouseOs.str() + "\n";

    statusLabel_->setText(text);
  }

  void setUpMouseManip_(simVis::Viewer* viewer)
  {
    if (viewer == NULL || viewer->getSceneManager() == NULL || !mouseDispatcher_)
      return;
    mouseManip_.reset(new simUtil::MousePositionManipulator(viewer->getSceneManager()->getMapNode(), viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks")));
    mouseManip_->setTerrainResolution(0.0001);
    mouseDispatcher_->setViewManager(viewer);
    mouseDispatcher_->addManipulator(0, mouseManip_);
    mouseManip_->addListener(latLonElevListener_.get(), showElevation_);
  }

  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<ui::LabelControl> statusLabel_;
  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher_;
  std::shared_ptr<LatLonElevListener> latLonElevListener_;
  std::shared_ptr<simUtil::MousePositionManipulator> mouseManip_;
  simData::DataStore& dataStore_;

  bool showElevation_;
  bool removeAllRequested_;
  bool insertViewPortMode_;
  bool dynamicScaleOn_;
  bool labelsOn_;
  int border_;
  int centeredGogIndex_;
  osg::ref_ptr<simVis::PlatformNode> platform_;
  simVis::GOG::AltitudeMode altMode_;
};


/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore, const std::string& iconFile)
{
  simData::ObjectId platformId;

  // create the new platform:
  {
    /// all DataStore operations require a transaction (to avoid races)
    simData::DataStore::Transaction transaction;

    /// create the platform, and get the properties for it
    simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

    /// save the platform id for our return value
    platformId = newProps->id();

    /// done
    transaction.complete(&newProps);
  }

  // now set up the platform:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->mutable_commonprefs()->set_name("Simulated Platform");
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->set_icon(iconFile);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // now add some data points
  {
    // add some orientation values for testing 3d Follow
    simCore::Coordinate lla(simCore::COORD_SYS_LLA,
      simCore::Vec3(38.8 * simCore::DEG2RAD, -77.0 * simCore::DEG2RAD, 10.0),
      simCore::Vec3(45.0, 45.0, 45.0),
      simCore::Vec3(0.0, 0.0, 0.0));

    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);

    simData::DataStore::Transaction t;
    simData::PlatformUpdate* u = dataStore.addPlatformUpdate(platformId, &t);
    u->set_time(1.0);
    u->set_x(ecef.x());
    u->set_y(ecef.y());
    u->set_z(ecef.z());
    u->set_psi(ecef.psi());
    u->set_theta(ecef.theta());
    u->set_phi(ecef.phi());
    t.complete(&u);
  }

  dataStore.update(1.0);

  return platformId;
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  if (argc < 2)
  {
    std::cout << "Usage: example_gogreader <gogfile> [--attach] [--showElevation] [--mark] [--sky] [--iconFile <icon file>]" << std::endl;
    return 0;
  }

  // Start by creating a map.
  simExamples::configureSearchPaths();
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // whether to add a push pin to each feature
  osg::ArgumentParser ap(&argc, argv);

  // start up a SIMDIS viewer
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(simVis::Viewer::WINDOWED, 100, 100, 800, 800);
  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  if (ap.read("--sky"))
    simExamples::addDefaultSkyNode(viewer.get());

  bool mark = ap.read("--mark");
  bool showElevation = ap.read("--showElevation");
  bool attach = ap.read("--attach");

  osg::ref_ptr<osg::Image> pin;
  if (mark)
    pin = URI("http://www.osgearth.org/chrome/site/pushpin_yellow.png").getImage();

  GeoPoint go;

  std::string iconFile = EXAMPLE_IMAGE_ICON;

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform
  simData::ObjectId platformId = addPlatform(dataStore, iconFile);
  osg::ref_ptr<simVis::PlatformNode> platform = scene->getScenario()->find<simVis::PlatformNode>(platformId);

  osg::Group* group = new osg::Group();

  // add the gog file vector layers.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--iconFile" && argc > i)
    {
      iconFile = argv[++i];
      continue;
    }

    simVis::GOG::Parser::OverlayNodeVector gogs;
    std::vector<simVis::GOG::GogFollowData> followData;
    simVis::GOG::Parser parser(scene->getMapNode());

    // sets a default reference location for relative GOGs:
    parser.setReferenceLocation(simVis::GOG::BSTUR);

    std::ifstream is(arg.c_str());
    if (!is.is_open())
    {
      std::string fileName(argv[i]);
      SIM_ERROR <<"Could not open GOG file " << fileName << "\n";
      return 1;
    }

    if (parser.loadGOGs(is, attach ? simVis::GOG::GOGNODE_HOSTED : simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData))
    {
      int followIndex = 0;
      for (simVis::GOG::Parser::OverlayNodeVector::iterator i = gogs.begin(); i != gogs.end(); ++i)
      {
        GogNodeInterfacePtr gogInterface(*i);
        osg::Node* gog = (*i)->osgNode();

        // attached GOGs get added to a locator based on the host platform
        if (attach)
        {
          simVis::Locator* locator = new simVis::Locator(platform->getLocator(), followData[followIndex].locatorFlags);
          locator->setLocalOffsets(simCore::Vec3(0, 0, 0), followData[followIndex].orientationOffsets);
          simVis::LocatorNode* locatorNode = new simVis::LocatorNode(locator, gog);
          group->addChild(locatorNode);
        }
        else
          group->addChild(gog);

        // manage the GogNodeInterface object memory
        s_overlayNodes.push_back(gogInterface);

        if (mark)
        {
          osg::Vec3d ecef0 = gog->getBound().center();
          simCore::Vec3 ecef(ecef0.x(), ecef0.y(), ecef0.z());
          simCore::Vec3 lla;
          simCore::CoordinateConverter::convertEcefToGeodeticPos(ecef, lla);

          std::string label = gog->getName();

          GeoPoint location(
            scene->getMapNode()->getMapSRS(),
            osg::RadiansToDegrees(lla.lon()),
            osg::RadiansToDegrees(lla.lat()),
            0.0,
            ALTMODE_ABSOLUTE);

          osg::ref_ptr<AnnotationNode> marker;
          if (label.empty())
            marker = new PlaceNode(scene->getMapNode(), location, pin.get(), label);
          else
            marker = new LabelNode(scene->getMapNode(), location, label);

          scene->getScenario()->addChild(marker);

          go = location;
        }
        followIndex++;
      }
    }
    else
    {
      SIM_WARN << "Unable to load GOG data from \"" << argv[i] << "\"" << std::endl;
    }
  }

  if (attach)
    platform->addChild(group);
  else
    scene->getScenario()->addChild(group);

  // nothing to do if no GOGs loaded
  if (s_overlayNodes.empty())
  {
    std::cerr << "No valid GOGs loaded\n";
    return 1;
  }

  if (mark && go.isValid())
  {
    Viewpoint vp;
    vp.focalPoint() = go;
    vp.pitch()->set(-80.0, Units::DEGREES);
    vp.range()->set(scene->getScenario()->getBound().radius(), Units::METERS);
    viewer->getMainView()->setViewpoint( vp );
  }

  simVis::View* mainView = viewer->getMainView();

  // add help and status labels
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new ui::LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  ui::LabelControl* statusLabel = new ui::LabelControl("STATUS", 14, osg::Vec4f(.8, .8, .8, 1));
  vbox->addControl(statusLabel);
  mainView->addOverlayControl(vbox);

  // Install a handler to respond to the demo keys in this sample.
  osg::ref_ptr<MouseAndMenuHandler> mouseHandler =
    new MouseAndMenuHandler(
      viewer.get(),
      statusLabel,
      dataStore,
      showElevation,
      attach ? platform.get() : NULL);

  mainView->getCamera()->addEventCallback(mouseHandler);
  viewer->run();
}

