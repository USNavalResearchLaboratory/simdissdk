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
#include <cassert>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QSlider>
#include <QLineEdit>
#include "simCore/Calc/Math.h"
#include "simNotify/Notify.h"
#include "simQt/ColorWidget.h"
#include "simQt/FileSelectorWidget.h"
#include "simQt/BoundSettings.h"

namespace simQt {

/** Observer for the setting so we can be kept up to date from external changes */
class BoundSetting::SettingsObserver : public simQt::Settings::Observer
{
public:
  /**  Constructor  */
  explicit SettingsObserver(BoundSetting& setting) : setting_(setting) {}
  /// @copydoc simQt::Settings::Observer::onSettingChange()
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    setting_.updateValue_(value);
  }
private:
  BoundSetting& setting_;
};


BoundSetting::BoundSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : QObject(parent),
    settings_(settings),
    variableName_(variableName)
{
  settingsObserver_.reset(new SettingsObserver(*this));
}

BoundSetting::~BoundSetting()
{
  settings_.removeObserver(variableName_, settingsObserver_);
}

simQt::Settings& BoundSetting::settings()
{
  return settings_;
}

QString BoundSetting::variableName() const
{
  return variableName_;
}

void BoundSetting::setToolTip_(QWidget* widget) const
{
  Settings::MetaData metaData;
  if (settings_.metaData(variableName_, metaData) == 0)
  {
    widget->setToolTip(metaData.toolTip());
  }
}

void BoundSetting::setToolTip_(QAction* action) const
{
  Settings::MetaData metaData;
  if (settings_.metaData(variableName_, metaData) == 0)
  {
    action->setToolTip(metaData.toolTip());
  }
}

/////////////////////////////////////////////////////////////////////////

BoundBooleanSetting::BoundBooleanSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toBool();
}

BoundBooleanSetting::BoundBooleanSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toBool();
}

/** Returns the current value of the variable */
bool BoundBooleanSetting::value() const
{
  return value_;
}

void BoundBooleanSetting::bindTo(QAbstractButton* button, bool populateToolTip)
{
  if (populateToolTip)
    setToolTip_(button);
  button->setChecked(value());
  connect(button, SIGNAL(toggled(bool)), this, SLOT(setValue(bool)));
  connect(this, SIGNAL(valueChanged(bool)), button, SLOT(setChecked(bool)));
}

void BoundBooleanSetting::bindTo(QAction* action, bool populateToolTip)
{
  if (populateToolTip)
    setToolTip_(action);
  action->setCheckable(true);
  action->setChecked(value());
  connect(action, SIGNAL(toggled(bool)), this, SLOT(setValue(bool)));
  // Update the state of the action
  connect(this, SIGNAL(valueChanged(bool)), action, SLOT(setChecked(bool)));
  // Update the state of anything listening to the action
  connect(this, SIGNAL(valueChanged(bool)), action, SIGNAL(triggered(bool)));
}

/** Sets a new value for the variable */
void BoundBooleanSetting::setValue(bool v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(v == value_);
}

void BoundBooleanSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toBool() != value_)
  {
    value_ = newValue.toBool();
    Q_EMIT(valueChanged(value_));
  }
}

/////////////////////////////////////////////////////////////////////////

BoundIntegerSetting::BoundIntegerSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toInt();
}

BoundIntegerSetting::BoundIntegerSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toInt();
}

/** Returns the current value of the variable */
int BoundIntegerSetting::value() const
{
  return value_;
}

/** Sets a new value for the variable */
void BoundIntegerSetting::setValue(int v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(v == value_);
}

void BoundIntegerSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toInt() != value_)
  {
    value_ = newValue.toInt();
    Q_EMIT(valueChanged(value_));
  }
}

void BoundIntegerSetting::bindTo(QComboBox* comboBox, bool populateToolTip, bool populateItems)
{
  if (populateToolTip || populateItems)
  {
    Settings::MetaData metaData;
    if (settings_.metaData(variableName_, metaData) == 0)
    {
      if (populateToolTip)
        comboBox->setToolTip(metaData.toolTip());
      // Very limited use cases for this.  Must be enumeration and have strictly increasing valid values
      if (populateItems)
        populateCombo_(metaData, comboBox);
    }
  }
  comboBox->setCurrentIndex(value());
  connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setValue(int)));
  connect(this, SIGNAL(valueChanged(int)), comboBox, SLOT(setCurrentIndex(int)));
}

