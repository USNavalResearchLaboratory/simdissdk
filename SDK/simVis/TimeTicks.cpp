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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
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
#include "simCore/Calc/Math.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/String.h"
#include "simData/DataTable.h"
#include "simVis/AlphaTest.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/PlatformFilter.h"
#include "simVis/Registry.h"
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
  static const double TICK_WIDTH = 40;
  static const int LABEL_FONT_SIZE = 30;

  // follow track history flat mode
  static const std::string SIMVIS_TIMETICKS_TRACK_FLATMODE = "simvis_track_flatmode";
}


/// constructor.
TimeTicks::TimeTicks(const simData::DataStore& ds, Locator* parentLocator, PlatformTspiFilterManager& platformTspiFilterManager, simData::ObjectId entityId)
  : ds_(ds),
  supportsShaders_(osgEarth::Registry::capabilities().supportsGLSL(3.3f)),
  chunkSize_(64),  // keep this lowish or your app won't scale.
  color_(osg::Vec4f(1.f, 1.f, 1.f, 0.5f)),
  textColor_(osg::Vec4f(1.f, 1.f, 1.f, 0.5f)),
  totalPoints_(0),
  singlePoint_(false),
  hasLastDrawTime_(false),
  lastDrawTime_(0.0),
  lastCurrentTime_(-1.0),
  lastLargeTickTime_(-1.0),
  largeTickInterval_(0.0),
  lastLabelTime_(-1.0),
  labelInterval_(0.0),
  timeDirection_(simCore::FORWARD),
  chunkGroup_(nullptr),
  labelGroup_(nullptr),
  platformTspiFilterManager_(platformTspiFilterManager),
  entityId_(entityId),
  currentPointChunk_(nullptr),
  parentLocator_(parentLocator)
{
  updateSliceBase_ = ds_.platformUpdateSlice(entityId);
  assert(updateSliceBase_); // should be a valid update slice before time tick is created

  localLocator_ = new Locator(parentLocator);
  setName("TrackHistory TimeTicks Group");
  setNodeMask(simVis::DISPLAY_MASK_TRACK_HISTORY);

  reset();

  // configure the local state set
  simVis::setLighting(getOrCreateStateSet(), 0);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
}

TimeTicks::~TimeTicks()
{
  chunkGroup_ = nullptr;
  labelGroup_ = nullptr;
  currentPointChunk_ = nullptr;
  parentLocator_ = nullptr;
  localLocator_ = nullptr;
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
  currentPointChunk_ = nullptr;
  lastLargeTickTime_ = -1.0;
  lastLabelTime_ = -1.0;
  singlePoint_ = false;
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
  return nullptr;
}

TimeTicksChunk* TimeTicks::getLastChunk_()
{
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TimeTicksChunk* chunk = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(num - 1));
    return chunk;
  }
  return nullptr;
}

TimeTicksChunk* TimeTicks::getFirstChunk_()
{
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TimeTicksChunk* chunk = static_cast<TimeTicksChunk*>(chunkGroup_->getChild(0));
    return chunk;
  }
  return nullptr;
}

double TimeTicks::toDrawTime_(double updateTime) const
{
  return updateTime * ((timeDirection_ == simCore::REVERSE) ? -1.0 : 1.0);
}

