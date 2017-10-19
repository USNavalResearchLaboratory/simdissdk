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
#include <limits>
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simNotify/Notify.h"
#include "simVis/AnimatedLine.h"
#include "simVis/LocalGrid.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/TrackHistory.h"
#include "simVis/PlatformFilter.h"
#include "simVis/Registry.h"
#include "simVis/EntityLabel.h"
#include "simCore/Calc/Math.h"

#define LC "[PlatformNode] "

namespace simVis
{

// colors for body axis vectors
static const simVis::Color BODY_AXIS_X_COLOR = simVis::Color::Yellow;
static const simVis::Color BODY_AXIS_Y_COLOR = simVis::Color::Fuchsia;
static const simVis::Color BODY_AXIS_Z_COLOR = simVis::Color::Aqua;
static const simVis::Color INERTIAL_AXIS_X_COLOR = simVis::Color::Red;
static const simVis::Color INERTIAL_AXIS_Y_COLOR = simVis::Color::Lime;
static const simVis::Color INERTIAL_AXIS_Z_COLOR = simVis::Color::Blue;
static const simVis::Color VELOCITY_VECTOR_COLOR = osg::Vec4f(1.0, 0.5, 0.0, 1.0); // Orange from SIMDIS 9
static const simVis::Color MOON_VECTOR_COLOR = simVis::Color::White;
static const simVis::Color SUN_VECTOR_COLOR = simVis::Color::Yellow;

// Distance in meters that a platform drawing optical or radio horizon must move laterally before the horizon is recalculated
static const double HORIZON_RANGE_STEP = 100;
// Distance in meters that a platform drawing optical or radio horizon must move vertically before the horizon is recalculated
static const double HORIZON_ALT_STEP = 10;

// Colors to use when drawing optical or radio horizon
static const osg::Vec4 HORIZON_VISIBLE_COLOR = osg::Vec4(0, 1, 0, 0.6); // Translucent green
static const osg::Vec4 HORIZON_OBSTRUCTED_COLOR = osg::Vec4(1, 0, 0, 0.6); // Translucent red

// this is used as a sentinel value for an platform that does not (currently) have a valid position
static const simData::PlatformUpdate NULL_PLATFORM_UPDATE = simData::PlatformUpdate();

/** OSG Callback that sets the axis length based on the platform's current model state */
class SetAxisLengthCallback : public osg::Callback
{
public:
  /** If bodyAxis, then the axes are scaled differently */
  SetAxisLengthCallback(PlatformNode* platform, bool bodyAxis)
    : Callback(),
      platform_(platform),
      xScalar_(bodyAxis ? -1.f : 1.f)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    AxisVector* vector = dynamic_cast<AxisVector*>(object);
    if (vector != NULL && platform_.valid())
    {
      const float axisScale = platform_->getPrefs().axisscale();
      const float lineLength = VectorScaling::lineLength(platform_->getModel(), axisScale);
      // Note that body axis reverses the X axis
      vector->setAxisLengths(lineLength * xScalar_, lineLength, lineLength);
    }
    return traverse(object, data);
  }

private:
  osg::observer_ptr<PlatformNode> platform_;
  float xScalar_;
};

/** OSG Callback that sets the circle radius based on the platform's current model state */
class SetCircleRadiusCallback : public osg::Callback
{
public:
  /** If bodyAxis, then the axes are scaled differently */
  explicit SetCircleRadiusCallback(PlatformNode* platform)
    : Callback(),
      platform_(platform)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    AreaHighlightNode* area = dynamic_cast<AreaHighlightNode*>(object);
    // Scale down the radius by a small amount -- 80% -- to reduce highlight size
    if (area != NULL && platform_.valid())
      area->setRadius(VectorScaling::lineLength(platform_->getModel(), 0.8));
    return traverse(object, data);
  }

private:
  osg::observer_ptr<PlatformNode> platform_;
};


//----------------------------------------------------------------------------

PlatformNode::PlatformNode(const simData::PlatformProperties& props,
                           const simData::DataStore& dataStore,
                           PlatformTspiFilterManager& manager,
                           osg::Group* trackParent,
                           Locator* locator, int referenceYear) :
