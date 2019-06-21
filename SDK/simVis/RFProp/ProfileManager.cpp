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
#include <algorithm>
#include "osg/Depth"
#include "osgEarth/VirtualProgram"
#include "simCore/Calc/Angle.h"
#include "simVis/Constants.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/BearingProfileMap.h"
#include "simVis/RFProp/ProfileManager.h"

namespace simRF {
//----------------------------------------------------------------------------
ProfileManager::ProfileManager()
 : osg::Group(),
   history_(osg::DegreesToRadians(15.0)),
   bearing_(0),
   height_(0),
   displayThickness_(1000.0f),
   agl_(false),
   displayOn_(false),
   alpha_(1.f),
   mode_(Profile::DRAWMODE_2D_HORIZONTAL),
   refCoord_(0, 0, 0),
   sphericalEarth_(true),
   elevAngle_(0),
   type_(simRF::ProfileDataProvider::THRESHOLDTYPE_NONE)
{
  // create the initial map
  currentProfileMap_ = new BearingProfileMap;
  timeBearingProfiles_[0] = currentProfileMap_;

  osg::StateSet* stateset = getOrCreateStateSet();
  stateset->setRenderBinDetails(simVis::BIN_RFPROPAGATION, simVis::BIN_TWO_PASS_ALPHA);

  // Turn off lighting; we do not set normals in profiles, so lighting will look bad
  simVis::setLighting(stateset, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

  // Use the visible horizon clip plane
  stateset->setMode(simVis::CLIPPLANE_VISIBLE_HORIZON_GL_MODE, osg::StateAttribute::ON);
  // Blending should be enabled
  stateset->setMode(GL_BLEND, osg::StateAttribute::ON);

  // Create a uniform for the textures
  osg::Uniform* textureUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "texture");
  textureUniform->set(0);
  stateset->addUniform(textureUniform);

  // Initialize the lossToColor function to the default
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
  simVis::Shaders package;
  const std::string src = osgEarth::ShaderLoader::load(package.rfPropLossToColorDefault(), package);
  vp->setShader(LOSS_TO_COLOR_VERTEX, new osg::Shader(osg::Shader::VERTEX, src));
  vp->setShader(LOSS_TO_COLOR_FRAGMENT, new osg::Shader(osg::Shader::FRAGMENT, src));

  // Bind the loss vertex attribute
  vp->addBindAttribLocation("loss", osg::Drawable::ATTRIBUTE_6);

  initShaders_();
}

ProfileManager::~ProfileManager()
{
  for (std::map<double, BearingProfileMap*>::const_iterator i = timeBearingProfiles_.begin(); i != timeBearingProfiles_.end(); ++i)
    delete i->second;
}

void ProfileManager::initShaders_()
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(getOrCreateStateSet());
  simVis::Shaders package;
  if (mode_ == Profile::DRAWMODE_3D_TEXTURE)
  {
    package.load(vp, package.rfPropTextureBasedVertex());
    package.load(vp, package.rfPropTextureBasedFragment());
  }
  else
  {
    package.load(vp, package.rfPropVertexBasedVertex());
    package.load(vp, package.rfPropVertexBasedFragment());
  }
  // need to update color provider, since stateset may have changed
  if (colorProvider_.valid())
    colorProvider_->install(getOrCreateStateSet());
}

void ProfileManager::addProfileMap(double time)
{
  // check for an existing map at the given time
  std::map<double, BearingProfileMap*>::const_iterator i = timeBearingProfiles_.find(time);
  if (i != timeBearingProfiles_.end())
    return;

  timeBearingProfiles_[time] = new BearingProfileMap;
}

void ProfileManager::removeProfileMap(double time)
{
  timeBearingProfiles_.erase(time);
}

void ProfileManager::update(double time)
{
  // get the map at time >= the given time
  std::map<double, BearingProfileMap*>::const_iterator i = timeBearingProfiles_.lower_bound(time);

  // TODO: if there is a change in currentProfileMap_, update the ref_coord for the profile manager (and all its profiles)

  // if requested time is after last map
  if (i == timeBearingProfiles_.end())
  {
    // there must be something
    assert(i != timeBearingProfiles_.begin());
    --i;
  }

  currentProfileMap_ = i->second;
}

