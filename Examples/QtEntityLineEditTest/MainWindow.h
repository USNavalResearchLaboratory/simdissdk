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
#ifndef EXAMPLES_ENTITY_LINE_EDIT_MAINWINDOW_H
#define EXAMPLES_ENTITY_LINE_EDIT_MAINWINDOW_H

#include <QDialog>

#include "simCore/Common/Common.h"

class Ui_MainWindow;

namespace simData { class DataStore; }
namespace simQt { class EntityTreeModel; }

/// A simple dialog for testing the EntityLineComposite widget
class MainWindow : public QDialog
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent, simData::DataStore* dataStore);
  virtual ~MainWindow();

protected Q_SLOTS:
  void addPlatform_();
  void addBeam_();
  void addGate_();
  void addPlatforms_();
  void delete_();
  void rename_();
  void itemSelected_(uint64_t id);

protected:
  Ui_MainWindow* mainWindowGui_;
  simData::DataStore* dataStore_;
  simQt::EntityTreeModel* entityTreeModel_;
};

#endif
