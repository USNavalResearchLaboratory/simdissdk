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
#include <algorithm>
#include "osg/Depth"
#include "osgEarth/VirtualProgram"
#include "simCore/Calc/Angle.h"
#include "simVis/Constants.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/RFProp/BearingProfileMap.h"
#include "simVis/RFProp/ColorProvider.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/ProfileContext.h"
#include "simVis/RFProp/ProfileManager.h"

namespace simRF {

ProfileManager::ProfileManager(std::shared_ptr<simCore::DatumConvert> datumConvert)
 : simVis::LocatorNode(new simVis::Locator()),
  history_(15.0 * simCore::DEG2RAD),
  bearing_(0.),
  alpha_(1.f),
  displayOn_(false),
  profileContext_(std::make_shared<ProfileContext>(datumConvert))
{
  // Create initial map; ownership moves to timeBearingProfiles_
  currentProfileMap_ = new BearingProfileMap;
  timeBearingProfiles_[0] = currentProfileMap_;

  osg::StateSet* stateset = getOrCreateStateSet();
  stateset->setRenderBinDetails(simVis::BIN_RFPROPAGATION, simVis::BIN_TWO_PASS_ALPHA);

  alphaUniform_ = stateset->getOrCreateUniform("alpha", osg::Uniform::FLOAT);
  alphaUniform_->set(alpha_);

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
  for (const auto& iter : timeBearingProfiles_)
    delete iter.second;
}

void ProfileManager::reset()
{
  for (const auto& iter : timeBearingProfiles_)
    delete iter.second;
  timeBearingProfiles_.clear();
  profileContext_.reset();
  history_ = 15.0 * simCore::DEG2RAD;
  bearing_ = 0;
  alpha_ = 1.f;
  displayOn_ = false;
}

void ProfileManager::initShaders_()
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(getOrCreateStateSet());
  simVis::Shaders package;
  if (profileContext_->mode == Profile::DRAWMODE_3D_TEXTURE)
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
  // only if there is no existing map at the given time
  if (timeBearingProfiles_.find(time) == timeBearingProfiles_.end())
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

  // TODO: if there is a change in currentProfileMap_, update the refLLA for the profile manager (and all its profiles)

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

  for (const auto& bearingProfilePair : *currentProfileMap_)
  {
    // send THRESHOLDTYPE_NONE to turn profiles off
    bearingProfilePair.second->setThresholdType(displayOn_ ? profileContext_->type : ProfileDataProvider::THRESHOLDTYPE_NONE);
  }

  updateVisibility_();
}

bool ProfileManager::display() const
{
  return displayOn_;
}

void ProfileManager::setAlpha(float alpha)
{
  alpha = osg::clampBetween(alpha, 0.0f, 1.0f);
  if (alpha_ == alpha)
    return;
  alpha_ = alpha;
  alphaUniform_->set(alpha_);
  // dirty not required
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
  return profileContext_->agl;
}

void ProfileManager::setAGL(bool agl)
{
  if (profileContext_->agl != agl)
  {
    profileContext_->agl = agl;
    dirty_();
  }
}

Profile::DrawMode ProfileManager::getMode() const
{
  return profileContext_->mode;
}

void ProfileManager::setMode(Profile::DrawMode mode)
{
  if (profileContext_->mode != mode)
  {
    profileContext_->mode = mode;
    initShaders_();
    dirty_();
  }
}

ProfileDataProvider::ThresholdType ProfileManager::getThresholdType() const
{
  return profileContext_->type;
}

void ProfileManager::setThresholdType(ProfileDataProvider::ThresholdType type)
{
  if (profileContext_->type != type)
  {
    profileContext_->type = type;
    for (const auto& bearingProfilePair : *currentProfileMap_)
    {
      bearingProfilePair.second->setThresholdType(profileContext_->type);
    }
  }
}

unsigned int ProfileManager::getDisplayThickness() const
{
  return profileContext_->displayThickness;
}

