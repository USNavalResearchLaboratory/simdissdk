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
#ifndef SIMUTIL_PLATFORM_SIMULATOR_H
#define SIMUTIL_PLATFORM_SIMULATOR_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Math.h"
#include "simData/ObjectId.h"
#include "simData/DataTypes.h"
#include "osgGA/GUIEventHandler"
#include <deque>

namespace simData { class DataStore; }
namespace simVis
{
  class SceneManager;
  class PlatformNode;
  class View;
}

namespace simUtil
{
/// A point to travel to, along with the time to get there
class /*SDKUTIL_EXPORT*/ Waypoint // no export; header only class
{
public:
  /// Construct a waypoint with position and duration
  Waypoint(double lat_deg, double lon_deg, double alt_m, float duration_s)
    : lat_deg_(lat_deg), lon_deg_(lon_deg), alt_m_(alt_m), duration_s_(duration_s)
  {
  }

  double lat_deg_;    ///< Latitude, in degrees
  double lon_deg_;    ///< Longitude, in degrees
  double alt_m_;      ///< Altitude, in degrees
  double duration_s_; ///< How long to take to get there
};
typedef std::deque<Waypoint> Waypoints; ///< a queue of Waypoints

/// A simple simulator that flies the platform between Waypoints
class SDKUTIL_EXPORT PlatformSimulator : public osg::Referenced
{
public:
  /// Constructs the simulator with its platform identifier
  PlatformSimulator(simData::ObjectId platformId);

  /// Set the start time of the simulator
  void setStartTime(double start) { start_ = start; }
  /// Get the start time of the simulator
  double startTime() const { return start_; }

  /// Returns true when the simulator is done simulating. Always false if looping
  bool doneSimulating() const { return done_; }

  /// Set whether this simulator should loop through its waypoints when simulating
  void setLoop(bool loop) { loop_ = loop; }

  /// Whether to simulate platform roll (default = false)
  void setSimulateRoll(bool value) { simulateRoll_ = value; }

  /// Whether to simulate platform pitch (default = false)
  void setSimulatePitch(bool value) { simulatePitch_ = value; }

  /// Set an override yaw value (in radians) for the sim. If set, the given yaw value will always be used instead of calculated yaw values
  void setOverrideYaw(double yaw) { overrideYaw_ = true; overrideYawValue_ = yaw; }

  /// id of the simulated platform
  simData::ObjectId getPlatformId() const { return platformId_; }

  /// Beam attached to the simulated platform
  void setBeamId(simData::ObjectId beamId) { beamId_ = beamId; }
  /// Retrieve Beam ID attached to the simulated platform
  simData::ObjectId getBeamId() const { return beamId_; }

public:
  /// Add 'wp' to the list of Waypoints
  void addWaypoint(const Waypoint &wp);

  /// Compute position for update, based on the specified time value
  void updatePlatform(double time, simData::PlatformUpdate* update);

  /// Compute a new Beam configuration
  void updateBeam(double time, simData::BeamUpdate* update, simData::PlatformUpdate* platform);

  /// Compute a new Gate configuration
  void updateGate(double time, simData::GateUpdate* update, simData::PlatformUpdate* platform);

protected:
  /// osg::Referenced-derived
  virtual ~PlatformSimulator() {}

private:
  double t0_;

  Waypoints waypoints_;

  double wp_t_;
  double wp_t0_;
  double wp_duration_;
  double wp_dist_rad_;

  simCore::Vec3 prev_lla_;
  double        prev_time_;

  simData::ObjectId platformId_;
  simData::ObjectId beamId_;

  double start_ = 0.; // Start time of the simulator
  bool done_ = false; // Track if done simulating
  bool loop_ = true; // If true, loop from end data point to beginning
  bool simulateRoll_;
  bool simulatePitch_;
  bool overrideYaw_ = false;
  double overrideYawValue_ = 0.;
};

/**
* Used in conjunction with one or more PlatformSimulators to update a datastore
* with the simulated values.
*/
class SDKUTIL_EXPORT PlatformSimulatorManager : public osg::Referenced
{
public:
  /// Constructs a simulation manager on a given data store
  PlatformSimulatorManager(simData::DataStore *datastore);

  /// DataStore to populate
  simData::DataStore* getDataStore() { return datastore_; }

  /// register a platform position simulator
  ///@param[in] simulator Simulator to use
  void addSimulator(simUtil::PlatformSimulator *simulator);

