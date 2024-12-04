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
#ifndef SIMQT_SETTINGS_H
#define SIMQT_SETTINGS_H

#include <memory>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "simCore/Common/Common.h"

namespace simQt {

const QString WINDOWS_SETTINGS="WindowsSettings/";

/// A wrapper to QSetting to provide meta data and callbacks to drive a user interface
class SDKQT_EXPORT Settings
{
public:

  /// Type information to drive what controls to use on a user interface
  enum DataTypes { INTEGER, DOUBLE, BOOLEAN, STRING, FILENAME, COLOR, POINT, SIZE, ENUMERATION, STRING_LIST, DIRECTORY, FONT, QFONT, HEX, VARIANT_MAP, LAYOUT, OTHER_DATATYPE };
  /// Level information to help filter what fields to display on the user interface
  enum DataLevel { DEFAULT, ADVANCED, PRIVATE, UNKNOWN};

  /// Meta data describes a setting and is used when building the user interface for settings
  class SDKQT_EXPORT MetaData
  {
  public:
    /** Equality operator */
    bool operator==(const MetaData& rhs) const;
    /** Returns the type */
    DataTypes type() const;
    /** Returns the default value */
    QVariant defaultValue() const;
    /** Returns the tool tip */
    QString toolTip() const;
    /** Returns the level */
    DataLevel level() const;
    /** Returns the minimum value */
    QVariant minValue() const;
    /** Returns the maximum value*/
    QVariant maxValue() const;
    /** Returns the number of decimals */
    int numDecimals() const;
    /** Returns the file name filter */
    QVariant filenameFilter() const;
    /** Returns the string value for each enumeration */
    QMap<int, QString> enumValues() const;
    /** Converts the setting value to the internal format */
    QVariant convertToInteralFormat(QVariant input) const;
    /** Converts the setting value to the save format */
    QVariant convertToSaveFormat(QVariant saveValue) const;

    /** Adds an additional key/value pair to the enumValues */
    MetaData& addEnumeration(int key, const QString& value);

