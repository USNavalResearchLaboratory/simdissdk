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
#include "osg/Billboard"
#include "osg/CullFace"
#include "osg/Depth"
#include "osg/Geometry"
#include "osg/Texture2D"
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"

#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Interpolation.h"
#include "simData/LimitData.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/OverrideColor.h"
#include "simVis/Platform.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/VaporTrail.h"

namespace simVis
{

VaporTrail::VaporTrailData::VaporTrailData()
  : startTime(10.0),
  endTime(20.0),
  numRadiiFromPreviousSmoke(1.5),
  metersBehindCurrentPosition(5.0),
  isWake(false)
{}

VaporTrail::VaporPuffData::VaporPuffData()
  : initialRadiusM(2.0),
  radiusExpansionRate(1.0),
  fadeTimeS(20.0)
{}

//////////////////////////////////////////////////////////////////////////

VaporTrail::VaporTrail(const simData::DataStore& dataStore, PlatformNode& hostPlatform, const VaporTrailData& vaporTrailData, const VaporPuffData& vaporPuffData, const std::vector< osg::ref_ptr<osg::Texture2D> >& textures)
  : dataStore_(dataStore),
    hostPlatform_(&hostPlatform),
    vaporTrailData_(vaporTrailData),
    vaporPuffData_(vaporPuffData),
    textureCounter_(0)
{
  vaporTrailGroup_ = new osg::Group();
  vaporTrailGroup_->setNodeMask(simVis::DISPLAY_MASK_NONE);

  osg::StateSet* groupState = vaporTrailGroup_->getOrCreateStateSet();
  // Vapor/Wake trails draw in the Two Pass Alpha render bin
  groupState->setRenderBinDetails(BIN_VAPOR_TRAIL, BIN_TWO_PASS_ALPHA);
  // Must be able to blend or the graphics will look awful
  groupState->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

  hostPlatform_->addChild(vaporTrailGroup_);

  // process the supplied texture(s)
  processTextures_(textures);

  // create locator to track our host, and generate our offset position
  locator_ = new Locator(hostPlatform_->getLocator());
  // add offset for wake to prevent wake from getting wet
  const double altOffset = (vaporTrailData_.isWake ? 0.1 : 0.0);
  locator_->setLocalOffsets(simCore::Vec3(0.0, -vaporTrailData_.metersBehindCurrentPosition, altOffset), simCore::Vec3());

  OverheadMode::enableGeometryFlattening(true, vaporTrailGroup_.get());
}

VaporTrail::~VaporTrail()
{
  puffs_.clear();
  recyclePuffs_.clear();
  if (hostPlatform_.valid())
    hostPlatform_->removeChild(vaporTrailGroup_);
  vaporTrailGroup_ = NULL;
  textures_.clear();
  locator_ = NULL;
}

void VaporTrail::update(double time)
{
  if (textures_.empty() ||
    vaporTrailData_.numRadiiFromPreviousSmoke <= 0 ||
    vaporPuffData_.initialRadiusM <= 0)
    return;

  // turn the trail off if update time is before start
  if (time < vaporTrailData_.startTime)
  {
    vaporTrailGroup_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }
  vaporTrailGroup_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // purge all puffs with time > current time; update all other puffs
  if (!puffs_.empty())
  {
    // if everything is greater than time so clear all
    if (puffs_.front()->time() > time)
    {
      dropPuffsFromFront_(puffs_.size());
    }
    // the last time crosses the time so need to trim
    else if (puffs_.back()->time() > time)
    {
      Puffs::reverse_iterator it = puffs_.rbegin();
      for (; it != puffs_.rend(); ++it)
      {
        if ((*it)->time() > time)
          (*it)->clear();
        else
          break;
      }
      // need a forward iterator for erase; distance will return a negative value
      Puffs::iterator startIt = puffs_.begin() + (puffs_.size() + std::distance(it, puffs_.rbegin()));
      recyclePuffs_.insert(recyclePuffs_.end(), startIt, puffs_.end());
      puffs_.erase(startIt, puffs_.end());
    }
  }

  // if this vapor trail has a non-trivial end time, do not add new puffs after that time
  if (time <= vaporTrailData_.endTime || vaporTrailData_.endTime == vaporTrailData_.startTime)
  {
    // add new puffs when required
    addNewPuffs_(time);
  }

  // Need to update all puffs
  for (Puffs::const_iterator it = puffs_.begin(); it != puffs_.end(); ++it)
    (*it)->update(time, vaporPuffData_);
}

void VaporTrail::getLimits_(unsigned int& pointLimit, double& timeLimit)
{
  if (!hostPlatform_.valid())
  {
    pointLimit = 1000;
    timeLimit = -1.0;
    return;
  }

  simData::DataStore::Transaction txn;
  const simData::CommonPrefs *commonPrefs = dataStore_.commonPrefs(hostPlatform_->getId(), &txn);
  if (commonPrefs)
  {
    pointLimit = commonPrefs->datalimitpoints();
    timeLimit = commonPrefs->datalimittime();
  }
  else
  {
    pointLimit = 1000;
    timeLimit = -1.0;
  }
}

void VaporTrail::dropPuffsFromFront_(size_t dropAmount)
{
  if (dropAmount == 0)
    return;

  for (Puffs::const_iterator it = puffs_.begin(); it != puffs_.begin() + dropAmount; ++it)
    (*it)->clear();
  recyclePuffs_.insert(recyclePuffs_.end(), puffs_.begin(), puffs_.begin() + dropAmount);
  puffs_.erase(puffs_.begin(), puffs_.begin() + dropAmount);
}

unsigned int VaporTrail::applyDataLimiting_(unsigned int puffsToAdd, double time, double prevPuffTime)
{
  // Time must move forward
  assert(time >= prevPuffTime);

  // Get Limits
  unsigned int pointLimit;
  double timeLimit;
  getLimits_(pointLimit, timeLimit);

  // Number of active puffs
  const size_t activePuffs = puffs_.size();
  // The number of puffs to add after limiting
  size_t addAmount = puffsToAdd;
  // The number of active puffs to drop
  size_t dropAmount = 0;

  // Calculate number to add and drop as a result of the points limit
  if (pointLimit > 0)
  {
    if (addAmount > pointLimit)
    {
      addAmount = pointLimit;
      dropAmount = activePuffs;
    }
    else
    {
      if (activePuffs + addAmount > pointLimit)
        dropAmount = activePuffs + addAmount - pointLimit;
    }
  }

  // Calculate number to add and drop as a result of the time limit
  if ((activePuffs > 0) && (timeLimit > 0))
  {
    double earliestTime = time - timeLimit;
    if (earliestTime < vaporTrailData_.startTime)
      earliestTime = vaporTrailData_.startTime;
    if (puffs_.front()->time() > earliestTime)
    {
      // Keep all from a time point of view
    }
    else if (prevPuffTime < earliestTime)
    {
      // drop all
      dropAmount = activePuffs;
      // A file seek might have caused a big jump so might need to limit the points added due to time
      double timeJump = time - prevPuffTime;
      // going backwards should have already been handled by update()
      assert(timeJump >= 0.0);
      if (puffsToAdd > 0)
      {
        if ((static_cast<double>(addAmount) / static_cast<double>(puffsToAdd)) * timeJump > timeLimit)
        {
          // if timeJump is zero the above if statement would fail, so already protected against division by zero
          addAmount = static_cast<size_t>((timeLimit / timeJump) * puffsToAdd);
        }
      }
    }
    else
    {
      // might drop some; if not already dropped by points
      if (dropAmount < activePuffs)
      {
        for (/*Intentional*/; dropAmount < activePuffs; ++dropAmount)
        {
          if (puffs_[dropAmount]->time() >= earliestTime)
            break;
        }
      }
    }
  }

  // apply limits before adding to prevent spikes
  dropPuffsFromFront_(dropAmount);
  return addAmount;
}

int VaporTrail::addFirstPuff_()
{
  const simData::PlatformUpdateSlice* platformUpdateSlice = dataStore_.platformUpdateSlice(hostPlatform_->getId());
  if (!platformUpdateSlice)
  {
    assert(0);
    return 2;
  }
  const double time = vaporTrailData_.startTime;
  const simData::PlatformUpdateSlice::Iterator platformIter = platformUpdateSlice->upper_bound(time);
  if (!platformIter.hasPrevious())
  {
    // cannot process this since there is no platform position at or before time; possibly the platform point was removed by data limiting.
    return 1;
  }
  // last update at or before time:
  const simData::PlatformUpdate* platformUpdate = platformIter.peekPrevious();

  // interpolation may be required
  simData::PlatformUpdate interpolatedPlatformUpdate;
  simData::Interpolator* li = dataStore_.interpolator();
  if (platformUpdate->time() != time && li != NULL && platformIter.hasNext())
  {
    // defn of upper_bound previous()
    assert(platformUpdate->time() < time);
    // defn of upper_bound next()
    assert(platformIter.peekNext()->time() > time);
    li->interpolate(time, *platformUpdate, *(platformIter.peekNext()), &interpolatedPlatformUpdate);
    platformUpdate = &interpolatedPlatformUpdate;
  }

  const simCore::Coordinate coord(
    simCore::COORD_SYS_ECEF,
    simCore::Vec3(platformUpdate->x(), platformUpdate->y(), platformUpdate->z()),
    simCore::Vec3(platformUpdate->psi(), platformUpdate->theta(), platformUpdate->phi()),
    simCore::Vec3(platformUpdate->vx(), platformUpdate->vy(), platformUpdate->vz()));

  osg::ref_ptr<Locator> startTimeLocator = new Locator(locator_->getSRS());
  startTimeLocator->setCoordinate(coord, platformUpdate->time(), locator_->getEciRefTime());
  // add offset for wake to prevent wake from getting wet
  const double altOffset = (vaporTrailData_.isWake ? 0.1 : 0.0);
  startTimeLocator->setLocalOffsets(simCore::Vec3(0.0, -vaporTrailData_.metersBehindCurrentPosition, altOffset), simCore::Vec3());

  simCore::Vec3 hostOffsetPosition;
  startTimeLocator->getLocatorPosition(&hostOffsetPosition);
  addPuff_(hostOffsetPosition, platformUpdate->time());

  return 0;
}

void VaporTrail::addNewPuffs_(double time)
{
  if (!hostPlatform_->isActive() || textures_.empty())
    return;

  if (puffs_.empty())
  {
    // guaranteed by caller: VaporTrail::update
    assert(time >= vaporTrailData_.startTime);

    // if time jumped over trail's start time, we need to add the start time puff and all succeeding puffs up to current time
    if ((time == vaporTrailData_.startTime) || (0 != addFirstPuff_()))
    {
      // there are no previous puffs, just add a puff for current time
      simCore::Vec3 hostOffsetPosition;
      locator_->getLocatorPosition(&hostOffsetPosition);
      addPuff_(hostOffsetPosition, time);
      return;
    }
    // we successfully added the puff for start time, now fall through to add all succeeding puffs up to current time
  }
  else if (puffs_.back()->time() == time)
  {
    // do not re-create if there is already a puff at this time, but check for changes in data limits
    applyDataLimiting_(0, time, puffs_.back()->time());
    return;
  }

  if (!puffs_.empty())
  {
    // add new puff(s) to an existing vaporTrail

    // find the last puff previous to the current time
    const double prevPuffTime = puffs_.back()->time();
    if (prevPuffTime > time)
    {
      // The trimming in update() should prevent this
      assert(false);
      return;
    }

    const double puffSpacing = vaporTrailData_.numRadiiFromPreviousSmoke * vaporPuffData_.initialRadiusM;
    if (puffSpacing <= 0.0)
      return;

    const simCore::Vec3& prevPuffPosition = puffs_.back()->position();
    simCore::Vec3 hostOffsetPosition;
    locator_->getLocatorPosition(&hostOffsetPosition);

    // distance between host and last puff determines how many puffs are required
    const double distanceSinceLastPuff = simCore::v3Distance(hostOffsetPosition, prevPuffPosition);

    // adjust the spacing to get evenly spaced puffs from last puff to host offset
    const unsigned int puffsToAdd = static_cast<unsigned int>(0.5 + distanceSinceLastPuff / puffSpacing);

    // return if nothing to do
    if (puffsToAdd == 0)
      return;

    // adjust the amount to add against the point and time limits
    const size_t actualAddAmount = applyDataLimiting_(puffsToAdd, time, prevPuffTime);

    for (size_t i = (puffsToAdd - actualAddAmount + 1); i <= puffsToAdd; ++i)
    {
      const double xFactor = static_cast<double>(i) / puffsToAdd;
      // interpolate time and position for each puff
      const double puffTime = simCore::linearInterpolate(prevPuffTime, time, xFactor);
      const simCore::Vec3& puffPosition = simCore::linearInterpolate(prevPuffPosition, hostOffsetPosition, xFactor);
      addPuff_(puffPosition, puffTime);
    }
  }
}

void VaporTrail::addPuff_(const simCore::Vec3& puffPosition, double puffTime)
{
  // textureCounter must always reference a valid texture; see arithmetic below
  assert(textureCounter_ < textures_.size());

  // create the puff class that will own the puff graphic
  osg::ref_ptr<VaporTrailPuff> puff;

  const osg::Matrixd& puffMatrix = (vaporTrailData_.isWake ? calcWakeMatrix_(puffPosition) :
    osg::Matrixd::translate(puffPosition.x(), puffPosition.y(), puffPosition.z()));

  if (recyclePuffs_.empty())
  {
    puff = new VaporTrailPuff(textures_[textureCounter_].get(), puffMatrix, puffTime);
    // add it to the group/scenegraph
    vaporTrailGroup_->addChild(puff);
  }
  else
  {
    puff = recyclePuffs_.front();
    puff->set(puffMatrix, puffTime);
    recyclePuffs_.pop_front();
  }

  // add it to the vaporTrail puff map
  puffs_.push_back(puff);

  // update our texture counter
  if (textureCounter_ < textures_.size() - 1)
    ++textureCounter_;
  else
    textureCounter_ = 0;
}

osg::Matrixd VaporTrail::calcWakeMatrix_(const simCore::Vec3& ecefPosition)
{
  // convert an lla coordinate with null orientation to an ecef coordinate.
  // the resultant ecef orientation will be flat on the earth/ocean surface
  simCore::Vec3 llaPos;
  simCore::Vec3 ecefOri;
  simCore::CoordinateConverter::convertEcefToGeodeticPos(ecefPosition, llaPos);
  simCore::CoordinateConverter::convertGeodeticOriToEcef(llaPos, simCore::Vec3(0., -M_PI_2, 0.), ecefOri);
  osg::Matrixd mat;
  simVis::Math::ecefEulerToEnuRotMatrix(ecefOri, mat);
  mat.setTrans(ecefPosition.x(), ecefPosition.y(), ecefPosition.z());
  return mat;
}

void VaporTrail::processTextures_(const std::vector< osg::ref_ptr<osg::Texture2D> >& textures)
{
  for (std::vector< osg::ref_ptr<osg::Texture2D> >::const_iterator it = textures.begin(); it != textures.end(); ++it)
  {
    if (vaporTrailData_.isWake)
    {
      osg::ref_ptr<osg::Geode> geode = new osg::Geode();
      createTexture_(*(geode.get()), (*it).get());
      // show back facing texture so that wake can be seen from under water
      geode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::OFF);
      textures_.push_back(geode);
    }
    else
    {
      osg::ref_ptr<osg::Billboard> billboard = new osg::Billboard();
      billboard->setMode(osg::Billboard::POINT_ROT_EYE);
      createTexture_(*(billboard.get()), (*it).get());
      textures_.push_back(billboard);
    }
  }
}

