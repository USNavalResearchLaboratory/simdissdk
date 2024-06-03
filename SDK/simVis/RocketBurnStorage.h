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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_ROCKET_BURN_STORAGE_H
#define SIMVIS_ROCKET_BURN_STORAGE_H

#include "simData/DataStore.h"
#include "simVis/RocketBurn.h"

namespace simVis {

class ScenarioManager;

/**
 * Store everything related to rocket burns for all platforms.
 *
 * Data is organized by platform and "burn id" (a platform can have multiple
 * burns attached).  A single burn object can have multiple data points over time.
 */
class SDKVIS_EXPORT RocketBurnStorage
{
public: // types
  /// one data point
  struct Update;

public:
  /// Constructor
  RocketBurnStorage(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager);
  virtual ~RocketBurnStorage();

  /// add burn data for the given platform, according to the given id
  void addBurnData(simData::ObjectId platId, uint64_t burnId, double time, const Update &data);

  /// update all burns on all platforms according to the given time
  void update(double time);

  /// remove burn data for the given platform, according to the given id
  void removeBurnsForPlatform(simData::ObjectId removedId);

private:
  typedef std::multimap<simData::ObjectId, unsigned int> RocketBurnIdByPlatform; ///< typedef to simplify usage of multimap
  class BurnUpdates; ///< all the data points for one burn object
  class DataStoreListener; ///< get platform removal notifications
  struct BurnKey; ///< uniquely identify a burn object

  simVis::ScenarioManager &scenarioManager_;
  simData::DataStore &dataStore_;

  /// Holds onto the billboard texture that is used for all rocket burns
  osg::ref_ptr<osg::Texture2D> texture_;

  simData::DataStore::ListenerPtr dataStoreListener_;
  RocketBurnIdByPlatform idsByPlatform_;   ///< multimap to keep track of rocket burns that are associated with platform id
  std::map<BurnKey, osg::ref_ptr<simVis::RocketBurn> > allBurns_; ///< visualization element
  std::map<BurnKey, BurnUpdates> allData_; ///< data to feed the visualization
};

//----------------------------------------------------------------------------
/// Uniquely identify a rocket burn
struct SDKVIS_EXPORT RocketBurnStorage::BurnKey
{
  /** Unique ID for the platform host */
  simData::ObjectId platId;
  /** Unique ID for the burn */
  uint32_t burnId;

  /** Constructs a new BurnKey */
  BurnKey(simData::ObjectId plat, uint64_t burn)
  : platId(plat),
    burnId(burn)
  {
  }

  /** Less-than operator for sorting */
  bool operator<(const BurnKey &rhs) const
  {
    if (platId < rhs.platId)
      return true;

    if (platId > rhs.platId)
      return false;

    // platId == rhs.platId
    return burnId < rhs.burnId;
  }
};

//----------------------------------------------------------------------------
/// one data point for a rocket burn
struct SDKVIS_EXPORT RocketBurnStorage::Update
{
  simVis::RocketBurn::ShapeData shapeData; ///< shape of the burn (used in visualization)
  osg::Vec3f pointingAngle; ///< angle (rad) relative to the host platform
  osg::Vec3f positionOffset; ///< position offset (m) from the host platform
  float duration; ///< duration of the burn update; -1 for infinite duration

  /// Default constructor
  Update();
  /// Constructor that specifies each value in the struct explicitly
  Update(const simVis::RocketBurn::ShapeData &shape, const simCore::Vec3 &angle, const simCore::Vec3 &position, double updateDuration);
};

}

#endif

