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

#include "osgEarth/Capabilities"
#include "osgEarth/Horizon"
#include "osgEarth/Registry"
#include "osgEarth/VirtualProgram"
#include "simVis/LineDrawable.h"

#include "simNotify/Notify.h"
#include "simData/DataTable.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/PlatformFilter.h"
#include "simVis/PointSize.h"
#include "simVis/Shaders.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/TrackHistory.h"

namespace simVis
{

namespace
{
  static const std::string SIMVIS_TRACK_FLATMODE = "simvis_track_flatmode";
  static const std::string SIMVIS_TRACK_FLATRADIUS = "simvis_track_flatradius";
  static const std::string SIMVIS_TRACK_ENABLE = "simvis_track_enable";
  static const std::string SIMVIS_TRACK_OVERRIDE_COLOR = "simvis_track_overridecolor";
}

class TrackHistoryNode::ColorTableObserver : public simData::DataTableManager::ManagerObserver
{
public:
  explicit ColorTableObserver(TrackHistoryNode& parent)
    : parent_(parent)
  {}

  void onAddTable(simData::DataTable* table)
  {
    if ((table != NULL) && (table->ownerId() == parent_.entityId_) && (table->tableName() == simData::INTERNAL_TRACK_HISTORY_TABLE))
      parent_.initializeTableId_();
  }

  void onPreRemoveTable(simData::DataTable* table)
  {
    if (table != NULL && table->tableId() == parent_.tableId_)
    {
      parent_.tableId_ = 0;
      table->removeObserver(parent_.colorChangeObserver_);
    }
  }

private:
  TrackHistoryNode& parent_;
};

class TrackHistoryNode::ColorChangeObserver : public simData::DataTable::TableObserver
{
public:
  explicit ColorChangeObserver(TrackHistoryNode& parent)
    : parent_(parent)
  {}

  virtual void onAddColumn(simData::DataTable& table, const simData::TableColumn& column) {}

  virtual void onAddRow(simData::DataTable& table, const simData::TableRow& row)
  {
    parent_.checkColorHistoryChange_(table, row);
  }

  virtual void onPreRemoveColumn(simData::DataTable& table, const simData::TableColumn& column) {}

  virtual void onPreRemoveRow(simData::DataTable& table, double rowTime) {}

private:
  TrackHistoryNode& parent_;
};


/// constructor.
TrackHistoryNode::TrackHistoryNode(const simData::DataStore& ds, const osgEarth::SpatialReference* srs, PlatformTspiFilterManager& platformTspiFilterManager, simData::ObjectId entityId)
  : ds_(ds),
   supportsShaders_(osgEarth::Registry::capabilities().supportsGLSL(3.3f)),
   chunkSize_(64),  // keep this lowish or your app won't scale.
   totalPoints_(0),
   hasLastDrawTime_(false),
   lastDrawTime_(0.0),
   lastCurrentTime_(-1.0),
   timeDirection_(simCore::FORWARD),
   timeDirectionSign_(1.0),
   chunkGroup_(NULL),
   altModeXform_(NULL),
   platformTspiFilterManager_(platformTspiFilterManager),
   entityId_(entityId),
   tableId_(0),
   currentPointChunk_(NULL)
{
  updateSliceBase_ = ds_.platformUpdateSlice(entityId);
  assert(updateSliceBase_); // should be a valid update slice before track history is created

  defaultColor_ = simVis::Color(simData::PlatformPrefs::default_instance().trackprefs().trackcolor(), simVis::Color::RGBA);
  locator_ = new simVis::Locator(srs);

  setNodeMask(simVis::DISPLAY_MASK_TRACK_HISTORY);

  reset();

  // configure the local state set
  simVis::setLighting(getOrCreateStateSet(), 0);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);

  // try to initialize our data table id for finding the track history color
  initializeTableId_();

  colorTableObserver_.reset(new ColorTableObserver(*this));
  ds_.dataTableManager().addObserver(colorTableObserver_);
}

TrackHistoryNode::~TrackHistoryNode()
{
  ds_.dataTableManager().removeObserver(colorTableObserver_);
  if (tableId_ > 0)
  {
    simData::DataTable* table = ds_.dataTableManager().getTable(tableId_);
    if (table != NULL)
      table->removeObserver(colorChangeObserver_);
  }
  chunkGroup_ = NULL;
  currentPointChunk_ = NULL;
  locator_ = NULL;
}

