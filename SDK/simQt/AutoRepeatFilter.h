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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_AUTOREPEATFILTER_H
#define SIMQT_AUTOREPEATFILTER_H

#include <QObject>
#include "simCore/Common/Common.h"

namespace simQt {

/**
 * Filter class that blocks auto-repeat keypress events from reaching the filtered object.  This is
 * useful for blocking auto-repeat keys from the GLWidget / ViewWidget.  osgEarth::EarthManipulator
 * can have poor keyboard interaction if the frame rate ever drops under the key autorepeat rate,
 * and this filter helps fix that problem.
 *
 * The following code can be used to install the filter on a widget:
 * <code>
 * AutoRepeatFilter* filter = new AutoRepeatFilter(viewWidget);
 * viewWidget->installEventFilter(filter);
 * </code>
 *
 * Note that this filter is auto-installed on simQt::ViewWidget instances, but is not automatically
 * installed on osgQt::GLWidget.  So if your application uses osgQt::GLWidget, consider using the filter.
 */
class SDKQT_EXPORT AutoRepeatFilter : public QObject
{
  Q_OBJECT;

public:
  explicit AutoRepeatFilter(QObject* parent = nullptr);

  /** Enables or disables the filtering.  If true (default), auto-repeated keys are filtered out. */
  void setEnabled(bool enabled);
  /** True if enabled (i.e., auto-repeated keys are filtered out) */
  bool isEnabled() const;

protected:
  // From QObject:
  virtual bool eventFilter(QObject* obj, QEvent* evt) override;

private:
  bool enabled_ = true;
};

}

#endif /* SIMQT_AUTOREPEATFILTER_H */
