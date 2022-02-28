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
#include <cmath>
#include "osgDB/FileUtils"
#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
#endif
#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/StatsHandler.h"
#include "simUtil/VelocityParticleLayer.h"

namespace {

/**
 * Filename of the velocities to render.  This is an image file that stores the X-velocity ("U") in the
 * red pixels, and the Y-velocity ("V") in the green pixels.  Blue and alpha pixels are ignored.
 */
static const std::string DEFAULT_VELOCITY_FILE = "nws_gfs_20201110_t00z_1p00_winduv.png";

/** Print the command line arguments */
int usage(char* argv[])
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --file [f]  : Read velocities from file 'f' instead of default"
    << std::endl;

  return 0;
}

#ifndef HAVE_IMGUI

using namespace osgEarth::Util::Controls;

/** Applies a templated value using a lambda */
template <typename T>
class LambdaT : public ControlEventHandler
{
public:
  explicit LambdaT(std::function<void(T)> func)
    : func_(func)
  {
  }

  virtual void onValueChanged(Control* c, T value) override
  {
    func_(value);
  }

private:
  std::function<void(T)> func_;
};

// Simplification typedefs
typedef LambdaT<float> FloatLambda;
typedef LambdaT<bool> BoolLambda;

/** Handles on-click, calling a lambda */
class OnClick : public ControlEventHandler
{
public:
  explicit OnClick(std::function<void(Control*)> func)
    : func_(func)
  {
  }

  virtual void onClick(Control* control) override
  {
    func_(control);
  }

private:
  std::function<void(Control*)> func_;
};

/** Helper function to pick between a few different gradients */
Control* createGradientPicker(const std::function<void(const simVis::GradientShader::ColorMap&)>& func)
{
  HBox* hbox = new HBox;
  hbox->setMargin(0);

  auto* defaultGrad = hbox->addControl(new LabelControl("Default", 12.f, simVis::Color::White));
  defaultGrad->addEventHandler(new OnClick([=](Control* c) {
    const simVis::GradientShader::ColorMap newColors = {
      { 0.f, osgEarth::Color::Blue },
      { 8.f, osgEarth::Color::Cyan },
      { 13.f, osgEarth::Color::Lime },
      { 18.f, osgEarth::Color::Yellow },
      { 50.f, osgEarth::Color::Red },
      { 75.f, osgEarth::Color::Purple },
    };
    func(newColors);
  }));

  auto* cyanRedGrad = hbox->addControl(new LabelControl("Cyan", 12.f, simVis::Color::White));
  cyanRedGrad->addEventHandler(new OnClick([=](Control* c) {
    simVis::GradientShader::ColorMap newColors = {
      {0.f, simVis::Color::Cyan},
      {25.f, simVis::Color::Red}
    };
    func(newColors);
  }));

  auto* grayGrad = hbox->addControl(new LabelControl("Grayscale", 12.f, simVis::Color::White));
  grayGrad->addEventHandler(new OnClick([=](Control* c) {
    simVis::GradientShader::ColorMap newColors = {
      {0.f, simVis::Color::Black},
      {25.f, simVis::Color::White}
    };
    func(newColors);
  }));

  auto* greenRedGrad = hbox->addControl(new LabelControl("Green", 12.f, simVis::Color::White));
  greenRedGrad->addEventHandler(new OnClick([=](Control* c) {
    simVis::GradientShader::ColorMap newColors = {
      {0.f, simVis::Color::Lime},
      {25.f, simVis::Color::Red}
    };
    func(newColors);
  }));

  auto* transparentGrad = hbox->addControl(new LabelControl("Transparent", 12.f, simVis::Color::White));
  transparentGrad->addEventHandler(new OnClick([=](Control* c) {
    simVis::GradientShader::ColorMap newColors = {
      // Merge alpha from 0 to 25
      {0.f, osg::Vec4f(0.f, 1.f, 0.f, 0.f)},
      {25.f, osg::Vec4f(0.f, 1.f, 0.f, 1.f)},
    };
    func(newColors);
  }));
  return hbox;
}

