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
#ifndef EXAMPLES_ENTITYVIEW_MAINWINDOW_H
#define EXAMPLES_ENTITYVIEW_MAINWINDOW_H

#include <QString>
#include <QDialog>
#include "simCore/Common/Common.h"

class Ui_MainWindow;

namespace simData { class MemoryDataStore; }
namespace simQt {
  class EntityTreeComposite;
  class EntityTreeModel;
}

class MainWindow : public QDialog
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent);
  virtual ~MainWindow();

protected Q_SLOTS:
  void addPlatforms_();
  void addBeams_();
  void addGates_();
  void test_();
  void itemsSelected_(QList<uint64_t> ids);
  void deleteEntity_();
  void itemDoubleClicked_(uint64_t id);

protected:
  Ui_MainWindow* mainWindowGui_;
  simData::MemoryDataStore* dataStore_;
  simQt::EntityTreeModel* entityTreeModel_;
  simQt::EntityTreeComposite* entityTreeComposite_;
};

#endif /* EXAMPLES_ENTITYVIEW_MAINWINDOW_H */
