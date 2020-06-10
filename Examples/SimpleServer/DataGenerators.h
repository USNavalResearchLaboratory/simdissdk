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
#ifndef SS_DATAGENERATORS_H
#define SS_DATAGENERATORS_H

#include <vector>
#include "osg/ref_ptr"
#include "osg/Referenced"
#include "simCore/Calc/CoordinateConverter.h"
#include "simData/ObjectId.h"

namespace simData { class DataStore; }

namespace simVis {
  class AnimatedLineNode;
  class ScenarioManager;
}

namespace SimpleServer {

/**
 * Virtual base class for a Simple Server data generator.
 *
 * Data generators are intended to be attached to the DataEngine class,
 * but may also be used in standalone contexts.  The intent is to provide
 * a pure virtual method generate_() that the derived version implements
 * in order to generate some sort of data.  An interval is specified in the
 * constructor.  The generate_() method is called no more frequently than
 * the interval provided in the constructor.  This allows for periodic
 * event generation.
 */
class DataGenerator : public osg::Referenced
{
public:
  /**
  * Constructor.  The generate_() method is called only after generateInterval
  * seconds have passed since last call.
  * @param generateInterval Minimum interval (seconds) between successive calls
  *   to your generate_() method.  If set to 0.0, then generate_() is called on
  *   every idle().  Higher values can introduce delays, useful for periodic
  *   state change generators.
  */
  explicit DataGenerator(double generateInterval = 0.0);

  /**
   * Called by Data Engine to trigger your generate_().  May stagger calls to
   * generate_() based on interval passed in at the time of DataGenerator construction.
   * @param scenarioTime Scenario time in seconds since the data store's reference
   *   year.  This time value is passed to generate_() directly.
   */
  void idle(double scenarioTime);

protected:
  /**
   * Override this method to generate your data.
   * @param scenarioTime Scenario time in seconds since the data store's reference
   *   year.  For live mode generation, it is expected this time will be used as
   *   your generator's data time.
   */
  virtual void generate_(double scenarioTime) = 0;

private:
  double lastTime_;
  double generateInterval_;
};

/** Rotates a platform around a central point */
class CirclingPlatform : public DataGenerator
{
public:
  explicit CirclingPlatform(simData::DataStore& dataStore);

  /** Central point to rotate around */
  void setCenterLla(const simCore::Vec3& lla);
  /** Apply this XYZ offset (meters), relative to the center LLA */
  void setXyzOffset(const simCore::Vec3& xyzOffset);
  /** Sets the radius for the circle */
  void setRange(double rangeMeters);

  /** Create the beam with the given name */
  void create(const std::string& name);
  /** Retrieve the ID from the created entity */
  simData::ObjectId id() const;

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  simCore::CoordinateConverter cc_;
  simCore::Vec3 xyzOffset_;
  double rangeMeters_;
};

/** Given a platform host, creates a beam and rotates it */
class RotatingBeam : public DataGenerator
{
public:
  explicit RotatingBeam(simData::DataStore& dataStore);

  /** Sets the range in meters */
  void setRange(double range);
  /** Sets the elevation angle in radians */
  void setElevation(double elevation);

  /** Create the platform with the given name */
  void create(simData::ObjectId hostPlatform, const std::string& name);
  /** Retrieve the ID from the created entity */
  simData::ObjectId id() const;

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  double rangeMeters_;
  double elevationRads_;
};

/** Given a platform host and an array of targets, creates a target beam that cycles targets */
class CyclingTargetBeam : public DataGenerator
{
public:
  CyclingTargetBeam(simData::DataStore& dataStore, double generateInterval=4.0);

  /** Adds a possible target to our list */
  void addTarget(simData::ObjectId target);

  /** Create the beam with the given name */
  void create(simData::ObjectId hostPlatform, const std::string& name);
  /** Retrieve the ID from the created entity */
  simData::ObjectId id() const;

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  std::vector<simData::ObjectId> targets_;
  size_t currentTargetIndex_;
};

/** Given a beam host, creates a gate and rotates it */
class RotatingGate : public DataGenerator
{
public:
  explicit RotatingGate(simData::DataStore& dataStore);

  /** Sets various gate parameters */
  void setParameters(double elevRad, double widthRad, double heightRad, double minRng, double maxRange);

  /** Create the gate with the given name */
  void create(simData::ObjectId hostBeam, const std::string& name);
  /** Retrieve the ID from the created entity */
  simData::ObjectId id() const;

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  double elevationRads_;
  double widthRads_;
  double heightRads_;
  double minRangeMeters_;
  double maxRangeMeters_;
};

/** Given an anchor entity, cycles through target entities for animated lines */
class CyclingAnimatedLine : public DataGenerator
{
public:
  CyclingAnimatedLine(simVis::ScenarioManager& scenario, double generateInterval=2.0);
  void setAnchor(simData::ObjectId anchor);
  void addTarget(simData::ObjectId target);

protected:
  virtual void generate_(double scenarioTime);

private:
  simVis::ScenarioManager& scenario_;
  simData::ObjectId anchor_;
  std::vector<simData::ObjectId> targets_;
  size_t currentTargetIndex_;
  osg::ref_ptr<simVis::AnimatedLineNode> line_;
};

/** Given a platform, toggles between original icon and an alternate icon */
class ToggleIcon : public DataGenerator
{
public:
  ToggleIcon(simData::DataStore& dataStore, simData::ObjectId id, const std::string& altIcon, double genInt=3.0);

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  std::string primaryIcon_;
  std::string altIcon_;
  bool showPrimary_;
};

/** Given an entity ID, toggles the draw state of the entity */
class ToggleDrawState : public DataGenerator
{
public:
  ToggleDrawState(simData::DataStore& dataStore, simData::ObjectId id, double genInt=3.0);
protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  bool show_;
};

/** Given an entity ID, toggles the draw state of the entity */
class CycleColor : public DataGenerator
{
public:
  CycleColor(simData::DataStore& dataStore, simData::ObjectId id, size_t startIndex, double genInt=3.0);
protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  size_t colorIndex_;
};

/** Launches a missile */
class MissileLaunchPlatform : public DataGenerator
{
public:
  explicit MissileLaunchPlatform(simData::DataStore& dataStore);

  void setStartingLla(const simCore::Vec3& lla);
  void setAcceleration(const simCore::Vec3& acceleration);
  void setGravity(const simCore::Vec3& gravity);
  void setMaxSpeed(double speed);

  /** Retrieve the ID from the created entity */
  simData::ObjectId id() const;

  /** Create the beam with the given name */
  void create(const std::string& name);

protected:
  virtual void generate_(double scenarioTime);

private:
  simData::DataStore& dataStore_;
  simData::ObjectId id_;
  simCore::CoordinateConverter cc_;

  // Starting state
  simCore::Vec3 startLla_;
  osg::Vec3f acceleration_;
  osg::Vec3f gravity_;
  double maxSpeed_;

  // Current state
  double currentTime_;
  simCore::Vec3 currentLla_;
  osg::Vec3f currentVelocity_;
  bool accelerating_;
};


}

#endif /* SS_DATAGENERATORS_H */
