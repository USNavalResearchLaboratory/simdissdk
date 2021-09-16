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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include <limits>
#include <QCheckBox>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include "simQt/ColorWidget.h"
#include "simQt/FileSelectorWidget.h"
#include "simQt/ValueMerge.h"

namespace simQt {

/** Helper method that sets a widget's font to indicate value conflict */
template<typename Widget>
void setConflictFont(Widget* widget, bool conflict)
{
  QFont font = widget->font();
  // if already matches, do nothing
  if (font.italic() == conflict)
    return;
  font.setItalic(conflict);
  widget->setFont(font);
}

//////////////////////////////////////////////////////////////

SpinBoxValueMerge::SpinBoxValueMerge(QSpinBox* spinBox, QLabel* label, int defaultValue)
  : ValueMerge<int>(defaultValue),
  spinBox_(spinBox),
  label_(label)
{
}

SpinBoxValueMerge::~SpinBoxValueMerge()
{
  // Apply text/value as needed to widget and label
  setConflictFont(label_, conflict_);

  int valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (spinBox_->value() != valueToSet)
    spinBox_->setValue(valueToSet);
}

//////////////////////////////////////////////////////////////

SpinBoxUValueMerge::SpinBoxUValueMerge(QSpinBox* spinBox, QLabel* label, unsigned int defaultValue)
  : ValueMerge<unsigned int>(defaultValue),
  spinBox_(spinBox),
  label_(label)
{
}

SpinBoxUValueMerge::~SpinBoxUValueMerge()
{
  // Apply text/value as needed to widget and label
  setConflictFont(label_, conflict_);

  int valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (spinBox_->value() != valueToSet)
    spinBox_->setValue(valueToSet);
}

//////////////////////////////////////////////////////////////

CheckBoxValueMerge::CheckBoxValueMerge(QCheckBox* checkBox, QLabel* label, bool defaultValue)
  : ValueMerge<bool>(defaultValue),
  checkBox_(checkBox),
  label_(label)
{
  // This class does not support tristate check boxes
  assert(!checkBox_->isTristate());
}

CheckBoxValueMerge::~CheckBoxValueMerge()
{
  // Apply text/value as needed to widget and label
  if (label_)
    setConflictFont(label_, conflict_);
  else
    setConflictFont(checkBox_, conflict_);

  bool valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (checkBox_->isChecked() != valueToSet)
    checkBox_->setChecked(valueToSet);
}

//////////////////////////////////////////////////////////////

LineEditValueMerge::LineEditValueMerge(QLineEdit* lineEdit, QLabel* label)
  : ValueMerge<QString>(QString()),
  lineEdit_(lineEdit),
  label_(label)
{
}

LineEditValueMerge::~LineEditValueMerge()
{
  // Apply text/value as needed to widget and label
  setConflictFont(label_, conflict_);

  QString valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (lineEdit_->text() != valueToSet)
    lineEdit_->setText(valueToSet);
}

//////////////////////////////////////////////////////////////

FileSelectorValueMerge::FileSelectorValueMerge(simQt::FileSelectorWidget* fileSelector, QLabel* label)
  : ValueMerge<QString>(QString()),
  fileSelector_(fileSelector),
  label_(label)
{
}

FileSelectorValueMerge::~FileSelectorValueMerge()
{
  // Apply text/value as needed to widget and label
  setConflictFont(label_, conflict_);

  QString valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (fileSelector_->filename() != valueToSet)
    fileSelector_->setFilename(valueToSet);
}

//////////////////////////////////////////////////////////////

ColorValueMerge::ColorValueMerge(simQt::ColorWidget* colorWidget, QLabel* label, const QColor& defaultValue)
  : ValueMerge<QColor>(defaultValue),
  colorWidget_(colorWidget),
  label_(label)
{
}

ColorValueMerge::~ColorValueMerge()
{
  // Apply text/value as needed to widget and label
  setConflictFont(label_, conflict_);

  QColor valueToSet = value_;
  if (conflict_ || !valueSet_)
    valueToSet = defaultValue_;

  if (colorWidget_->color() != valueToSet)
    colorWidget_->setColor(valueToSet);
}

}
