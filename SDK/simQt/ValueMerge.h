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

#ifndef SIMQT_VALUEMERGE_H
#define SIMQT_VALUEMERGE_H

#include <QColor>
#include "simCore/Common/Export.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace simQt {

class ColorWidget;
class FileSelectorWidget;

/**
 * ValueMerge and its derived classes provide the ability to merge
 * values from multiple sources of the same type into a Qt widget.
 * Using merge(), a value from all sources will be merged using a
 * call to the provided std::function. See merge() documentation
 * for more information.
 *
 * ValueMerge is a base class and should not be used directly.
 * Example usage of a derived class is below.
 *
 * <code>
 * // Simple Example:
 * std:vector<int> sample = { ... };
 * {
 *   // Changes to the spin box will not apply until the ValueMerge
 *   // instance goes out of scope and the destructor is called.
 *   simQt::SpinBoxValueMerge sampleMerge(ui_->sampleSpin, ui_->sampleSpinLabel, 0);
 *   sampleMerge.merge(sample, [](int value) -> int { return value; });
 * }
 *
 * // Class Example:
 * std::vector<Entry> entries = { ... }
 * {
 *   // Changes to the spin box will not apply until the ValueMerge
 *   // instance goes out of scope and the destructor is called.
 *   simQt::SpinBoxValueMerge entryMerge(ui_->entryWidthSpin, ui_->entryWidthLabel, 1);
 *   entryMerge.merge(entries, [](const Entry& entry) -> int { return entry.width(); });
 * }
 * </code>
 */
template<typename ValueTypeT>
class SDKQT_EXPORT ValueMerge
{
public:
  virtual ~ValueMerge() {}

  /**
   * Checks values from all entries in the provided container
   * to determine if all values match or if a conflict exists.
   * @param entries  Container containing entries to scan for values.
   *   The container should be any type that can be iterated using a
   *   range-based for loop.
   * @param func  std::function used to access a value from each entry
   *   in the entries container. The function should accept a single
   *   parameter of the type in the entries container, and return a
   *   value matching the type described by ValueTypeT. For example:
   *   for a container of instances of a class called Entry where
   *   ValueTypeT represents an integer, a useful function could be:
   *   [](const Entry& entry) -> int {
   *     return entry.getIntValue();
   *     }
   */
  template<typename ContainerT, typename FunctionT>
  void merge(const ContainerT& entries, const FunctionT& func)
  {
    for (auto entry : entries)
    {
      ValueTypeT thisVal = func(entry);
      if (!valueSet_)
      {
        valueSet_ = true;
        value_ = thisVal;
        continue;
      }

      if (thisVal != value_)
      {
        conflict_ = true;
        value_ = defaultValue_;
        break; // Break out early on conflict
      }
    }
  }

protected:
  /** Restrict constructor usage to derived classes only */
  explicit ValueMerge(const ValueTypeT& defaultValue)
    : defaultValue_(defaultValue),
    valueSet_(false),
    conflict_(false)
  {
  }

  ValueTypeT defaultValue_; ///< Default value, used when there are conflicts
  ValueTypeT value_; ///< Value found by merge(). Ignored if there are conflicts
  bool valueSet_; ///< True if value_ has been set
  bool conflict_; ///< True if values differ among provided entries
};

//////////////////////////////////////////////////////////////

/** QSpinBox ValueMerge implementation, integer version. */
class SDKQT_EXPORT SpinBoxValueMerge : public ValueMerge<int>
{
public:
  /**
   * QSpinBox ValueMerge implementation, integer version.
   * @param spinBox  QSpinBox to update with a merged value
   * @param label  QLabel associated with the spin box, made italic when there's a conflict
   * @param defaultValue  Default value used when there's a conflict
   */
  SpinBoxValueMerge(QSpinBox* spinBox, QLabel* label, int defaultValue);
  /** Destructor. Applies merged value and font changes to the spin box and label as needed. */
  virtual ~SpinBoxValueMerge();

private:
  QSpinBox* spinBox_;
  QLabel* label_;
};

//////////////////////////////////////////////////////////////

