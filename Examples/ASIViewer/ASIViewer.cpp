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
 * Very simple utility to read and display Platform tracks from an ASI file
 */
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/String/Format.h"
#include "simCore/Time/Clock.h"
#include "simCore/Time/ClockImpl.h"
#include "simCore/Time/String.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/Utils.h"
#include "simUtil/ExampleResources.h"

#include <osgEarth/StringUtils>
#include <osgEarthSymbology/Color>
#include <osgEarthUtil/Controls>
namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------
namespace
{
void deQuote(std::string &str)
{
  if (str.length() >= 2 && str.at(0) == '\"' && str[str.length() - 1] == '\"')
  {
    str = str.substr(1, str.length()-2);
  }
}

simData::BeamProperties::BeamType beamTypeFromString(const std::string &beamTypeString)
{
  if (beamTypeString == "BODY")
    return simData::BeamProperties::BODY_RELATIVE;
  if (beamTypeString == "TARGET")
    return simData::BeamProperties::TARGET;

  // default
  return simData::BeamProperties::ABSOLUTE_POSITION;
}

simData::GateProperties::GateType gateTypeFromString(const std::string &gateTypeString)
{
  if (gateTypeString == "BODY")
    return simData::GateProperties::BODY_RELATIVE;
  if (gateTypeString == "TARGET")
    return simData::GateProperties::TARGET;

  // default
  return simData::GateProperties::ABSOLUTE_POSITION;
}

unsigned colorFromString(const std::string &colorString)
{
  using namespace osgEarth::Symbology;

  if (colorString[0] == '0' && colorString[1] == 'x')
  {
    // 0xAABBGGRR
    return Color(colorString, Color::ABGR).as(Color::RGBA);
  }
  else
  {
    // look for a color name
    if (simCore::caseCompare("blue", colorString))
      return Color::Blue.as(Color::RGBA);
    if (simCore::caseCompare("red", colorString))
      return Color::Red.as(Color::RGBA);
    if (simCore::caseCompare("green", colorString))
      return Color::Green.as(Color::RGBA);
    if (simCore::caseCompare("white", colorString))
      return Color::White.as(Color::RGBA);
    if (simCore::caseCompare("yellow", colorString))
      return Color::Yellow.as(Color::RGBA);
    if (simCore::caseCompare("purple", colorString))
      return Color::Purple.as(Color::RGBA);
    if (simCore::caseCompare("magenta", colorString))
      return Color::Magenta.as(Color::RGBA);
    if (simCore::caseCompare("cyan", colorString))
      return Color::Cyan.as(Color::RGBA);
    if (simCore::caseCompare("black", colorString))
      return Color::Black.as(Color::RGBA);
    if (simCore::caseCompare("brown", colorString))
      return Color::Brown.as(Color::RGBA);
    if (simCore::caseCompare("orange", colorString))
      return Color::Orange.as(Color::RGBA);
    if (simCore::caseCompare("gray", colorString))
      return Color::Gray.as(Color::RGBA);
  }
  return Color::Red.as(Color::RGBA);
}

/// convert an ASI time string to seconds since reference year
double timeFromString(std::string t, int referenceYear)
{
  deQuote(t);
  if (t[0] == '-')
  {
    if (t[1] != '1')
      std::cerr << "ASI Parser: Static platforms should use -1, not other negative numbers." << std::endl;

    return -1;
  }

  simCore::TimeStamp ts;

  simCore::SecondsTimeFormatter secondsFormat;
  if (secondsFormat.canConvert(t) && secondsFormat.fromString(t, ts, referenceYear) == 0)
    return ts.secondsSinceRefYear(referenceYear);

  simCore::MinutesTimeFormatter minutesFormat;
  if (minutesFormat.canConvert(t) && minutesFormat.fromString(t, ts, referenceYear) == 0)
    return ts.secondsSinceRefYear(referenceYear);

  simCore::HoursTimeFormatter hoursFormat;
  if (hoursFormat.canConvert(t) && hoursFormat.fromString(t, ts, referenceYear) == 0)
    return ts.secondsSinceRefYear(referenceYear);

  simCore::OrdinalTimeFormatter ordFormat;
  if (ordFormat.canConvert(t) && ordFormat.fromString(t, ts, referenceYear) == 0)
    return ts.secondsSinceRefYear(referenceYear);

  simCore::MonthDayTimeFormatter mdyFormat;
  if (mdyFormat.canConvert(t) && mdyFormat.fromString(t, ts, referenceYear) == 0)
    return ts.secondsSinceRefYear(referenceYear);

  std::cerr << "ASI Parser: failed to parse time '" << t << '\'' << std::endl;
  return 1.0;
}
}

