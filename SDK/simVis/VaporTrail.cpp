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
#include "osg/Depth"
#include "osg/Geometry"
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"

#include "simCore/Calc/Interpolation.h"
#include "simData/LimitData.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/Platform.h"
#include "simVis/Registry.h"
#include "simVis/VaporTrail.h"

namespace simVis
{


VaporTrail::VaporTrailData::VaporTrailData()
  : startTime(10.0),
  endTime(20.0),
  numRadiiFromPreviousSmoke(1.5),
  metersBehindCurrentPosition(5.0)
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

  for (std::vector< osg::ref_ptr<osg::Texture2D> >::const_iterator it = textures.begin(); it != textures.end(); ++it)
  {
    textureBillboards_.push_back(createTextureBillboard_(*it));
  }

  hostPlatform_->addChild(vaporTrailGroup_);

  // create locator to track our host, and generate our offset position
  locator_ = new Locator(hostPlatform_->getLocator());
  locator_->setLocalOffsets(simCore::Vec3(0.0, -vaporTrailData_.metersBehindCurrentPosition, 0.0), simCore::Vec3());

  OverheadMode::enableGeometryFlattening(true, vaporTrailGroup_);
}

VaporTrail::~VaporTrail()
{
  puffs_.clear();
  recyclePuffs_.clear();
  if (hostPlatform_.valid())
    hostPlatform_->removeChild(vaporTrailGroup_);
  vaporTrailGroup_ = NULL;
  textureBillboards_.clear();
  locator_ = NULL;
}

void VaporTrail::update(double time)
{
  if (textureBillboards_.empty() ||
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

void VaporTrail::addNewPuffs_(double time)
{
  if (!hostPlatform_->isActive() || textureBillboards_.empty())
    return;

  if (puffs_.empty())
  {
    simCore::Vec3 hostOffsetPosition;
    locator_->getLocatorPosition(&hostOffsetPosition);
    addPuff_(hostOffsetPosition, time);
  }
  else if (puffs_.back()->time() == time)
  {
    // do not re-create if there is already a puff at this time, but check for changes in data limits
    applyDataLimiting_(0, time, puffs_.back()->time());
    return;
  }
  else
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

    const simCore::Vec3& prevPuffPosition = puffs_.back()->position();
    simCore::Vec3 hostOffsetPosition;
    locator_->getLocatorPosition(&hostOffsetPosition);

    // distance between host and last puff determines how many puffs are required
    const double distanceSinceLastPuff = simCore::v3Distance(hostOffsetPosition, prevPuffPosition);
    const double puffSpacing = vaporTrailData_.numRadiiFromPreviousSmoke * vaporPuffData_.initialRadiusM;
    if (puffSpacing <= 0.0)
      return;
    // adjust the spacing to get evenly spaced puffs from last puff to host offset
    unsigned int puffsToAdd = static_cast<unsigned int>(0.5 + distanceSinceLastPuff / puffSpacing);

    // return if nothing to do
    if (puffsToAdd == 0)
      return;

    // adjust the amount to add against the point and time limits
    size_t actualAddAmount = applyDataLimiting_(puffsToAdd, time, prevPuffTime);

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
  assert(textureCounter_ < textureBillboards_.size());

  // create the puff class that will own the puff graphic
  osg::ref_ptr<VaporTrailPuff> puff;

  if (recyclePuffs_.empty())
  {
    osg::Matrixd pos;
    pos.makeTranslate(puffPosition.x(), puffPosition.y(), puffPosition.z());
    // create the transform that will contain the puff graphic
    osg::ref_ptr<osg::MatrixTransform> puffTransform = new osg::MatrixTransform(pos);
    puffTransform->addChild(textureBillboards_[textureCounter_]);

    puff = new VaporTrailPuff(puffTransform, puffPosition, puffTime);
    // add it to the group/scenegraph
    vaporTrailGroup_->addChild(puffTransform);
  }
  else
  {
    puff = recyclePuffs_.front();
    puff->set(puffPosition, puffTime);
    recyclePuffs_.pop_front();
  }

  // add it to the vaporTrail puff map
  puffs_.push_back(puff);

  // update our texture counter
  if (textureCounter_ < textureBillboards_.size() - 1)
    ++textureCounter_;
  else
    textureCounter_ = 0;
}

osg::Billboard* VaporTrail::createTextureBillboard_(osg::Texture2D* texture) const
{
  const unsigned int textureUnit = 1;
  const double initialRadius = vaporPuffData_.initialRadiusM;

  osg::ref_ptr<osg::Billboard> textureBillboard = new osg::Billboard();
  osg::StateSet* stateSet = textureBillboard->getOrCreateStateSet();
  stateSet->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);

  // Set up the render bin, turn off depth writes, and turn on depth reads
  stateSet->setRenderBinDetails(BIN_ROCKETBURN, BIN_GLOBAL_SIMSDK);
  // Depth writes need to be off because sometimes the puffs, even in depth-sorted bin, will
  // draw in front of one another incorrectly, causing the puff alpha to take a depth that
  // other puffs should have instead.
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));

  // Must be able to blend or the graphics will look awful
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array; // vertices to draw
  geom->setVertexArray(verts);

  // map (x,y) pixel coordinate to (s,t) texture coordinate
  osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
  geom->setTexCoordArray(textureUnit, texcoords);

  // colors
  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
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

  textureBillboard->addDrawable(geom);

  textureBillboard->setMode(osg::Billboard::POINT_ROT_EYE);

  osgEarth::Registry::shaderGenerator().run(textureBillboard);

  return textureBillboard.release();
}

