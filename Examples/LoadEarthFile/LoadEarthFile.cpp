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
 * Demonstrates loading an osgEarth .earth file at runtime, showing how you
 * can swap terrain configurations on the fly.
 */
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simVis/Viewer.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/Platform.h"
#include "simVis/SceneManager.h"
#include "simUtil/DbConfigurationFile.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

static std::vector<std::string> earthFiles;
static unsigned earthFileIndex = 0;

struct earthFileLoader
{
  static void loadEarthFile(const std::string& earthFile, simVis::Viewer* viewer, bool mapOnly =false);
};

//----------------------------------------------------------------------------

static std::string s_title =
  "Load Earth File Example";

// A mouse position listener to update the elevation label with the current lat/lon/elevation value under the mouse
class LatLonElevationListener : public simUtil::MousePositionManipulator::Listener
{
public:
#ifdef HAVE_IMGUI
  explicit LatLonElevationListener(std::string& elevationLabel)
    : elevationLabel_(elevationLabel),
    showLatLonElevation_(false),
    lastElevation_(0.0)
  {
  }
#endif
  virtual void mouseOverLatLon(double lat, double lon, double elev)
  {
    if (!showLatLonElevation_)
      return;
    std::ostringstream os;
    os << "Lat: " << lat << ", Lon: " << lon;
    os << ", Elevation: ";
    if (elev == simUtil::MousePositionManipulator::INVALID_POSITION_VALUE)
      os << "INVALID";
    else
      os << elev;
#ifdef HAVE_IMGUI
    elevationLabel_ = os.str();
#endif
  }

  void showLatLonElevation(bool show)
  {
    if (show == showLatLonElevation_)
      return;
    showLatLonElevation_ = show;

#ifdef HAVE_IMGUI
    if (!show)
      elevationLabel_.clear();
#endif
  }

private:
#ifdef HAVE_IMGUI
  std::string& elevationLabel_;
#endif
  bool showLatLonElevation_;
  double lastElevation_;
};

#ifdef HAVE_IMGUI

struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel(simVis::Viewer* viewer, simVis::CreateInsetEventHandler* handler, simUtil::MouseDispatcher* mouseDispatcher)
    : simExamples::SimExamplesGui(s_title),
    viewer_(viewer),
    handler_(handler),
    latLonElevListener_(nullptr),
    showLatLonElevation_(false),
    mouseDispatcher_(mouseDispatcher)
  {
    latLonElevListener_ = new LatLonElevationListener(llaLabel_);
    mouseDispatcher_->setViewManager(nullptr);
    setUpMouseManip_(viewer_.get());

    addKeyFunc_(ImGuiKey_R, [this]()
      {
      });

    addKeyFunc_(ImGuiKey_R, [this]()
      {
        simVis::View::Insets insets;
        if (viewer_.valid())
        {
          viewer_->getMainView()->getInsets(insets);
          for (simVis::View::Insets::const_iterator i = insets.begin(); i != insets.end(); ++i)
            viewer_->getMainView()->removeInset(i->get());
        }
        SIM_NOTICE << "Removed all insets..." << std::endl;
      });
    addKeyFunc_(ImGuiKey_I, [this]() { if (handler_.valid()) handler_->setEnabled(!handler_->isEnabled()); });
    addKeyFunc_(ImGuiKey_1, [this]()
      {
        mouseManip_->removeListener(latLonElevListener_);
        earthFileIndex = (earthFileIndex + 1) % earthFiles.size();
        earthFileLoader::loadEarthFile(earthFiles[earthFileIndex], viewer_.get(), false);
        setUpMouseManip_(viewer_.get());
      });
    addKeyFunc_(ImGuiKey_2, [this]()
      {
        mouseManip_->removeListener(latLonElevListener_);
        earthFileIndex = (earthFileIndex + 1) % earthFiles.size();
        earthFileLoader::loadEarthFile(earthFiles[earthFileIndex], viewer_.get(), true);
        setUpMouseManip_(viewer_.get());
      });
    addKeyFunc_(ImGuiKey_E, [this]()
      {
        // always remove listener
        mouseManip_->removeListener(latLonElevListener_);
        showLatLonElevation_ = !showLatLonElevation_;
        // if showing elevation, add the elevation mouse listener
        if (showLatLonElevation_)
          mouseManip_->addListener(latLonElevListener_, true);
        latLonElevListener_->showLatLonElevation(showLatLonElevation_);
      });
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    if (firstDraw_)
    {
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("1 : load next earth file");
    ImGui::Text("2 : load next earth file (map only)");
    ImGui::Text("e : toggle show lat/lon/elevation");
    ImGui::Text("i : toggle add-inset mouse mode");
    ImGui::Text("r : remove all insets");

    if (!llaLabel_.empty())
      ImGui::Text("%s", llaLabel_.c_str());

    ImGui::End();
    handlePressedKeys_();
  }


private:
  // create a new mouse position manipulator for the specified viewer, set the viewer as view manager in the mouse dispatcher
  void setUpMouseManip_(simVis::Viewer* viewer)
  {
    mouseManip_.reset(new simUtil::MousePositionManipulator(viewer->getSceneManager()->getMapNode(), viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks")));
    mouseManip_->setTerrainResolution(0.0001);
    mouseDispatcher_->setViewManager(viewer);
    mouseDispatcher_->addManipulator(0, mouseManip_);
    if (showLatLonElevation_)
      mouseManip_->addListener(latLonElevListener_, true);
  }

  osg::observer_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::CreateInsetEventHandler> handler_;
  LatLonElevationListener* latLonElevListener_;
  bool showLatLonElevation_;
  simUtil::MouseDispatcher* mouseDispatcher_;
  std::shared_ptr<simUtil::MousePositionManipulator> mouseManip_;
  std::string llaLabel_;
};

#endif

void earthFileLoader::loadEarthFile(const std::string& earthFile,
                                    simVis::Viewer*    viewer,
                                    bool               mapOnly)
{
  // Load the map -- note use of readEarthFile() to configure default options (vs osgDB::readNodeFile() directly)
  osg::ref_ptr<osg::Node> loadedModel = simUtil::DbConfigurationFile::readEarthFile(earthFile);

  // Find the MapNode and replace it.
  osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(loadedModel.get());
  if (mapNode)
  {
    if (mapOnly)
      viewer->setMap(mapNode->getMap());
    else
      viewer->setMapNode(mapNode.get());
  }
}

int main(int argc, char** argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  if (argc < 3)
  {
    std::cerr << "USAGE: pass in earth files on command line: \n"
      << " --earthFiles <file1> <file2> ...\n";
    return 0;
  }

  for (int index = 0; index < argc; index++)
  {
    std::string arg = argv[index];
    if (arg == "--earthFiles")
    {
      int newIndex = index+1;
      while (newIndex < argc)
      {
        std::string earthFile = argv[newIndex];
        earthFiles.push_back(earthFile);
        newIndex++;
      }
      index = newIndex;
    }
  }

  if (earthFiles.empty())
  {
    std::cerr << "USAGE: pass in earth files on command line: \n"
      << " --earthFiles <file1> <file2> ...\n";
    return 0;
  }

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  // inset view support
  simVis::View* mainView = viewer->getMainView();
  osg::ref_ptr<simVis::InsetViewEventHandler> insetHandler = new simVis::InsetViewEventHandler(mainView);
  mainView->addEventHandler(insetHandler);
  osg::ref_ptr<simVis::CreateInsetEventHandler> createInsetsHandler = new simVis::CreateInsetEventHandler(mainView);
  mainView->addEventHandler(createInsetsHandler);

  if (!earthFiles.empty())
  {
    earthFileLoader::loadEarthFile(earthFiles[0], viewer.get());
  }
  else
  {
    osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
    viewer->setMap(map.get());
  }

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Add an entity flying around
  osg::ref_ptr<simUtil::CircumnavigationPlatformSimulation> platformSim = new simUtil::CircumnavigationPlatformSimulation(viewer->getSceneManager(), mainView);
  simVis::Viewpoint vp;
  vp.heading()->set(20, osgEarth::Units::DEGREES);
  vp.pitch()->set(-60, osgEarth::Units::DEGREES);
  vp.range()->set(10000000, osgEarth::Units::METERS);
  mainView->tetherCamera(platformSim->platformNode(), vp, 0);

  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher;
  mouseDispatcher.reset(new simUtil::MouseDispatcher);

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(viewer.get(), createInsetsHandler.get(), mouseDispatcher.get()));
#endif

  viewer->installDebugHandlers();
  return viewer->run();
}
