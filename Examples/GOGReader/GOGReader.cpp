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
 * GOG READER EXAMPLE - SIMDIS SDK
 *
 * Demonstrates the loading and display of SIMDIS .gog format vector overlay data.
 */

#include <vector>
#include <sstream>
#include <iomanip>
#include "osgEarth/LabelNode"
#include "osgEarth/NodeUtils"
#include "osgEarth/optional"
#include "osgEarth/PlaceNode"

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/Parser.h"
#include "simCore/String/UtfUtils.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Loader.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/GogManipulatorController.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"

using namespace osgEarth;
using namespace osgEarth::Util;

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

using GogNodeInterfacePtr = std::shared_ptr<simVis::GOG::GogNodeInterface>;
using GogNodeVector = std::vector<GogNodeInterfacePtr>;

constexpr const char* HELP_TEXT =
"c : center on next GOG\n"
"e : toggle edit mode for centered GOG\n"
"g : toggle global opt-in editing\n"
"a : change altitude mode for centered GOG\n"
"f : toggle fill state for centered GOG\n"
"n : toggle labels for all platforms\n"
"d : toggle dynamic scale for all platforms\n";

//----------------------------------------------------------------------------
/// A mouse position listener to update the elevation label with the current lat/lon/elevation value under the mouse
class LatLonElevListener : public simUtil::MousePositionManipulator::Listener
{
public:
  // Returns true if the LLA is valid, false otherwise
  bool isValid() const { return lastLat_ != simUtil::MousePositionManipulator::INVALID_POSITION_VALUE; }
  // latitude in degrees
  double lat() const { return lastLat_; }
  // longitude in degrees
  double lon() const { return lastLon_; }
  // elevation in meters
  double elev() const { return lastElev_; }

  void mouseOverLatLon(double lat, double lon, double elev) override
  {
    lastLat_ = lat;
    lastLon_ = lon;
    lastElev_ = elev;
  }

private:
  double lastLat_ = simUtil::MousePositionManipulator::INVALID_POSITION_VALUE;
  double lastLon_ = simUtil::MousePositionManipulator::INVALID_POSITION_VALUE;
  double lastElev_ = simUtil::MousePositionManipulator::INVALID_POSITION_VALUE;
};

//------------------------------------------------------------------------------
/// An event handler to assist in testing the GOG dynamic update functionality.
class MouseAndMenuHandler : public osgGA::GUIEventHandler
{
public:
  MouseAndMenuHandler(simVis::Viewer* viewer, simData::DataStore& dataStore,
    bool showElevation, simVis::PlatformNode* platform, GogNodeVector& overlayNodes,
    std::shared_ptr<simUtil::GogManipulatorController> manipulatorController)
    : viewer_(viewer),
    dataStore_(dataStore),
    overlayNodes_(overlayNodes),
    platform_(platform),
    manipulatorController_(manipulatorController),
    showElevation_(showElevation)
  {
    centeredGogIndex_.init(0);
    mouseDispatcher_ = std::make_shared<simUtil::MouseDispatcher>();
    mouseDispatcher_->setViewManager(nullptr);
    latLonElevListener_ = std::make_shared<LatLonElevListener>();

    setUpMouseManip_(viewer_.get());
    updateStatusAndLabel_();
  }

