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
#include <QMainWindow>
#include <QScreen>
#ifdef WIN32
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QStyleHints>
#endif
#endif
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

bool QtUtils::isDarkTheme()
{
#ifdef WIN32
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
  return (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
  return false;
#endif
#endif

  // Testing has shown QStyleHints::colorScheme() is unreliable on linux
  const QPalette palette = QApplication::palette();
  const QColor windowColor = palette.color(QPalette::Window);
  const QColor textColor = palette.color(QPalette::WindowText);

  // Compare lightness on the window and text if this looks like a standard theme (low saturation colors)
  const int maxSaturation = 50;
  const bool standardTheme = (windowColor.hsvSaturation() < maxSaturation && textColor.hsvSaturation() < maxSaturation);

  if (standardTheme)
  {
    // Guess a dark theme if the window color is darker than the text color
    return windowColor.lightness() < textColor.lightness();
  }

  // In a non-standard theme, assume dark theme if the window color is relatively dark
  return windowColor.lightness() < 80;
}

QWidget* QtUtils::getMainWindowParent(QWidget* widget)
{
  if (!widget)
    return widget;

  for (QWidget* rv = dynamic_cast<QWidget*>(widget->parent()); rv != nullptr; rv = dynamic_cast<QWidget*>(rv->parent()))
  {
    if (dynamic_cast<QMainWindow*>(rv) != nullptr)
      return rv;
  }
  return widget;
}

}
