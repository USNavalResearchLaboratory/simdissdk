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
#ifndef SIMQT_SPLASHSCREEN_H
#define SIMQT_SPLASHSCREEN_H

#include "simCore/Common/Export.h"
#include <QSplashScreen>

class QScreen;

namespace simQt {

/**
 * Reusable splash screen based on QSplashScreen class.  Addresses annoyances in the original
 * implementation, including:
 *  - Text messages can now be reliably shown with a simple call to showMessage()
 *  - Color and position is set and remembered for calls to showMessage()
 *  - Splash shows on the Windows taskbar to give users a hint that the application is starting
 *  - Click-to-close behavior is disabled through a setVisible() override
 *  - Font size consistently applied to 12 pixels; useful for making a background banner for text
 */
class SDKQT_EXPORT SplashScreen : public QSplashScreen
{
  Q_OBJECT;
public:
  explicit SplashScreen(const QPixmap& pixmap=QPixmap());
  explicit SplashScreen(QWidget* parent, const QPixmap& pixmap=QPixmap());
  virtual ~SplashScreen();

  /** Moves to another screen */
  void moveToScreen(const QScreen& screen);

  /** Color of the text to show in the splash screen */
  QColor textColor() const;
  /** Alignment of the text */
  int textAlignment() const;

  /** Don't permit splash screen to hide until the destructor. (Avoids click-to-close behavior) */
  virtual void setVisible(bool showIt);

public Q_SLOTS:
  /** Sets the text foreground color; only affects future showMessage() calls */
  void setTextColor(const QColor& color);
  /** Set the text alignment relative to the window (e.g. Qt::AlignHCenter|Qt::AlignBottom); only affects future showMessage() calls */
  void setTextAlignment(int qtTextAlign);
  /** Change the message being shown */
  void showMessage(const QString& message);

private:
  /** Adds the taskbar icon back, on Windows (which doesn't show a taskbar entry by default) */
  void addToWindowsTaskbar_();
  /** Recenters the splash screen on the system's primary screen, which is an issue in Qt 5.15+ */
  void recenterOnPrimaryScreen_();

  QColor color_;
  int textAlign_;
  bool destructing_;
};

}

#endif /* SIMQT_SPLASHSCREEN_H */