// handle an explicit reset
void TrackHistoryNode::reset()
{
  // blow everything away
  this->removeChildren(0, this->getNumChildren());
  hasLastDrawTime_   = false;
  lastCurrentTime_   = -1.0;
  totalPoints_       = 0;
  altModeXform_      = NULL;
  chunkGroup_ = new osg::Group();
  this->addChild(chunkGroup_);
  currentPointChunk_ = NULL;
}

void TrackHistoryNode::checkColorHistoryChange_(const simData::DataTable& table, const simData::TableRow& row)
{
  simData::TableColumn* col = table.column(simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN);
  if (col == NULL)
  {
    assert(0); // if the table exists, the column should exist
    return;
  }
  // ensure that this row has a value for the track color history column
  if (!row.containsCell(col->columnId()))
    return;

  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);

  // there might be no current data after a flush, if a color command is added before any new update data
  if (updateSlice == NULL || updateSlice->current() == NULL)
    return;

  // if this row is not in the span of our slice, don't bother to reset
  if (row.time() > updateSlice->current()->time() || row.time() < updateSlice->firstTime())
    return;

  // a track history color changed, rebuild the history points
  // NOTE: may want to queue up this reset request and execute it later, maybe using an OSG fire-once callback,
  // to mitigate performance when many track color commands are merged in
  reset();
  update();
}

/**
* Return a chunk to which you can add a new point, or NULL if one needs to be created for that addition.
*/
TrackChunkNode* TrackHistoryNode::getCurrentChunk_()
{
  // see if there's already a chunk we can use.
  unsigned int num = chunkGroup_->getNumChildren();
  if (num > 0)
  {
    TrackChunkNode* chunk = static_cast<TrackChunkNode*>(chunkGroup_->getChild(num - 1));
    if (!chunk->isFull())
    {
      // yes.
      return chunk;
    }
  }
  return NULL;
}

osg::Vec4f TrackHistoryNode::historyColorAtTime_(double time)
{
  // if not using shaders for override color, and there is a visible override color to apply
  if (!supportsShaders_ && lastOverrideColor_.a() > 0.f)
    return lastOverrideColor_;

  // time may be negative in reverse clock mode, so alway adjust to normal time
  if (time < 0)
    time *= -1;
  if (tableId_ == 0)
    return defaultColor_;

  // find the table
  // use the tableId_ if we have it
  simData::DataTable* table = ds_.dataTableManager().getTable(tableId_);
  // no color history table, simply return the default color
  if (table == NULL)
  {
    assert(0); // table id is no longer valid, somehow the table got removed
    return defaultColor_;
  }
  simData::TableColumn* column = table->column(simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN);
  if (column == NULL)
  {
    assert(0); // found the table, but missing the expected data column
    return defaultColor_;
  }

  simData::TableColumn::Iterator iter = column->findAtOrBeforeTime(time);
  if (!iter.hasNext())
    return defaultColor_;

  uint32_t color;
  if (iter.next()->getValue(color).isError())
  {
    assert(0); // getValue failed, but hasNext claims it exists
    return defaultColor_;
  }
  return simVis::Color(color, simVis::Color::RGBA);
}

void TrackHistoryNode::initializeTableId_()
{
  // try to initialize the table id for quick access to the internal table, if it exists
  if (tableId_ != 0)
    return;
  simData::DataTable* table = ds_.dataTableManager().findTable(entityId_, simData::INTERNAL_TRACK_HISTORY_TABLE);
  if (table == NULL)
    return;
  tableId_ = table->tableId();
  assert(tableId_ > 0); // a table was created with an invalid table id
  colorChangeObserver_.reset(new ColorChangeObserver(*this));
  table->addObserver(colorChangeObserver_);
}

double TrackHistoryNode::toDrawTime_(double updateTime) const
{
  return updateTime * timeDirectionSign_;
}

