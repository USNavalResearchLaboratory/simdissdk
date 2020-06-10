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

/**
 * Beam TEST
 * Test app for the various features of the BeamNode.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simVis/Platform.h"
#include "simVis/Beam.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simData/MemoryDataStore.h"
#include "simUtil/PlatformSimulator.h"
/// paths to models
#include "simUtil/ExampleResources.h"

#include <osgEarth/Controls>
#include <osgEarth/StringUtils>
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

  osg::ref_ptr<ui::HSliderControl> rangeSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeLabel_;

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

  osg::ref_ptr<ui::HSliderControl> capResSlider_;
  osg::ref_ptr<ui::LabelControl>   capResLabel_;

  osg::ref_ptr<ui::HSliderControl> coneResSlider_;
  osg::ref_ptr<ui::LabelControl>   coneResLabel_;

  osg::ref_ptr<ui::CheckBoxControl> useOffsetIconCheck_;
  osg::ref_ptr<ui::CheckBoxControl> shadedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> blendedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> renderConeCheck_;
  osg::ref_ptr<ui::CheckBoxControl> animateCheck_;

  osg::ref_ptr<ui::CheckBoxControl> globalToggle_;

  std::vector< std::pair<simData::BeamProperties_BeamType, std::string> > types_;
  std::vector< std::pair<simData::BeamPrefs_DrawMode,      std::string> > modes_;
  std::vector< std::pair<simVis::Color, std::string> >                    colors_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  simData::ObjectId    beamId_;
  osg::ref_ptr<simVis::View> view_;
  double               t_;

  AppData()
   : typeSlider_(NULL),
     typeLabel_(NULL),
     modeSlider_(NULL),
     modeLabel_(NULL),
     rangeSlider_(NULL),
     rangeLabel_(NULL),
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
     capResSlider_(NULL),
     capResLabel_(NULL),
     coneResSlider_(NULL),
     coneResLabel_(NULL),
     useOffsetIconCheck_(NULL),
     shadedCheck_(NULL),
     blendedCheck_(NULL),
     renderConeCheck_(NULL),
     animateCheck_(NULL),
     ds_(NULL),
     hostId_(0),
     beamId_(0),
     view_(NULL),
     t_(0.0)
  {
    types_.push_back(std::make_pair(simData::BeamProperties_BeamType_ABSOLUTE_POSITION, "ABSOLUTE"));
    types_.push_back(std::make_pair(simData::BeamProperties_BeamType_BODY_RELATIVE,     "BODY RELATIVE"));

    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_SOLID, "SOLID"));
    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_WIRE,  "WIRE"));
    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_WIRE_ON_SOLID, "WIRE ON SOLID"));

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
    int capRes     = simCore::sdkMax(1, (int)floor(capResSlider_->getValue()));
    int coneRes    = simCore::sdkMax(1, (int)floor(coneResSlider_->getValue()));

    // fetch properties:
    {
      const simData::BeamProperties* props = ds_->beamProperties(beamId_, &xaction);
      typeIndex = props->type() == simData::BeamProperties_BeamType_ABSOLUTE_POSITION ? 0 : 1;
      xaction.complete(&props);
    }

    // apply preferences:
    {
      simData::BeamPrefs* prefs = ds_->mutable_beamPrefs(beamId_, &xaction);
      prefs->mutable_commonprefs()->set_draw(true);

      prefs->mutable_commonprefs()->set_color(colors_[colorIndex].first.as(simVis::Color::RGBA));

      prefs->set_beamdrawmode(modes_[modeIndex].first);
      prefs->set_horizontalwidth(horizSlider_->getValue() * simCore::DEG2RAD);
      prefs->set_verticalwidth(vertSlider_->getValue() * simCore::DEG2RAD);
      prefs->set_useoffseticon(useOffsetIconCheck_->getValue());
      prefs->set_shaded(shadedCheck_->getValue());
      prefs->set_blended(blendedCheck_->getValue());
      prefs->set_rendercone(renderConeCheck_->getValue());
      prefs->set_capresolution(capRes);
      prefs->set_coneresolution(coneRes);
      prefs->set_animate(animateCheck_->getValue());
      prefs->set_pulserate(0.1);
      prefs->set_pulsestipple(0xfff0);

      xaction.complete(&prefs);
    }

    // apply update:
    {
      simData::BeamUpdate* update = ds_->addBeamUpdate(beamId_, &xaction);
      update->set_time(t_);

      update->set_range(rangeSlider_->getValue());
      update->set_azimuth(azimuthSlider_->getValue() * simCore::DEG2RAD);
      update->set_elevation(elevSlider_->getValue() * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_->update(t_);

    // update labels.
    typeLabel_->setText(types_[typeIndex].second);
    modeLabel_->setText(modes_[modeIndex].second);
    colorLabel_->setText(colors_[colorIndex].second);

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_->getValue() ?
      (displayMask |  simVis::DISPLAY_MASK_BEAM) :
      (displayMask & ~simVis::DISPLAY_MASK_BEAM));

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
  top->addControl(new ui::LabelControl("Beams - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  ui::Grid* grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Type"));
  app.typeLabel_ = grid->setControl(c+1, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Draw Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 2, applyUI.get()));
  app.modeSlider_->setHorizFill(true, 250);
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Range"));
  app.rangeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 250.0, applyUI.get()));
  app.rangeLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Horiz. Size"));
  app.horizSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 400.0, 45.0, applyUI.get()));
  app.horizLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.horizSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Vert. Size"));
  app.vertSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 200.0, 45.0, applyUI.get()));
  app.vertLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.vertSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Azimuth"));
  app.azimuthSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-180.0, 180.0, 0.0, applyUI.get()));
  app.azimuthLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.azimuthSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Elevation"));
  app.elevSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-90, 90.0, 0.0, applyUI.get()));
  app.elevLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.elevSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Color"));
  app.colorSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors_.size()-1, 0, applyUI.get()));
  app.colorLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Cap Res."));
  app.capResSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1, 20, 15, applyUI.get()));
  app.capResLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.capResSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Cone Res."));
  app.coneResSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(4, 40, 30, applyUI.get()));
  app.coneResLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.coneResSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Use Offset"));
  app.useOffsetIconCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Shaded"));
  app.shadedCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Blended"));
  app.blendedCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Render Cone"));
  app.renderConeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Animate"));
  app.animateCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Global Beam Toggle"));
  app.globalToggle_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

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
    simCore::Vec3 pos(simCore::DEG2RAD*51.0, 0.0, 200.0);

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

simData::ObjectId addBeam(simData::DataStore& ds,
                          simData::ObjectId   hostId,
                          int                 argc,
                          char**              argv)
{
  // see if they user wants body-relative mode
  simData::BeamProperties_BeamType type = simExamples::hasArg("--br", argc, argv)?
    simData::BeamProperties_BeamType_BODY_RELATIVE :
    simData::BeamProperties_BeamType_ABSOLUTE_POSITION;

  simData::ObjectId beamId;

  // create the beam
  {
    simData::DataStore::Transaction xaction;
    simData::BeamProperties* props = ds.addBeam(&xaction);
    beamId = props->id();
    props->set_hostid(hostId);
    props->set_type(type);
    xaction.complete(&props);
  }

  // tick the clock
  ds.update(0);

  return beamId;
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
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

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
  app.beamId_ = addBeam(dataStore, app.hostId_, argc, argv);

  osg::observer_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(app.hostId_);
  app.view_->tetherCamera(platformModel.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