void ProfileManager::setDisplay(bool onOff)
{
  if (displayOn_ == onOff)
    return;
  displayOn_ = onOff;
  // setThresholdType will turn the profiles off
  setThresholdType(type_);
  updateVisibility_();
}

bool ProfileManager::display() const
{
  return displayOn_;
}

void ProfileManager::setAlpha(float alpha)
{
  if (alpha_ == alpha)
    return;
  alpha_ = alpha;

  // Loop through all times
  for (auto allIter = timeBearingProfiles_.begin(); allIter != timeBearingProfiles_.end(); ++allIter)
  {
    // Loop through all bearings
    for (auto profileIter = allIter->second->begin(); profileIter != allIter->second->end(); ++profileIter)
      profileIter->second->setAlpha(alpha);
  }
}

float ProfileManager::getAlpha() const
{
  return alpha_;
}

double ProfileManager::getHistory() const
{
  return history_;
}

void ProfileManager::setHistory(double history)
{
  if (history_ != history)
  {
    history_ = osg::clampBetween(history, 0.0, M_TWOPI);
    updateVisibility_();
  }
}

bool ProfileManager::getAGL() const
{
  return agl_;
}

void ProfileManager::setAGL(bool agl)
{
  if (agl_ != agl)
  {
    agl_ = agl;
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setAGL(agl_);
    }
  }
}

Profile::DrawMode ProfileManager::getMode() const
{
  return mode_;
}

void ProfileManager::setMode(Profile::DrawMode mode)
{
  if (mode_ != mode)
  {
    mode_ = mode;
    initShaders_();
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setMode(mode_);
    }
  }
}

float ProfileManager::getDisplayThickness() const
{
  return displayThickness_;
}

void ProfileManager::setDisplayThickness(float displayThickness)
{
  if (displayThickness_ != displayThickness)
  {
    displayThickness_ = displayThickness;
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setDisplayThickness(displayThickness_);
    }
  }
}

int ProfileManager::setThicknessBySlots(int numSlots)
{
  // Fail if there are no profiles
  if (currentProfileMap_ == NULL || currentProfileMap_->empty() || numSlots < 1)
    return 1;

  // Figure out the height step in the first profile
  osg::ref_ptr<Profile> firstProfile = currentProfileMap_->begin()->second;
  if (!firstProfile.valid())
    return 1;
  const CompositeProfileProvider* dataProvider = firstProfile->getDataProvider();
  if (dataProvider == NULL)
    return 1;
  // Note that we subtract 1 in order to prevent an extra point from showing
  setDisplayThickness((numSlots - 1) * dataProvider->getHeightStep());
  return 0;
}

double ProfileManager::getBearing() const
{
  return bearing_;
}

void ProfileManager::setBearing(double bearing)
{
  if (bearing_ != bearing)
  {
    bearing_ = bearing;
    updateVisibility_();
  }
}

double ProfileManager::getHeight() const
{
  return height_;
}

void ProfileManager::setHeight(double height)
{
  if (height_ != height)
  {
    height_ = height;
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setHeight(height_);
    }
  }
}

double ProfileManager::getRefLat() const
{
  return refCoord_.y();
}

double ProfileManager::getRefLon() const
{
  return refCoord_.x();
}

double ProfileManager::getRefAlt() const
{
  return refCoord_.z();
}

void ProfileManager::setRefCoord(double latRad, double lonRad, double alt)
{
  if (latRad != refCoord_.y() || lonRad != refCoord_.x() || alt != refCoord_.z())
  {

    refCoord_.set(lonRad, latRad, alt);
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setRefCoord(refCoord_.y(), refCoord_.x(), refCoord_.z());
    }
  }
}

bool ProfileManager::getSphericalEarth() const
{
  return sphericalEarth_;
}

void ProfileManager::setSphericalEarth(bool sphericalEarth)
{
  if (sphericalEarth_ != sphericalEarth)
  {
    sphericalEarth_ = sphericalEarth;
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setSphericalEarth(sphericalEarth_);
    }
  }
}

