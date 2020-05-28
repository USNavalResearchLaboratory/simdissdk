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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_BOUNDSETTINGS_H
#define SIMQT_BOUNDSETTINGS_H

#include <QObject>
#include <QColor>
#include <QStringList>
#include "simCore/Common/Export.h"
#include "simQt/Settings.h"

class QComboBox;
class QDoubleSpinBox;
class QAbstractButton;
class QSpinBox;
class QSlider;
class QLineEdit;
class QAction;

namespace simQt {

class ColorWidget;
class FileSelectorWidget;

/**
 * Abstract base class for a number of bound settings, based on variable type.  The
 * purpose of these classes is to provide a binding from a simQt::Settings value (such
 * as "Foo/Bar") to a concrete value.  The bound concrete value (a boolean, string,
 * integer, double, etc.) is kept in sync with the simQt::Settings using Observers.
 * When settings change, the concrete variable (which serves as a cache) changes, and
 * vice versa.
 *
 * Additionally, the BoundSetting instances are designed to emit Qt signals.  This means
 * you can easily define a setting in your GUI and bind it to a control widget:
 *
 * // Bind the Console/ShowOnTop setting to an instanced variable
 * BoundBooleanSetting* check = new BoundBooleanSetting(this, settings_, "Console/ShowOnTop", ...);
 * // ui_->showOnTopCheck is a QCheckBox
 * connect(check, SIGNAL(valueChanged(bool)), ui_->showOnTopCheck, SLOT(setChecked(bool)));
 * connect(ui_->showOnTopCheck, SIGNAL(toggled(bool)), check, SLOT(setValue(bool));
 *
 * ... and now your local "check" instance, your QCheckBox, and the Settings GUI are
 * all kept in perfect sync.  Changing any one, will update all the others.
 */
class SDKQT_EXPORT BoundSetting : public QObject
{
  Q_OBJECT;
public:

  /**
   * Constructor.
   * @param parent What is bound to
   * @param settings Container for all the settings
   * @param variableName The name of the setting
   */
  BoundSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);
  virtual ~BoundSetting();

  /** Returns Settings instance */
  simQt::Settings& settings();
  /** Returns BoundSetting variable name */
  QString variableName() const;

protected:
  /** Changes the cached value based on the SettingsObserver event, emitting valueChanged() */
  virtual void updateValue_(const QVariant& newValue) = 0;
  /** Sets the tool tip of a widget based on the simQt::Settings's MetaData */
  void setToolTip_(QWidget* widget) const;
  /** Sets the tool tip of an action based on the simQt::Settings's MetaData */
  void setToolTip_(QAction* action) const;

  /** Observer for the setting so we can be kept up to date from external changes */
  class SettingsObserver;

  /** Settings pointer, for removing the observer later */
  simQt::Settings& settings_;
  /** Retrieves the name of the settings variable */
  QString variableName_;
  /** Observer tied to the variable in settings */
  simQt::Settings::ObserverPtr settingsObserver_;
};