EntityNode(simData::PLATFORM, locator),
ds_(dataStore),
platformTspiFilterManager_(manager),
lastUpdateTime_(-std::numeric_limits<float>::max()),
firstHistoryTime_(std::numeric_limits<float>::max()),
trackParent_(trackParent),
track_(NULL),
localGrid_(NULL),
bodyAxisVector_(NULL),
inertialAxisVector_(NULL),
scaledInertialTransform_(new PlatformInertialTransform),
velocityAxisVector_(NULL),
ephemerisVector_(NULL),
model_(NULL),
contentCallback_(new NullEntityCallback()),
losCreator_(NULL),
opticalLosNode_(NULL),
radioLosNode_(NULL),
frontOffset_(0.0),
valid_(false),
lastPrefsValid_(false),
forceUpdateFromDataStore_(false),
queuedInvalidate_(false)
{
  PlatformModelNode* node = new PlatformModelNode(new Locator(locator));
  this->addChild(node);
  model_ = node;

  this->setProperties(props);

  setName("PlatformNode");

  setNodeMask(simVis::DISPLAY_MASK_NONE);

  localGrid_ = new LocalGridNode(locator, this, referenceYear);
  addChild(localGrid_);

  scaledInertialTransform_->setLocator(getLocator());
  model_->addScaledChild(scaledInertialTransform_);
}

PlatformNode::~PlatformNode()
{
  if (track_.valid())
    trackParent_->removeChild(track_);
}

void PlatformNode::setProperties(const simData::PlatformProperties& props)
{
  if (model_)
  {
    model_->setProperties(props);
  }

  lastProps_ = props;
}

simCore::RadarCrossSectionPtr PlatformNode::getRcs() const
{
  return rcs_;
}

void PlatformNode::setRcsPrefs_(const simData::PlatformPrefs& prefs)
{
  if (prefs.rcsfile() != lastPrefs_.rcsfile())
  {
    if (prefs.rcsfile().empty())
    {
      rcs_.reset();
    }
    else
    {
      std::string uri = simVis::Registry::instance()->findModelFile(prefs.rcsfile());
      if (!uri.empty())
      {
        rcs_.reset(simCore::RcsFileParser::loadRCSFile(uri));
        if (rcs_ == NULL)
        {
          SIM_WARN << LC << "Failed to load RCS file \"" << uri << "\"" << std::endl;
        }
      }
      else
      {
        SIM_WARN << LC << "Failed to load RCS file \"" << prefs.rcsfile() << "\"" << std::endl;
      }
    }
    if (model_)
      model_->setRcsData(rcs_);
  }
}

void PlatformNode::setPrefs(const simData::PlatformPrefs& prefs)
{
  const bool prefsDraw = prefs.commonprefs().datadraw() && prefs.commonprefs().draw();
  // if the platform is valid, update if this platform should be drawn
  if (valid_)
    setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);

  // update our model prefs
  if (model_ != NULL)
  {
    if (prefsDraw)
    {
      model_->setPrefs(prefs);
      updateLabel_(prefs);
    }

    updateOrRemoveBodyAxis_(prefsDraw, prefs);
    updateOrRemoveInertialAxis_(prefsDraw, prefs);
    updateOrRemoveVelocityVector_(prefsDraw, prefs);
    updateOrRemoveEphemerisVector_(prefsDraw, prefs);
    updateOrRemoveCircleHighlight_(prefsDraw, prefs);
    updateOrRemoveHorizons_(prefs);
  }

  setRcsPrefs_(prefs);

  // remove or create track history
  if (showTrack_(prefs))
  {
    if (!track_.valid())
      createTrackHistoryNode_(prefs);
    else
    {
      // normal processing: update the track history data
      track_->setPrefs(prefs, lastProps_);

      // track_ cannot be valid without having had platform prefs set at least once;
      // if assert fails, check whether prefs are initialized correctly when platform is created
      assert(lastPrefsValid_);

      if (PB_SUBFIELD_CHANGED((&lastPrefs_), (&prefs), commonprefs, datalimitpoints) ||
        PB_SUBFIELD_CHANGED((&lastPrefs_), (&prefs), commonprefs, datalimittime))
      {
        // track history is constrained by platform data limiting
        track_->reset();
        track_->update();
      }
      if (track_.valid())
        track_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);
    }
  }
  else
  {
    trackParent_->removeChild(track_);
    track_ = NULL;
  }

  // validate localgrid prefs changes that might provide user notifications
  if (localGrid_.valid())
  {
    localGrid_->validatePrefs(prefs.commonprefs().localgrid());

    // update the local grid, only if platform drawn
    if (prefsDraw)
      localGrid_->setPrefs(prefs.commonprefs().localgrid());
  }

  // check for a prefs change that would require re-computing the bounds of the model
  // if the properties of the model have changed, adjust the host bounding box to match
  if (!lastPrefsValid_ ||
      PB_FIELD_CHANGED((&lastPrefs_), (&prefs), icon) ||
      PB_FIELD_CHANGED((&lastPrefs_), (&prefs), scale) ||
      PB_FIELD_CHANGED((&lastPrefs_), (&prefs), dynamicscale) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, scalexyz) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, platpositionoffset) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, yaw) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, pitch) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, roll))
  {
    updateHostBounds_(prefs.scale());
  }

  if (lastPrefsValid_ &&
     (PB_FIELD_CHANGED(&lastPrefs_, &prefs, surfaceclamping) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, useclampalt) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, clampvalaltmin) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, clampvalaltmax)))
  {
    // these prefs changes require an update to the locator
    forceUpdateFromDataStore_ = true;
  }

  lastPrefs_ = prefs;
  lastPrefsValid_ = true;
}

