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
#include <cassert>

#include "osg/Depth"
#include "osgText/Text"
#include "osgEarth/Capabilities"
#include "osgEarth/Horizon"
#include "osgEarth/LabelNode"
#include "osgEarth/LineDrawable"
#include "osgEarth/Registry"
#include "osgEarth/VirtualProgram"

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/String.h"
#include "simData/DataTable.h"
#include "simVis/AlphaTest.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/PlatformFilter.h"
#include "simVis/PointSize.h"
#include "simVis/Shaders.h"
#include "simVis/TimeTicksChunk.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/TimeTicks.h"

namespace simVis
{

namespace
{
  // TODO: these will be prefs, SIM-4428
  static const double TIME_INTERVAL_SMALL_SECS = 10;
  static const double TIME_INTERVAL_LARGE_SECS = 6 * TIME_INTERVAL_SMALL_SECS;
  static const int TICK_TYPE = 1; // 0 = point, 1 = line
  static const double TICK_WIDTH = 40;
  static const double TICK_LINE_WIDTH = 2;
  static const int LABEL_FONT_SIZE = 30;

  // follow track history flat mode
  static const std::string SIMVIS_TIMETICKS_TRACK_FLATMODE = "simvis_track_flatmode";
}


/// constructor.
TimeTicks::TimeTicks(const simData::DataStore& ds, const osgEarth::SpatialReference* srs, PlatformTspiFilterManager& platformTspiFilterManager, simData::ObjectId entityId)
  : ds_(ds),
   supportsShaders_(osgEarth::Registry::capabilities().supportsGLSL(3.3f)),
   chunkSize_(64),  // keep this lowish or your app won't scale.
   color_(osg::Vec4f(1.0, 1.0, 1.0, 0.5)),
   totalPoints_(0),
   hasLastDrawTime_(false),
   lastDrawTime_(0.0),
   lastCurrentTime_(-1.0),
   lastLargeTickTime_(-1.0),
   timeDirection_(simCore::FORWARD),
   chunkGroup_(NULL),
   labelGroup_(NULL),
   platformTspiFilterManager_(platformTspiFilterManager),
   entityId_(entityId),
   currentPointChunk_(NULL)
{
  updateSliceBase_ = ds_.platformUpdateSlice(entityId);
  assert(updateSliceBase_); // should be a valid update slice before time tick is created

  locator_ = new simVis::Locator(srs);

  setNodeMask(simVis::DISPLAY_MASK_TRACK_HISTORY);

  reset();

  // configure the local state set
  simVis::setLighting(getOrCreateStateSet(), 0);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
}

TimeTicks::~TimeTicks()
{
  chunkGroup_ = NULL;
  labelGroup_ = NULL;
  currentPointChunk_ = NULL;
  locator_ = NULL;
}

void TimeTicks::reset()
{
  // blow everything away
  removeChildren(0, getNumChildren());
  labels_.clear();

  hasLastDrawTime_   = false;
  lastCurrentTime_   = -1.0;
  totalPoints_       = 0;
  chunkGroup_ = new osg::Group();
  labelGroup_ = new osg::Group();
  addChild(labelGroup_);
  addChild(chunkGroup_);
  currentPointChunk_ = NULL;
  lastLargeTickTime_ = -1.0;
}

TimeTicksChunk* TimeTicks::getCurrentChunk_()
{
  // see if there's already a chunk we can use.
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TimeTicksChunk* chunk = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(num - 1));
    if (!chunk->isFull())
    {
      // yes.
      return chunk;
    }
  }
  return NULL;
}

TimeTicksChunk* TimeTicks::getLastChunk_()
{
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TimeTicksChunk* chunk = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(num - 1));
    return chunk;
  }
  return NULL;
}

TimeTicksChunk* TimeTicks::getFirstChunk_()
{
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TimeTicksChunk* chunk = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(0));
    return chunk;
  }
  return NULL;
}

double TimeTicks::toDrawTime_(double updateTime) const
{
  return updateTime * ((timeDirection_ == simCore::REVERSE) ? -1.0 : 1.0);
}

