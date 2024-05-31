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
#include <QColumnView>
#include <QDialog>
#include <QGuiApplication>
#include <QHeaderView>
#include <QMainWindow>
#include <QScreen>
#include <QString>
#include <QSplitter>
#include <QTableView>
#include <QTreeView>
#include "simNotify/Notify.h"
#include "simQt/Settings.h"
#include "simQt/WidgetSettings.h"

namespace simQt
{

static const QString& COLUMN_WIDTHS = "/Column Widths";
static const QString& SPLITTER_DATA = "/Splitter Data";
static const QString& SORT_COLUMN = "/Sort Column";
static const QString& SORT_ORDER = "/Sort Order";

void WidgetSettings::saveWidget(simQt::Settings& settings, QWidget* widget)
{
  if (widget == nullptr)
    return;

  WidgetSettings::saveWindowGeometry_(settings, WINDOWS_SETTINGS, widget);
}

void WidgetSettings::loadWidget(simQt::Settings& settings, QWidget* widget)
{
  if (widget == nullptr)
    return;

  WidgetSettings::loadWindowGeometry_(settings, WINDOWS_SETTINGS, widget);
}

int WidgetSettings::saveQSplitter_(simQt::Settings& settings, const QString& path, const QSplitter* splitter)
{
  if (splitter == nullptr)
    return 1;

  if (splitter->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use saveWidget [QSplitter].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  settings.setValue(path + SPLITTER_DATA, splitter->saveState(), simQt::Settings::MetaData::makeInteger(QVariant(), "", simQt::Settings::PRIVATE, 1));
  return 0;
}

int WidgetSettings::saveQTreeView_(simQt::Settings& settings, const QString& path, const QTreeView* view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use saveWidget [QTreeView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  QAbstractItemModel* model = view->model();
  if (model)
  {
    int columns = model->columnCount();
    // Ignore the last column if it stretches across
    if (view->header() != nullptr && view->header()->stretchLastSection())
      --columns;
    if (columns > 0)
    {
      QList<QVariant> sizes;
      for (int ii = 0; ii < columns; ++ii)
        sizes.push_back(view->columnWidth(ii));

      settings.setValue(path + COLUMN_WIDTHS, sizes, simQt::Settings::MetaData::makeInteger(QVariant(), "", simQt::Settings::PRIVATE, 0));
    }

    if (view->isSortingEnabled())
    {
      settings.setValue(path + SORT_COLUMN, view->header()->sortIndicatorSection());
      settings.setValue(path + SORT_ORDER, static_cast<int>(view->header()->sortIndicatorOrder()));
    }
  }

  return 0;
}

int WidgetSettings::saveQColumnView_(simQt::Settings& settings, const QString& path, const QColumnView* view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use saveWidget [QColumnView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  // TODO: Need to implement saving column widths
  assert(false);

  return 0;
}

int WidgetSettings::saveQTableView_(simQt::Settings& settings, const QString& path, const QTableView* view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use saveWidget [QTableView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  // Intentional did not save row height

  QAbstractItemModel* model = view->model();
  if (model)
  {
    int columns = model->columnCount();
    // Ignore the last column if it stretches across
    if (view->horizontalHeader() != nullptr && view->horizontalHeader()->stretchLastSection())
      --columns;
    if (columns > 0)
    {
      QList<QVariant> sizes;
      for (int ii = 0; ii < columns; ++ii)
        sizes.push_back(view->columnWidth(ii));

      settings.setValue(path + COLUMN_WIDTHS, sizes, simQt::Settings::MetaData::makeInteger(QVariant(), "", simQt::Settings::PRIVATE, 0));
    }
  }

  return 0;
}

int WidgetSettings::saveQDialog_(simQt::Settings& settings, const QString& path, const QDialog* dialog)
{
  if (dialog == nullptr)
    return 1;

  if (dialog->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use saveQDialog_ [QDialog].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  QPoint point = dialog->pos();
  settings.setValue(path + "/Position", point, simQt::Settings::MetaData::makePoint(point, "", simQt::Settings::PRIVATE));

  QSize size = dialog->size();
  settings.setValue(path + "/Size", size, simQt::Settings::MetaData::makeSize(size, "", simQt::Settings::PRIVATE));

  return 0;
}

void WidgetSettings::saveWindowGeometry_(simQt::Settings& settings, const QString& path, const QObject *object)
{
  auto shouldSkip = object->property(DO_NOT_SAVE_GEOMETRY);
  if (shouldSkip.isValid() && shouldSkip.toBool())
    return;

  QString newPath = path;
  if (object->objectName().isEmpty())
    newPath += "NA";  // NA for Not Available
  else
    newPath += object->objectName();

  if (object->isWidgetType())
  {
    int rv = WidgetSettings::saveQSplitter_(settings, newPath, dynamic_cast<const QSplitter*>(object));

    if (rv != 0)
      rv = WidgetSettings::saveQTreeView_(settings, newPath, dynamic_cast<const QTreeView*>(object));

    if (rv != 0)
      rv = WidgetSettings::saveQColumnView_(settings, newPath, dynamic_cast<const QColumnView*>(object));

    if (rv != 0)
      rv = WidgetSettings::saveQTableView_(settings, newPath, dynamic_cast<const QTableView*>(object));

    if (rv != 0)
      rv = WidgetSettings::saveQDialog_(settings, newPath, dynamic_cast<const QDialog*>(object));

    if (rv != 0)
    {
      const QTabWidget* tab = dynamic_cast<const QTabWidget*>(object);
      if (tab != nullptr)
      {
        for (int ii = 0; ii < tab->count(); ++ii)
          WidgetSettings::saveWindowGeometry_(settings, newPath + "/", tab->widget(ii));
      }
    }
  }

  auto childrenList = object->children();
  for (auto it = childrenList.begin(); it != childrenList.end(); ++it)
    WidgetSettings::saveWindowGeometry_(settings, newPath + "/", *it);
}

int WidgetSettings::loadQSplitter_(simQt::Settings& settings, const QString& path, QSplitter *splitter)
{
  if (splitter == nullptr)
    return 1;

  if (splitter->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use loadWidget [QSplitter].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  if (settings.contains(path + SPLITTER_DATA))
    splitter->restoreState(settings.value(path + SPLITTER_DATA).toByteArray());

  return 0;
}

int WidgetSettings::loadQTreeView_(simQt::Settings& settings, const QString& path, QTreeView *view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use loadWidget [QTreeView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  QAbstractItemModel* model = view->model();
  if (model)
  {
    if (settings.contains(path + COLUMN_WIDTHS))
    {
      int modelColumns = model->columnCount();
      QList<QVariant> sizes = settings.value(path + COLUMN_WIDTHS).toList();
      for (int ii = 0; ii < modelColumns && ii < sizes.size(); ++ii)
        view->setColumnWidth(ii, sizes[ii].toInt());
    }

    if (view->isSortingEnabled() && settings.contains(path + SORT_COLUMN) && settings.contains(path + SORT_ORDER))
    {
      const int sortColumn = settings.value(path + SORT_COLUMN).toInt();
      const int sortOrder = settings.value(path + SORT_ORDER).toInt();
      view->sortByColumn(sortColumn, static_cast<Qt::SortOrder>(sortOrder));
    }
  }

  return 0;
}

int WidgetSettings::loadQColumnView_(simQt::Settings& settings, const QString& path, QColumnView *view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use loadWidget [QColumnView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  // TODO: Need to implement loading column widths
  assert(false);

  return 0;
}

int WidgetSettings::loadQTableView_(simQt::Settings& settings, const QString& path, QTableView *view)
{
  if (view == nullptr)
    return 1;

  if (view->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use loadWidget [QTableView].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  QAbstractItemModel* model = view->model();
  if (model)
  {
    if (settings.contains(path + COLUMN_WIDTHS))
    {
      int modelColumns = model->columnCount();
      QList<QVariant> sizes = settings.value(path + COLUMN_WIDTHS).toList();
      for (int ii = 0; ii < modelColumns && ii < sizes.size(); ++ii)
        view->setColumnWidth(ii, sizes[ii].toInt());
    }
  }

  return 0;
}

int WidgetSettings::loadQDialog_(simQt::Settings& settings, const QString& path, QDialog* dialog)
{
  if (dialog == nullptr)
    return 1;

  if (dialog->objectName().isEmpty())
  {
#ifndef NDEBUG
    // Implies that a child with the given path has items without objectNames; use setObjectName() as needed to solve.
    SIM_ERROR << "Widget objectName must not be empty to use loadQDialog_ [QDialog].  path = " << path.toStdString() << std::endl;
#endif
    return 0;
  }

  QPoint point;
  if (settings.contains(path + "/Position"))
  {
    point = settings.value(path + "/Position").toPoint();

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // Only call move() if the point is valid in the available screen geometry. This covers
    // use cases where position USED to be valid, but is no longer available (screen removed, etc)
    const auto& screens = QGuiApplication::screens();
    bool validPoint = false;
    for (const auto* screen : screens)
    {
      if (screen && screen->availableGeometry().contains(point))
      {
        validPoint = true;
        break;
      }
    }
    if (validPoint)
#endif
    {
      dialog->move(point);
    }
  }

  QSize size;
  if (settings.contains(path + "/Size"))
  {
    size = settings.value(path + "/Size").toSize();
    dialog->resize(size);
  }

  QMainWindow* main = dynamic_cast<QMainWindow*>(dialog->parent());
  if (main)
    main->updateGeometry();

  return 0;
}

void WidgetSettings::loadWindowGeometry_(simQt::Settings& settings, const QString& path, QObject *object)
{
  QString newPath = path;
  if (object->objectName().isEmpty())
    newPath += "NA";  // NA for Not Available
  else
    newPath += object->objectName();

  if (object->isWidgetType())
  {
    int rv = WidgetSettings::loadQSplitter_(settings, newPath, dynamic_cast<QSplitter*>(object));

    if (rv != 0)
      rv = WidgetSettings::loadQTreeView_(settings, newPath, dynamic_cast<QTreeView*>(object));

    if (rv != 0)
      rv = WidgetSettings::loadQColumnView_(settings, newPath, dynamic_cast<QColumnView*>(object));

    if (rv != 0)
      rv = WidgetSettings::loadQTableView_(settings, newPath, dynamic_cast<QTableView*>(object));

    if (rv != 0)
      rv = WidgetSettings::loadQDialog_(settings, newPath, dynamic_cast<QDialog*>(object));

    if (rv != 0)
    {
      const QTabWidget* tab = dynamic_cast<const QTabWidget*>(object);
      if (tab != nullptr)
      {
        for (int ii = 0; ii < tab->count(); ++ii)
          WidgetSettings::loadWindowGeometry_(settings, newPath + "/", tab->widget(ii));
      }
    }
  }

  // Scan children
  auto childrenList = object->children();
  for (auto it = childrenList.begin(); it != childrenList.end(); ++it)
    WidgetSettings::loadWindowGeometry_(settings, newPath + "/", *it);
}

}
