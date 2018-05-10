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
 * BasicViewer shows how to create, configure, and control the simVis main
 * map control. It shows how to adjust window appearance, how to add or remove
 * inset views, and how to change the motion model.
 */

#include "osgEarthUtil/Controls"
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simVis/Compass.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/NavigationModes.h"
#include "simUtil/ExampleResources.h"

#include "osgEarthDrivers/gdal/GDALOptions"
#include "osgDB/ReadFile"

#define LC "[BasicViewer demo] "

namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static std::string s_title = "Viewer Example";
static std::string s_help =
  "i : toggles the mode for creating a new inset\n"
  "v : toggle visibility of all insets\n"
  "r : remove all insets \n"
  "1 : activate 'Perspective' navigation mode \n"
  "2 : activate 'Overhead' navigation mode \n"
  "3 : activate 'GIS' navigation mode \n"
  "h : toggle between click-to-focus and hover-to-focus \n"
  "l : toggle sky lighting \n"
  "tab : cycle focus (in click-to-focus mode only) \n";

static ui::Control* createHelp()
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(s_help, 14, simVis::Color::Silver));
  return vbox;
}

//----------------------------------------------------------------------------
// Demonstrates the use of the simVis::ViewManager::ViewCallback to respond to
// view events.
struct ViewReportCallback : public simVis::ViewManager::Callback
{
  void operator()(simVis::View* view, const EventType& e)
  {
    switch (e)
    {
      case VIEW_ADDED:
        SIM_NOTICE << LC << "View '" << view->getName() << "' added" << std::endl;
        break;

      case VIEW_REMOVED:
        SIM_NOTICE << LC << "View '" << view->getName() << "' removed" << std::endl;
        break;
    }
  }
};

//----------------------------------------------------------------------------

// An event handler to assist in testing the Inset functionality.
struct MenuHandler : public osgGA::GUIEventHandler
{
  MenuHandler(simVis::Viewer* viewer, simVis::InsetViewEventHandler* insetViewHandler, simVis::CreateInsetEventHandler* createHandler)
  : viewer_(viewer),
    insetViewHandler_(insetViewHandler),
    createHandler_(createHandler)
  {
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
          for (unsigned int i = 0; i < insets.size(); ++i)
            viewer_->getMainView()->removeInset(insets[i].get());

          SIM_NOTICE << LC << "Removed all insets." << std::endl;
          handled = true;
        }
        break;

        case 'h': // TOGGLE BETWEEN HOVER-TO-FOCUS and CLICK-TO-FOCUS
        {
          int mask = insetViewHandler_->getFocusActions();
          bool hover = (mask & simVis::InsetViewEventHandler::ACTION_HOVER) != 0;
          if (hover)
          {
            mask = simVis::InsetViewEventHandler::ACTION_CLICK_SCROLL | simVis::InsetViewEventHandler::ACTION_TAB;
            SIM_NOTICE << LC << "Switched to click-to-focus mode." << std::endl;
          }
          else
          {
            mask = simVis::InsetViewEventHandler::ACTION_HOVER;
            SIM_NOTICE << LC << "Switched to hover-to-focus mode." << std::endl;
          }
          insetViewHandler_->setFocusActions(mask);
          handled = true;
        }
        break;

        case 'i':
          createHandler_->setEnabled(!createHandler_->isEnabled());
          break;

        case '1': // ACTIVATE PERSPECTIVE NAV MODE
          viewer_->getMainView()->enableOverheadMode(false);
          viewer_->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
          handled = true;
          break;

        case '2': // ACTIVATE OVERHEAD NAV MODE
          viewer_->getMainView()->enableOverheadMode(true);
          viewer_->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
          handled = true;
        break;

        case '3': // ACTIVATE GIS NAV MODE
          viewer_->setNavigationMode(simVis::NAVMODE_GIS);
          handled = true;
        break;

        case 'l': // SKY LIGHTING
        {
          osgEarth::Util::SkyNode* sky = viewer_->getSceneManager()->getSkyNode();
          if (sky)
          {
            osg::StateAttribute::OverrideValue ov = sky->getLighting();
            ov = (ov & osg::StateAttribute::ON) ? osg::StateAttribute::OFF : osg::StateAttribute::ON;
            sky->setLighting(ov);
            aa.requestRedraw();
          }
        }
        break;

        case 'v': // TOGGLE VISIBILITY of ALL INSETS (for testing)
        {
          simVis::View* main = viewer_->getMainView();
          for (unsigned i = 0; i < main->getNumInsets(); ++i)
          {
             simVis::View* inset = main->getInset(i);
             inset->setVisible(!inset->isVisible());
          }
          aa.requestRedraw();
          handled = true;
        }
        break;

        case 'o':
        {
          simVis::View* main = viewer_->getMainView();
          main->enableOrthographic(!main->isOrthographicEnabled());
          aa.requestRedraw();
          handled = true;
        }
        break;
      }
    }
    return handled;
  }

private:
  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::InsetViewEventHandler> insetViewHandler_;
  osg::observer_ptr<simVis::CreateInsetEventHandler> createHandler_;
};

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Demonstrate the view callback. This notifies us whenever new inset views are created or
  // removed or get focus.
  viewer->addCallback(new ViewReportCallback());

  // Demonstrate the view-drawing service.  This is used to create new inset views with the mouse.
  simVis::View* mainView = viewer->getMainView();
  osg::ref_ptr<simVis::InsetViewEventHandler> insetFocusHandler = new simVis::InsetViewEventHandler(mainView);
  mainView->addEventHandler(insetFocusHandler);
  osg::ref_ptr<simVis::CreateInsetEventHandler> createInsetsHandler = new simVis::CreateInsetEventHandler(mainView);
  mainView->addEventHandler(createInsetsHandler);

  // Install a handler to respond to the demo keys in this sample.
  mainView->getCamera()->addEventCallback(new MenuHandler(viewer.get(), insetFocusHandler.get(), createInsetsHandler.get()));

  // set an initial viewpoint
  mainView->lookAt(45, 0, 0, 0, -89, 12e6);

  dynamic_cast<osgEarth::Util::EarthManipulator*>(mainView->getCameraManipulator())
      ->getSettings()->setTerrainAvoidanceEnabled(false);

  simVis::Viewpoint viewPoint("Start",
      -159.87757019550978, 22.525663828229778, 13361.200000000001,
      359.99969592100859, 2.5436404019053387, 81514.399999999994);
  mainView->setViewpoint(viewPoint);

  // create a compass image control, add it to the HUD/Overlay
  osg::ref_ptr<simVis::Compass> compass = new simVis::Compass("compass.png");
  compass->setDrawView(mainView);
  // create an adapter to let compass display heading for current focused view
  simVis::CompassFocusManagerAdapter adapter(mainView->getFocusManager(), compass.get());

  // show the help menu
  //mainView->addOverlayControl(createHelp());

  simVis::View* hud = new simVis::View();
  hud->setUpViewAsHUD(mainView);
  mainView->getViewManager()->addView(hud);
  hud->addOverlayControl(createHelp());

  // for status and debugging
  viewer->installDebugHandlers();

  viewer->run();
}

