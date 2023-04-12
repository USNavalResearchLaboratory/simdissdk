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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

/**
 * MapScale shows how to associate a map scale with a view and an inset.
 */

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
#endif
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Units.h"
#include "simVis/BoxGraphic.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/MapScale.h"

#define LC "[MapScale demo] "

namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

namespace {

static std::string s_title = "Map Scale Example";
static std::string s_help =
  "o : toggle overhead mode \n";

#ifdef HAVE_IMGUI
class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(simUtil::MapScale* scale)
    : simExamples::SimExamplesGui("Map Scale Example"),
    scale_(scale),
    metricUnits_(new simUtil::MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::KILOMETERS, 10000.0)),
    imperialUnits_(new simUtil::MapScaleTwoUnitsProvider(simCore::Units::YARDS, simCore::Units::MILES, 16093.4)),
    nauticalUnits_(new simUtil::MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::NAUTICAL_MILES, 18520.0))
  {
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

    ImGui::Text("o: Toggle overhead mode");

    auto setScaleColor = [this](const osgEarth::Color& color1, const osgEarth::Color& color2, const osgEarth::Color& unitsColor, const osgEarth::Color& valuesColor, const osgEarth::Color& bgColor)
    {
      scale_->setBarColor1(color1);
      scale_->setBarColor2(color2);
      scale_->setUnitsColor(unitsColor);
      scale_->setValuesColor(valuesColor);
      scale_->setBackgroundColor(bgColor);
    };

    auto setScaleHeight = [this](float scalar)
    {
      scale_->setBarHeight(8.f + 20.f * (scalar - 1));
      scale_->setUnitsCharacterSize(simVis::osgFontSize(12.f * scalar));
      scale_->setValuesCharacterSize(simVis::osgFontSize(13.f * scalar));
      scale_->setPadding(10.f * scalar, 10.f * scalar, 5.f * scalar, 5.f * scalar);
    };

    auto setScaleWidth = [this](float scalar)
    {
      scale_->setBarHeight(8.f + 20.f * (scalar - 1));
      scale_->setUnitsCharacterSize(simVis::osgFontSize(12.f * scalar));
      scale_->setValuesCharacterSize(simVis::osgFontSize(13.f * scalar));
      scale_->setPadding(10.f * scalar, 10.f * scalar, 5.f * scalar, 5.f * scalar);
    };

    static const float ALPHA_VALUE = 0.7f;

    if (ImGui::BeginTable("Table", 2))
    {
      // Color row
      ImGui::TableNextColumn(); ImGui::Text("Color:"); ImGui::TableNextColumn();
      if (ImGui::Button("Mono")) setScaleColor(
        osgEarth::Color::Black,
        osgEarth::Color::White,
        osgEarth::Color::White,
        osgEarth::Color::White,
        osg::Vec4f(1.f, 1.f, 1.f, 0.f)
        ); ImGui::SameLine();
      if (ImGui::Button("Alpha")) setScaleColor(
        osg::Vec4f(0.f, 0.f, 0.f, ALPHA_VALUE),
        osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE),
        osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE),
        osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE),
        osg::Vec4f(1.f, 1.f, 1.f, 0.f)
        ); ImGui::SameLine();
      if (ImGui::Button("Dim")) setScaleColor(
        osgEarth::Color::Gray,
        osgEarth::Color::Silver,
        osgEarth::Color::Silver,
        osgEarth::Color::Silver,
        osg::Vec4f(1.f, 1.f, 1.f, 0.25f)
      ); ImGui::SameLine();
      if (ImGui::Button("Colorful")) setScaleColor(
        osgEarth::Color::Green,
        osgEarth::Color::Purple,
        osgEarth::Color::Yellow,
        osgEarth::Color::Orange,
        osg::Vec4f(0.25f, 0.25f, 1.f, 0.25f)
      );

      // Height row
      ImGui::TableNextColumn(); ImGui::Text("Height:"); ImGui::TableNextColumn();
      if (ImGui::Button("80%")) setScaleHeight(.8f);
      ImGui::SameLine();
      if (ImGui::Button("100%")) setScaleHeight(1.f);
      ImGui::SameLine();
      if (ImGui::Button("125%")) setScaleHeight(1.25f);

      // Width row
      ImGui::TableNextColumn(); ImGui::Text("Width:"); ImGui::TableNextColumn();
      if (ImGui::Button("350px"))
        scale_->setWidth(350.f);
      ImGui::SameLine();
      if (ImGui::Button("500px"))
        scale_->setWidth(500.f);
      ImGui::SameLine();
      if (ImGui::Button("650px"))
        scale_->setWidth(650.f);

      // Units row
      ImGui::TableNextColumn(); ImGui::Text("Units:"); ImGui::TableNextColumn();
      if (ImGui::Button("Metric"))
        scale_->setUnitsProvider(metricUnits_);
      ImGui::SameLine();
      if (ImGui::Button("Imperial"))
        scale_->setUnitsProvider(imperialUnits_);
      ImGui::SameLine();
      if (ImGui::Button("Nautical"))
        scale_->setUnitsProvider(nauticalUnits_);

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  osg::ref_ptr<simUtil::MapScale> scale_;
  osg::ref_ptr<simUtil::MapScaleTwoUnitsProvider> metricUnits_;
  osg::ref_ptr<simUtil::MapScaleTwoUnitsProvider> imperialUnits_;
  osg::ref_ptr<simUtil::MapScaleTwoUnitsProvider> nauticalUnits_;
};