void VaporTrail::createTexture_(osg::Geode& geode, osg::Texture2D* texture) const
{
  const unsigned int textureUnit = 1;
  const float initialRadius = static_cast<float>(vaporPuffData_.initialRadiusM);

  osg::StateSet* stateSet = geode.getOrCreateStateSet();
  stateSet->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);
  // Disable depth writing, even in the second pass for TPA
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setName("simVis::VaporTrail");
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array; // vertices to draw
  geom->setVertexArray(verts.get());

  // map (x,y) pixel coordinate to (s,t) texture coordinate
  osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
  geom->setTexCoordArray(textureUnit, texcoords.get());

  // colors
  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  geom->setColorArray(colors.get());
  colors->push_back(simVis::Color::White);

  // add an instance of vapor trail
  texcoords->push_back(osg::Vec2(1, 0));
  verts->push_back(osg::Vec3(initialRadius, 0, -initialRadius));

  texcoords->push_back(osg::Vec2(1, 1));
  verts->push_back(osg::Vec3(initialRadius, 0, initialRadius));

  texcoords->push_back(osg::Vec2(0, 0));
  verts->push_back(osg::Vec3(-initialRadius, 0, -initialRadius));

  texcoords->push_back(osg::Vec2(0, 1));
  verts->push_back(osg::Vec3(-initialRadius, 0, initialRadius));

  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, verts->size()));

  geode.addDrawable(geom.get());

  osgEarth::Registry::shaderGenerator().run(&geode);
}

