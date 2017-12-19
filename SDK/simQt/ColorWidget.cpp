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
#include <QColorDialog>
#include <QPushButton>
#include <QLabel>
#include "simQt/ColorWidget.h"
#include "ui_ColorWidget.h"

namespace simQt {

// On Linux, avoid the native dialog due to popup stacking problems with System GUI (SIMDIS-2466)
#ifndef WIN32
static const QColorDialog::ColorDialogOption BASE_OPTIONS = QColorDialog::DontUseNativeDialog;
#else
static const QColorDialog::ColorDialogOption BASE_OPTIONS = static_cast<QColorDialog::ColorDialogOption>(0x0);
#endif

ColorWidget::ColorWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_ColorWidget),
    color_(QColor(0,0,0,255)),
    title_(tr("Choose Color")),
    text_(tr("Color")),
    showAlpha_(true),
    includeText_(true),
    showDialog_(true)
{
  ui_->setupUi(this);
  ui_->colorLabel->setText(text_); // default label text
  ui_->colorButton->setShowAlpha(showAlpha_);
  // cache the layout spacing
  spacing_ = ui_->horizontalLayout->spacing();
  setColorButton_();
  connect(ui_->colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog_()));
}

ColorWidget::~ColorWidget()
{
  delete ui_;
}

QColor ColorWidget::color() const
{
  return color_;
}

void ColorWidget::setColor(const QColor& value)
{
  color_ = value;
  setColorButton_();
}

QString ColorWidget::text() const
{
  return text_;
}

QString ColorWidget::dialogTitle() const
{
  return title_;
}

bool ColorWidget::showAlpha() const
{
  return showAlpha_;
}

void ColorWidget::setText(const QString& text)
{
  if (text_ == text)
    return;
  text_ = text;
  if (includeText_)
    ui_->colorLabel->setText(text_);
  // Hide the label if the text is empty, or show it if it's non-empty and includeText_ is true
  updateLabelVisibility_();
}

void ColorWidget::setDialogTitle(const QString& title)
{
  title_ = title;
}

void ColorWidget::setShowAlpha(bool showAlpha)
{
  showAlpha_ = showAlpha;
  ui_->colorButton->setShowAlpha(showAlpha);
}

void ColorWidget::setIncludeText(bool include)
{
  if (include == includeText_)
    return;
  includeText_ = include;
  ui_->colorLabel->setText(include ? text_ : "");
  updateLabelVisibility_();
}

bool ColorWidget::dialogEnable() const
{
  return showDialog_;
}

void ColorWidget::setDialogEnable(bool value)
{
  showDialog_ = value;
}

void ColorWidget::updateLabelVisibility_()
{
  // The layout spacing is updated based on if includeText_ is set and if the label is
  // non-empty.  This prevents a common error where a user will set an empty string.
  // When there is no label text, there is extra spacing on the right side
  // of the color well that is undesired. Update the layout spacing for this case.
  bool visible = includeText_ && !ui_->colorLabel->text().isEmpty();
  ui_->horizontalLayout->setSpacing(visible ? spacing_ : 0);
}

bool ColorWidget::includeText() const
{
  return includeText_;
}

void ColorWidget::showColorDialog_()
{
  if (!showDialog_)
    return;

  QColor tempColor;
  if (showAlpha_)
    tempColor = QColorDialog::getColor(color_, this, title_, BASE_OPTIONS | QColorDialog::ShowAlphaChannel);
  else
    tempColor = QColorDialog::getColor(color_, this, title_, BASE_OPTIONS);
  if (tempColor.isValid())
  {
    color_ = tempColor;
    setColorButton_();
    emit(colorChanged(color_));
  }
}

void ColorWidget::setColorButton_()
{
  if (!isEnabled())
    ui_->colorButton->setColor(QColor(0, 0, 0, 0));
  else
    ui_->colorButton->setColor(color_);
}

void ColorWidget::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);
  if (event->type() == QEvent::EnabledChange)
    setColorButton_();
}

simQt::ColorButton* ColorWidget::colorButton_() const
{
  return ui_->colorButton;
}

QLabel* ColorWidget::colorLabel_() const
{
  return ui_->colorLabel;
}

}