/** Helper method that creates the upper-left menu */
Control* createMenu(osgEarth::Map* map, simUtil::VelocityParticleLayer* layer)
{
  VBox* b = new VBox();
  b->setBackColor(0, 0, 0, 0.5);
  b->addControl(new LabelControl("Velocity Particle Layer Demo", 14.f, simVis::Color::Yellow));
  Grid* grid = b->addControl(new Grid);
  grid->setChildSpacing(1.f); // Decrease spacing because of so many controls
  int row = 0;

  // Opacity
  grid->setControl(0, row, new LabelControl("Opacity", 12.f, simVis::Color::White));
  HSliderControl* opacitySlider = grid->setControl(1, row, new HSliderControl(0.f, 1.f, layer->getOpacity(),
    new FloatLambda([=](float val) { layer->setOpacity(val); })));
  opacitySlider->setHorizFill(true, 250.0f);
  grid->setControl(2, row, new LabelControl(opacitySlider, 12.f, simVis::Color::White));

  // Num Particles
  ++row;
  grid->setControl(0, row, new LabelControl("Num Particles", 12.f, simVis::Color::White));
  auto* numParticlesLabel = new LabelControl(std::to_string(layer->getParticleDimension()), 12.f, simVis::Color::White);
  // Map 0 to 256 (2^(8+0)), 3.0 to 2048 (2^(8+3))
  grid->setControl(1, row, new HSliderControl(0.f, 3.99f, log2(static_cast<double>(layer->getParticleDimension())) - 8,
    new FloatLambda([=](float val) {
      layer->setParticleDimension(static_cast<unsigned int>(pow(2.0, static_cast<int>(val) + 8.f)));
      numParticlesLabel->setText(std::to_string(layer->getParticleDimension()));
    })));
  grid->setControl(2, row, numParticlesLabel);

  // Die Speed
  ++row;
  grid->setControl(0, row, new LabelControl("Die Speed", 12.f, simVis::Color::White));
  auto* dieSpeed = grid->setControl(1, row, new HSliderControl(0.f, 50.f, layer->getDieSpeed(),
    new FloatLambda([=](float val) { layer->setDieSpeed(val); })));
  grid->setControl(2, row, new LabelControl(dieSpeed, 12.f, simVis::Color::White));

  // Speed Factor
  ++row;
  grid->setControl(0, row, new LabelControl("Speed", 12.f, simVis::Color::White));
  auto* speed = grid->setControl(1, row, new HSliderControl(0.01f, 2.f, layer->getSpeedFactor(),
    new FloatLambda([=](float val) { layer->setSpeedFactor(val); })));
  grid->setControl(2, row, new LabelControl(speed, 12.f, simVis::Color::White));

  // Point Size
  ++row;
  grid->setControl(0, row, new LabelControl("Point Size", 12.f, simVis::Color::White));
  auto* pointSize = grid->setControl(1, row, new HSliderControl(1.f, 10.f, layer->getPointSize(),
    new FloatLambda([=](float val) { layer->setPointSize(val); })));
  grid->setControl(2, row, new LabelControl(pointSize, 12.f, simVis::Color::White));

  // Drop Chance
  ++row;
  grid->setControl(0, row, new LabelControl("Drop Chance", 12.f, simVis::Color::White));
  auto* dropChance = grid->setControl(1, row, new HSliderControl(0.f, 0.1f, layer->getDropChance(),  // 0-10% chance
    new FloatLambda([=](float val) { layer->setDropChance(val); })));
  grid->setControl(2, row, new LabelControl(dropChance, 12.f, simVis::Color::White));

  // Particle Altitude
  ++row;
  grid->setControl(0, row, new LabelControl("Altitude", 12.f, simVis::Color::White));
  auto* altitude = grid->setControl(1, row, new HSliderControl(0.f, 10000.f, layer->getParticleAltitude(),
    new FloatLambda([=](float val) { layer->setParticleAltitude(val); })));
  grid->setControl(2, row, new LabelControl(altitude, 12.f, simVis::Color::White));

  // Discrete: Since most gradients are not thoroughly defined and only depend on two points,
  // this only really works well with the default gradient.
  ++row;
  grid->setControl(0, row, new LabelControl("Discrete Colors", 12.f, simVis::Color::White));
  grid->setControl(1, row, new CheckBoxControl(layer->getGradient().isDiscrete(), new BoolLambda([=](bool val) {
    auto newGradient = layer->getGradient();
    newGradient.setDiscrete(val);
    layer->setGradient(newGradient);
  })));

  // Gradient
  ++row;
  grid->setControl(0, row, new LabelControl("Gradient", 12.f, simVis::Color::White));
  grid->setControl(1, row, createGradientPicker([=](const simVis::GradientShader::ColorMap& colors) {
    simVis::GradientShader newGrad;
    newGrad.setDiscrete(layer->getGradient().isDiscrete());
    newGrad.setColorMap(colors);
    layer->setGradient(newGrad);
  }));

  // Use Sprites
  ++row;
  grid->setControl(0, row, new LabelControl("Use Sprites", 12.f, simVis::Color::White));
  grid->setControl(1, row, new CheckBoxControl(!layer->getPointSprite().empty(),
    new BoolLambda([=](bool val) { layer->setPointSprite(val ? "WindSprite.png" : ""); })));

  return b;
}