#else
/** Responds to click on mono color button */
struct MonoColorHandler : public ui::ControlEventHandler
{
  explicit MonoColorHandler(simUtil::MapScale* mapScale) : mapScale_(mapScale) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setBarColor1(simVis::Color::Black);
    mapScale_->setBarColor2(simVis::Color::White);
    mapScale_->setUnitsColor(simVis::Color::White);
    mapScale_->setValuesColor(simVis::Color::White);
    mapScale_->setBackgroundColor(osg::Vec4f(1.f, 1.f, 1.f, 0.f));
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
};

/** Responds to click on alpha color button */
struct AlphaColorHandler : public ui::ControlEventHandler
{
  explicit AlphaColorHandler(simUtil::MapScale* mapScale) : mapScale_(mapScale) {}
  virtual void onClick(ui::Control* control) {
    static const float ALPHA_VALUE = 0.7f;
    mapScale_->setBarColor1(osg::Vec4f(0.f, 0.f, 0.f, ALPHA_VALUE));
    mapScale_->setBarColor2(osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE));
    mapScale_->setUnitsColor(osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE));
    mapScale_->setValuesColor(osg::Vec4f(1.f, 1.f, 1.f, ALPHA_VALUE));
    mapScale_->setBackgroundColor(osg::Vec4f(1.f, 1.f, 1.f, 0.f));
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
};

/** Responds to click on gray color button */
struct GrayColorHandler : public ui::ControlEventHandler
{
  explicit GrayColorHandler(simUtil::MapScale* mapScale) : mapScale_(mapScale) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setBarColor1(simVis::Color::Gray);
    mapScale_->setBarColor2(simVis::Color::Silver);
    mapScale_->setUnitsColor(simVis::Color::Silver);
    mapScale_->setValuesColor(simVis::Color::Silver);
    mapScale_->setBackgroundColor(osg::Vec4f(1.f, 1.f, 1.f, 0.25f));
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
};

/** Responds to click on colorful color button */
struct ColorfulColorHandler : public ui::ControlEventHandler
{
  explicit ColorfulColorHandler(simUtil::MapScale* mapScale) : mapScale_(mapScale) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setBarColor1(simVis::Color::Green);
    mapScale_->setBarColor2(simVis::Color::Purple);
    mapScale_->setUnitsColor(simVis::Color::Yellow);
    mapScale_->setValuesColor(simVis::Color::Orange);
    mapScale_->setBackgroundColor(osg::Vec4f(0.25f, 0.25f, 1.f, 0.25f));
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
};

