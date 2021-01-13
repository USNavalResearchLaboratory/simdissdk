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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <limits>
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simNotify/Notify.h"
#include "simData/DataSlice.h"
#include "simVis/AreaHighlight.h"
#include "simVis/AxisVector.h"
#include "simVis/EntityLabel.h"
#include "simVis/EphemerisVector.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/PlatformInertialTransform.h"
#include "simVis/PlatformFilter.h"
#include "simVis/PlatformModel.h"
#include "simVis/RadialLOSNode.h"
#include "simVis/Registry.h"
#include "simVis/TimeTicks.h"
#include "simVis/TrackHistory.h"
#include "simVis/Utils.h"
#include "simVis/VelocityVector.h"
#include "simVis/Platform.h"
#include "simVis/Projector.h"
#include "simVis/Shaders.h"

#undef LC
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
static const simVis::Color MOON_VECTOR_COLOR = simVis::Color::White;
static const simVis::Color SUN_VECTOR_COLOR = simVis::Color::Yellow;

// Distance in meters that a platform drawing optical or radio horizon must move laterally before the horizon is recalculated
static const double HORIZON_RANGE_STEP = 100;
// Distance in meters that a platform drawing optical or radio horizon must move vertically before the horizon is recalculated
static const double HORIZON_ALT_STEP = 10;

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
    if (vector != nullptr && platform_.valid())
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
    CompositeHighlightNode* area = dynamic_cast<CompositeHighlightNode*>(object);
    // Scale down the radius by a small amount -- 80% -- to reduce highlight size
    if (area != nullptr && platform_.valid())
      area->setRadius(VectorScaling::lineLength(platform_->getModel(), 0.8));
    return traverse(object, data);
  }

private:
  osg::observer_ptr<PlatformNode> platform_;
};

//----------------------------------------------------------------------------

/** Calls PlatformNode::updateHostBounds() when model node gets a bounds update. */
class BoundsUpdater : public simVis::PlatformModelNode::Callback
{
public:
  explicit BoundsUpdater(PlatformNode* platform)
    : platform_(platform)
  {
  }

  virtual void operator()(simVis::PlatformModelNode* model, Callback::EventType eventType)
  {
    if (eventType == Callback::BOUNDS_CHANGED)
    {
      osg::ref_ptr<PlatformNode> refPlat;
      if (platform_.lock(refPlat))
        refPlat->updateHostBounds();
    }
  }

private:
  osg::observer_ptr<PlatformNode> platform_;
};

//----------------------------------------------------------------------------

PlatformNode::PlatformNode(const simData::PlatformProperties& props,
                           const simData::DataStore& dataStore,
                           PlatformTspiFilterManager& manager,
                           osg::Group* expireModeGroupAttach,
                           Locator* eciLocator,
                           int referenceYear)
  : EntityNode(simData::PLATFORM, new CachingLocator()),
  ds_(dataStore),
  platformTspiFilterManager_(manager),
  lastUpdateTime_(-std::numeric_limits<float>::max()),
  firstHistoryTime_(std::numeric_limits<float>::max()),
  eciLocator_(eciLocator),
  expireModeGroupAttach_(expireModeGroupAttach),
  track_(nullptr),
  timeTicks_(nullptr),
  localGrid_(nullptr),
  bodyAxisVector_(nullptr),
  inertialAxisVector_(nullptr),
  scaledInertialTransform_(new PlatformInertialTransform),
  velocityAxisVector_(nullptr),
  ephemerisVector_(nullptr),
  model_(nullptr),
  losCreator_(nullptr),
  opticalLosNode_(nullptr),
  radioLosNode_(nullptr),
  frontOffset_(0.0),
  valid_(false),
  lastPrefsValid_(false),
  forceUpdateFromDataStore_(false),
  queuedInvalidate_(false)
{
  model_ = new PlatformModelNode(new Locator(getLocator()));
  addChild(model_);
  model_->addCallback(new BoundsUpdater(this));

  this->setProperties(props);

  setName("PlatformNode");

  setNodeMask(simVis::DISPLAY_MASK_NONE);

  localGrid_ = new LocalGridNode(getLocator(), this, referenceYear);
  addChild(localGrid_);

  scaledInertialTransform_->setLocator(getLocator());
  model_->addScaledChild(scaledInertialTransform_.get());
}

