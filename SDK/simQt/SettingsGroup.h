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
#ifndef SIMQT_SETTINGS_GROUP_H
#define SIMQT_SETTINGS_GROUP_H

#include "Settings.h"

namespace simQt {

 /**
  * A wrapper to Settings to provide a group concept to Settings.
  * The group concept applies to all routines except clear and resetDefaults.
  * Support for clear and resetDefault will be added when necessary.
  * The group concept simplifies the layering of names and allows one
  * callback for all the entries in the group.
  **/
class SDKQT_EXPORT SettingsGroup : public Settings
{
public:

 /**
  * Constructor
  * @param settings The Settings to wrap and provide group level support
  * @param path The path of the group; all values added will be below this path
  */
  SettingsGroup(Settings* settings, const QString& path);
  virtual ~SettingsGroup();

  /// Removes all the entries including metadata and callbacks; current NOT limited by the group concept
  virtual void clear() override;
  /// Reset factory default value for all settings; currently NOT limited by the group concept
  virtual void resetDefaults() override;
  /// Reset factory default values for settings of a single group and its sub-groups
  virtual void resetDefaults(const QString& name) override;

  /// Set value; will create if does not exist
  virtual void setValue(const QString& name, const QVariant& value) override;
  /// Set value; will create if does not exist; updates the metaData and observer, will not add a duplicate observer
  virtual void setValue(const QString& name, const QVariant& value, const MetaData& metaData, ObserverPtr observer=simQt::Settings::ObserverPtr()) override;
  /// Set value; will create if does not exist; updates the metaData; will not call the specified observer to prevent a feedback loop
  virtual void setValue(const QString& name, const QVariant& value, ObserverPtr skipThisObserver) override;
  /// Returns value for specified name; will return QVariant::Invalid if name does not exist
  virtual QVariant value(const QString& name) const override;
  /// Returns value for specified name; will create if it does not exist and return default value in MetaData
  virtual QVariant value(const QString& name, const MetaData& metaData, ObserverPtr observer=simQt::Settings::ObserverPtr()) override;
  /// Returns value for specified name; will create if it does not exist and use default value in MetaData
  virtual QVariant value(const QString& name, ObserverPtr observer) override;
  /// Returns true if the name exists
  virtual bool contains(const QString& name) const override;

   /// Saves widget info starting at the specified path
  virtual void saveWidget(QWidget* widget) override;
  /// Loads widget info starting at the specified path
  virtual void loadWidget(QWidget* widget) override;

  /// Returns all names
  virtual QStringList allNames() const override;

  /// Set Metadata
  virtual int setMetaData(const QString& name, const MetaData& metaData) override;
  /// Get Metadata
  virtual int metaData(const QString& name, MetaData& metaData) const override;

  /// Add local observers; only when this entry changes do a callback
  virtual int addObserver(const QString& name, ObserverPtr observer) override;
  /// Remove local observers
  virtual int removeObserver(const QString& name, ObserverPtr observer) override;

  /// Add global observers; if any entry changes do a callback
  virtual void addObserver(ObserverPtr observer) override;
  /// Remove global observers
  virtual int removeObserver(ObserverPtr observer) override;

  /// Retrieves the path to the underlying storage; on Windows if using NativeFormat, this could be a registry path and not a file path
  virtual QString fileName() const override;

  /// Creates a memento for saving and restoring state
  virtual Memento* createMemento() const override;

private:
  QString getFullPath_(const QString& name) const;
  int getObserver_(const QString& name, ObserverPtr unwrapped, ObserverPtr& wrapped) const;

  Settings* settings_ = nullptr;
  QString path_;  ///< The initial part of the name
  QMap<QString, QList<ObserverPtr> > localObservers_;  ///< Keep track of the wrapped local observers
  QList<ObserverPtr> globalObservers_; ///< Keep track of the wrapped global observers
};

typedef std::shared_ptr<SettingsGroup> SettingsGroupPtr;
}

#endif