/** Responds to click on height change buttons */
struct HeightHandler : public ui::ControlEventHandler
{
  explicit HeightHandler(simUtil::MapScale* mapScale, float scalar) : mapScale_(mapScale), scalar_(scalar) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setBarHeight(8.f + 20.f * (scalar_ - 1));
    mapScale_->setUnitsCharacterSize(simVis::osgFontSize(12.f * scalar_));
    mapScale_->setValuesCharacterSize(simVis::osgFontSize(13.f * scalar_));
    mapScale_->setPadding(10.f * scalar_, 10.f * scalar_, 5.f * scalar_, 5.f * scalar_);
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
  float scalar_;
};

/** Responds to click on width change buttons */
struct WidthHandler : public ui::ControlEventHandler
{
  explicit WidthHandler(simUtil::MapScale* mapScale, float scalar) : mapScale_(mapScale), scalar_(scalar) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setWidth(scalar_);
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
  float scalar_;
};

/** Responds to click on unit change buttons */
struct UnitsHandler : public ui::ControlEventHandler
{
  explicit UnitsHandler(simUtil::MapScale* mapScale, simUtil::MapScale::UnitsProvider* unitsProvider)
    : mapScale_(mapScale), unitsProvider_(unitsProvider) {}
  virtual void onClick(ui::Control* control) {
    mapScale_->setUnitsProvider(unitsProvider_.get());
  }
  osg::ref_ptr<simUtil::MapScale> mapScale_;
  osg::ref_ptr<simUtil::MapScale::UnitsProvider> unitsProvider_;
};


