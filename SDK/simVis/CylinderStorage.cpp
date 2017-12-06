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
#include "simCore/Calc/Interpolation.h"
#include "simNotify/Notify.h"
#include "simData/LimitData.h"
#include "simVis/Platform.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/CylinderStorage.h"


namespace simCore {

/** Template helper method to interpolate between two simVis::CylinderStorage::Update instances */
template<>
simVis::CylinderStorage::Update linearInterpolate<simVis::CylinderStorage::Update>(const simVis::CylinderStorage::Update &prev, const simVis::CylinderStorage::Update &next, double mixFactor)
{
  simVis::CylinderStorage::Update returnValue;

  // Blend colors
  returnValue.shapeData.colorNear.r() = simCore::linearInterpolate(prev.shapeData.colorNear.r(), next.shapeData.colorNear.r(), mixFactor);
  returnValue.shapeData.colorNear.g() = simCore::linearInterpolate(prev.shapeData.colorNear.g(), next.shapeData.colorNear.g(), mixFactor);
  returnValue.shapeData.colorNear.b() = simCore::linearInterpolate(prev.shapeData.colorNear.b(), next.shapeData.colorNear.b(), mixFactor);
  returnValue.shapeData.colorNear.a() = simCore::linearInterpolate(prev.shapeData.colorNear.a(), next.shapeData.colorNear.a(), mixFactor);

  // Blend colors
  returnValue.shapeData.colorFar.r() = simCore::linearInterpolate(prev.shapeData.colorFar.r(), next.shapeData.colorFar.r(), mixFactor);
  returnValue.shapeData.colorFar.g() = simCore::linearInterpolate(prev.shapeData.colorFar.g(), next.shapeData.colorFar.g(), mixFactor);
  returnValue.shapeData.colorFar.b() = simCore::linearInterpolate(prev.shapeData.colorFar.b(), next.shapeData.colorFar.b(), mixFactor);
  returnValue.shapeData.colorFar.a() = simCore::linearInterpolate(prev.shapeData.colorFar.a(), next.shapeData.colorFar.a(), mixFactor);

  // Linear interpolation
  returnValue.shapeData.length = simCore::linearInterpolate(prev.shapeData.length, next.shapeData.length, mixFactor);
  returnValue.shapeData.radiusFar = simCore::linearInterpolate(prev.shapeData.radiusFar, next.shapeData.radiusFar, mixFactor);
  returnValue.shapeData.radiusNear = simCore::linearInterpolate(prev.shapeData.radiusNear, next.shapeData.radiusNear, mixFactor);

  returnValue.pointingAngle = simCore::linearInterpolate(prev.pointingAngle, next.pointingAngle, mixFactor);
  returnValue.positionOffset = simCore::linearInterpolate(prev.positionOffset, next.positionOffset, mixFactor);

  return returnValue;
}
}

namespace simVis {

//----------------------------------------------------------------------------
/** Store all the data (indexed by time) for one cylinder on one platform */
class CylinderStorage::CylinderUpdates
{
public:
  /// Default constructor
  CylinderUpdates()
  {
  }

  /** Add an Update at the given time */
  void addUpdate(double time, const Update &data);
  /** Limits size of the updates_ based on owner's data store prefs */
  void applyDataLimiting(const simData::DataStore &dataStore, simData::ObjectId platId);

  /**
   * Retrieve the data for the given time.
   * Might use interpolation if the time is not an exact match.
   */
  Update dataForTime(double time) const;

private:
  /** Map from time to update data */
  std::map<double, Update> updates_;
};

//----------------------------------------------------------------------------

/** Listens for DataStore notifications (e.g. platform removal) */
class CylinderStorage::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  /** Constructor */
  explicit DataStoreListener(CylinderStorage &storage)
  : storage_(storage)
  {
  }

  /** Removes the Cylinders from storage when the entity is removed from data store */
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    storage_.removeCylindersForPlatform_(removedId);
  }

private:
  CylinderStorage &storage_;
};
//----------------------------------------------------------------------------

CylinderStorage::CylinderStorage(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager)
  : scenarioManager_(scenarioManager),
  dataStore_(dataStore)
{
}

CylinderStorage::~CylinderStorage()
{
  dataStore_.removeListener(dataStoreListener_);
}