void TimeTicks::addUpdate_(double tickTime)
{
  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
  if (updateSlice == NULL)
  {
    // a valid/active platform must have an updateSlice - if assert fails, ensure that time ticks is not being updated for a non valid platform
    assert(0);
    return;
  }
  auto iter = updateSlice->lower_bound(tickTime);
  if (!iter.hasNext())
    return;

  bool hasPrevious = iter.hasPrevious();
  auto prevIter = iter;
  const simData::PlatformUpdate* prev = prevIter.previous();
  const simData::PlatformUpdate* update = iter.next();
  osg::Matrix hostMatrix;

  bool largeTick = false;
  // if first tick, get the current platform position
  if (chunkGroup_->getNumChildren() == 0)
  {
    if (!getMatrix_(*update, hostMatrix))
      return;
    // first tick is always large
    if (lastLargeTickTime_ == -1)
    {
      largeTick = true;
      lastLargeTickTime_ = tickTime;
    }
  }
  // not first tick, get the next position, possibly interpolated
  else if (hasPrevious)
  {
    if (!getMatrix_(*prev, *update, tickTime, hostMatrix))
      return;
    // check to see if it is time for the next large tick
    if (lastLargeTickTime_ == -1.0 || abs(tickTime - lastLargeTickTime_) >= TIME_INTERVAL_LARGE_SECS)
    {
      lastLargeTickTime_ = tickTime;
      largeTick = true;
    }
  }
  else
    return;

  // add label for large tick TODO: SIM-4428 this might be a separate pref to define label interval
  if (largeTick)
  {
    int refYear = 1970;
    simData::DataStore::Transaction t;
    const simData::ScenarioProperties* sp = ds_.scenarioProperties(&t);
    if (sp)
      refYear = sp->referenceyear();

    simCore::TimeStamp textTime(refYear, tickTime);
    simCore::HoursTimeFormatter fmt;
    std::string labelText = fmt.toString(textTime, refYear, 0);

    osg::MatrixTransform* xform = new osg::MatrixTransform();
    osgText::Text* text = new osgText::Text();
    text->setPosition(osg::Vec3(0, 0, 0));
    text->setText(labelText);
    text->setFont(osgEarth::Registry::instance()->getDefaultFont());
    text->setAutoRotateToScreen(true);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAlignment(osgText::TextBase::LEFT_BOTTOM);
    text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    text->setCharacterSize(LABEL_FONT_SIZE);
    text->getOrCreateStateSet()->setRenderBinToInherit();
    osg::Depth* noDepthTest = new osg::Depth(osg::Depth::ALWAYS, 0, 1, false);
    text->getOrCreateStateSet()->setAttributeAndModes(noDepthTest, 1);
    xform->addChild(text);
    xform->setMatrix(hostMatrix);
    labelGroup_->addChild(xform);
    labels_[toDrawTime_(tickTime)] = xform;
  }

  // get a chunk to which to add the new point, creating a new one if necessary
  TimeTicksChunk* chunk = getCurrentChunk_();
  if (!chunk)
  {
    // allocate a new chunk
    TimeTicksChunk::Type type = ((TICK_TYPE == 0) ? TimeTicksChunk::POINT_TICKS : TimeTicksChunk::LINE_TICKS);
    chunk = new TimeTicksChunk(chunkSize_, type, TICK_WIDTH, TICK_WIDTH * 2);

    // if there is a preceding chunk, duplicate its last point so there is no
    // discontinuity from previous chunk to this new chunk - this matters for line drawing mode
    int numc = chunkGroup_->getNumChildren();
    if (numc > 0)
    {
      osg::Matrix last;
      if (getLastChunk_()->getEndMatrix(last) == 0)
      {
        double last_t = getLastChunk_()->getEndTime();
        chunk->addPoint(last, last_t, color_, largeTick);
      }
    }

    // add the new chunk and update its appearance
    chunkGroup_->addChild(chunk);
    chunk->addCullCallback(new osgEarth::HorizonCullCallback());
  }

  const double drawTime = toDrawTime_(tickTime);

  // add the point (along with its timestamp)
  bool addSuccess = chunk->addPoint(hostMatrix, drawTime, color_, largeTick);
  if (!addSuccess)
  {
    // if assert fails, check that getCurrentChunk_ and previous code ensure that either chunk is not full, or new chunk created
    assert(0);
  }
  else
    totalPoints_++;

  // record time of last draw update - must be an actual point time that can be found in the chunk
  // in forward mode, lastDrawTime_ represents the newest time tick
  // in reverse mode, lastDrawTime_ represents the earliest time tick
  lastDrawTime_ = drawTime;
  hasLastDrawTime_ = true;

}

void TimeTicks::updateClockMode(const simCore::Clock* clock)
{
  // STOP does not require any change in time ticks
  if (clock->timeDirection() == simCore::STOP)
    return;

  // we only care about fwd-rev, rev-fwd, including fwd-stop-rev and rev-stop-fwd
  if (timeDirection_ != clock->timeDirection())
  {
    // clear time ticks
    reset();
    timeDirection_ = clock->timeDirection();
    update();
  }
}

