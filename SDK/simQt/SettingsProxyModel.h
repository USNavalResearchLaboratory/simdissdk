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
#ifndef SIMQT_SETTINGSPROXYMODEL_H
#define SIMQT_SETTINGSPROXYMODEL_H

#include <set>
#include <QSortFilterProxyModel>
#include "simQt/Settings.h"

namespace simQt {

/**
 * Proxy model for simQt::SettingsModel that adds a search capability.
 */
class SDKQT_EXPORT SettingsSearchFilter : public QSortFilterProxyModel
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsSearchFilter(QAbstractItemModel* settingsModel, QWidget* parent=nullptr);

  /** Implements the QSortFilterProxyModel method to apply filtering to the row */
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
  /** Returns the current filter text */
  QString filterText() const;

public slots:
  /** Changes the filter text */
  void setFilterText(const QString& filterText);

private:
  /** Apply the reg exp filtering, returns true if no regexp filter is set */
  bool testRegExp_(const QModelIndex& index0, const QModelIndex& index1, const QModelIndex& parent) const;
};

/**
 * Proxy model for simQt::SettingsModel that filters out PRIVATE settings, and
 * optionally filters out ADVANCED settings.
 */
class SDKQT_EXPORT SettingsDataLevelFilter : public QSortFilterProxyModel
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsDataLevelFilter(QAbstractItemModel* settingsModel, QWidget* parent=nullptr);

  /** Implements the QSortFilterProxyModel method to apply filtering to the row  */
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

  /** Set the show advanced filter, which will show settings with the ADVANCED data level settings if true */
  void setShowAdvanced(bool showAdvanced);
  /** Returns whether we show ADVANCED data level settings */
  bool showAdvanced() const;
  /** Sets the show-unknown filter, which will show settings with the UNKNOWN data level settings if true */
  void setShowUnknown(bool showUnknown);
  /** Returns whether we show UNKNOWN data level settings */
  bool showUnknown() const;

private:
  /** Apply the data level filtering */
  bool testDataLevel_(Settings::DataLevel dataLevel) const;

  /// Flag indicating if data with DataLevel of ADVANCED can pass the filter
  bool showAdvanced_;
  /// Flag indicating if data with DataLevel of UNKNOWN can pass the filter
  bool showUnknown_;

  //// all data types that are considered invalid by the filter
  std::set<QVariant::Type> invalidDataTypes_;

};

/**
 * Proxy model for simQt::SettingsModel that removes any empty folders.  If you have
 * other filtering proxy models on top of your SettingsModel, you likely want this
 * to be the top-most filter so that empty folders are always removed from display.
 */
class SDKQT_EXPORT SettingsNoEmptyFoldersFilter : public QSortFilterProxyModel
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsNoEmptyFoldersFilter(QAbstractItemModel* settingsModel, QWidget* parent=nullptr);

  /** Implements the QSortFilterProxyModel method to apply filtering to the row  */
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  /** True when the source parent provided actually has a child */
  bool hasChildren_(const QModelIndex& sourceParent) const;
  /** True when the source parent is a folder (with or without children) */
  bool isFolder_(int sourceRow, const QModelIndex& sourceParent) const;
};

/**
 * Composite filter that combines all the other filters in an easy-to-use way.  To
 * use the search, simply call setFilterText().  You may use or chain any of the
 * other filters together manually, or use this filter instead for convenience.
 */
class SDKQT_EXPORT SettingsProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT;
public:
  /** Constructor */
  SettingsProxyModel(QAbstractItemModel* settingsModel, QWidget* parent=nullptr);
  /** Destructor */
  virtual ~SettingsProxyModel();

public slots:
  /** Set the show advanced filter, which will show settings with the ADVANCED data level settings if true */
  void setShowAdvanced(bool showAdvanced);
  /** Sets the show-unknown filter, which will show settings with the UNKNOWN data level settings if true */
  void setShowUnknown(bool showUnknown);
  /** Changes the filter text */
  void setFilterText(const QString& filterText);

private slots:
  /** Reacts to a change in the list of settings, such as when a new setting is registered */
  void invalidateAll_();

private:
  SettingsSearchFilter* search_;
  SettingsDataLevelFilter* dataLevel_;
  SettingsNoEmptyFoldersFilter* noEmptyFolders_;
};

}

#endif


