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

#include "osgEarth/CullingUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/String/TextReplacer.h"
#include "simCore/String/Utils.h"
#include "simData/MemoryDataStore.h"
#include "simVis/ClassificationBanner.h"
#include "simVis/Compass.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/HudPositionEditor.h"
#include "simUtil/HudPositionManager.h"
#include "simUtil/MapScale.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/Replaceables.h"
#include "simUtil/StatusText.h"

namespace ui = osgEarth::Util::Controls;

static const std::string KEY_MAP_SCALE = "MapScale";
static const std::string KEY_STATUS_TEXT = "StatusText";
static const std::string KEY_DEMO_TEXT = "DemoText";
static const std::string KEY_CLASSIFICATION_TOP = "ClassificationTop";
static const std::string KEY_CLASSIFICATION_BOTTOM = "ClassificationBottom";
static const std::string KEY_COMPASS = "Compass";

//----------------------------------------------------------------------------

static std::string s_title =
  "HUD Position Manager Example \n";

static std::string s_help =
  "1 : Move 'Demo Text' to the mouse position\n"
  "2 : Move 'Map Scale' to the mouse position\n"
  "3 : Move 'Status Text' to mouse position\n"
  "4 : Move 'Top Classification' to mouse position\n"
  "5 : Move 'Bottom Classification' to mouse position\n"
  "6 : Move 'Compass' to mouse position\n"
  "c : Cycle classification string and color\n"
  "e : Toggle HUD Editor mode\n"
  "r : Reset all to default positions\n"
  "w : Toggle Wind Vane on Compass\n"
  "z : Cycle wind angle and speed values\n"
  ;

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

//----------------------------------------------------------------------------

// An event handler to assist in testing the Inset functionality.
struct MenuHandler : public osgGA::GUIEventHandler
{
  MenuHandler(simUtil::HudPositionEditor& hudEditor, simData::DataStore& ds)
    : hudEditor_(hudEditor),
      dataStore_(ds),
      classificationCycle_(0),
      windCycle_(0)
  {
  }