  /**
  * Run all the simulators from startTime to endTime, stepping by "hertz"
  * frames per second.
  * This populates the datastore with updates that you can later play
  * back by calling play().
  */
  void simulate(double startTime, double endTime, double hertz);

  /// Update the data store to the given timestamp.
  void play(double timestamp);

protected:
  /// osg::Referenced-derived
  virtual ~PlatformSimulatorManager() {}

protected: // types
  /// Vector of pointers to simulators
  typedef std::vector< osg::ref_ptr<simUtil::PlatformSimulator> > PlatformSimulators;

protected: // data
  simData::DataStore* datastore_; ///< reference to the global data store
  PlatformSimulators simulators_; ///< map of platform ids to references to simulators

  /// Moves time forward to the given time
  void simulate_(double time);
};

/**
  * Utility class that creates a Data Store and Simulation Manager, adds a single PlatformSimulation,
  * and binds it to the scenario.  This is a one-stop-shop for generating example data using
  * PlatformSimulatorManager in example code.
  */
class SDKUTIL_EXPORT CircumnavigationPlatformSimulation : public osg::Referenced
{
public:
  /** Constructor */
  CircumnavigationPlatformSimulation(simVis::SceneManager* sceneManager, simVis::View* mainView);

  /** Pointer to the class-owned simulation manager. */
  simUtil::PlatformSimulatorManager* simulationManager() const;
  /** Pointer to the class-owned data store. */
  simData::DataStore* dataStore() const;
  /** ID of the circumnavigating platform. */
  simData::ObjectId platformId() const;
  /** Pointer to the node representing the platform in the scene */
  simVis::PlatformNode* platformNode() const;

protected:
  /** Derived from osg::Referenced */
  virtual ~CircumnavigationPlatformSimulation();

private:
  /** Develops a simulation of an entity circumnavigating the globe, and binds it to the given scene manager. */
  void init_(simVis::View* mainView);
  /** Responsible for creating the platform simulator for the single entity circling the globe */
  void createPlatform_();

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMan_;
  osg::observer_ptr<simVis::SceneManager> sceneManager_;
  simData::DataStore* dataStore_;
  osg::observer_ptr<simVis::PlatformNode> platformNode_;
  simData::ObjectId platformId_;
};

/**
 * Utility class that creates a Data Store and Simulation Manager, then allows adding
 * multiple platforms to the scenario, each with their own PlatformSimulator instances.
 */
class SDKUTIL_EXPORT MultiPlatformSimulation : public osg::Referenced
{
public:
  /** Constructor */
  MultiPlatformSimulation(simVis::SceneManager* sceneManager, simVis::View* mainView);

  /** Pointer to the class-owned data store. */
  simData::DataStore* dataStore() const;
  /** Run the simulators from the given start to end times, populating the data store in file mode. */
  void simulate(double start, double end, double hertz);

  /** Convenience method used to create a new platform with the given name in the data store */
  simData::ObjectId createPlatform(const std::string& name, const std::string& icon);
  /** Add a platform with the given ID and given simulator to the simulation */
  void addPlatformSim(simData::ObjectId id, simUtil::PlatformSimulator* simulator);

protected:
  /** Derived from osg::Referenced */
  virtual ~MultiPlatformSimulation();

private:
  /** Initializes the simulation with the given view and binds it to the scene manager. */
  void init_(simVis::View* mainView);

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMan_;
  osg::observer_ptr<simVis::SceneManager> sceneManager_;
  simData::DataStore* dataStore_;

  std::map<simData::ObjectId, osg::ref_ptr<PlatformSimulator> > plats_;
};

/// Update a platform simulator using the OSG frame timer.
class SDKUTIL_EXPORT SimulatorEventHandler : public osgGA::GUIEventHandler
{
public:
  /// Constructs a new SimulatorEventHandler
  SimulatorEventHandler(simUtil::PlatformSimulatorManager *simMgr, double startTime, double endTime, bool loop = true);

  /// handle an event
  virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

  /// Changes the current time
  void setTime(double t);

  /// Retrieves the current time
  double getTime() const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }
  /** Return the class name */
  virtual const char* className() const { return "SimulatorEventHandler"; }

protected:
  /// osg::Referenced-derived
  virtual ~SimulatorEventHandler();

private:
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr_;
  double startTime_, endTime_, currentTime_, lastEventTime_;
  bool loop_;
  bool playing_;
};

} // namespace simUtil

#endif /* SIMUTIL_PLATFORM_SIMULATOR_H */