// update the track given a new host update.
void TrackHistoryNode::addUpdate_(const simData::PlatformUpdate& u, const simData::PlatformUpdate* prevUpdate)
{
  osg::Matrix hostMatrix;
  if (!getMatrix_(u, hostMatrix))
    return;

  // get a chunk to which to add the new point, creating a new one if necessary
  TrackChunkNode* chunk = getCurrentChunk_();
  if (!chunk)
  {
    // allocate a new chunk
    chunk = new TrackChunkNode(chunkSize_, locator_->getSRS(), lastPlatformPrefs_.trackprefs().trackdrawmode());

    // if there is a preceding chunk, duplicate its last point so there is no
    // discontinuity from previous chunk to this new chunk - this matters for line, bridge drawing modes
    int numc = chunkGroup_->getNumChildren();
    if (numc > 0 && prevUpdate != NULL)
    {
      osg::Matrix last;
      double last_t = prevUpdate->time();
      // Extra point needs to be removed during data limiting
      if (getMatrix_(*prevUpdate, last))
        chunk->addPoint(last, last_t, historyColorAtTime_(last_t), hostBounds_);
    }

    // add the new chunk and update its appearance
    chunkGroup_->addChild(chunk);
    chunk->addCullCallback(new osgEarth::HorizonCullCallback());
  }

  const double drawTime = toDrawTime_(u.time());

  // add the point (along with its timestamp)
  bool addSuccess = chunk->addPoint(hostMatrix, drawTime, historyColorAtTime_(drawTime), hostBounds_);
  if (!addSuccess)
  {
    // if assert fails, check that getCurrentChunk_ and previous code ensure that either chunk is not full, or new chunk created
    assert(0);
  }
  else
    totalPoints_++;

  // record time of last draw update - must be an actual point time that can be found in the chunk
  // in forward mode, lastDrawTime_ represents the newest point in the track history
  // in reverse mode, lastDrawTime_ represents the earliest point in the track history
  lastDrawTime_ = drawTime;
  hasLastDrawTime_ = true;
}

void TrackHistoryNode::updateClockMode(const simCore::Clock* clock)
{
  // STOP does not require any change in track history
  if (clock->timeDirection() == simCore::STOP)
    return;

  // we only care about fwd-rev, rev-fwd, including fwd-stop-rev and rev-stop-fwd
  if (timeDirection_ != clock->timeDirection())
  {
    // clear track history
    reset();
    timeDirection_ = clock->timeDirection();
    timeDirectionSign_ = (timeDirection_ == simCore::REVERSE) ? -1.0 : 1.0;
    update();
  }
}

/// prune the point set of all entries older than
void TrackHistoryNode::removePointsOlderThan_(double oldestDrawTime)
{
  while (chunkGroup_->getNumChildren() > 0)
  {
    TrackChunkNode* oldest = static_cast<TrackChunkNode*>(chunkGroup_->getChild(0));
    unsigned int numRemoved = oldest->removePointsBefore(oldestDrawTime);
    totalPoints_ -= numRemoved;
    if (oldest->size() == 0)
    {
      chunkGroup_->removeChild(0, 1);
      if (chunkGroup_->getNumChildren() > 0)
      {
        // Last point was duplicated to prevent discontinuity, remove it
        static_cast<TrackChunkNode*>(chunkGroup_->getChild(0))->removeOldestPoint();
      }
      else
        assert(totalPoints_ == 0);
    }
    else
      break;
  }
}

void TrackHistoryNode::updateVisibility_(const simData::TrackPrefs& prefs)
{
  const bool invisible = (prefs.trackdrawmode() == simData::TrackPrefs::OFF);
  setNodeMask(invisible ? simVis::DISPLAY_MASK_NONE : simVis::DISPLAY_MASK_TRACK_HISTORY);
}

void TrackHistoryNode::updateAltMode_(bool altmode, const simData::PlatformUpdateSlice& updateSlice)
{
  // create the altmode group if necessary:
  if (altmode && altModeXform_ == NULL)
  {
    dropVertsDrawable_ = new osgEarth::LineDrawable(GL_LINES);
    dropVertsDrawable_->setColor(simVis::Color::White);
    dropVertsDrawable_->allocate(2);
    dropVertsDrawable_->dirty();

    altModeXform_ = new osg::MatrixTransform();
    altModeXform_->addChild(dropVertsDrawable_.get());

    this->addChild(altModeXform_);
  }

  if (altModeXform_)
  {
    bool show = altmode;
    if (updateSlice.current() && altmode)
    {
      osg::Matrix pos;
      if (getMatrix_(*updateSlice.current(), pos))
        updateAltModePositionAndAppearance_(pos, historyColorAtTime_(updateSlice.current()->time()));
    }
    else // if there is no current data, then don't show the altimeter
      show = false;

    altModeXform_->setNodeMask(show ? DISPLAY_MASK_TRACK_HISTORY : DISPLAY_MASK_NONE);
    altModeXform_->dirtyBound();
  }
}

