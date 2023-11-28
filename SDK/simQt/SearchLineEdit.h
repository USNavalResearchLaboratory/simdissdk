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
#ifndef SIMQT_SEARCHLINEEDIT_H
#define SIMQT_SEARCHLINEEDIT_H

#include <QFrame>
#include <QLineEdit>
#include <QPixmap>
#include "simCore/Common/Common.h"

class QProxyStyle;
class QTimer;
class QToolButton;
class QWidgetAction;

namespace simQt
{

/**
 * The SearchLineEdit subclasses the QLineEdit, adding in a magnifying glass
 * icon on the left side and enabling the clear button by default.
 *
 * Uses a timer and the searchRequested() signal to allow a delay
 * before triggering a (potentially costly) search operation
 */
class SDKQT_EXPORT SearchLineEdit : public QLineEdit
{
  Q_OBJECT;
public:
  explicit SearchLineEdit(QWidget* parent = nullptr);
  virtual ~SearchLineEdit();

  /** Interval (milliseconds) after editing before sending signal searchRequested() */
  int searchDelayInterval() const;
  /** True if the search icon should be shown, false otherwise */
  bool searchIconEnabled() const;

public Q_SLOTS:
  /** Changes the search icon.  @see QLabel::setPixmap() */
  void setSearchPixmap(const QPixmap& pixmap);
  /** Sets interval in milliseconds after an edit to send out searchRequested() signal */
  void setSearchDelayInterval(int msec);
  /** Set true to show search icon (default), or false to hide it */
  void setSearchIconEnabled(bool enabled);

Q_SIGNALS:
  /** Timer has expired after last edit.  Tie into this for a convenient method to activate your search */
  void searchRequested(const QString& text);

private Q_SLOTS:
  /** Responsible for emitting searchRequested() with appropriate text, from QTimer */
  void emitSearchRequested_();

private:
  QTimer* searchTimer_;
  QWidgetAction* iconAction_;
  bool iconEnabled_;
  QProxyStyle* proxyStyle_;
};

}

#endif /* SIMQT_SEARCHLINEEDIT_H */
