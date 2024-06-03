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
#ifndef SIMQT_MERGEVALUES_H
#define SIMQT_MERGEVALUES_H

#include <string>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <osg/Vec4f>
#include "simQt/ColorWidget.h"
#include "simQt/QtConversion.h"
#include "simQt/FontWidget.h"
#include "simQt/TimeWidget.h"

namespace simQt {

/** helper method to set a widget's font to indicate value conflict */
template<class T>
void setConflictFont(T* widget, bool conflict)
{
  QFont font = widget->font();
  // if already matches, do nothing
  if (font.italic() == conflict)
    return;
  font.setItalic(conflict);
  widget->setFont(font);
}

/**
 * Utility base class to manage merging multiple values. Useful when
 * attempting to represent multiple values that may or may not vary
 * in a Qt widget, e.g. when multiple objects are selected in a list
 * and a Qt widget is responsible for displaying values for the selected
 * objects. To use, create an instance of this class (or one of its
 * derived classes), then call applyValue() using each value that the
 * Qt widget is concerned with. If there's a conflict (non-matching values),
 * this class will set the Qt widget's value to the default value and
 * change the associated label to an italicized font on destruction.
 * If there's no conflict, the value from applyValue() is shown, and
 * the label's font is fixed to non-italic.
 */
template <class T>
class MergeValues
{
public:
  explicit MergeValues(const T& defaultValue)
    : valueSet_(false),
      hasValues_(false),
      conflict_(false),
      defaultValue_(defaultValue),
      value_(defaultValue_)
  {}

  virtual ~MergeValues(){}

  /**
   * Retrieve the stored value, which is the applied value (see applyValue())
   * if no conflict, and the default value is there is a conflict.
   */
  T value() const
  {
    return value_;
  }

  /** Returns true if there's conflicting values that have been sent via applyValue(). */
  bool hasConflict() const
  {
    return conflict_;
  }

  /**
   * Apply a new value to this merge. The new value will be tested against
   * previously merged values to check for a conflict.
   * @param[in] value  The new value to apply
   * @param[in] hasValues  If false, then no-op. Useful when merging values
   *   programmatically in cases where values may not exist, such as when
   *   using simCore::Optional.
   */
  virtual void applyValue(T value, bool hasValues)
  {
    // first, if we ever get a hit, set the hasValues_ flag to true
    if (hasValues)
      hasValues_ = true;
    else // don't proceed if there are no values
      return;

    // don't bother processing anymore if we have a conflict
    if (conflict_)
      return;

    // don't test for conflict at initial value set
    if (!valueSet_)
    {
      valueSet_ = true;
      value_ = value;
    }
    else if (value_ != value)
    {
      conflict_ = true;
      value_ = defaultValue_;
      return;
    }
  }

protected:
  /**
   * Update the enabled state of the widget and handle the conflict font state. This method
   * is typically called by derived classes in their destructors.
   * @param[in] widget  pointer to the widget being updated
   * @param[in] label  pointer to the label to update. If nullptr, widget's font is updated instead.
   * @param[in] hasValues  enabled state to set on the widget and label. See description of hasValues in applyValues() docs
   * @param[in] conflict  if true, conflict font (italics) is set on the label (or widget, if label is nullptr). Clears italic font if false.
   */
  void updateEnabled_(QWidget* widget, QLabel* label, bool hasValues, bool conflict) const
  {
    if (!widget)
      return;
    if (widget->isEnabled() != hasValues)
      widget->setEnabled(hasValues);
    if (label)
    {
      label->setEnabled(hasValues);
      setConflictFont(label, conflict);
    }
    else
      setConflictFont(widget, conflict);
  }

  bool valueSet_;
  bool hasValues_;
  bool conflict_;
  T defaultValue_;
  T value_;
};

/** Merging class for bool values in a QCheckBox. Destructor updates the widget. */
class MergeBool : public MergeValues<bool>
{
public:
  explicit MergeBool(QCheckBox* widget)
    :MergeValues<bool>(false),
    widget_(widget)
  {}

  virtual ~MergeBool()
  {
    updateEnabled_(widget_, nullptr, hasValues_, conflict_);
    if (widget_->isChecked() != value_)
      widget_->setChecked(value_);
  }

private:
  QCheckBox* widget_;
};

/** Merging class for int values in a QSpinBox. Destructor updates the widget. */
class MergeSpinBox : public MergeValues<int>
{
public:
  MergeSpinBox(QSpinBox* widget, QLabel* label)
    :MergeValues<int>(1),
    widget_(widget),
    label_(label)
  {}

