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
#include <QApplication>
#include <QScreen>
#include <QWidget>
#include "simQt/QtUtils.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#include <QDesktopWidget>
#endif

namespace simQt {

QRect QtUtils::getAvailableScreenGeometry(const QWidget& widget, const QWidget* parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  return (parent ? QApplication::desktop()->availableGeometry(parent) : QApplication::desktop()->availableGeometry(&widget));
#else
  return (parent ? parent->screen()->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry());
#endif
}

void QtUtils::centerWidgetOnParent(QWidget& widget, const QWidget* parent)
{
  const auto& screenGeometry = getAvailableScreenGeometry(widget, parent);

  QPoint newPos;
  bool posFound = false;
  if (parent && parent->isVisible())
  {
    // make sure the parent center is visible in the current screen geometry
    const auto& centerPos = parent->mapToGlobal(parent->rect().center());
    if (screenGeometry.contains(centerPos))
    {
      newPos = centerPos - widget.rect().center();
      posFound = true;
    }
  }
  // could not center on parent, just center on the parent's screen
  if (!posFound)
    newPos = screenGeometry.center() - widget.rect().center();

  // make sure the upper left corner is onscreen
  if (newPos.y() < screenGeometry.top())
    newPos.setY(screenGeometry.top());
  if (newPos.x() < screenGeometry.left())
    newPos.setX(screenGeometry.left());

  // add padding for bottom check to ensure titlebar is visible
  if (newPos.y() > screenGeometry.bottom() - 30)
    newPos.setY(screenGeometry.bottom() - 30);

  widget.move(newPos);
}

}
