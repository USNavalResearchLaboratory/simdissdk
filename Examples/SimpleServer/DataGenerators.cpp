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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simVis/AnimatedLine.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simUtil/ExampleResources.h"
#include "DataGenerators.h"

/**
 * When USE_COMMANDS is defined, the data generators will prefer to use
 * simData::DataStore commands to change prefs based on a time, as opposed
 * to simply setting the pref value directly.  Commands are stored, time-
 * stamped in the datastore and are data limited.  They are essentially
 * time-stamped prefs.  Prefs are also stored in the DataStore, but only
 * the most recent value is stored.
 */
#define USE_COMMANDS

namespace SimpleServer {

DataGenerator::DataGenerator(double generateInterval)
  : lastTime_(0.0),
    generateInterval_(generateInterval)
{
}

void DataGenerator::idle(double scenarioTime)
{
  if (scenarioTime >= lastTime_ + generateInterval_)
  {
    lastTime_ = scenarioTime;
    generate_(scenarioTime);
  }
}

///////////////////////////////////////////////////////////

CirclingPlatform::CirclingPlatform(simData::DataStore& dataStore)
  : dataStore_(dataStore),
    id_(0),
    rangeMeters_(100.0)
{
}

void CirclingPlatform::setCenterLla(const simCore::Vec3& lla)
{
  cc_.setReferenceOrigin(lla);
}

void CirclingPlatform::setXyzOffset(const simCore::Vec3& xyzOffset)
{
  xyzOffset_ = xyzOffset;
}

void CirclingPlatform::setRange(double rangeMeters)
{
  rangeMeters_ = rangeMeters;
}

void CirclingPlatform::create(const std::string& name)
{
  {
    simData::DataStore::Transaction txn;
    simData::PlatformProperties* props = dataStore_.addPlatform(&txn);
    id_ = props->id();
    props->set_source("CirclingPlatform");
    txn.complete(&props);
  }
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(id_, &txn);
    prefs->mutable_commonprefs()->set_name(name);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    txn.complete(&prefs);
  }
}

void CirclingPlatform::generate_(double atTime)
{
  if (!id_)
    return;
  simCore::Coordinate xEast;
  xEast.setCoordinateSystem(simCore::COORD_SYS_XEAST);
  simCore::Vec3 position(rangeMeters_ * sin(atTime * 0.01), rangeMeters_ * cos(atTime * 0.01), 0.0);
  for (size_t k = 0; k < 3; ++k)
    position[k] += xyzOffset_[k];
  xEast.setPosition(position);
  xEast.setOrientation(simCore::Vec3(simCore::angFix2PI(M_PI_2 + atTime * 0.1), 0.0, 0.0));
  // Calculate the position in ECEF
  simCore::Coordinate ecef;
  cc_.convert(xEast, ecef, simCore::COORD_SYS_ECEF);

  // Create the transaction and add the data
  simData::DataStore::Transaction txn;
  simData::PlatformUpdate* point = dataStore_.addPlatformUpdate(id_, &txn);
  point->set_time(atTime);
  point->setPosition(ecef.position());
  point->setOrientation(ecef.orientation());
  txn.complete(&point);
}

simData::ObjectId CirclingPlatform::id() const
{
  return id_;
}

///////////////////////////////////////////////////////////

RotatingBeam::RotatingBeam(simData::DataStore& dataStore)
  : dataStore_(dataStore),
    id_(0),
    rangeMeters_(300.0),
    elevationRads_(15.0 * simCore::DEG2RAD)
{
}

void RotatingBeam::setRange(double rangeMeters)
{
  rangeMeters_ = rangeMeters;
}

void RotatingBeam::setElevation(double elevation)
{
  elevationRads_ = elevation;
}

void RotatingBeam::create(simData::ObjectId hostPlatform, const std::string& name)
{
  {
    simData::DataStore::Transaction txn;
    simData::BeamProperties* props = dataStore_.addBeam(&txn);
    id_ = props->id();
    props->set_hostid(hostPlatform);
    props->set_type(simData::BeamProperties_BeamType_ABSOLUTE_POSITION);
    props->set_source("RotatingBeam");
    txn.complete(&props);
  }
  {
    simData::DataStore::Transaction txn;
    simData::BeamPrefs* prefs = dataStore_.mutable_beamPrefs(id_, &txn);
    prefs->mutable_commonprefs()->set_name(name);
    prefs->mutable_commonprefs()->set_color(0x00ff0080); // green
    prefs->mutable_commonprefs()->set_datadraw(true);
    prefs->mutable_commonprefs()->set_draw(true);
    prefs->set_verticalwidth(3 * simCore::DEG2RAD);
    prefs->set_horizontalwidth(3 * simCore::DEG2RAD);
    prefs->set_useoffseticon(true);
    txn.complete(&prefs);
  }
}

