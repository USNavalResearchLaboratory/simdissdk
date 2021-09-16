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
#include "simUtil/GridTransform.h"
#include "simUtil/HudPositionEditor.h"
#include "simUtil/HudPositionManager.h"
#include "simUtil/MapScale.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/Replaceables.h"
#include "simUtil/StatusText.h"

// Uncomment this define to add various GridTransform test grids
// #define GRID_TESTING

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
namespace ui = osgEarth::Util::Controls;
#endif

static const std::string KEY_MAP_SCALE = "MapScale";
static const std::string KEY_STATUS_TEXT = "StatusText";
static const std::string KEY_DEMO_TEXT = "DemoText";
static const std::string KEY_CLASSIFICATION_TOP = "ClassificationTop";
static const std::string KEY_CLASSIFICATION_BOTTOM = "ClassificationBottom";
static const std::string KEY_COMPASS = "Compass";
static const std::string KEY_LEGEND = "Legend";

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
  "7 : Move 'Legend' to mouse position\n"
  "c : Cycle classification string and color\n"
  "e : Toggle HUD Editor mode\n"
  "r : Reset all to default positions\n"
  "w : Toggle Wind Vane on Compass\n"
  "z : Cycle wind angle and speed values\n"
  ;

#ifdef HAVE_IMGUI

struct ControlPanel : public GUI::BaseGui
{
  ControlPanel(simUtil::HudPositionEditor& hudEditor, simData::DataStore& dataStore)
    : GUI::BaseGui("HUD Position Manager Example"),
    hudEditor_(hudEditor),
    dataStore_(dataStore)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::Text("1 : Move 'Demo Text' to the mouse position");
    ImGui::Text("2 : Move 'Map Scale' to the mouse position");
    ImGui::Text("3 : Move 'Status Text' to mouse position");
    ImGui::Text("4 : Move 'Top Classification' to mouse position");
    ImGui::Text("5 : Move 'Bottom Classification' to mouse position");
    ImGui::Text("6 : Move 'Compass' to mouse position");
    ImGui::Text("7 : Move 'Legend' to mouse position");
    ImGui::Text("c : Cycle classification string and color");
    ImGui::Text("e : Toggle HUD Editor mode");
    ImGui::Text("r : Reset all to default positions");
    ImGui::Text("w : Toggle Wind Vane on Compass");
    ImGui::Text("z : Cycle wind angle and speed values");

    auto& io = ImGui::GetIO();
    auto mouse = io.MousePos;

    if (io.InputQueueCharacters.size() > 0)
    {
      switch (io.InputQueueCharacters.front())
      {
      case '1':
        moveWindowToMouse_(KEY_DEMO_TEXT);
        break;
      case '2':
        moveWindowToMouse_(KEY_MAP_SCALE);
        break;
      case '3':
        moveWindowToMouse_(KEY_STATUS_TEXT);
        break;
      case '4':
        moveWindowToMouse_(KEY_CLASSIFICATION_TOP);
        break;
      case '5':
        moveWindowToMouse_(KEY_CLASSIFICATION_BOTTOM);
        break;
      case '6':
        moveWindowToMouse_(KEY_COMPASS);
        break;
      case '7':
        moveWindowToMouse_(KEY_LEGEND);
        break;
      case 'c':
      {
        // Cycle through a few different classification strings
        classificationCycle_ = ((classificationCycle_ + 1) % 3);
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
        break;
      }
      case 'e':
        hudEditor_.setVisible(!hudEditor_.isVisible());
        break;
      case 'r':
        hudEditor_.resetAllPositions();
        break;
      case 'w':
        if (compass_.valid())
          compass_->setWindVaneVisible(!compass_->isWindVaneVisible());
        break;
      case 'z':
      {
        // Cycle through a few different wind settings
        windCycle_ = ((windCycle_ + 1) % 3);
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
        break;
      }
      }
    }

    ImGui::End();
  }

  void setCompass(simVis::CompassNode* compass)
  {
    compass_ = compass;
  }

private:
  void moveWindowToMouse_(const std::string& windowName)
  {
    auto io = ImGui::GetIO();
    ImVec2 viewSize = ImGui::GetMainViewport()->WorkSize;
    const osg::Vec2d pos(io.MousePos.x / viewSize.x, (viewSize.y - io.MousePos.y) / viewSize.y);
    hudEditor_.setPosition(windowName, pos);
  }

  simUtil::HudPositionEditor& hudEditor_;
  osg::observer_ptr<simVis::CompassNode> compass_;
  simData::DataStore& dataStore_;
  int classificationCycle_;
  int windCycle_;
};

