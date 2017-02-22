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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_SEARCHLINEEDIT_H
#define SIMQT_SEARCHLINEEDIT_H

#include <QFrame>
#include <QPixmap>
#include "simCore/Common/Common.h"

class Ui_SearchLineEdit;
class QTimer;
class QLineEdit;
class QToolButton;

namespace simQt
{

/**
 * QLineEdit-based widget that contains a search pane and has search-friendly features.
 *
 * The SearchLineEdit class displays a unified line edit widget that shows an optional icon
 * icon on left side of the input text, and (where supported by the underlying QLineEdit) an
 * optional clear button on the right side of the line edit.  This is a convenience widget
 * wrapping a QLineEdit intended to be used for uniform look-and-feel.
 *
 * SearchLineEdit provides access to the clearButtonEnabled property, similar to that of
 * QLineEdit, but reimplemented for uniform support across Qt4 and Qt5.
 *
 * In addition to the other features, the SearchLineEdit queues keypresses from the textChanged()
 * signal and will send them out searchDelayInterval() milliseconds after the last keypress.
 * This can be useful to consolidate several quick filtering operations into a single operation,
 * and shows most benefit for end users who are fast typers.  Tie into the searchRequested()
 * signal to use this feature.
 */
class SDKQT_EXPORT SearchLineEdit : public QFrame
{
  Q_OBJECT;
  Q_PROPERTY(QString text READ text WRITE setText);
  Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText);
  Q_PROPERTY(QPixmap searchPixmap READ searchPixmap WRITE setSearchPixmap);
  Q_PROPERTY(int searchDelayInterval READ searchDelayInterval WRITE setSearchDelayInterval);
  Q_PROPERTY(bool clearButtonEnabled READ clearButtonEnabled WRITE setClearButtonEnabled);
  Q_PROPERTY(bool searchIconEnabled READ searchIconEnabled WRITE setSearchIconEnabled);

public:
  /** Constructor */
  explicit SearchLineEdit(QWidget* parent=NULL);
  virtual ~SearchLineEdit();

  /** @see QLineEdit::text() */
  QString text() const;
  /** @see QLineEdit::placeholderText() */
  QString placeholderText() const;
  /** Image for search.  @see QLabel::pixmap() */
  const QPixmap* searchPixmap() const;
  /** Interval (milliseconds) after editing before sending signal searchRequested() */
  int searchDelayInterval() const;
  /** @see QLineEdit::clearButtonEnabled() */
  bool clearButtonEnabled() const;
  /** True if the search icon should be shown, false otherwise */
  bool searchIconEnabled() const;

  /** Retrieve the QLineEdit wrapped by this SearchLineEdit */
  QLineEdit* lineEdit() const;

public slots:
  /** @see QLineEdit::setText() */
  void setText(const QString& text);
  /** @see QLineEdit::clear() */
  void clear();
  /** @see QLineEdit::selectAll() */
  void selectAll();
  /** @see QLineEdit::setPlaceholderText() */
  void setPlaceholderText(const QString& text);
  /** Changes the search icon.  @see QLabel::setPixmap() */
  void setSearchPixmap(const QPixmap& pixmap);
  /** Sets interval in milliseconds after an edit to send out searchRequested() signal */
  void setSearchDelayInterval(int msec);
  /** @see QLineEdit::setClearButtonEnabled() */
  void setClearButtonEnabled(bool enabled);
  /** Set true to show search icon (default), or false to hide it */
  void setSearchIconEnabled(bool enabled);

signals:
  /** @see QLineEdit::editingFinished() */
  void editingFinished();
  /** @see QLineEdit::returnPressed() */
  void returnPressed();
  /** @see QLineEdit::textChanged() */
  void textChanged(const QString& text);
  /** @see QLineEdit::textEdited() */
  void textEdited(const QString& text);
  /** Timer has expired after last edit.  Tie into this for a convenient method to activate your search */
  void searchRequested(const QString& text);

private slots:
  /** Responsible for emitting searchRequested() with appropriate text, from QTimer */
  void emitSearchRequested_();
  /** Turns the clear icon on or off based on the clearButtonEnabled_ and state of the text */
  void showOrHideClearButton_();

protected:
  /** Adjust the size of the clear button */
  virtual void resizeEvent(QResizeEvent* evt);

private:
  bool clearButtonEnabled_;
  QTimer* searchTimer_;
  Ui_SearchLineEdit* ui_;
  QToolButton* clearButton_;
};

}

#endif /* SIMQT_SEARCHLINEEDIT_H */