// Note: simVis::ScenarioManager notifies beams of changes in platform visual size
void PlatformNode::updateHostBounds_(double scale)
{
  if (model_)
  {
    scaledModelBounds_   = model_->getScaledIconBounds();
    unscaledModelBounds_ = model_->getUnscaledIconBounds();
    frontOffset_         = unscaledModelBounds_.yMax() * scale;

    if (track_.valid())
      track_->setHostBounds(osg::Vec2(unscaledModelBounds_.xMin() * scale, unscaledModelBounds_.xMax() * scale));
  }
}

PlatformModelNode* PlatformNode::getModel()
{
  return model_;
}

TrackHistoryNode* PlatformNode::getTrackHistory()
{
  return track_;
}

void PlatformNode::updateLocator_(const simData::PlatformUpdate& u)
{
  // static platforms by convention have elapsedEciTime 0
  simCore::Coordinate coord(
        simCore::COORD_SYS_ECEF,
        simCore::Vec3(u.x(), u.y(), u.z()),
        simCore::Vec3(u.psi(), u.theta(), u.phi()),
        simCore::Vec3(u.vx(), u.vy(), u.vz()));

  getLocator()->setCoordinate(coord, u.time(), lastProps_.coordinateframe().ecireferencetime());

  // if locator has changed and localGrid is displayed, update it
  if (localGrid_)
  {
    localGrid_->notifyHostLocatorChange();
  }

  if (lastPrefsValid_)
  {
    updateOrRemoveHorizons_(lastPrefs_);
  }
}

bool PlatformNode::isActive() const
{
  return isActive_(lastPrefs_);
}

simData::ObjectId PlatformNode::getId() const
{
  return lastProps_.id();
}