void BoundIntegerSetting::bindTo(QSpinBox* spinBox, bool populateToolTip, bool populateLimits)
{
  if (populateToolTip || populateLimits)
  {
    Settings::MetaData metaData;
    if (settings_.metaData(variableName_, metaData) == 0)
    {
      if (populateToolTip)
        spinBox->setToolTip(metaData.toolTip());
      if (populateLimits)
      {
        if (metaData.minValue().isValid())
          spinBox->setMinimum(metaData.minValue().toInt());
        if (metaData.maxValue().isValid())
          spinBox->setMaximum(metaData.maxValue().toInt());
      }
    }
  }
  spinBox->setValue(value());
  connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
  connect(this, SIGNAL(valueChanged(int)), spinBox, SLOT(setValue(int)));
}

void BoundIntegerSetting::bindTo(QSlider* slider, bool populateToolTip, bool populateLimits)
{
  if (populateToolTip || populateLimits)
  {
    Settings::MetaData metaData;
    if (settings_.metaData(variableName_, metaData) == 0)
    {
      if (populateToolTip)
        slider->setToolTip(metaData.toolTip());
      if (populateLimits)
      {
        // Slider value, by nature, is between two points; meta data should have reasonable limits.
        assert(metaData.minValue().isValid() && metaData.maxValue().isValid());

        if (metaData.minValue().isValid())
          slider->setMinimum(metaData.minValue().toInt());
        if (metaData.maxValue().isValid())
          slider->setMaximum(metaData.maxValue().toInt());
      }
    }
  }
  slider->setValue(value());
  connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
  connect(this, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
}

int BoundIntegerSetting::populateCombo_(const Settings::MetaData& metaData, QComboBox* comboBox) const
{
  // Must be an enumeration with valid values
  if (metaData.type() != Settings::ENUMERATION)
  {
    SIM_ERROR << "Unable to populate combo box for " << variableName_.toStdString()
      << ", not an enumeration setting." << std::endl;
    return 1;
  }

  // Must be non-empty
  QMap<int, QString> enumValues = metaData.enumValues();
  if (enumValues.empty())
  {
    SIM_ERROR << "Unable to populate combo box for " << variableName_.toStdString()
      << ", no valid enumeration values." << std::endl;
    return 1;
  }

  // Must start with 0 and end with numItems-1; implies integral order
  int firstItem = enumValues.begin().key();
  int lastItem = (--enumValues.end()).key();
  if (firstItem != 0 || lastItem != enumValues.size() - 1)
  {
    SIM_ERROR << "Unable to populate combo box for " << variableName_.toStdString()
      << ", enumeration values not strictly increasing from 0." << std::endl;
    return 1;
  }

  // At this point we can clear out the values and add our own
  comboBox->clear();
  auto values = enumValues.values();
  for (auto it = values.begin(); it != values.end(); ++it)
    comboBox->addItem(*it);
  return 0;
}

/////////////////////////////////////////////////////////////////////////

BoundDoubleSetting::BoundDoubleSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toDouble();
}

BoundDoubleSetting::BoundDoubleSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toDouble();
}

/** Returns the current value of the variable */
double BoundDoubleSetting::value() const
{
  return value_;
}

void BoundDoubleSetting::bindTo(QDoubleSpinBox* box, bool populateToolTip, bool populateLimits)
{
  if (populateToolTip || populateLimits)
  {
    Settings::MetaData metaData;
    if (settings_.metaData(variableName_, metaData) == 0)
    {
      if (populateToolTip)
        box->setToolTip(metaData.toolTip());
      if (populateLimits)
      {
        double minVal = -std::numeric_limits<double>::max();
        double maxVal = std::numeric_limits<double>::max();
        if (metaData.minValue().isValid())
        {
          minVal = metaData.minValue().toDouble();
          box->setMinimum(minVal);
        }
        if (metaData.maxValue().isValid())
        {
          maxVal = metaData.maxValue().toDouble();
          box->setMaximum(maxVal);
        }
        box->setDecimals(metaData.numDecimals());

        // If minVal and maxVal both have a valid value and aren't equal, set a reasonable step based on range and precision
        if (minVal != -std::numeric_limits<double>::max() && maxVal != std::numeric_limits<double>::max())
          box->setSingleStep(simCore::guessStepSize(maxVal - minVal, metaData.numDecimals()));
      }
    }
  }
  box->setValue(value_);
  connect(this, SIGNAL(valueChanged(double)), box, SLOT(setValue(double)));
  connect(box, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
}

/** Sets a new value for the variable */
void BoundDoubleSetting::setValue(double v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(simCore::areEqual(v, value_));
}

void BoundDoubleSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toDouble() != value_)
  {
    value_ = newValue.toDouble();
    Q_EMIT(valueChanged(value_));
  }
}

/////////////////////////////////////////////////////////////////////////

BoundColorSetting::BoundColorSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = toColor_(settings_.value(variableName_, metaData, settingsObserver_));
}

BoundColorSetting::BoundColorSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = toColor_(settings_.value(variableName_, settingsObserver_));
}

/** Returns the current value of the variable */
QColor BoundColorSetting::value() const
{
  return value_;
}

