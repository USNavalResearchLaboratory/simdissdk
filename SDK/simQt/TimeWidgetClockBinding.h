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
#ifndef SIMQT_TIMEWIDGETCLOCKBINDING_H
#define SIMQT_TIMEWIDGETCLOCKBINDING_H

#include <QObject>
#include "simCore/Common/Common.h"
#include "simCore/Time/Clock.h"
#include "simData/DataStore.h"

namespace simCore { class TimeStamp; }

namespace simQt
{

class TimeWidget;

/**
 * Responsible for binding a simQt::TimeWidget to a simCore::Clock object.  A time widget that is bound
 * to a clock will automatically update bounds and enable state based on callback from the clock, and
 * optionally also bind to the current time.  If the option to bind to current time is set, then changing
 * the time widget's value will change the current time.
  */
class SDKQT_EXPORT TimeWidgetClockBinding : public QObject
{
  Q_OBJECT;
public:

  /**
   * Instantiate a time widget binding for the given time widget.  Optionally
   * supply a clock instance to bind the slider.  Note that the memory
   * is managed by the parent/child relationship of this class to simQt::TimeWidget
   */
  TimeWidgetClockBinding(simQt::TimeWidget* parent);
  /** Automatically unbinds the clock as needed */
  virtual ~TimeWidgetClockBinding();

  /**
   * Set whether end time is respected in live mode.  If end time is not respected, time widget will
   * have an infinite upper bound in live mode.
   * @param respectLiveModeEndtTime If false, the upper bound of the widget's time range while in live mode
   *   will be infinite.  If true, the upper bound will be kept equal to the clock's end time.
   */
  void setRespectLiveModeEndTime(bool respectLiveModeEndTime);

  /**
   * Binds the slider to the value of the clock.  Can optionally bind the current time, which
   * will cause the time widget to reflect the current time, and set the current time when changed.
   * Additionally, when bound to current time, this binding will update the enabled/disabled state
   * for the widget.
   * @param clock Clock to bind to; sets up the proper begin/end times, at a minimum
   * @param bindCurrentTime If false, then we only update begin/end times.  If true, then this class
   *   will keep the time widget's time in sync with the clock's time, and vice versa, while also
   *   managing the enable/disable state of the time widget.
   */
  void bindClock(simCore::Clock* clock, bool bindCurrentTime = false);
  /** Removes bindings to a previously bound clock */
  void unbindClock();

  /** Binds to a data store, required for the reference year */
  void bindDataStore(simData::DataStore* dataStore);
  /** Removes bindings from a previously bound data store */
  void unbindDataStore();

private slots:
  void setClockTime_(const simCore::TimeStamp& clockTime);

  /** Called when the reference year changes, which requires a bounds update and a set-time on all children */
  void passRefYearToChildren_();

private:
  void updateDisabledState_();
  void updateWidgetTime_(const simCore::TimeStamp &t);

  /**
   * Updates the min/max bounds of the spinner. Usually called on initialization of scenario
   * but could be called whenever the time bounds changes. If current time is not inside the
   * bounds, the time is also adjusted.
   * @param notifyTimeChange boolean true if it is desired to emit a signal to set the clock time if it was out of bounds
   */
  void updateWidgetBounds_(bool notifyTimeChange);

  simQt::TimeWidget* timeWidget_;
  simCore::Clock* clock_;
  bool bindCurrentTime_;
  class TimeObserver;
  class ModeObserver;
  simCore::Clock::TimeObserverPtr timeObserver_;
  simCore::Clock::ModeChangeObserverPtr modeObserver_;
  simData::DataStore* dataStore_;
  simData::DataStore::ScenarioListenerPtr refYearCache_;
  bool respectLiveModeEndTime_;
};

/** Caches the current reference year and emits signals when it changes */
class ReferenceYearCache : public QObject, public simData::DataStore::ScenarioListener
{
  Q_OBJECT;
public:
  /** Initializes the cache from a given data store (could be nullptr) */
  ReferenceYearCache(simData::DataStore *dataStore=nullptr);

  /** Checks on reference year change and emits signal if needed */
  virtual void onScenarioPropertiesChange(simData::DataStore* source);
  /** Returns the current reference year */
  int referenceYear() const;

signals:
  /** Emitted only when reference year changes */
  void referenceYearChanged(int newYear);

private:
  int refYear_;
};

}

#endif /* SIMQT_TIMEWIDGETCLOCKBINDING_H */
