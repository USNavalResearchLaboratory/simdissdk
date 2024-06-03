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
#include <sstream>
#include <QTreeWidget>
#include "simQt/DataTableModel.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/EntityTreeModel.h"
#include "simUtil/DataStoreTestHelper.h"
#include "ui_MainWindow.h"
#include "MainWindow.h"


namespace DataTableViewTest
{
  class ColumnVisitor : public simData::DataTable::ColumnVisitor
  {
  public:
    ColumnVisitor(){}
    virtual void visit(simData::TableColumn* column)
    {
      columns_.push_back(column);
    }

    void getColumns(std::vector<simData::TableColumn*>& columns)
    {
      columns.swap(columns_);
    }

  private:
    std::vector<simData::TableColumn*> columns_;
  };

  static const int CellValueRole = Qt::UserRole + 0;

  /** Fills in a QTreeWidget item with the data in the TableRow cells */
  class CellVisitor : public simData::TableRow::CellVisitor
  {
  public:
    explicit CellVisitor(QTreeWidgetItem* item) : item_(item) {}
    virtual void visit(simData::TableColumnId columnId, uint8_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, int8_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, uint16_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, int16_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, uint32_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, int32_t value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, uint64_t value)
    {
      addItemData_(columnId, QVariant(static_cast<unsigned long long>(value)));
    }
    virtual void visit(simData::TableColumnId columnId, int64_t value)
    {
      addItemData_(columnId, QVariant(static_cast<long long>(value)));
    }
    virtual void visit(simData::TableColumnId columnId, float value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, double value)
    {
      addItemData_(columnId, QVariant(value));
    }
    virtual void visit(simData::TableColumnId columnId, const std::string& value)
    {
      addItemData_(columnId, QVariant(value.c_str()));
    }
  private:

    void addItemData_(simData::TableColumnId colId, const QVariant& value)
    {
      item_->setData(static_cast<int>(colId + 1), CellValueRole, value);
      item_->setText(static_cast<int>(colId + 1), value.toString());
    }
    QTreeWidgetItem* item_;

  };

