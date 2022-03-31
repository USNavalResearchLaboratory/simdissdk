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
#include <cassert>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>
#include "simQt/Toast.h"

namespace simQt
{

  /** QSS Style Sheet for the label display */
static const QString LABEL_STYLE =
"border: 0.1em solid rgb(128, 128, 128);\n"
"color: rgb(255, 255, 255);\n"
"font-size: 12pt;\n"
"padding: 0.25em 0.4em;\n"  // padding: vertical horizontal
"background-color: rgb(0, 0, 64);";
/** CSS for colored links.  Should be compatible with background color above */
static const QString LABEL_LINK_CSS = "<head><style>a { color: #8080ff; }\n</style></head>";

/** Duration of the animation to pop in and out, in milliseconds */
static const int ANIMATION_DURATION = 300; // milliseconds
/** Duration of the long popup, in milliseconds */
static const int INTERVAL_LONG = 4000;
/** Duration of the short popup, in milliseconds */
static const int INTERVAL_SHORT = 2000;

ToastOnWidget::ToastOnWidget(QWidget* widget)
  : widget_(widget),
    hideTimer_(nullptr),
    popIn_(nullptr),
    popOut_(nullptr)
{
  // Set up the toast button
  if (widget_)
    toast_ = new ClickableLabel(widget_);
  else
  {
    // This shouldn't happen; indicates that we were given a nullptr widget.
    assert(0);
    return;
  }

  // Background is overridden by style sheet
  toast_->hide();
  toast_->setAutoFillBackground(false);
  toast_->setStyleSheet(LABEL_STYLE);
  toast_->setWindowFlags(Qt::WindowStaysOnTopHint);
  toast_->setOpenExternalLinks(true);
  toast_->setTextFormat(Qt::AutoText);

  // Set up the timer that hides the toast
  hideTimer_ = new QTimer;
  hideTimer_->setSingleShot(true);

  // Create the pop-in animation
  popIn_ = new QPropertyAnimation(toast_, "geometry");
  popIn_->setDuration(ANIMATION_DURATION);
  popIn_->setEasingCurve(QEasingCurve::InOutQuad);

  // Animate the button out
  popOut_ = new QPropertyAnimation(toast_, "geometry");
  popOut_->setDuration(ANIMATION_DURATION);
  popOut_->setEasingCurve(QEasingCurve::InOutQuad);

  // Create the timer chain to pop in, wait, then pop out
  connect(popIn_, SIGNAL(finished()), hideTimer_, SLOT(start()));
  connect(hideTimer_, SIGNAL(timeout()), this, SLOT(softCloseToast_()));

  // Toast should hide at the end of the animation, or when user clicks on it
  connect(popOut_, SIGNAL(finished()), toast_, SLOT(hide()));
  // note that clicked() is preferred over pressed(), because otherwise links won't work
  connect(toast_, SIGNAL(clicked()), hideTimer_, SLOT(stop()));
  connect(toast_, SIGNAL(clicked()), popIn_, SLOT(stop()));
  connect(toast_, SIGNAL(clicked()), popOut_, SLOT(stop()));
  connect(toast_, SIGNAL(clicked()), toast_, SLOT(hide()));
}

ToastOnWidget::~ToastOnWidget()
{
  delete popIn_;
  delete popOut_;
  delete hideTimer_;
  // Do not delete toast_, its life span handled by Qt parent
}

void ToastOnWidget::showText(const QString& text, Duration duration)
{
  // Assertion failure means we don't have a widget
  assert(widget_);
  if (!widget_)
    return;

  // Stop all the animations and reset everything
  popIn_->stop();
  hideTimer_->stop();
  popOut_->stop();
  toast_->hide();

  // Apply the user's text string
  if (text.contains("</a>"))
  {
    // ... But prefix it with a link color if there are hyperlinks
    toast_->setText(LABEL_LINK_CSS + text);
  }
  else
    toast_->setText(text);

  // Configure the toast features with the new requested options
  hideTimer_->setInterval(duration == Toast::DURATION_SHORT ? INTERVAL_SHORT : INTERVAL_LONG);
  toast_->adjustSize();
  // Put it at the bottom out of sight currently
  toast_->move((widget_->width() - toast_->width()) / 2, widget_->height());

  // Calculate the starting and ending geometries
  QRect loweredGeometry(toast_->geometry());
  QRect raisedGeometry(loweredGeometry);
  raisedGeometry.moveTop(raisedGeometry.top() - static_cast<int>(toast_->height() * 1.3));

  // Set the animation ending positions
  popIn_->setStartValue(loweredGeometry);
  popIn_->setEndValue(raisedGeometry);
  popOut_->setStartValue(raisedGeometry);
  popOut_->setEndValue(loweredGeometry);

  // Remove any previous disconnection to hide the window (from the soft close)
  disconnect(toast_, SIGNAL(mouseLeft()), popOut_, SLOT(start()));

  // Show the button, raise it, and start the animation
  toast_->show();
  toast_->raise();
  popIn_->start();
}

void ToastOnWidget::softCloseToast_()
{
  if (toast_->isMouseInside())
  {
    // Need to disconnect this signal later so that future show()'s don't hide immediately
    connect(toast_, SIGNAL(mouseLeft()), popOut_, SLOT(start()));
  }
  else
  {
    popOut_->start();
  }
}

//////////////////////////////////////////////////////////////////

ClickableLabel::ClickableLabel(QWidget* parent)
  : QLabel(parent),
    mouseInside_(false)
{
}

bool ClickableLabel::isMouseInside() const
{
  return mouseInside_;
}

void ClickableLabel::mousePressEvent(QMouseEvent* evt)
{
  QLabel::mousePressEvent(evt);
  evt->setAccepted(true);
  emit pressed();
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* evt)
{
  QLabel::mouseReleaseEvent(evt);
  evt->setAccepted(true);
  emit clicked();
}

void ClickableLabel::enterEvent(QEvent* evt)
{
  mouseInside_ = true;
  emit mouseEntered();
  return QLabel::enterEvent(evt);
}

void ClickableLabel::leaveEvent(QEvent* evt)
{
  mouseInside_ = false;
  emit mouseLeft();
  return QLabel::leaveEvent(evt);
}

} // namespace simQt