PlatformNode::~PlatformNode()
{
  if (!expireModeGroup_.valid())
    return;

  if (track_.valid())
    expireModeGroup_->removeChild(track_);
  track_ = nullptr;
  if (timeTicks_.valid())
    expireModeGroup_->removeChild(timeTicks_);
  timeTicks_ = nullptr;

  if (expireModeGroupAttach_.valid())
    expireModeGroupAttach_->removeChild(expireModeGroup_);
  expireModeGroup_ = nullptr;
}

void PlatformNode::setProperties(const simData::PlatformProperties& props)
{
  model_->setProperties(props);
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
        if (rcs_ == nullptr)
        {
          SIM_WARN << LC << "Failed to load RCS file \"" << uri << "\"" << std::endl;
        }
      }
      else
      {
        SIM_WARN << LC << "Failed to load RCS file \"" << prefs.rcsfile() << "\"" << std::endl;
      }
    }
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
  if (prefsDraw)
  {
    model_->setPrefs(prefs);
    updateLabel_(prefs);
  }

  if (expireModeGroup_.valid() &&
    (!lastPrefsValid_ || PB_FIELD_CHANGED((&lastPrefs_), (&prefs), ecidatamode)))
  {
    // on change of eci data mode, track and timeticks need to be completely recreated
    if (track_.valid())
    {
      expireModeGroup_->removeChild(track_);
      track_ = nullptr;
    }
    if (timeTicks_.valid())
    {
      expireModeGroup_->removeChild(timeTicks_);
      timeTicks_ = nullptr;
    }
    forceUpdateFromDataStore_ = true;
  }

  updateOrRemoveBodyAxis_(prefsDraw, prefs);
  updateOrRemoveInertialAxis_(prefsDraw, prefs);
  updateOrRemoveVelocityVector_(prefsDraw, prefs);
  updateOrRemoveEphemerisVector_(prefsDraw, prefs);
  updateOrRemoveCircleHighlight_(prefsDraw, prefs);
  updateOrRemoveHorizons_(prefs, true);

  setRcsPrefs_(prefs);

  // manage visibility of expireModeGroup (track, timeticks and vaporTrail)
  if (expireModeGroup_.valid())
    expireModeGroup_->setNodeMask(showTrackTrail_(prefs) ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);

  // remove or create track history
  if (showTrack_(prefs))
  {
    // first check time ticks
    if (prefs.trackprefs().timeticks().drawstyle() != simData::TimeTickPrefs::NONE)
    {
      if (!timeTicks_.valid())
        createTimeTicks_(prefs);
      // creating timeticks must also ensure that there is a valid group
      assert(expireModeGroup_.valid());
    }
    else if (timeTicks_.valid())
    {
      // can't have valid timeticks without also having its group
      assert(expireModeGroup_.valid());
      expireModeGroup_->removeChild(timeTicks_);
      timeTicks_ = nullptr;
    }

    if (!track_.valid())
    {
      createTrackHistoryNode_(prefs);
      // creating track history must also ensure that there is a valid group
      assert(expireModeGroup_.valid());
    }
    else
    {
      // normal processing: update the track history data
      track_->setPrefs(prefs, lastProps_);
      if (timeTicks_.valid())
        timeTicks_->setPrefs(prefs, lastProps_);

      // track_ cannot be valid without having had platform prefs set at least once;
      // if assert fails, check whether prefs are initialized correctly when platform is created
      assert(lastPrefsValid_);

      if (PB_SUBFIELD_CHANGED((&lastPrefs_), (&prefs), commonprefs, datalimitpoints) ||
        PB_SUBFIELD_CHANGED((&lastPrefs_), (&prefs), commonprefs, datalimittime))
      {
        // track history is constrained by platform data limiting
        track_->reset();
        track_->update();
        // time ticks follows data limiting same as track history
        if (timeTicks_.valid())
        {
          timeTicks_->reset();
          timeTicks_->update();
        }
      }
      track_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);
      if (timeTicks_.valid())
        timeTicks_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);
    }
  }
  else if (expireModeGroup_.valid())
  {
    expireModeGroup_->removeChild(track_);
    track_ = nullptr;
    // time ticks is always hidden if track history is hidden
    expireModeGroup_->removeChild(timeTicks_);
    timeTicks_ = nullptr;
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
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, clampvalaltmax) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, abovesurfaceclamping)))
  {
    // these prefs changes require an update to the locator
    forceUpdateFromDataStore_ = true;
  }

  if (!lastPrefsValid_ || PB_FIELD_CHANGED((&lastPrefs_.commonprefs()), (&prefs.commonprefs()), acceptprojectorid))
    applyProjectorPrefs_(lastPrefs_.commonprefs(), prefs.commonprefs());

  lastPrefs_ = prefs;
  lastPrefsValid_ = true;
}

