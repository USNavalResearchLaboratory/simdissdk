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

#include <vector>
#include <QKeySequence>
#include <QObject>
#include "simCore/Common/Export.h"

class QKeyEvent;
class QWidget;

namespace simQt {

/**
 * A QShortcut-like utility that intercepts raw key events to prevent
 * global shortcut ambiguity warnings. It triggers its activated() signal
 * forcefully when the exact key sequence is pressed within the parent
 * widget. For example:
 *
 * <code>
 * simQt::OverrideShortcut* nextShortcut = new OverrideShortcut(Qt::CTRL | Qt::Key_Tab, myWindow);
 * connect(nextShortcut, &simQt::OverrideShortcut::activated, this, &MyWindow::nextTab);
 * </code>
 */
class SDKQT_EXPORT OverrideShortcut : public QObject
{
  Q_OBJECT;
public:
  /** Installs an override shortcut to the given parent, with key to be set later */
  explicit OverrideShortcut(QWidget* parent);
  /** Installs an override shortcut with given key sequence to the given parent */
  explicit OverrideShortcut(const QKeySequence& key, QWidget* parent);
  /** Installs an override shortcut list with given key sequence to the given parent */
  explicit OverrideShortcut(const std::vector<QKeySequence>& keys, QWidget* parent);

  /** Sets a single key sequence for the shortcut */
  void setKey(const QKeySequence& key);
  /** Sets a multiple key sequences for the shortcut */
  void setKeys(const std::vector<QKeySequence>& keys);
  /** Retrieves all currently set key sequences */
  std::vector<QKeySequence> keys() const;

  /** Set true to enable, false to disable */
  void setEnabled(bool enabled);
  /** True if enabled */
  bool isEnabled() const;

Q_SIGNALS:
  void activated();

protected:
  // From QObject:
  bool eventFilter(QObject* object, QEvent* evt) override;

private:
  /** True if the key event action matches shortcut we're looking for */
  bool isMatch_(const QKeyEvent* keyEvent) const;

  std::vector<QKeySequence> keys_;
  bool enabled_ = true;
};

}