void CylinderStorage::addCylinderData(simData::ObjectId platId, uint64_t cylinderId, double time, const Update &data)
{
  const CylinderKey key(platId, cylinderId);

  // See if the visualization element already exists
  const std::map<CylinderKey, osg::ref_ptr<simVis::CylinderGeode> >::const_iterator i = allCylinders_.find(key);
  if (i == allCylinders_.end())
  {
    // Create a new cylinder
    osg::observer_ptr<simVis::PlatformNode> hostPlat = scenarioManager_.find<simVis::PlatformNode>(platId);
    if (!hostPlat.valid())
    {
      SIM_DEBUG << "Cylinder created for non-existent platform" << std::endl;
      return;
    }

    if (dataStoreListener_ == NULL)
    {
      // Listens for changes to the dataStore
      dataStoreListener_.reset(new DataStoreListener(*this));
      dataStore_.addListener(dataStoreListener_);
    }

    // Associate the new cylinderId with the host platform id
    idsByPlatform_.insert(std::make_pair(platId, static_cast<unsigned int>(cylinderId)));
    // Create the Cylinder visual and add it to the map
    allCylinders_.insert(std::make_pair(key, new simVis::CylinderGeode(*hostPlat)));
  }

  std::map<CylinderKey, CylinderUpdates>::iterator dataIterator = allData_.find(key);
  if (dataIterator == allData_.end())
  {
    dataIterator = allData_.insert(std::make_pair(key, CylinderUpdates())).first;
  }

  // Add the data, then apply limits
  dataIterator->second.addUpdate(time, data);

  dataIterator->second.applyDataLimiting(dataStore_, platId);
}

void CylinderStorage::update(double time)
{
  // For each key
  for (std::map<CylinderKey, osg::ref_ptr<simVis::CylinderGeode> >::const_iterator i = allCylinders_.begin(); i != allCylinders_.end(); ++i)
  {
    // Get the data appropriate for time
    const Update& data = allData_[i->first].dataForTime(time);
    simVis::CylinderGeode* rb = i->second.get();
    rb->update(data.shapeData);
    // Update position only if rb data is valid (otherwise update has turned rb off)
    if (data.shapeData.length != 0)
      rb->setPositionOrientation(data.positionOffset, data.pointingAngle);
  }
}

void CylinderStorage::removeCylindersForPlatform_(simData::ObjectId removedId)
{
  // Find all cylinderIds that associate with the platform
  const std::pair<CylinderIdByPlatform::iterator, CylinderIdByPlatform::iterator> ranges = idsByPlatform_.equal_range(removedId);
  for (CylinderIdByPlatform::const_iterator iter = ranges.first; iter != ranges.second; ++iter)
  {
    const CylinderKey key(removedId, iter->second);
    allCylinders_.erase(key);
    allData_.erase(key);
  }
  idsByPlatform_.erase(removedId);
}

//----------------------------------------------------------------------------
void CylinderStorage::CylinderUpdates::addUpdate(double time, const Update &data)
{
  updates_[time] = data;
}

void CylinderStorage::CylinderUpdates::applyDataLimiting(const simData::DataStore &dataStore, simData::ObjectId platId)
{
  simData::limitEntityData<Update>(updates_, dataStore, platId);
}

CylinderStorage::Update CylinderStorage::CylinderUpdates::dataForTime(double time) const
{
  // No valid cylinder data before the current time
  Update invalidValue;
  invalidValue.shapeData.length = 0;

  // Find the value at-or-before the provided time
  std::map<double, Update>::const_iterator i = updates_.upper_bound(time);
  if (i == updates_.begin())
  {
    // If upper bound returns begin(), time has not advanced to the first update yet
    return invalidValue;
  }

  bool lastOne = false;
  if (i == updates_.end())
    lastOne = true;

  // Iterator is pointing ahead of the update we want (possibly end())
  --i; // Now pointing at-or-before

  // Per documentation the duration field is ignored

  // If its the only one or the last one, return it
  if ((updates_.size() == 1) || lastOne)
    return i->second;

  // We need to interpolate between the current cylinder and the next.
  Update retVal;
  if (!simCore::linearInterpolate(updates_, time, retVal))
    return invalidValue;
  return retVal;
}

//----------------------------------------------------------------------------

/** Default constructor */
CylinderStorage::Update::Update()
  : duration(-1)
{
}

/** Constructor specifying all struct values */
CylinderStorage::Update::Update(const simVis::CylinderGeode::ShapeData &shape, const simCore::Vec3 &angle, const simCore::Vec3 &position, double duration)
: shapeData(shape),
  pointingAngle(angle),
  positionOffset(position),
  duration(duration)
{
}

} // Namespace

