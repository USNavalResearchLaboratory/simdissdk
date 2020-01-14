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
#include "simCore/String/Tokenizer.h"
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
#include "simVis/Registry.h"
#include "simVis/Projector.h"
#include "simVis/Types.h"
#include "simUtil/ExampleResources.h"

#include "osg/ImageStream"
#include "osgDB/ReadFile"
#include "osgEarth/Controls"
#include "osgEarth/StringUtils"
#include "osgDB/ReadFile"

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

uint32_t colorFromString(const std::string &colorString)
{
  if (colorString[0] == '0' && colorString[1] == 'x')
  {
    // 0xAABBGGRR
    return simVis::Color(colorString, simVis::Color::ABGR).as(simVis::Color::RGBA);
  }
  else
  {
    // look for a color name
    if (simCore::caseCompare("blue", colorString))
      return simVis::Color::Blue.as(simVis::Color::RGBA);
    if (simCore::caseCompare("red", colorString))
      return simVis::Color::Red.as(simVis::Color::RGBA);
    if (simCore::caseCompare("green", colorString))
      return simVis::Color::Green.as(simVis::Color::RGBA);
    if (simCore::caseCompare("white", colorString))
      return simVis::Color::White.as(simVis::Color::RGBA);
    if (simCore::caseCompare("yellow", colorString))
      return simVis::Color::Yellow.as(simVis::Color::RGBA);
    if (simCore::caseCompare("purple", colorString))
      return simVis::Color::Purple.as(simVis::Color::RGBA);
    if (simCore::caseCompare("magenta", colorString))
      return simVis::Color::Magenta.as(simVis::Color::RGBA);
    if (simCore::caseCompare("cyan", colorString))
      return simVis::Color::Cyan.as(simVis::Color::RGBA);
    if (simCore::caseCompare("black", colorString))
      return simVis::Color::Black.as(simVis::Color::RGBA);
    if (simCore::caseCompare("brown", colorString))
      return simVis::Color::Brown.as(simVis::Color::RGBA);
    if (simCore::caseCompare("orange", colorString))
      return simVis::Color::Orange.as(simVis::Color::RGBA);
    if (simCore::caseCompare("gray", colorString))
      return simVis::Color::Gray.as(simVis::Color::RGBA);
  }
  return simVis::Color::Red.as(simVis::Color::RGBA);
}