void RotatingBeam::generate_(double atTime)
{
  if (!id_)
    return;
  // Create the transaction and add the data
  simData::DataStore::Transaction txn;
  simData::BeamUpdate* point = dataStore_.addBeamUpdate(id_, &txn);
  point->set_time(atTime);
  point->set_range(rangeMeters_);
  point->set_elevation(elevationRads_);
  point->set_azimuth(simCore::angFix2PI(atTime * simCore::DEG2RAD));
  txn.complete(&point);
}

simData::ObjectId RotatingBeam::id() const
{
  return id_;
}

///////////////////////////////////////////////////////////

CyclingTargetBeam::CyclingTargetBeam(simData::DataStore& dataStore, double generateInterval)
  : DataGenerator(generateInterval),
    dataStore_(dataStore),
    id_(0),
    currentTargetIndex_(0)
{
}

void CyclingTargetBeam::addTarget(simData::ObjectId target)
{
  targets_.push_back(target);
}

void CyclingTargetBeam::create(simData::ObjectId hostPlatform, const std::string& name)
{
  {
    simData::DataStore::Transaction txn;
    simData::BeamProperties* props = dataStore_.addBeam(&txn);
    id_ = props->id();
    props->set_hostid(hostPlatform);
    props->set_type(simData::BeamProperties_BeamType_TARGET);
    props->set_source("CyclingTargetBeam");
    txn.complete(&props);
  }
  {
    simData::DataStore::Transaction txn;
    simData::BeamPrefs* prefs = dataStore_.mutable_beamPrefs(id_, &txn);
    prefs->mutable_commonprefs()->set_name(name);
    prefs->mutable_commonprefs()->set_color(0x00ff0080); // green
    prefs->mutable_commonprefs()->set_datadraw(true);
    prefs->mutable_commonprefs()->set_draw(true);
    prefs->set_verticalwidth(3 * simCore::DEG2RAD);
    prefs->set_horizontalwidth(3 * simCore::DEG2RAD);
    prefs->set_useoffseticon(true);
    txn.complete(&prefs);
  }
}

void CyclingTargetBeam::generate_(double atTime)
{
  if (targets_.empty() || id_  == 0)
    return;

  // Cycle targets
  currentTargetIndex_++;
  if (currentTargetIndex_ >= targets_.size())
    currentTargetIndex_ = 0;
  simData::ObjectId targetId = targets_[currentTargetIndex_];

  // Create the transaction and add the data
  simData::DataStore::Transaction txn;
#ifndef USE_COMMANDS
  simData::BeamPrefs* prefs = dataStore_.mutable_beamPrefs(id_, &txn);
  prefs->set_targetid(targetId);
  txn.complete(&prefs);
#else
  simData::BeamCommand* cmd = dataStore_.addBeamCommand(id_, &txn);
  cmd->set_time(atTime);
  cmd->mutable_updateprefs()->set_targetid(targetId);
  txn.complete(&cmd);
#endif
}

simData::ObjectId CyclingTargetBeam::id() const
{
  return id_;
}

///////////////////////////////////////////////////////////

RotatingGate::RotatingGate(simData::DataStore& dataStore)
  : dataStore_(dataStore),
    id_(0),
    elevationRads_(15.0 * simCore::DEG2RAD),
    widthRads_(3.0 * simCore::DEG2RAD),
    heightRads_(3.0 * simCore::DEG2RAD),
    minRangeMeters_(280),
    maxRangeMeters_(290)
{
}

void RotatingGate::setParameters(double elevRad, double widthRad, double heightRad, double minRng, double maxRange)
{
  elevationRads_ = elevRad;
  widthRads_ = widthRad;
  heightRads_ = heightRad;
  minRangeMeters_ = minRng;
  maxRangeMeters_ = maxRange;
}

void RotatingGate::create(simData::ObjectId hostBeam, const std::string& name)
{
  {
    simData::DataStore::Transaction txn;
    simData::GateProperties* props = dataStore_.addGate(&txn);
    id_ = props->id();
    props->set_hostid(hostBeam);
    props->set_type(simData::GateProperties_GateType_ABSOLUTE_POSITION);
    props->set_source("RotatingGate");
    txn.complete(&props);
  }
  {
    simData::DataStore::Transaction txn;
    simData::GatePrefs* prefs = dataStore_.mutable_gatePrefs(id_, &txn);
    prefs->mutable_commonprefs()->set_name(name);
    prefs->mutable_commonprefs()->set_color(0x00ff0080); // green
    prefs->mutable_commonprefs()->set_datadraw(true);
    prefs->mutable_commonprefs()->set_draw(true);
    txn.complete(&prefs);
  }
}