//////////////////////////////////////////////////////////////////////////

VaporTrail::VaporTrailPuff::VaporTrailPuff(osg::Geode* graphic, const simCore::Vec3& position, double startTime)
  : scale_(1.0),
  startTime_(startTime),
  active_(true)
{
  addChild(graphic);
  setMatrix(osg::Matrixd::translate(position.x(), position.y(), position.z()));
  setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // set up our uniform for parent's shader, setting the default color.
  overrideColor_ = new OverrideColor(getOrCreateStateSet());
  overrideColor_->setColor(simVis::Color::White);
  overrideColor_->setCombineMode(OverrideColor::MULTIPLY_COLOR);
}

VaporTrail::VaporTrailPuff::VaporTrailPuff(osg::Geode* graphic, const osg::Matrixd& mat, double startTime)
  : scale_(1.0),
  startTime_(startTime),
  active_(true)
{
  addChild(graphic);
  setMatrix(mat);
  setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // set up our uniform for parent's shader, setting the default color.
  overrideColor_ = new OverrideColor(getOrCreateStateSet());
  overrideColor_->setColor(simVis::Color::White);
  overrideColor_->setCombineMode(OverrideColor::MULTIPLY_COLOR);
}

VaporTrail::VaporTrailPuff::~VaporTrailPuff()
{
  // remove from scene graph
  removeChildren(0, getNumChildren());

  const osg::Node::ParentList& parents = getParents();
  for (osg::Node::ParentList::const_iterator j = parents.begin(); j != parents.end(); ++j)
  {
    osg::observer_ptr<osg::Group> parentAsGroup = (*j)->asGroup();
    if (parentAsGroup.valid())
      parentAsGroup->removeChild(this);
  }
}