void ProfileManager::setDisplayThickness(unsigned int displayThickness)
{
  if (profileContext_->displayThickness != displayThickness)
  {
    profileContext_->displayThickness = displayThickness;
    dirty_();
  }
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
  return profileContext_->heightM;
}

void ProfileManager::setHeight(double height)
{
  if (profileContext_->heightM != height)
  {
    profileContext_->heightM = height;
    dirty_();
  }
}

double ProfileManager::getRefLat() const
{
  return profileContext_->refLla.lat();
}

double ProfileManager::getRefLon() const
{
  return profileContext_->refLla.lon();
}

double ProfileManager::getRefAlt() const
{
  return profileContext_->refLla.alt();
}

void ProfileManager::setRefCoord(double latRad, double lonRad, double alt)
{
  const simCore::Vec3 refLla(latRad, lonRad, alt);
  getLocator()->setCoordinate(simCore::Coordinate(simCore::COORD_SYS_LLA, refLla), 0.);
  profileContext_->setRefLla(refLla);
  dirty_();
}

bool ProfileManager::getSphericalEarth() const
{
  return profileContext_->sphericalEarth;
}

void ProfileManager::setSphericalEarth(bool sphericalEarth)
{
  if (profileContext_->sphericalEarth != sphericalEarth)
  {
    profileContext_->sphericalEarth = sphericalEarth;
    dirty_();
  }
}

double ProfileManager::getElevAngle() const
{
  return profileContext_->elevAngleR;
}

void ProfileManager::setElevAngle(double elevAngleRad)
{
  if (profileContext_->elevAngleR != elevAngleRad)
  {
    profileContext_->elevAngleR = elevAngleRad;
    dirty_();
  }
}

Profile* ProfileManager::getProfileByBearing(double bearingR) const
{
  return currentProfileMap_->getProfileByBearing(bearingR);
}

const Profile* ProfileManager::getProfile(unsigned int index) const
{
  return ((index < getNumChildren()) ? dynamic_cast<const Profile*>(getChild(index)) : nullptr);
}

void ProfileManager::addProfile(Profile* profile)
{
  if (!profile)
    return;

  profile->setProfileContext(profileContext_);
  if (!displayOn_)
  {
    // force the type to NONE to turn off
    profile->setThresholdType(ProfileDataProvider::THRESHOLDTYPE_NONE);
  }
  // check to see if the new profile is replacing an existing profile
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
  const double minBearing = currentProfileMap_->getSlotBearing(bearing_ - (history_ / 2.0));
  double maxBearing = currentProfileMap_->getSlotBearing(bearing_ + (history_ / 2.0));

  // displayWrapsZero indicates conditions where the display wraps 360 -> 0;
  // e.g., minBearing is a large angle less than 360, maxBearing is a small number > 0;
  // or a complete 360 deg display where minBearing = maxBearing
  const bool displayWrapsZero = (minBearing > maxBearing) ||
    (minBearing == maxBearing && history_ >= (M_TWOPI - FLT_EPSILON));
  if (displayWrapsZero)
    maxBearing += M_TWOPI;

  for (const auto& bearingProfilePair : *currentProfileMap_)
  {
    const double profileBearing = bearingProfilePair.first;
    bool visible = (profileBearing >= minBearing && profileBearing <= maxBearing);
    if (displayWrapsZero && !visible)
    {
      // test if profile is in the piece that wraps around after TwoPI
      const double profileBearingAddTwoPi = profileBearing + M_TWOPI;
      visible = (profileBearingAddTwoPi >= minBearing) && (profileBearingAddTwoPi <= maxBearing);
    }
    bearingProfilePair.second->setNodeMask(visible ? simVis::DISPLAY_MASK_BEAM : simVis::DISPLAY_MASK_NONE);
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

void ProfileManager::dirty_()
{
  for (const auto& bearingProfilePair : *currentProfileMap_)
    bearingProfilePair.second->dirty();
}

}
