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
#include <QKeyEvent>
#include "simQt/AutoRepeatFilter.h"

namespace simQt {

AutoRepeatFilter::AutoRepeatFilter(QObject* parent)
  : QObject(parent)
{
}

bool AutoRepeatFilter::eventFilter(QObject* obj, QEvent* evt)
{
  if (enabled_ && evt && evt->type() == QEvent::KeyPress)
  {
    const QKeyEvent* keyEvt = dynamic_cast<const QKeyEvent*>(evt);
    if (keyEvt && keyEvt->isAutoRepeat())
      return true;
  }
  return QObject::eventFilter(obj, evt);
}

void AutoRepeatFilter::setEnabled(bool enabled)
{
  enabled_ = enabled;
}

bool AutoRepeatFilter::isEnabled() const
{
  return enabled_;
}

}