void TrackHistoryNode::updateAltModePositionAndAppearance_(const osg::Matrixd& mat, const osg::Vec4& color)
{
  if (!altModeXform_)
    return;

  altModeXform_->setMatrix(mat);

  dropVertsDrawable_->setColor(color);

  osg::Matrixd world2local;
  world2local.invert(mat);

  // calculate the local point.
  static const osg::Vec3d s_zero;
  const osg::Vec3d world = s_zero * mat;

  osg::Vec3d up;
  osgEarth::GeoPoint geo;
  geo.fromWorld(locator_->getSRS()->getGeographicSRS(), world);
  geo.createWorldUpVector(up);
  up.normalize();

  dropVertsDrawable_->setVertex(1, (world - up*geo.alt()) * world2local);
}

void TrackHistoryNode::setHostBounds(const osg::Vec2& bounds)
{
  hostBounds_ = bounds;
  // the size of the ribbon depends on the size of the model, so force a rebuild
  if (lastPlatformPrefs_.trackprefs().trackdrawmode() == simData::TrackPrefs_Mode_RIBBON)
  {
    reset();
    update();
  }
}

void TrackHistoryNode::installShaderProgram(osg::StateSet* intoStateSet)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
  simVis::Shaders package;
  package.load(vp, package.trackHistoryVertex());
  intoStateSet->getOrCreateUniform(SIMVIS_TRACK_FLATMODE, osg::Uniform::BOOL)->set(false);
  intoStateSet->getOrCreateUniform(SIMVIS_TRACK_FLATRADIUS, osg::Uniform::FLOAT)->set(6371000.f);

  package.load(vp, package.trackHistoryFragment());
  intoStateSet->getOrCreateUniform(SIMVIS_TRACK_ENABLE, osg::Uniform::BOOL)->set(false);
  intoStateSet->getOrCreateUniform(SIMVIS_TRACK_OVERRIDE_COLOR, osg::Uniform::FLOAT_VEC4)->set(simVis::Color::White);
}

void TrackHistoryNode::updateFlatMode_(bool flatMode)
{
  if (!supportsShaders_)
    return;

  if (!flatModeUniform_.valid())
  {
    if (flatMode == false)
      return;    // Does not exist and not needed so return;

    osg::StateSet* stateset = this->getOrCreateStateSet();
    flatModeUniform_ = stateset->getOrCreateUniform(SIMVIS_TRACK_FLATMODE, osg::Uniform::BOOL);
  }

  flatModeUniform_->set(flatMode);
}

void TrackHistoryNode::setOverrideColor_(const osgEarth::Symbology::Color& color)
{
  if (supportsShaders_ && !overrideColorUniform_.valid())
  {
    if (color.a() == 0)
      return;  // Does not exist and not needed so return;

    osg::StateSet* stateset = this->getOrCreateStateSet();
    enableOverrideColorUniform_ = stateset->getOrCreateUniform(SIMVIS_TRACK_ENABLE, osg::Uniform::BOOL);
    enableOverrideColorUniform_->set(true);
    lastOverrideColor_ = color;
    overrideColorUniform_ = stateset->getOrCreateUniform(SIMVIS_TRACK_OVERRIDE_COLOR, osg::Uniform::FLOAT_VEC4);
    overrideColorUniform_->set(color);
    return;
  }

  if (lastOverrideColor_ != color)
  {
    lastOverrideColor_ = color;
    if (supportsShaders_)
    {
      overrideColorUniform_->set(color);
      enableOverrideColorUniform_->set(true);
    }
  }

}

