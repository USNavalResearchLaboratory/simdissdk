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
#include "simCore/String/Utils.h"
#include "simQt/FileDialog.h"

namespace simQt
{

// On Linux, avoid the native dialog due to event loop problems with FOX in SIMDIS 10
//#ifndef WIN32
// SPR-1348 Due an important demo with the JaarDs plugin disable all native dialog
static const QFileDialog::Option BASE_OPTIONS = QFileDialog::DontUseNativeDialog;
//#else
//static const QFileDialog::Option BASE_OPTIONS = static_cast<QFileDialog::Option>(0x0);
//#endif

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
  QString directory = FileDialog::getRegistryDir(registryDir);
  QString file = QFileDialog::getSaveFileName(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), selectedFilter, options | BASE_OPTIONS);
  if (!file.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, file, true);
  }
  return file;
}

QString FileDialog::loadFile(QWidget* owner, const QString& caption, const QString& registryDir,
                             const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  QString directory = FileDialog::getRegistryDir(registryDir);
  QString file = QFileDialog::getOpenFileName(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), selectedFilter, options | BASE_OPTIONS);
  if (!file.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, file, true);
  }
  return file;
}

QStringList FileDialog::loadFiles(QWidget* owner, const QString& caption, const QString& registryDir,
                                  const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  QString directory = FileDialog::getRegistryDir(registryDir);
  QStringList files = QFileDialog::getOpenFileNames(owner, caption, directory,
    FileDialog::foxToQtFilter(filter), selectedFilter, options | BASE_OPTIONS);
  if (!files.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, files.first(), true);
  }
  return files;
}

QString FileDialog::findDirectory(QWidget* owner, const QString& caption, const QString& registryDir,
   QFileDialog::Options options)
{
  QString priorDirectory = FileDialog::getRegistryDir(registryDir);
  QString directory = QFileDialog::getExistingDirectory(owner, caption, priorDirectory, options | BASE_OPTIONS);
  if (!directory.isEmpty() && !registryDir.isEmpty())
  {
    FileDialog::setRegistryDir(registryDir, directory, true);
  }
  return directory;
}

} // namespace simQt
