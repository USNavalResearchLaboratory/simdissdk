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
#include "osgEarthUtil/Controls"
#include "osgEarthUtil/Sky"
#include "osgEarthDrivers/sky_simple/SimpleSkyOptions"
#include "osgEarthDrivers/sky_gl/GLSkyOptions"

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/NullSkyModel.h"
#include "simUtil/PlatformSimulator.h"

#include "simVis/Utils.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/Platform.h"
#include "simVis/SceneManager.h"


namespace
{
using namespace osgEarth::Util::Controls;

/** Command line argument usage */
int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --sluser <username> : Use username for SilverLining license\n"
    << "    --sllicense <key>   : Use key for SilverLining license\n"
    << "    --slpath <path>     : Use path for SilverLining resources\n"
    << std::endl;

  return 0;
}

/** Enumeration of supported sky models */
enum SkyModel
{
  SKY_NONE,
  SKY_SIMPLE,
  SKY_GL,
  SKY_SILVERLINING
};
/** Default ambient value */
static const float INITIAL_AMBIENT = 0.5f;
/** Default sky model value */
static const SkyModel INITIAL_SKY_MODEL = SKY_SIMPLE;

/** Application settings in a struct with basic set/apply functions */
struct AppData
{
  osg::ref_ptr<HSliderControl> ambient;
  osg::ref_ptr<HSliderControl> skyModelSlider;
  osg::ref_ptr<LabelControl> skyModelText;
  osg::ref_ptr<simVis::SceneManager> sceneManager;
  osg::ref_ptr<simVis::View> mainView;
  std::string slUser;
  std::string slLicense;
  std::string slResourcePath;

  /** initialize the structure */
  AppData(simVis::SceneManager* sceneMgr, simVis::View* mnView)
    : sceneManager(sceneMgr),
      mainView(mnView),
      slResourcePath(simExamples::getSilverLiningResourcesPath()),
      skyModelValue_(static_cast<SkyModel>(INITIAL_SKY_MODEL+1)) // offset so we can call setSkyModel below
  {
  }

  /** Apply ambient value from slider */
  void applyAmbient()
  {
    float mag = ambient->getValue();
    if (sceneManager->getSkyNode() != NULL)
      sceneManager->getSkyNode()->setMinimumAmbient(osg::Vec4f(mag, mag, mag, 1.f));
  }

  /** Changes the current sky model */
  void setSkyModel(SkyModel model)
  {
    // Update the slider unconditionally for crisp values
    skyModelSlider->setValue(static_cast<float>(model));

    // No-op if setting to the current value
    if (model == skyModelValue_)
      return;
    // Save new value
    skyModelValue_ = model;

    // Update the sky model
    switch (model)
    {
    case SKY_NONE:
      setNoSky_();
      break;
    case SKY_SIMPLE:
      setSimpleSky_();
      break;
    case SKY_GL:
      setGlSky_();
      break;
    case SKY_SILVERLINING:
      setSilverLiningSky_();
      break;
    }
    // Changing the sky model requires a reset of the lighting
    applyAmbient();
  }

private:
  /** Turns off the sky model */
  void setNoSky_()
  {
    skyModelText->setText("None");
    setSky_(new simUtil::NullSkyModel);
  }

  /** Sets up the Simple sky model */
  void setSimpleSky_()
  {
    skyModelText->setText("Simple");

    // Set up the Config for Simple
    Config skyOptions;
    skyOptions.set("driver", "simple");
    skyOptions.set("atmospheric_lighting", false);
    setSky_(createSky_(skyOptions));
  }

  /** Sets up the GL sky model */
  void setGlSky_()
  {
    skyModelText->setText("GL");

    // Set up the Config for GL
    Config skyOptions;
    skyOptions.set("driver", "gl");
    setSky_(createSky_(skyOptions));
  }

  /** Sets up the SilverLining sky model with configured user/license */
  void setSilverLiningSky_()
  {
    skyModelText->setText("SilverLining");

    // Set up the Config for SilverLining
    Config skyOptions;
    skyOptions.set("driver", "silverlining");
    skyOptions.set("clouds", true);
    skyOptions.set("clouds_max_altitude", 100000.0);
    if (!slUser.empty())
      skyOptions.set("user", slUser);
    if (!slLicense.empty())
      skyOptions.set("license_code", slLicense);
    if (!slResourcePath.empty())
      skyOptions.set("resource_path", slResourcePath);
    setSky_(createSky_(skyOptions));
  }

