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

#include "osgDB/ReadFile"
#include "osgEarth/Map"
#include "osgEarth/MapNode"
#include "osgEarth/VisibleLayer"
#include "osgEarth/DateTime"
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif


// Structure that holds all our application data.
struct App
{
  App()
    : activeLayer_(nullptr),
      firstTime_(INT_MAX),
      lastTime_(0),
      now_(0)
  {
  }

#ifndef HAVE_IMGUI
  // UI controls
  ui::HSliderControl* timeSlider_ = nullptr;
  ui::LabelControl* clockLabel_ = nullptr;
#endif

  std::string timeLabel_;

  // Table that holds all timestamped layers, sorted by time:
  typedef std::map<osgEarth::TimeStamp, osg::ref_ptr<osgEarth::VisibleLayer> > LayerTable;
  LayerTable layers_;

  // Currently visible layer:
  osgEarth::VisibleLayer* activeLayer_;

  // Start and end times of the layer series (buffered out a little)
  osgEarth::TimeStamp firstTime_, lastTime_;

  // Current clock time.
  float now_;

  // Set a new clock time.
  void setTime(double t)
  {
    now_ = t;

    // If we had a DataStore, we could call dataStore->update(t) here.

    osgEarth::DateTime dt(t);

    timeLabel_ = dt.asISO8601();

#ifndef HAVE_IMGUI
    timeSlider_->setValue(t, false);
    clockLabel_->setText(timeLabel_);
#endif

    // Make the appropriate layer visible.
    LayerTable::const_iterator i = layers_.lower_bound(t);
    if (i != layers_.end())
    {
      if (activeLayer_ && activeLayer_ != i->second.get())
      {
        activeLayer_->setVisible(false);
      }

      if (activeLayer_ != i->second.get())
      {
        activeLayer_ = i->second.get();
        activeLayer_->setVisible(true);
      }
    }
  }
};

#ifdef HAVE_IMGUI

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(App& app)
    : simExamples::SimExamplesGui("Timestamped Layer Example"),
    app_(app)
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

    if (ImGui::BeginTable("table", 2))
    {
      ImGui::TableNextColumn(); ImGui::Text("Time:"); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200.f);
      // Min Range
      float time = app_.now_;
      ImGui::SliderFloat("##Time", &time, app_.firstTime_, app_.lastTime_, "", ImGuiSliderFlags_AlwaysClamp);
      if (time != app_.now_)
        app_.setTime(time);

      ImGui::TableNextColumn(); ImGui::Text("Clock:"); ImGui::TableNextColumn();
      if (!app_.timeLabel_.empty())
        ImGui::Text(app_.timeLabel_.c_str());

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  App& app_;
};

#else

// Callback to set the current clock using the slider
struct ChangeTime : public ui::ControlEventHandler
{
  App& app_;
  explicit ChangeTime(App& app)
    : app_(app)
  {
  }
  void onValueChanged(ui::Control* control, float value)
  {
    app_.setTime(value);
  }
};

// Builds a UI to control the clock
ui::Control* createUI(App& app)
{
  ui::Grid* grid = new ui::Grid();
  grid->setPadding(10);
  grid->setBackColor(0, 0, 0, 0.6);
  int row = 0;

  // The time slider
  grid->setControl(0, row, new ui::LabelControl("Time:"));
  app.timeSlider_ = grid->setControl(1, row, new ui::HSliderControl(
    app.firstTime_, app.lastTime_, app.firstTime_,
    new ChangeTime(app)));
  ++row;

  // The clock label
  grid->setControl(0, row, new ui::LabelControl("Clock:"));
  app.clockLabel_ = grid->setControl(1, row, new ui::LabelControl());
  ++row;

  app.timeSlider_->setHorizFill(true, 400.f);

  return grid;
}

#endif


// Loads an earth file and installs its map in the viewer.
bool loadEarthFile(const std::string& earthFile, simVis::Viewer* viewer)
{
  osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(earthFile);
  osgEarth::MapNode* mapNode = osgEarth::MapNode::get(node.get());
  if (mapNode == nullptr)
    return false;

  viewer->setMapNode(mapNode);
  return true;
}

// Looks in the map, finds any layer with a "time" property in ISO8601 format
// and remembers it in the App.
void discoverTimestampedLayers(App& app, osgEarth::Map* map)
{
  typedef std::vector<osg::ref_ptr<osgEarth::VisibleLayer> > VisibleLayerVector;

  // collect all visible layers
  VisibleLayerVector layers;
  map->getLayers(layers);

  for (VisibleLayerVector::iterator i = layers.begin(); i != layers.end(); ++i)
  {
    osgEarth::VisibleLayer* layer = i->get();

    // Gets the serialized data so we can look for custom attributes
    osgEarth::Config conf = layer->getConfig();

    std::string iso8601 = conf.value("time");
    // Fall back to "times" if possible
    if (iso8601.empty())
      iso8601 = conf.value("times");
    if (!iso8601.empty())
    {
      osgEarth::DateTime dt(iso8601);

      // Convert to seconds post epoch
      osgEarth::TimeStamp timestamp = dt.asTimeStamp();

      // Compute the earliest and latest times
      if (timestamp < app.firstTime_)
        app.firstTime_ = timestamp;

      if (timestamp > app.lastTime_)
        app.lastTime_ = timestamp;

      app.layers_[timestamp] = layer;

      SIM_NOTICE << "Found layer \"" << layer->getName()
        << "\" with timestamp " << dt.asISO8601() << std::endl;

      // Start them all invisible
      layer->setVisible(false);
    }
  }

  // Buffer the start and end times a little to accommodate the time slider.
  if (app.layers_.size() >= 2)
  {
    App::LayerTable::const_iterator begin = app.layers_.begin();
    double t0 = begin->first;
    ++begin;
    double t1 = begin->first;
    app.firstTime_ -= (t1-t0);

    App::LayerTable::const_iterator end = app.layers_.end();
    --end;
    t1 = end->first; --end;
    t0 = end->first;
    app.lastTime_ += (t1-t0);
  }
}


int main(int argc, char** argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  if (argc != 2)
  {
    std::cerr << "USAGE:\n"
      << argv[0] << " <terrain.earth>\n\n"
      << "  <terrain.earth>: Earth file to load.\n\n";
    return -1;
  }

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  // Load the earth file containing the timestamped layers:
  std::string earthFile = argv[argc-1];
  if (loadEarthFile(earthFile, viewer.get()) == false)
    return -1;

  // App holds all the application data.
  App app;

  // Locate and remember the timestamped layers in the earth file.
  discoverTimestampedLayers(app, viewer->getSceneManager()->getMap());
  if (app.layers_.empty())
  {
    SIM_WARN << "No timestamped layers found!\n";
    return -1;
  }

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  // Install the time slider UI:
  viewer->getMainView()->addOverlayControl(createUI(app));
#endif

  // Set the initial time to the time of the first timestamped layer:
  app.setTime(app.firstTime_);

  // Run the clock.
  double last_t = 0.0;
  while (!viewer->getViewer()->done())
  {
    double t = viewer->getViewer()->getFrameStamp()->getReferenceTime();
    double dt = t - last_t;
    app.setTime(app.now_ + dt);
    last_t = t;

    viewer->frame();
  }
  return 0;
}