void TimeTicks::addUpdate_(double tickTime)
{
  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
  if (updateSlice == nullptr)
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
  const simData::PlatformUpdate* update = iter.next();
  simCore::Coordinate ecefTickCoord;
  bool largeTick = false;

  // if tick is at first platform point, use that for position
  if (!hasPrevious)
  {
    // if only a single point, use it
    const simData::PlatformUpdate* next = iter.next();
    if (!next)
    {
      if (!getTickCoord_(*update, ecefTickCoord))
        return;
      // if drawing line ticks, set singlePoint_ flag since orientation may not be correct and will need to update once next point comes in
      // this may occur in live mode
      if (lastPlatformPrefs_.trackprefs().timeticks().drawstyle() == simData::TimeTickPrefs::LINE)
        singlePoint_ = true;
    }
    // use the next point to calculate the correct orientation for the first tick
    else
    {
      if (!getTickCoord_(*next, *update, tickTime, ecefTickCoord))
        return;
      singlePoint_ = false;
    }
  }
  // not first tick, or not at first platform position, get the next position, possibly interpolated
  else
  {
    const simData::PlatformUpdate* prev = prevIter.previous();
    singlePoint_ = false;
    if (!getTickCoord_(*prev, *update, tickTime, ecefTickCoord))
      return;
  }

  // check to see if it is time for the next large tick
  if (largeTickInterval_ > 0 && (lastLargeTickTime_ == -1 || fabs(tickTime - lastLargeTickTime_) >= largeTickInterval_))
  {
    lastLargeTickTime_ = tickTime;
    largeTick = true;
  }

  // only need a new locator in two cases
  const bool addLabel = (labelInterval_ > 0 && (lastLabelTime_ == -1.0 || fabs(tickTime - lastLabelTime_) >= labelInterval_));
  const bool newChunk = (!getCurrentChunk_());
  osg::ref_ptr<Locator> newLocator;
  if (addLabel || newChunk)
  {
    // these are the only two cases that require a new locator
    newLocator = new Locator(parentLocator_.get());
    newLocator->setCoordinate(ecefTickCoord, tickTime);
    if (parentLocator_->isEci())
      newLocator->setEciRotationTime(-tickTime, tickTime, false);
  }
  else
  {
    localLocator_->setCoordinate(ecefTickCoord, tickTime);
    if (localLocator_->isEci())
      localLocator_->setEciRotationTime(-tickTime, tickTime, false);
  }

  // add label for large tick
  if (addLabel)
  {
    // code above guarantees this
    assert(newLocator.get());

    lastLabelTime_ = tickTime;

    // get formatted time string
    int refYear = 1970;
    {
      simData::DataStore::Transaction t;
      const simData::ScenarioProperties* sp = ds_.scenarioProperties(&t);
      if (sp)
        refYear = sp->referenceyear();
    }
    const simCore::TimeStamp textTime(refYear, tickTime);
    std::string labelText;
    const simData::TimeTickPrefs& timeTicks = lastPlatformPrefs_.trackprefs().timeticks();
    const simData::ElapsedTimeFormat timeFormat = timeTicks.labeltimeformat();
    // show HH:MM:SS
    if (timeFormat == simData::ELAPSED_HOURS)
    {
      simCore::HoursWrappedTimeFormatter formatter;
      labelText = formatter.toString(textTime, refYear, 0);
    }

    // show MM:SS
    else if (timeFormat == simData::ELAPSED_MINUTES)
    {
      simCore::MinutesWrappedTimeFormatter formatter;
      labelText = formatter.toString(textTime, refYear, 0);
    }

    // show SS
    else
    {
      simCore::SecondsTimeFormatter formatter;
      labelText = formatter.toString(textTime, refYear, 0);
    }

    // locator node syncs its matrix to locator immediately
    LocatorNode* locNode = new LocatorNode(newLocator.get());
    osgText::Text* text = new osgText::Text();
    text->setPosition(osg::Vec3(0.f, 0.f, 0.f));
    text->setText(labelText);
    std::string fileFullPath = simVis::Registry::instance()->findFontFile(timeTicks.labelfontname());
    if (!fileFullPath.empty()) // only set if font file found, use default font otherwise
      text->setFont(fileFullPath);
    else
      text->setFont(osgEarth::Registry::instance()->getDefaultFont());
    text->setAutoRotateToScreen(true);
    text->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
    text->setAlignment(osgText::TextBase::RIGHT_BOTTOM);
    text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    text->setCharacterSize(timeTicks.labelfontpointsize());
    text->getOrCreateStateSet()->setRenderBinDetails(simVis::BIN_LABEL, simVis::BIN_TRAVERSAL_ORDER_SIMSDK);
    osg::Depth* noDepthTest = new osg::Depth(osg::Depth::ALWAYS, 0, 1, false);
    text->getOrCreateStateSet()->setAttributeAndModes(noDepthTest, 1);
    text->setColor(textColor_);
    locNode->addChild(text);
    labelGroup_->addChild(locNode);
    labels_[toDrawTime_(tickTime)] = locNode;
  }

  // get a chunk to which to add the new point, creating a new one if necessary
  TimeTicksChunk* chunk = getCurrentChunk_();
  if (!chunk)
  {
    // code above guarantees that newLocator created for this case
    assert(newLocator.get());

    // allocate a new chunk
    const simData::TimeTickPrefs& timeTicks = lastPlatformPrefs_.trackprefs().timeticks();
    TimeTicksChunk::Type type = ((timeTicks.drawstyle() == simData::TimeTickPrefs::POINT) ? TimeTicksChunk::POINT_TICKS : TimeTicksChunk::LINE_TICKS);
    chunk = new TimeTicksChunk(chunkSize_, type, timeTicks.linelength() / 2, timeTicks.linewidth(), timeTicks.largesizefactor());
    // set the new chunk's locator - this establishes the position of the chunk
    chunk->setLocator(newLocator.get());
    // add the new chunk and update its appearance
    chunkGroup_->addChild(chunk);
    chunk->addCullCallback(new osgEarth::HorizonCullCallback());
  }

  const double drawTime = toDrawTime_(tickTime);

  // add the point (along with its timestamp)
  osg::ref_ptr<Locator> ptLocator = (newLocator.get()) ? newLocator : localLocator_;
  const bool addSuccess = chunk->addPoint(*ptLocator, drawTime, color_, largeTick);
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
      if (chunkGroup_->getNumChildren() == 0)
        assert(totalPoints_ == 0); // removal logic is faulty in chunk
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
  const simData::TimeTickPrefs& timeTicks = prefs.timeticks();
  const simData::TimeTickPrefs& lastTimeTicks = lastPrefs.timeticks();

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

  // check for any clamping changes, which will redraw ticks
  if (force || PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, useclampalt) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmin) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmax) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, surfaceclamping))
  {
    // Did not test for the clamped angles since they are intended for stationary platforms
    resetRequested = true;
  }

  if (force || PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, linewidth))
  {
    osg::StateSet* stateSet = this->getOrCreateStateSet();
    osgEarth::LineDrawable::setLineWidth(stateSet, timeTicks.linewidth());
    // need to redraw points if line width changed
    if (timeTicks.drawstyle() == simData::TimeTickPrefs::POINT)
      resetRequested = true;
  }

  // update the tick color and label color
  if (force || PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, color) || PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, labelcolor))
  {
    color_ = simVis::Color(timeTicks.color(), simVis::Color::RGBA);

    // If label color is set (non-zero) use it. If not, fall back to the line color for labels
    if (timeTicks.labelcolor() != 0)
      textColor_ = simVis::Color(timeTicks.labelcolor(), simVis::Color::RGBA);
    else
      textColor_ = color_;

    resetRequested = true;
  }

  if (force || PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, interval) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, largeintervalfactor) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, labelintervalfactor))
  {
    double interval = timeTicks.interval();
    largeTickInterval_ = interval * timeTicks.largeintervalfactor();
    labelInterval_ = interval * timeTicks.labelintervalfactor();
    resetRequested = true;
  }
  // check on other changes that could force a redraw
  if (force || PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, drawstyle) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, linelength) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, largesizefactor) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, labelfontname) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, labelfontpointsize) ||
    PB_FIELD_CHANGED(&lastTimeTicks, &timeTicks, labeltimeformat))
  {
    resetRequested = true;
  }

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
  if (updateSlice == nullptr)
  {
    // a valid/active platform must have an updateSlice - if assert fails, ensure that time ticks is not being updated for a non valid platform
    assert(0);
    return;
  }

  // if the current is not valid, and scenario is prior to first update time, nothing to do
  if (updateSlice->current() == nullptr && ds_.updateTime() < updateSlice->firstTime())
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

  // first tick was built from a single point, so orientation may have been off, reset to ensure first tick is drawn with correct orientation
  if (singlePoint_ && updateSlice.numItems() > 1)
    reset();

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
  // check last draw time again, since it may have been updated in the block above
  if (!hasLastDrawTime_)
  {
    // time ticks should all be referenced from 0:00:00.000
    double firstTime = 0.0;

    // update begin time to always count up to a valid draw time from first time in case data limiting is occurring
    const double interval = lastPlatformPrefs_.trackprefs().timeticks().interval();
    if (beginTime != firstTime)
    {
      const double beginSpan = beginTime - firstTime;
      double numIntervals = floor(beginSpan / interval);
      beginTime = firstTime + (numIntervals * interval);
      beginTime += interval;

      if (timeDirection_ == simCore::FORWARD)
      {
        // update last large tick time and labels to ensure they are always drawn consistently
        numIntervals = floor(beginSpan / largeTickInterval_);
        lastLargeTickTime_ = firstTime + (numIntervals * largeTickInterval_);
        numIntervals = floor(beginSpan / labelInterval_);
        lastLabelTime_ = firstTime + (numIntervals * labelInterval_);
      }
    }
    if (timeDirection_ == simCore::REVERSE)
    {
      // set the end tick time to ensure it is at a valid interval from the first time
      const double endSpan = endTime - firstTime;
      double numIntervals = floor(endSpan / interval);
      endTime = firstTime + (numIntervals * interval);

      // update last large tick time and labels to ensure they are always drawn consistently, make sure they are one interval past the current valid end time
      numIntervals = floor(endSpan / largeTickInterval_);
      lastLargeTickTime_ = firstTime + (numIntervals * largeTickInterval_);
      lastLargeTickTime_ += largeTickInterval_;
      numIntervals = floor(endSpan / labelInterval_);
      lastLabelTime_ = firstTime + (numIntervals * labelInterval_);
      lastLabelTime_ += labelInterval_;
    }
  }

  // store currentTime to enable time jump detection
  lastCurrentTime_ = currentTime;

  // update time ticks with points in the requested window
  backfillHistory_(endTime, beginTime);
}

