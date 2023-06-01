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
#include <QLineEdit>
#include <QKeyEvent>
#include "simQt/KeySequenceEdit.h"

namespace simQt {

KeySequenceEdit::KeySequenceEdit(QWidget* parent)
  : QLineEdit(parent)
{
  // Sets a strong focus to prevent accidental focus issues, and sets
  // similar flags to Qt5's QKeySequenceEdit
  setReadOnly(true);
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_MacShowFocusRect, true);
  setAttribute(Qt::WA_InputMethodEnabled, false);
  setPlaceholderText(tr("Press shortcut"));
}

KeySequenceEdit::~KeySequenceEdit()
{
}

QKeySequence KeySequenceEdit::key() const
{
  return key_;
}

bool KeySequenceEdit::isKeyValid() const
{
  return !key_.isEmpty();
}

void KeySequenceEdit::setKey(const QKeySequence& key, bool emitSignal)
{
  if (key_ == key)
    return;

  key_ = key;
  setText(key_.toString());
  if (emitSignal)
  {
    Q_EMIT(keyChanged(key));
  }
}

void KeySequenceEdit::acceptKey(const QKeyEvent* keyEvent)
{
  Qt::Key key = static_cast<Qt::Key>(keyEvent->key());

  // Filter the key
  if (key == Qt::Key_unknown || key == Qt::Key_Control || key == Qt::Key_Shift ||
    key == Qt::Key_Alt || key == Qt::Key_Meta)
  {
    // Unknown key could be from a macro according to the source of this code.  Other
    // keys are modifier keys and indicate the user just started a modifier sequence.
    // Don't want to serialize this yet to a key sequence, so we just ignore and return
    return;
  }

  // Combine all of the modifiers into a single keyInt
  Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
  int keyInt = key;
  const QString text = keyEvent->text();

  // From Qt5's QKeySequenceEdit:
  // The shift modifier only counts when it is not used to type a symbol
  // that is only reachable using the shift key anyway
  if (modifiers & Qt::ShiftModifier && (text.isEmpty() ||
                                        !text.at(0).isPrint() ||
                                        text.at(0).isLetterOrNumber() ||
                                        text.at(0).isSpace()))
    keyInt += Qt::SHIFT;
  if (modifiers & Qt::ControlModifier)
    keyInt += Qt::CTRL;
  if (modifiers & Qt::AltModifier)
    keyInt += Qt::ALT;
  if (modifiers & Qt::MetaModifier)
    keyInt += Qt::META;
  if (modifiers & Qt::KeypadModifier)
    keyInt += Qt::KeypadModifier;

  if (keyInt == Qt::Key_Escape)
  {
    // Escape is a special key used to clear out hotkeys.  Nothing can be bound to escape
    // in this scheme, but you can bind e.g. Shift+Esc
    setKey(QKeySequence(), true);
  }
  else
  {
    // Create the hotkey mapping
    setKey(QKeySequence(keyInt), true);
  }
}

void KeySequenceEdit::keyPressEvent(QKeyEvent* keyEvent)
{
  acceptKey(keyEvent);
}

bool KeySequenceEdit::event(QEvent* evt)
{
  // KeySequenceEdit will drop all shortcut and shortcut override events
  // so that we can capture the key press event for creating new shortcuts
  switch (evt->type())
  {
  case QEvent::Shortcut:
    return true;
  case QEvent::ShortcutOverride:
    evt->accept();
    return true;
  default:
    break;
  }
  return QWidget::event(evt);
}

}
