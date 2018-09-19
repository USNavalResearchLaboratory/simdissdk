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
#include "osgDB/ReadFile"
#include "simCore/Calc/Interpolation.h"
#include "simNotify/Notify.h"
#include "simData/LimitData.h"
#include "simVis/Platform.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"
#include "simVis/RocketBurnStorage.h"

static const std::string BURN_TEXTURE_FILE = "p.rgb";

namespace simCore {

/** Template helper method to interpolate between two simVis::RocketBurnStorage::Update instances */
template<>
simVis::RocketBurnStorage::Update linearInterpolate<simVis::RocketBurnStorage::Update>(const simVis::RocketBurnStorage::Update &prev, const simVis::RocketBurnStorage::Update &next, double mixFactor)
{
  simVis::RocketBurnStorage::Update returnValue;

  // blend colors
  returnValue.shapeData.color.r() = simCore::linearInterpolate(prev.shapeData.color.r(), next.shapeData.color.r(), mixFactor);
  returnValue.shapeData.color.g() = simCore::linearInterpolate(prev.shapeData.color.g(), next.shapeData.color.g(), mixFactor);
  returnValue.shapeData.color.b() = simCore::linearInterpolate(prev.shapeData.color.b(), next.shapeData.color.b(), mixFactor);
  returnValue.shapeData.color.a() = simCore::linearInterpolate(prev.shapeData.color.a(), next.shapeData.color.a(), mixFactor);

  // copy scale alpha from prev
  returnValue.shapeData.scaleAlpha = prev.shapeData.scaleAlpha;

  // linear interpolation
  returnValue.shapeData.length = simCore::linearInterpolate(prev.shapeData.length, next.shapeData.length, mixFactor);
  returnValue.shapeData.radiusFar = simCore::linearInterpolate(prev.shapeData.radiusFar, next.shapeData.radiusFar, mixFactor);
  returnValue.shapeData.radiusNear = simCore::linearInterpolate(prev.shapeData.radiusNear, next.shapeData.radiusNear, mixFactor);

  returnValue.pointingAngle.x() = simCore::linearInterpolate(prev.pointingAngle.x(), next.pointingAngle.x(), mixFactor);
  returnValue.pointingAngle.y() = simCore::linearInterpolate(prev.pointingAngle.y(), next.pointingAngle.y(), mixFactor);
  returnValue.pointingAngle.z() = simCore::linearInterpolate(prev.pointingAngle.z(), next.pointingAngle.z(), mixFactor);
  returnValue.positionOffset.x() = simCore::linearInterpolate(prev.positionOffset.x(), next.positionOffset.x(), mixFactor);
  returnValue.positionOffset.y() = simCore::linearInterpolate(prev.positionOffset.y(), next.positionOffset.y(), mixFactor);
  returnValue.positionOffset.z() = simCore::linearInterpolate(prev.positionOffset.z(), next.positionOffset.z(), mixFactor);

  return returnValue;
}
}

namespace simVis {

//----------------------------------------------------------------------------
/// Store all the data (indexed by time) for one rocket burn on one platform
class RocketBurnStorage::BurnUpdates
{
public:
  /// Default constructor
  BurnUpdates()
  {
  }

  /// add an update at the given time
  void addUpdate(double time, const Update &data);
  /// Limits size of the updates_ based on owner's data store prefs
  void applyDataLimiting(const simData::DataStore &dataStore, simData::ObjectId platId);

  /**
   * Retrieve the data for the given time.
   * Might use interpolation if there the time is not an exact match.
   */
  Update dataForTime(double time) const;

private:
  std::map<double, Update> updates_; ///< map from time to update data
};

//----------------------------------------------------------------------------
/// get platform removal notifications
class RocketBurnStorage::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  /** Constructor */
  explicit DataStoreListener(RocketBurnStorage &storage)
  : storage_(storage)
  {
  }

  /** Removes the burns from storage when the entity is removed from data store */
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    storage_.removeBurnsForPlatform_(removedId);
  }

private:
  RocketBurnStorage &storage_;
};

//----------------------------------------------------------------------------
RocketBurnStorage::RocketBurnStorage(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager)
  : scenarioManager_(scenarioManager),
  dataStore_(dataStore),
  texture_(NULL)
{
}

RocketBurnStorage::~RocketBurnStorage()
{
  dataStore_.removeListener(dataStoreListener_);
}

