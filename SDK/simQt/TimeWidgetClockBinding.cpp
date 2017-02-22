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
#include "simCore/Time/Clock.h"
#include "simCore/Time/TimeClass.h"
#include "simData/DataStore.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/TimeWidget.h"
#include "simQt/TimeWidgetClockBinding.h"

namespace simQt
{

/// forward callbacks to binding (and thus time widget) when clock time changes
class TimeWidgetClockBinding::TimeObserver : public simCore::Clock::TimeObserver
{
public:
  /// constructor
  explicit TimeObserver(TimeWidgetClockBinding* binding)
    : binding_(binding)
  {
  }
  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    binding_->updateWidgetTime_(t);
  }
  virtual void onTimeLoop()
  { // no-op
  }
  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  { // no-op
  }

private:
  TimeWidgetClockBinding* binding_;
};

/// forward callbacks to binder (and thus time widget) when clock mode changes
class TimeWidgetClockBinding::ModeObserver : public simCore::Clock::ModeChangeObserver
{
public:
  /// constructor
  explicit ModeObserver(TimeWidgetClockBinding* binding)
    : binding_(binding)
  {
  }
  /// clock mode has changed
  virtual void onModeChange(simCore::Clock::Mode newMode) {}
  /// direction has changed
  virtual void onDirectionChange(simCore::TimeDirection newDirection) {}
  /// time scale has changed
  virtual void onScaleChange(double newValue) {}
  /// start/end times have changed
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end)
  {
    binding_->updateWidgetBounds_(true);
  }
  /// canLoop has changed
  virtual void onCanLoopChange(bool newVal) {}
  /// User permission to edit changed
  virtual void onUserEditableChanged(bool isEditable)
  {
    binding_->updateDisabledState_();
  }
private:
  TimeWidgetClockBinding* binding_;
};

/////////////////////////////////////////////////////////////////////////

ReferenceYearCache::ReferenceYearCache(simData::DataStore* dataStore)
  : QObject(),
    refYear_(1970)
{
  if (dataStore != NULL)
    refYear_ = dataStore->referenceYear();
}

void ReferenceYearCache::onScenarioPropertiesChange(simData::DataStore* source)
{
  if (source != NULL)
  {
    int newYear = source->referenceYear();
    if (refYear_ != newYear)
    {
      refYear_ = newYear;
      emit referenceYearChanged(refYear_);
    }
  }
}

int ReferenceYearCache::referenceYear() const
{
  return refYear_;
}

/////////////////////////////////////////////////////////////////////////

TimeWidgetClockBinding::TimeWidgetClockBinding(simQt::TimeWidget* parent)
 : QObject(parent),
   timeWidget_(parent),
   clock_(NULL),
   dataStore_(NULL)
{
  assert(parent != NULL); // must pass in simQt::TimeWidget in constructor
  timeObserver_.reset(new TimeObserver(this));
  modeObserver_.reset(new ModeObserver(this));
  ReferenceYearCache* cache = new ReferenceYearCache(NULL);
  refYearCache_.reset(cache);
  connect(timeWidget_, SIGNAL(timeChanged(const simCore::TimeStamp&)),
    this, SLOT(setClockTime_(const simCore::TimeStamp&)));
  connect(cache, SIGNAL(referenceYearChanged(int)), this, SLOT(passRefYearToChildren_()));
  bindClock(NULL);
  bindDataStore(NULL);
}

TimeWidgetClockBinding::~TimeWidgetClockBinding()
{
  unbindClock();
  unbindDataStore();
}

void TimeWidgetClockBinding::bindClock(simCore::Clock* clock, bool bindCurrentTime)
{
  unbindClock();
  clock_ = clock;
  bindCurrentTime_ = bindCurrentTime;
  if (clock_)
  {
    clock_->registerTimeCallback(timeObserver_);
    clock_->registerModeChangeCallback(modeObserver_);
  }
  // Set the initial state
  updateDisabledState_();
  updateWidgetBounds_(false);
  if (clock_ != NULL)
    updateWidgetTime_(clock_->currentTime());
}

void TimeWidgetClockBinding::unbindClock()
{
  if (clock_)
  {
    clock_->removeTimeCallback(timeObserver_);
    clock_->removeModeChangeCallback(modeObserver_);
  }
  clock_ = NULL;
}

void TimeWidgetClockBinding::bindDataStore(simData::DataStore* dataStore)
{
  unbindDataStore();
  dataStore_ = dataStore;
  if (dataStore_)
  {
    dataStore_->addScenarioListener(refYearCache_);
    refYearCache_->onScenarioPropertiesChange(dataStore);
  }
  // Set up initial state
  updateWidgetBounds_(false);
}

void TimeWidgetClockBinding::unbindDataStore()
{
  if (dataStore_)
  {
    dataStore_->removeScenarioListener(refYearCache_);
  }
  dataStore_ = NULL;
}

void TimeWidgetClockBinding::setClockTime_(const simCore::TimeStamp& clockTime)
{
  if (bindCurrentTime_ && clock_ != NULL && clock_->currentTime() != clockTime && clock_->isUserEditable())
    clock_->setTime(clockTime);
}

void TimeWidgetClockBinding::updateDisabledState_()
{
  if (bindCurrentTime_)
    timeWidget_->setEnabled(clock_ != NULL && clock_->isUserEditable());
}

void TimeWidgetClockBinding::updateWidgetTime_(const simCore::TimeStamp &t)
{
  if (bindCurrentTime_)
    timeWidget_->setTimeStamp(t);
}

void TimeWidgetClockBinding::updateWidgetBounds_(bool notifyTimeChange)
{
  // Can't do anything without a clock, because clock gives the begin/end times
  if (!clock_)
    return;
  ReferenceYearCache* cache = dynamic_cast<ReferenceYearCache*>(refYearCache_.get());
  // Programming error or dynamic_cast<> RTTI failure can cause assert
  assert(cache != NULL);
  if (cache != NULL)
  {
    // Pull out the cache value, but fall back to a valid value if cache isn't right due to no DataStore
    int refYear = cache->referenceYear();
    if (refYear <= 0)
      refYear = clock_->startTime().referenceYear();
    if (!notifyTimeChange)
    {
      // Range is set before the clock time is set. On initialization, block signals so setting time range does not alter clock time
      simQt::ScopedSignalBlocker blockSignals(*timeWidget_);
      timeWidget_->setTimeRange(refYear, clock_->startTime(), clock_->endTime());
    }
    else
      timeWidget_->setTimeRange(refYear, clock_->startTime(), clock_->endTime());
  }
}

void TimeWidgetClockBinding::passRefYearToChildren_()
{
  if (!clock_)
    return;
  updateWidgetBounds_(true);
  timeWidget_->setTimeStamp(clock_->currentTime());
}

}
