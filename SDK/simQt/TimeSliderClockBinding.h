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
#ifndef SIMQT_TIMESLIDERCLOCKBINDING_H
#define SIMQT_TIMESLIDERCLOCKBINDING_H

#include <QObject>
#include "simCore/Common/Common.h"
#include "simCore/Time/Clock.h"

class QSlider;

namespace simQt
{

/**
 * Responsible for binding a QSlider to a simCore::Clock object.  A slider that is bound
 * to a clock will automatically update position and enable state based on callback from
 * the clock.  It will also set the clock time based on user interaction with the slider.
  */
class SDKQT_EXPORT TimeSliderClockBinding : public QObject
{
  Q_OBJECT;
public:

  /** Enumeration of actions to take when controls are disabled */
  enum DisabledMode
  {
    /** Hide the slider when clock controls disabled */
    HIDE,
    /** Disable the slider when clock controls disabled */
    DISABLE,
    /** Hides AND disables the slider when clock controls disabled */
    HIDE_AND_DISABLE
  };

  /**
   * Instantiate a time slider binding for the given slider.  Optionally
   * supply a clock instance to bind the slider.  Note that the memory
   * is managed by the parent/child relationship of this class to QSlider
   */
  TimeSliderClockBinding(QSlider* parent, simCore::Clock* clock=nullptr, DisabledMode disabledMode=HIDE_AND_DISABLE);
  /** Automatically unbinds the clock as needed */
  virtual ~TimeSliderClockBinding();

  /** Binds the slider to the value of the clock */
  void bindClock(simCore::Clock* clock);
  /** Removes bindings to a previously bound clock */
  void unbindClock();

  /** Returns whether the slider gets disabled or hidden when the bound clock has controls disabled. */
  DisabledMode disabledMode() const;
  /** Returns the state of allowing visibility; see setVisible(); widget might still be hidden by disabledMode */
  bool allowVisible() const;

public Q_SLOTS:
  /**
   * Indicates whether the slider can be shown; if true, it's still only shown if not disabled (depending on disabledMode()).
   * The following truth table is followed for whether clock is visible:
   *
   *  Allow-Visible  Clock-Editable    Disabled-Mode     Widget-Shown
   *  false          *                 *                 false
   *  true           true              *                 true
   *  true           *                 DISABLE           true
   *  true           true              *                 true
   *  true           false             HIDE              false
   *  true           false             HIDE_AND_DISABLE  false
   */
  void setAllowVisible(bool allowVisible);

private Q_SLOTS:
  void valueChanged_(int sliderPos);

private:
  void fixEnabledState_(bool enableIt);
  void updateSliderTime_();
  void updateSliderTime_(const simCore::TimeStamp &t);

  QSlider* slider_;
  simCore::Clock* clock_;
  class TimeObserver;
  class ModeObserver;
  simCore::Clock::TimeObserverPtr timeObserver_;
  simCore::Clock::ModeChangeObserverPtr modeObserver_;
  DisabledMode disabledMode_;
  bool allowVisible_;
};

}

#endif /* SIMQT_TIMESLIDERCLOCKBINDING_H */
