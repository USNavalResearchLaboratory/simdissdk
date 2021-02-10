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
#include <iomanip>
#include "osg/CoordinateSystemNode"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simNotify/Notify.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Platform.h"
#include "simVis/SceneManager.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#define LC "[PlatformSimulator] "

//----------------------------------------------------------------------------

namespace
{
  static osg::EllipsoidModel* ellip = new osg::EllipsoidModel();
}

namespace simUtil {

//----------------------------------------------------------------------------
PlatformSimulator::PlatformSimulator(simData::ObjectId platformId)
  : t0_(osg::Timer::instance()->time_s()),
    wp_t_(-1.),
    platformId_(platformId),
    beamId_(~(static_cast<simData::ObjectId>(0))),
    simulateRoll_(false),
    simulatePitch_(false)
{
  //nop
}

void PlatformSimulator::addWaypoint(const Waypoint &wp)
{
  waypoints_.push_back(wp);
}

void PlatformSimulator::updatePlatform(double time, simData::PlatformUpdate *update)
{
  double now = time;

  // Track if we're done early
  if (done_ || waypoints_.size() < 2)
    return;

  // our 2 waypoints.
  Waypoint wp0 = waypoints_.front();
  Waypoint wp1 = *(waypoints_.begin() + 1);

  // see if we need to advance to the next waypoint.
  if (wp_t_ > 1.0)
  {
    // if not looping and only two waypoints left, simulation is complete
    if (!loop_ && (waypoints_.size() == 2))
    {
      done_ = true;
      return;
    }

    // Keep the loop going if necessary
    if (loop_)
      waypoints_.push_back(wp0);
    waypoints_.pop_front();
    wp0 = waypoints_.front();
    wp1 = *(waypoints_.begin() + 1);
    wp_t_ = -1.0;
  }

  double lat0_rad = osg::DegreesToRadians(wp0.lat_deg_);
  double lon0_rad = osg::DegreesToRadians(wp0.lon_deg_);
  double lat1_rad = osg::DegreesToRadians(wp1.lat_deg_);
  double lon1_rad = osg::DegreesToRadians(wp1.lon_deg_);
  double dlon_rad = lon1_rad - lon0_rad;

  // initialize the distance.
  if (wp_t_ < 0.0)
  {
    wp_t_ = 0.0;

    // great circle distance
    wp_dist_rad_ = acos((sin(lat0_rad)*sin(lat1_rad))+(cos(lat0_rad)*cos(lat1_rad)*cos(dlon_rad)));
    double wp_dist_deg = osg::RadiansToDegrees(wp_dist_rad_);

    wp_t0_ = now;
    prev_time_ = now;
    wp_duration_ = wp1.duration_s_;
    //wp_duration_ = wp_dist_deg / wp1.speed_dps_;

    SIM_DEBUG << LC
      << "distance = " << wp_dist_deg << " degrees; duration = " << wp_duration_ << " seconds"
      << std::endl;
  }
  //else
  {
    // ratio of segment completed:
    wp_t_ = (now - wp_t0_) / wp_duration_;

    // slerp along the great circle
    osg::Vec3d p0;
    ellip->convertLatLongHeightToXYZ(lat0_rad, lon0_rad, wp0.alt_m_, p0.x(), p0.y(), p0.z());
    double p0len = p0.length();
    p0.normalize();

    osg::Vec3d p1;
    ellip->convertLatLongHeightToXYZ(lat1_rad, lon1_rad, wp1.alt_m_, p1.x(), p1.y(), p1.z());
    double p1len = p1.length();
    p1.normalize();

    // spherical interpolation:
    double dotProd = p0 * p1;
    double theta = 0.;
    // limit theta to range of acos, since rounding errors may produce a value > 1 or < -1
    if (abs(dotProd) <= 1.0)
      theta = acos(dotProd); // * = dot product
    osg::Vec3d slerp;
    // Avoid divide by zero
    if (!simCore::areEqual(sin(theta), 0.))
    {
      slerp = (p0*sin((1-wp_t_)*theta) + p1*sin(wp_t_*theta)) / sin(theta);
      slerp *= 0.5 * (p0len + p1len);
    }
    else
    {
      slerp = p0 * p0len;
    }

    double lat_rad, lon_rad, height;
    ellip->convertXYZToLatLongHeight(slerp.x(), slerp.y(), slerp.z(), lat_rad, lon_rad, height);

    // now calculate the bearing on the great circle:
    double new_dlon_rad = lon1_rad - lon_rad;
    double by = sin(new_dlon_rad)*cos(lat1_rad);
    double bx = cos(lat_rad)*sin(lat1_rad) - sin(lat_rad)*cos(lat1_rad)*cos(new_dlon_rad);
    double bearing_rad = atan2(by, bx);
    if (overrideYaw_)
      bearing_rad = overrideYawValue_;

    // interpolate the altitude:
    double alt = wp0.alt_m_ + wp_t_ * (wp1.alt_m_ - wp0.alt_m_);

    // uncomment the following to simulate roll:
    double roll_rad = 0.0;
    if (simulateRoll_)
      roll_rad = simCore::DEG2RAD*(20.0 * sin(now*0.35));

    double pitch_rad = 0.0;
    if (simulatePitch_)
      pitch_rad = simCore::DEG2RAD*(45.0 * sin(now*0.35));

    // calculate a velocity vector (LTP):
    simCore::Vec3 new_lla(lat_rad, lon_rad, alt);
    simCore::Vec3 velocityVector;
    simCore::calculateVelFromGeodeticPos(new_lla, prev_lla_, now-prev_time_, velocityVector);

    // convert coordinates to ECEF (orientation to NED)
    simCore::Coordinate inCoords(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(lat_rad, lon_rad, alt),
      simCore::Vec3(bearing_rad, pitch_rad, roll_rad),
      velocityVector);

    simCore::Coordinate ecefCoords;
    simCore::CoordinateConverter coordConverter;
    coordConverter.convertGeodeticToEcef(inCoords, ecefCoords, simCore::LOCAL_LEVEL_FRAME_NED);

    // Fill update with new position.
    update->set_time(now);
    update->set_x(ecefCoords.x());
    update->set_y(ecefCoords.y());
    update->set_z(ecefCoords.z());
    update->set_psi(ecefCoords.psi());
    update->set_theta(ecefCoords.theta());
    update->set_phi(ecefCoords.phi());
    update->set_vx(ecefCoords.vx());
    update->set_vy(ecefCoords.vy());
    update->set_vz(ecefCoords.vz());

    // Calculate the velocity vector
    //simCore::Vec3 new_lla( lat_rad, lon_rad, alt );
    //simCore::Vec3 vv;
    //simCore::calculateVelFromGeodeticPos( new_lla, prev_lla_, now-prev_time_, vv );
    //update->mutable_velocity()->set_vx( vv.x() );
    //update->mutable_velocity()->set_vy( vv.y() );
    //update->mutable_velocity()->set_vz( vv.z() );

    prev_lla_.set(lat_rad, lon_rad, alt);
    prev_time_ = now;

    SIM_DEBUG
      << "POS: ("
      << osg::RadiansToDegrees(lat_rad) << ", "
      << osg::RadiansToDegrees(lon_rad) << ", "
      << alt << ") "
      << "bearing = " << osg::RadiansToDegrees(bearing_rad)
      << std::endl;
  }
}

void PlatformSimulator::updateBeam(double now, simData::BeamUpdate *update, simData::PlatformUpdate *platform)
{
  // oscillate the az and el offsets
  double azimuth   = osg::DegreesToRadians(5.0 * sin(0.5 * now));
  double range     = 100000.0;
  simCore::Vec3 ecefPos;
  platform->position(ecefPos);
  simCore::Vec3 ecefOri;
  platform->orientation(ecefOri);
  simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()), simCore::Vec3(ecefOri.psi(), ecefOri.theta(), ecefOri.phi()));
  simCore::Coordinate llaCoord;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord, simCore::LOCAL_LEVEL_FRAME_NED);

  update->set_time(now);
  update->set_azimuth(azimuth + llaCoord.yaw());
  update->set_elevation(llaCoord.pitch());
  update->set_range(range);
}