const osg::BoundingBox& PlatformNode::getActualSize() const
{
  return model_->getUnscaledIconBounds();
}

const osg::BoundingBox& PlatformNode::getVisualSize() const
{
  return model_->getScaledIconBounds();
}

// Note: simVis::ScenarioManager notifies beams of changes in platform visual size
void PlatformNode::updateHostBounds_(double scale)
{
  const osg::BoundingBox& unscaledBounds = model_->getUnscaledIconBounds();
  frontOffset_ = unscaledBounds.yMax() * scale;
  if (track_.valid())
    track_->setHostBounds(osg::Vec2(unscaledBounds.xMin() * scale, unscaledBounds.xMax() * scale));
  // Ephemeris vector length depends on model bounds
  if (ephemerisVector_.valid())
    ephemerisVector_->update(lastUpdate_);
}

void PlatformNode::updateHostBounds()
{
  // It does not matter here whether lastPrefs is valid or not.  The bounds of the
  // child definitely updated, and we just need to fix the track values and front offset
  updateHostBounds_(lastPrefs_.scale());
}

osg::Group* PlatformNode::getOrCreateExpireModeGroup()
{
  // Container for platform-related objects that can be rendered even when platform is no longer valid.
  // Platform manages the visibility of the group.
  // Any class that adds a child is reponsible for removing that child.

  if (!expireModeGroup_.valid())
  {
    // lazy creation of expireModeGroup, SIM-12258
    expireModeGroup_ = new osg::Group;
    expireModeGroup_->setName("PlatformNode Expire Mode Group");

    if (expireModeGroupAttach_.valid())
      expireModeGroupAttach_->addChild(expireModeGroup_);
  }
  return expireModeGroup_.get();
}

PlatformModelNode* PlatformNode::getModel()
{
  return model_.get();
}

TrackHistoryNode* PlatformNode::getTrackHistory()
{
  return track_.get();
}

