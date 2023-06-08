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
 * Demonstration of fragment effects on platforms.
 */

#include <algorithm>
#include <functional>
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
#endif

//----------------------------------------------------------------------------

inline const char* EFFECT_NAMES[] = {
  "None",
  "Forward Stripe",
  "Backward Stripe",
  "Horizontal Stripe",
  "Vertical Stripe",
  "Checkerboard",
  "Diamond",
  "Glow",
  "Flash"
};
static const int EFFECT_COUNT = simData::FragmentEffect_ARRAYSIZE;

/**
 * Sets platform prefs using a lambda. Returns 0 if the prefs are set. For example:
 *
 * <code>
 * const int rv = setPlatformPrefs(dataStore, id, [](auto& prefs) {prefs.set_draw(true);});
 * </code>
 *
 * Multiple prefs may be set in this way.
 */
int setPlatformPrefs(simData::DataStore& dataStore, simData::ObjectId id, const std::function<void(simData::PlatformPrefs&)>& setFunc)
{
  simData::DataStore::Transaction txn;
  auto* prefs = dataStore.mutable_platformPrefs(id, &txn);
  if (!prefs)
    return 1;
  setFunc(*prefs);
  txn.complete(&prefs);
  return 0;
}

//----------------------------------------------------------------------------

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
// Forces a width of 150 on the sliders. Otherwise, the sliders claim no horizontal space and are unusable.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(150); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId id)
    : simExamples::SimExamplesGui("Fragment Effect Example"),
    ds_(ds),
    id_(id)
  {
    update_();
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
    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      // Extract the fragment effect
      simData::DataStore::Transaction txn;
      auto* prefs = ds_.platformPrefs(id_, &txn);
      currentEffect_ = prefs->fragmenteffect();
      txn.release(&prefs);

      int oldEffect = currentEffect_;
      IMGUI_ADD_ROW(ImGui::Combo, "Effect:", &currentEffect_, EFFECT_NAMES, EFFECT_COUNT);
      if (currentEffect_ != oldEffect)
        needUpdate = true;

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Scaling:");
      ImGui::TableNextColumn();
      nextScaleMult_ = 1.0;
      if (ImGui::Button("+"))
      {
        nextScaleMult_ = 2.0;
        needUpdate = true;
      }
      ImGui::SameLine(0.f, 10.f);
      if (ImGui::Button("-"))
      {
        nextScaleMult_ = 0.5;
        needUpdate = true;
      }

      ImGui::EndTable();
    }

    if (needUpdate)
      update_();

    ImGui::End();
  }

private:
  /** Update platform prefs using the current values */
  void update_()
  {
    setPlatformPrefs(ds_, id_, [this](simData::PlatformPrefs& prefs) {
      prefs.set_fragmenteffect(static_cast<simData::FragmentEffect>(currentEffect_));
      if (nextScaleMult_ != 1.0)
        prefs.set_scale(prefs.scale() * nextScaleMult_);
      });
  }

  simData::DataStore& ds_;
  simData::ObjectId id_ = 0;
  int currentEffect_ = simData::FE_NONE;
  double nextScaleMult_ = 1.0;
};

#else
osg::ref_ptr<osgEarth::Util::Controls::Control> createHelp()
{
  using VBox = osgEarth::Util::Controls::VBox;
  using LabelControl = osgEarth::Util::Controls::LabelControl;

  osg::ref_ptr<VBox> vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->setMargin(10);

  vbox->addControl(new LabelControl("Fragment Effect Example", 20.f, simVis::Color::Yellow));
  vbox->addControl(new LabelControl("1: Cycle Effect", 14.f, simVis::Color::White));
  vbox->addControl(new LabelControl("*: Increase Scale", 14.f, simVis::Color::White));
  vbox->addControl(new LabelControl("/: Decrease Scale", 14.f, simVis::Color::White));
  vbox->addControl(new LabelControl("c: Center on Platform", 14.f, simVis::Color::White));
  return vbox;
}

#endif

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore)
{
  // create the platform:
  simData::ObjectId platformId;
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
    platformId = props->id();
    xaction.complete(&props);
  }

  // now configure its preferences:
  {
    setPlatformPrefs(dataStore, platformId, [](simData::PlatformPrefs& prefs) {
      prefs.set_icon(EXAMPLE_AIRPLANE_ICON);
      prefs.set_scale(1.0f);
      prefs.set_dynamicscale(true);
      prefs.mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
      });
  }

  return platformId;
}

//----------------------------------------------------------------------------

void simulatePlatform(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(21.5, -158.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(21.5, -157.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim.get());
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0);
  viewer->addEventHandler(simHandler.get());
}

//----------------------------------------------------------------------------

/** Helper class to process keys and execute a function when pressed. */
class KeyEventHandler : public osgGA::GUIEventHandler
{
public:
  explicit KeyEventHandler(int keyOfInterest)
    : keyOfInterest_(keyOfInterest)
  {
  }

