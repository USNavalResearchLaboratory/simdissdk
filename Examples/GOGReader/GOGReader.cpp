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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
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
#include "osgEarth/Controls"
#include "osgEarth/LabelNode"
#include "osgEarth/MouseCoordsTool"
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
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"


using namespace osgEarth;
using namespace osgEarth::Util;
#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
using namespace osgEarth::Util::Controls;
namespace ui = osgEarth::Util::Controls;
#endif

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
  // longitude in degrees
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
#ifdef HAVE_IMGUI
  MouseAndMenuHandler(simVis::Viewer* viewer, simData::DataStore& dataStore,
    bool showElevation, simVis::PlatformNode* platform)
    : viewer_(viewer),
    dataStore_(dataStore),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true),
    border_(0),
    platform_(platform),
    altMode_(simVis::GOG::ALTITUDE_NONE)
  {
    centeredGogIndex_.init(0);
    mouseDispatcher_.reset(new simUtil::MouseDispatcher);
    mouseDispatcher_->setViewManager(nullptr);
    latLonElevListener_.reset(new LatLonElevListener());
    setUpMouseManip_(viewer_.get());
    updateStatusAndLabel_();
  }
#else
  MouseAndMenuHandler(simVis::Viewer* viewer, ui::LabelControl* status, simData::DataStore& dataStore,
    bool showElevation, simVis::PlatformNode* platform)
    : viewer_(viewer),
    statusLabel_(status),
    dataStore_(dataStore),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true),
    border_(0),
    platform_(platform),
    altMode_(simVis::GOG::ALTITUDE_NONE)
  {
    centeredGogIndex_.init(0);
    mouseDispatcher_.reset(new simUtil::MouseDispatcher);
    mouseDispatcher_->setViewManager(nullptr);
    latLonElevListener_.reset(new LatLonElevListener());
    setUpMouseManip_(viewer_.get());
    updateStatusAndLabel_();
  }