  virtual ~MouseAndMenuHandler() override
  {
    if (mouseManip_)
      mouseManip_->removeListener(latLonElevListener_.get());
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
      return handleKeyPress_(ea.getKey());

    case osgGA::GUIEventAdapter::DRAG:
      // panning uncenters from GOG
      if (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
      {
        centeredGogIndex_.clear();
        updateStatusAndLabel_();
      }
      // zooming updates camera distance label
      if (ea.getButtonMask() & osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
        updateStatusAndLabel_();
      return false;

    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::SCROLL:
      updateStatusAndLabel_();
      return false;

    default:
      return false;
    }
  }

  std::string statusText() const
  {
    return statusText_;
  }

private:
  bool handleKeyPress_(int keyPress)
  {
    switch (keyPress)
    {
    case 'c': // center on next GOG
    {
      if (overlayNodes_.empty()) return false;

      centeredGogIndex_ = centeredGogIndex_.get() + 1;
      if (centeredGogIndex_ >= overlayNodes_.size())
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
        overlayNodes_[centeredGogIndex_.get()]->getPosition(position, &referencePosition);
      }
      else
      {
        overlayNodes_[centeredGogIndex_.get()]->getPosition(position);
      }

      simVis::GOG::AltitudeMode curMode;
      if (overlayNodes_[centeredGogIndex_.get()]->getAltitudeMode(curMode) == 0)
        altMode_ = curMode;

      simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();
      simVis::Viewpoint eyePos = focusedView->getViewpoint();

      // update the eye position's focal point
      focusedView->tetherCamera(nullptr);
      eyePos.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("wgs84"), position);
      eyePos.setNode(nullptr);

      focusedView->setViewpoint(eyePos);
      updateStatusAndLabel_();
      return true;
    }

    case 'a': // change altitude mode for centered GOG
    {
      if (!centeredGogIndex_.isSet() || centeredGogIndex_ >= overlayNodes_.size())
        return false;

      if (altMode_ == simVis::GOG::ALTITUDE_EXTRUDE)
        altMode_ = simVis::GOG::ALTITUDE_NONE;
      else
        altMode_ = static_cast<simVis::GOG::AltitudeMode>(altMode_ + 1);

      overlayNodes_[centeredGogIndex_.get()]->setAltitudeMode(altMode_);
      updateStatusAndLabel_();
      return true;
    }

    case 'f': // change fill state for centered GOG
    {
      if (!centeredGogIndex_.isSet() || centeredGogIndex_ >= overlayNodes_.size())
        return false;

      bool filled = false;
      osg::Vec4f fillColor;
      if (overlayNodes_[centeredGogIndex_.get()]->getFilledState(filled, fillColor) == 0)
        overlayNodes_[centeredGogIndex_.get()]->setFilledState(!filled);
      return true;
    }

    case 'd': // toggle dynamic scale
    {
      dynamicScaleOn_ = !dynamicScaleOn_;
      std::vector<simData::ObjectId> ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (const auto& id : ids)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(id, &tn);
        prefs->set_dynamicscale(dynamicScaleOn_);
        tn.complete(&prefs);
      }
      updateStatusAndLabel_();
      return true;
    }