  /** Fill in a QTreeWiget with data in all the rows of a DataTable */
  class RowVisitor : public simData::DataTable::RowVisitor
  {
  public:
    explicit RowVisitor(QTreeWidget* tree) : tree_(tree) {}
    virtual VisitReturn visit(const simData::TableRow& row)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      tree_->addTopLevelItem(item);
      // fill in first column with time
      item->setData(0, CellValueRole, row.time());
      item->setText(0, QString("%1").arg(row.time()));
      // now visit the row, passing in the QTreeWidgetItem to be filled in
      CellVisitor cv(item);
      row.accept(cv);
      return simData::DataTable::RowVisitor::VISIT_CONTINUE;
    }
  private:
    QTreeWidget* tree_;
  };

  MainWindow::MainWindow(QWidget *parent) : QDialog(parent)
  {
    counter_ = 0;
    // create test helper
    testHelper_ = new simUtil::DataStoreTestHelper();
    simData::DataStore* ds = testHelper_->dataStore();

    // add 3 platforms
    for (int i = 0; i < 3; i++)
    {
      testHelper_->addPlatform();
    }

    // create ui
    ui_ = new Ui_MainWindow();
    ui_->setupUi(this);

    // set up entity tree view
    entityTreeModel_ = new simQt::EntityTreeModel(nullptr, ds);
    entityTreeComposite_ = ui_->EntityTreeComposite;
    entityTreeComposite_->setModel(entityTreeModel_);
    connect(entityTreeComposite_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(itemsSelected(QList<uint64_t>)));
    connect(ui_->AddDataTableButton, SIGNAL(pressed()), this, SLOT(addTable_()));
    connect(ui_->AddRowButton, SIGNAL(pressed()), this, SLOT(addRow_()));
    connect(ui_->AddColumnButton, SIGNAL(pressed()), this, SLOT(addColumn_()));
    connect(ui_->DataTableComboBox, SIGNAL(dataTableSelected(simData::DataTable*)), this, SLOT(tableSelected_(simData::DataTable*)));
    connect(ui_->RemoveTableButton, SIGNAL(pressed()), this, SLOT(removeTable_()));
    connect(ui_->DataLimitPointsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setDataLimitPoints_(int)));
    connect(ui_->DataLimitTimeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setDataLimitTime_(double)));
    connect(ui_->DataLimitEnableCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableDataLimiting_(int)));

    ui_->TableSizeSpinBox->setValue(3);

    // create our data table model, pass it to our views
    tableModel_ = new simQt::DataTableModel();
    ui_->DataTableTreeView->setModel(tableModel_);
    ui_->tableViewTest->setModel(tableModel_);
    // setting these causes bad performance with large tables
    //ui_->tableViewTest->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    //ui_->tableViewTest->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    //ui_->DataTableTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    // now set the data store for our DataTableComboBox
    ui_->DataTableComboBox->setProviders(ds);
  }

  MainWindow::~MainWindow()
  {
    // need to set providers to nullptr
    ui_->DataTableComboBox->setProviders(nullptr);
    delete ui_;
    ui_ = nullptr;
    delete entityTreeModel_;
    entityTreeModel_ = nullptr;
    delete tableModel_;
    tableModel_ = nullptr;
    delete testHelper_;
    testHelper_ = nullptr;
  }

  void MainWindow::addTable_()
  {
    QList<uint64_t> items = entityTreeComposite_->selectedItems();
    if (items.empty())
      return;
    testHelper_->addDataTable(items[0], ui_->TableSizeSpinBox->value());
    fillDataTableWidget_(ui_->DataTableComboBox->currentSelection());
  }

  void MainWindow::tableSelected_(simData::DataTable* table)
  {
    fillDataTableWidget_(table);
  }

  void MainWindow::addRow_()
  {
    simData::DataTable* table = ui_->DataTableComboBox->currentSelection();
    if (table == nullptr)
      return;
    simData::TableRow row;
    // use column visitor to get all the columns
    ColumnVisitor cv;
    table->accept(cv);
    std::vector<simData::TableColumn*> columns;
    cv.getColumns(columns);
    short val = counter_++;
    for (std::vector<simData::TableColumn*>::const_iterator iter = columns.begin(); iter != columns.end(); ++iter)
    {
      row.setValue((*iter)->columnId(), val++);
    }

    row.setTime(ui_->RowTimeSpinBox->value());
    table->addRow(row);
    fillDataTableWidget_(ui_->DataTableComboBox->currentSelection());
  }

  void MainWindow::addColumn_()
  {
    simData::DataTable* table = ui_->DataTableComboBox->currentSelection();
    if (table == nullptr)
      return;
    // give the new column a unique name
    std::ostringstream os;
    size_t colCount = table->columnCount();
    os << "New Col " << colCount;
    simData::TableColumn* column;
    table->addColumn(os.str(), simData::VT_UINT64, 0, &column);
    fillDataTableWidget_(ui_->DataTableComboBox->currentSelection());
  }

  void MainWindow::removeTable_()
  {
    simData::DataTable* table = ui_->DataTableComboBox->currentSelection();
    if (table == nullptr)
      return;
    testHelper_->dataStore()->dataTableManager().deleteTable(table->tableId());
  }

  void MainWindow::removeRow_()
  {
    // TODO
  }

  void MainWindow::removeColumn_()
  {
    // TODO
  }

  void MainWindow::itemsSelected(QList<uint64_t> ids)
  {
    if (ids.empty())
      return;
    simData::DataStore::Transaction t;
    const simData::PlatformPrefs* prefs = testHelper_->dataStore()->platformPrefs(ids[0], &t);
    ui_->PlatformNameEdit->setText(prefs->commonprefs().name().c_str());
    ui_->DataLimitPointsSpinBox->setValue(prefs->commonprefs().datalimitpoints());
    ui_->DataLimitTimeSpinBox->setValue(prefs->commonprefs().datalimittime());
    ui_->DataTableComboBox->setEntity(ids[0]);
    fillDataTableWidget_(ui_->DataTableComboBox->currentSelection());
  }

  void MainWindow::enableDataLimiting_(int enable)
  {
    testHelper_->dataStore()->setDataLimiting(enable != 0);
  }

  void MainWindow::setDataLimitPoints_(int numPoints)
  {
    QList<uint64_t> items = entityTreeComposite_->selectedItems();
    if (items.empty())
      return;
    simData::DataStore::Transaction t;
    simData::PlatformPrefs* prefs = testHelper_->dataStore()->mutable_platformPrefs(items[0], &t);
    prefs->mutable_commonprefs()->set_datalimitpoints(numPoints);
    t.commit();
  }

  void MainWindow::setDataLimitTime_(double numSeconds)
  {
    QList<uint64_t> items = entityTreeComposite_->selectedItems();
    if (items.empty())
      return;
    simData::DataStore::Transaction t;
    simData::PlatformPrefs* prefs = testHelper_->dataStore()->mutable_platformPrefs(items[0], &t);
    prefs->mutable_commonprefs()->set_datalimittime(numSeconds);
    t.commit();
  }

  void MainWindow::fillDataTableWidget_(simData::DataTable* table)
  {
    // update our DataTableModel
    tableModel_->setDataTable(table);
  }

}

//////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  DataTableViewTest::MainWindow* window = new DataTableViewTest::MainWindow(nullptr);
  window->show();

  int rv = app.exec();
  delete window;
  return rv;
}
