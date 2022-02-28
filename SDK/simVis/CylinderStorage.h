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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_CYLINDER_STORAGE_H
#define SIMVIS_CYLINDER_STORAGE_H

#include "simData/DataStore.h"
#include "simVis/CylinderGeode.h"

namespace simVis {

class ScenarioManager;

/**
 * Store everything related to cylinders for all platforms.
 *
 * Data is organized by platform and "cylinder id" (a platform can have multiple
 * cylinders attached).  A single cylinder object can have multiple data points over time.
 */
class SDKVIS_EXPORT CylinderStorage
{
public:
  /// one data point
  struct Update;

  /// Constructor
  CylinderStorage(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager);
  virtual ~CylinderStorage();

  /** Add Cylinder data for the given platform at a specified time, according to the given id */
  void addCylinderData(simData::ObjectId platId, uint64_t cylinderId, double time, const Update &data);

  /** Update all Cylinders on all platforms according to the given time */
  void update(double time);

  /// Remove all cylinders for the given platform
  void removeCylindersForPlatform(simData::ObjectId removedId);

private:
  /** typedef to simplify usage of multimap */
  typedef std::multimap<simData::ObjectId, unsigned int> CylinderIdByPlatform;
  /** All the data points for one Cylinder object */
  class CylinderUpdates;
  /** Listens for DataStore notifications (e.g. platform removal) */
  class DataStoreListener;
  /** Uniquely identifies a Cylinder object */
  struct CylinderKey;

  simVis::ScenarioManager &scenarioManager_;
  simData::DataStore &dataStore_;

  simData::DataStore::ListenerPtr dataStoreListener_;
  /** Multimap to keep track of cylinders that are associated with platform id */
  CylinderIdByPlatform idsByPlatform_;
  std::map<CylinderKey, osg::ref_ptr<simVis::CylinderGeode> > allCylinders_;
  std::map<CylinderKey, CylinderUpdates> allData_;
};

//----------------------------------------------------------------------------
/** Uniquely identifies a cylinder */
struct SDKVIS_EXPORT CylinderStorage::CylinderKey
{
  /** Unique ID for the platform host */
  simData::ObjectId platId;
  /** Unique ID for the cylinder */
  uint32_t cylinderId;

  /** Constructs a new CylinderKey */
  CylinderKey(simData::ObjectId plat, uint64_t cylinder)
  : platId(plat),
    cylinderId(cylinder)
  {
  }

  /** Less-than operator for sorting */
  bool operator<(const CylinderKey &rhs) const
  {
    if (platId < rhs.platId)
      return true;

    if (platId > rhs.platId)
      return false;

    // platId == rhs.platId
    return cylinderId < rhs.cylinderId;
  }
};

//----------------------------------------------------------------------------
/** Holds all data for one cylinder at one point in time*/
struct SDKVIS_EXPORT CylinderStorage::Update
{
  /** Holds all information about the shape of the Cylinder */
  simVis::CylinderGeode::ShapeData shapeData;
  /** Angle (rad) relative to the host platform */
  simCore::Vec3 pointingAngle;
  /** Position offset (m) from the host platform */
  simCore::Vec3 positionOffset;
  /** Duration of the Cylinder update data; -1 for infinite duration */
  double duration;

  /// Default constructor
  Update();
  /// Constructor that specifies each value in the struct explicitly
  Update(const simVis::CylinderGeode::ShapeData &shape, const simCore::Vec3 &angle, const simCore::Vec3 &position, double duration);
};

}

#endif