void TimeTicks::removePointsOlderThan_(double oldestDrawTime)
{
  if (!labels_.empty())
  {
    auto iter = labels_.begin();
    while (!labels_.empty() && iter->first < oldestDrawTime)
    {
      labelGroup_->removeChild(iter->second);
      labels_.erase(iter);
      iter = labels_.begin();
    }
  }
  while (chunkGroup_->getNumChildren() > 0)
  {
    TimeTicksChunk* oldest = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(0));
    unsigned int numRemoved = oldest->removePointsBefore(oldestDrawTime);
    totalPoints_ -= numRemoved;
    if (oldest->size() == 0)
    {
      chunkGroup_->removeChild(0, 1);
      if (chunkGroup_->getNumChildren() > 0)
      {
        // Last point was duplicated to prevent discontinuity, remove it
        static_cast<TimeTicksChunk*>(chunkGroup_->getChild(0))->removeOldestPoint();
      }
      else // removal logic is faulty in chunk
        assert(totalPoints_ == 0);
    }
    else
      break;
  }
}

void TimeTicks::updateVisibility_(const simData::TrackPrefs& prefs)
{
  const bool invisible = (prefs.trackdrawmode() == simData::TrackPrefs::OFF);
  setNodeMask(invisible ? simVis::DISPLAY_MASK_NONE : simVis::DISPLAY_MASK_TRACK_HISTORY);
}

void TimeTicks::updateFlatMode_(bool flatMode)
{
  if (!supportsShaders_)
    return;

  if (!flatModeUniform_.valid())
  {
    if (flatMode == false)
      return;    // Does not exist and not needed so return;

    osg::StateSet* stateset = this->getOrCreateStateSet();
    flatModeUniform_ = stateset->getOrCreateUniform(SIMVIS_TIMETICKS_TRACK_FLATMODE, osg::Uniform::BOOL);
  }

  flatModeUniform_->set(flatMode);
}

void TimeTicks::setPrefs(const simData::PlatformPrefs& platformPrefs, const simData::PlatformProperties& platformProps, bool force)
{
  const simData::TrackPrefs& prefs = platformPrefs.trackprefs();
  // lastPlatformPrefs_ will not have data that represents current state on initial call
  // force should be true in this case;
  // in any case, if force is set, we should not test on lastPlatformPrefs_
  const simData::TrackPrefs& lastPrefs = lastPlatformPrefs_.trackprefs();

  // platform should be deleting track when trackdrawmode turned off, this should never be called with trackdrawmode off
  // if assert fails, check platform setPrefs logic that processes prefs.trackprefs().trackdrawmode()
  assert(prefs.trackdrawmode() != simData::TrackPrefs_Mode_OFF);
  bool resetRequested = false;

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, tracklength))
  {
    // clear the time ticks and recreate
    resetRequested = true;
  }

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, flatmode))
  {
    updateFlatMode_(prefs.flatmode());
  }

  if (force || PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, useclampalt) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmin) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmax) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, surfaceclamping))
  {
    // Did not test for the clamped angles since they are intended for stationary platforms
    resetRequested = true;
  }

  // TODO: SIM-4428 update linewidth and other time tick specific prefs
  osg::StateSet* stateSet = this->getOrCreateStateSet();
  osgEarth::LineDrawable::setLineWidth(stateSet, TICK_LINE_WIDTH);

  lastPlatformPrefs_ = platformPrefs;
  lastPlatformProps_ = platformProps;

  if (resetRequested)
  {
    reset();
    update();
  }
  updateVisibility_(prefs);
}

void TimeTicks::update()
{
  // tracklength 0 means no time ticks are shown
  if (lastPlatformPrefs_.trackprefs().tracklength() == 0)
    return;

  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
  if (updateSlice == NULL)
  {
    // a valid/active platform must have an updateSlice - if assert fails, ensure that time ticks is not being updated for a non valid platform
    assert(0);
    return;
  }

  // if the current is not valid, and scenario is prior to first update time, nothing to do
  if (updateSlice->current() == NULL && ds_.updateTime() < updateSlice->firstTime())
  {
    // platform is not valid, this should only occur during platform creation
    return;
  }

  // ignore static platforms
  if (updateSlice->current() && updateSlice->current()->time() == -1)
  {
    // time ticks should never be created for a static platform. - see PlatformNode::createTimeTicks_
    assert(0);
    return;
  }

  // update time ticks to match current time window
  updateTrackData_(ds_.updateTime(), *updateSlice);
}