void TimeTicks::backfillHistory_(double endTime, double beginTime)
{
  const double interval = lastPlatformPrefs_.trackprefs().timeticks().interval();
  if (timeDirection_ == simCore::FORWARD)
  {
    double tickTime = beginTime;
    // if there is already a chunk, start counting from its end time
    simVis::TimeTicksChunk* lastChunk = getLastChunk_();
    if (lastChunk)
      tickTime = lastChunk->getEndTime() + interval;

    while (tickTime <= endTime)
    {
      addUpdate_(tickTime);
      tickTime += interval;
    }
  }
  else
  {
    double tickTime = endTime;
    // if there is already a chunk, start counting from its end time
    simVis::TimeTicksChunk* lastChunk = getLastChunk_();
    if (lastChunk)
      tickTime = toDrawTime_(lastChunk->getEndTime()) - interval;

    while (tickTime >= beginTime)
    {
      addUpdate_(tickTime);
      tickTime -= interval;
    }
  }
}

bool TimeTicks::getTickCoord_(const simData::PlatformUpdate& u, simCore::Coordinate& ecefTickCoord)
{
  simData::PlatformUpdate update = u;
  if (platformTspiFilterManager_.filter(update, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED)
    return false;

  // update our locator for the current update
  ecefTickCoord = simCore::Coordinate(
    simCore::COORD_SYS_ECEF,
    simCore::Vec3(update.x(), update.y(), update.z()),
    simCore::Vec3(update.psi(), update.theta(), update.phi()));

  return true;
}

bool TimeTicks::getTickCoord_(const simData::PlatformUpdate& prevPoint, const simData::PlatformUpdate& curPoint, double time, simCore::Coordinate& ecefTickCoord)
{
  simData::PlatformUpdate prevUpdate = prevPoint;
  simData::PlatformUpdate curUpdate = curPoint;
  // apply filters, which may change position values
  if (platformTspiFilterManager_.filter(curUpdate, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED
    || platformTspiFilterManager_.filter(prevUpdate, lastPlatformPrefs_, lastPlatformProps_) == PlatformTspiFilterManager::POINT_DROPPED)
    return false;

  simData::PlatformUpdate platformUpdate = curUpdate;
  if (curUpdate.time() != time && ds_.interpolator())
    ds_.interpolator()->interpolate(time, prevUpdate, curUpdate, &platformUpdate);

  // for point ticks, only need the position
  if (lastPlatformPrefs_.trackprefs().timeticks().drawstyle() == simData::TimeTickPrefs::POINT)
  {
    ecefTickCoord = simCore::Coordinate(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate.x(), platformUpdate.y(), platformUpdate.z()));
    return true;
  }

  // for line ticks, need to calculate the orientation as well as the position
  simCore::Coordinate ecefCoordCur(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate.x(), platformUpdate.y(), platformUpdate.z()));
  simCore::Coordinate ecefCoordPrev(simCore::COORD_SYS_ECEF, simCore::Vec3(prevUpdate.x(), prevUpdate.y(), prevUpdate.z()));
  simCore::Coordinate llaCoordPrev;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoordPrev, llaCoordPrev);
  simCore::Coordinate llaCoordCur;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoordCur, llaCoordCur);

  double az = 0;
  simCore::sodanoInverse(llaCoordPrev.lat(), llaCoordPrev.lon(), 0, llaCoordCur.lat(), llaCoordCur.lon(), &az);
  simCore::Vec3 ecefOri;
  simCore::CoordinateConverter::convertGeodeticOriToEcef(llaCoordCur.position(), simCore::Vec3(az, 0, 0), ecefOri);

  ecefTickCoord = simCore::Coordinate(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate.x(), platformUpdate.y(), platformUpdate.z()), ecefOri);
  return true;
}
}