void PlatformNode::updateLocator_(const simData::PlatformUpdate& u)
{
  // static platforms by convention have elapsedEciTime 0
  const simCore::Coordinate coord(
        simCore::COORD_SYS_ECEF,
        simCore::Vec3(u.x(), u.y(), u.z()),
        simCore::Vec3(u.psi(), u.theta(), u.phi()),
        simCore::Vec3(u.vx(), u.vy(), u.vz()));

  getLocator()->setCoordinate(coord, u.time(), lastProps_.coordinateframe().ecireferencetime());

  if (lastPrefsValid_)
  {
    updateOrRemoveHorizons_(lastPrefs_, false);
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
  // Do not assert on lastPrefsValid_; this routine can get called during platform creation.
  if (!lastPrefsValid_)
    return false;

  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase);
  assert(updateSlice);

  firstHistoryTime_ = (updateSlice->numItems() == 0) ? std::numeric_limits<float>::max() : updateSlice->firstTime();

  // apply the queued invalidate first, so the state can then be further arbitrated by any new data points
  if (queuedInvalidate_)
  {
    setInvalid_();
    queuedInvalidate_ = false;
  }

  // in file mode, a platform is not valid until time reaches its first datapoint time.
  // standard interfaces will return nullptr or a sentinel value to indicate that the platform does not have a valid position.
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

  // check if time changed based on last data store update time, ignoring static platforms
  const bool timeChanged = (lastUpdateTime_ != -1.0) && (ds_.updateTime() != lastUpdateTime_);

  // Time can completely jump over the life span of the platform.
  // The method updateSlice->hasChanged() will indicate no change, but the code should not kick out early.
  const bool timeJumpOverLifeSpan = (timeChanged && (updateSlice->numItems() != 0) &&
    (((lastUpdateTime_ <= updateSlice->firstTime()) && (ds_.updateTime() >= updateSlice->lastTime())) ||
    ((ds_.updateTime() <= updateSlice->firstTime()) && (lastUpdateTime_ >= updateSlice->lastTime()))));

  if (!updateSlice->hasChanged() && !timeJumpOverLifeSpan && !force && !forceUpdateFromDataStore_)
  {
    if (timeTicks_.valid())
      timeTicks_->update();
    // Even if the platform has not changed, the label can still change - entity name could change as a result of category data, for example.
    updateLabel_(lastPrefs_);
    return false;
  }

  lastUpdateTime_ = ds_.updateTime();

  if (updateSlice->current())
  {
    simData::PlatformUpdate current = *updateSlice->current();
    lastUnfilteredUpdate_ = current;
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
    // a nullptr update means the platform should be disabled
    setInvalid_();
  }

  // manage visibility of track and trail group
  if (expireModeGroup_.valid())
    expireModeGroup_->setNodeMask(showTrackTrail_(lastPrefs_) ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);

  // remove or create track history
  if (showTrack_(lastPrefs_))
  {
    if (!track_.valid())
      createTrackHistoryNode_(lastPrefs_);
    else
    {
      if (timeChanged || updateSlice->hasChanged())
        track_->update();
      // always update time ticks
      if (timeTicks_.valid())
        timeTicks_->update();
    }
  }
  else if (expireModeGroup_.valid())
  {
    if (track_.valid())
    {
      expireModeGroup_->removeChild(track_);
      track_ = nullptr;
    }
    if (timeTicks_.valid())
    {
      expireModeGroup_->removeChild(timeTicks_);
      timeTicks_ = nullptr;
    }
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
  lastUnfilteredUpdate_ = NULL_PLATFORM_UPDATE;
  setNodeMask(simVis::DISPLAY_MASK_NONE);
}

// Track may be shown even when platform is not shown only in the "show expired track history" case.
// Track history is not maintained when not drawn, but it is recreated from datastore when platform draw is turned on.
// Due to common occurrence of scenarios with lots of platforms that are not of particular interest,
// it was decided to make platform as lightweight an object as possible when not drawn.
// The downside is that in a large scenario, turning draw off for all platforms then turning back on might cause hiccups.
bool PlatformNode::showTrack_(const simData::PlatformPrefs& prefs) const
{
  return showTrackTrail_(prefs) &&
    prefs.trackprefs().trackdrawmode() != simData::TrackPrefs_Mode_OFF;
}

bool PlatformNode::showTrackTrail_(const simData::PlatformPrefs& prefs) const
{
  return (lastUpdateTime_ != -1.0) &&
    (prefs.commonprefs().draw()) &&
    (isActive_(prefs) || showExpiredTrackHistory_(prefs));
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
  if (prefs.ecidatamode())
  {
    // trackhistory for an ECI platform gets a locator derived from the scenario ECI locator
    // dev error to construct a platform with a nullptr locator
    assert(eciLocator_);
    if (eciLocator_)
      track_ = new TrackHistoryNode(ds_, eciLocator_.get(), platformTspiFilterManager_, getId());
  }
  // trackhistory for platform in ecef datamode gets a new empty locator
  if (!track_.valid())
    track_ = new TrackHistoryNode(ds_, new Locator(), platformTspiFilterManager_, getId());

  getOrCreateExpireModeGroup()->addChild(track_);
  track_->setPrefs(prefs, lastProps_, true);
  updateHostBounds_(prefs.scale());
  track_->update();

  const bool prefsDraw = lastPrefs_.commonprefs().datadraw() && prefs.commonprefs().draw();
  track_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);

  // check to see if we need to create time ticks, since time ticks, like track history, can be turned on before platform is actually valid
  if ((prefs.trackprefs().timeticks().drawstyle() != simData::TimeTickPrefs::NONE) && !timeTicks_.valid())
    createTimeTicks_(prefs);

  return true;
}

