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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QSettings>
#include <QFileDialog>
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simQt/FileDialog.h"

namespace simQt
{

/** Retrieves the default options for every file dialog.  Used to override native dialogs. */
QFileDialog::Options FileDialog::getFileDialogDefaultOptions()
{
#ifdef WIN32
  // If not defined, or if it's defined as "1", then use the native dialog.  There are some applications,
  // e.g. ones that use certain types of COM from SIMDIS Plug-ins, that may need to force Native Dialogs off.
  if (!getenv("SDK_NATIVE_FILE_DIALOG") || (strcmp(getenv("SDK_NATIVE_FILE_DIALOG"), "1") == 0))
    return 0;
#endif
  // On Linux, always avoid the native dialog due to event loop problems with FOX in SIMDIS 10
  return QFileDialog::DontUseNativeDialog;
}

QString FileDialog::getRegistryDir(const QString& registryDir)
{
  if (!registryDir.isEmpty())
  {
    QSettings settings;
    return settings.value(registryDir).toString();
  }
  return "";
}

void FileDialog::setRegistryDir(const QString& registryDir, const QString& path, bool overwrite)
{
  if (registryDir.isEmpty())
    return;
  QSettings settings;
  if (overwrite || !settings.contains(registryDir))
  {
    settings.setValue(registryDir, QDir::toNativeSeparators(
      QString::fromStdString(simCore::expandEnv(path.toStdString()))));
  }
}

QString FileDialog::foxToQtFilter(const QString& foxFilter)
{
  // Need to replace newlines with two semicolons, so split the string
  QStringList eachLine = foxFilter.split("\n", QString::SkipEmptyParts);
  // Replaces the commas in (*.xml,*.txt) with whitespace (*.xml *.txt)
  eachLine.replaceInStrings(", ", " "); // (*.xml, *.txt) [note the space after the comma]
  eachLine.replaceInStrings(",", " "); // (*.xml,*.txt)
  return eachLine.join(";;");
}

QString FileDialog::saveFile(QWidget* owner, const QString& caption, const QString& registryDir,
                             const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  // linux needs the main window activated, to ensure dialog shows on top
#ifndef WIN32
  if (owner)
    owner->activateWindow();
#endif

  QString directory = FileDialog::getRegistryDir(registryDir);

  // If the caller did not provide a selected filter, attempt to find a matching filter
  QString* localPointer = selectedFilter;
  QString localSelectedFilter;
  if (localPointer == NULL)
  {
    QString ext = QString::fromStdString(simCore::getExtension(directory.toStdString()));
    if (!ext.isEmpty())
    {
      QStringList eachLine = filter.split("\n", QString::SkipEmptyParts);
      QString match = "(*" + ext + ")";
      for (auto it = eachLine.begin(); it != eachLine.end(); ++it)
      {
        if ((*it).contains(match))
        {
          localSelectedFilter = *it;
          localPointer = &localSelectedFilter;
          break;
        }
      }
    }
  }

  QString file = QFileDialog::getSaveFileName(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), localPointer, options | getFileDialogDefaultOptions());
  if (!file.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, file, true);
  }
  return file;
}

QString FileDialog::loadFile(QWidget* owner, const QString& caption, const QString& registryDir,
                             const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  // linux needs the main window activated, to ensure dialog shows on top
#ifndef WIN32
  if (owner)
    owner->activateWindow();
#endif
  QString directory = FileDialog::getRegistryDir(registryDir);
  QString file = QFileDialog::getOpenFileName(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), selectedFilter, options | getFileDialogDefaultOptions());
  if (!file.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, file, true);
  }
  return file;
}

QStringList FileDialog::loadFiles(QWidget* owner, const QString& caption, const QString& registryDir,
                                  const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  // linux needs the main window activated, to ensure dialog shows on top
#ifndef WIN32
  if (owner)
    owner->activateWindow();
#endif
  QString directory = FileDialog::getRegistryDir(registryDir);
  QStringList files = QFileDialog::getOpenFileNames(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), selectedFilter, options | getFileDialogDefaultOptions());
  if (!files.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, files.first(), true);
  }
  return files;
}

QString FileDialog::findDirectory(QWidget* owner, const QString& caption, const QString& registryDir,
   QFileDialog::Options options)
{
  // linux needs the main window activated, to ensure dialog shows on top
#ifndef WIN32
  if (owner)
    owner->activateWindow();
#endif
  QString priorDirectory = FileDialog::getRegistryDir(registryDir);
  QString directory = QFileDialog::getExistingDirectory(owner, caption, priorDirectory, options | getFileDialogDefaultOptions());
  if (!directory.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, directory, true);
  }
  return directory;
}

} // namespace simQt