/** Boolean setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundBooleanSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound boolean setting with metadata */
  BoundBooleanSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound boolean setting without metadata */
  BoundBooleanSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  bool value() const;

  /**
   * Helper routine to bind a QAbstractButton to the BoundBooleanSetting; works for radio, tool, push, and check buttons
   * @param button Abstract button to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   */
  void bindTo(QAbstractButton* button, bool populateToolTip=true);

  /**
   * Helper routine to bind a QAction's check state to the BoundBooleanSetting.
   * @param action Action to bind to
   * @param populateToolTip If true, replace the existing tool tip on the action with the value from Settings MetaData
   */
  void bindTo(QAction* action, bool populateToolTip=true);

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(bool v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(bool v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Cache of the settings value */
  bool value_;
};

/** Integer setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundIntegerSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound integer setting with metadata */
  BoundIntegerSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound integer setting without metadata */
  BoundIntegerSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  int value() const;

  /**
   * Helper routine to bind a QComboBox to a BoundIntegerSetting, optionally replacing its
   * items with the enumerated values listed in simQt::Settings (only if ENUMERATION value).
   *
   * This method binds the integer value to the index of the combo box, and therefore is only
   * suitable for use in cases where the combo box items are strictly increasing from 0 with
   * no gaps between values.
   * @param comboBox Combo box to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   * @param populateItems If true, replace the existing values in the combo box with items from the Settings MetaData.
   *   Note that this feature is limited only to ENUMERATION metadata with strictly increasing values from 0 to numItems.
   */
  void bindTo(QComboBox* comboBox, bool populateToolTip=true, bool populateItems=true);

  /**
   * Helper routine to bind a QSpinBox to a BoundIntegerSetting
   * @param spinBox Spin box to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   * @param populateLimits If true, replace minimum and maximum values on spin box with values from Settings MetaData
   */
  void bindTo(QSpinBox* spinBox, bool populateToolTip=true, bool populateLimits=true);

  /**
   * Helper routine to bind a QSlider to a BoundIntegerSetting
   * @param slider QSlider to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   * @param populateLimits If true, replace minimum and maximum values on spin box with values from Settings MetaData
   */
  void bindTo(QSlider* slider, bool populateToolTip=true, bool populateLimits=true);

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(int v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(int v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Populates a combo box with enumerated values */
  int populateCombo_(const Settings::MetaData& metaData, QComboBox* comboBox) const;
  /** Cache of the settings value */
  int value_;
};

/** Double setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundDoubleSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound double setting with metadata */
  BoundDoubleSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound double setting without metadata */
  BoundDoubleSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  double value() const;

  /**
   * Helper routine to bind a QDoubleSpinBox to the BoundDoubleSetting
   * @param box Spin box to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   * @param populateLimits If true, replace minimum and maximum values, and decimals, on spin box with values from Settings MetaData
   */
  void bindTo(QDoubleSpinBox* box, bool populateToolTip=true, bool populateLimits=true);

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(double v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(double v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Cache of the settings value */
  double value_;
};

/** Color setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundColorSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound color setting with metadata */
  BoundColorSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound color setting without metadata */
  BoundColorSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  QColor value() const;

  /**
   * Helper routine to bind a simQt::ColorWidget to the BoundColorSetting
   * @param colorWidget Color widget to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   */
  void bindTo(simQt::ColorWidget* colorWidget, bool populateToolTip=true);

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(const QColor& v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(const QColor& v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Convenience method to convert QVariant to QColor (there is no toColor()) */
  QColor toColor_(const QVariant& value) const;
  /** Convenience method to convert from QColor to QVariant */
  QVariant toVariant_(const QColor& color) const;
  /** Cache of the settings value */
  QColor value_;
};

/** String setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundStringSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound string setting with metadata */
  BoundStringSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound string setting without metadata */
  BoundStringSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  QString value() const;

  /**
   * Helper routine to bind a QTextEdit to the BoundStringSetting.
   * @param lineEdit Single line edit (QLineEdit) to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   */
  void bindTo(QLineEdit* lineEdit, bool populateToolTip=true);

  /**
   * Helper routine to bind a FileSelectorWidget to the BoundStringSetting.
   * @param fileSelector FileSelectorWidget to bind to
   * @param populateToolTip If true, replace the existing tool tip on the widget with the value from Settings MetaData
   */
  void bindTo(FileSelectorWidget* fileSelector, bool populateToolTip=true);

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(const QString& v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(const QString& v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Cache of the settings value */
  QString value_;
};

/** String-List setting that updates automatically from simQt::Settings and has slots/signals */
class SDKQT_EXPORT BoundStringListSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound string list setting with metadata */
  BoundStringListSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound string list setting without metadata */
  BoundStringListSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  QStringList value() const;

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(const QStringList& v);

signals:
  /** Emitted when the settings value changes */
  void valueChanged(const QStringList& v);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Cache of the settings value */
  QStringList value_;
};

/** Bound Setting implementation for QMap<QString, QVariant>, using QVariant::toMap(). */
class SDKQT_EXPORT BoundVariantMapSetting : public BoundSetting
{
  Q_OBJECT;
public:
  /** Instantiates a new bound variant map setting with metadata */
  BoundVariantMapSetting(QObject* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  /** Instantiates a new bound variant map setting without metadata */
  BoundVariantMapSetting(QObject* parent, simQt::Settings& settings, const QString& variableName);

  /** Current data value */
  QMap<QString, QVariant> value() const;

public slots:
  /** Change the data value in Settings (and the cache); might emit valueChanged() */
  void setValue(const QMap<QString, QVariant>& v);
  /** Add entries in 'v' to current value.  Does not remove values. */
  void mergeValues(const QMap<QString, QVariant>& v);

signals:
  /** Emitted when the settings value changes. */
  void valueChanged(const QMap<QString, QVariant>& value);

private:
  /** Called by observer to update the value */
  virtual void updateValue_(const QVariant& newValue);
  /** Cache of the settings value */
  QMap<QString, QVariant> value_;
};

}

#endif /* SIMQT_BOUNDSETTINGS_H */