bool PlatformNode::createTimeTicks_(const simData::PlatformPrefs& prefs)
{
  // if assert fails, check that callers only call on !valid() condition
  assert(!timeTicks_.valid());

  if (prefs.ecidatamode())
  {
    // dev error to construct a platform with a nullptr locator argument
    assert(eciLocator_);
    if (eciLocator_)
      timeTicks_ = new TimeTicks(ds_, eciLocator_.get(), platformTspiFilterManager_, getId());
  }
  // for non-ECI platform use a new empty locator
  if (!timeTicks_.valid())
    timeTicks_ = new TimeTicks(ds_, new Locator(), platformTspiFilterManager_, getId());
  getOrCreateExpireModeGroup()->addChild(timeTicks_);
  timeTicks_->setPrefs(prefs, lastProps_, true);
  timeTicks_->update();

  const bool prefsDraw = lastPrefs_.commonprefs().datadraw() && prefs.commonprefs().draw();
  timeTicks_->setNodeMask(prefsDraw ? simVis::DISPLAY_MASK_TRACK_HISTORY : simVis::DISPLAY_MASK_NONE);

  return true;
}

void PlatformNode::updateClockMode(const simCore::Clock* clock)
{
  // notify the track history of a change in time direction
  if (track_.valid())
    track_->updateClockMode(clock);
  if (timeTicks_.valid())
    timeTicks_->updateClockMode(clock);
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
  return isActive() ? &lastUpdate_ : nullptr;
}

const simData::PlatformUpdate* PlatformNode::labelUpdate() const
{
  return isActive() ? labelUpdate_(lastPrefs_) : nullptr;
}

const std::string PlatformNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // if assert fails, check whether prefs are initialized correctly when entity is created
  assert(lastPrefsValid_);
  return getEntityName_(lastPrefs_.commonprefs(), nameType, allowBlankAlias);
}

void PlatformNode::updateLabel_(const simData::PlatformPrefs& prefs)
{
  if (!valid_)
    return;

  std::string label = getEntityName_(prefs.commonprefs(), EntityNode::DISPLAY_NAME, true);
  if (prefs.commonprefs().labelprefs().namelength() > 0)
    label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

  std::string text;
  if (prefs.commonprefs().labelprefs().draw())
    text = labelContentCallback().createString(prefs, *labelUpdate_(prefs), prefs.commonprefs().labelprefs().displayfields());

  if (!text.empty())
  {
    if (!label.empty())
      label += "\n";
    label += text;
  }

  float zOffset = 0.0f;
  model_->label()->update(prefs.commonprefs(), label, zOffset);
}

std::string PlatformNode::popupText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    std::string prefix;
    // if alias is defined show both in the popup to match SIMDIS 9's behavior.  SIMDIS-2241
    if (!lastPrefs_.commonprefs().alias().empty())
    {
      if (lastPrefs_.commonprefs().usealias())
        prefix = getEntityName(EntityNode::REAL_NAME);
      else
        prefix = getEntityName(EntityNode::ALIAS_NAME);
      prefix += "\n";
    }
    return prefix + labelContentCallback().createString(lastPrefs_, *labelUpdate_(lastPrefs_), lastPrefs_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string PlatformNode::hookText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    return labelContentCallback().createString(lastPrefs_, *labelUpdate_(lastPrefs_), lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  }

  return "";
}

