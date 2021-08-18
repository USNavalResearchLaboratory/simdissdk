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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <sstream>
#include "osgGA/GUIEventHandler"
#include "simCore/Calc/Angle.h"
#include "simCore/Time/Utils.h"
#include "simVis/Scenario.h"
#include "simUtil/ExampleResources.h"
#include "DataGenerators.h"
#include "DataEngine.h"

namespace SimpleServer {

/// Interval (seconds) at which to generate data points
static const double IDLE_TIMEOUT = 0.05; // 20 hz
/// Interval (seconds) between new platform creation
static const double NEW_PLATFORM_TIMEOUT = 1.0;
/// Number of rotating platforms to support
static const size_t NUM_ROTATING_PLATFORMS = 10;

/** Ties into the FRAME event to call generateData_(); throttled by IDLE_TIMEOUT */
class DataEngine::GenerateDataTimer : public osgGA::GUIEventHandler
{
public:
  explicit GenerateDataTimer(DataEngine& engine)
    : engine_(engine),
      lastIdle_(0.0)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      // Simulate a callback that is called at a throttled rate
      const double nowTime = simCore::getSystemTime();
      if (nowTime > lastIdle_ + IDLE_TIMEOUT)
      {
        lastIdle_ = nowTime;
        engine_.generateData_();
      }
    }
    return false;
  }

private:
  DataEngine& engine_;
  double lastIdle_;
};

//////////////////////////////////////////////////////////////////

DataEngine::DataEngine(simData::DataStore& dataStore, simVis::ScenarioManager& scenario)
  : dataStore_(dataStore),
    scenario_(scenario),
    lastCreateTime_(0.0)
{
  // Our times will all be relative to 1970
  {
    simData::DataStore::Transaction txn;
    simData::ScenarioProperties* props = dataStore_.mutable_scenarioProperties(&txn);
    props->set_referenceyear(1970);
    props->mutable_classification()->set_label("UNCLASSIFIED");
    props->mutable_classification()->set_fontcolor(0x00ff0080);
    props->set_description("Simple Server Data Engine");
    txn.complete(&props);
  }

  generateDataTimer_ = new GenerateDataTimer(*this);
  scenario_.addEventCallback(generateDataTimer_);
}

DataEngine::~DataEngine()
{
  scenario_.removeEventCallback(generateDataTimer_);
}

void DataEngine::generateData_()
{
  const double nowTime = simCore::getSystemTime();

  // Create new platforms
  if (platforms_.size() < NUM_ROTATING_PLATFORMS && nowTime > lastCreateTime_ + NEW_PLATFORM_TIMEOUT)
  {
    lastCreateTime_ = nowTime;
    CirclingPlatform* newPlatform = new CirclingPlatform(dataStore_);
    newPlatform->setCenterLla(simCore::Vec3(22.0 * simCore::DEG2RAD, -159 * simCore::DEG2RAD, 100.0));
    newPlatform->setXyzOffset(simCore::Vec3(platforms_.size() * 100, platforms_.size() * 100, 0.0));
    newPlatform->setRange(100.0);
    std::stringstream name;
    name << platforms_.size() + 1 << " p-3c_orion_nrl";
    newPlatform->create(name.str());
    platforms_.push_back(newPlatform);
    generators_.push_back(newPlatform);

    // Create a cycling animated line between entities 4 and 3/5
    if (platforms_.size() == 5)
    {
      CyclingAnimatedLine* line = new CyclingAnimatedLine(scenario_);
      line->setAnchor(platforms_[3]->id());
      line->addTarget(platforms_[2]->id());
      line->addTarget(platforms_[4]->id());
      generators_.push_back(line);
    }

    // Toggle the 2nd platform icon between normal and a 2D icon
    if (platforms_.size() == 2)
      generators_.push_back(new ToggleIcon(dataStore_, platforms_[1]->id(), EXAMPLE_IMAGE_ICON));

    // Add the platform to the target beam list if needed
    if (targetBeam_.valid())
      targetBeam_->addTarget(platforms_.back()->id());

    // Add a target beam to platform 4 (index 3); add a rotating beam to others
    simData::ObjectId beamId = 0;
    if (platforms_.size() == 4)
    {
      targetBeam_ = new CyclingTargetBeam(dataStore_);
      targetBeam_->create(platforms_[3]->id(), "Target Beam");
      for (size_t k = 0; k < 3; ++k)
        targetBeam_->addTarget(platforms_[k]->id());
      beamId = targetBeam_->id();
      generators_.push_back(targetBeam_);
    }
    else
    {
      std::stringstream beamName;
      beamName << "Beam " << platforms_.size() + 1;
      RotatingBeam* beam = new RotatingBeam(dataStore_);
      beam->create(platforms_.back()->id(), beamName.str());
      beamId = beam->id();
      generators_.push_back(beam);
    }
    // For the beam, toggle draw every few seconds and cycle the colors
    generators_.push_back(new ToggleDrawState(dataStore_, beamId, 5.0));
    // Note that the color cycle multiplies by a prime number in an attempt to better spread colors out
    generators_.push_back(new CycleColor(dataStore_, beamId, platforms_.size() * 13, 2.0));

    // Create a gate for each beam
    std::stringstream gateName;
    gateName << "Gate " << platforms_.size() + 1;
    RotatingGate* gate = new RotatingGate(dataStore_);
    gate->create(beamId, gateName.str());
    generators_.push_back(gate);
    // For the gate, toggle draw every few seconds and cycle the colors
    generators_.push_back(new ToggleDrawState(dataStore_, gate->id(), 5.0));
    generators_.push_back(new CycleColor(dataStore_, gate->id(), platforms_.size() * 13 + 5, 2.0));

    // Add a missile launch after the second platform is created
    if (platforms_.size() == 2)
    {
      MissileLaunchPlatform* missile = new MissileLaunchPlatform(dataStore_);
      missile->create("Missile");
      // Set some parameters on the track history
      simData::DataStore::Transaction txn;
      simData::PlatformPrefs* missilePrefs = dataStore_.mutable_platformPrefs(missile->id(), &txn);
      missilePrefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_POINT);
      missilePrefs->mutable_trackprefs()->set_linewidth(2.0);
      missilePrefs->mutable_trackprefs()->set_tracklength(6000);
      missilePrefs->mutable_trackprefs()->set_trackoverridecolor(0x00ff00ff);
      missilePrefs->mutable_trackprefs()->set_usetrackoverridecolor(true);
      missilePrefs->mutable_commonprefs()->clear_datalimittime();
      missilePrefs->mutable_commonprefs()->set_datalimitpoints(2000);
      missilePrefs->set_lighted(true);
      missilePrefs->set_brightness(64);
      txn.complete(&missilePrefs);
      generators_.push_back(missile);
    }
  }

  // Activate all registered generators
  for (std::vector<osg::ref_ptr<DataGenerator> >::const_iterator i = generators_.begin(); i != generators_.end(); ++i)
    (*i)->idle(nowTime);
}

}