double ProfileManager::getElevAngle() const
{
  return elevAngle_;
}

void ProfileManager::setElevAngle(double elevAngleRad)
{
  if (elevAngle_ != elevAngleRad)
  {
    elevAngle_ = elevAngleRad;
    for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
    {
      itr->second->setElevAngle(elevAngle_);
    }
  }
}

Profile* ProfileManager::getProfileByBearing(double bearingR) const
{
  return currentProfileMap_->getProfileByBearing(bearingR);
}

void ProfileManager::addProfile(Profile* profile)
{
  if (!profile)
    return;

  profile->setHeight(height_);
  profile->setMode(mode_);
  profile->setAGL(agl_);
  profile->setDisplayThickness(displayThickness_);
  profile->setRefCoord(refCoord_.y(), refCoord_.x(), refCoord_.z());
  profile->setSphericalEarth(sphericalEarth_);
  profile->setElevAngle(elevAngle_);
  profile->setThresholdType(displayOn_ ? type_ : ProfileDataProvider::THRESHOLDTYPE_NONE);
  profile->setAlpha(alpha_);

  // old profile must match exactly
  Profile *oldProfile = currentProfileMap_->getProfileByBearing(profile->getBearing());
  if (oldProfile)
    removeChild(oldProfile);

  addChild(profile);
  currentProfileMap_->addProfile(*profile);
  updateVisibility_();
}

void ProfileManager::updateVisibility_()
{
  if (!displayOn_)
    return;
  // only changes in beam bearing or history require recalc of minbearing & maxBearing -> optimization possible here
  const double minBearing = currentProfileMap_->getSlotBearing(bearing_ - history_ / 2.0);
  double maxBearing = currentProfileMap_->getSlotBearing(bearing_ + history_ / 2.0);
  // addTwoPi indicates the condition that the display wraps 360 -> 0 and the max is shifted to > 360
  bool addTwoPi = false;
  if (minBearing >= maxBearing || history_ >= (M_TWOPI - FLT_EPSILON))
  {
    addTwoPi = true;
    maxBearing += M_TWOPI;
  }

  for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
  {
    const double profileBearing = itr->first;
    bool visible = (profileBearing >= minBearing && profileBearing <= maxBearing);
    if (addTwoPi && !visible)
    {
      // test if profile is in the piece that wraps around after TwoPI
      const double profileBearingAddTwoPi = profileBearing + M_TWOPI;
      visible = (profileBearingAddTwoPi >= minBearing) && (profileBearingAddTwoPi <= maxBearing);
    }
    itr->second->setNodeMask(visible ? simVis::DISPLAY_MASK_BEAM : simVis::DISPLAY_MASK_NONE);
  }
}

ColorProvider* ProfileManager::getColorProvider() const
{
  return colorProvider_.get();
}

void ProfileManager::setColorProvider(ColorProvider* colorProvider)
{
  if (colorProvider_ != colorProvider)
  {
    // Uninstall the old provider
    if (colorProvider_.valid())
    {
      colorProvider_->uninstall(getOrCreateStateSet());
    }

    colorProvider_ = colorProvider;

    // Install the new provider
    if (colorProvider_.valid())
    {
      colorProvider_->install(getOrCreateStateSet());
    }
  }
}

ProfileDataProvider::ThresholdType ProfileManager::getThresholdType() const
{
  return type_;
}

void ProfileManager::setThresholdType(ProfileDataProvider::ThresholdType type)
{
  type_ = type;
  // when display is off, do not propagate the type to the profiles; instead use THRESHOLDTYPE_NONE to turn profiles off
  for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
  {
    itr->second->setThresholdType(displayOn_ ? type_ : ProfileDataProvider::THRESHOLDTYPE_NONE);
  }
}

void ProfileManager::dirty()
{
  for (BearingProfileMap::iterator itr = currentProfileMap_->begin(); itr != currentProfileMap_->end(); ++itr)
  {
    itr->second->dirty();
  }
}

}