void BoundColorSetting::bindTo(simQt::ColorWidget* colorWidget, bool populateToolTip)
{
  if (populateToolTip)
  {
    Settings::MetaData metaData;
    if (settings_.metaData(variableName_, metaData) == 0)
      colorWidget->setToolTip(metaData.toolTip());
  }
  colorWidget->setColor(value_);
  connect(this, SIGNAL(valueChanged(const QColor&)), colorWidget, SLOT(setColor(const QColor&)));
  connect(colorWidget, SIGNAL(colorChanged(const QColor&)), this, SLOT(setValue(const QColor&)));
}

/** Sets a new value for the variable */
void BoundColorSetting::setValue(const QColor& v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, toVariant_(v));
  }
  // Assertion failure means observer is not correctly firing, or QVariant conversion wrong
  assert(v == value_);
}

void BoundColorSetting::updateValue_(const QVariant& newValue)
{
  QColor newValueColor = toColor_(newValue);
  if (newValueColor != value_)
  {
    value_ = newValueColor;
    Q_EMIT(valueChanged(value_));
  }
}

QColor BoundColorSetting::toColor_(const QVariant& value) const
{
  if (value.isValid() && value.canConvert<QRgb>())
    return QColor::fromRgba(value.value<QRgb>());
  return QColor(0, 0, 0, 0);
}

QVariant BoundColorSetting::toVariant_(const QColor& color) const
{
  return color.rgba();
}

/////////////////////////////////////////////////////////////////////////

BoundStringSetting::BoundStringSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toString();
}

BoundStringSetting::BoundStringSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toString();
}

/** Returns the current value of the variable */
QString BoundStringSetting::value() const
{
  return value_;
}

void BoundStringSetting::bindTo(QLineEdit* lineEdit, bool populateToolTip)
{
  if (populateToolTip)
    setToolTip_(lineEdit);
  lineEdit->setText(value());
  connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setValue(const QString&)));
  connect(this, SIGNAL(valueChanged(const QString&)), lineEdit, SLOT(setText(const QString&)));
}

void BoundStringSetting::bindTo(FileSelectorWidget* fileSelector, bool populateToolTip)
{
  if (populateToolTip)
    setToolTip_(fileSelector);
  fileSelector->setFilename(value());
  connect(fileSelector, SIGNAL(filenameChanged(QString)), this, SLOT(setValue(QString)));
  connect(this, SIGNAL(valueChanged(QString)), fileSelector, SLOT(setFilename(QString)));
}

/** Sets a new value for the variable */
void BoundStringSetting::setValue(const QString& v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(v == value_);
}

void BoundStringSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toString() != value_)
  {
    value_ = newValue.toString();
    Q_EMIT(valueChanged(value_));
  }
}

/////////////////////////////////////////////////////////////////////////

BoundStringListSetting::BoundStringListSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toStringList();
}

BoundStringListSetting::BoundStringListSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toStringList();
}

/** Returns the current value of the variable */
QStringList BoundStringListSetting::value() const
{
  return value_;
}

/** Sets a new value for the variable */
void BoundStringListSetting::setValue(const QStringList& v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(v == value_);
}

void BoundStringListSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toStringList() != value_)
  {
    value_ = newValue.toStringList();
    Q_EMIT(valueChanged(value_));
  }
}

/////////////////////////////////////////////////////////////////////////

BoundVariantMapSetting::BoundVariantMapSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, metaData, settingsObserver_).toMap();
}

BoundVariantMapSetting::BoundVariantMapSetting(QObject* parent, simQt::Settings& settings, const QString& variableName)
  : BoundSetting(parent, settings, variableName)
{
  value_ = settings_.value(variableName_, settingsObserver_).toMap();
}

/** Returns the current value of the variable */
QMap<QString, QVariant> BoundVariantMapSetting::value() const
{
  return value_;
}

void BoundVariantMapSetting::setValue(const QMap<QString, QVariant>& v)
{
  if (v != value_)
  {
    settings_.setValue(variableName_, v);
  }
  // Assertion failure means observer is not correctly firing
  assert(v == value_);
}

void BoundVariantMapSetting::mergeValues(const QMap<QString, QVariant>& v)
{
  QMap<QString, QVariant> newValues = value_;
  for (QMap<QString, QVariant>::const_iterator vIter = v.begin(); vIter != v.end(); ++vIter)
    newValues[vIter.key()] = vIter.value();

  // Setting the Settings value will trigger updateValue_, which will detect changes and emit
  settings_.setValue(variableName_, newValues);
}

void BoundVariantMapSetting::updateValue_(const QVariant& newValue)
{
  if (newValue.toMap() != value_)
  {
    value_ = newValue.toMap();
    Q_EMIT(valueChanged(value_));
  }
}

}