  virtual ~MergeSpinBox()
  {
    updateEnabled_(widget_, label_, hasValues_, conflict_);
    if (widget_->value() != value_)
      widget_->setValue(value_);
  }

private:
  QSpinBox* widget_;
  QLabel* label_;
};

/** Merging class for double values in a QDoubleSpinBox. Destructor updates the widget. */
class MergeDoubleSpinBox : public MergeValues<double>
{
public:
  MergeDoubleSpinBox(QDoubleSpinBox* widget, QLabel* label)
    :MergeValues<double>(0.0),
    widget_(widget),
    label_(label)
  {}

  virtual ~MergeDoubleSpinBox()
  {
    updateEnabled_(widget_, label_, hasValues_, conflict_);
    if (widget_->value() != value_)
      widget_->setValue(value_);
  }

private:
  QDoubleSpinBox* widget_;
  QLabel* label_;
};

/** Merge class for color values in a simQt::ColorWidget. Destructor updates the widget. */
class MergeColor : public MergeValues<osg::Vec4f>
{
public:
  explicit MergeColor(simQt::ColorWidget* widget)
    :MergeValues<osg::Vec4f>(osg::Vec4f(1.0, 1.0, 1.0, 1.0)),
  widget_(widget)
  {}

  virtual ~MergeColor()
  {
    updateEnabled_(widget_, widget_->colorLabel(), hasValues_, conflict_);
    QColor color = simQt::getQtColorFromOsg(value_);
    if (widget_->color() != color)
      widget_->setColor(color);
  }

private:
  simQt::ColorWidget* widget_;
};

/** Merging class for SimCore::TimeStamp values in a Time Widget.  Destructor updates the widget. */
class MergeTime : public MergeValues<simCore::TimeStamp>
{
public:
  /** Time merger on conflict can show one of three states: */
  enum class ConflictValue {
    /** Show whatever the default value is (typically infinite time stamp) */
    SHOW_DEFAULT,
    /** Show the minimum of collected time values */
    SHOW_MINIMUM,
    /** Show the maximum of collected time values */
    SHOW_MAXIMUM
  };

  MergeTime(simQt::TimeWidget* widget, ConflictValue showOnConflict)
    : MergeValues<simCore::TimeStamp>(simCore::INFINITE_TIME_STAMP),
      widget_(widget),
      showOnConflict_(showOnConflict)
  {}

  virtual ~MergeTime()
  {
    updateEnabled_(widget_, nullptr, hasValues_, conflict_);
    widget_->setTimeEnabled(hasValues_);

    if (!conflict_ || showOnConflict_ == ConflictValue::SHOW_DEFAULT || !hasTime_)
      widget_->setTimeStamp(value_);
    else
    {
      // Show either the minimum or maximum
      if (showOnConflict_ == ConflictValue::SHOW_MINIMUM)
        widget_->setTimeStamp(minTime_);
      else
        widget_->setTimeStamp(maxTime_);
    }

    // If there are values and there's a conflict, set the text in italics
    if (conflict_ && hasValues_)
      widget_->setStyleSheet("font-style: italic;");
    else
      widget_->setStyleSheet("");
  }

  virtual void applyValue(simCore::TimeStamp value, bool hasValues) override
  {
    MergeValues<simCore::TimeStamp>::applyValue(value, hasValues);
    // Only update the min/max if the incoming time stamp is valid
    if (!hasValues)
      return;
    // Save both minimum and maximum time values
    if (value < minTime_)
      minTime_ = value;
    if (value > maxTime_)
      maxTime_ = value;
    hasTime_ = true;
  }

private:
  simQt::TimeWidget* widget_;
  simCore::TimeStamp minTime_ = simCore::INFINITE_TIME_STAMP;
  simCore::TimeStamp maxTime_ = simCore::MIN_TIME_STAMP;
  bool hasTime_ = false;
  ConflictValue showOnConflict_;
};

/** helper struct to hold font values used in simQt::FontWidget */
struct FontValues
{
  std::string name;
  int size;
  osg::Vec4f color;

  FontValues()
    :name("arial.ttf"), size(2), color(0.0, 0.0, 0.0, 1.0)
  {}

  bool operator!=(FontValues& rhs) const
  {
    return name != rhs.name || size != rhs.size || color != rhs.color;
  }
};

/** Merging class for font values in a simQt::FontWidget */
class MergeFont : public MergeValues<FontValues>
{
public:
  explicit MergeFont(simQt::FontWidget* widget)
    :MergeValues<FontValues>(FontValues()),
     widget_(widget)
  {}

  virtual ~MergeFont()
  {
    updateEnabled_(widget_, nullptr, hasValues_, conflict_);
    if (widget_->fontFile().toStdString() != value_.name)
      widget_->setFontFile(QString::fromStdString(value_.name));
    if (widget_->fontSize() != value_.size)
      widget_->setFontSize(value_.size);

    QColor newColor = simQt::getQtColorFromOsg(value_.color);
    if (widget_->fontColor() != newColor)
      widget_->setFontColor(newColor);
  }

private:
  simQt::FontWidget* widget_;
};

}

#endif