void PlatformSimulator::updateGate(double now, simData::GateUpdate *update, simData::PlatformUpdate *platform)
{
  double azimuth = osg::DegreesToRadians(5.0 * sin(0.5 * now));
  simCore::Vec3 ecefPos;
  platform->position(ecefPos);
  simCore::Vec3 ecefOri;
  platform->orientation(ecefOri);
  simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()), simCore::Vec3(ecefOri.psi(), ecefOri.theta(), ecefOri.phi()));
  simCore::Coordinate llaCoord;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord, simCore::LOCAL_LEVEL_FRAME_NED);

  update->set_time(now);
  update->set_azimuth(azimuth + llaCoord.yaw());
  update->set_elevation(llaCoord.pitch());
  update->set_width(osg::DegreesToRadians(60.0));
  update->set_height(osg::DegreesToRadians(30.0));
  update->set_minrange(85000);
  update->set_maxrange(90000);
  update->set_centroid(88000);
}

//---------------------------------------------------------------------------
PlatformSimulatorManager::PlatformSimulatorManager(simData::DataStore* datastore)
  : datastore_(datastore)
{
}

void PlatformSimulatorManager::addSimulator(simUtil::PlatformSimulator* simulator)
{
  simulators_.push_back(simulator);
}

void PlatformSimulatorManager::play(double time)
{
  SIM_DEBUG << LC << "Updating datastore to time = " << std::setprecision(5) << time << std::endl;
  datastore_->update(time);
}

