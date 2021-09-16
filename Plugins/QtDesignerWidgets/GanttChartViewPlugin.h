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
#ifndef GANTTCHARTVIEWPLUGIN_H
#define GANTTCHARTVIEWPLUGIN_H

#include <QDesignerCustomWidgetInterface>

class QStandardItemModel;

// Wrapper class for the GanttChartView to provide QDesignerCustomWidgetInterface
class GanttChartViewPlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT;
  Q_INTERFACES(QDesignerCustomWidgetInterface);

public:
  explicit GanttChartViewPlugin(QObject *parent = 0);

  bool isContainer() const;
  bool isInitialized() const;
  QIcon icon() const;
  QString domXml() const;
  QString group() const;
  QString includeFile() const;
  QString name() const;
  QString toolTip() const;
  QString whatsThis() const;
  QWidget *createWidget(QWidget *parent);
  void initialize(QDesignerFormEditorInterface *core);

private:
  void addSampleData_(QStandardItemModel* model) const;
  bool initialized_;
};

#endif // GANTTCHARTVIEWPLUGIN_H
