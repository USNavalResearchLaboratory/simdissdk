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
#pragma once

#include <QFrame>
#include "simCore/Common/Export.h"

class QEvent;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QWidget;

namespace simQt {

/**
* @brief Shows information in a banner that drops from the top of the parent widget
*/
class SDKQT_EXPORT NotificationBanner : public QFrame
{
  Q_OBJECT

public:
  /** Defines the severity levels for outgoing messages */
  enum class Level {
    Info,
    Warning,
    Error
  };

  explicit NotificationBanner(QWidget& parent); // must be non-const ref, installing an event filter means that a nullptr parent is invalid

  // Prevent copy/default construction
  NotificationBanner() = delete;
  NotificationBanner(const NotificationBanner&) = delete;
  NotificationBanner& operator=(const NotificationBanner&) = delete;

  /**
  * @brief Displays the banner with specific styling based on the severity level
  * @param level The severity level (Info, Warning, Error)
  * @param message The text to display
  * @param timeoutMs Auto-dismiss timer (0 = disabled)
  */
  void showMessage(Level level, const QString& message, int timeoutMs = 0);
public Q_SLOTS:
  /// Hides the banner on timeout or exit
  void hideBanner();

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
  /**
  * @brief Handles the ending of different animations
  */
  void resolveStateTransition_();

private:
  /// Updates the position of the widget
  void updatePosition_();
  /**
  * @brief Applies a specific styling to the widget based on its severity level
  * @param level The severity level of the event
  */
  void applyStyling_(Level level);

  /**
  * @brief Private enum to help handle animation states
  * @details This enum class tracks the animations' lifecycles.
  */
  enum class AnimationState
  {
    Idle, /// Idle state, not moving
    SlidingIn, /// New alert coming through, no alert already on parent
    SlidingOut, /// Alert leaving via timeout or exit
    Bouncing /// New alert, with an alert already on parent
  };
  AnimationState animState_ = simQt::NotificationBanner::AnimationState::Idle;

  QLabel* titleLabel_ = nullptr;
  QLabel* messageLabel_ = nullptr;
  QLabel* subtitleLabel_ = nullptr;
  QPushButton* closeButton_ = nullptr;
  QPropertyAnimation* animation_ = nullptr;
  QTimer* timer_ = nullptr;
  int timeoutMs_ = 0;

};

}