#else

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
    case '7':
      windowName = KEY_LEGEND;
      break;

    case 'c':
    {
      // Cycle through a few different classification strings
      classificationCycle_ = ((classificationCycle_ + 1) % 3);
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
      windCycle_ = ((windCycle_ + 1) % 3);
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

#endif

/** Helper method that creates a colored square from (0,0) to (width,width) */
osg::Geometry* newSquare(const osg::Vec4f& color, float width=1.f)
{
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  osg::Vec3Array* verts = new osg::Vec3Array;
  verts->push_back(osg::Vec3f(0.f, 0.f, 0.f));
  verts->push_back(osg::Vec3f(width, 0.f, 0.f));
  verts->push_back(osg::Vec3f(width, width, 0.f));
  verts->push_back(osg::Vec3f(0.f, width, 0.f));
  geom->setVertexArray(verts);
  osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  colors->push_back(color);
  geom->setColorArray(colors);
  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, 4));
  return geom.release();
}

/** size of square for legend icons */
static const float SQUARE_SIZE = 48.f;

/** Draws a green square of fixed size at upper-left of cell. */
class LegendIconCell : public simUtil::GridCell
{
public:
  LegendIconCell()
  {
    // Placeholder, just use a green-ish square
    addChild(newSquare(osg::Vec4f(0.f, 0.6f, 0.f, 1.f), SQUARE_SIZE));
    // Configure the default size
    setDefaultSize(SQUARE_SIZE, SQUARE_SIZE);
    // Icon cells have a fixed width
    setFixedWidth(SQUARE_SIZE);
  }

protected:
  /** Similar to GridCell::setPositionImpl_(), but do not scale contents, and positions a little different. */
  virtual void setPositionImpl_()
  {
    osg::Matrix m;
    // Do not scale contents; we're already at right pixel size.
    // Translate so upper-left corner is at (height) pixels.
    m.postMult(osg::Matrix::translate(osg::Vec3f(x(), y() + height() - SQUARE_SIZE, 0.f)));
    setMatrix(m);
  }
};

/** Encapsulates the content and label text for a single cell in the legend */
class LegendTextCell : public simUtil::GridCell
{
public:
  LegendTextCell(const std::string& title, const std::string& content)
    : title_(new osgText::Text)
  {
    // Create the title label
    title_->setText(title);
    title_->setAlignment(osgText::TextBase::LEFT_TOP);
    title_->setAxisAlignment(osgText::TextBase::SCREEN);
    title_->setAutoRotateToScreen(true);
    title_->setCharacterSize(simVis::osgFontSize(16.f));
    title_->setColor(osg::Vec4f(1.f, 1.f, 1.f, 1.f));
    title_->setFont("arialbd.ttf");
    title_->setBackdropColor(osg::Vec4f(0.f, 0.f, 0.f, 1.f));
    title_->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);

    // Create the content label
    content_ = new osgText::Text(*title_);
    content_->setText(content);
    content_->setCharacterSize(simVis::osgFontSize(10.f));

    // Add the children
    addChild(title_.get());
    addChild(content_.get());

    // Assign the width and height based on title/outline size
    const auto& titleBb = title_->getBoundingBox();
    const auto& contentBb = content_->getBoundingBox();
    const osg::Vec2f titleSize(titleBb.xMax() - titleBb.xMin(), titleBb.yMax() - titleBb.yMin());
    const osg::Vec2f contentSize(contentBb.xMax() - contentBb.xMin(), contentBb.yMax() - contentBb.yMin());
    setDefaultSize(simCore::sdkMax(titleSize.x(), contentSize.x()),
      2.f + titleSize.y() + contentSize.y());
    titleHeight_ = titleSize.y();
  }

protected:
  /** Override to reposition the text based on the X, Y, and Height values configured. */
  virtual void setPositionImpl_()
  {
    // Adjust the position of the text manually
    title_->setPosition(osg::Vec3f(x(), y() + height(), 0.f));
    content_->setPosition(osg::Vec3f(x(), y() + height() - titleHeight_ - 2.f, 0.f));
  }

private:
  osg::ref_ptr<osgText::Text> title_;
  osg::ref_ptr<osgText::Text> content_;
  float titleHeight_;
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
  mouseDispatcher.setViewManager(viewer.get());

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
#ifndef HAVE_IMGUI
  // Add a help control
  superHUD->addOverlayControl(createHelp());
#endif
  mainView->getViewManager()->addView(superHUD);

  // For osgEarth::LineDrawable to work on SuperHUD, need an InstallCameraUniform
  superHUD->getOrCreateHUD()->addCullCallback(new osgEarth::InstallCameraUniform());
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

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);
  ControlPanel* controlPanel = new ControlPanel(hudEditor, dataStore);
  gui->add(controlPanel);
#else
  // Install a handler to respond to the demo keys in this sample.
  MenuHandler* menuHandler = new MenuHandler(hudEditor, dataStore);
  mainView->getCamera()->addEventCallback(menuHandler);
#endif

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
#ifdef HAVE_IMGUI
    controlPanel->setCompass(compass.get());
#else
    menuHandler->setCompass(compass.get());