#endif

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
        centeredGogIndex_.clear();
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
      centeredGogIndex_ = centeredGogIndex_.get() + 1;
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
        s_overlayNodes[centeredGogIndex_.get()]->getPosition(position, &referencePosition);

      }
      else
        s_overlayNodes[centeredGogIndex_.get()]->getPosition(position);

      simVis::GOG::AltitudeMode curMode;
      if (s_overlayNodes[centeredGogIndex_.get()]->getAltitudeMode(curMode) == 0)
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
      if (!centeredGogIndex_.isSet() || centeredGogIndex_ >= s_overlayNodes.size())
        return false;
      if (altMode_ == simVis::GOG::ALTITUDE_EXTRUDE)
        altMode_ = simVis::GOG::ALTITUDE_NONE;
      else
        altMode_ = static_cast<simVis::GOG::AltitudeMode>(altMode_ + 1);
      s_overlayNodes[centeredGogIndex_.get()]->setAltitudeMode(altMode_);
      updateStatusAndLabel_();
      return true;
    }

    case 'f': // change fill state for centered GOG
    {
      if (!centeredGogIndex_.isSet() || centeredGogIndex_ >= s_overlayNodes.size())
        return false;
      bool filled = false;
      osg::Vec4f fillColor;
      if (s_overlayNodes[centeredGogIndex_.get()]->getFilledState(filled, fillColor) == 0)
        s_overlayNodes[centeredGogIndex_.get()]->setFilledState(!filled);
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
    statusText_ = std::string();

    // get centered GOG name
    statusText_ += "Centered: ";
    if (centeredGogIndex_.isSet() && centeredGogIndex_ < s_overlayNodes.size())
      statusText_ += s_overlayNodes[centeredGogIndex_.get()]->osgNode()->getName() + "\n";
    else
      statusText_ += "None\n";

    statusText_ += "Altitude Mode: ";
    switch (altMode_)
    {
    case simVis::GOG::ALTITUDE_NONE:
      statusText_ += "NONE\n";
      break;
    case simVis::GOG::ALTITUDE_GROUND_RELATIVE:
      statusText_ += "GROUND RELATIVE\n";
      break;
    case simVis::GOG::ALTITUDE_GROUND_CLAMPED:
      statusText_ += "GROUND CLAMPED\n";
      break;
    case simVis::GOG::ALTITUDE_EXTRUDE:
      statusText_ += "EXTRUDE\n";
      break;
    }

    // indicate dynamic scale state
    statusText_ += "\nDynamic Scale: ";
    statusText_ += dynamicScaleOn_ ? "ON" : "OFF";
    statusText_ += "\n";

    const simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();

    // get camera distance
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m";
    statusText_ += os.str() + " \n";

    std::ostringstream mouseOs;
    mouseOs << "Mouse lat:" << latLonElevListener_->lat() << ", lon:" << latLonElevListener_->lon();
    if (showElevation_)
      mouseOs << ", elev:" << latLonElevListener_->elev();
    statusText_ += mouseOs.str() + "\n";

#ifndef HAVE_IMGUI
    statusLabel_->setText(statusText_);
#endif
  }

  void setUpMouseManip_(simVis::Viewer* viewer)
  {
    if (viewer == nullptr || viewer->getSceneManager() == nullptr || !mouseDispatcher_)
      return;
    mouseManip_.reset(new simUtil::MousePositionManipulator(viewer->getSceneManager()->getMapNode(), viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks")));
    mouseManip_->setTerrainResolution(0.0001);
    mouseDispatcher_->setViewManager(viewer);
    mouseDispatcher_->addManipulator(0, mouseManip_);
    mouseManip_->addListener(latLonElevListener_.get(), showElevation_);
  }

  osg::ref_ptr<simVis::Viewer> viewer_;
#ifndef HAVE_IMGUI
  osg::observer_ptr<ui::LabelControl> statusLabel_;
#endif
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
  osgEarth::optional<size_t> centeredGogIndex_;
  osg::ref_ptr<simVis::PlatformNode> platform_;
  simVis::GOG::AltitudeMode altMode_;
  std::string statusText_;
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

#ifdef HAVE_IMGUI
struct ControlPanel : public GUI::BaseGui
{
  explicit ControlPanel(MouseAndMenuHandler& handler)
    : GUI::BaseGui("GOG Example"),
    handler_(handler)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text(s_help.c_str());
    ImGui::Text(handler_.statusText().c_str());

    float opacity = opacity_;
    ImGui::Text("Opacity: "); ImGui::SameLine();
    ImGui::SliderFloat("##Opacity", &opacity_, 0.f, 100.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
    if (opacity != opacity_)
    {
      float zeroToOne = opacity * 0.01f;
      // Set the override color on all nodes based on the provided opacity
      for (const auto& overlay : s_overlayNodes)
        overlay->setOpacity(zeroToOne);
    }

    ImGui::End();
  }

private:
  MouseAndMenuHandler& handler_;
  float opacity_ = 100.f;
};
#else
/** Process changes on the opacity slider. */
class OpacitySliderCallback : public ui::ControlEventHandler
{
public:
  void setLabel(ui::LabelControl* label)
  {
    label_ = label;
  }

  virtual void onValueChanged(ui::Control* control, float value) override
  {
    // Write the percentage to the label
    osg::ref_ptr<ui::LabelControl> label;
    if (label_.lock(label))
      label->setText(std::to_string(static_cast<int>((value * 100) + 0.5f)) + "%");

    // Set the override color on all nodes based on the provided opacity
    for (const auto& overlay : s_overlayNodes)
      overlay->setOpacity(value);
  }

private:
  osg::observer_ptr<ui::LabelControl> label_;
};
#endif

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
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  if (ap.read("--sky"))
    simExamples::addDefaultSkyNode(viewer.get());

  bool mark = ap.read("--mark");
  bool showElevation = ap.read("--showElevation");
  bool attach = ap.read("--attach");

  // parse the remaining args
  std::vector<std::string> gogFiles;
  std::string iconFile = EXAMPLE_IMAGE_ICON;
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--iconFile" && argc > i)
      iconFile = argv[++i];
    else
      gogFiles.push_back(arg);
  }

  osg::ref_ptr<osg::Image> pin;
  if (mark)
    pin = URI("http://www.osgearth.org/chrome/site/pushpin_yellow.png").getImage();

  GeoPoint go;
  // data source that provides positions for the platform based on the simulation time
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform
  simData::ObjectId platformId = addPlatform(dataStore, iconFile);
  osg::ref_ptr<simVis::PlatformNode> platform = scene->getScenario()->find<simVis::PlatformNode>(platformId);

  osg::Group* group = new osg::Group();

  // add the gog file vector layers.
  for (const std::string& gogFile : gogFiles)
  {
    simCore::GOG::Parser parser;
    simVis::GOG::Loader loader(parser, scene->getMapNode());
    // sets a default reference location for relative GOGs:
    loader.setReferencePosition(simCore::GOG::BSTUR);

    std::ifstream is(simCore::streamFixUtf8(gogFile));
    if (!is.is_open())
    {
      std::string fileName(gogFile);
      SIM_ERROR <<"Could not open GOG file " << gogFile << "\n";
      return 1;
    }

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadGogs(is, gogFile, attach, gogs);

    if (!gogs.empty())
    {
      int followIndex = 0;
      for (auto i = gogs.begin(); i != gogs.end(); ++i)
      {
        GogNodeInterfacePtr gogInterface(*i);
        osg::Node* gog = (*i)->osgNode();

        // attached GOGs get added to a locator based on the host platform
        if (attach)
        {
          // TODO: SIM-13358- incorporate this logic into simVis PlatformNode
          bool followYaw = false;
          bool followPitch = false;
          bool followRoll = false;
          const simCore::GOG::GogShape* shape = (*i)->shapeObject();
          if (shape)
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

          osg::ref_ptr<GeoPositionNode> marker;
          if (label.empty())
          {
            PlaceNode* place = new PlaceNode();
            place->setIconImage(pin.get());
            marker = place;
            marker->setMapNode(scene->getMapNode());
          }
          else
            marker = new LabelNode(label);

          marker->setMapNode(scene->getMapNode());
          marker->setPosition(location);

          scene->getScenario()->addChild(marker);

          go = location;
        }
        followIndex++;
      }
    }
    else
    {
      SIM_WARN << "Unable to load GOG data from \"" << gogFile << "\"" << std::endl;
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

#ifndef HAVE_IMGUI
  // add help and status labels
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(s_help, 14, simVis::Color::Silver));
  ui::LabelControl* statusLabel = new ui::LabelControl("STATUS", 14, simVis::Color::Silver);
  vbox->addControl(statusLabel);

  // Add a section to control the opacity
  vbox->addControl(new ui::LabelControl("Opacity:", 14));
  OpacitySliderCallback* sliderCallback = new OpacitySliderCallback;
  ui::HSliderControl* opacitySlider = new ui::HSliderControl(0.f, 1.f, 1.f, sliderCallback);
  opacitySlider->setHorizFill(false);
  opacitySlider->setWidth(250.f);
  ui::LabelControl* opacityPercent = new ui::LabelControl("100%", 14);
  sliderCallback->setLabel(opacityPercent);
  ui::HBox* hbox = new ui::HBox();
  hbox->addControl(opacitySlider);
  hbox->addControl(opacityPercent);
  vbox->addControl(hbox);

  mainView->addOverlayControl(vbox);
#endif

  // Install a handler to respond to the demo keys in this sample.
  osg::ref_ptr<MouseAndMenuHandler> mouseHandler =
    new MouseAndMenuHandler(
      viewer.get(),
#ifndef HAVE_IMGUI
      statusLabel,
#endif
      dataStore,
      showElevation,
      attach ? platform.get() : nullptr);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(*mouseHandler.get()));
#endif

  mainView->getCamera()->addEventCallback(mouseHandler);
  viewer->run();
}
