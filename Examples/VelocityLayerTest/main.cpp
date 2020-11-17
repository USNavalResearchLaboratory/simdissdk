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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cmath>
#include "osgDB/FileUtils"
#include "osgEarth/Controls"
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

/** Helper function to pick between a few different colors */
Control* createColorPicker(const std::function<void(const osg::Vec4f&)>& func, const osg::Vec4f& defaultColor)
{
  HBox* hbox = new HBox;
  hbox->setMargin(0);
  auto* red = hbox->addControl(new LabelControl("Red", 12.f, simVis::Color::Red));
  auto* lime = hbox->addControl(new LabelControl("Lime", 12.f, simVis::Color::Lime));
  auto* blue = hbox->addControl(new LabelControl("Blue", 12.f, simVis::Color::Blue));
  auto* cyan = hbox->addControl(new LabelControl("Cyan", 12.f, simVis::Color::Cyan));
  auto* yellow = hbox->addControl(new LabelControl("Yellow", 12.f, simVis::Color::Yellow));
  auto* purple = hbox->addControl(new LabelControl("Purple", 12.f, simVis::Color::Purple));
  // Create an on-click method that each control will use
  auto onClick = new OnClick([=](Control* c) {
    // Make the highlighted one filled
    for (auto ctl : { red, lime, blue, cyan, yellow, purple })
      ctl->setBackColor(ctl == c ? simVis::Color::Black : osg::Vec4f(0.f, 0.f, 0.f, 0.f));
    // Call the function with the given color
    func(*c->foreColor());
  });
  // Add the event handler to all colors
  for (auto ctl : { red, lime, blue, cyan, yellow, purple })
    ctl->addEventHandler(onClick);
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

  // Min Color
  ++row;
  grid->setControl(0, row, new LabelControl("Min Color", 12.f, simVis::Color::White));
  grid->setControl(1, row, createColorPicker([=](const osg::Vec4f& color) { layer->setMinColor(color); }, layer->getMinColor()));

  // Max Color
  ++row;
  grid->setControl(0, row, new LabelControl("Max Color", 12.f, simVis::Color::White));
  grid->setControl(1, row, createColorPicker([=](const osg::Vec4f& color) { layer->setMaxColor(color); }, layer->getMaxColor()));

  // Use Sprites
  ++row;
  grid->setControl(0, row, new LabelControl("Use Sprites", 12.f, simVis::Color::White));
  grid->setControl(1, row, new CheckBoxControl(!layer->getPointSprite().empty(),
    new BoolLambda([=](bool val) { layer->setPointSprite(val ? "WindSprite.png" : ""); })));

  // Show RTT Image
  ++row;
  grid->setControl(0, row, new LabelControl("Show RTT", 12.f, simVis::Color::White));
  grid->setControl(1, row, new CheckBoxControl(layer->getRenderRttImage(),
    new BoolLambda([=](bool val) { layer->setRenderRttImage(val); })));

  return b;
}

}


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

  // Add a GUI for manipulating fields
  Control* menu = createMenu(map, newLayer);
  mainView->addOverlayControl(menu);

  // run until the user quits by hitting ESC.
  viewMan->run();
  return 0;
}