void VaporTrail::VaporTrailPuff::set(const simCore::Vec3& position, double startTime)
{
  // set this position in our matrix; it is required to set position for puffs with no expansion;
  // if there is a radius expansion/scaling, that will be handled in update() below
  setMatrix(osg::Matrixd::translate(position.x(), position.y(), position.z()));
  startTime_ = startTime;
  setNodeMask(simVis::DISPLAY_MASK_PLATFORM);
  active_ = true;
  scale_ = 1.0;
}

void VaporTrail::VaporTrailPuff::set(const osg::Matrixd& mat, double startTime)
{
  // set puff position and orientation; puff scaling is handled in update() below
  setMatrix(mat);
  startTime_ = startTime;
  setNodeMask(simVis::DISPLAY_MASK_PLATFORM);
  active_ = true;
  scale_ = 1.0;
}

void VaporTrail::VaporTrailPuff::clear()
{
  active_ = false;
  setNodeMask(simVis::DISPLAY_MASK_NONE);
}

simCore::Vec3 VaporTrail::VaporTrailPuff::position() const
{
  const osg::Vec3d& trans = getMatrix().getTrans();
  return simCore::Vec3(trans.x(), trans.y(), trans.z());
}

double VaporTrail::VaporTrailPuff::time() const
{
  return startTime_;
}

