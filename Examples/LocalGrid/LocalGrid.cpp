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
 * LocalGrid shows how the manipulation of various local grid settings will
 * impact the display of the grid.  This is useful for seeing the impact of
 * various settings and testing the local grid code.
 */
#include <sstream>

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Angle.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Scenario.h"
#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"

#define LC "[LocalGrid demo] "

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif

//----------------------------------------------------------------------------

namespace {

static std::string s_title = "Local Grid Example";
static const double LATITUDE = 22.326; // degrees
static const double LONGITUDE = -159.878; // degrees
static const double HEADING = 45; // degrees
static const double SPEED = 10; // m/s

//----------------------------------------------------------------------------

/** Creates an entity at LATITUDE LONGITUDE */
simData::ObjectId createPlatform(simData::DataStore& ds)
{
  simData::ObjectId id = 0;
  { // Create the entity and get its ID
    simData::DataStore::Transaction txn;
    simData::PlatformProperties* props = ds.addPlatform(&txn);
    id = props->id();
    txn.complete(&props);
  }

  { // Set some default prefs
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(id, &txn);
    prefs->mutable_commonprefs()->set_name("Entity");
    prefs->mutable_commonprefs()->mutable_localgrid()->set_drawgrid(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(-10);
    prefs->set_icon(EXAMPLE_SHIP_ICON);
    prefs->set_dynamicscale(true);
    prefs->set_dynamicscalescalar(2.0);
    prefs->set_drawvelocityvec(true);
    txn.complete(&prefs);
  }

  { // Add an update
    const double headingRad = HEADING * simCore::DEG2RAD;
    simCore::Coordinate lla(simCore::COORD_SYS_LLA,
      simCore::Vec3(LATITUDE * simCore::DEG2RAD, LONGITUDE * simCore::DEG2RAD, 0.0),
      simCore::Vec3(headingRad, 0.0, 0.0),
      simCore::Vec3(SPEED * sin(headingRad), SPEED * cos(headingRad), 0.0)
    );
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);

    // Add an update with the ECEF coordinate
    simData::DataStore::Transaction txn;
    simData::PlatformUpdate* update = ds.addPlatformUpdate(id, &txn);
    update->set_time(0.0);
    update->set_x(ecef.x());
    update->set_y(ecef.y());
    update->set_z(ecef.z());
    update->set_psi(ecef.psi());
    update->set_theta(ecef.theta());
    update->set_phi(ecef.phi());
    update->set_vx(ecef.vx());
    update->set_vy(ecef.vy());
    update->set_vz(ecef.vz());

    // Complete the transaction
    txn.complete(&update);
  }

  ds.update(0.0);
  return id;
}

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(150); func("##" label, __VA_ARGS__)

struct ControlPanel : public GUI::BaseGui
{
  ControlPanel(simData::DataStore& ds, simData::ObjectId id)
    : BaseGui(s_title),
    ds_(ds),
    id_(id)
  {
    update_();
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      bool drawGrid = drawGrid_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Draw Grid", &drawGrid_);
      if (drawGrid != drawGrid_)
        needUpdate = true;

      // Type combo box
      ImGui::TableNextColumn(); ImGui::Text("Type"); ImGui::TableNextColumn();
      static const char* TYPES[] = { "Cartesian", "Polar", "Range Rings", "Speed Rings", "Speed Line" };
      static int currentTypeIdx = static_cast<int>(type_) - 1;
      if (ImGui::BeginCombo("##type", TYPES[currentTypeIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(TYPES); i++)
        {
          const bool isSelected = (currentTypeIdx == i);
          if (ImGui::Selectable(TYPES[i], isSelected))
            currentTypeIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentTypeIdx + 1 != static_cast<int>(type_))
      {
        needUpdate = true;
        type_ = static_cast<simData::LocalGridPrefs_Type>(currentTypeIdx + 1);
      }

      int prec = prec_;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Text Precision", &prec_, 0, 10, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (prec != prec_)
        needUpdate = true;

      if (needUpdate)
        update_();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  void update_()
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = ds_.mutable_platformPrefs(id_, &txn);
    prefs->mutable_commonprefs()->mutable_localgrid()->set_drawgrid(drawGrid_);
    prefs->mutable_commonprefs()->mutable_localgrid()->set_gridtype(type_);
    prefs->mutable_commonprefs()->mutable_localgrid()->set_gridlabelprecision(prec_);
    txn.complete(&prefs);
  }

  simData::DataStore& ds_;
  simData::ObjectId id_;
  bool drawGrid_ = true;
  simData::LocalGridPrefs::Type type_ = simData::LocalGridPrefs::POLAR;
  int prec_ = 1;
};

#else

/** Base class that provides a useful mergeIn() method */
class ApplyPrefs : public ui::ControlEventHandler
{
public:
  ApplyPrefs(simData::DataStore& ds, simData::ObjectId id)
    : ds_(ds),
      id_(id)
  {
  }

  /** Given sparse input message, merges changes into the current datastore platform settings */
  void mergeIn(const simData::PlatformPrefs& prefs)
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* dsPrefs = ds_.mutable_platformPrefs(id_, &txn);
    dsPrefs->MergeFrom(prefs);
    txn.complete(&dsPrefs);
  }

private:
  simData::DataStore& ds_;
  simData::ObjectId id_;
};

/** Turns the grid on and off */
class ToggleLocalGrid : public ApplyPrefs
{
public:
  ToggleLocalGrid(simData::DataStore& ds, simData::ObjectId id) : ApplyPrefs(ds, id) { }