void RotatingGate::generate_(double atTime)
{
  if (!id_)
    return;
  // Create the transaction and add the data
  simData::DataStore::Transaction txn;
  simData::GateUpdate* point = dataStore_.addGateUpdate(id_, &txn);
  point->set_time(atTime);
  point->set_azimuth(simCore::angFix2PI(atTime * simCore::DEG2RAD));
  point->set_elevation(elevationRads_);
  point->set_width(widthRads_);
  point->set_height(heightRads_);
  point->set_minrange(minRangeMeters_);
  point->set_centroid(0.5 * (minRangeMeters_ + maxRangeMeters_));
  point->set_maxrange(maxRangeMeters_);
  txn.complete(&point);
}

simData::ObjectId RotatingGate::id() const
{
  return id_;
}

///////////////////////////////////////////////////////////

CyclingAnimatedLine::CyclingAnimatedLine(simVis::ScenarioManager& scenario, double generateInterval)
  : DataGenerator(generateInterval),
    scenario_(scenario),
    anchor_(0),
    currentTargetIndex_(0),
    line_(new simVis::AnimatedLineNode)
{
  line_->setStipple1(0x00ff);
  line_->setStipple2(0xff00);
  line_->setColor1(osg::Vec4f(1, 0.4509803921568627, 0, 1));  // rgba, red from SIMDIS 9
  line_->setColor2(osg::Vec4f(0, 0.2784313725490196, 1, 1));  // rgba, green from SIMDIS 9
  line_->setLineWidth(1);
  line_->setShiftsPerSecond(60);
  line_->setNodeMask(0);
}

void CyclingAnimatedLine::setAnchor(simData::ObjectId anchor)
{
  anchor_ = anchor;
}

void CyclingAnimatedLine::addTarget(simData::ObjectId target)
{
  targets_.push_back(target);
}

void CyclingAnimatedLine::generate_(double scenarioTime)
{
  if (targets_.empty() || anchor_ == 0)
    return;

  // Cycle targets
  currentTargetIndex_++;
  if (currentTargetIndex_ >= targets_.size())
    currentTargetIndex_ = 0;
  simData::ObjectId targetId = targets_[currentTargetIndex_];

  // get the end point locators
  osg::observer_ptr<simVis::PlatformNode> hostPlat = scenario_.find<simVis::PlatformNode>(anchor_);
  osg::observer_ptr<simVis::PlatformNode> targetPlat = scenario_.find<simVis::PlatformNode>(targetId);
  // hide the line if either host or target don't exist
  if (!hostPlat.valid() || !targetPlat.valid() || !hostPlat->getLocator()->isValid() ||
    !targetPlat->getLocator()->isValid())
  {
    line_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  // Make sure line is in scene
  if (line_->getNumParents() == 0)
    hostPlat->addChild(line_);

  line_->setEndPoints(hostPlat->getLocator(), targetPlat->getLocator());
  line_->setNodeMask(~0);
}

///////////////////////////////////////////////////////////

ToggleIcon::ToggleIcon(simData::DataStore& dataStore, simData::ObjectId id, const std::string& altIcon, double genInt)
  : DataGenerator(genInt),
    dataStore_(dataStore),
    id_(id),
    altIcon_(altIcon),
    showPrimary_(false)
{
  simData::DataStore::Transaction txn;
  const simData::PlatformPrefs* prefs = dataStore.platformPrefs(id, &txn);
  if (prefs)
    primaryIcon_ = prefs->icon();
}

void ToggleIcon::generate_(double scenarioTime)
{
  showPrimary_ = !showPrimary_;

  simData::DataStore::Transaction txn;
#ifndef USE_COMMANDS
  simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(id_, &txn);
  if (prefs)
  {
    prefs->set_icon(showPrimary_ ? primaryIcon_ : altIcon_);
    txn.complete(&prefs);
  }
#else
  simData::PlatformCommand* cmd = dataStore_.addPlatformCommand(id_, &txn);
  if (cmd)
  {
    cmd->set_time(scenarioTime);
    cmd->mutable_updateprefs()->set_icon(showPrimary_ ? primaryIcon_ : altIcon_);
    txn.complete(&cmd);
  }
#endif
}

///////////////////////////////////////////////////////////

ToggleDrawState::ToggleDrawState(simData::DataStore& dataStore, simData::ObjectId id, double genInt)
  : DataGenerator(genInt),
    dataStore_(dataStore),
    id_(id),
    show_(false)
{
}

/** Define a macro to make adding a Draw command easier */
#define ADD_DRAW_COMMAND(COMMANDTYPE, ADDCOMMANDMETHOD, COMMONPREFS_METHOD, VALUE) {\
  simData::COMMANDTYPE* cmd = dataStore_.ADDCOMMANDMETHOD(id_, &txn); \
  if (cmd) \
  { \
    cmd->set_time(scenarioTime); \
    cmd->mutable_updateprefs()->mutable_commonprefs()->COMMONPREFS_METHOD(VALUE); \
    txn.complete(&cmd); \
} }

