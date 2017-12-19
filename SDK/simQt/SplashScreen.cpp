// -*- mode: c++ -*-
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
*               EW Modeling & Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*/
#ifdef WIN32
#include <windows.h>
#endif

#include <QCoreApplication>
#include <QCloseEvent>
#include "simQt/SplashScreen.h"

namespace simQt {

/// Text will default to dark blue
static const QColor NAVY_BLUE = QColor(0, 0, 128, 255);
/// Text shows up centered along the bottom
static const int BOTTOM_CENTER = Qt::AlignLeft|Qt::AlignBottom;
/// Use a Qt Style Sheet to adjust the font size
static const QString FONT_SIZE_12 = "QSplashScreen { font-weight: bold; font-size: 12px; }\n";

/// Constructor without a parent
SplashScreen::SplashScreen(const QPixmap& pixmap)
  : QSplashScreen(pixmap),
    color_(NAVY_BLUE),
    textAlign_(BOTTOM_CENTER),
    destructing_(false)
{
  setStyleSheet(FONT_SIZE_12);
  addToWindowsTaskbar_();
}

/// Constructor with a parent
SplashScreen::SplashScreen(QWidget* parent, const QPixmap& pixmap)
  : QSplashScreen(parent, pixmap),
    color_(NAVY_BLUE),
    textAlign_(BOTTOM_CENTER),
    destructing_(false)
{
  setStyleSheet(FONT_SIZE_12);
  addToWindowsTaskbar_();
}

SplashScreen::~SplashScreen()
{
  destructing_ = true;
  // Make sure we really hide the window before shutting down
  QSplashScreen::setVisible(false);
  close();
}

void SplashScreen::addToWindowsTaskbar_()
{
  // Note that as of Qt 4.8.2, setWindowFlags() MAY work to do this, but fails
  // due to parent() being NULL; a call in setWindowFlags() accesses parent()
  // directly without checking NULL, causing a crash.  So we must use Windows API

#ifdef WIN32
  // Pull out the HWND for the splash window
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
  HWND windowId = winId();
#else
  HWND windowId = reinterpret_cast<HWND>(winId());
#endif

  // Turn on the taskbar icon for Windows
  int exStyle = GetWindowLong(windowId, GWL_EXSTYLE);
  SetWindowLong(windowId, GWL_EXSTYLE, exStyle & ~WS_EX_TOOLWINDOW);
#endif
}

void SplashScreen::setVisible(bool showIt)
{
  // Don't let the mouse clicks hide the window, overriding default QSplashScreen behavior
  if (showIt)
    QSplashScreen::setVisible(showIt);
}

QColor SplashScreen::textColor() const
{
  return color_;
}

int SplashScreen::textAlignment() const
{
  return textAlign_;
}

void SplashScreen::setTextColor(const QColor& color)
{
  color_ = color;
}

void SplashScreen::setTextAlignment(int qtTextAlign)
{
  textAlign_ = qtTextAlign;
}

void SplashScreen::showMessage(const QString& message)
{
  QSplashScreen::showMessage(message, textAlign_, color_);
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

}
