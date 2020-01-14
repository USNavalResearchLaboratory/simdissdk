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
 * BasicViewerText is the BasicViewer example with Help overlay removed and various HudManager HudText elements added.
 * It demonstrates HudText layout behaviors.
 */

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/String/Utils.h"
#include "simCore/String/TextReplacer.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Viewer.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/NavigationModes.h"
#include "simVis/ClassificationBanner.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/HudManager.h"
#include "simUtil/Replaceables.h"
#include "simUtil/StatusText.h"
#include "osgDB/ReadFile"

#define LC "[BasicViewerText demo] "

namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static std::string s_title =
  "Viewer with HudManager Text Example \n";

static std::string s_help =
  "i : toggles the mode for creating a new inset\n"
  "r : remove all inset views \n"
  "1 : activate 'Perspective' navigation mode \n"
  "2 : activate 'Overhead' navigation mode \n"
  "3 : activate 'GIS' navigation mode \n"
  "h : toggle between click-to-focus and hover-to-focus \n"
  "l : toggle sky lighting \n"
  "tab : cycle focus (in click-to-focus mode only) \n"
  "v : create viewport (doesn't obscure text)\n"
  "b : create viewport (blown up, doesn't obscure)\n";

static ui::Control* createHelp()
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(s_help, 14, simVis::Color::Silver));
  // Move it down just a bit
  vbox->setPosition(10, 40);
  return vbox;
}


// In BasicViewer, initial view window size is full screen size, and no resize events occur at startup
// This results in bad positioning of text until first resize event is prompted by user action

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

      case 'v':
        createInset(1, 1, 50, 40);
        break;

      case 'b':
        createInset(1, 1, 98, 98);
        break;
      }
    }
    return handled;
  }

  void createInset(float x, float y, float w, float h)
  {
    simVis::View* inset = new simVis::View();
    inset->setName("Inset");
    inset->setExtentsAsRatio(x * 0.01, y * 0.01, w * 0.01, h * 0.01);
    inset->setSceneManager(viewer_->getMainView()->getSceneManager());
    // Apply EarthManipulator settings from parent view to our new inset
    inset->applyManipulatorSettings(*viewer_->getMainView());
    viewer_->getMainView()->addInset(inset);
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

  // add sky node
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

  // show the help menu, using HudManager
  simVis::View* superHUD = new simVis::View();
  superHUD->setUpViewAsHUD(mainView);
  mainView->getViewManager()->addView(superHUD);
  simUtil::HudManager hm(superHUD);

  // Create a background for some of the text using a large hyphen
  osg::ref_ptr<simUtil::HudText> background1 = hm.createText("-", 130, 132, false, false);
  background1->setBackdropType(osgText::Text::NONE);
  background1->setFont("arialbd.ttf", 520);
  background1->setColor(osg::Vec4(0,0.6,0.6,1)); // Cyan-ish, but darker

  osg::ref_ptr<simUtil::HudText> title = hm.createText(s_title, 50.0, 99.0, true, simUtil::ALIGN_CENTER_X, simUtil::ALIGN_CENTER_Y, simVis::Color::Yellow, "arial.ttf", 20.0);

  osg::ref_ptr<simUtil::HudText> abs1 = hm.createText("ABS 300/300\nALIGN_BOTTOM\nALIGN_LEFT", 300.0, 300.0, false, simUtil::ALIGN_LEFT, simUtil::ALIGN_BOTTOM, simVis::Color::Lime, "arial.ttf", 20);
  osg::ref_ptr<simUtil::HudText> abs2 = hm.createText("ABS 300/300\nALIGN_TOP\nALIGN_RIGHT", 300.0, 300.0, false, simUtil::ALIGN_RIGHT, simUtil::ALIGN_TOP, simVis::Color::Red, "arial.ttf", 20);

  simCore::TextReplacerPtr textReplacer(new simCore::TextReplacer());
  textReplacer->addReplaceable(new simUtil::AzimuthVariable(mainView));
  textReplacer->addReplaceable(new simUtil::ElevationVariable(mainView));
  textReplacer->addReplaceable(new simUtil::LatitudeVariable(mainView));
  textReplacer->addReplaceable(new simUtil::LongitudeVariable(mainView));
  textReplacer->addReplaceable(new simUtil::AltitudeVariable(mainView));

  // Show a compass RGB in the top right
  osg::ref_ptr<simUtil::HudImage> hudImage = hm.createImage(osgDB::readImageFile("compass.rgb"), 85, 85, 15, 15);

  osg::ref_ptr<simUtil::StatusText> status1 = new simUtil::StatusText(superHUD, textReplacer, simUtil::StatusText::LEFT_BOTTOM);
  status1->setStatusSpec("Azim:\t%AZ%\tLat:\t%LAT%\tAlt:\t%ALT%\nElev:\t%EL%\tLon:\t%LON%\t \t \n");

  osg::ref_ptr<simUtil::StatusText> status2 = new simUtil::StatusText(superHUD, textReplacer, simUtil::StatusText::LEFT_CENTER);
  status2->setStatusSpec("Azim:\t%AZ%\tLat:\t%LAT%\nElev:\t%EL%\tLon:\t%LON%\n");

  osg::ref_ptr<simUtil::StatusText> status3 = new simUtil::StatusText(superHUD, textReplacer, simUtil::StatusText::LEFT_TOP);
  status3->setStatusSpec("Lat:\t%LAT%\tAzim:\t%AZ%\nLon:\t%LON%\tElev:\t%EL%\n");

  // Add a classification banner
  simData::MemoryDataStore dataStore;
  {
    simData::DataStore::Transaction txn;
    simData::ScenarioProperties* props = dataStore.mutable_scenarioProperties(&txn);
    props->mutable_classification()->set_fontcolor(0x00ff0080); // Transparent green, RRGGBBAA
    props->mutable_classification()->set_label("UNCLASSIFIED");
    txn.complete(&props);
  }
  osg::ref_ptr<simVis::ClassificationBanner> banner = new simVis::ClassificationBanner(&dataStore, 24, "arialbd.ttf");
  banner->addToView(superHUD);

  // Add a help control
  superHUD->addOverlayControl(createHelp());

  // for status and debugging
  viewer->installDebugHandlers();

  viewer->run();
}