void ToggleDrawState::generate_(double scenarioTime)
{
  show_ = !show_;

  simData::DataStore::Transaction txn;
#ifndef USE_COMMANDS
  simData::CommonPrefs* prefs = dataStore_.mutable_commonPrefs(id_, &txn);
  if (prefs)
  {
    prefs->set_datadraw(show_);
    txn.complete(&prefs);
  }
#else
  switch (dataStore_.objectType(id_))
  {
  case simData::DataStore::NONE:
    break;
  case simData::DataStore::PLATFORM:
    ADD_DRAW_COMMAND(PlatformCommand, addPlatformCommand, set_datadraw, show_);
    break;
  case simData::DataStore::BEAM:
    ADD_DRAW_COMMAND(BeamCommand, addBeamCommand, set_datadraw, show_);
    break;
  case simData::DataStore::GATE:
    ADD_DRAW_COMMAND(GateCommand, addGateCommand, set_datadraw, show_);
    break;
  case simData::DataStore::LASER:
    ADD_DRAW_COMMAND(LaserCommand, addLaserCommand, set_datadraw, show_);
    break;
  case simData::DataStore::PROJECTOR:
    ADD_DRAW_COMMAND(ProjectorCommand, addProjectorCommand, set_datadraw, show_);
    break;
  case simData::DataStore::LOB_GROUP:
    ADD_DRAW_COMMAND(LobGroupCommand, addLobGroupCommand, set_datadraw, show_);
    break;
  default:
    break;
  }
#endif
}

///////////////////////////////////////////////////////////

/** Colors are in protobuf style, 0xRRGGBBAA */
const uint32_t COLOR_MAP[10] = {
  0xff0000a0, 0xffffffa0, 0x0000ffa0, 0x808080a0, 0xff0000a0,
  0x00ff00a0, 0x0000ffa0, 0xffff00a0, 0xff00ffa0, 0x00ffffa0
  };

CycleColor::CycleColor(simData::DataStore& dataStore, simData::ObjectId id, size_t startIndex, double genInt)
  : DataGenerator(genInt),
    dataStore_(dataStore),
    id_(id),
    colorIndex_(startIndex)
{
}

void CycleColor::generate_(double scenarioTime)
{
  colorIndex_++;

  simData::DataStore::Transaction txn;
  const uint32_t color = COLOR_MAP[colorIndex_ % 10];
#ifndef USE_COMMANDS
  simData::CommonPrefs* prefs = dataStore_.mutable_commonPrefs(id_, &txn);
  if (prefs)
  {
    prefs->set_color(color);
    txn.complete(&prefs);
  }
#else
  switch (dataStore_.objectType(id_))
  {
  case simData::DataStore::NONE:
    break;
  case simData::DataStore::PLATFORM:
    ADD_DRAW_COMMAND(PlatformCommand, addPlatformCommand, set_color, color);
    break;
  case simData::DataStore::BEAM:
    ADD_DRAW_COMMAND(BeamCommand, addBeamCommand, set_color, color);
    break;
  case simData::DataStore::GATE:
    ADD_DRAW_COMMAND(GateCommand, addGateCommand, set_color, color);
    break;
  case simData::DataStore::LASER:
    ADD_DRAW_COMMAND(LaserCommand, addLaserCommand, set_color, color);
    break;
  case simData::DataStore::PROJECTOR:
    ADD_DRAW_COMMAND(ProjectorCommand, addProjectorCommand, set_color, color);
    break;
  case simData::DataStore::LOB_GROUP:
    ADD_DRAW_COMMAND(LobGroupCommand, addLobGroupCommand, set_color, color);
    break;
  default:
    break;
  }
#endif
}

///////////////////////////////////////////////////////////