std::string PlatformNode::legendText() const
{
  if (lastPrefsValid_ && valid_)
  {
    // a valid_ platform should never have an update that does not have a time
    assert(lastUpdate_.has_time());
    return labelContentCallback().createString(lastPrefs_, *labelUpdate_(lastPrefs_), lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  }

  return "";
}

const simData::PlatformUpdate* PlatformNode::labelUpdate_(const simData::PlatformPrefs& prefs) const
{
  switch (prefs.commonprefs().labelprefs().usevalues())
  {
  case simData::LabelPrefs::DISPLAY_VALUE:
    break;
  case simData::LabelPrefs::ACTUAL_VALUE:
    return &lastUnfilteredUpdate_;
  }
  return &lastUpdate_;
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
      model_->addScaledChild(bodyAxisVector_.get());
    }
  }
  else if (bodyAxisVector_.valid()) // remove if present
  {
    model_->removeScaledChild(bodyAxisVector_.get());
    bodyAxisVector_ = nullptr;
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
    inertialAxisVector_ = nullptr;
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
      velocityAxisVector_ = new VelocityVector(getLocator());
      addChild(velocityAxisVector_);
      // force rebuild
      velocityAxisVector_->update(lastUpdate_);
      velocityAxisVector_->setPrefs(prefs.drawvelocityvec(), prefs, true);
    }
  }
  else if (velocityAxisVector_.valid()) // remove if present
  {
    removeChild(velocityAxisVector_);
    velocityAxisVector_ = nullptr;
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
      ephemerisVector_->setModelNode(model_.get());
      scaledInertialTransform_->addChild(ephemerisVector_);
      // force rebuild
      ephemerisVector_->setPrefs(prefs);
      ephemerisVector_->update(lastUpdate_);
    }
  }
  else if (ephemerisVector_.valid()) // remove if present
  {
    scaledInertialTransform_->removeChild(ephemerisVector_);
    ephemerisVector_ = nullptr;
  }
}

void PlatformNode::updateOrRemoveCircleHighlight_(bool prefsDraw, const simData::PlatformPrefs& prefs)
{
  if (prefsDraw && prefs.drawcirclehilight())
  {
    if (!highlight_.valid())
    {
      highlight_ = new CompositeHighlightNode(prefs.circlehilightshape());
      highlight_->addUpdateCallback(new SetCircleRadiusCallback(this));
      scaledInertialTransform_->addChild(highlight_.get());
    }
    else
      highlight_->setShape(prefs.circlehilightshape());
    highlight_->setColor(simVis::Color(prefs.circlehilightcolor(), simVis::Color::RGBA));
  }
  else if (highlight_.valid()) // remove if present
  {
    scaledInertialTransform_->removeChild(highlight_);
    highlight_ = nullptr;
  }
}

void PlatformNode::updateOrRemoveHorizons_(const simData::PlatformPrefs& prefs, bool force)
{
  updateOrRemoveHorizon_(simCore::OPTICAL_HORIZON, prefs, force);
  updateOrRemoveHorizon_(simCore::RADAR_HORIZON, prefs, force);
}

