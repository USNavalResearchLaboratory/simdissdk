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
/* -*-c++-*- */
/**
 * Demonstrates and tests the loading and display of a radar cross section (RCS) on a platform.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simVis/Beam.h"
#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth;
using namespace osgEarth::Util;

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
using namespace osgEarth::Util::Controls;
#endif

//----------------------------------------------------------------------------

static const std::string s_title = "RCS Example";


struct AppData
{
  simData::MemoryDataStore   ds;
  simData::ObjectId          platformId;

#ifdef HAVE_IMGUI
  bool draw2D = true;
  bool draw3D = true;
  simData::Polarity polarity = simData::POL_UNKNOWN;
  float freq = 7000.f;
  float elev = 45.f;
  float detail = 5.;
#else
  osg::ref_ptr<CheckBoxControl>  draw2D;
  osg::ref_ptr<CheckBoxControl>  draw3D;
  osg::ref_ptr<HSliderControl>   polarity;
  osg::ref_ptr<HSliderControl>   frequency;
  osg::ref_ptr<HSliderControl>   elevation;
  osg::ref_ptr<HSliderControl>   detail;
  osg::ref_ptr<LabelControl>     polarityLabel;
#endif

  void applyPrefs()
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(platformId, &xaction);
    {
#ifdef HAVE_IMGUI
      prefs->set_drawrcs(draw2D);
      prefs->set_draw3drcs(draw3D);
      prefs->set_rcsdetail(detail);
      prefs->set_rcselevation(elev);
      prefs->set_rcsfrequency(freq);
      prefs->set_rcspolarity(polarity);
#else
      unsigned polarityIndex = (unsigned)floor(polarity->getValue());
      prefs->set_drawrcs(draw2D->getValue());
      prefs->set_draw3drcs(draw3D->getValue());
      prefs->set_rcsdetail(detail->getValue());
      prefs->set_rcselevation(elevation->getValue());
      prefs->set_rcsfrequency(frequency->getValue());
      prefs->set_rcspolarity((simData::Polarity)polarityIndex);
      polarityLabel->setText(simCore::polarityString((simCore::PolarityType)polarityIndex));
#endif
    }
    xaction.complete(&prefs);
  }
};

#ifdef HAVE_IMGUI
// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public GUI::BaseGui
{
public:
  explicit ControlPanel(AppData& app)
    : GUI::BaseGui(s_title),
    app_(app)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      bool draw2D = app_.draw2D;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Draw 2D", &app_.draw2D);
      if (draw2D != app_.draw2D)
        needUpdate = true;

      bool draw3D = app_.draw3D;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Draw 3D", &app_.draw3D);
      if (draw3D != app_.draw3D)
        needUpdate = true;

      // Polarity combo box
      ImGui::TableNextColumn(); ImGui::Text("Polarity"); ImGui::TableNextColumn();
      static const char* POLARITY[] = { "UNKNOWN", "HORIZONTAL", "VERTICAL", "CIRCULAR", "HORZVERT", "VERTHORZ", "LEFTCIRC", "RIGHTCIRC", "LINEAR" };
      static int currentPolIdx = 0;
      if (ImGui::BeginCombo("##pol", POLARITY[currentPolIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(POLARITY); i++)
        {
          const bool isSelected = (currentPolIdx == i);
          if (ImGui::Selectable(POLARITY[i], isSelected))
            currentPolIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentPolIdx != static_cast<int>(app_.polarity))
      {
        needUpdate = true;
        app_.polarity = static_cast<simData::Polarity>(currentPolIdx);
      }

      float freq = app_.freq;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Frequency", &app_.freq, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (freq != app_.freq)
        needUpdate = true;

      float elev = app_.elev;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Elevation", &app_.elev, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (elev != app_.elev)
        needUpdate = true;

      float detail = app_.detail;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Detail Angle", &app_.detail, 1.f, 15.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (detail != app_.detail)
        needUpdate = true;

      if (needUpdate)
        app_.applyPrefs();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  AppData& app_;
};
#else
struct ApplyUI : public ControlEventHandler
{
  explicit ApplyUI(AppData* app) : app_(app) { }
  AppData* app_;
  void onValueChanged(Control* c, bool value) { app_->applyPrefs(); }
  void onValueChanged(Control* c, float value) { app_->applyPrefs(); }
  void onValueChanged(Control* c, double value) { onValueChanged(c, (float)value); }
};


Control* createUI(AppData* app)
{
  VBox* vbox = new VBox();
  vbox->setAbsorbEvents(true);
  vbox->setVertAlign(Control::ALIGN_TOP);
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20.f, simVis::Color::Yellow));

  // sensor parameters
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(app);

  osg::ref_ptr<Grid> g = vbox->addControl(new Grid());
  unsigned row=0, col=0;

  row++;
  app->draw2D = g->setControl(col, row, new CheckBoxControl(true, applyUI.get()));
  g->setControl(col+1, row, new LabelControl("Draw 2D RCS"));

  row++;
  app->draw3D = g->setControl(col, row, new CheckBoxControl(true, applyUI.get()));
  g->setControl(col+1, row, new LabelControl("Draw 3D RCS"));

  row++;
  g->setControl(col, row, new LabelControl("Polarity"));
  app->polarity = g->setControl(col+1, row, new HSliderControl(0.0, 9.0, 0.0, applyUI.get()));
  app->polarity->setHorizFill(true, 250.0);
  app->polarityLabel = g->setControl(col+2, row, new LabelControl());

  row++;
  g->setControl(col, row, new LabelControl("Frequency"));
  app->frequency = g->setControl(col+1, row, new HSliderControl(0.0, 10000.0, 7000.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->frequency.get()));
  app->frequency->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Elevation"));
  app->elevation = g->setControl(col+1, row, new HSliderControl(0.0, 90.0, 45.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->elevation.get()));
  app->elevation->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Detail angle"));
  app->detail = g->setControl(col+1, row, new HSliderControl(1.0, 15.0, 5.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->detail.get()));
  app->detail->setHorizFill(true, 250.0);

  return vbox;
}
#endif

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
void addPlatform(AppData* app)
{
  // create the platform:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* newProps = app->ds.addPlatform(&xaction);
    app->platformId = newProps->id();
    xaction.complete(&newProps);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = app->ds.mutable_platformPrefs(app->platformId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_rcsfile(EXAMPLE_RCS_FILE);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // apply the initial configuration:
  app->applyPrefs();
}

//----------------------------------------------------------------------------

void simulate(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.5, -0.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0.5,  0.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim.get());
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0);
  viewer->addEventHandler(simHandler.get());
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  AppData app;

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  // install the GUI
  viewer->getMainView()->addOverlayControl(createUI(&app));
#endif

  // Create the platform:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&app.ds);
  addPlatform(&app);

  // make the sim
  simulate(app.platformId, app.ds, viewer.get());

  // zoom the camera
  viewer->getMainView()->tetherCamera(scene->getScenario()->find(app.platformId));
  viewer->getMainView()->setFocalOffsets(0, -45, 800.);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