void VaporTrail::VaporTrailPuff::update(double currentTime, const VaporTrail::VaporPuffData& puffData)
{
  if (!active_)
    return;

  if (currentTime < startTime_)
  {
    // if assert fails, check that VaporTrail::update removes all puffs with time > current time
    assert(0);
    setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }
  // turn the puff off if update time is after fadeTime
  if (puffData.fadeTimeS != 0.0 && currentTime >= (startTime_ + puffData.fadeTimeS))
  {
    setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  setNodeMask(simVis::DISPLAY_MASK_PLATFORM);
  const double deltaTime = currentTime - startTime_;
  if (puffData.radiusExpansionRate != 0.0 && scale_ != 0.0)
  {
    const double newScale = (puffData.initialRadiusM + (puffData.radiusExpansionRate * deltaTime)) / puffData.initialRadiusM;
    const double scaleRatio = newScale / scale_;
    osg::Matrixd rescaled = getMatrix();
    rescaled.preMultScale(osg::Vec3d(scaleRatio, scaleRatio, scaleRatio));
    setMatrix(rescaled);
    scale_ = newScale;
  }

  if (puffData.fadeTimeS != 0.0)
  {
    const float alpha = 1.0f - deltaTime / puffData.fadeTimeS;
    assert(alpha > 0.0f && alpha <= 1.0f);
    overrideColor_->setColor(simVis::Color(simVis::Color::White, alpha));
  }
}

}
