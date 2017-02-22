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
#ifndef SIMVIS_VAPOR_TRAIL_STORAGE_H
#define SIMVIS_VAPOR_TRAIL_STORAGE_H

#include "simData/DataStore.h"
#include "simVis/VaporTrail.h"

namespace simVis {

class ScenarioManager;

/**
 * Store everything related to vapor trails for all platforms.
 * Data is organized by platform and "vaporTrail id" (a platform can have multiple vaporTrails).
 */
class SDKVIS_EXPORT VaporTrailStorage
{
public:
  /**
  * Constructor.  Adds to the scene.
  * @param dataStore the dataStore that provides platform info.
  * @param scenarioManager the scenarioManager (scene graph) to which to attach vapor trails.
  */
  VaporTrailStorage(simData::DataStore &dataStore, ScenarioManager &scenarioManager);
  virtual ~VaporTrailStorage();

  /**
  * Add vaporTrail for the given platform, according to the given id, with given shape specification.
  * @param platId platform the vapor trail is connected to.
  * @param id unique id for vapor trail on the specified platform.
  * @param vaporTrailData data used to construct the vapor trail.
  * @param vaporPuffData data used to specify the vapor puff.
  * @param textures list of textures to use for alternating puffs.
  * @return 0 on success, non-zero on failure.
  */
  int addVaporTrail(simData::ObjectId platId, unsigned int id, const VaporTrail::VaporTrailData &vaporTrailData, const VaporTrail::VaporPuffData &vaporPuffData, const std::vector< osg::ref_ptr<osg::Texture2D> >& textures);

  /**
  * update all vaporTrails on all platforms according to the given time
  * @param time new time to update all existing vapor trails
  */
  void update(double time);

private: // methods
  /**
  * remove all vaporTrails associated with a deleted platform
  * @param removedId id of platform that will be removed
  */
  void removeVaporTrailsForPlatform_(simData::ObjectId removedId);

private:
  typedef std::multimap<simData::ObjectId, unsigned int> VaporTrailIdByPlatform;

  struct Key;
  class DataStoreListener;

  ScenarioManager &scenarioManager_;
  simData::DataStore &dataStore_;
  simData::DataStore::ListenerPtr dataStoreListener_;
  VaporTrailIdByPlatform idsByPlatform_;
  std::map<Key, osg::ref_ptr<VaporTrail> > vaporTrailsByKey_;
};


/// Uniquely identify a vaporTrail
struct SDKVIS_EXPORT VaporTrailStorage::Key
{
  /** Unique ID for the platform host */
  simData::ObjectId platId;
  /** Unique ID for one vaporTrail; ID is unique only to the platform to which it belongs */
  unsigned int vaporTrailId;

  /** Constructs a new key */
  Key(simData::ObjectId plat, unsigned int id)
    : platId(plat),
    vaporTrailId(id)
  {
  }

  /** Less-than operator for sorting */
  bool operator<(const Key &rhs) const
  {
    if (platId < rhs.platId)
      return true;
    return (platId > rhs.platId) ? false : (vaporTrailId < rhs.vaporTrailId);
  }
};

}

#endif