/** QSpinBox ValueMerge implementation, unsigned integer version. */
class SDKQT_EXPORT SpinBoxUValueMerge : public ValueMerge<unsigned int>
{
public:
  /**
   * Construct a new SpinBoxUValueMerge.
   * @param spinBox  QSpinBox to update with a merged value
   * @param label  QLabel associated with the spin box, made italic when there's a conflict
   * @param defaultValue  Default value used when there's a conflict
   */
  SpinBoxUValueMerge(QSpinBox* spinBox, QLabel* label, unsigned int defaultValue);
  /** Destructor. Applies merged value and font changes to the spin box and label as needed. */
  virtual ~SpinBoxUValueMerge();

private:
  QSpinBox* spinBox_;
  QLabel* label_;
};

//////////////////////////////////////////////////////////////

/** QCheckBox ValueMerge implementation. Does not handle tristate check boxes.*/
class SDKQT_EXPORT CheckBoxValueMerge : public ValueMerge<bool>
{
public:
  /**
   * Construct a new CheckBoxValueMerge.
   * @param checkbox  QCheckBox to update with a merged value
   * @param label  QLabel associated with the check box, made italic when there's a conflict.
   *  If label is nullptr, italics will be applied to the QCheckBox's text instead.
   * @param defaultValue  Default value used when there's a conflict
   */
  CheckBoxValueMerge(QCheckBox* checkBox, QLabel* label, bool defaultValue);
  /** Destructor. Applies merged value and font changes to the check box and label as needed. */
  virtual ~CheckBoxValueMerge();

private:
  QCheckBox* checkBox_;
  QLabel* label_;
};

//////////////////////////////////////////////////////////////

/** QLineEdit ValueMerge implementation. */
class SDKQT_EXPORT LineEditValueMerge : public ValueMerge<QString>
{
public:
  /**
   * Construct a new LineEditValueMerge. This implementation purposely does not
   * accept a default value. When a conflict arises, QLineEdit will be cleared.
   * @param lineEdit  QLineEdit to update with a merged value
   * @param label  QLabel associated with the line edit, made italic when there's a conflict
   */
  LineEditValueMerge(QLineEdit* lineEdit, QLabel* label);
  /** Destructor. Applies merged value and font changes to the line edit and label as needed. */
  virtual ~LineEditValueMerge();

private:
  QLineEdit* lineEdit_;
  QLabel* label_;
};

//////////////////////////////////////////////////////////////

/** simQt::FileSelectorWidget ValueMerge implementation. */
class SDKQT_EXPORT FileSelectorValueMerge : public ValueMerge<QString>
{
public:
  /**
   * Construct a new FileSelectorValueMerge. This implementation purposely does not
   * accept a default value. When a conflict arises, simQt::FileSelectorWidget will be cleared.
   * @param fileSelector  simQt::FileSelectorWidget to update with a merged value
   * @param label  QLabel associated with the spin box, made italic when there's a conflict
   */
  FileSelectorValueMerge(simQt::FileSelectorWidget* fileSelector, QLabel* label);
  /** Destructor. Applies merged value and font changes to the file selector and label as needed. */
  virtual ~FileSelectorValueMerge();

private:
  simQt::FileSelectorWidget* fileSelector_;
  QLabel* label_;
};

//////////////////////////////////////////////////////////////

/** simQt::ColorWidget ValueMerge implementation, integer version. */
class SDKQT_EXPORT ColorValueMerge : public ValueMerge<QColor>
{
public:
  /**
   * Construct a new simQt::ColorWidget.
   * @param colorWidget  simQt::ColorWidget to update with a merged value
   * @param label  QLabel associated with the color widget, made italic when there's a conflict
   * @param defaultValue  Default value used when there's a conflict
   */
  ColorValueMerge(simQt::ColorWidget* colorWidget, QLabel* label, const QColor& defaultValue);
  /** Destructor. Applies merged value and font changes to the color widget and label as needed. */
  virtual ~ColorValueMerge();

private:
  simQt::ColorWidget* colorWidget_;
  QLabel* label_;
};

}

#endif