/// convert an ASI time string to seconds since reference year
double timeFromString(std::string t, int referenceYear)
{
  deQuote(t);
  if (t.size() > 1 && t[0] == '-')
  {
    if (t[1] != '1')
      std::cerr << "ASI Parser: Static platforms should use -1, not other negative numbers." << std::endl;

    return -1;
  }

  simCore::TimeStamp ts;
  simCore::TimeFormatterRegistry timeFormats;
  if (timeFormats.fromString(t, ts, referenceYear) == 0)
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
  osg::ref_ptr<ui::LabelControl> timeReadout_;
  simData::DataStore *ds_;
  simVis::View* view_;
  simCore::Clock* clock_;
  double startTime_;
  double endTime_;
  double lastTime_;
  bool playing_;
  std::vector<uint64_t> platformIDs_;
  int tetherIndex_;
  osgEarth::DateTime refDateTime_;

  explicit AppData(simData::DataStore *ds, simVis::View* view)
  : timeSlider_(NULL),
    ds_(ds),
    view_(view),
    startTime_(0.0),
    endTime_(0.0),
    lastTime_(0.0),
    playing_(true),
    tetherIndex_(-1)
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

  void tetherNext()
  {
    osg::Node* node = view_->getCameraTether();
    if (node)
      node->setNodeMask(~0);

    tetherIndex_++;
    if (tetherIndex_ >= static_cast<int>(platformIDs_.size()))
      tetherIndex_ = -1;

    if (tetherIndex_ < 0)
    {
      view_->tetherCamera(0L);
    }
    else
    {
      simVis::EntityNode* node = view_->getSceneManager()->getScenario()->find(platformIDs_[tetherIndex_]);
      view_->tetherCamera(node);
      dynamic_cast<osgEarth::Util::EarthManipulator*>(view_->getCameraManipulator())
        ->getSettings()->setTetherMode(osgEarth::Util::EarthManipulator::TETHER_CENTER_AND_ROTATION);
    }
  }

  void advance(double dt)
  {
    if (playing_)
    {
      const double t = lastTime_ + dt;
      ds_->update(t);
      lastTime_ = t;
      timeSlider_->setValue(lastTime_, false);

      osgEarth::DateTime now = refDateTime_ + (t/3600.0);
      timeReadout_->setText(now.asRFC1123());
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
    while (simCore::getStrippedLine(infile, line))
    {
      handleAsiCommand_(line);
    }

    for (std::map<uint64_t, unsigned int>::const_iterator i = dataCount_.begin(); i != dataCount_.end(); ++i)
    {
      SIM_NOTICE << "Platform " << i->first << ": " << i->second << " updates\n";
    }
  }

private:
  /// accumulate beam data, so it can all be set at creation time
  struct BeamData
  {
    uint64_t hostId;
    uint64_t beamId;
    simData::BeamProperties::BeamType beamType;
    double hw;
    double vw;

    BeamData(uint64_t id = 0, uint64_t host = 0)
    : hostId(host),
      beamId(id),
      beamType(simData::BeamProperties::ABSOLUTE_POSITION),
      hw(0),
      vw(0)
    {
    }
  };

  void createBeamIfNeeded_(uint64_t id)
  {
    // check the pending map for the given id
    std::map<uint64_t, BeamData>::iterator i = pendingBeams_.find(id);
    if (i == pendingBeams_.end())
      return; // beam has been created

    // create it!
    simData::DataStore::Transaction xaction;
    simData::BeamProperties *props = app_.ds_->addBeam(&xaction);
    idMap_[id] = props->id(); // stash data-store id

    const uint64_t dsId = props->id();

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
    uint64_t hostId;
    uint64_t gateId;
    simData::GateProperties::GateType gateType;

    GateData(uint64_t id = 0, uint64_t host = 0)
    : hostId(host),
      gateId(id),
      gateType(simData::GateProperties::ABSOLUTE_POSITION)
    {
    }
  };

  void createGateIfNeeded_(uint64_t id)
  {
    // check the pending map for the given id
    std::map<uint64_t, GateData>::iterator i = pendingGates_.find(id);
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

  /**
   * Simple wrapper around a vector<string> that provides stringstream-like extraction operators.
   * This is better than using a raw stringstream on a line that needs tokenization, because stringstream
   * does not handle tokenization correctly with quotes.
   */
  class VecExtraction
  {
  public:
    /** Initialize with a vector of strings */
    explicit VecExtraction(const std::vector<std::string>& vec)
      : vec_(vec),
        index_(0)
    {
    }

    /** Extract a type, presuming that stringstream does the right thing with the token. */
    template<typename T>
    VecExtraction& operator>>(T& out)
    {
      std::string str;
      operator>>(str);
      std::stringstream ss(str);
      ss >> out;
      return *this;
    }

    /** Extract a single string -- specialized to return the whole string */
    VecExtraction& operator>>(std::string& out)
    {
      if (index_ < vec_.size())
        out = vec_[index_++];
      return *this;
    }

  private:
    std::vector<std::string> vec_;
    size_t index_;
  };

  /// take action for the given command (one line in the file)
  void handleAsiCommand_(const std::string &line)
  {
    simData::DataStore::Transaction xaction;
    std::vector<std::string> vec;
    simCore::quoteCommentTokenizer(line, vec);
    VecExtraction buf(vec);

    std::string token;
    buf >> token;

    if (token == "PlatformID")
    {
      uint64_t id;
      buf >> id;

      simData::PlatformProperties *props = app_.ds_->addPlatform(&xaction);
      idMap_[id] = props->id();
      xaction.complete(&props);
      app_.platformIDs_.push_back(id);
    }
    else if (token == "PlatformIcon")
    {
      uint64_t id;
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
      uint64_t id;
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
      uint64_t id;
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
      uint64_t host, id;
      buf >> host >> id;

      // convert ASI host id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(host);
      if (i == idMap_.end())
      {
        std::cerr << "ASI Parser: Attempt to create beam " << id << " before host platform " << host << std::endl;
        return;
      }

      pendingBeams_.insert(std::make_pair(id, BeamData(id, i->second)));
    }
    else if (token == "BeamType")
    {
      uint64_t id;
      std::string beamTypeString;
      buf >> id >> beamTypeString;

      deQuote(beamTypeString);
      pendingBeams_[id].beamType = beamTypeFromString(beamTypeString);
    }
    else if (token == "VertBW")
    {
      uint64_t id;
      double bw;
      buf >> id >> bw;

      pendingBeams_[id].vw = bw;
    }
    else if (token == "HorzBW")
    {
      uint64_t id;
      double bw;
      buf >> id >> bw;

      pendingBeams_[id].hw = bw;
    }
    else if (token == "BeamOnOffCmd")
    {
      uint64_t id;
      std::string timeString;
      int onOff;
      buf >> id >> timeString >> onOff;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
      simData::BeamCommand *cmd = app_.ds_->addBeamCommand(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();
      commonPrefs->set_draw(onOff == 1);
      xaction.complete(&cmd);
    }
    else if (token == "BeamColorCmd")
    {
      uint64_t id;
      std::string timeString;
      std::string colorString;
      buf >> id >> timeString >> colorString;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
      simData::BeamCommand *cmd = app_.ds_->addBeamCommand(dsId, &xaction);
      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      commonPrefs->set_color(colorFromString(colorString));
      xaction.complete(&cmd);
    }
    else if (token == "BeamDataRAE")
    {
      uint64_t id;
      std::string timeString;
      double az, el, range;
      buf >> id >> timeString >> az >> el >> range;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
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
      uint64_t id;
      std::string timeString;
      uint64_t targetId;
      buf >> id >> timeString >> targetId;

      createBeamIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
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
      uint64_t host, id;
      buf >> host >> id;

      // convert ASI host id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(host);
      if (i == idMap_.end())
      {
        std::cerr << "ASI Parser: Attempt to create gate " << id << " before host beam " << host << std::endl;
        return;
      }

      pendingGates_.insert(std::make_pair(id, GateData(id, i->second)));
    }
    else if (token == "GateType")
    {
      uint64_t id;
      std::string gateTypeString;
      buf >> id >> gateTypeString;
      deQuote(gateTypeString);

      pendingGates_[id].gateType = gateTypeFromString(gateTypeString);
    }
    else if (token == "GateOnOffCmd")
    {
      uint64_t id;
      std::string timeString;
      int onOff;
      buf >> id >> timeString >> onOff;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
      simData::GateCommand *cmd = app_.ds_->addGateCommand(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();
      commonPrefs->set_draw(onOff == 1);
      xaction.complete(&cmd);
    }
    else if (token == "GateColorCmd")
    {
      uint64_t id;
      std::string timeString;
      std::string colorString;
      buf >> id >> timeString >> colorString;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
      simData::GateCommand *cmd = app_.ds_->addGateCommand(dsId, &xaction);
      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      commonPrefs->set_color(colorFromString(colorString));
      xaction.complete(&cmd);
    }
    else if (token == "GateDataRAE")
    {
      uint64_t id;
      std::string timeString;
      double az, el, w, h, start, end, center;
      buf >> id >> timeString >> az >> el >> w >> h >> start >> end >> center;

      createGateIfNeeded_(id);

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
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
      app_.refDateTime_ = osgEarth::DateTime(refYear_, 1, 1, 0.0);
    }
    else if (token == "Projector")
    {
      uint64_t host, id;
      buf >> host >> id;

      // convert ASI host id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(host);
      if (i == idMap_.end())
      {
        std::cerr << "ASI Parser: Attempt to create projector " << id << " before host platform " << host << std::endl;
        return;
      }

      simData::DataStore::Transaction xaction;
      simData::ProjectorProperties* props = app_.ds_->addProjector(&xaction);
      idMap_[id] = props->id(); // stash data-store id
      props->set_hostid(host);
      xaction.complete(&props);
    }
    else if (token == "ProjectorRasterFile")
    {
      uint64_t id;
      std::string fileString;
      buf >> id;
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      buf >> fileString;
      deQuote(fileString);

      const uint64_t dsId = i->second;
      simData::ProjectorPrefs* prefs = app_.ds_->mutable_projectorPrefs(dsId, &xaction);
      prefs->set_rasterfile(fileString);
      xaction.complete(&prefs);
    }
    else if (token == "ProjectorInterpolateFOV")
    {
      uint64_t id;
      buf >> id;
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      uint64_t value;
      buf >> value;

      const uint64_t dsId = i->second;
      simData::ProjectorPrefs* prefs = app_.ds_->mutable_projectorPrefs(dsId, &xaction);
      prefs->set_interpolateprojectorfov(value==1? true: false);
      xaction.complete(&prefs);
    }
    else if (token == "ProjectorOn")
    {
      uint64_t id;
      std::string timeString;
      int onOff;
      buf >> id >> timeString >> onOff;

      // convert ASI id to data-store id
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      const uint64_t dsId = i->second;
      simData::ProjectorCommand* cmd = app_.ds_->addProjectorCommand(dsId, &xaction);

      const double t = timeFromString(timeString, refYear_);
      cmd->set_time(t);

      simData::CommonPrefs *commonPrefs = cmd->mutable_updateprefs()->mutable_commonprefs();
      commonPrefs->set_draw(onOff == 1);
      xaction.complete(&cmd);
    }
    else if (token == "ProjectorFOV")
    {
      uint64_t id;
      buf >> id;
      std::map<uint64_t, uint64_t>::const_iterator i = idMap_.find(id);
      if (i == idMap_.end())
        return;

      std::string timeString;
      double value;
      buf >> timeString >> value;
      const double t = timeFromString(timeString, refYear_);

      const uint64_t dsId = i->second;
      simData::ProjectorUpdate* update = app_.ds_->addProjectorUpdate(dsId, &xaction);
      update->set_time(t);
      update->set_fov(simCore::DEG2RAD * value);
      xaction.complete(&update);
    }
  }

  AppData &app_;

  std::map<uint64_t, BeamData> pendingBeams_;
  std::map<uint64_t, GateData> pendingGates_;

  int refYear_; /// scenario reference year
  bool degreeAngles_; /// true: data units are in degrees, false: radians

  std::map<uint64_t, uint64_t> idMap_; /// map from ASI id to data-store id
  std::map<uint64_t, unsigned int> dataCount_; /// number of data points (indexed by ASI id)
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

struct TetherNext : public ui::ControlEventHandler
{
    explicit TetherNext(AppData* app) : app_(app) {}
    AppData* app_;
    void onClick(ui::Control* c) { app_->tetherNext(); }
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
  app.timeReadout_ = grid->setControl(c+1, r, new ui::LabelControl());

  ++r;
  grid->setControl(c, r, new ui::LabelControl("Playing:"));
  app.playCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(app.playing_, applyUI.get()));

  ++r;
  grid->setControl(c, r, new ui::LabelControl("Overhead:"));
  app.overheadMode_ = grid->setControl(c + 1, r, new ui::CheckBoxControl(false, applyUI.get()));

  ++r;
  grid->setControl(c, r, new ui::ButtonControl("Tether Next", new TetherNext(&app)));

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

  osg::ArgumentParser args(&argc, argv);
  std::string earthFile;
  osg::ref_ptr<osgEarth::MapNode> mapNode;
  if (args.read("--earthfile", earthFile))
  {
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(earthFile);
    mapNode = osgEarth::MapNode::get(node.get());
  }

  // fire up the viewer.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  if (mapNode.valid())
    viewer->setMapNode(mapNode.get());
  else
    viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // read the ASI data into the datastore.
  simData::MemoryDataStore dataStore;
  AppData app(&dataStore, viewer->getMainView());
  readASI(args, app);
  viewer->getSceneManager()->getScenario()->bind(&dataStore);

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  viewer->getSceneManager()->setSkyNode(osgEarth::SkyNode::create());

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