//----------------------------------------------------------------------------
struct AppData
{
  osg::ref_ptr<ui::HSliderControl> timeSlider_;
  osg::ref_ptr<ui::CheckBoxControl> playCheck_;
  osg::ref_ptr<ui::CheckBoxControl> overheadMode_;
  simData::DataStore *ds_;
  simVis::View* view_;
  double startTime_;
  double endTime_;
  double lastTime_;
  bool playing_;

  explicit AppData(simData::DataStore *ds, simVis::View* view)
  : timeSlider_(NULL),
    ds_(ds),
    view_(view),
    startTime_(0.0),
    endTime_(0.0),
    lastTime_(0.0),
    playing_(false)
  {
  }

  void apply()
  {
    lastTime_ = timeSlider_->getValue();
    ds_->update(lastTime_);
  }

  void applyToggles()
  {
    playing_ = playCheck_->getValue();
    view_->enableOverheadMode(overheadMode_->getValue());
  }

  void advance(double dt)
  {
    if (playing_)
    {
      const double t = lastTime_ + dt;
      ds_->update(t);
      lastTime_ = t;
      timeSlider_->setValue(lastTime_, false);
    }
  }
};

//----------------------------------------------------------------------------
/// Only handles most basic commands for platforms, beams, and gates
class SimpleAsiParser
{
public:
  explicit SimpleAsiParser(AppData &appData)
  : app_(appData),
    refYear_(simCore::currentYear()),
    degreeAngles_(false)
  {
  }

  void parse(const std::string &filename)
  {
    std::ifstream infile(filename.c_str());

    std::string line;
    while (std::getline(infile, line))
    {
      std::stringstream buf(line);
      handleAsiCommand_(buf);
    }

    for (std::map<unsigned, unsigned>::iterator i = dataCount_.begin(); i != dataCount_.end(); ++i)
    {
      SIM_NOTICE << "Platform " << i->first << ": " << i->second << " updates\n";
    }
  }

private:
  /// accumulate beam data, so it can all be set at creation time
  struct BeamData
  {
    unsigned hostId;
    unsigned beamId;
    simData::BeamProperties::BeamType beamType;
    double hw;
    double vw;

    BeamData(unsigned id = 0, unsigned host = 0)
    : hostId(host),
      beamId(id),
      beamType(simData::BeamProperties::ABSOLUTE_POSITION),
      hw(0),
      vw(0)
    {
    }
  };

  void createBeamIfNeeded_(unsigned id)
  {
    // check the pending map for the given id
    std::map<unsigned, BeamData>::iterator i = pendingBeams_.find(id);
    if (i == pendingBeams_.end())
      return; // beam has been created

    // create it!
    simData::DataStore::Transaction xaction;
    simData::BeamProperties *props = app_.ds_->addBeam(&xaction);
    idMap_[id] = props->id(); // stash data-store id

    const unsigned dsId = props->id();

    props->set_hostid(i->second.hostId);
    props->set_type(i->second.beamType);
    props->set_originalid(id);
    xaction.complete(&props);

    simData::BeamPrefs *prefs = app_.ds_->mutable_beamPrefs(dsId, &xaction);
    // beam widths are always degrees in ASI, and radians in data-store
    i->second.hw *= simCore::DEG2RAD;
    i->second.vw *= simCore::DEG2RAD;

    prefs->set_verticalwidth(i->second.vw);
    prefs->set_horizontalwidth(i->second.hw);
    xaction.complete(&prefs);

    // done!
    pendingBeams_.erase(i);
  }