bool PlatformNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  // if assert fails, check whether prefs are initialized correctly when platform is created
  assert(lastPrefsValid_);

  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase);
  assert(updateSlice);

  // apply the queued invalidate first, so the state can then be further arbitrated by any new data points
  if (queuedInvalidate_)
  {
    setInvalid_();
    queuedInvalidate_ = false;
  }

  // in file mode, a platform is not valid until time reaches its first datapoint time.
  // standard interfaces will return NULL or a sentinel value to indicate that the platform does not have a valid position.
  // but there are cases where it is useful to know the position the platform will have when it becomes valid.
  // as an example, you may want to create a viewport to show the moment the platform becomes valid and starts to move.
  // to best show this, you want to be able to create the viewport's eyeposition based on that position in advance.

  // this intent of this code is to:
  // set the locator position to the first datapoint's platform position when time is earlier than the platform's first valid time.
  // set the locator position to the last valid position when time has exceeded the last valid time.
  // ensure that the locator value is reset only once
  // ensure that locator position is set in cases where time has been jumped to an early time or to a late time.
  //
  // this should only matter in file mode.
  if (!updateSlice->current() && (updateSlice->hasChanged() || updateSlice->isDirty()))
  {
    const double firstTime = updateSlice->firstTime();
    if (firstTime != std::numeric_limits<double>::max() && ds_.updateTime() < firstTime)
    {
      if (getLocator()->getTime() != firstTime)
      {
        const simData::PlatformUpdateSlice::Iterator platformIter = updateSlice->lower_bound(firstTime);
        const simData::PlatformUpdate* platformUpdate = platformIter.peekNext();
        // we verified that the slice had a first time, so we must have a valid update at that time
        assert(platformUpdate);
        if (platformUpdate)
          updateLocator_(*platformUpdate);
      }
    }
    else
    {
      const double lastTime = updateSlice->lastTime();
      if (lastTime != -std::numeric_limits<double>::max() && ds_.updateTime() > lastTime)
      {
        if (getLocator()->getTime() != lastTime)
        {
          const simData::PlatformUpdateSlice::Iterator platformIter = updateSlice->lower_bound(lastTime);
          const simData::PlatformUpdate* platformUpdate = platformIter.peekNext();
          // we verified that the slice had a last time, so we must have a valid update at that time
          assert(platformUpdate);
          if (platformUpdate)
            updateLocator_(*platformUpdate);
        }
      }
    }
  }

  if (!updateSlice->hasChanged() && !force && !forceUpdateFromDataStore_)
  {
    // Even if the platform has not changed, the label can still change - entity name could change as a result of category data, for example.
    updateLabel_(lastPrefs_);
    return false;
  }

  // check if time changed based on last data store update time, ignoring static platforms
  bool timeChanged = (lastUpdateTime_ != -1.0) && (ds_.updateTime() != lastUpdateTime_);
  lastUpdateTime_ = ds_.updateTime();

  if (updateSlice->current())
  {
    simData::PlatformUpdate current = *updateSlice->current();

    PlatformTspiFilterManager::FilterResponse modified = platformTspiFilterManager_.filter(current, lastPrefs_, lastProps_);
    if (modified == PlatformTspiFilterManager::POINT_DROPPED)
    {
      setInvalid_();
      if (velocityAxisVector_)
        velocityAxisVector_->update(NULL_PLATFORM_UPDATE);
      if (ephemerisVector_)
        ephemerisVector_->update(NULL_PLATFORM_UPDATE);
      return true;
    }
    valid_ = true;
    // need to update lastUpdate_ and lastUpdateTime_ before calling updateLocator which will reference them and expect them to be up to date
    lastUpdate_ = current;
    lastUpdateTime_ = current.time();
    firstHistoryTime_ = updateSlice->firstTime();
    updateLocator_(current);

    // update only if entity should be visible
    if (lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw())
      setNodeMask(DISPLAY_MASK_PLATFORM);
    else
    {
      // if commands/prefs have turned the platform off, DISPLAY_MASK_NONE will already be set
      assert(getNodeMask() == DISPLAY_MASK_NONE);
    }
  }
  else
  {
    // a NULL update means the platform should be disabled
    setInvalid_();
  }

  // remove or create track history
  if (showTrack_(lastPrefs_))
  {
    if (!track_.valid())
      createTrackHistoryNode_(lastPrefs_);
    else if (timeChanged)
      track_->update();
  }
  else if (track_.valid())
  {
    trackParent_->removeChild(track_);
    track_ = NULL;
  }

  // avoid applying a null update over and over
  if (!updateSlice->current() && getNodeMask() == DISPLAY_MASK_NONE && !valid_)
    return false;

  if (velocityAxisVector_)
    velocityAxisVector_->update(lastUpdate_);
  if (ephemerisVector_)
    ephemerisVector_->update(lastUpdate_);

  updateLabel_(lastPrefs_);
  forceUpdateFromDataStore_ = false;
  return true;
}

bool PlatformNode::isActive_(const simData::PlatformPrefs& prefs) const
{
  // the valid_ flag indicates that the platform node has data at current scenario time, but this can be manually overridden by the datadraw flag
  return valid_ && lastPrefs_.commonprefs().datadraw();
}

void PlatformNode::setInvalid_()
{
  valid_ = false;
  lastUpdate_ = NULL_PLATFORM_UPDATE;
  setNodeMask(simVis::DISPLAY_MASK_NONE);
}

