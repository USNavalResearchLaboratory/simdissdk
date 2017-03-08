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
 * Demonstrates loading an osgEarth .earth file at runtime, showing how you
 * can swap terrain configurations on the fly.
 */
#include "osgEarthUtil/Controls"
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simVis/Viewer.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/Platform.h"
#include "simUtil/DbConfigurationFile.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

using namespace osgEarth::Util::Controls;

static std::vector<std::string> earthFiles;
static unsigned earthFileIndex = 0;

struct earthFileLoader
{
  static void loadEarthFile(const std::string& earthFile, simVis::Viewer* viewer, bool mapOnly =false);
};

//----------------------------------------------------------------------------

static std::string s_title =
  "Load Earth File Example";

static std::string s_help =
  "1 : load next earth file\n"
  "2 : load next earth file (map only)\n"
  "e : toggle show lat/lon/elevation\n"
  "i : toggle add-inset mouse mode\n"
  "r : remove all insets\n";


static Control* createHelp(LabelControl* elevationLabel)
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  vbox->addControl(elevationLabel);
  return vbox;
}


void earthFileLoader::loadEarthFile(const std::string& earthFile,
                                    simVis::Viewer*    viewer,
                                    bool               mapOnly)
{
  // Load the map -- note use of readEarthFile() to configure default options (vs osgDB::readNodeFile() directly)
  osg::ref_ptr<osg::Node> loadedModel = simUtil::DbConfigurationFile::readEarthFile(earthFile);

  // Find the MapNode and replace it.
  osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(loadedModel);
  if (mapNode)
  {
    if (mapOnly)
      viewer->setMap(mapNode->getMap());
    else
      viewer->setMapNode(mapNode);
  }
}


// A mouse position listener to update the elevation label with the current lat/lon/elevation value under the mouse
class LatLonElevationListener : public simUtil::MousePositionManipulator::Listener
{
public:
  explicit LatLonElevationListener(LabelControl* elevationLabel)
    : elevationLabel_(elevationLabel),
      showLatLonElevation_(false),
      lastElevation_(0.0)
  {
  }
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
    elevationLabel_->setText(os.str());
  }

  void showLatLonElevation(bool show)
  {
    if (show == showLatLonElevation_)
      return;
    showLatLonElevation_ = show;

    if (!show)
      elevationLabel_->setText("");
  }

private:
  LabelControl* elevationLabel_;
  bool showLatLonElevation_;
  double lastElevation_;
};

/// An event handler to assist in testing the InsetViewManager / Load Earth functionality.
struct MenuHandler : public osgGA::GUIEventHandler
{
  MenuHandler(simVis::Viewer* viewer, simVis::InsetViewEventHandler* handler, LabelControl* elevationLabel, simUtil::MouseDispatcher* mouseDispatcher)
   : viewer_(viewer),
     handler_(handler),
     latLonElevListener_(NULL),
     showLatLonElevation_(false)
  {
    latLonElevListener_ = new LatLonElevationListener(elevationLabel);
    mouseDispatcher_ = mouseDispatcher;
    mouseDispatcher_->setViewManager(NULL);
    setUpMouseManip_(viewer_);
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case 'r': // REMOVE ALL INSETS.
      {
        simVis::View::Insets insets;
        viewer_->getMainView()->getInsets(insets);
        for (simVis::View::Insets::const_iterator i = insets.begin(); i != insets.end(); ++i)
          viewer_->getMainView()->removeInset(*i);
        SIM_NOTICE << "Removed all insets..." << std::endl;
        handled = true;
        break;
      }

      case 'i': // TOGGLE INSERT INSET MODE
        handler_->setAddInsetMode(!handler_->isAddInsetMode());
        handled = true;
        break;

      case 'l': // LOAD EARTH FILE
      case '1':
        mouseManip_->removeListener(latLonElevListener_);
        earthFileIndex = (earthFileIndex + 1) % earthFiles.size();
        earthFileLoader::loadEarthFile(earthFiles[earthFileIndex], viewer_, false);
        setUpMouseManip_(viewer_);
        handled = true;
        break;

      case '2': // LOAD EARTH FILE, MAP ONLY
        mouseManip_->removeListener(latLonElevListener_);
        earthFileIndex = (earthFileIndex + 1) % earthFiles.size();
        earthFileLoader::loadEarthFile(earthFiles[earthFileIndex], viewer_, true);
        setUpMouseManip_(viewer_);
        handled = true;
        break;

      case 'e': // TOGGLE SHOW LAT/LON/ELEVATION
        // always remove listener
        mouseManip_->removeListener(latLonElevListener_);
        showLatLonElevation_ = !showLatLonElevation_;
        // if showing elevation, add the elevation mouse listener
        if (showLatLonElevation_)
          mouseManip_->addListener(latLonElevListener_, true);
        latLonElevListener_->showLatLonElevation(showLatLonElevation_);
        handled = true;
        break;
      }
    }
    return handled;
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

  simVis::Viewer* viewer_;
  simVis::InsetViewEventHandler* handler_;
  LatLonElevationListener* latLonElevListener_;
  bool showLatLonElevation_;
  simUtil::MouseDispatcher* mouseDispatcher_;
  std::tr1::shared_ptr<simUtil::MousePositionManipulator> mouseManip_;
};


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

  if (!earthFiles.empty())
  {
    earthFileLoader::loadEarthFile(earthFiles[0], viewer);
  }
  else
  {
    osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
    viewer->setMap(map);
  }

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  // Add an entity flying around
  osg::ref_ptr<simUtil::CircumnavigationPlatformSimulation> platformSim = new simUtil::CircumnavigationPlatformSimulation(viewer->getSceneManager(), mainView);
  simVis::Viewpoint vp;
  vp.heading()->set(20, osgEarth::Units::DEGREES);
  vp.pitch()->set(-60, osgEarth::Units::DEGREES);
  vp.range()->set(10000000, osgEarth::Units::METERS);
  mainView->tetherCamera(platformSim->platformNode(), vp, 0);

  // label for elevation readout
  osg::ref_ptr<LabelControl> elevationLabel = new LabelControl("", 14, osg::Vec4f(.8, .8, .8, 1));
  std::tr1::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher;
  mouseDispatcher.reset(new simUtil::MouseDispatcher);

  // Handles hotkeys from user
  mainView->addEventHandler(new MenuHandler(viewer, insetHandler, elevationLabel, mouseDispatcher.get()));

  // show the help menu
  viewer->getMainView()->addOverlayControl(createHelp(elevationLabel));
  viewer->installDebugHandlers();
  viewer->run();
}
