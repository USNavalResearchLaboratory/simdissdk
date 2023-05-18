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
#ifndef SIMQT_KEYSEQUENCEEDIT_H
#define SIMQT_KEYSEQUENCEEDIT_H

#include <QKeySequence>
#include <QLineEdit>
#include "simCore/Common/Common.h"

namespace simQt {

/** Line edit class for editing QKeySequences.  The widget supports most keys that are
 * not preprocessed by the operating system (e.g. Shift+Esc, Alt+Tab).  This has been
 * designed to be used alongside the ActionItemModelDelegate editor for the ActionItemModel
 * item model.
 */
class SDKQT_EXPORT KeySequenceEdit : public QLineEdit
{
  Q_OBJECT;
public:
  /// constructor
  explicit KeySequenceEdit(QWidget* parent=nullptr);
  virtual ~KeySequenceEdit();

  ///@return most recent key set by this widget
  QKeySequence key() const;
  ///@return true if the key sequence is valid
  bool isKeyValid() const;
  /// Sets a key sequence, optionally emitting keyChanged
  void setKey(const QKeySequence& key, bool emitSignal=false);

public Q_SLOTS:

  /**
   * Call this function to notify on key press.  Note that this can be called from the
   * QStyledItemDelegate::eventFilter() function.  This is present in order to accept a
   * larger set of keys than would be available without the eventFilter() override.
   */
  void acceptKey(const QKeyEvent* keyEvent);

protected:
  /**
   * Override the QLineEdit's keyPressEvent and pass the event to acceptKey().  If you
   * have problems with keys like Tab, Shift Tab, Escape, etc., consider looking at whether
   * there is an event filter set up that will omit these keys.  In the case of
   * ActionItemModelDelegate, the eventFilter() code forwards key events to acceptKey()
   * directly in order to bypass filtering of these special keys.
   */
  virtual void keyPressEvent(QKeyEvent* keyEvent);

  /** Override event() to ignore Shortcut and ShortcutOverride events */
  virtual bool event(QEvent* evt);

Q_SIGNALS:
  /// Hot key has been changed; newKey.isEmpty() means the key was removed
  void keyChanged(const QKeySequence& newKey);

private:
  QKeySequence key_;
};

} // namespace

#endif /* SIMQT_KEYSEQUENCEEDIT_H */