    case 'n': // toggle labels
    {
      labelsOn_ = !labelsOn_;
      std::vector<simData::ObjectId> ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (const auto& id : ids)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(id, &tn);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(labelsOn_);
        tn.complete(&prefs);
      }
      return true;
    }

    case 'e': // toggle edit mode for centered GOG
      if (!centeredGogIndex_.isSet() || centeredGogIndex_ >= overlayNodes_.size())
        return false;

      // Let the controller handle the spawn/destroy logic
      if (manipulatorController_)
      {
        auto gog = overlayNodes_[centeredGogIndex_.get()];
        manipulatorController_->toggleExplicitEdit(gog);
      }

      updateStatusAndLabel_();
      return true;

    case 'g': // toggle global opt-in edit mode
      if (manipulatorController_)
      {
        const bool newState = !manipulatorController_->isGlobalEditMode();
        manipulatorController_->setGlobalEditMode(newState);
      }
      updateStatusAndLabel_();
      return true;
    }
    return false;
  }

  void updateStatusAndLabel_()
  {
    std::ostringstream os;

    os << "Centered: ";
    if (centeredGogIndex_.isSet() && centeredGogIndex_ < overlayNodes_.size())
    {
      auto gog = overlayNodes_[centeredGogIndex_.get()];
      os << gog->osgNode()->getName();
      if (manipulatorController_ && manipulatorController_->isEditing(*gog))
        os << " [EDITING]";
      os << "\n";
    }
    else
      os << "None\n";

    os << "Altitude Mode: ";
    switch (altMode_)
    {
    case simVis::GOG::ALTITUDE_NONE:             os << "NONE\n"; break;
    case simVis::GOG::ALTITUDE_GROUND_RELATIVE:  os << "GROUND RELATIVE\n"; break;
    case simVis::GOG::ALTITUDE_GROUND_CLAMPED:   os << "GROUND CLAMPED\n"; break;
    case simVis::GOG::ALTITUDE_EXTRUDE:          os << "EXTRUDE\n"; break;
    }

    os << "\nDynamic Scale: " << (dynamicScaleOn_ ? "ON" : "OFF") << "\n";

    if (manipulatorController_)
      os << "Global Edit Mode: " << (manipulatorController_->isGlobalEditMode() ? "ON" : "OFF") << "\n";

    const simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();
    os << std::fixed << std::setprecision(2)
      << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m\n";

    if (!latLonElevListener_->isValid())
      os << "Mouse not over earth\n";
    else
    {
      os << "Mouse lat: " << std::setprecision(5) << latLonElevListener_->lat() << ", lon: " << latLonElevListener_->lon();
      if (showElevation_)
        os << ", elev:" << latLonElevListener_->elev();
      os << "\n";
    }

    statusText_ = os.str();
  }

  void setUpMouseManip_(simVis::Viewer* viewer)
  {
    if (!viewer || !viewer->getSceneManager() || !mouseDispatcher_)
      return;

    mouseManip_ = std::make_shared<simUtil::MousePositionManipulator>(
      viewer->getSceneManager()->getMapNode(),
      viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks"));

    mouseManip_->setTerrainResolution(0.0001);
    mouseDispatcher_->setViewManager(viewer);
    mouseDispatcher_->addManipulator(0, mouseManip_);
    mouseManip_->addListener(latLonElevListener_.get(), showElevation_);
  }

  osg::ref_ptr<simVis::Viewer> viewer_;
  simData::DataStore& dataStore_;
  GogNodeVector& overlayNodes_;
  osg::ref_ptr<simVis::PlatformNode> platform_;

  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher_;
  std::shared_ptr<LatLonElevListener> latLonElevListener_;
  std::shared_ptr<simUtil::MousePositionManipulator> mouseManip_;
  std::shared_ptr<simUtil::GogManipulatorController> manipulatorController_;

  bool showElevation_ = false;
  bool dynamicScaleOn_ = true;
  bool labelsOn_ = true;

  osgEarth::optional<size_t> centeredGogIndex_;
  simVis::GOG::AltitudeMode altMode_ = simVis::GOG::ALTITUDE_NONE;
  std::string statusText_;
};

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore& dataStore, const std::string& iconFile)
{
  simData::ObjectId platformId;

  // create the new platform:
  {
    simData::DataStore::Transaction transaction;
    simData::PlatformProperties* newProps = dataStore.addPlatform(&transaction);
    platformId = newProps->id();
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
    xaction.complete(&prefs);
  }

  // now add some data points; include orientation for testing 3d follow
  {
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

#ifdef HAVE_IMGUI
struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel(MouseAndMenuHandler& handler, const GogNodeVector& overlayNodes)
    : simExamples::SimExamplesGui("GOG Example"),
    handler_(handler),
    overlayNodes_(overlayNodes)
  {}

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible()) return;

    ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("%s", HELP_TEXT);
    ImGui::Text("%s", handler_.statusText().c_str());

    float opacity = opacity_;
    ImGui::Text("Opacity: "); ImGui::SameLine();
    ImGui::SliderFloat("##Opacity", &opacity_, 0.f, 100.f, "%.f", ImGuiSliderFlags_AlwaysClamp);

    if (opacity != opacity_)
    {
      float zeroToOne = opacity_ * 0.01f;
      for (const auto& overlay : overlayNodes_)
        overlay->setOpacity(zeroToOne);
    }

    ImGui::End();
  }

