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
#include <QSlider>
#include "simCore/Time/TimeClass.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/TimeWidget.h"
#include "simQt/SliderTimeWidgetBinding.h"

namespace simQt
{
static const int MAX_RANGE = 10000;

SliderTimeWidgetBinding::SliderTimeWidgetBinding(QSlider* slider, TimeWidget* timeWidget, QWidget *parent)
  : QObject(parent),
    slider_(slider),
    timeWidget_(timeWidget)
{
  // Must pass in slider and time widget
  assert(slider_ != nullptr);
  assert(timeWidget_ != nullptr);

  slider_->setRange(0, MAX_RANGE);
  connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(setTimeWidgetValue_(int)));
  connect(timeWidget_, SIGNAL(timeChanged(simCore::TimeStamp)), this, SLOT(setSliderValue_(simCore::TimeStamp)));
  connect(timeWidget_, SIGNAL(timeRangeChanged()), this, SLOT(rescaleSlider_()));

  // Initialize the slider to match the time widget
  rescaleSlider_();
}

void SliderTimeWidgetBinding::setTimeWidgetValue_(int newValue)
{
  int referenceYear = timeWidget_->timeRangeStart().referenceYear();
  double minTime = timeWidget_->timeRangeStart().secondsSinceRefYear();
  double maxTime = timeWidget_->timeRangeEnd().secondsSinceRefYear(referenceYear);

  if (minTime >= maxTime)
    return;

  int max = slider_->maximum();

  if (max == 0)
    return;

  double seconds = newValue * (maxTime - minTime) / max + minTime;
  const simCore::TimeStamp newTime(referenceYear, seconds);
  if (newTime != timeWidget_->timeStamp())
    timeWidget_->setTimeStamp(newTime);
}

void SliderTimeWidgetBinding::setSliderValue_(const simCore::TimeStamp& time)
{
  int referenceYear = timeWidget_->timeRangeStart().referenceYear();
  double minTime = timeWidget_->timeRangeStart().secondsSinceRefYear();
  double maxTime = timeWidget_->timeRangeEnd().secondsSinceRefYear(referenceYear);

  if (minTime >= maxTime)
    return;

  double currentTime = time.secondsSinceRefYear(referenceYear);

  if (currentTime < minTime)
    currentTime = minTime;
  else if (currentTime > maxTime)
    currentTime = maxTime;

  int max = slider_->maximum();

  if (max == 0)
    return;

  int index = static_cast<int>(max * (currentTime - minTime) / (maxTime - minTime));

  // Block slider signals when setting slider value to prevent constantly changing time bounds (e.g. live mode) from causing drift
  simQt::ScopedSignalBlocker blocker(*slider_);
  slider_->setValue(index);
}

void SliderTimeWidgetBinding::rescaleSlider_()
{
  setSliderValue_(timeWidget_->timeStamp());
}

}