MissileLaunchPlatform::MissileLaunchPlatform(simData::DataStore& dataStore)
  : dataStore_(dataStore),
    id_(0),
    startLla_(22 * simCore::DEG2RAD, -159 * simCore::DEG2RAD, 100.0),
    gravity_(0, 0, -9.8),
    maxSpeed_(4000), // https://www.google.com/search?q=typical+missile+speed
    currentTime_(0),
    currentLla_(startLla_),
    accelerating_(true)
{
  acceleration_ = osg::Vec3f(-1, 1.2, 2.5);
  acceleration_.normalize();
  // https://www.google.com/search?q=typical+missile+acceleration
  acceleration_ *= 50;
}

void MissileLaunchPlatform::setStartingLla(const simCore::Vec3& lla)
{
  startLla_ = lla;
}

void MissileLaunchPlatform::setAcceleration(const simCore::Vec3& acceleration)
{
  acceleration_.set(acceleration.x(), acceleration.y(), acceleration.z());
}

void MissileLaunchPlatform::setGravity(const simCore::Vec3& gravity)
{
  gravity_.set(gravity.x(), gravity.y(), gravity.z());
}

void MissileLaunchPlatform::setMaxSpeed(double speed)
{
  maxSpeed_ = speed;
}

simData::ObjectId MissileLaunchPlatform::id() const
{
  return id_;
}

void MissileLaunchPlatform::create(const std::string& name)
{
  {
    simData::DataStore::Transaction txn;
    simData::PlatformProperties* props = dataStore_.addPlatform(&txn);
    id_ = props->id();
    props->set_source("MissileLaunchPlatform");
    txn.complete(&props);
  }
  {
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(id_, &txn);
    prefs->mutable_commonprefs()->set_name(name);
    prefs->set_icon(EXAMPLE_MISSILE_ICON);
    txn.complete(&prefs);
  }
}

void MissileLaunchPlatform::generate_(double scenarioTime)
{
  if (!id_)
    return;

  // Initialize currentTime_ as needed
  if (currentTime_ == 0)
    currentTime_ = scenarioTime - 1.0;

  // Calculate the new velocity
  const double delta = scenarioTime - currentTime_;
  osg::Vec3f effectiveAccel = gravity_;
  if (accelerating_)
  {
    effectiveAccel += acceleration_;
    // Stop accelerating if we reached our max speed (i.e. burnout)
    if (currentVelocity_.length() > maxSpeed_)
      accelerating_ = false;
  }
  currentVelocity_ += effectiveAccel * delta;

  // Calculate an orientation that follows missile path (not realistic)
  simCore::Vec3 enuVelocity(currentVelocity_.x(), currentVelocity_.y(), currentVelocity_.z());
  simCore::Vec3 enuOrientation;
  simCore::calculateFlightPathAngles(enuVelocity, enuOrientation);

  // Apply the current velocity to the most recent position (move the platform)
  cc_.setReferenceOrigin(currentLla_);
  const osg::Vec3f newXyzPos = currentVelocity_ * delta;
  const simCore::Coordinate xyz(simCore::COORD_SYS_XEAST,
    simCore::Vec3(newXyzPos.x(), newXyzPos.y(), newXyzPos.z()),
    enuOrientation,
    enuVelocity,
    simCore::Vec3(effectiveAccel.x(), effectiveAccel.y(), effectiveAccel.z())
    );
  // Get the position in LLA.  We could go right to ECEF, but using LLA intermediary makes reading
  // the code a little easier due to some required comparisons later, and because of later reset.
  simCore::Coordinate newLla;
  cc_.convert(xyz, newLla, simCore::COORD_SYS_LLA);

  // Is the platform falling, and under the ground?  If so, reset
  if (currentVelocity_.z() < 0.0 && newLla.position().alt() < 0.0)
  {
    newLla.setPosition(startLla_);
    // Give it a starting velocity equal to 1 second of acceleration
    newLla.setVelocity(acceleration_.x(), acceleration_.y(), acceleration_.z());
    // Recalculate the orientation
    simCore::calculateFlightPathAngles(newLla.velocity(), enuOrientation);
    newLla.setOrientation(enuOrientation);
    accelerating_ = true;
    currentVelocity_ = acceleration_;
  }
  // Save the current position so we can initialize the coordinate converter next round
  currentLla_ = newLla.position();

  // Convert the data point into ECEF so we can add it to the data store
  simCore::Coordinate ecef;
  cc_.convert(newLla, ecef, simCore::COORD_SYS_ECEF);

  // Create the transaction and add the data
  simData::DataStore::Transaction txn;
  simData::PlatformUpdate* point = dataStore_.addPlatformUpdate(id_, &txn);
  point->set_time(scenarioTime);
  point->setPosition(ecef.position());
  point->setOrientation(ecef.orientation());
  point->setVelocity(ecef.velocity());
  txn.complete(&point);

  // Save the time so we have reasonable deltas
  currentTime_ = scenarioTime;
}

}