  /// accumulate gate data, so it can all be set at creation time
  struct GateData
  {
    unsigned hostId;
    unsigned gateId;
    simData::GateProperties::GateType gateType;

    GateData(unsigned id = 0, unsigned host = 0)
    : hostId(host),
      gateId(id),
      gateType(simData::GateProperties::ABSOLUTE_POSITION)
    {
    }
  };

  void createGateIfNeeded_(unsigned id)
  {
    // check the pending map for the given id
    std::map<unsigned, GateData>::iterator i = pendingGates_.find(id);
    if (i == pendingGates_.end())
      return; // gate has been created

    // create it!
    simData::DataStore::Transaction xaction;
    simData::GateProperties *props = app_.ds_->addGate(&xaction);
    idMap_[id] = props->id();

    props->set_hostid(i->second.hostId);
    props->set_type(i->second.gateType);
    xaction.complete(&props);

    // done!
    pendingGates_.erase(i);
  }

  /// take action for the given command (one line in the file)
  void handleAsiCommand_(std::stringstream &buf)
  {
    simData::DataStore::Transaction xaction;

    std::string token;
    buf >> token;

    if (token == "PlatformID")
    {
      unsigned id;
      buf >> id;

      simData::PlatformProperties *props = app_.ds_->addPlatform(&xaction);
      idMap_[id] = props->id();
      xaction.complete(&props);
    }
    else if (token == "PlatformIcon")
    {
      unsigned id;
      std::string icon;
      buf >> id >> icon;
      deQuote(icon);

      simData::PlatformPrefs *prefs = app_.ds_->mutable_platformPrefs(idMap_[id], &xaction);
      prefs->set_icon(icon);
      prefs->mutable_commonprefs()->set_draw(true);
      prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_BRIDGE);
      //prefs->set_dynamicscale(true);
      //prefs->set_surfaceclamping(true);
      xaction.complete(&prefs);
    }
    else if (token == "PlatformName")
    {
      unsigned id;
      std::string name;
      buf >> id >> name;
      deQuote(name);

      simData::PlatformPrefs *prefs = app_.ds_->mutable_platformPrefs(idMap_[id], &xaction);
      prefs->mutable_commonprefs()->set_name(name);
      prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
      xaction.complete(&prefs);
    }
    else if (token == "PlatformData")
    {
      unsigned id;
      std::string timeString;
      double lat, lon, alt, yaw, pitch, roll, vx, vy, vz;
      buf >> id >> timeString >> lat >> lon >> alt >> yaw >> pitch >> roll >> vx >> vy >> vz;

      const double t = timeFromString(timeString, refYear_);

      simCore::Coordinate coord;
      coord.setCoordinateSystem(simCore::COORD_SYS_LLA);
      coord.setPositionLLA(simCore::DEG2RAD*lat, simCore::DEG2RAD*lon, alt);
      coord.setOrientationEuler(simCore::DEG2RAD*yaw, simCore::DEG2RAD*pitch, simCore::DEG2RAD*roll);
      coord.setVelocity(vx, vy, vz);

      simCore::Coordinate ecef;
      simCore::CoordinateConverter conv;
      conv.convertGeodeticToEcef(coord, ecef);

      simData::PlatformUpdate *update = app_.ds_->addPlatformUpdate(idMap_[id], &xaction);

      update->set_time(t);
      update->setPosition(ecef.position());
      update->setOrientation(ecef.orientation());
      update->setVelocity(ecef.velocity());

      if (t != -1 && t < app_.startTime_)
        app_.startTime_ = t;
      if (t > app_.endTime_)
        app_.endTime_ = t;

      xaction.complete(&update);

      dataCount_[id]++;
    }
    else if (token == "BeamID")
    {
      unsigned host, id;
      buf >> host >> id;

      // convert ASI host id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(host);
      if (i == idMap_.end())
      {
        std::cerr << "ASI Parser: Attempt to create beam " << id << " before host platform " << host << std::endl;
        return;
      }

      pendingBeams_.insert(std::make_pair(id, BeamData(id, i->second)));
    }
    else if (token == "BeamType")
    {
      unsigned id;
      std::string beamTypeString;
      buf >> id >> beamTypeString;

      deQuote(beamTypeString);
      pendingBeams_[id].beamType = beamTypeFromString(beamTypeString);
    }
    else if (token == "VertBW")
    {
      unsigned id;
      double bw;
      buf >> id >> bw;

      pendingBeams_[id].vw = bw;
    }
    else if (token == "HorzBW")
    {
      unsigned id;
      double bw;
      buf >> id >> bw;

      pendingBeams_[id].hw = bw;
    }
    else if (token == "BeamOnOffCmd")
    {
      unsigned id;
      std::string timeString;
      int onOff;
      buf >> id >> timeString >> onOff;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::BeamCommand *cmd = app_.ds_->addBeamCommand(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();
      commonPrefs->set_draw(onOff == 1);
      xaction.complete(&cmd);
    }
    else if (token == "BeamColorCmd")
    {
      unsigned id;
      std::string timeString;
      std::string colorString;
      buf >> id >> timeString >> colorString;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::BeamCommand *cmd = app_.ds_->addBeamCommand(dsId, &xaction);
      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      commonPrefs->set_color(colorFromString(colorString));
      xaction.complete(&cmd);
    }
    else if (token == "BeamDataRAE")
    {
      unsigned id;
      std::string timeString;
      double az, el, range;
      buf >> id >> timeString >> az >> el >> range;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::BeamUpdate *up = app_.ds_->addBeamUpdate(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      up->set_time(t);

      if (degreeAngles_)
      {
        // convert input degrees to expected radians
        az *= simCore::DEG2RAD;
        el *= simCore::DEG2RAD;
      }
      //else data is already in radians

      up->set_azimuth(az);
      up->set_elevation(el);
      up->set_range(range);

      xaction.complete(&up);
    }
    else if (token == "BeamTargetIDCmd")
    {
      unsigned id;
      std::string timeString;
      unsigned targetId;
      buf >> id >> timeString >> targetId;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::BeamCommand *cmd = app_.ds_->addBeamCommand(dsId, &xaction);
      simData::BeamPrefs *prefs = cmd->mutable_updateprefs();

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      // convert target id to data-store version
      i = idMap_.find(targetId);
      if (i == idMap_.end())
        return;

      prefs->set_targetid(i->second);

      xaction.complete(&cmd);
    }
    else if (token == "GateID")
    {
      unsigned host, id;
      buf >> host >> id;

      // convert ASI host id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(host);
      if (i == idMap_.end())
      {
        std::cerr << "ASI Parser: Attempt to create gate " << id << " before host beam " << host << std::endl;
        return;
      }

      pendingGates_.insert(std::make_pair(id, GateData(id, i->second)));
    }
    else if (token == "GateType")
    {
      unsigned id;
      std::string gateTypeString;
      buf >> id >> gateTypeString;
      deQuote(gateTypeString);

      pendingGates_[id].gateType = gateTypeFromString(gateTypeString);
    }
    else if (token == "GateOnOffCmd")
    {
      unsigned id;
      std::string timeString;
      int onOff;
      buf >> id >> timeString >> onOff;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::GateCommand *cmd = app_.ds_->addGateCommand(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();
      commonPrefs->set_draw(onOff == 1);
      xaction.complete(&cmd);
    }
    else if (token == "GateColorCmd")
    {
      unsigned id;
      std::string timeString;
      std::string colorString;
      buf >> id >> timeString >> colorString;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::GateCommand *cmd = app_.ds_->addGateCommand(dsId, &xaction);
      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      commonPrefs->set_color(colorFromString(colorString));
      xaction.complete(&cmd);
    }
    else if (token == "GateDataRAE")
    {
      unsigned id;
      std::string timeString;
      double az, el, w, h, start, end, center;
      buf >> id >> timeString >> az >> el >> w >> h >> start >> end >> center;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<unsigned, unsigned>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const unsigned dsId = i->second;
      simData::GateUpdate *up = app_.ds_->addGateUpdate(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      up->set_time(t);

      if (degreeAngles_)
      {
        // convert input degrees to expected radians
        az *= simCore::DEG2RAD;
        el *= simCore::DEG2RAD;
        w *= simCore::DEG2RAD;
        h *= simCore::DEG2RAD;
      }
      //else data is already in radians

      up->set_azimuth(az);
      up->set_elevation(el);
      up->set_width(w);
      up->set_height(h);
      up->set_minrange(start);
      up->set_maxrange(end);
      up->set_centroid(center);

      xaction.complete(&up);
    }
    else if (token == "DegreeAngles")
    {
      int val;
      buf >> val;
      degreeAngles_ = (val == 1);
    }
    else if (token == "ReferenceYear")
    {
      int year;
      buf >> year;
      if (year < 1970)
        std::cerr << "ASI Parser: Reference year before 1970 is not reliable." << std::endl;
      refYear_ = year;
    }
  }

  AppData &app_;

  std::map<unsigned, BeamData> pendingBeams_;
  std::map<unsigned, GateData> pendingGates_;

  int refYear_; /// scenario reference year
  bool degreeAngles_; /// true: data units are in degrees, false: radians

  std::map<unsigned, unsigned> idMap_; /// map from ASI id to data-store id
  std::map<unsigned, unsigned> dataCount_; /// number of data points (indexed by ASI id)
};

//----------------------------------------------------------------------------
struct ApplyUI : public ui::ControlEventHandler
{
  explicit ApplyUI(AppData* app): app_(app) {}
  AppData* app_;
  void onValueChanged(ui::Control* c, bool value) { app_->applyToggles(); }
  void onValueChanged(ui::Control* c, float value) { app_->apply(); }
  void onValueChanged(ui::Control* c, double value) { onValueChanged(c, (float)value); }
};

ui::Control* createUI(AppData& app)
{
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(&app);

  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("ASI Simple Viewer", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  osg::ref_ptr<ui::Grid> grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  // note that the slider control uses float, so seconds since 1970 will cause it to lose precision
  grid->setControl(c, r, new ui::LabelControl("Time:"));
  app.timeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(app.startTime_, app.endTime_, app.startTime_, applyUI.get()));
  app.timeSlider_->setHorizFill(true, 700);

  ++r;
  grid->setControl(c, r, new ui::LabelControl("Playing:"));
  app.playCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  ++r;
  grid->setControl(c, r, new ui::LabelControl("Overhead:"));
  app.overheadMode_ = grid->setControl(c + 1, r, new ui::CheckBoxControl(false, applyUI.get()));

  return top;
}

//----------------------------------------------------------------------------
void readASI(osg::ArgumentParser& args, AppData& app)
{
  if (args.argc() < 2)
    return;

  std::string asiFileName = args.argv()[1];
  if (asiFileName.empty())
    return;

  app.startTime_ = DBL_MAX, app.endTime_ = -DBL_MAX;

  SimpleAsiParser p(app);
  p.parse(asiFileName);
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  // fire up the viewer.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // read the ASI data into the datastore.
  simData::MemoryDataStore dataStore;
  AppData app(&dataStore, viewer->getMainView());
  osg::ArgumentParser args(&argc, argv);
  readASI(args, app);
  viewer->getSceneManager()->getScenario()->bind(&dataStore);

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  double last_t = 0.0;
  while (!viewer->getViewer()->done())
  {
    double t = viewer->getViewer()->getFrameStamp()->getReferenceTime();
    double delta = t - last_t;
    app.advance(delta);
    last_t = t;

    viewer->frame();
  }
}