#else

///////////////////////////////////////////////////////////////////////////////////////////

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
// TODO: This should probably be added to osgImGuiHandler.h so that it's available to all SDK examples.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(-1); func("##" label, __VA_ARGS__)

class ControlPanel : public GUI::BaseGui
{
public:
  explicit ControlPanel(simUtil::VelocityParticleLayer* layer)
    : GUI::BaseGui("Velocity Particle Layer Demo"),
    layer_(layer)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::BeginTable("Table", 2))
    {
      // Opacity
      float opacity = layer_->getOpacity();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Opacity", &opacity, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (opacity != layer_->getOpacity())
        layer_->setOpacity(opacity);

      // Num particles
      unsigned int num = layer_->getParticleDimension();
      ImGui::TableNextColumn(); ImGui::Text("Num Particles"); ImGui::TableNextColumn();
      if (ImGui::RadioButton("256", num == 256)) { layer_->setParticleDimension(256); } ImGui::SameLine();
      if (ImGui::RadioButton("512", num == 512)) { layer_->setParticleDimension(512); } ImGui::SameLine();
      if (ImGui::RadioButton("1024", num == 1024)) { layer_->setParticleDimension(1024); } ImGui::SameLine();
      if (ImGui::RadioButton("2048", num == 2048)) { layer_->setParticleDimension(2048); }

      // Die speed
      int dieSpeed = layer_->getDieSpeed();
      IMGUI_ADD_ROW(ImGui::SliderInt, "Die Speed", &dieSpeed, 0, 50, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (dieSpeed != layer_->getDieSpeed())
        layer_->setDieSpeed(dieSpeed);

      // Speed
      float speed = layer_->getSpeedFactor();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Speed", &speed, 0.1f, 2.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (speed != layer_->getSpeedFactor())
        layer_->setSpeedFactor(speed);

      // Point size
      float pointSize = layer_->getPointSize();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Point Size", &pointSize, 1.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (pointSize != layer_->getPointSize())
        layer_->setPointSize(pointSize);

      // Drop chance
      float dropChance = layer_->getDropChance();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Drop Chance", &dropChance, 0.f, 0.1f, "%.3f", ImGuiSliderFlags_AlwaysClamp); // 0-10% chance
      if (dropChance != layer_->getDropChance())
        layer_->setDropChance(dropChance);

      // Altitude
      float altitude = layer_->getParticleAltitude();
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Altitude", &altitude, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (altitude != layer_->getParticleAltitude())
        layer_->setParticleAltitude(altitude);

      // Discrete gradients
      bool isDiscrete = layer_->getGradient().isDiscrete();
      bool newDiscrete = isDiscrete;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Discrete Colors", &newDiscrete);
      if (isDiscrete != newDiscrete)
      {
        auto newGradient = layer_->getGradient();
        newGradient.setDiscrete(newDiscrete);
        layer_->setGradient(newGradient);
      }

      // Gradient options
      auto setGradient = [=](const simVis::GradientShader::ColorMap& colors) {
        simVis::GradientShader newGrad;
        newGrad.setDiscrete(layer_->getGradient().isDiscrete());
        newGrad.setColorMap(colors);
        layer_->setGradient(newGrad);
      };

      ImGui::TableNextColumn(); ImGui::Text("Gradient"); ImGui::TableNextColumn();
      if (ImGui::Button("Default")) setGradient({
        { 0.f, osgEarth::Color::Blue },
        { 8.f, osgEarth::Color::Cyan },
        { 13.f, osgEarth::Color::Lime },
        { 18.f, osgEarth::Color::Yellow },
        { 50.f, osgEarth::Color::Red },
        { 75.f, osgEarth::Color::Purple }
      }); ImGui::SameLine();
      if (ImGui::Button("Cyan")) setGradient({
        {0.f, simVis::Color::Cyan},
        {25.f, simVis::Color::Red}
      }); ImGui::SameLine();
      if (ImGui::Button("Grayscale")) setGradient({
        {0.f, simVis::Color::Black},
        {25.f, simVis::Color::White}
      });
      ImGui::TableNextColumn(); ImGui::TableNextColumn();
      if (ImGui::Button("Green")) setGradient({
        {0.f, simVis::Color::Lime},
        {25.f, simVis::Color::Red}
      }); ImGui::SameLine();
      if (ImGui::Button("Transparent")) setGradient({
        {0.f, osg::Vec4f(0.f, 1.f, 0.f, 0.f)},
        {25.f, osg::Vec4f(0.f, 1.f, 0.f, 1.f)}
      });

      // Use sprites
      bool useSprites = !layer_->getPointSprite().empty();
      bool newUseSprites = useSprites;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Sprites", &newUseSprites);
      if (useSprites != newUseSprites)
        layer_->setPointSprite(newUseSprites ? "WindSprite.png" : "");

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  osg::observer_ptr<simUtil::VelocityParticleLayer> layer_;
};

#endif

}

