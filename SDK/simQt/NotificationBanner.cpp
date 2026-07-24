/* -*- mode: c++ -*- */
/*******************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 *******************************************************************************
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
 *****************************************************************************/
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include "NotificationBanner.h"

namespace simQt {

NotificationBanner::NotificationBanner(QWidget& parent)
  : QFrame(&parent)
{
  setObjectName("NotificationBannerWidget");
  setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(15, 10, 15, 15);
  mainLayout->setSpacing(5);

  QHBoxLayout* topRow = new QHBoxLayout();
  topRow->setContentsMargins(0, 0, 0, 0);

  titleLabel_ = new QLabel(this);
  titleLabel_->setStyleSheet("font-weight: bold; font-size: 14px; letter-spacing: 1px;");
  topRow->addWidget(titleLabel_);

  topRow->addStretch(); // Pushes close button to the right

  closeButton_ = new QPushButton(tr("X"), this);
  closeButton_->setFixedSize(20, 20);
  closeButton_->setFlat(true);
  topRow->addWidget(closeButton_);
  connect(closeButton_, &QPushButton::clicked, this, &NotificationBanner::hideBanner);

  mainLayout->addLayout(topRow);

  messageLabel_ = new QLabel(this);
  messageLabel_->setWordWrap(true);
  messageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  mainLayout->addWidget(messageLabel_);

  subtitleLabel_ = new QLabel(tr("Check console for full history and details."), this);
  subtitleLabel_->setWordWrap(true);
  subtitleLabel_->setStyleSheet("font-size: 11px; font-style: italic; color: rgba(255, 255, 255, 180); ");
  mainLayout->addWidget(subtitleLabel_);
  subtitleLabel_->hide();

  timer_ = new QTimer(this);
  timer_->setSingleShot(true);
  connect(timer_, &QTimer::timeout, this, &NotificationBanner::hideBanner);

  animation_ = new QPropertyAnimation(this, "pos", this);
  animation_->setDuration(400);
  animation_->setEasingCurve(QEasingCurve::OutCubic);

  connect(animation_, &QPropertyAnimation::finished, this, &NotificationBanner::resolveStateTransition_);

  parent.installEventFilter(this);
  hide();
}

void NotificationBanner::applyStyling_(Level level)
{
  QString bgColor;
  QString borderColor;
  QString titleColor;
  QString titleText;

  switch (level)
  {
  case simQt::NotificationBanner::Level::Error:
    titleText = tr("ERROR");
    bgColor = "rgba(220, 38, 38, 230)";      // Dark Red
    borderColor = "rgba(255, 100, 100, 150)";
    titleColor = "#ffffff";
    break;
  case simQt::NotificationBanner::Level::Warning:
    titleText = tr("WARNING");
    bgColor = "rgba(234, 152, 35, 230)";     // Muted Orange
    borderColor = "rgba(255, 200, 100, 150)";
    titleColor = "#ffffff";
    break;
  case simQt::NotificationBanner::Level::Info:
  default:
    titleText = tr("INFO");
    bgColor = "rgba(59, 130, 246, 230)";     // Soft Blue
    borderColor = "rgba(100, 150, 255, 150)";
    titleColor = "#ffffff";
    break;
  }

  titleLabel_->setText(titleText);

  setStyleSheet(QString(
    "#NotificationBannerWidget { "
    "  background-color: %1; "
    "  border: 1px solid %2; "
    "  border-radius: 6px; "
    "} "
    "QLabel { color: %3; background: transparent; } "
    "QPushButton { color: rgba(255, 255, 255, 180); font-weight: bold; border: none; background: transparent; } "
    "QPushButton:hover { color: #ffffff; }"
  ).arg(bgColor).arg(borderColor).arg(titleColor));
}

void NotificationBanner::showMessage(Level level, const QString& message, int timeoutMs)
{
  if (!parentWidget())
    return;

  animation_->stop();
  timer_->stop();
  timeoutMs_ = timeoutMs;

  applyStyling_(level);

  int idealWidth = parentWidget()->width() * 0.5;
  int finalWidth = qBound(280, idealWidth, 550);

  if (finalWidth > parentWidget()->width() - 40)
    finalWidth = parentWidget()->width() - 40;

  setFixedWidth(finalWidth);

  // Different animation if another message is on-screen
  if (isVisible() && y() > 0)
  {
    subtitleLabel_->show();

    messageLabel_->setText(message);
    adjustSize(); // Update size to accommodate the text

    const int xPos = (parentWidget()->width() - width()) / 2;
    const int currentY = y();
    setGeometry(xPos, currentY, width(), height());

    animState_ = simQt::NotificationBanner::AnimationState::Bouncing;

    if (timeoutMs_ > 0)
      timer_->start(timeoutMs_);

    animation_->setStartValue(QPoint(xPos, currentY));
    animation_->setKeyValueAt(0.5, QPoint(xPos, currentY + 8)); // Little bounce
    animation_->setEndValue(QPoint(xPos, currentY));
    animation_->start();

    return;
  }

  subtitleLabel_->hide();

  messageLabel_->setText(message);
  adjustSize();

  const int startY = -height();
  const int endY = 25;
  const int xPos = (parentWidget()->width() - width()) / 2;

  const int currentY = isVisible() ? y() : startY;

  setGeometry(xPos, currentY, width(), height());

  raise();
  show();

  animState_ = simQt::NotificationBanner::AnimationState::SlidingIn;

  animation_->setStartValue(QPoint(xPos, currentY));
  animation_->setEndValue(QPoint(xPos, endY));
  animation_->start();
}

void NotificationBanner::hideBanner()
{
  if (!parentWidget())
    return;

  const int startY = y();
  const int endY = -height();
  const int xPos = x();

  animation_->stop();

  animState_ = simQt::NotificationBanner::AnimationState::SlidingOut;

  animation_->setStartValue(QPoint(xPos, startY));
  animation_->setEndValue(QPoint(xPos, endY));

  animation_->start();
}

bool NotificationBanner::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == parentWidget() && event->type() == QEvent::Resize)
    updatePosition_();

  return QFrame::eventFilter(watched, event);
}

void NotificationBanner::updatePosition_()
{
  if (!isVisible() || !parentWidget())
    return;

  const int xPos = (parentWidget()->width() - width()) / 2;
  move(xPos, y());
}

void NotificationBanner::resolveStateTransition_()
{
  switch (animState_)
  {
  case simQt::NotificationBanner::AnimationState::SlidingIn:
  case simQt::NotificationBanner::AnimationState::Bouncing:
    // Start countdown timer to auto-dismiss
    if (timeoutMs_ > 0)
      timer_->start(timeoutMs_);
    animState_ = simQt::NotificationBanner::AnimationState::Idle;
    break;
  case simQt::NotificationBanner::AnimationState::SlidingOut:
    // Hide the widget from the display now that it is off-screen
    hide();
    animState_ = simQt::NotificationBanner::AnimationState::Idle;
    break;
  case simQt::NotificationBanner::AnimationState::Idle:
    break;
  }
}

} // namespace simQt