void PlatformSimulatorManager::simulate(double start, double end, double hertz)
{
  const double step = 1.0 / hertz;
  for (double now = start; now <= end; now += step)
  {
    simulate_(now);
  }
}

void PlatformSimulatorManager::simulate_(double now)
{
  //for each simulator
  for (PlatformSimulators::iterator iter = simulators_.begin(); iter != simulators_.end(); ++iter)
  {
    simUtil::PlatformSimulator* sim = iter->get();
    if (sim->doneSimulating() || now < sim->startTime())
      continue;

    // create a position update transaction
    simData::DataStore::Transaction platformTransaction;

    simData::PlatformUpdate* platformUpdate = nullptr;

    // add the update for the platform
    if (sim->getPlatformId() != ~(static_cast<simData::ObjectId>(0)))
    {
      platformUpdate = datastore_->addPlatformUpdate(sim->getPlatformId(), &platformTransaction);
      if (platformUpdate)
      {
        sim->updatePlatform(now, platformUpdate);
        platformTransaction.commit();      // Commit the change but do not release platformUdpate yet

        // add any beam updates:
        simData::DataStore::IdList beamIds;
        datastore_->beamIdListForHost(sim->getPlatformId(), &beamIds);
        for (simData::DataStore::IdList::iterator b = beamIds.begin(); b != beamIds.end(); ++b)
        {
          simData::DataStore::Transaction beamTransaction;
          simData::BeamUpdate* beamUpdate = datastore_->addBeamUpdate(*b, &beamTransaction);
          if (beamUpdate)
          {
            sim->updateBeam(now, beamUpdate, platformUpdate);
            beamTransaction.complete(&beamUpdate);      // Commit and release; done with beamUpdate

            // add any gate updates:
            simData::DataStore::IdList gateIds;
            datastore_->gateIdListForHost(*b, &gateIds);
            for (simData::DataStore::IdList::iterator g = gateIds.begin(); g != gateIds.end(); ++g)
            {
              simData::DataStore::Transaction gateTransaction;
              simData::GateUpdate* gateUpdate = datastore_->addGateUpdate(*g, &gateTransaction);
              if (gateUpdate)
              {
                sim->updateGate(now, gateUpdate, platformUpdate);
                gateTransaction.complete(&gateUpdate);      // Commit and release; done with gateUpdate
              }
            }
          }
        }

        // Release the platformUpdate and close the transaction now that all updateGate and updateBeam calls are done
        platformTransaction.release(&platformUpdate);
      }
    }
  }
}

//---------------------------------------------------------------------------
CircumnavigationPlatformSimulation::CircumnavigationPlatformSimulation(simVis::SceneManager* sceneManager, simVis::View* mainView)
  : sceneManager_(sceneManager),
    dataStore_(new simData::MemoryDataStore),
    platformId_(0)
{
  init_(mainView);
}

CircumnavigationPlatformSimulation::~CircumnavigationPlatformSimulation()
{
  delete dataStore_;
}

void CircumnavigationPlatformSimulation::init_(simVis::View* mainView)
{
  // Don't crash on nullptr accesses
  if (!sceneManager_.valid() || mainView == nullptr)
    return;

  // Bind the scene manager to the data store
  sceneManager_->getScenario()->bind(dataStore_);
  simMan_ = new simUtil::PlatformSimulatorManager(dataStore_);
  createPlatform_();
  simMan_->simulate(0, 120, 60);
  mainView->addEventHandler(new simUtil::SimulatorEventHandler(simMan_.get(), 0, 120, true));
  platformNode_ = sceneManager_->getScenario()->find<simVis::PlatformNode>(platformId_);
}

simUtil::PlatformSimulatorManager* CircumnavigationPlatformSimulation::simulationManager() const
{
  return simMan_.get();
}

simData::DataStore* CircumnavigationPlatformSimulation::dataStore() const
{
  return dataStore_;
}

simData::ObjectId CircumnavigationPlatformSimulation::platformId() const
{
  return platformId_;
}

simVis::PlatformNode* CircumnavigationPlatformSimulation::platformNode() const
{
  return platformNode_.get();
}