private:
  MouseAndMenuHandler& handler_;
  const GogNodeVector& overlayNodes_;
  float opacity_ = 100.f;
};
#endif

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();

  osg::ArgumentParser ap(&argc, argv);
  if (ap.argc() < 2 || ap.read("-h") || ap.read("--help"))
  {
    std::cout << "Usage: example_gogreader <gogfiles...> [--attach] [--showElevation] [--mark] [--sky] [--iconFile <icon file>]" << std::endl;
    return 0;
  }

  // Parse OpenSceneGraph standard arguments
  const bool sky = ap.read("--sky");
  const bool mark = ap.read("--mark");
  const bool showElevation = ap.read("--showElevation");
  const bool attach = ap.read("--attach");

  std::string iconFile = EXAMPLE_IMAGE_ICON;
  ap.read("--iconFile", iconFile);

  // Extract remaining non-option arguments as GOG files
  std::vector<std::string> gogFiles;
  for (int i = 1; i < ap.argc(); ++i)
  {
    if (!ap.isOption(i))
      gogFiles.push_back(ap[i]);
  }

  // Localized state container for the application
  GogNodeVector overlayNodes;

  simExamples::configureSearchPaths();
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // Create central manipulator controller
  auto manipulatorController = std::make_shared<simUtil::GogManipulatorController>(scene->getMapNode(), scene->getScenario());

  if (sky)
    simExamples::addDefaultSkyNode(viewer.get());

  osg::ref_ptr<osg::Image> pin;
  if (mark)
    pin = URI("http://www.osgearth.org/chrome/site/pushpin_yellow.png").getImage();

  GeoPoint go;
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  const simData::ObjectId platformId = addPlatform(dataStore, iconFile);
  osg::ref_ptr<simVis::PlatformNode> platform = scene->getScenario()->find<simVis::PlatformNode>(platformId);

  osg::ref_ptr<osg::Group> group = new osg::Group();

  for (const std::string& gogFile : gogFiles)
  {
    const simCore::GOG::Parser parser;
    simVis::GOG::Loader loader(parser, scene->getMapNode());
    loader.setReferencePosition(simCore::GOG::BSTUR);

    std::ifstream is(simCore::streamFixUtf8(gogFile));
    if (!is.is_open())
    {
      SIM_ERROR << "Could not open GOG file " << gogFile << "\n";
      return 1;
    }

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadGogs(is, gogFile, attach, gogs);

    if (gogs.empty())
    {
      SIM_WARN << "Unable to load GOG data from \"" << gogFile << "\"\n";
      continue;
    }

    for (const auto& gogInterface : gogs)
    {
      osg::Node* gog = gogInterface->osgNode();

      if (attach)
      {
        // TODO: SIM-13358- incorporate this logic into simVis PlatformNode
        bool followYaw = false;
        bool followPitch = false;
        bool followRoll = false;
        if (const auto* shape = gogInterface->shapeObject())
        {
          shape->getIsFollowingYaw(followYaw);
          shape->getIsFollowingPitch(followPitch);
          shape->getIsFollowingRoll(followRoll);
        }

        simVis::Locator* locator = new simVis::Locator(platform->getLocator(),
          simVis::Locator::COMP_POSITION
          | (followYaw ? simVis::Locator::COMP_HEADING : 0)
          | (followPitch ? simVis::Locator::COMP_PITCH : 0)
          | (followRoll ? simVis::Locator::COMP_ROLL : 0));

        simVis::LocatorNode* locatorNode = new simVis::LocatorNode(locator, gog);
        group->addChild(locatorNode);
      }
      else
        group->addChild(gog);

      overlayNodes.push_back(gogInterface);

      if (mark)
      {
        const osg::Vec3d ecef0 = gog->getBound().center();
        const simCore::Vec3 ecef(ecef0.x(), ecef0.y(), ecef0.z());
        simCore::Vec3 lla;
        simCore::CoordinateConverter::convertEcefToGeodeticPos(ecef, lla);

        const std::string label = gog->getName();
        const GeoPoint location(
          scene->getMapNode()->getMapSRS(),
          osg::RadiansToDegrees(lla.lon()),
          osg::RadiansToDegrees(lla.lat()),
          0.0, ALTMODE_ABSOLUTE);

        osg::ref_ptr<GeoPositionNode> marker;
        if (label.empty())
        {
          PlaceNode* place = new PlaceNode();
          place->setIconImage(pin.get());
          marker = place;
        }
        else
          marker = new LabelNode(label);

        marker->setMapNode(scene->getMapNode());
        marker->setPosition(location);
        scene->getScenario()->addChild(marker);

        go = location;
      }
    }
  }

  if (attach)
    platform->addChild(group);
  else
    scene->getScenario()->addChild(group);

  if (overlayNodes.empty())
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
    viewer->getMainView()->setViewpoint(vp);
  }

  simVis::View* mainView = viewer->getMainView();

  // Controller will need to know about the overlay nodes
  manipulatorController->notifyShapesAdded(overlayNodes);

  osg::ref_ptr<MouseAndMenuHandler> mouseHandler = new MouseAndMenuHandler(
    viewer.get(),
    dataStore,
    showElevation,
    attach ? platform.get() : nullptr,
    overlayNodes,
    manipulatorController);

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(*mouseHandler.get(), overlayNodes));
#endif

  mainView->getCamera()->addEventCallback(mouseHandler);
  viewer->run();
}