  // From GUIEventHandler:
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      if (ea.getKey() == keyOfInterest_)
      {
        execute();
        return true;
      }
    }
    return false;
  }

  /** Override this method to run code when your key of interest is pressed. */
  virtual void execute() = 0;

protected:
  // From osg::Referenced
  virtual ~KeyEventHandler() = default;

private:
  int keyOfInterest_ = 0;
};

//----------------------------------------------------------------------------

/** Helper to tie a lambda to a key press */
class LambdaKeyEventHandler : public KeyEventHandler
{
public:
  LambdaKeyEventHandler(int key, const std::function<void()>& lambda)
    : KeyEventHandler(key),
    lambda_(lambda)
  {
  }

  // From KeyEventHandler:
  virtual void execute() override
  {
    lambda_();
  }

protected:
  // From osg::Referenced
  virtual ~LambdaKeyEventHandler() = default;

private:
  std::function<void()> lambda_;
};

//----------------------------------------------------------------------------

/** Helper to tie a lambda to a key press */
class PrefsKeyEventHandler : public KeyEventHandler
{
public:
  PrefsKeyEventHandler(int key, simData::DataStore& dataStore, simData::ObjectId uid,
    const std::function<void(simData::PlatformPrefs&)>& setFunc)
    : KeyEventHandler(key),
    dataStore_(dataStore),
    uid_(uid),
    setFunc_(setFunc)
  {
  }

  // From KeyEventHandler:
  virtual void execute() override
  {
    setPlatformPrefs(dataStore_, uid_, setFunc_);
  }

protected:
  // From osg::Referenced
  virtual ~PrefsKeyEventHandler() = default;

private:
  simData::DataStore& dataStore_;
  simData::ObjectId uid_ = 0;
  std::function<void(simData::PlatformPrefs&)> setFunc_;
};

//----------------------------------------------------------------------------

class ChangeEffect
{
public:
  ChangeEffect(simData::DataStore& dataStore, simData::ObjectId entityId)
    : dataStore_(dataStore),
    entityId_(entityId)
  {
    setValue_(currentEffect_);
  }

  void cycleNext()
  {
    int nextValue = (static_cast<int>(currentEffect_) + 1);
    if (nextValue >= simData::FragmentEffect_ARRAYSIZE)
      nextValue = 0;
    setValue_(static_cast<simData::FragmentEffect>(nextValue));
  }

private:
  /** Changes the associated preference */
  void setValue_(simData::FragmentEffect effect)
  {
    currentEffect_ = effect;
    setPlatformPrefs(dataStore_, entityId_, [this](simData::PlatformPrefs& prefs) {
      prefs.set_fragmenteffect(currentEffect_);
      });
  }

  simData::DataStore& dataStore_;
  simData::ObjectId entityId_ = 0;
  simData::FragmentEffect currentEffect_ = simData::FE_NONE;
};

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  simExamples::addDefaultSkyNode(viewer.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // Set up the data:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);
  const simData::ObjectId platform = addPlatform(dataStore);
  simulatePlatform(platform, dataStore, viewer.get());

  // tick the sim
  dataStore.update(0);

  // zoom the camera
  simVis::View* mainView = viewer->getMainView();
  mainView->setFocalOffsets(0, -45, 15.0);

  // Center on platform and bind 'c' to recenter
  simVis::EntityNode* platformNode = scene->getScenario()->find(platform);
  const auto centerView = [mainView, platformNode]() {
    mainView->tetherCamera(platformNode);
  };
  centerView();
  mainView->addEventHandler(new LambdaKeyEventHandler('c', centerView));

  // Cycle fragment effects
  ChangeEffect cycle(dataStore, platform);
  mainView->addEventHandler(new LambdaKeyEventHandler('1', std::bind(&ChangeEffect::cycleNext, &cycle)));

  // Grow/shrink the model (use keypad or normal keys)
  const auto& scaleUp = [](simData::PlatformPrefs& prefs) {prefs.set_scale(prefs.scale() * 2.); };
  mainView->addEventHandler(new PrefsKeyEventHandler(osgGA::GUIEventAdapter::KEY_KP_Multiply, dataStore, platform, scaleUp));
  mainView->addEventHandler(new PrefsKeyEventHandler('*', dataStore, platform, scaleUp));
  const auto& scaleDown = [](simData::PlatformPrefs& prefs) {prefs.set_scale(prefs.scale() / 2.); };
  mainView->addEventHandler(new PrefsKeyEventHandler(osgGA::GUIEventAdapter::KEY_KP_Divide, dataStore, platform, scaleDown));
  mainView->addEventHandler(new PrefsKeyEventHandler('/', dataStore, platform, scaleDown));

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new ::GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);
  auto* panel = new ControlPanel(dataStore, platform);
  gui->add(panel);
#else
  /// show the instructions overlay
  mainView->addOverlayControl(createHelp());
#endif

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