    /** Factory method for meta data representing an integer value */
    static MetaData makeInteger(const QVariant& defaultValue, const QString& tooltip="",
      DataLevel inLevel=DEFAULT, const QVariant& minValue=QVariant(),
      const QVariant& maxValue=QVariant());
    /** Factory method for meta data representing a double value */
    static MetaData makeDouble(const QVariant& defaultValue, const QString& tooltip="",
      DataLevel inLevel=DEFAULT, const QVariant& minValue=QVariant(),
      const QVariant& maxValue=QVariant(), int numDecimals=2);
    /** Factory method for meta data representing a boolean value */
    static MetaData makeBoolean(const QVariant& defaultValue, const QString& tooltip="",
      DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a string value */
    static MetaData makeString(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /**
     * Factory method for meta data representing a filename value.  Filter follows rules of
     * formatting described by simQt::FileDialog::loadFile() and simQt::FileSelectorWidget.
     */
    static MetaData makeFilename(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT,
      const QVariant filenameFilter=QVariant());
    /** Factory method for meta data representing a directory path. */
    static MetaData makeDirectory(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a color value.  Use QRgb. */
    static MetaData makeColor(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a point value */
    static MetaData makePoint(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a size value */
    static MetaData makeSize(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing an enumeration value */
    static MetaData makeEnumeration(const QVariant& defaultValue, const QMap<int, QString>& enumValues,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing an enumeration value, with no initial values (use addEnumeration()) */
    static MetaData makeEnumeration(const QVariant& defaultValue, const QString& tooltip="",
      DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a font - filename only (not size). */
    static MetaData makeFont(const QVariant& defaultValue,
      const QString& tooltip="", DataLevel inLevel=DEFAULT);
    /** Factory method for meta data representing a QFont. */
    static MetaData makeQFont(const QVariant& defaultValue,
      const QString& tooltip = "", DataLevel inLevel = DEFAULT);
    /** Factory method for meta data representing a hex value */
    static MetaData makeHex(const QVariant& defaultValue, const QString& tooltip="",
      DataLevel inLevel=DEFAULT, const QVariant& minValue=0, const QVariant& maxValue=QVariant());

    /** Public multifaceted default constructor; NOTE: Used by QVariant */
    MetaData(DataTypes inType=STRING, const QVariant& inDefaultValue=QVariant(),
             const QString& inToolTip="", DataLevel inLevel=DEFAULT,
             const QVariant& inMinValue=QVariant(), const QVariant& inMaxValue=QVariant(),
             int inNumDecimals=2, const QVariant filenameFilter=QVariant());

    /** Serializes to an output stream */
    void serialize(QDataStream& out) const;
    /** Deserializes from an input stream */
    void deserialize(QDataStream& in);

  private:
    DataTypes type_;          ///< What type of data
    QVariant defaultValue_;   ///< Initial value
    QString toolTip_;         ///< Tool tip to display in the user interface
    DataLevel level_;         ///< A field to provide filter in the user interface
    QVariant minValue_;       ///< Minimum value, if applicable
    QVariant maxValue_;       ///< Maximum value, if applicable
    int numDecimals_;         ///< Number of decimal places for DOUBLE data
    QVariant filenameFilter_; ///< Filter for filename pop up GUI; if empty, then "All Files (*)" is used
    QMap<int, QString> enumValues_; ///< Map from integer to string for enumeration values
  };

  /// A callback class when entry value is changed
  class Observer
  {
  public:
    virtual ~Observer() {}
    /** Settings value has changed.  Includes the fully qualified name, and the new value. */
    virtual void onSettingChange(const QString& name, const QVariant& value) = 0;
  };

  /// Shared pointer for Observer
  typedef std::shared_ptr<Observer> ObserverPtr;

  virtual ~Settings() {}

  /// Removes all the entries including metadata and callbacks
  virtual void clear() = 0;
  /// Reset factory default value for all settings
  virtual void resetDefaults() = 0;
  /// Reset factory default values for settings of a single group and its sub-groups
  virtual void resetDefaults(const QString& name) = 0;

  /// Set value; will create if does not exist
  virtual void setValue(const QString& name, const QVariant& value) = 0;
  /// Set value; will create if does not exist; updates the metaData and observer, will not add a duplicate observer
  virtual void setValue(const QString& name, const QVariant& value, const MetaData& metaData, ObserverPtr observer=ObserverPtr()) = 0;
  /// Set value; will create if does not exist; updates the metaData; will not call the specified observer to prevent a feedback loop
  virtual void setValue(const QString& name, const QVariant& value, ObserverPtr skipThisObserver) = 0;
  /// Returns value for specified name; will return QVariant::Invalid if name does not exist
  virtual QVariant value(const QString& name) const = 0;
  /// Returns value for specified name; will create if it does not exist and return default value in MetaData
  virtual QVariant value(const QString& name, const MetaData& metaData, ObserverPtr observer=ObserverPtr()) = 0;
  /// Returns value for specified name; will create if it does not exist and use default value in MetaData
  virtual QVariant value(const QString& name, ObserverPtr observer) = 0;
  /// Returns true if the name exists
  virtual bool contains(const QString& name) const = 0;

  /*********************************************************************************************
   * The routines saveWidget and loadWidget are helper routines to save off routine window layout
   * information.   Information includes:
   *
   * QDialog: Window location and size
   * QSplitter: Splitter location
   * QTreeView: Column widths
   * QTableView: Column widths
   *
   * The routines start at the given object and recursively search children for the supported
   * widgets.  The names of the objects are used to make the Setting names.  The routine will
   * ignore objects without a name.    In a Debug build objects without names will generate a
   * SIM_ERROR message.   Call the load routine after the widget has been created and call the save
   * routine before the widget is destroyed.  The routines will not catch any dynamic behavior such
   * as adding and removing QTreeView.
   *
   *********************************************************************************************/

  /// Saves widget info; implementations may want to use simQt::WidgetSettings helper functions
  virtual void saveWidget(QWidget* widget) = 0;
  /// Loads widget info; implementations may want to use simQt::WidgetSettings helper functions
  virtual void loadWidget(QWidget* widget) = 0;

  /// Returns all names
  virtual QStringList allNames() const = 0;

  /// Set Metadata
  virtual int setMetaData(const QString& name, const MetaData& metaData) = 0;
  /// Get Metadata
  virtual int metaData(const QString& name, MetaData& metaData) const = 0;

  /// Add local observers; only when this entry changes do a callback
  virtual int addObserver(const QString& name, ObserverPtr observer) = 0;
  /// Remove local observers
  virtual int removeObserver(const QString& name, ObserverPtr observer) = 0;

  /// Add global observers; if any entry changes do a callback
  virtual void addObserver(ObserverPtr observer) = 0;
  /// Remove global observers
  virtual int removeObserver(ObserverPtr observer) = 0;

  /// Retrieves the path to the underlying storage; on Windows if using NativeFormat, this could be a registry path and not a file path
  virtual QString fileName() const = 0;

  /** Narrow interface to a memento that stores Settings state */
  class Memento
  {
  public:
    virtual ~Memento() {}

    /** Restores the state of the memento into the provided settings object; returns 0 on success */
    virtual int restore(Settings& settings) const = 0;
  };
  /// Generate a memento with the current state of settings.  Caller is responsible for deleting.
  virtual Memento* createMemento() const = 0;
};

typedef std::shared_ptr<Settings> SettingsPtr;
}

Q_DECLARE_METATYPE(simQt::Settings::MetaData);
SDKQT_EXPORT QDataStream& operator<<(QDataStream& out, const simQt::Settings::MetaData& metaData);
SDKQT_EXPORT QDataStream& operator>>(QDataStream& in, simQt::Settings::MetaData& metaData);

#endif
