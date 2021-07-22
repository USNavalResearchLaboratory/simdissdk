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
#ifndef SIMQT_TOAST_H
#define SIMQT_TOAST_H

#include <QLabel>
#include "simCore/Common/Export.h"

class QPropertyAnimation;
class QTimer;

namespace simQt
{

class ClickableLabel;

/**
 * Defines an interface for showing Toast messages on-screen.  The actual implementation of the
 * toast messages is up to an extension.  Toast messages are intended for brief popup text messages
 * for the end user, that disappear automatically after either a brief or long time.  The concept
 * is modeled after both the Windows and Android toast features.
 *
 * An extension might implement this as a small popup that times out, with new string values
 * replacing old values.  Or new string values might be appended on new lines or as new popups.
 * The implementation is up to an extension.  Callers just need to post a message using the
 * showText() slot and forget about it.
 */
class SDKQT_EXPORT Toast
{
public:
  /** Virtual destructor */
  virtual ~Toast() {}

  /** Duration specifies how long to show the text */
  enum Duration
  {
    DURATION_SHORT,
    DURATION_LONG
  };

  /** Show toast text for the given duration. */
  virtual void showText(const QString& text, Duration duration=DURATION_SHORT) = 0;
};

/** Null object implementation for Toast */
class NullToast : public Toast
{
public:
  /// Show toast text for the given duration
  virtual void showText(const QString& text, Duration duration=DURATION_SHORT) {}
};

///////////////////////////////////////////////////////////////

/** Implementation of the Toast interface from the Extensions API that shows a small window. */
class SDKQT_EXPORT ToastOnWidget : public QObject, public Toast
{
  Q_OBJECT;

public:
  explicit ToastOnWidget(QWidget* widget);
  virtual ~ToastOnWidget();

  /** Shows the text in a small popup */
  virtual void showText(const QString& text, Duration duration = DURATION_SHORT);

private slots:
  /** "Soft" hide of the toast window; If the mouse is over the label, then we will hide when mouse leaves it */
  void softCloseToast_();

private:
  QWidget* widget_;
  ClickableLabel* toast_;
  QTimer* hideTimer_;
  QPropertyAnimation* popIn_;
  QPropertyAnimation* popOut_;
};

///////////////////////////////////////////////////////////////

/** Label that emits signals when the mouse enters and leaves, and when buttons pressed */
class SDKQT_EXPORT ClickableLabel : public QLabel
{
  Q_OBJECT;

public:
  explicit ClickableLabel(QWidget* parent);
  bool isMouseInside() const;

protected:
  virtual void mousePressEvent(QMouseEvent* evt);
  virtual void mouseReleaseEvent(QMouseEvent* evt);
  virtual void enterEvent(QEvent* evt);
  virtual void leaveEvent(QEvent* evt);

signals:
  /** Mouse pressed down */
  void pressed();
  /** Previous press leads to a release, creates a click. */
  void clicked();
  /** Mouse enters the label */
  void mouseEntered();
  /** Mouse has left the label */
  void mouseLeft();

private:
  bool mouseInside_;
};

} // simQt

#endif /* SIMQT_TOAST_H */