void TimeTicks::updateTrackData_(double currentTime, const simData::PlatformUpdateSlice& updateSlice)
{
  // determine the time window that time ticks should display
  double endTime = currentTime;
  double beginTime = updateSlice.firstTime();
  int trackLength = lastPlatformPrefs_.trackprefs().tracklength();
  if (trackLength > 0 && (endTime - trackLength) > beginTime)
    beginTime = endTime - trackLength;

  // if there is an existing time ticks, determine if we can add only new points; this should be the case for normal time movement
  if (hasLastDrawTime_)
  {
    if (timeDirection_ == simCore::FORWARD)
    {
      // backward jump (e.g. time slider move) while in forward mode in time requires reset()
      if (currentTime < lastCurrentTime_)
      {
        reset();
        // assume now going in reverse, so switch direction
        timeDirection_ = simCore::REVERSE;
      }
      else
      {
        // enforce tracklength/data limiting prefs - remove all points older than new begin time
        removePointsOlderThan_(beginTime);
        // if new window overlaps previous window, then reuse existing points, only add the new points
        if (lastDrawTime_ >= beginTime)
        {
          // adjust beginTime so that we add only new points, avoid re-adding points that are still in the history
          beginTime = FLT_EPSILON + lastDrawTime_;
        }
      }
    }
    else if (timeDirection_ == simCore::REVERSE)
    {
      // forward jump in time (e.g. time slider move) while in reverse mode requires reset
      if (currentTime > lastCurrentTime_)
      {
        reset();
        // assume now going forward, so switch direction
        timeDirection_ = simCore::FORWARD;
      }
      else
      {
        // remove all points with drawtime "older" than reverse mode end drawtime; i.e., remove all points with time newer than current time
        removePointsOlderThan_(toDrawTime_(endTime));
        // if new window overlaps previous window, then reuse existing points, only add the new points
        if (toDrawTime_(lastDrawTime_) <= endTime)
        {
          // adjust endTime so that we add only new points, and avoid re-adding points that are still in the history
          endTime = -FLT_EPSILON + toDrawTime_(lastDrawTime_);
        }
      }
    }
  }

  // store currentTime to enable time jump detection
  lastCurrentTime_ = currentTime;

  // update time ticks with points in the requested window
  backfillHistory_(endTime, beginTime);

  // when the current point is interpolated, line mode requires special processing
  updateCurrentPoint_(updateSlice, beginTime);
}

void TimeTicks::backfillHistory_(double endTime, double beginTime)
{
  if (timeDirection_ == simCore::FORWARD)
  {
    double tickTime = beginTime;
    // if there is already a chunk, start counting from its end time
    simVis::TimeTicksChunk* lastChunk = getLastChunk_();
    if (lastChunk)
      tickTime = lastChunk->getEndTime() + TIME_INTERVAL_SMALL_SECS;

    while (tickTime <= endTime)
    {
      addUpdate_(tickTime);
      tickTime += TIME_INTERVAL_SMALL_SECS;
    }
  }
  else
  {
    double tickTime = endTime;
    // if there is already a chunk, start counting from its end time
    simVis::TimeTicksChunk* lastChunk = getLastChunk_();
    if (lastChunk)
      tickTime = toDrawTime_(lastChunk->getEndTime()) - TIME_INTERVAL_SMALL_SECS;

    while (tickTime >= beginTime)
    {
      addUpdate_(tickTime);
      tickTime -= TIME_INTERVAL_SMALL_SECS;
    }
  }
}

void TimeTicks::updateCurrentPoint_(const simData::PlatformUpdateSlice& updateSlice, double beginTime)
{
  // remove previous, will recreate if needed
  if (currentPointChunk_ != NULL)
    currentPointChunk_->reset();

  // no current point if not line ticks
  if (TICK_TYPE == 0)
    return;

  // only line, ribbon, and bridge draw modes require this processing,
  // but if there is not a previous point, there is nothing to do
  if (chunkGroup_->getNumChildren() == 0)
    return;

  // create the special chunk for rendering the interpolated point, has two points to connect to rest of history for line mode
  if (currentPointChunk_ == NULL)
  {
    currentPointChunk_ = new TimeTicksChunk(2, TimeTicksChunk::LINE, TICK_WIDTH, TICK_WIDTH * 2);
    if (currentPointChunk_ == NULL)
      return;
    addChild(currentPointChunk_);
  }
  // find the most current update, either whatever is current, or the last available update
  const simData::PlatformUpdate* current = updateSlice.current();
  if (current == NULL)
  {
    simData::PlatformUpdateSlice::Iterator iter = updateSlice.lower_bound(updateSlice.lastTime());
    current = iter.next();
  }
  osg::Matrix curMatrix;

  // if this fails, this platform node got created with no platform data
  assert(current);

  if (!getMatrix_(*current, curMatrix))
  {
    // failed to get the current position, check locator
    assert(0);
    return;
  }

  if (timeDirection_ == simCore::FORWARD)
  {
    TimeTicksChunk* curChunk = getLastChunk_();
    // should have a last chunk if chunk group is not empty
    assert(curChunk != NULL);
    osg::Matrix end;
    if (curChunk->getEndMatrix(end) == 0)
      currentPointChunk_->addPoint(end, curChunk->getEndTime(), color_, false);
    else
      assert(0); // chunk is out of sync
  }

  // points must be added in order of increasing drawTime
  currentPointChunk_->addPoint(curMatrix, toDrawTime_(beginTime), color_, false);

  // duplicate the most recent (non-current) datapoint so that this chunk connects to the previous chunk
  if (timeDirection_ == simCore::REVERSE)
  {
    TimeTicksChunk* curChunk = getLastChunk_();
    curChunk = getFirstChunk_();
    // should have a first chunk if chunk group is not empty
    assert(curChunk != NULL);
    osg::Matrix begin;
    if (curChunk->getBeginMatrix(begin) == 0)
      currentPointChunk_->addPoint(begin, curChunk->getBeginTime(), color_, false);
    else
      assert(0); // chunk is out of sync
  }
}