void CircumnavigationPlatformSimulation::createPlatform_()
{
  platformId_ = 0;

  { // create the platform in the database (artificial scope for transaction)
    simData::DataStore::Transaction transaction;
    simData::PlatformProperties* newProps = dataStore_->addPlatform(&transaction);
    platformId_ = newProps->id();
    transaction.complete(&newProps);
  }

  { // Set platform prefs (artificial scope for transaction)
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(platformId_, &xaction);
    prefs->mutable_commonprefs()->set_name("Satellite");
    prefs->set_dynamicscale(true);
    prefs->set_icon(EXAMPLE_IMAGE_ICON);
    prefs->set_rotateicons(simData::IR_2D_YAW);
    xaction.complete(&prefs);
  }

  // Run the simulator
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId_);
  sim->addWaypoint(simUtil::Waypoint(0, -90, 15000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0, 0, 15000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0, 90, 15000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0, 180, 15000, 30.0));
  simMan_->addSimulator(sim.get());
}

//---------------------------------------------------------------------------
MultiPlatformSimulation::MultiPlatformSimulation(simVis::SceneManager* sceneManager, simVis::View* mainView)
  : sceneManager_(sceneManager),
  dataStore_(new simData::MemoryDataStore)
{
  init_(mainView);
}

MultiPlatformSimulation::~MultiPlatformSimulation()
{
  delete dataStore_;
}

void MultiPlatformSimulation::simulate(double start, double end, double hertz)
{
  if (simMan_)
    simMan_->simulate(start, end, hertz);
}

simData::DataStore* MultiPlatformSimulation::dataStore() const
{
  return dataStore_;
}

simData::ObjectId  MultiPlatformSimulation::createPlatform(const std::string& name, const std::string& icon)
{
  simData::ObjectId id = 0;
  if (name.empty())
    return id;

  // create the platform in the database
  simData::DataStore::Transaction transaction;
  simData::PlatformProperties* newProps = dataStore_->addPlatform(&transaction);
  id = newProps->id();
  transaction.complete(&newProps);

  // Set platform prefs
  simData::PlatformPrefs* prefs = dataStore_->mutable_platformPrefs(id, &transaction);
  prefs->mutable_commonprefs()->set_name(name);
  prefs->set_dynamicscale(true);
  prefs->set_icon(icon);
  prefs->set_rotateicons(simData::IR_2D_UP);
  transaction.complete(&prefs);

  return id;
}

void MultiPlatformSimulation::addPlatformSim(simData::ObjectId id, simUtil::PlatformSimulator* simulator)
{
  // If assert fails, this ID is already in the map. Somebody is tampering with the data store and this class doesn't know about it
  assert(plats_.find(id) == plats_.end());
  plats_[id] = simulator;
  simMan_->addSimulator(simulator);
}

void MultiPlatformSimulation::init_(simVis::View* mainView)
{
  // Don't crash on nullptr accesses
  if (!sceneManager_.valid() || mainView == nullptr)
    return;

  // Bind the scene manager to the data store
  sceneManager_->getScenario()->bind(dataStore_);
  simMan_ = new simUtil::PlatformSimulatorManager(dataStore_);
  mainView->addEventHandler(new simUtil::SimulatorEventHandler(simMan_.get(), 0, 120, true));
}

//---------------------------------------------------------------------------
SimulatorEventHandler::SimulatorEventHandler(simUtil::PlatformSimulatorManager *simMgr, double startTime, double endTime, bool loop)
 : simMgr_(simMgr),
   startTime_(startTime),
   endTime_(endTime),
   currentTime_(startTime),
   lastEventTime_(-1.0),
   loop_(loop),
   playing_(true)
{
  //nop
}

SimulatorEventHandler::~SimulatorEventHandler()
{
}

bool SimulatorEventHandler::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
  // handle FRAME events
  if (ea.getEventType() == ea.FRAME)
  {
    double t = ea.getTime();

    if (lastEventTime_ < 0.0)
      lastEventTime_ = t;

    if (playing_)
    {
      double delta = t - lastEventTime_;

      if (simMgr_->getDataStore()->getBoundClock() &&
          simMgr_->getDataStore()->getBoundClock()->timeDirection() == simCore::REVERSE)
      {
          currentTime_ -= delta;
      }
      else
      {
          currentTime_ += delta;
      }

      double simTime = loop_ ? fmod(currentTime_, (endTime_-startTime_)) : currentTime_;
      simMgr_->play(simTime);
    }

    lastEventTime_ = t;
  }

  // PLAY/PAUSE
  else if (ea.getEventType() == ea.KEYDOWN)
  {
    if (ea.getKey() == '.')
    {
      playing_ = !playing_;
    }
  }

  return false;
}

void SimulatorEventHandler::setTime(double t)
{
  currentTime_ = simCore::sdkMax(t, startTime_);
  lastEventTime_ = -1.0;
}

double SimulatorEventHandler::getTime() const
{
  return currentTime_;
}

}
