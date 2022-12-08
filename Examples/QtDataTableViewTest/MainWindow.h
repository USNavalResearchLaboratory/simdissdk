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
#ifndef DATATABLEVIEWTEST_MAINWINDOW_H
#define DATATABLEVIEWTEST_MAINWINDOW_H

#include <QDialog>
#include "simCore/Common/Common.h"
#include "simData/DataTable.h"

class Ui_MainWindow;
class QTreeView;

namespace simData { class DataTable; }
namespace simUtil { class DataStoreTestHelper; }
namespace simQt {
  class DataTableModel;
  class EntityTreeComposite;
  class EntityTreeModel;
}

namespace DataTableViewTest
{

  /**
  * Class provides a basic test dialog to test out the DataTableView, adding/removing DataTables to
  * platforms, and adding/removing rows/columns to DataTables
  */
  class MainWindow : public QDialog
  {
    Q_OBJECT;

  public:
    explicit MainWindow(QWidget *parent);
    virtual ~MainWindow();

  private Q_SLOTS:
    void addTable_();
    void tableSelected_(simData::DataTable* table);
    void addRow_();
    void addColumn_();
    void removeTable_();
    void removeRow_();
    void removeColumn_();
    void itemsSelected(QList<uint64_t> ids);
    void enableDataLimiting_(int enable);
    void setDataLimitPoints_(int numPoints);
    void setDataLimitTime_(double numSeconds);

  private:
    /** Responds to new and removed data tables */
    class TableManagerObserver;

    /** Fill the tree widget with the data table items */
    void fillDataTableWidget_(simData::DataTable* table);

    int counter_;
    Ui_MainWindow* ui_;
    simUtil::DataStoreTestHelper* testHelper_;
    simQt::EntityTreeModel* entityTreeModel_;
    simQt::EntityTreeComposite* entityTreeComposite_;
    simQt::DataTableModel* tableModel_;
  };

}

#endif