  virtual void onValueChanged(ui::Control* control, bool value)
  {
    simData::PlatformPrefs prefs;
    prefs.mutable_commonprefs()->mutable_localgrid()->set_drawgrid(value);
    mergeIn(prefs);
  }
};

/** Changes the grid type to a specific display format */
class ApplyGridType : public ApplyPrefs
{
public:
  ApplyGridType(simData::DataStore& ds, simData::ObjectId id, simData::LocalGridPrefs::Type type)
   : ApplyPrefs(ds, id),
     type_(type)
  { }

  virtual void onClick(ui::Control* control)
  {
    simData::PlatformPrefs prefs;
    prefs.mutable_commonprefs()->mutable_localgrid()->set_gridtype(type_);
    mergeIn(prefs);
  }

private:
  simData::LocalGridPrefs::Type type_;
};

/** Changes the text precision on the labels for the grid */
class ApplyGridTextPrecision : public ApplyPrefs
{
public:
  ApplyGridTextPrecision(simData::DataStore& ds, simData::ObjectId id, ui::LabelControl* label)
   : ApplyPrefs(ds, id),
     label_(label)
  {
  }

  virtual void onValueChanged(ui::Control* control, float valueFloat)
  {
    int value = static_cast<int>(valueFloat + 0.5f);
    simData::PlatformPrefs prefs;
    prefs.mutable_commonprefs()->mutable_localgrid()->set_gridlabelprecision(value);
    mergeIn(prefs);
    if (label_)
    {
      std::stringstream ss;
      ss << value;
      label_->setText(ss.str());
    }
  }

private:
  ui::LabelControl* label_;
};

ui::Control* createHelp(simData::DataStore& ds, simData::ObjectId id)
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));

  // Grid for the various controls, memory will be owned by vbox
  ui::Grid* grid = new ui::Grid();
  grid->setBackColor(0, 0, 0, 0.5);
  grid->setMargin(10);
  grid->setPadding(10);
  grid->setChildSpacing(10);
  grid->setChildVertAlign(ui::Control::ALIGN_CENTER);
  grid->setAbsorbEvents(true);
  grid->setVertAlign(ui::Control::ALIGN_BOTTOM);
  vbox->addControl(grid);

  int row = 0;

  grid->setControl(0, row, new ui::LabelControl("Draw Grid"));
  ui::CheckBoxControl* drawGrid = new ui::CheckBoxControl(true);
  drawGrid->addEventHandler(new ToggleLocalGrid(ds, id));
  grid->setControl(1, row, drawGrid);

  row++;

  grid->setControl(0, row, new ui::LabelControl("Type"));
  ui::LabelControl* cartesian = new ui::LabelControl("Cartesian");
  cartesian->addEventHandler(new ApplyGridType(ds, id, simData::LocalGridPrefs::CARTESIAN));
  grid->setControl(1, row, cartesian);
  ui::LabelControl* polar = new ui::LabelControl("Polar");
  polar->addEventHandler(new ApplyGridType(ds, id, simData::LocalGridPrefs::POLAR));
  grid->setControl(2, row, polar);
  ui::LabelControl* rangeRings = new ui::LabelControl("Range Rings");
  rangeRings->addEventHandler(new ApplyGridType(ds, id, simData::LocalGridPrefs::RANGE_RINGS));
  grid->setControl(3, row, rangeRings);
  ui::LabelControl* speedRings = new ui::LabelControl("Speed Rings");
  speedRings->addEventHandler(new ApplyGridType(ds, id, simData::LocalGridPrefs::SPEED_RINGS));
  grid->setControl(4, row, speedRings);
  ui::LabelControl* speedLine = new ui::LabelControl("Speed Line");
  speedLine->addEventHandler(new ApplyGridType(ds, id, simData::LocalGridPrefs::SPEED_LINE));
  grid->setControl(5, row, speedLine);

  row++;

  grid->setControl(0, row, new ui::LabelControl("Text Precision"));
  ui::HSliderControl* precision = new ui::HSliderControl(0, 10, 1);
  grid->setControl(1, row, precision);
  ui::LabelControl* precisionLabel = new ui::LabelControl("1");
  grid->setControl(2, row, precisionLabel);
  precision->addEventHandler(new ApplyGridTextPrecision(ds, id, precisionLabel));

  return vbox;
}

#endif

}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  if (arguments.read("--multisample"))
    osg::DisplaySettings::instance()->setNumMultiSamples(4);
  simExamples::configureSearchPaths();

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // Put viewer into a window
  viewer->getMainView()->setUpViewInWindow(20, 20, 1024, 768, 0);

  // set an initial viewpoint
  simVis::Viewpoint viewPoint("Start",
      LONGITUDE, LATITUDE, 0.0,
      0, -75, 6.0e4);
  viewer->getMainView()->setViewpoint(viewPoint);

  // for status and debugging
  viewer->installDebugHandlers();

  // Add a scenario
  simData::MemoryDataStore dataStore;
  simVis::ScenarioDataStoreAdapter adapter(&dataStore, viewer->getSceneManager()->getScenario());

  // Create the entity
  simData::ObjectId platformId = createPlatform(dataStore);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, platformId));
#else
  // Create a HUD
  ui::ControlCanvas* canvas = ui::ControlCanvas::get(viewer->getMainView());
  canvas->addControl(createHelp(dataStore, platformId));
#endif

  return viewer->run();
}