void TrackHistoryNode::setPrefs(const simData::PlatformPrefs& platformPrefs, const simData::PlatformProperties& platformProps, bool force)
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

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, trackdrawmode))
  {
    resetRequested = true;
  }

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, trackcolor))
  {
    // store the trackcolor as the default track history color
    osg::Vec4f newColor = simVis::Color(prefs.trackcolor(), simVis::Color::RGBA);
    if (defaultColor_ != newColor)
    {
      defaultColor_ = newColor;
      //resetRequested = true;
    }
  }

  // use override color & override color (user settings)
  // also "use platform color"

  // track override color has priority

  simVis::Color origOverideColor = lastOverrideColor_;
  // if now using track override color and
  // just started or color changed
  if (prefs.usetrackoverridecolor())
  {
    const osgEarth::Symbology::Color color(simVis::Color(prefs.trackoverridecolor(), simVis::Color::RGBA));
    setOverrideColor_(color);
  }
  else if (prefs.useplatformcolor())
  {
    const osgEarth::Symbology::Color color(simVis::Color(platformPrefs.commonprefs().overridecolor(), simVis::Color::RGBA));
    setOverrideColor_(color);
  }
  else if (prefs.multitrackcolor())
  {
    // Set lastOverrideColor so re-enabling an override will trigger the logic at the end of setOverrideColor_
    lastOverrideColor_ = osg::Vec4f();
    // Can only disable the override after one has been created
    if (enableOverrideColorUniform_.valid())
      enableOverrideColorUniform_->set(false);
  }
  else
  {
    // If Multiple Color is off, and both overrides are off, SIMDIS displays a line matching the default color
    setOverrideColor_(defaultColor_);
  }

  if (!supportsShaders_ && origOverideColor != lastOverrideColor_)
    resetRequested = true;

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, linewidth))
  {
    const double lineWidth = osg::clampAbove(prefs.linewidth(), 1.0);
    osg::StateSet* stateSet = this->getOrCreateStateSet();
    osgEarth::LineDrawable::setLineWidth(stateSet, lineWidth);
    PointSize::setValues(stateSet, lineWidth, osg::StateAttribute::ON);
  }

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, tracklength))
  {
    // clear the track history and recreate
    resetRequested = true;
  }

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, flatmode))
  {
    updateFlatMode_(prefs.flatmode());
  }

  if (force || PB_FIELD_CHANGED(&lastPrefs, &prefs, altmode))
  {
    const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
    if (updateSlice)
      updateAltMode_(prefs.altmode(), *updateSlice);
  }

  if (force || PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, useclampalt) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmin) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, clampvalaltmax) ||
      PB_FIELD_CHANGED(&lastPlatformPrefs_, &platformPrefs, surfaceclamping))
  {
    // Did not test for the clamped angles since they are intended for stationary platforms
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

void TrackHistoryNode::update()
{
  // tracklength 0 means no track history is shown
  if (lastPlatformPrefs_.trackprefs().tracklength() == 0)
    return;

  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
  if (updateSlice == NULL)
  {
    // a valid/active platform must have an updateSlice - if assert fails, ensure that track history is not being updated for a non valid platform
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
    // track history should never be created for a static platform. - see PlatformNode::createTrackHistoryNode_
    assert(0);
    return;
  }

  // update track history to match current time window
  updateTrackData_(ds_.updateTime(), updateSlice->firstTime());

  // when the current point is interpolated, line, ribbon and bridge draw modes require special processing
  updateCurrentPoint_(*updateSlice);

  // update drop line from platform current position
  if (lastPlatformPrefs_.trackprefs().altmode())
      updateAltMode_(true, *updateSlice);

}

void TrackHistoryNode::updateTrackData_(double currentTime, double firstTime)
{
  // determine the time window that track history should display
  double endTime = currentTime;
  double beginTime = firstTime;
  int trackLength = lastPlatformPrefs_.trackprefs().tracklength();
  if (trackLength > 0 && (endTime - trackLength) > beginTime)
    beginTime = endTime - trackLength;

  // if there is an existing track history, determine if we can add only new points; this should be the case for normal time movement
  if (hasLastDrawTime_)
  {
    if (timeDirection_ == simCore::FORWARD)
    {
      // backward jump (e.g. time slider move) while in forward mode in time requires reset()
      if (currentTime < lastCurrentTime_)
        reset();
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
        reset();
      else
      {
        // remove all points with drawtime "older" than reverse mode end drawtime; i.e., remove all points with time newer than current time
        removePointsOlderThan_(toDrawTime_(endTime));
        // if new window overlaps previous window, then reuse existing points, only add the new points
        if (lastDrawTime_ * timeDirectionSign_ <= endTime)
        {
          // adjust endTime so that we add only new points, and avoid re-adding points that are still in the history
          endTime = -FLT_EPSILON + lastDrawTime_ * timeDirectionSign_;
        }
      }
    }
  }
  // store currentTime to enable time jump detection
  lastCurrentTime_ = currentTime;

  // update track history with points in the requested window
  backfillTrackHistory_(endTime, beginTime);
}

// given the desired time window, access the datastore to obtain points in that window, adding them to the track history
void TrackHistoryNode::backfillTrackHistory_(double endTime, double beginTime)
{
  const simData::PlatformUpdateSlice* updateSlice = static_cast<const simData::PlatformUpdateSlice*>(updateSliceBase_);
  if (updateSlice == NULL)
  {
    // a valid/active platform must have an updateSlice - if assert fails, ensure that track history is not being updated for a non valid platform
    assert(0);
    return;
  }

  if (timeDirection_ == simCore::FORWARD)
  {
    // get an iterator that will take us from beginTime up to and including endTime: [beginTime, endTime]
    simData::PlatformUpdateSlice::Iterator iter = updateSlice->lower_bound(beginTime);
    while (iter.hasNext() && iter.peekNext()->time() <= endTime)
    {
      simData::PlatformUpdateSlice::Iterator prevIter = iter;
      const simData::PlatformUpdate* prevUpdate = prevIter.previous();
      const simData::PlatformUpdate* u = iter.next();
      // if assert fails, hasNext() and next() are not in agreement, check iterator implementation
      assert(u);
      addUpdate_(*u, prevUpdate);
    }
  }
  else
  {
    // get an iterator that will take us from [endTime, beginTime]
    simData::PlatformUpdateSlice::Iterator iter = updateSlice->upper_bound(endTime);
    while (iter.hasPrevious() && iter.peekPrevious()->time() >= beginTime)
    {
      // since this is going backwards in time, prevIter is actually next, grab it before iterator moves backwards
      simData::PlatformUpdateSlice::Iterator prevIter = iter;
      const simData::PlatformUpdate* prevUpdate = prevIter.next();
      const simData::PlatformUpdate* u = iter.previous();
      // if assert fails, hasPrevious() and previous() are not in agreement, check iterator implementation
      assert(u);
      addUpdate_(*u, prevUpdate);
    }
  }
}

// update the track's representation of the current point, if that point is interpolated
void TrackHistoryNode::updateCurrentPoint_(const simData::PlatformUpdateSlice& updateSlice)
{
  // remove previous, will recreate if needed
  if (currentPointChunk_ != NULL)
    currentPointChunk_->reset();

  // only line, ribbon, and bridge draw modes require this processing,
  // but if there is not a previous point, there is nothing to do
  if (!updateSlice.isInterpolated() ||
      lastPlatformPrefs_.trackprefs().trackdrawmode() == simData::TrackPrefs_Mode_POINT ||
      chunkGroup_->getNumChildren() == 0)
    return;

  // create the special chunk for rendering the interpolated point, has two points to connect to rest of history
  // for ribbon and bridge modes, SIMDIS 9 draws a center line from platform to first data point, use simData::TrackPrefs_Mode_LINE to duplicate that behavior
  if (currentPointChunk_ == NULL)
  {
    currentPointChunk_ = new TrackChunkNode(2, locator_->getSRS(), simData::TrackPrefs_Mode_LINE);
    if (currentPointChunk_ == NULL)
      return;
    addChild(currentPointChunk_);
  }
  osg::Matrix hostMatrix;

  // find the most current update, either whatever is current, or the last available update
  const simData::PlatformUpdate* current = updateSlice.current();
  if (current == NULL)
  {
    simData::PlatformUpdateSlice::Iterator iter = updateSlice.lower_bound(updateSlice.lastTime());
    current = iter.next();
  }
  // if this fails, this platform node got created with no platform data
  assert(current);

  // points must be added in order of increasing drawTime
  if (timeDirection_ == simCore::REVERSE && getMatrix_(*current, hostMatrix))
  {
    const double drawTime = toDrawTime_(current->time());
    currentPointChunk_->addPoint(hostMatrix, drawTime, historyColorAtTime_(drawTime), hostBounds_);
  }

  // duplicate the most recent (non-current) datapoint so that this chunk connects to the previous chunk
  assert(chunkGroup_->getNumChildren() > 0);
  assert(updateSlice.numItems() > 0);
  simData::PlatformUpdateSlice::Iterator iter = updateSlice.lower_bound(current->time());
  const simData::PlatformUpdate* u = iter.previous();
  if (u && getMatrix_(*u, hostMatrix))
  {
    // this point should never be the current point; if assert fails, check to make sure we only process interpolated points
    assert(u->time() < current->time());
    currentPointChunk_->addPoint(hostMatrix, u->time(), historyColorAtTime_(u->time()), hostBounds_);
  }

  if (timeDirection_ == simCore::FORWARD && getMatrix_(*current, hostMatrix))
  {
    currentPointChunk_->addPoint(hostMatrix, current->time(), historyColorAtTime_(current->time()), hostBounds_);
  }
}

bool TrackHistoryNode::getMatrix_(const simData::PlatformUpdate& u, osg::Matrix& hostMatrix)
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

}