bool TimeTicks::getMatrix_(const simData::PlatformUpdate& u, osg::Matrix& hostMatrix)
{
  simData::PlatformUpdate update = u;
  if (platformTspiFilterManager_.filter(update, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED)
    return false;

  // update our locator for the current update
  simCore::Coordinate ecefCoord(
    simCore::COORD_SYS_ECEF,
    simCore::Vec3(update.x(), update.y(), update.z()),
    simCore::Vec3(update.psi(), update.theta(), update.phi()));

  locator_->setCoordinate(ecefCoord, u.time());

  // fetch the positioning matrix from the locator we are tracking
  if (!locator_->getLocatorMatrix(hostMatrix))
  {
    // if assert fails, check that invalid platform updates are not being sent to track
    assert(0);
    return false;
  }

  return true;
}

bool TimeTicks::getMatrix_(const simData::PlatformUpdate& prevPoint, const simData::PlatformUpdate& curPoint, double time, osg::Matrix& hostMatrix)
{
  simData::PlatformUpdate prevUpdate = prevPoint;
  simData::PlatformUpdate curUpdate = curPoint;
  // apply filters, which may change position values
  if (platformTspiFilterManager_.filter(curUpdate, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED
    || platformTspiFilterManager_.filter(prevUpdate, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED)
    return false;

  simData::Interpolator* li = ds_.interpolator();
  simData::PlatformUpdate platformUpdate = curUpdate;
  if (curUpdate.time() != time)
    li->interpolate(time, prevUpdate, curUpdate, &platformUpdate);

  simCore::Coordinate ecefCoordCur(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate.x(), platformUpdate.y(), platformUpdate.z()));

  // for point ticks, only need the position
  if (TICK_TYPE == 0)
  {
    locator_->setCoordinate(ecefCoordCur, time);

    // fetch the positioning matrix from the locator we are tracking
    if (locator_->getLocatorMatrix(hostMatrix))
      return true;
    else
    {
      // if assert fails, check that invalid platform updates are not being sent to track
      assert(0);
      return false;
    }
  }

  // for line ticks, need to calculate the orientation as well as the position

  simCore::Coordinate ecefCoordPrev(simCore::COORD_SYS_ECEF, simCore::Vec3(prevUpdate.x(), prevUpdate.y(), prevUpdate.z()));
  simCore::Coordinate llaCoordPrev;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoordPrev, llaCoordPrev);
  simCore::Coordinate llaCoordCur;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoordCur, llaCoordCur);

  double az = 0;
  simCore::sodanoInverse(llaCoordPrev.lat(), llaCoordPrev.lon(), 0, llaCoordCur.lat(), llaCoordCur.lon(), &az);
  simCore::Vec3 ecefOri;
  simCore::CoordinateConverter::convertGeodeticOriToEcef(llaCoordCur.position(), simCore::Vec3(az, 0, 0), ecefOri);
  // update our locator for the current update
  simCore::Coordinate finalCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate.x(), platformUpdate.y(), platformUpdate.z()), ecefOri);

  locator_->setCoordinate(finalCoord, time);

  // fetch the positioning matrix from the locator we are tracking
  if (!locator_->getLocatorMatrix(hostMatrix))
  {
    // if assert fails, check that invalid platform updates are not being sent to track
    assert(0);
    return false;
  }
  return true;
}

}
