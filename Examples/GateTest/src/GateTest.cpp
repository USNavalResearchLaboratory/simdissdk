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

/**
 * Gate TEST
 * Test app for the various features of the GateNode.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simVis/EntityLabel.h"
#include "simVis/Gate.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Platform.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simData/MemoryDataStore.h"
#include "simUtil/PlatformSimulator.h"
/// paths to models
#include "simUtil/ExampleResources.h"

#include <osgEarth/StringUtils>
#include <osgEarthUtil/Controls>
namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

namespace
{
  std::string SAYBOOL(bool x)
  {
    return x ? "ON" : "OFF";
  }
}

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<ui::HSliderControl> typeSlider_;
  osg::ref_ptr<ui::LabelControl>   typeLabel_;

  osg::ref_ptr<ui::HSliderControl> modeSlider_;
  osg::ref_ptr<ui::LabelControl>   modeLabel_;

  osg::ref_ptr<ui::HSliderControl> fillPatternSlider_;
  osg::ref_ptr<ui::LabelControl>   fillPatternLabel_;

  osg::ref_ptr<ui::HSliderControl> rangeMinSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeMinLabel_;

  osg::ref_ptr<ui::HSliderControl> rangeMaxSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeMaxLabel_;

  osg::ref_ptr<ui::HSliderControl> horizSlider_;
  osg::ref_ptr<ui::LabelControl>   horizLabel_;

  osg::ref_ptr<ui::HSliderControl> vertSlider_;
  osg::ref_ptr<ui::LabelControl>   vertLabel_;

  osg::ref_ptr<ui::HSliderControl> azimuthSlider_;
  osg::ref_ptr<ui::LabelControl>   azimuthLabel_;

  osg::ref_ptr<ui::HSliderControl> elevSlider_;
  osg::ref_ptr<ui::LabelControl>   elevLabel_;

  osg::ref_ptr<ui::HSliderControl> colorSlider_;
  osg::ref_ptr<ui::LabelControl>   colorLabel_;

  osg::ref_ptr<ui::CheckBoxControl> lightedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> globalToggle_;

  std::vector< std::pair<simData::GateProperties_GateType, std::string> > types_;
  std::vector< std::pair<simData::GatePrefs_DrawMode,      std::string> > modes_;
  std::vector< std::pair<simData::GatePrefs_FillPattern,   std::string> > fillPatterns_;
  std::vector< std::pair<simVis::Color, std::string> >                    colors_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  simData::ObjectId    gateId_;
  osg::ref_ptr<simVis::View> view_;
  double               t_;

  AppData()
   : typeSlider_(NULL),
     typeLabel_(NULL),
     modeSlider_(NULL),
     modeLabel_(NULL),
     fillPatternSlider_(NULL),
     fillPatternLabel_(NULL),
     rangeMinSlider_(NULL),
     rangeMinLabel_(NULL),
     rangeMaxSlider_(NULL),
     rangeMaxLabel_(NULL),
     horizSlider_(NULL),
     horizLabel_(NULL),
     vertSlider_(NULL),
     vertLabel_(NULL),
     azimuthSlider_(NULL),
     azimuthLabel_(NULL),
     elevSlider_(NULL),
     elevLabel_(NULL),
     colorSlider_(NULL),
     colorLabel_(NULL),
     lightedCheck_(NULL),
     ds_(NULL),
     hostId_(0),
     gateId_(0),
     view_(NULL),
     t_(0.0)
  {
    types_.push_back(std::make_pair(simData::GateProperties_GateType_ABSOLUTE_POSITION, "ABSOLUTE"));
    types_.push_back(std::make_pair(simData::GateProperties_GateType_BODY_RELATIVE,     "BODY RELATIVE"));

    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_ANGLE,    "ANGLE"));
    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_CLUTTER,  "CLUTTER"));
    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_COVERAGE, "COVERAGE"));

    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_STIPPLE,  "STIPPLE"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_SOLID,    "SOLID"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_ALPHA,    "ALPHA"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_WIRE,     "WIRE"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_CENTROID, "CENTROID"));

    colors_.push_back(std::make_pair(simVis::Color(0xffffff7f), "White"));
    colors_.push_back(std::make_pair(simVis::Color(0x00ff007f), "Green"));
    colors_.push_back(std::make_pair(simVis::Color(0xff00007f), "Red"));
    colors_.push_back(std::make_pair(simVis::Color(0xff7f007f), "Orange"));
    colors_.push_back(std::make_pair(simVis::Color(0xffff007f), "Yellow"));
  }

  void apply()
  {
    simData::DataStore::Transaction xaction;

    t_ += 1.0;
    int typeIndex;
    int modeIndex  = simCore::sdkMax(0, (int)floor(modeSlider_->getValue()));
    int colorIndex = simCore::sdkMax(0, (int)floor(colorSlider_->getValue()));
    int fillPatternIndex = simCore::sdkMax(0, (int)floor(fillPatternSlider_->getValue()));

    // fetch properties:
    {
      const simData::GateProperties* props = ds_->gateProperties(gateId_, &xaction);
      typeIndex = props->type() == simData::GateProperties_GateType_ABSOLUTE_POSITION ? 0 : 1;
      xaction.complete(&props);
    }

    // apply preferences:
    {
      simData::GatePrefs* prefs = ds_->mutable_gatePrefs(gateId_, &xaction);
      prefs->mutable_commonprefs()->set_draw(true);

      prefs->mutable_commonprefs()->set_color(colors_[colorIndex].first.as(simVis::Color::RGBA));
      prefs->set_fillpattern(fillPatterns_[fillPatternIndex].first);

      prefs->set_gatedrawmode(modes_[modeIndex].first);
      prefs->set_gatelighting(lightedCheck_->getValue());
      xaction.complete(&prefs);
    }

    // apply update:
    {
      simData::GateUpdate* update = ds_->addGateUpdate(gateId_, &xaction);
      update->set_time(t_);

      update->set_minrange(rangeMinSlider_->getValue());
      update->set_maxrange(rangeMaxSlider_->getValue());
      update->set_centroid(0.5 * (rangeMaxSlider_->getValue() + rangeMinSlider_->getValue()));
      update->set_azimuth(azimuthSlider_->getValue() * simCore::DEG2RAD);
      update->set_elevation(elevSlider_->getValue() * simCore::DEG2RAD);
      update->set_width(horizSlider_->getValue() * simCore::DEG2RAD);
      update->set_height(vertSlider_->getValue() * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_->update(t_);

    // update labels.
    typeLabel_->setText(types_[typeIndex].second);
    modeLabel_->setText(modes_[modeIndex].second);
    fillPatternLabel_->setText(fillPatterns_[fillPatternIndex].second);
    colorLabel_->setText(colors_[colorIndex].second);

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_->getValue() ?
      (displayMask |  simVis::DISPLAY_MASK_GATE) :
      (displayMask & ~simVis::DISPLAY_MASK_GATE));
  }
};

//----------------------------------------------------------------------------

struct ApplyUI : public ui::ControlEventHandler
{
  explicit ApplyUI(AppData* app) : app_(app) { }
  AppData* app_;
  void onValueChanged(ui::Control* c, bool value) { app_->apply(); }
  void onValueChanged(ui::Control* c, float value) { app_->apply(); }
  void onValueChanged(ui::Control* c, double value) { onValueChanged(c, (float)value); }
  void onClick(ui::Control* c, int buttons) { }
};

ui::Control* createUI(AppData& app)
{
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(&app);

  // top is returned to caller, memory owned by caller
  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("GATES - Test App", 22.0f, osg::Vec4(1, 1, 0, 1)));

  int c=0, r=0;
  ui::Grid* grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Type"));
  app.typeLabel_ = grid->setControl(c+1, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Draw Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 0, applyUI));
  app.modeSlider_->setHorizFill(true, 250);
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Fill Pattern"));
  app.fillPatternSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.fillPatterns_.size(), 0, applyUI));
  app.fillPatternSlider_->setHorizFill(true, 250);
  app.fillPatternLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Min Range"));
  app.rangeMinSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 100.0, applyUI));
  app.rangeMinLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeMinSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Max Range"));
  app.rangeMaxSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 350.0, applyUI));
  app.rangeMaxLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeMaxSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Horiz. Size"));
  app.horizSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 400.0, 45.0, applyUI));
  app.horizLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.horizSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Vert. Size"));
  app.vertSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 200.0, 45.0, applyUI));
  app.vertLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.vertSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Azimuth"));
  app.azimuthSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-180.0, 180.0, 0.0, applyUI));
  app.azimuthLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.azimuthSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Elevation"));
  app.elevSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-90, 90.0, 0.0, applyUI));
  app.elevLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.elevSlider_));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Color"));
  app.colorSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors_.size()-1, 0, applyUI));
  app.colorLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Lighted"));
  app.lightedCheck_ = grid->setControl(c + 1, r, new ui::CheckBoxControl(false, applyUI));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Global Gate Toggle"));
  app.globalToggle_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI));

  return top;
}

//----------------------------------------------------------------------------

/// Add a platform to use for the test.
simData::ObjectId addPlatform(simData::DataStore& ds,
                              int                 argc,
                              char**              argv)
{
  simData::ObjectId               hostId;
  simData::DataStore::Transaction xaction;

  // create the platform
  {
    simData::PlatformProperties* props = ds.addPlatform(&xaction);
    hostId = props->id();
    xaction.complete(&props);
  }

  // configure initial preferences
  {
    simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(hostId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(1.0);
    prefs->set_dynamicscale(false);
    prefs->mutable_commonprefs()->set_name("My Platform");
    prefs->mutable_commonprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // place it somewhere.
  {
    simCore::Vec3 pos(simCore::DEG2RAD*51.0, 0.0, 25000.0);

    simCore::Vec3 ori = simExamples::hasArg("--br", argc, argv) ?
      simCore::Vec3(simCore::DEG2RAD*45.0, simCore::DEG2RAD*45.0, 0.0) :
      simCore::Vec3(0.0, 0.0, 0.0);

    simCore::Coordinate lla(simCore::COORD_SYS_LLA, pos, ori);
    simCore::Coordinate ecef;
    simCore::CoordinateConverter conv;
    conv.convert(lla, ecef, simCore::COORD_SYS_ECEF);

    simData::PlatformUpdate* update = ds.addPlatformUpdate(hostId, &xaction);
    update->setPosition(ecef.position());
    update->setOrientation(ecef.orientation());
    update->set_time(0);
    xaction.complete(&update);

    // Note that each property update ticks 1 second; make the platform persist from
    // time 0 to time 1e5, allowing for 1e5 updates before the platform disappears.
    update = ds.addPlatformUpdate(hostId, &xaction);
    update->setPosition(ecef.position());
    update->setOrientation(ecef.orientation());
    update->set_time(1e5);
    xaction.complete(&update);
  }

  // tick the clock.
  ds.update(0);

  return hostId;
}

simData::ObjectId addGate(simData::DataStore& ds,
                          simData::ObjectId   hostId,
                          int                 argc,
                          char**              argv)
{
  // see if they user wants body-relative mode
  simData::GateProperties_GateType type = simExamples::hasArg("--br", argc, argv)?
    simData::GateProperties_GateType_BODY_RELATIVE :
    simData::GateProperties_GateType_ABSOLUTE_POSITION;

  simData::ObjectId gateId;

  // create the beam
  {
    simData::DataStore::Transaction xaction;
    simData::GateProperties* props = ds.addGate(&xaction);
    gateId = props->id();
    props->set_hostid(hostId);
    props->set_type(type);
    xaction.complete(&props);
  }

  // tick the clock
  ds.update(0);

  return gateId;
}

//----------------------------------------------------------------------------

int usage(char** argv)
{
  SIM_NOTICE << "USAGE: " << argv[0] << "\n"
    << "    --help               : this message\n"
    << "    --br                 : body-relative mode\n";
  return 0;
}

int main(int argc, char** argv)
{
  /// usage?
  if (simExamples::hasArg("--help", argc, argv))
    return usage(argv);

  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map);
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  // disable lighting on the map node.
  simVis::setLighting(scene->getMapNode()->getOrCreateStateSet(), 0);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// Set up the application data
  AppData app;
  app.ds_     = &dataStore;
  app.view_   = viewer->getMainView();

  /// add in the platform and beam
  app.hostId_ = addPlatform(dataStore, argc, argv);
  app.gateId_ = addGate(dataStore, app.hostId_, argc, argv);

  osg::observer_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(app.hostId_);
  app.view_->tetherCamera(platformModel.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  viewer->run();
}

