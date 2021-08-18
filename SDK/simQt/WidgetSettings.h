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
#ifndef SIMQT_WIDGETSETTINGS_H
#define SIMQT_WIDGETSETTINGS_H

#include "simCore/Common/Export.h"

class QObject;
class QString;
class QDialog;
class QSplitter;
class QTreeView;
class QColumnView;
class QTableView;

namespace simQt
{

class Settings;

/**
 * The routines saveWidget and loadWidget are helper routines to save off routine window layout
 * information.  Information includes:
 *
 * QDialog: Window location and size
 * QSplitter: Splitter location
 * QTreeView: Column widths
 * QTableView: Column widths
 *
 * The routines start at the given object and recursively search children for the supported
 * widgets.  The names of the objects are used to make the Setting names.  The routine will
 * ignore objects without a name.  In a Debug build objects without names will generate a
 * SIM_ERROR message.  Call the load routine after the widget has been created and call the save
 * routine before the widget is destroyed.  The routines will not catch any dynamic behavior such
 * as adding and removing QTreeView.
 */
class SDKQT_EXPORT WidgetSettings
{
public:

  /**
   * Saves widget layout information.  Includes QDialog window location and size, QSplitter
   * splitter locations, QTreeView and QTable column widgets.
   * @param settings Settings in which to save the values
   * @param widget Widget to recursively search for other widgets to save
   */
  static void saveWidget(simQt::Settings& settings, QWidget* widget);

  /**
   * Loads widget layout information.  Includes QDialog window location and size, QSplitter
   * splitter locations, QTreeView and QTable column widgets.
   * @param settings Settings from which to load the values
   * @param widget Widget to load data from the settings
   */
  static void loadWidget(simQt::Settings& settings, QWidget* widget);

private:
  /// Saves splitter info at the specified path; returns 0 if splitter is non-nullptr and 1 if someone else needs to process it
  static int saveQSplitter_(simQt::Settings& settings, const QString& path, const QSplitter *splitter);
  /// Saves tree view info at the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int saveQTreeView_(simQt::Settings& settings, const QString& path, const QTreeView *view);
  /// Saves column view info at the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int saveQColumnView_(simQt::Settings& settings, const QString& path, const QColumnView *view);
  /// Saves table view info at the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int saveQTableView_(simQt::Settings& settings, const QString& path, const QTableView *view);
  /// Saves dialog info at the specified path; returns 0 if dialog is non-nullptr and 1 if someone else needs to process it
  static int saveQDialog_(simQt::Settings& settings, const QString& path, const QDialog* dialog);
  /// Recursively searches object for savable widgets and stores the info starting at path
  static void saveWindowGeometry_(simQt::Settings& settings, const QString& path, const QObject *object);

  /// Loads splitter info from the specified path; returns 0 if splitter is non-nullptr and 1 if someone else needs to process it
  static int loadQSplitter_(simQt::Settings& settings, const QString& path, QSplitter *splitter);
  /// Loads tree view info from the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int loadQTreeView_(simQt::Settings& settings, const QString& path, QTreeView *view);
  /// Loads column view info from the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int loadQColumnView_(simQt::Settings& settings, const QString& path, QColumnView *view);
  /// Loads table view info from the specified path; returns 0 if view is non-nullptr and 1 if someone else needs to process it
  static int loadQTableView_(simQt::Settings& settings, const QString& path, QTableView *view);
  /// Loads dialog info from the specified path; returns 0 if dialog is non-nullptr and 1 if someone else needs to process it
  static int loadQDialog_(simQt::Settings& settings, const QString& path, QDialog* dialog);
  /// Recursively searches object for loadable widgets and gets the info starting at path
  static void loadWindowGeometry_(simQt::Settings& settings, const QString& path, QObject *object);
};

}

#endif /* SIMQT_WIDGETSETTINGS_H */