//////////////////////////////////////////////////////////////////////////


VaporTrail::VaporTrailPuff::VaporTrailPuff(osg::MatrixTransform* puffTransform, const simCore::Vec3& position, double startTime)
  :  position_(position),
     startTime_(startTime),
     active_(true)
{
  puff_ = puffTransform;
  puff_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // set up our uniform for parent's shader, setting the default color.
  overrideColor_ = new OverrideColor(puff_->getOrCreateStateSet());
  overrideColor_->setColor(simVis::Color::White);
}

VaporTrail::VaporTrailPuff::~VaporTrailPuff()
{
  // remove from scene graph
  puff_->removeChildren(0, puff_->getNumChildren());

  const osg::Node::ParentList& parents = puff_->getParents();
  for (osg::Node::ParentList::const_iterator j = parents.begin(); j != parents.end(); ++j)
  {
    osg::observer_ptr<osg::Group> parentAsGroup = (*j)->asGroup();
    if (parentAsGroup.valid())
      parentAsGroup->removeChild(puff_);
  }
}

void VaporTrail::VaporTrailPuff::set(const simCore::Vec3& position, double startTime)
{
  position_ = position;
  startTime_ = startTime;
  puff_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);
  active_ = true;
}

void VaporTrail::VaporTrailPuff::clear()
{
  active_ = false;
  puff_->setNodeMask(simVis::DISPLAY_MASK_NONE);
}

simCore::Vec3 VaporTrail::VaporTrailPuff::position() const
{
  return position_;
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
    puff_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }
  // turn the puff off if update time is after fadeTime
  if (puffData.fadeTimeS != 0.0 && currentTime >= (startTime_ + puffData.fadeTimeS))
  {
    puff_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  puff_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);
  const double deltaTime = currentTime - startTime_;
  if (puffData.radiusExpansionRate != 0.0)
  {
    const double newScale = (puffData.initialRadiusM + (puffData.radiusExpansionRate * deltaTime)) / puffData.initialRadiusM;
    osg::Matrixd scaled;
    scaled.makeTranslate(position_.x(), position_.y(), position_.z());
    scaled.preMultScale(osg::Vec3d(newScale, newScale, newScale));
    puff_->setMatrix(scaled);
  }

  if (puffData.fadeTimeS != 0.0)
  {
    const float alpha = 1.0f - deltaTime / puffData.fadeTimeS;
    assert(alpha > 0.0f && alpha <= 1.0f);
    overrideColor_->setColor(simVis::Color(simVis::Color::White, alpha));
  }
}

}
