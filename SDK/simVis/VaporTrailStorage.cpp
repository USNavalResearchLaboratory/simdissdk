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
#include <map>
#include "simNotify/Notify.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/VaporTrailStorage.h"

namespace simVis {

//----------------------------------------------------------------------------
/// get platform removal notifications
class VaporTrailStorage::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  /** Constructor */
  explicit DataStoreListener(VaporTrailStorage &storage)
  : storage_(storage)
  {
  }

  /** Removes the vaporTrails from storage when the entity is removed from data store */
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    storage_.removeVaporTrailsForPlatform_(removedId);
  }

private:
  VaporTrailStorage &storage_;
};

//----------------------------------------------------------------------------
VaporTrailStorage::VaporTrailStorage(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager)
: scenarioManager_(scenarioManager),
  dataStore_(dataStore)
{
}

VaporTrailStorage::~VaporTrailStorage()
{
  dataStore_.removeListener(dataStoreListener_);

  // remove the vaporTrails from the scene graph
  for (std::map<Key, osg::ref_ptr<simVis::VaporTrail> >::iterator it = vaporTrailsByKey_.begin(); it != vaporTrailsByKey_.end(); ++it)
  {
    it->second = NULL;
  }
  vaporTrailsByKey_.clear();
  idsByPlatform_.clear();
}

int VaporTrailStorage::addVaporTrail(simData::ObjectId platId, unsigned int id, const VaporTrail::VaporTrailData &vaporTrailData, const VaporTrail::VaporPuffData &vaporPuffData, const std::vector< osg::ref_ptr<osg::Texture2D> >& textures)
{
  const Key key(platId, id);

  std::map<Key, osg::ref_ptr<simVis::VaporTrail> >::iterator i = vaporTrailsByKey_.find(key);

  // if a vaporTrail with this key already exists, delete and recreate new
  if (i != vaporTrailsByKey_.end())
  {
    vaporTrailsByKey_.erase(key);

    // remove from multimap
    std::pair<VaporTrailIdByPlatform::iterator, VaporTrailIdByPlatform::iterator> ranges;
    ranges = idsByPlatform_.equal_range(platId);
    VaporTrailIdByPlatform::iterator iter;
    for (iter = ranges.first; iter != ranges.second; ++iter)
    {
      if (iter->second == id)
      {
        idsByPlatform_.erase(iter);
        break;
      }
    }
  }

  // find the host platform
  osg::observer_ptr<simVis::PlatformNode> hostPlat = scenarioManager_.find<simVis::PlatformNode>(platId);
  if (!hostPlat.valid())
  {
    SIM_DEBUG << "Vapor Trail created for non-existent platform" << std::endl;
    return 1;
  }

  if (dataStoreListener_ == NULL)
  {
    dataStoreListener_.reset(new DataStoreListener(*this));
    dataStore_.addListener(dataStoreListener_);
  }

  // create a new VaporTrail
  osg::ref_ptr<simVis::VaporTrail> newTrail = new simVis::VaporTrail(dataStore_, *hostPlat, vaporTrailData, vaporPuffData, textures);
  idsByPlatform_.insert(std::make_pair(platId, id));
  vaporTrailsByKey_[key] = newTrail;
  return 0;
}

void VaporTrailStorage::update(double time)
{
  // for each key
  for (std::map<Key, osg::ref_ptr<simVis::VaporTrail> >::const_iterator i = vaporTrailsByKey_.begin(); i != vaporTrailsByKey_.end(); ++i)
  {
    // apply it; update does data limiting to prevent spikes when time jumps in file mode
    i->second->update(time);
  }
}

void VaporTrailStorage::removeVaporTrailsForPlatform_(simData::ObjectId removedId)
{
  // find all vapor trail keys that associate with the platform
  std::pair<VaporTrailIdByPlatform::iterator, VaporTrailIdByPlatform::iterator> ranges;
  ranges = idsByPlatform_.equal_range(removedId);
  VaporTrailIdByPlatform::iterator iter;
  for (iter = ranges.first; iter != ranges.second; ++iter)
  {
    vaporTrailsByKey_.erase(Key(removedId, iter->second));
  }
  idsByPlatform_.erase(removedId);
}

} // namespace