bool PlatformNode::showTrack_(const simData::PlatformPrefs& prefs) const
{
  return (lastUpdateTime_ != -1.0) && (prefs.trackprefs().trackdrawmode() != simData::TrackPrefs_Mode_OFF) && (isActive_(prefs) || showExpiredTrackHistory_(prefs));
}

bool PlatformNode::showExpiredTrackHistory_(const simData::PlatformPrefs& prefs) const
{
  const bool showHistory = prefs.has_trackprefs() && prefs.trackprefs().has_expiremode() && prefs.trackprefs().expiremode();
  return showHistory && (ds_.updateTime() >= firstHistoryTime_);
}

bool PlatformNode::createTrackHistoryNode_(const simData::PlatformPrefs& prefs)
{
  // if assert fails, check that callers only call on !valid() condition
  assert(!track_.valid());
  // create the Track History "on demand" if requested
  track_ = new TrackHistoryNode(ds_, getLocator()->getSRS(), platformTspiFilterManager_, getId());
  trackParent_->addChild(track_);
  track_->setPrefs(prefs, lastProps_, true);
  updateHostBounds_(prefs.scale());
  track_->update();
  const bool prefsDraw = lastPrefs_.commonprefs().datadraw() && prefs.commonprefs().draw();
  track_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);
  return true;
}

void PlatformNode::updateClockMode(const simCore::Clock* clock)
{
  // notify the track history of a change in time direction
  if (track_.valid())
    track_->updateClockMode(clock);
}

void PlatformNode::flush()
{
  // static platforms don't get flushed
  if (lastUpdateTime_ == -1.0)
    return;
  // queue up the invalidate to apply on the next data store update. SIMDIS-2805
  queuedInvalidate_ = true;
  if (track_.valid())
    track_->reset();
  if (velocityAxisVector_)
    velocityAxisVector_->update(NULL_PLATFORM_UPDATE);
  if (ephemerisVector_)
    ephemerisVector_->update(NULL_PLATFORM_UPDATE);
}

double PlatformNode::range() const
{
  // Platform has no concept of range so should not be making this call
  assert(false);
  return 0.0;
}

const simData::PlatformUpdate* PlatformNode::update() const
{
  return isActive() ? &lastUpdate_ : NULL;
}

const std::string PlatformNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // if assert fails, check whether prefs are initialized correctly when entity is created
  assert(lastPrefsValid_);
  switch (nameType)
  {
  case EntityNode::REAL_NAME:
    return lastPrefs_.commonprefs().name();
  case EntityNode::ALIAS_NAME:
    return lastPrefs_.commonprefs().alias();
  case EntityNode::DISPLAY_NAME:
    if (lastPrefs_.commonprefs().usealias())
    {
      if (!lastPrefs_.commonprefs().alias().empty() || allowBlankAlias)
        return lastPrefs_.commonprefs().alias();
    }
    return lastPrefs_.commonprefs().name();
  }
  return "";
}

void PlatformNode::updateLabel_(const simData::PlatformPrefs& prefs)
{
  if (model_ && valid_)
  {
    std::string label = getEntityName(EntityNode::DISPLAY_NAME, true);
    if (prefs.commonprefs().labelprefs().namelength() > 0)
      label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

    std::string text;
    if (prefs.commonprefs().labelprefs().draw())
      text = contentCallback_->createString(prefs, lastUpdate_, prefs.commonprefs().labelprefs().displayfields());

    if (!text.empty())
    {
      if (!label.empty())
        label += "\n";
      label += text;
    }

    float zOffset = 0.0f;
    model_->label()->update(prefs.commonprefs(), label, zOffset);
  }
}

void PlatformNode::setLabelContentCallback(LabelContentCallback* cb)
{
  if (cb == NULL)
    contentCallback_ = new NullEntityCallback();
  else
    contentCallback_ = cb;
}

LabelContentCallback* PlatformNode::labelContentCallback() const
{
  return contentCallback_.get();
}