///////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  // Initialize
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();
  if (arguments.read("--help"))
    return usage(argv);

  // Determine if the end user has a different file in mind from the default
  std::string velocityFile = DEFAULT_VELOCITY_FILE;
  arguments.read("--file", velocityFile);

  // Create a map
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // Add the layer with wind velocity particles
  simUtil::VelocityParticleLayer* newLayer = new simUtil::VelocityParticleLayer;
  newLayer->setVelocityTexture(osgDB::findDataFile(velocityFile));
  newLayer->setPointSize(2);
  newLayer->setParticleDimension(2048);
  map->addLayer(newLayer);

  // Create the scene and the view manager
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());
  simExamples::addDefaultSkyNode(sceneMan.get());
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager(arguments);

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan.get());

  // Create views and connect them to our scene.
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneMan.get());
  mainView->setUpViewInWindow(100, 100, 1280, 720);
  viewMan->addView(mainView.get());

  // Add a frame rate display
  osgViewer::StatsHandler* stats = new osgViewer::StatsHandler();
  stats->getCamera()->setAllowEventFocus(false);
  mainView->addEventHandler(stats);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewMan->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewMan->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);

  gui->add(new ControlPanel(newLayer));
#else
  // Add a GUI for manipulating fields
  Control* menu = createMenu(map.get(), newLayer);
  mainView->addOverlayControl(menu);
#endif
  // run until the user quits by hitting ESC.
  viewMan->run();
  return 0;
}