static ui::Control* createHelp(simUtil::MapScale* mapScale)
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(s_help, 14, simVis::Color::Silver));

  ui::Grid* grid = new ui::Grid;
  grid->setMargin(0);
  grid->setPadding(10);
  grid->setChildSpacing(10);
  grid->setChildVertAlign(ui::Control::ALIGN_CENTER);
  vbox->addControl(grid);

  int row = 0;

  // Color settings
  static const float CONTROL_FONT_SIZE = 14.f;
  auto* widget = grid->setControl(0, row, new ui::LabelControl("Color"));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(1, row, new ui::ButtonControl("Mono", new MonoColorHandler(mapScale)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(2, row, new ui::ButtonControl("Alpha", new AlphaColorHandler(mapScale)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(3, row, new ui::ButtonControl("Dim", new GrayColorHandler(mapScale)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(4, row, new ui::ButtonControl("Colorful", new ColorfulColorHandler(mapScale)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  row++;

  // Size settings
  widget = grid->setControl(0, row, new ui::LabelControl("Height"));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(1, row, new ui::ButtonControl("80%", new HeightHandler(mapScale, 0.8f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(2, row, new ui::ButtonControl("100%", new HeightHandler(mapScale, 1.f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(3, row, new ui::ButtonControl("125%", new HeightHandler(mapScale, 1.25f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  row++;

  // Width
  widget = grid->setControl(0, row, new ui::LabelControl("Width"));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(1, row, new ui::ButtonControl("350px", new WidthHandler(mapScale, 350.f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(2, row, new ui::ButtonControl("500px", new WidthHandler(mapScale, 500.f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(3, row, new ui::ButtonControl("650px", new WidthHandler(mapScale, 650.f)));
  widget->setFontSize(CONTROL_FONT_SIZE);
  row++;

  // Units settings
  widget = grid->setControl(0, row, new ui::LabelControl("Units"));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(1, row, new ui::ButtonControl("Metric", new UnitsHandler(mapScale,
    new simUtil::MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::KILOMETERS, 10000.0))));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(2, row, new ui::ButtonControl("Imperial", new UnitsHandler(mapScale,
    new simUtil::MapScaleTwoUnitsProvider(simCore::Units::YARDS, simCore::Units::MILES, 16093.4))));
  widget->setFontSize(CONTROL_FONT_SIZE);
  widget = grid->setControl(3, row, new ui::ButtonControl("Nautical", new UnitsHandler(mapScale,
    new simUtil::MapScaleTwoUnitsProvider(simCore::Units::METERS, simCore::Units::NAUTICAL_MILES, 18520.0))));
  widget->setFontSize(CONTROL_FONT_SIZE);

  return vbox;
}

#endif

//----------------------------------------------------------------------------

// An event handler to assist in testing the Inset functionality.
struct MenuHandler : public osgGA::GUIEventHandler
{
  explicit MenuHandler(simVis::View* mainView)
    : mainView_(mainView)
  {
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN && ea.getKey() == 'o')
    {
      const bool newOverheadValue = !mainView_->isOverheadEnabled();
      mainView_->enableOrthographic(newOverheadValue);
      mainView_->enableOverheadMode(newOverheadValue);
      aa.requestRedraw();
      return true;
    }
    return false;
  }

private:
  osg::ref_ptr<simVis::View> mainView_;
};

/** Mouse event handler that will translate the Map Scale around the screen when we click on it */
class TranslateMapScaleHandler : public osgGA::GUIEventHandler
{
public:
  TranslateMapScaleHandler(simUtil::MapScale* scale, osg::MatrixTransform* xform)
    : scale_(scale),
      xform_(xform),
      outline_(new simVis::BoxGraphic(-5, -5, 510, 110, 2.f, 0xf0f0, simVis::Color::Lime)),
      clicked_(false)
  {
    outline_->setNodeMask(0);
    xform_->addChild(outline_);
  }

  // Override GUIEventHandler
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::MOVE:
      // Show the outline with correct size if the mouse is inside
      if (isInside_(osg::Vec3f(ea.getX(), ea.getY(), 0.0)))
      {
        // Give a boundary around the scale
        outline_->setGeometry(-5, -5, scale_->width() + 10, scale_->height() + 10);
        outline_->setNodeMask(~0);
      }
      else
        outline_->setNodeMask(0);
      break;

    case osgGA::GUIEventAdapter::RELEASE:
      // If we had clicked, do a final reposition for accuracy and then show the outline
      if (clicked_)
      {
        processDrag_(osg::Vec3f(ea.getX(), ea.getY(), 0.0));
        outline_->setNodeMask(~0);
        clicked_ = false;
        return true;
      }
      break;

    case osgGA::GUIEventAdapter::DRAG:
      // If we had clicked, move the box around
      if (clicked_)
      {
        processDrag_(osg::Vec3f(ea.getX(), ea.getY(), 0.0));
        return true;
      }
      break;

    case osgGA::GUIEventAdapter::PUSH:
      // Only care about left button presses that are inside the scale
      if (ea.getButton() == ea.LEFT_MOUSE_BUTTON)
      {
        clickPos_ = osg::Vec3f(ea.getX(), ea.getY(), 0.0);
        if (isInside_(clickPos_))
        {
          // Remember where the scale started (don't accumulate mouse x/y changes)
          startingPos_ = xform_->getMatrix().getTrans();
          clicked_ = true;
          // Turn off the outline
          outline_->setNodeMask(0);
          return true;
        }
      }
      break;

    default:
      break;
    }
    return false;
  }

  /** Return true if the given mouse position is inside the scale's box */
  bool isInside_(const osg::Vec3f& pos) const
  {
    osg::Vec3d trans = xform_->getMatrix().getTrans();
    // Clicked below or left
    if (pos.x() < trans.x() || pos.y() < trans.y())
      return false;
    // Clicked right
    if (pos.x() > trans.x() + scale_->width())
      return false;
    // Clicked above
    if (pos.y() > trans.y() + scale_->height())
      return false;
    return true;
  }

  /** Calculate the new position of the scale and apply it to the xform matrix */
  void processDrag_(const osg::Vec3f& newMousePos)
  {
    const osg::Vec3f delta = (newMousePos - clickPos_);
    const osg::Vec3f newPos = startingPos_ + delta;
    xform_->setMatrix(osg::Matrix::translate(newPos));
  }

  osg::ref_ptr<simUtil::MapScale> scale_;
  osg::ref_ptr<osg::MatrixTransform> xform_;
  osg::ref_ptr<simVis::BoxGraphic> outline_;
  bool clicked_;
  osg::Vec3f clickPos_;
  osg::Vec3f startingPos_;
};

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setLogarithmicDepthBufferEnabled(true);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Install a handler to respond to the demo keys in this sample.
  simVis::View* mainView = viewer->getMainView();
  mainView->setUpViewInWindow(100, 100, 1024, 768);
  mainView->getCamera()->addEventCallback(new MenuHandler(mainView));

  // Add a callback for inset view events, which forwards them to the focus manager
  simVis::InsetViewEventHandler* insetViewEventHandler = new simVis::InsetViewEventHandler(mainView);
  insetViewEventHandler->setFocusActions(simVis::InsetViewEventHandler::ACTION_CLICK_SCROLL);
  mainView->addEventHandler(insetViewEventHandler);

  // Turn on terrain avoidance
  dynamic_cast<osgEarth::Util::EarthManipulator*>(mainView->getCameraManipulator())
    ->getSettings()->setTerrainAvoidanceEnabled(true);

  // Create an inset view
  simVis::View* inset = new simVis::View;
  inset->setExtents(simVis::View::Extents(0.66, 0.66, 0.34, 0.34, true));
  inset->setSceneManager(mainView->getSceneManager());
  inset->setName("Inset");
  inset->applyManipulatorSettings(*mainView);
  mainView->addInset(inset);

  // Set the initial viewpoint for main view
  simVis::Viewpoint viewPoint1;
  viewPoint1.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), -159.8, 22., 0.0, osgEarth::ALTMODE_ABSOLUTE);
  viewPoint1.heading()->set(0., osgEarth::Units::DEGREES);
  viewPoint1.pitch()->set(-25.0, osgEarth::Units::DEGREES);
  viewPoint1.range()->set(80000.0, osgEarth::Units::METERS);
  mainView->setViewpoint(viewPoint1);

  // Set viewpoint for inset
  simVis::Viewpoint viewPoint2;
  viewPoint2.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), -50., 42., 0.0, osgEarth::ALTMODE_ABSOLUTE);
  viewPoint2.heading()->set(0., osgEarth::Units::DEGREES);
  viewPoint2.pitch()->set(-35.0, osgEarth::Units::DEGREES);
  viewPoint2.range()->set(600000.0, osgEarth::Units::METERS);
  inset->setViewpoint(viewPoint2);

  // Create a Super-HUD for drawing on top of all insets
  simVis::View* superHudView = new simVis::View();
  superHudView->setUpViewAsHUD(mainView);
  mainView->getViewManager()->addView(superHudView);

  osg::Camera* superHud = superHudView->getOrCreateHUD();
  osg::StateSet* hudStateSet = superHud->getOrCreateStateSet();
  // Turn off lighting and depth test by default for the SuperHUD
  hudStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  hudStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

  // Add a map scale under the Super HUD, pointing to the main view
  osg::MatrixTransform* xform = new osg::MatrixTransform;
  xform->setMatrix(osg::Matrix::translate(osg::Vec3d(20.0, 20.0, 0.0)));
  simUtil::MapScale* mapScale = new simUtil::MapScale;
  mapScale->setView(mainView);
  superHud->addChild(xform);
  xform->addChild(mapScale);

  // Whenever the focus manager gets a focus event, change the scale to point to it
  mapScale->bindToFocusManager(mainView->getFocusManager());

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);

  gui->add(new ControlPanel(mapScale));
#else
  // Create a HUD for managing everything
  mainView->addOverlayControl(createHelp(mapScale));
#endif

  // Add a mouse handler that lets us move the scale around the screen
  TranslateMapScaleHandler* translateScaleByMouse = new TranslateMapScaleHandler(mapScale, xform);
  mainView->addEventHandler(translateScaleByMouse);
  inset->addEventHandler(translateScaleByMouse);

  // for status and debugging
  viewer->installDebugHandlers();

  return viewer->run();
}