  /** Given a Config, creates a Sky node */
  osgEarth::Util::SkyNode* createSky_(const osgEarth::Config& options)
  {
    return osgEarth::Util::SkyNode::create(ConfigOptions(options), sceneManager->getMapNode());
  }

  /** Given a Config, creates and attaches a sky node */
  void setSky_(osgEarth::Util::SkyNode* sky)
  {
    sceneManager->setSkyNode(sky);
    // Calling setSceneManager forces the sky to reattach
    mainView->setSceneManager(sceneManager.get());
    // Assign a date/time to the sky to initialize it
    if (sky)
      sky->setDateTime(osgEarth::Util::DateTime(2014, 4, 22, 16.5));
  }

  /** Current value for the Sky Model */
  SkyModel skyModelValue_;
};

/** Handler for Ambient changes, applying them using the AppData */
struct ApplyAmbient : public ControlEventHandler
{
  explicit ApplyAmbient(AppData& app) : app_(app) {}
  AppData& app_;
  void onValueChanged(Control* c, float value) { app_.applyAmbient(); }
  void onValueChanged(Control* c, double value) { app_.applyAmbient(); }
};

/** Handler for SkyModel changes, applying them using the AppData */
struct ApplySkyModel : public ControlEventHandler
{
  explicit ApplySkyModel(AppData& app)
    : app_(app)
  {
  }
  AppData& app_;
  void onValueChanged(Control* c, float value)
  {
    int idx = static_cast<int>(value + 0.5f);
    app_.setSkyModel(static_cast<SkyModel>(idx));
  }
  void onValueChanged(Control* c, double value) { onValueChanged(c, static_cast<float>(value)); }
};

/** Creates a UI for the AppData, returning the VBox containing all UI items */
Control* createUi(AppData& app)
{
  VBox* vbox = new VBox;
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl("Sky Model Example", 20, osg::Vec4f(1, 1, 0, 1)));

  osg::ref_ptr<Grid> grid = vbox->addControl(new Grid());
  unsigned int row = 0;
  unsigned int col = 0;

  row++;
  grid->setControl(col, row, new LabelControl("Ambient"));
  app.ambient = grid->setControl(col + 1, row, new HSliderControl(0.0, 1.0, INITIAL_AMBIENT, new ApplyAmbient(app)));
  app.ambient->setHorizFill(true, 250.0);
  grid->setControl(col + 2, row, new LabelControl(app.ambient.get()));

  row++;
  grid->setControl(col, row, new LabelControl("Model"));
  app.skyModelSlider = grid->setControl(col + 1, row, new HSliderControl(0.0f, 3.0f, static_cast<float>(INITIAL_SKY_MODEL), new ApplySkyModel(app)));
  app.skyModelSlider->setHorizFill(true, 250.0);
  app.skyModelText = grid->setControl(col + 2, row, new LabelControl("Sky Model", osg::Vec4f(1.f, 1.f, 1.f, 1.f)));

  return vbox;
}

}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // We need a view manager. This handles all of our Views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager(arguments);

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan.get());

  // Create views and connect them to our scene.
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneMan.get());
  mainView->setUpViewInWindow(100, 100, 640, 480);

  // Add it to the view manager
  viewMan->addView(mainView.get());

  // Set up the application data
  AppData app(sceneMan.get(), mainView.get());

  // Read SilverLining command line arguments
  osg::ArgumentParser::Parameter sluserArg(app.slUser);
  osg::ArgumentParser::Parameter sllicenseArg(app.slLicense);
  osg::ArgumentParser::Parameter slpathArg(app.slResourcePath);
  arguments.read("--sluser", sluserArg);
  arguments.read("--sllicense", sllicenseArg);
  arguments.read("--slpath", slpathArg);

  // Create the User Interface controls
  mainView->addOverlayControl(createUi(app));

  // Apply the current settings so the GUI is up to date
  app.applyAmbient();
  app.setSkyModel(INITIAL_SKY_MODEL);

  // Add an entity flying around
  osg::ref_ptr<simUtil::CircumnavigationPlatformSimulation> platformSim = new simUtil::CircumnavigationPlatformSimulation(sceneMan.get(), mainView.get());
  // Get an offset angle, tethered to the platform
  simVis::Viewpoint vp;
  vp.heading()->set(20, osgEarth::Units::DEGREES);
  vp.pitch()->set(-60, osgEarth::Units::DEGREES);
  vp.range()->set(5000000, osgEarth::Units::METERS);
  mainView->tetherCamera(platformSim->platformNode(), vp, 0);

  // run until the user quits by hitting ESC.
  viewMan->run();
}
