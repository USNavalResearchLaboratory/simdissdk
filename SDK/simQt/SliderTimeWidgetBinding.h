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

#ifndef SIMQT_SLIDER_TIME_WIDGET_BINDING_H
#define SIMQT_SLIDER_TIME_WIDGET_BINDING_H

#include <QObject>
#include "simCore/Common/Export.h"

class QSlider;

namespace simCore { class TimeStamp; }

namespace simQt
{

class TimeWidget;

/**
  * Binds a QSlider with a simQt::TimeWidget so that changes in one affects the other.  This
  * class modifies the range of the slider.
  */
class SDKQT_EXPORT SliderTimeWidgetBinding : public QObject
{
  Q_OBJECT

public:
  /** Constructor */
  SliderTimeWidgetBinding(QSlider* slider, TimeWidget* timeWidget, QWidget *parent = NULL);

private slots:
  void setTimeWidgetValue_(int value);
  void setSliderValue_(const simCore::TimeStamp& time);
  void rescaleSlider_();

private:
  QSlider* slider_;
  TimeWidget* timeWidget_;

};

}

#endif