void PlatformNode::updateOrRemoveHorizon_(simCore::HorizonCalculations horizonType, const simData::PlatformPrefs& prefs, bool force)
{
  RadialLOSNode* los = nullptr;
  bool drawHorizon = false;
  switch (horizonType)
  {
  case simCore::OPTICAL_HORIZON:
    if (!prefs.drawopticlos())
    {
      if (opticalLosNode_)
        opticalLosNode_->setActive(false);
      return;
    }
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
    if (!prefs.drawrflos())
    {
      if (radioLosNode_)
        radioLosNode_->setActive(false);
      return;
    }
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

  // Deactivate temporarily to prevent unnecessary calculations while updating LOS fields
  los->setActive(false);

  // Update the visible color if it has changed
  if (prefs.has_visibleloscolor())
  {
    osg::Vec4f color = ColorUtils::RgbaToVec4(prefs.visibleloscolor());
    if (color != los->getVisibleColor())
      los->setVisibleColor(color);
  }

  // Update the obstructed color if it has changed
  if (prefs.has_obstructedloscolor())
  {
    osg::Vec4f color = ColorUtils::RgbaToVec4(prefs.obstructedloscolor());
    if (color != los->getObstructedColor())
      los->setObstructedColor(color);
  }

  // Update the range resolution if it has changed
  if (prefs.has_losrangeresolution())
  {
    const auto& rr = los->getRangeResolution();
    const double val = rr.getValue();
    if (val > 0.0 && val != prefs.losrangeresolution())
      los->setRangeResolution(osgEarth::Distance(prefs.losrangeresolution(), osgEarth::Units::METERS));
  }

  // Update the azimuthal resolution if it has changed
  if (prefs.has_losazimuthalresolution())
  {
    const auto& ar = los->getAzimuthalResolution();
    const double val = ar.getValue();
    if (val > 0.0 && val != prefs.losazimuthalresolution())
      los->setAzimuthalResolution(osgEarth::Angle(prefs.losazimuthalresolution(), osgEarth::Units::DEGREES));
  }

  double rangeDist = 0;
  double altDist = 0;

  simCore::Coordinate platCoord = getLocator()->getCoordinate();
  // Make sure the position has been set
  if (platCoord.position() == simCore::Vec3(0.0, 0.0, 0.0))
  {
    // No need to reactivate the LOS, it is not valid
    return;
  }

  simCore::Coordinate platLlaCoord;
  simCore::CoordinateConverter converter;
  converter.convert(platCoord, platLlaCoord, simCore::COORD_SYS_LLA);
  // Add the altitude offset after the conversion for correctness
  platLlaCoord.setPositionLLA(platLlaCoord.x(), platLlaCoord.y(), platLlaCoord.z() + prefs.losaltitudeoffset());

  // Draw/update horizon
  simCore::Coordinate losCoord = los->getCoordinate();

  if (losCoord.coordinateSystem() != simCore::COORD_SYS_NONE) // losNode is not guaranteed to have a valid coord
  {
    simCore::Coordinate losLlaCoord;
    converter.convert(losCoord, losLlaCoord, simCore::COORD_SYS_LLA);

    rangeDist = simCore::calculateGroundDist(losLlaCoord.position(), platLlaCoord.position(), simCore::WGS_84, nullptr);
    altDist = fabs(losLlaCoord.alt() - platLlaCoord.alt());
  }
  else
  {
    // Always trigger a redraw if the losNode doesn't have a valid coordinate
    rangeDist = HORIZON_RANGE_STEP + 1;
  }

  // Don't update if platform is within acceptable range of last horizon center
  if ((HORIZON_ALT_STEP > altDist) && (HORIZON_RANGE_STEP > rangeDist) && !force)
  {
    // Reactivate the LOS, undoing the setActive(false) above
    los->setActive(true);
    return;
  }

  // Need to convert the updated coord back to original system before giving to LOS Node
  converter.convert(platLlaCoord, platCoord, platCoord.coordinateSystem());
  los->setCoordinate(platCoord);
  los->setMaxRange(osgEarth::Distance(simCore::calculateHorizonDist(platLlaCoord.position(), horizonType), osgEarth::Units::METERS));

  // Reactivate the LOS, undoing the setActive(false) above
  los->setActive(true);
}

void PlatformNode::setLosCreator(LosCreator* losCreator)
{
  losCreator_ = losCreator;
}

unsigned int PlatformNode::objectIndexTag() const
{
  return model_->objectIndexTag();
}

int PlatformNode::acceptProjector(ProjectorNode* proj)
{
  // Stop accepting the previous projector node, if one exists
  if (acceptedProjectorNode_ != nullptr)
  {
    acceptedProjectorNode_->removeProjectionFromNode(model_->offsetNode());
    acceptedProjectorNode_ = nullptr;
  }

  // Passing in NULL clears the pairing, not an error
  if (proj == nullptr)
    return 0;

  int rv = proj->addProjectionToNode(this, model_->offsetNode());
  if (rv == 0)
    acceptedProjectorNode_ = proj;
  return rv;
}

}
