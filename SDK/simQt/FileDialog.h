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
#ifndef SIMQT_FILEDIALOG_H
#define SIMQT_FILEDIALOG_H

#include <QFileDialog>
#include "simCore/Common/Export.h"

namespace simQt
{

/**
 * Defines common functions to use for save and load dialog boxes.  A common
 * feature of the SIMDIS load/save dialog boxes is to remember the last directory
 * the user was in
 */
class SDKQT_EXPORT FileDialog
{
public:
  /**
   * Returns the file location of the registry directory given.  Typically not required by
   * end user applications except to verify contents of the setting
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @return Registry directory full path, or empty if not found
   */
  static QString getRegistryDir(const QString& registryDir);

  /**
  * Returns the suggested file dialog default options for the current system. This is useful when users
  * want to instantiate their own instance of QFileDialog, and the options may be differ depending
  * on system conditions.
  * @return file dialog options
  */
  static QFileDialog::Options getFileDialogDefaultOptions();

  /**
   * Sets the default value for a registry directory.  This can either overwrite the existing
   * entry, or simply prime the entry if one does not already exist.
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @param path Path setting to save in the registry for this variable
   * @param overwrite If false, the value is only set if it doesn't exist; if true, the value is set regardless of what is there
   */
  static void setRegistryDir(const QString& registryDir, const QString& path, bool overwrite=false);

  /**
   * Displays a dialog box suitable for saving a file
   * @param owner Widget owner for stacking order; nullptr to show with no parent
   * @param caption Top level window text title
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @param filter Newline separated list of filters, like "Text Files (*.txt,*.ascii)\nAll Files (*)"
   * @param selectedFilter If non-nullptr, the input value will select the default filter, the output value will contain the text of
   *   the filter that was selected by the user when file was opened.  If nullptr the code will use the extension of the last saved
   *   file to set the default filter.
   * @param options Qt options for the file dialog, such as QFileDialog::ShowDirsOnly.  See QFileDialog documentation for all options
   * @return Name of file selected by user for saving
   */
  static QString saveFile(QWidget* owner=nullptr, const QString& caption="", const QString& registryDir="",
    const QString& filter="All Files (*)", QString* selectedFilter=nullptr, QFileDialog::Options options=0);

  /**
   * Displays a dialog box suitable for loading a single file
   * @param owner Widget owner for stacking order; nullptr to show with no parent
   * @param caption Top level window text title
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @param filter Newline separated list of filters, like "Text Files (*.txt,*.ascii)\nAll Files (*)"
   * @param selectedFilter If non-nullptr, will contain the text of the filter that was selected by the user when file was opened
   * @param options Qt options for the file dialog, such as QFileDialog::ShowDirsOnly.  See QFileDialog documentation for all options
   * @return Name of file selected by user for loading
   */
  static QString loadFile(QWidget* owner=nullptr, const QString& caption="", const QString& registryDir="",
    const QString& filter="All Files (*)", QString* selectedFilter=nullptr, QFileDialog::Options options=0);

  /**
   * Displays a dialog box suitable for loading a multiple files
   * @param owner Widget owner for stacking order; nullptr to show with no parent
   * @param caption Top level window text title
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @param filter Newline separated list of filters, like "Text Files (*.txt,*.ascii)\nAll Files (*)"
   * @param selectedFilter If non-nullptr, will contain the text of the filter that was selected by the user when file was opened
   * @param options Qt options for the file dialog, such as QFileDialog::ShowDirsOnly.  See QFileDialog documentation for all options
   * @return Names of files selected by user for loading
   */
  static QStringList loadFiles(QWidget* owner=nullptr, const QString& caption="", const QString& registryDir="",
    const QString& filter="All Files (*)", QString* selectedFilter=nullptr, QFileDialog::Options options=0);

  /**
   * Displays a dialog box suitable for browsing for a directory
   * @param owner Widget owner for stacking order; nullptr to show with no parent
   * @param caption Top level window text title
   * @param registryDir Registry directory to load/save last directory, such as "SETTINGS/LastDir" or "DirectorySettings/ImportantFiles"
   * @param options Qt options for the file dialog, such as QFileDialog::ShowDirsOnly.  See QFileDialog documentation for all options
   * @return Name of directory selected by user
   */
  static QString findDirectory(QWidget* owner=nullptr, const QString& caption="", const QString& registryDir="",
    QFileDialog::Options options=0);


  /**
   * Converts a FOX-style filter to a Qt filter.  FOX filters separate multiple filter lines with newlines (\n) while
   * Qt uses a double semicolon (;;).  FOX separates filters on the same line using a comma, but Qt uses a space or semicolon.
   * @param foxFilter Fox-formatted filter, using newlines to separate filters and commas to separate extensions in a filter
   * @return Qt-formatted filter, using double semicolon to separate filters and single semicolon to separate extensions in a filter
   */
  static QString foxToQtFilter(const QString& foxFilter);
};

} // simQt

#endif /* SIMQT_FILEDIALOG_H */