  void setCompass(simVis::CompassNode* compass)
  {
    compass_ = compass;
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    if (ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN)
      return false;

    bool handled = false;
    std::string windowName;
    // Figure out which key was pressed
    switch (ea.getKey())
    {
    case '1':
      windowName = KEY_DEMO_TEXT;
      break;
    case '2':
      windowName = KEY_MAP_SCALE;
      break;
    case '3':
      windowName = KEY_STATUS_TEXT;
      break;
    case '4':
      windowName = KEY_CLASSIFICATION_TOP;
      break;
    case '5':
      windowName = KEY_CLASSIFICATION_BOTTOM;
      break;
    case '6':
      windowName = KEY_COMPASS;
      break;

    case 'c':
    {
      // Cycle through a few different classification strings
      classificationCycle_ = (++classificationCycle_ % 3);
      simData::DataStore::Transaction txn;
      simData::ScenarioProperties* props = dataStore_.mutable_scenarioProperties(&txn);
      switch (classificationCycle_)
      {
      case 0:
        props->mutable_classification()->set_label("UNCLASSIFIED");
        props->mutable_classification()->set_fontcolor(0x00ff0080);
        break;
      case 1:
        props->mutable_classification()->set_label("U N C L A S S I F I E D");
        props->mutable_classification()->set_fontcolor(0xffffff80);
        break;
      case 2:
        props->mutable_classification()->set_label("YOUR STRING HERE");
        props->mutable_classification()->set_fontcolor(0xffff0080);
        break;
      }
      txn.complete(&props);
      handled = true;
      break;
    }

    case 'e':
      hudEditor_.setVisible(!hudEditor_.isVisible());
      handled = true;
      break;

    case 'r':
      hudEditor_.resetAllPositions();
      handled = true;
      break;

    case 'w':
      if (compass_.valid())
      {
        compass_->setWindVaneVisible(!compass_->isWindVaneVisible());
        handled = true;
      }
      break;

    case 'z':
    {
      // Cycle through a few different wind settings
      windCycle_ = (++windCycle_ % 3);
      simData::DataStore::Transaction txn;
      simData::ScenarioProperties* props = dataStore_.mutable_scenarioProperties(&txn);
      switch (windCycle_)
      {
      case 0:
        props->set_windangle(35.0 * simCore::DEG2RAD);
        props->set_windspeed(11.0);
        break;
      case 1:
        props->set_windangle(282.0 * simCore::DEG2RAD);
        props->set_windspeed(6.0);
        break;
      case 2:
        props->set_windangle(179.625 * simCore::DEG2RAD);
        props->set_windspeed(36.15698);
        break;
      }
      txn.complete(&props);
      handled = true;
      break;
    }
    }

    // Assign the position to the mouse's location
    if (!windowName.empty())
    {
      // Rescale normalized from (-1,+1) to (0,1)
      const osg::Vec2d pos(0.5 * (1.0 + ea.getXnormalized()),
        0.5 * (1.0 + ea.getYnormalized()));
      hudEditor_.setPosition(windowName, pos);
      handled = true;
    }
    return handled;
  }

private:
  simUtil::HudPositionEditor& hudEditor_;
  osg::observer_ptr<simVis::CompassNode> compass_;
  simData::DataStore& dataStore_;
  int classificationCycle_;
  int windCycle_;
};

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // Create a mouse dispatcher for the HUD Editor
  simUtil::MouseDispatcher mouseDispatcher;
  mouseDispatcher.setViewManager(viewer);

  // Create a HUD position manager that will move on-screen objects
  simUtil::HudPositionEditor hudEditor;
  osg::ref_ptr<simUtil::HudPositionManager> hud = hudEditor.hud();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());
  // Demonstrate the view-drawing service.  This is used to create new inset views with the mouse.
  simVis::View* mainView = viewer->getMainView();
  // set an initial viewpoint
  mainView->lookAt(45, 0, 0, 0, -89, 12e6);

  // Create a "Super HUD" on top of all other views and insets
  simVis::View* superHUD = new simVis::View();
  superHUD->setUpViewAsHUD(mainView);
  // Add a help control
  superHUD->addOverlayControl(createHelp());
  mainView->getViewManager()->addView(superHUD);

  // For osgEarth::LineDrawable to work on SuperHUD, need an InstallViewportSizeUniform
  superHUD->getOrCreateHUD()->addCullCallback(new osgEarth::InstallViewportSizeUniform());
  // Configure the HUD Editor properly
  hudEditor.bindAll(*superHUD->getOrCreateHUD(), mouseDispatcher, -100);

  // Add a classification banner
  simData::MemoryDataStore dataStore;
  {
    simData::DataStore::Transaction txn;
    simData::ScenarioProperties* props = dataStore.mutable_scenarioProperties(&txn);
    props->mutable_classification()->set_fontcolor(0x00ff0080); // Transparent green, RRGGBBAA
    props->mutable_classification()->set_label("UNCLASSIFIED");
    props->set_windangle(35.0 * simCore::DEG2RAD);
    props->set_windspeed(11.0);
    txn.complete(&props);
  }

  // Install a handler to respond to the demo keys in this sample.
  MenuHandler* menuHandler = new MenuHandler(hudEditor, dataStore);
  mainView->getCamera()->addEventCallback(menuHandler);

  // Configure text replacement variables that will be used for status text
  simCore::TextReplacerPtr textReplacer(new simCore::TextReplacer());
  textReplacer->addReplaceable(new simUtil::AzimuthVariable(mainView));
  textReplacer->addReplaceable(new simUtil::ElevationVariable(mainView));
  textReplacer->addReplaceable(new simUtil::LatitudeVariable(mainView));
  textReplacer->addReplaceable(new simUtil::LongitudeVariable(mainView));
  textReplacer->addReplaceable(new simUtil::AltitudeVariable(mainView));

  {
    // Add status text
    osg::ref_ptr<simUtil::StatusTextNode> statusText = new simUtil::StatusTextNode(textReplacer);
    statusText->setStatusSpec("Azim:\t%AZ%\tLat:\t%LAT%\tAlt:\t%ALT%\nElev:\t%EL%\tLon:\t%LON%\t \t \n");
    superHUD->getOrCreateHUD()->addChild(statusText.get());
    hud->addWindow(KEY_STATUS_TEXT, osg::Vec2d(0.005, 0.005), new simUtil::RepositionMatrixPxCallback(statusText.get()));
    // Estimate the size.  No need to be exact at this time.
    hud->setSize(KEY_STATUS_TEXT, osg::Vec2d(0.0, 0.0), osg::Vec2d(300, 150));
  }

  {
    // Add a map scale
    osg::ref_ptr<simUtil::MapScale> mapScale = new simUtil::MapScale;
    mapScale->setView(mainView);
    osg::MatrixTransform* xform = new osg::MatrixTransform;
    xform->addChild(mapScale.get());
    superHUD->getOrCreateHUD()->addChild(xform);
    hud->addWindow(KEY_MAP_SCALE, osg::Vec2d(0.65, 0.1), new simUtil::RepositionMatrixPxCallback(xform));
    hud->setSize(KEY_MAP_SCALE, osg::Vec2d(0.0, 0.0), osg::Vec2d(mapScale->width(), mapScale->height()));
  }

  {
    // Add top classification text
    osg::ref_ptr<simVis::ClassificationLabelNode> top = new simVis::ClassificationLabelNode();
    top->bindTo(&dataStore);
    top->setAlignment(osgText::TextBase::CENTER_TOP);
    osg::MatrixTransform* xform = new osg::MatrixTransform();
    xform->addChild(top.get());
    superHUD->getOrCreateHUD()->addChild(xform);
    hud->addWindow(KEY_CLASSIFICATION_TOP, osg::Vec2d(0.5, 0.995), new simUtil::RepositionMatrixPxCallback(xform));
    const osg::BoundingBox& bbox = top->getBoundingBox();
    hud->setSize(KEY_CLASSIFICATION_TOP, osg::Vec2d(bbox.xMin(), bbox.yMin()), osg::Vec2d(bbox.xMax(), bbox.yMax()));
  }

  {
    // Add bottom classification text
    osg::ref_ptr<simVis::ClassificationLabelNode> bottom = new simVis::ClassificationLabelNode();
    bottom->bindTo(&dataStore);
    bottom->setAlignment(osgText::TextBase::CENTER_BOTTOM);
    osg::MatrixTransform* xform = new osg::MatrixTransform();
    xform->addChild(bottom.get());
    superHUD->getOrCreateHUD()->addChild(xform);
    hud->addWindow(KEY_CLASSIFICATION_BOTTOM, osg::Vec2d(0.5, 0.005), new simUtil::RepositionMatrixPxCallback(xform));
    const osg::BoundingBox& bbox = bottom->getBoundingBox();
    hud->setSize(KEY_CLASSIFICATION_BOTTOM, osg::Vec2d(bbox.xMin(), bbox.yMin()), osg::Vec2d(bbox.xMax(), bbox.yMax()));
  }

  {
    // Add Compass
    osg::ref_ptr<simVis::CompassNode> compass = new simVis::CompassNode("compass.png");
    compass->setActiveView(mainView);
    menuHandler->setCompass(compass.get());
    std::shared_ptr<simVis::UpdateWindVaneListener> dsUpdate(new simVis::UpdateWindVaneListener(compass.get()));
    dataStore.addScenarioListener(dsUpdate);

    // Adjust the anchor position so the compass lower-right is at (1.0,0.0)
    static const float WIDTH_PX = 128.f;
    static const float BUFFER_PX = 25.f;
    const float OFFSET = WIDTH_PX * 0.5f + BUFFER_PX;
    compass->setMatrix(osg::Matrix::translate(osg::Vec3f(-OFFSET, OFFSET, 0.f)));

    // Wrap it in a transform to allow it to tie into the unified HUD easily
    osg::MatrixTransform* xform = new osg::MatrixTransform();
    xform->addChild(compass.get());
    superHUD->getOrCreateHUD()->addChild(xform);
    hud->addWindow(KEY_COMPASS, osg::Vec2d(1.0, 0.0), new simUtil::RepositionMatrixPxCallback(xform));
    const float TOTAL_PX = WIDTH_PX + BUFFER_PX;
    hud->setSize(KEY_COMPASS, osg::Vec2d(-TOTAL_PX, BUFFER_PX), osg::Vec2d(-BUFFER_PX, TOTAL_PX));
  }

  {
    // Create a simple text object and wrap it in a matrix transform
    osgText::Text* demoText = new osgText::Text;
    demoText->setText("Demonstration osgText String");
    demoText->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
    demoText->setAxisAlignment(osgText::TextBase::SCREEN);
    demoText->setAutoRotateToScreen(true);
    demoText->setCharacterSize(simVis::osgFontSize(16.f));
    demoText->setColor(osg::Vec4f(1.f, 1.f, 1.f, 1.f));
    demoText->setFont("arialbd.ttf");
    demoText->setBackdropColor(osg::Vec4f(0.f, 0.f, 0.f, 1.f));
    demoText->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    osg::MatrixTransform* xform = new osg::MatrixTransform();
    xform->addChild(demoText);
    superHUD->getOrCreateHUD()->addChild(xform);

    // Add the text to the HUD at 10% / 50%
    hud->addWindow(KEY_DEMO_TEXT, osg::Vec2d(0.1, 0.5), new simUtil::RepositionMatrixPxCallback(xform));
    const osg::BoundingBox& bbox = demoText->getBoundingBox();
    hud->setSize(KEY_DEMO_TEXT, osg::Vec2d(bbox.xMin(), bbox.yMin()), osg::Vec2d(bbox.xMax(), bbox.yMax()));
  }

  // for status and debugging
  viewer->installDebugHandlers();

  viewer->run();
}