#endif
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

#ifdef GRID_TESTING
  {
    // Add a "horizontal" grid with 3 columns, each child increasing to right, wrapping to next row
    simUtil::GridTransform* grid3Cols = new simUtil::GridTransform(3, true);
    grid3Cols->setSize(250.f, 200.f);
    grid3Cols->setSpacing(5.f);
    grid3Cols->setMatrix(osg::Matrix::translate(osg::Vec3f(100.f, 100.f, 0.f)));
    superHUD->getOrCreateHUD()->addChild(grid3Cols);
    // Build a 3x4 array (3 wide, 4 tall)
    for (int k = 0; k < 12; ++k)
    {
      // First has almost no color; last has full red
      const float magnitude = (k + 1) / 12.f;
      osg::ref_ptr<osg::Geometry> geom = newSquare(osg::Vec4f(magnitude, 0.f, 0.f, 1.f));
      osg::ref_ptr<simUtil::GridCell> item = new simUtil::GridCell();
      item->addChild(geom);
      grid3Cols->addChild(item);
    }
  }

  {
    // Add a "vertical" grid with 3 rows, each child increasing down, wrapping to next column
    simUtil::GridTransform* grid3Rows = new simUtil::GridTransform(3, false);
    grid3Rows->setSize(250.f, 200.f);
    grid3Rows->setSpacing(5.f);
    grid3Rows->setMatrix(osg::Matrix::translate(osg::Vec3f(400.f, 100.f, 0.f)));
    superHUD->getOrCreateHUD()->addChild(grid3Rows);
    // Build a 4x3 array (4 wide, 3 tall)
    for (int k = 0; k < 12; ++k)
    {
      // First has almost no color; last has full green
      const float magnitude = (k + 1) / 12.f;
      osg::ref_ptr<osg::Geometry> geom = newSquare(osg::Vec4f(0.f, magnitude, 0.f, 1.f));
      osg::ref_ptr<simUtil::GridCell> item = new simUtil::GridCell();
      item->addChild(geom);
      grid3Rows->addChild(item);
    }
  }

  {
    // Add a grid that tests fixed width
    simUtil::GridTransform* grid = new simUtil::GridTransform(3, false);
    grid->setSize(250.f, 200.f);
    grid->setSpacing(5.f);
    grid->setMatrix(osg::Matrix::translate(osg::Vec3f(700.f, 100.f, 0.f)));
    superHUD->getOrCreateHUD()->addChild(grid);
    // Build a 4x3 array (4 wide, 3 tall)
    for (int k = 0; k < 12; ++k)
    {
      // First has almost no color; last has full blue
      const float magnitude = (k + 1) / 12.f;
      osg::ref_ptr<osg::Geometry> geom = newSquare(osg::Vec4f(0.f, 0.f, magnitude, 1.f));
      osg::ref_ptr<simUtil::GridCell> item = new simUtil::GridCell();
      if (k / 3 == 1) // Second row is super wide
      {
        item->setFixedWidth(175.f);
        item->unsetOption(simUtil::GRID_STRETCH_COLUMN);
      }
      if (k % 3 == 1) // Second column is super short
      {
        item->setFixedHeight(10.f);
        item->unsetOption(simUtil::GRID_STRETCH_ROW);
      }
      item->addChild(geom);
      grid->addChild(item);
    }
  }
#endif /* GRID_TESTING */

  {
    // Create a legend that is 4x2
    simUtil::GridTransform* legendGrid = new simUtil::GridTransform(2, true);
    legendGrid->setSpacing(6.f);
    superHUD->getOrCreateHUD()->addChild(legendGrid);

    // Add 4 entries
    legendGrid->addChild(new LegendIconCell);
    legendGrid->addChild(new LegendTextCell("Entity 1", "Content for entity 1"));
    legendGrid->addChild(new LegendIconCell);
    legendGrid->addChild(new LegendTextCell("Entity 2", "[none]"));
    legendGrid->addChild(new LegendIconCell);
    legendGrid->addChild(new LegendTextCell("Entity 3", "Multi-line content\nEntity 3\nhas multiple lines of text.\nThere are 4 lines of text in this legend entry."));
    legendGrid->addChild(new LegendIconCell);
    legendGrid->addChild(new LegendTextCell("Entity 4", "Content"));

    // Adjust size
    legendGrid->setSize(legendGrid->getDefaultWidth(), legendGrid->getDefaultHeight());

    // Add the text to the HUD at 80% / 50%
    hud->addWindow(KEY_LEGEND, osg::Vec2d(0.8, 0.5), new simUtil::RepositionMatrixPxCallback(legendGrid));
    hud->setSize(KEY_LEGEND, osg::Vec2d(0.0, 0.0), osg::Vec2d(legendGrid->width(), legendGrid->height()));
  }

  // for status and debugging
  viewer->installDebugHandlers();

  viewer->run();
}