void RocketBurnStorage::addBurnData(simData::ObjectId platId, uint64_t burnId, double time, const Update &data)
{
  const BurnKey key(platId, burnId);

  // see if the visualization element already exists
  const std::map<BurnKey, osg::ref_ptr<simVis::RocketBurn> >::const_iterator i = allBurns_.find(key);
  if (i == allBurns_.end())
  {
    // create a new burn
    osg::observer_ptr<simVis::PlatformNode> hostPlat = scenarioManager_.find<simVis::PlatformNode>(platId);
    if (!hostPlat.valid())
    {
      SIM_DEBUG << "Rocket burn created for non-existent platform" << std::endl;
      return;
    }

    // lazy initialization of texture and other resources that are only needed if a rocketBurn is instantiated
    if (texture_ == NULL)
    {
      const std::string imageFile = simVis::Registry::instance()->findModelFile(BURN_TEXTURE_FILE);
      osg::ref_ptr<osg::Image> image;
      if (imageFile.empty())
        image = osgDB::readImageFile(BURN_TEXTURE_FILE); // Fall back on OSG
      else
        image = osgDB::readImageFile(imageFile);
      texture_ = new osg::Texture2D(image.get());
      simVis::fixTextureForGlCoreProfile(texture_.get());

      dataStoreListener_.reset(new DataStoreListener(*this));
      dataStore_.addListener(dataStoreListener_);
    }

    // associate the new burnId with the host platform id
    idsByPlatform_.insert(std::make_pair(platId, static_cast<unsigned int>(burnId)));
    // create the rocketBurn visual and add it to the map
    allBurns_.insert(std::make_pair(key, new simVis::RocketBurn(*hostPlat, *texture_)));
  }

  std::map<BurnKey, BurnUpdates>::iterator dataIterator = allData_.find(key);
  if (dataIterator == allData_.end())
  {
    dataIterator = allData_.insert(std::make_pair(key, BurnUpdates())).first;
  }

  // add the data, then apply limits
  dataIterator->second.addUpdate(time, data);

  dataIterator->second.applyDataLimiting(dataStore_, platId);
}

void RocketBurnStorage::update(double time)
{
  // for each key
  for (std::map<BurnKey, osg::ref_ptr<simVis::RocketBurn> >::const_iterator i = allBurns_.begin(); i != allBurns_.end(); ++i)
  {
    // get the data appropriate for time
    const Update& data = allData_[i->first].dataForTime(time);
    simVis::RocketBurn* rb = i->second.get();
    rb->update(data.shapeData);
    // update position only if rb data is valid (otherwise update has turned rb off)
    if (data.shapeData.length != 0)
      rb->setPositionOrientation(data.positionOffset, data.pointingAngle);
  }
}

void RocketBurnStorage::removeBurnsForPlatform_(simData::ObjectId removedId)
{
  // find all burnIds that associate with the platform
  const std::pair<RocketBurnIdByPlatform::iterator, RocketBurnIdByPlatform::iterator> ranges = idsByPlatform_.equal_range(removedId);
  for (RocketBurnIdByPlatform::const_iterator iter = ranges.first; iter != ranges.second; ++iter)
  {
    const BurnKey key(removedId, iter->second);
    allBurns_.erase(key);
    allData_.erase(key);
  }
  idsByPlatform_.erase(removedId);
}

//----------------------------------------------------------------------------
void RocketBurnStorage::BurnUpdates::addUpdate(double time, const Update &data)
{
  updates_[time] = data;
}

void RocketBurnStorage::BurnUpdates::applyDataLimiting(const simData::DataStore &dataStore, simData::ObjectId platId)
{
  simData::limitEntityData<Update>(updates_, dataStore, platId);
}

RocketBurnStorage::Update RocketBurnStorage::BurnUpdates::dataForTime(double time) const
{
  // No valid burn data before the current time
  Update invalidValue;
  invalidValue.shapeData.length = 0;

  // Find the value at-or-before the provided time
  std::map<double, Update>::const_iterator i = updates_.upper_bound(time);
  if (i == updates_.begin())
  {
    // if upper bound returns begin(), time has not advanced to the first update yet
    return invalidValue;
  }

  bool lastOne = false;
  if (i == updates_.end())
    lastOne = true;

  // iterator is pointing ahead of the update we want (possibly end())
  --i; // Now pointing at-or-before

  // If we have a duration, then we can just return this value as long as it's within time duration bounds
  if (i->second.duration >= 0)
  {
    // Make sure we're inside [first, first+duration] -- if not, then return an invalid value
    if (time > i->first + i->second.duration)
      return invalidValue;
    // We're within the time bounds, return the burn
    return i->second;
  }

  // There is no duration.

  // If its the only one or the last one, return it
  if ((updates_.size() == 1) || lastOne)
    return i->second;

  // We need to interpolate between the current flame and the next flame.
  // This is the not-special-case (sc) case, which...by now...is a special case.... (not common)
  Update retVal;
  if (!simCore::linearInterpolate(updates_, time, retVal))
    return invalidValue;
  return retVal;
}

//----------------------------------------------------------------------------

/** Default constructor */
RocketBurnStorage::Update::Update()
  : duration(-1)
{
}

/** Constructor specifying all struct values */
RocketBurnStorage::Update::Update(const simVis::RocketBurn::ShapeData &shape, const simCore::Vec3 &angle, const simCore::Vec3 &position, double updateDuration)
: shapeData(shape)
{
  pointingAngle.x() = angle.x();
  pointingAngle.y() = angle.y();
  pointingAngle.z() = angle.z();

  positionOffset.x() = position.x();
  positionOffset.y() = position.y();
  positionOffset.z() = position.z();

  duration = static_cast<float>(updateDuration);
}

} // namespace

