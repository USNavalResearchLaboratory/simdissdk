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
#include <cassert>
#include <QKeyEvent>
#include <QWidget>
#include "simQt/OverrideShortcut.h"

namespace simQt {

OverrideShortcut::OverrideShortcut(QWidget* parent)
  : QObject(parent)
{
  if (parent)
    parent->installEventFilter(this);
}

OverrideShortcut::OverrideShortcut(const QKeySequence& key, QWidget* parent)
  : OverrideShortcut(parent)
{
  keys_.push_back(key);
}

OverrideShortcut::OverrideShortcut(const std::vector<QKeySequence>& keys, QWidget* parent)
  : OverrideShortcut(parent)
{
  keys_ = keys;
}

void OverrideShortcut::setKey(const QKeySequence& key)
{
  keys_ = { key };
}

void OverrideShortcut::setKeys(const std::vector<QKeySequence>& keys)
{
  keys_ = keys;
}

std::vector<QKeySequence> OverrideShortcut::keys() const
{
  return keys_;
}

void OverrideShortcut::setEnabled(bool enabled)
{
  enabled_ = enabled;
}

bool OverrideShortcut::isEnabled() const
{
  return enabled_;
}

bool OverrideShortcut::isMatch_(const QKeyEvent* keyEvent) const
{
  if (keys_.empty())
    return false;

  // Convert the raw key event into an integer that QKeySequence understands.
  // We strip out irrelevant modifiers like NumLock/Keypad to ensure clean matching.
  const int keyInt = keyEvent->key();
  const Qt::KeyboardModifiers mods = keyEvent->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);

  const QKeySequence eventSeq(keyInt | mods);
  for (const QKeySequence& seq : keys_)
  {
    // QKeySequence::matches handles complex matching logic safely
    if (seq.matches(eventSeq) == QKeySequence::ExactMatch)
      return true;
  }
  return false;
}

bool OverrideShortcut::eventFilter(QObject* object, QEvent* evt)
{
  // Must be valid and enabled
  if (!enabled_ || keys_.empty())
    return QObject::eventFilter(object, evt);
  // Must be a keypress or shortcut override notify
  if (evt->type() != QEvent::ShortcutOverride && evt->type() != QEvent::KeyPress)
    return QObject::eventFilter(object, evt);

  QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
  if (!isMatch_(keyEvent))
    return QObject::eventFilter(object, evt);

  if (evt->type() == QEvent::ShortcutOverride)
  {
    // Defense: Claim the key to prevent global ambiguity/QAction triggers
    keyEvent->accept();
    return true;
  }

  if (evt->type() == QEvent::KeyPress)
  {
    // Consume the key press and emit our signal
    keyEvent->accept();
    Q_EMIT activated();
    return true;
  }

  // Not possible to get here
  assert(0);
  return QObject::eventFilter(object, evt);
}

}
