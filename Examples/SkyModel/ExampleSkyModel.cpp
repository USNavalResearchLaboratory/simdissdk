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
#include "osgEarth/Sky"
#include "osgEarthDrivers/sky_simple/SimpleSkyOptions"
#include "osgEarthDrivers/sky_gl/GLSkyOptions"

#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
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

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
using namespace osgEarth::Util::Controls;
#endif

namespace
{

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
#ifdef HAVE_IMGUI
  float mag = INITIAL_AMBIENT;
#else
  osg::ref_ptr<HSliderControl> ambient;
  osg::ref_ptr<HSliderControl> skyModelSlider;
  osg::ref_ptr<LabelControl> skyModelText;
#endif
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
#ifndef HAVE_IMGUI
    float mag = ambient->getValue();
#endif

    if (sceneManager->getSkyNode() != nullptr)
      sceneManager->getSkyNode()->getSunLight()->setAmbient(osg::Vec4f(mag, mag, mag, 1.f));
  }

  SkyModel skyModel() const
  {
    return skyModelValue_;
  }

  /** Changes the current sky model */
  void setSkyModel(SkyModel model)
  {
#ifndef HAVE_IMGUI
    // Update the slider unconditionally for crisp values
    skyModelSlider->setValue(static_cast<float>(model));
#endif

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
#ifndef HAVE_IMGUI
    skyModelText->setText("None");
#endif
    setSky_(new simUtil::NullSkyModel);
  }

  /** Sets up the Simple sky model */
  void setSimpleSky_()
  {
#ifndef HAVE_IMGUI
    skyModelText->setText("Simple");
#endif

    // Set up the Config for Simple
    osgEarth::Config skyOptions;
    skyOptions.set("driver", "simple");
    skyOptions.set("atmospheric_lighting", false);
    setSky_(createSky_(skyOptions));
  }

  /** Sets up the GL sky model */
  void setGlSky_()
  {
#ifndef HAVE_IMGUI
    skyModelText->setText("GL");
#endif

    // Set up the Config for GL
    osgEarth::Config skyOptions;
    skyOptions.set("driver", "gl");
    setSky_(createSky_(skyOptions));
  }

  /** Sets up the SilverLining sky model with configured user/license */
  void setSilverLiningSky_()
  {
#ifndef HAVE_IMGUI
    skyModelText->setText("SilverLining");
#endif

    // Set up the Config for SilverLining
    osgEarth::Config skyOptions;
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
  osgEarth::SkyNode* createSky_(const osgEarth::Config& options)
  {
    return osgEarth::SkyNode::create(osgEarth::ConfigOptions(options));
  }

  /** Given a Config, creates and attaches a sky node */
  void setSky_(osgEarth::SkyNode* sky)
  {
    sceneManager->setSkyNode(sky);
    // Calling setSceneManager forces the sky to reattach
    mainView->setSceneManager(sceneManager.get());
    // Assign a date/time to the sky to initialize it
    if (sky)
      sky->setDateTime(osgEarth::DateTime(2014, 4, 22, 16.5));
  }

  /** Current value for the Sky Model */
  SkyModel skyModelValue_;
};

#ifdef HAVE_IMGUI
// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

/** Delay sky model update until the update operation, to avoid SL GL problems changing mid-ImGui */
class SetSkyModelOperation : public osg::Operation
{
public:
  SetSkyModelOperation(AppData& app, SkyModel model)
    : Operation("Set Sky Model", false),
      app_(app),
      model_(model)
  {
  }

  // From osg::Operation:
  virtual void operator()(osg::Object*) override
  {
    app_.setSkyModel(model_);
  }

private:
  AppData& app_;
  SkyModel model_;
};

class ControlPanel : public GUI::BaseGui
{
public:
  explicit ControlPanel(AppData& app)
    : GUI::BaseGui("Sky Model Example"),
      app_(app)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    static int currentModelIdx = static_cast<int>(app_.skyModel());

    if (ImGui::BeginTable("Table", 2))
    {
      float mag = app_.mag;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Ambient", &app_.mag, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (mag != app_.mag)
        app_.applyAmbient();

      // Sky model combo box
      ImGui::TableNextColumn(); ImGui::Text("Model"); ImGui::TableNextColumn();
      static const char* MODELS[] = { "None", "Simple", "GL", "SilverLining" };
      if (ImGui::BeginCombo("##model", MODELS[currentModelIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(MODELS); i++)
        {
          const bool isSelected = (currentModelIdx == i);
          if (ImGui::Selectable(MODELS[i], isSelected))
            currentModelIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }

      // Must set the sky model to/from SilverLining NOT in this loop, as this is during
      // rendering, and SL messes up state. Better to postpone using a one-time operation.
      auto newSkyModel = static_cast<SkyModel>(currentModelIdx);
      if (newSkyModel != app_.skyModel())
        app_.mainView->getViewerBase()->addUpdateOperation(new SetSkyModelOperation(app_, newSkyModel));

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  AppData& app_;
};

#else
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
  vbox->addControl(new LabelControl("Sky Model Example", 20.f, simVis::Color::Yellow));

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
  app.skyModelText = grid->setControl(col + 2, row, new LabelControl("Sky Model", simVis::Color::White));

  return vbox;
}
#endif

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

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewMan->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewMan->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  // Create the User Interface controls
  mainView->addOverlayControl(createUi(app));
#endif

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