std::string PlatformNode::popupText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    std::string prefix;
    /// if alias is defined show both in the popup to match SIMDIS 9's behavior.  SIMDIS-2241
    if (!lastPrefs_.commonprefs().alias().empty())
    {
      if (lastPrefs_.commonprefs().usealias())
        prefix = getEntityName(EntityNode::REAL_NAME);
      else
        prefix = getEntityName(EntityNode::ALIAS_NAME);
      prefix += "\n";
    }
    return prefix + contentCallback_->createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string PlatformNode::hookText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    return contentCallback_->createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  }

  return "";
}

std::string PlatformNode::legendText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    return contentCallback_->createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  }

  return "";
}

void PlatformNode::updateOrRemoveBodyAxis_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  // Create or remove body axis vectors
  if (prefsDraw && prefs.drawbodyaxis())
  {
    if (!bodyAxisVector_)
    {
      bodyAxisVector_ = new AxisVector();
      bodyAxisVector_->setColors(BODY_AXIS_X_COLOR, BODY_AXIS_Y_COLOR, BODY_AXIS_Z_COLOR);
      bodyAxisVector_->addUpdateCallback(new SetAxisLengthCallback(this, true));
      // Set a node mask so we don't mouse-over a wide region
      bodyAxisVector_->setNodeMask(simVis::DISPLAY_MASK_LABEL);
      model_->addScaledChild(bodyAxisVector_);
    }
  }
  else if (bodyAxisVector_.valid()) // remove if present
  {
    model_->removeScaledChild(bodyAxisVector_);
    bodyAxisVector_ = NULL;
  }
}

void PlatformNode::updateOrRemoveInertialAxis_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  // Create or remove body axis vectors
  if (prefsDraw && prefs.drawinertialaxis())
  {
    if (!inertialAxisVector_)
    {
      inertialAxisVector_ = new AxisVector();
      inertialAxisVector_->setColors(INERTIAL_AXIS_X_COLOR, INERTIAL_AXIS_Y_COLOR, INERTIAL_AXIS_Z_COLOR);
      inertialAxisVector_->addUpdateCallback(new SetAxisLengthCallback(this, false));
      // Set a node mask so we don't mouse-over a wide region
      inertialAxisVector_->setNodeMask(simVis::DISPLAY_MASK_LABEL);
      scaledInertialTransform_->addChild(inertialAxisVector_);
    }
  }
  else if (inertialAxisVector_.valid()) // remove if present
  {
    scaledInertialTransform_->removeChild(inertialAxisVector_);
    inertialAxisVector_ = NULL;
  }
}

void PlatformNode::updateOrRemoveVelocityVector_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  // Update or remove velocity axis vectors
  if (prefsDraw && prefs.drawvelocityvec())
  {
    if (velocityAxisVector_)
      velocityAxisVector_->setPrefs(prefs.drawvelocityvec(), prefs, PB_FIELD_CHANGED(&lastPrefs_, &prefs, drawvelocityvec));
    else
    {
      velocityAxisVector_ = new VelocityVector(getLocator(), VELOCITY_VECTOR_COLOR);
      addChild(velocityAxisVector_);
      // force rebuild
      velocityAxisVector_->setPrefs(prefs.drawvelocityvec(), prefs, true);
      velocityAxisVector_->update(lastUpdate_);
    }
  }
  else if (velocityAxisVector_.valid()) // remove if present
  {
    removeChild(velocityAxisVector_);
    velocityAxisVector_ = NULL;
  }
}

void PlatformNode::updateOrRemoveEphemerisVector_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  // Update or remove ephemeris axis vectors
  if (prefsDraw && (prefs.drawmoonvec() || prefs.drawsunvec()))
  {
    if (ephemerisVector_)
      ephemerisVector_->setPrefs(prefs);
    else
    {
      ephemerisVector_ = new EphemerisVector(MOON_VECTOR_COLOR, SUN_VECTOR_COLOR);
      ephemerisVector_->setModelNode(model_);
      scaledInertialTransform_->addChild(ephemerisVector_);
      // force rebuild
      ephemerisVector_->setPrefs(prefs);
      ephemerisVector_->update(lastUpdate_);
    }
  }
  else if (ephemerisVector_.valid()) // remove if present
  {
    scaledInertialTransform_->removeChild(ephemerisVector_);
    ephemerisVector_ = NULL;
  }
}

