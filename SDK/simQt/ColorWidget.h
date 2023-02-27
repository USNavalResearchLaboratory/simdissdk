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
#ifndef SIMQT_COLORWIDGET_H
#define SIMQT_COLORWIDGET_H

#include <QColor>
#include <QColorDialog>
#include <QString>
#include <QWidget>
#include "simCore/Common/Export.h"

class Ui_ColorWidget;
class QLabel;

namespace simQt {

/** Dialog flag that detects when to avoid the native dialog */
extern const QColorDialog::ColorDialogOption COLOR_DIALOG_OPTIONS;

class ColorButton;

/**
 * ColorWidget is a widget that shows a color square and a text label.  Pressing the color
 * square opens the QColorDialog, with a title defined by the user.  The color selected
 * is passed to the user through a signal, or by querying the object for its current color.
 */
class SDKQT_EXPORT ColorWidget : public QWidget  // QDESIGNER_WIDGET_EXPORT
{
  Q_OBJECT;

  /** Sets/gets the initial color in Qt Designer */
  Q_PROPERTY(QColor InitialColor READ color WRITE setColor)
  /** Sets/gets the initial text in Qt Designer */
  Q_PROPERTY(QString Text READ text WRITE setText)
  /** Sets/gets the dialog title in Qt Designer */
  Q_PROPERTY(QString DialogTitle READ dialogTitle WRITE setDialogTitle)
  /** Show/hide alpha in Qt Designer */
  Q_PROPERTY(bool ShowAlpha READ showAlpha WRITE setShowAlpha)
  /** Show/hide text in Qt Designer */
  Q_PROPERTY(bool IncludeText READ includeText WRITE setIncludeText);
  /** Enable/Disable Color Dialog in Qt Designer */
  Q_PROPERTY(bool EnableColorDialog READ dialogEnable WRITE setDialogEnable);

public:
  /** Constructor */
  ColorWidget(QWidget* parent = nullptr);
  virtual ~ColorWidget();

  /** returns the current color selection */
  QColor color() const;
  /** returns the label text */
  QString text() const;
  /** returns the QColorDialogTitle */
  QString dialogTitle() const;
  /** returns whether to show alpha channel */
  bool showAlpha() const;
  /** Returns true if we should include the text in the display */
  bool includeText() const;
  /** Return true if clicking on the color well will display the color dialog */
  bool dialogEnable() const;

public Q_SLOTS:
  /** Changes the color of the widget */
  void setColor(const QColor& value);
  /** set the label text in the widget */
  void setText(const QString& text);
  /** set the QColorDialog title */
  void setDialogTitle(const QString& title);
  /** set whether to show alpha channel or not */
  void setShowAlpha(bool showAlpha);
  /** Sets a flag indicating whether display text is shown. */
  void setIncludeText(bool include);
  /** Sets a flag indicating whether clicking on the color well will display the color dialog */
  void setDialogEnable(bool value);
  /** Retrieve the pointer to the color label */
  QLabel* colorLabel() const;

Q_SIGNALS:
  /** emitted when a color selection is made */
  void colorChanged(const QColor& color);

private Q_SLOTS:
  /** handles internally updating the selected color, and passing it out to the user */
  void showColorDialog_();

protected:
  /** Override change event to know when we're disabled */
  virtual void changeEvent(QEvent* event);

  /** Retrieve the pointer to the color button */
  simQt::ColorButton* colorButton_() const;
  /** Retrieve the pointer to the color label */
  QLabel* colorLabel_() const;

private:
  /** set up the color button based on enabled state */
  void setColorButton_();
  /** Shows or hides the label based on the flag, and the empty-string check */
  void updateLabelVisibility_();

  Ui_ColorWidget* ui_;
  QColor color_;
  QString title_;
  QString text_;
  bool showAlpha_;
  bool includeText_;
  bool showDialog_;
  int spacing_;
};

}

#endif /* SIMQT_COLORWIDGET_H */
