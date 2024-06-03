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
#include <QtCore/QtPlugin>
#include <QStandardItemModel>
#include "simQt/GanttChartView.h"
#include "GanttChartViewPlugin.h"

GanttChartViewPlugin::GanttChartViewPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void GanttChartViewPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool GanttChartViewPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *GanttChartViewPlugin::createWidget(QWidget *parent)
{
  simQt::GanttChartView* view = new simQt::GanttChartView(parent);
  QStandardItemModel* tmpModel = new QStandardItemModel(view);
  addSampleData_(tmpModel);
  view->setModel(tmpModel);
  return view;
}

QString GanttChartViewPlugin::name() const
{
  return "simQt::GanttChartView";
}

QString GanttChartViewPlugin::group() const
{
  return "simQt";
}

QIcon GanttChartViewPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Chart Gantt.png");
}

QString GanttChartViewPlugin::toolTip() const
{
  return "View which creates a Gantt chart from a Qt model.";
}

QString GanttChartViewPlugin::whatsThis() const
{
  return "View which creates a Gantt chart from a Qt model.";
}

bool GanttChartViewPlugin::isContainer() const
{
  return false;
}

QString GanttChartViewPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Gantt Chart View\">"
    "<widget class=\"simQt::GanttChartView\" name=\"ganttChartView\">\n"
    "</widget>\n"
    "</ui>";
}

QString GanttChartViewPlugin::includeFile() const
{
  return "simQt/GanttChartView.h";
}

void GanttChartViewPlugin::addSampleData_(QStandardItemModel* model) const
{
  // Child items of the model root are treated as rows.  Children of those row items are bars in those rows
  QStandardItem* row1 = new QStandardItem();

  // Creating one bar on the chart uses three QStandardItems.  Using lists to organize them for clarity
  QList<QStandardItem*> item1 = QList<QStandardItem*>();

  QIcon icon(":/SDKPlugins/images/Chart Gantt.png");
  // First item contains icon, tool tip, and bar color
  QStandardItem* item1Details = new QStandardItem(tr("Display Text 1"));
  item1Details->setData(icon, Qt::DecorationRole);
  item1Details->setData(tr("Tooltip 1"), Qt::ToolTipRole);
  item1Details->setData(QColor(Qt::red), Qt::ForegroundRole);

  // Second and third items store the start and end points of the bar respectively
  item1.append(item1Details);
  QStandardItem* item1Start = new QStandardItem();
  item1Start->setData(100, Qt::DisplayRole);
  item1.append(item1Start);

  QStandardItem* item1End = new QStandardItem();
  item1End->setData(200, Qt::DisplayRole);
  item1.append(item1End);

  row1->appendRow(item1);

  QList<QStandardItem*> item2 = QList<QStandardItem*>();

  QStandardItem* item2Details = new QStandardItem(tr("Display Text 2"));
  item2Details->setData(icon, Qt::DecorationRole);
  item2Details->setData(tr("Tooltip 2"), Qt::ToolTipRole);
  item2Details->setData(QColor(Qt::blue), Qt::ForegroundRole);

  item2.append(item2Details);
  QStandardItem* item2Start = new QStandardItem();
  item2Start->setData(300, Qt::DisplayRole);
  item2.append(item2Start);

  QStandardItem* item2End = new QStandardItem();
  item2End->setData(500, Qt::DisplayRole);
  item2.append(item2End);

  row1->appendRow(item2);
  // QStandardItems are deleted by model on destruction.  Don't need to manage that memory.
  model->appendRow(row1);

  // End of first row, beginning of second
  QStandardItem* row2 = new QStandardItem();

  QStandardItem* item3Details = new QStandardItem(tr("Display 3"));
  item3Details->setData(icon, Qt::DecorationRole);
  item3Details->setData(tr("Tooltip 3"), Qt::ToolTipRole);
  item3Details->setData(QColor(Qt::red), Qt::ForegroundRole);

  QList<QStandardItem*> item3 = QList<QStandardItem*>();
  item3.append(item3Details);
  QStandardItem* item3Start = new QStandardItem();
  item3Start->setData(300, Qt::DisplayRole);
  item3.append(item3Start);

  QStandardItem* item3End = new QStandardItem();
  item3End->setData(400, Qt::DisplayRole);
  item3.append(item3End);

  row2->appendRow(item3);
  model->appendRow(row2);
}