void PlatformNode::updateOrRemoveCircleHighlight_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  if (prefsDraw && prefs.drawcirclehilight())
  {
    if (!areaHighlight_.valid())
    {
      areaHighlight_ = new AreaHighlightNode();
      areaHighlight_->addUpdateCallback(new SetCircleRadiusCallback(this));
      scaledInertialTransform_->addChild(areaHighlight_);
    }
    areaHighlight_->setColor(simVis::Color(prefs.circlehilightcolor(), simVis::Color::RGBA));
  }
  else if (areaHighlight_.valid()) // remove if present
  {
    scaledInertialTransform_->removeChild(areaHighlight_);
    areaHighlight_ = NULL;
  }
}

void PlatformNode::updateOrRemoveHorizons_(const simData::PlatformPrefs& prefs)
{
  updateOrRemoveHorizon_(simCore::OPTICAL_HORIZON, prefs);
  updateOrRemoveHorizon_(simCore::RADAR_HORIZON, prefs);
}

void PlatformNode::updateOrRemoveHorizon_(simCore::HorizonCalculations horizonType, const simData::PlatformPrefs& prefs)
{
  RadialLOSNode* los = NULL;
  bool drawHorizon = false;
  switch (horizonType)
  {
  case simCore::OPTICAL_HORIZON:
    // Create and add node if we haven't already
    if (!opticalLosNode_ && losCreator_)
    {
      opticalLosNode_ = losCreator_->newLosNode();
      opticalLosNode_->setNodeMask(simVis::DISPLAY_MASK_LABEL);
      addChild(opticalLosNode_);
    }
    los = opticalLosNode_;
    drawHorizon = prefs.drawopticlos();
    break;
  case simCore::RADAR_HORIZON:
    // Create and add node if we haven't already
    if (!radioLosNode_ && losCreator_)
    {
      radioLosNode_ = losCreator_->newLosNode();
      radioLosNode_->setNodeMask(simVis::DISPLAY_MASK_LABEL);
      addChild(radioLosNode_);
    }
    los = radioLosNode_;
    drawHorizon = prefs.drawrflos();
    break;
  case simCore::GEOMETRIC_HORIZON:
    // Horizon calculations are only allowed for optical or radar
    assert(0);
    break;
  }

  if (!los)
  {
    // Do not assert.  Null los nodes are valid
    return;
  }

  if (!prefs.commonprefs().datadraw() || !prefs.commonprefs().draw() || !drawHorizon) // Remove horizon if it's currently visible
  {
    los->setActive(false);
    return;
  }

  double rangeDist = 0;
  double altDist = 0;

  simCore::Coordinate platCoord = getLocator()->getCoordinate();
  simCore::Coordinate platLlaCoord;
  simCore::CoordinateConverter converter;
  converter.convert(platCoord, platLlaCoord, simCore::COORD_SYS_LLA);

  // Draw/update horizon
  simCore::Coordinate losCoord = los->getCoordinate();

  if (losCoord.coordinateSystem() != simCore::COORD_SYS_NONE) // losNode is not guaranteed to have a valid coord
  {
    simCore::Coordinate losLlaCoord;
    converter.convert(losCoord, losLlaCoord, simCore::COORD_SYS_LLA);

    rangeDist = simCore::calculateGroundDist(losLlaCoord.position(), platLlaCoord.position(), simCore::WGS_84, NULL);
    altDist = fabs(losLlaCoord.alt() - platLlaCoord.alt());
  }
  else
  {
    // Always trigger a redraw if the losNode doesn't have a valid coordinate
    rangeDist = HORIZON_RANGE_STEP + 1;
  }

  // Don't update if horizon is already active and platform is within acceptable range of last horizon center
  if (HORIZON_ALT_STEP > altDist && HORIZON_RANGE_STEP > rangeDist && los->getActive())
    return;

  // Deactivate temporarily to prevent unnecessary calculations while updating los fields
  los->setActive(false);

  los->setCoordinate(getLocator()->getCoordinate());

  los->setMaxRange(Distance(simCore::calculateHorizonDist(platLlaCoord.position(), horizonType), osgEarth::Units::METERS));
  los->setAzimuthalResolution(Angle(5, osgEarth::Units::DEGREES));

  los->setActive(true);
}

void PlatformNode::setLosCreator(LosCreator* losCreator)
{
  losCreator_ = losCreator;
}

}
