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
#include <QMetaType>
#include "simData/DataStore.h"
#include "ui_DataTableComboBox.h"
#include "DataTableComboBox.h"

Q_DECLARE_METATYPE(simData::DataTable*);

namespace simQt {

/** Listens to data table add and remove events */
class DataTableComboBox::TableManagerObserver : public simData::DataTableManager::ManagerObserver
{
public:
  /** Constructor */
  explicit TableManagerObserver(DataTableComboBox* parent) : parent_(parent){}

  /** @copydoc simData::DataTableManager::ManagerObserver::onPreRemoveTable */
  virtual void onAddTable(simData::DataTable* table)
  {
    parent_->addTable_(table);
  }

  /** @copydoc simData::DataTableManager::ManagerObserver::onPreRemoveTable */
  virtual void onPreRemoveTable(simData::DataTable* table)
  {
    parent_->removeTable_(table);
  }

private:
  DataTableComboBox* parent_;
};

/** Visits the TableList and populates a QComboBox with all the data tables */
class TableListVisitor : public simData::TableList::Visitor
{
public:
  /** Constructor */
  explicit TableListVisitor(QComboBox* comboBox) : comboBox_(comboBox) {}

  /** @copydoc simData::TableList::Visitor::visit */
  virtual void visit(simData::DataTable* table)
  {
    comboBox_->addItem(table->tableName().c_str(), (unsigned long long)table);
  }

private:
  QComboBox* comboBox_;

};

DataTableComboBox::DataTableComboBox(QWidget* parent)
:QWidget(parent),
entityId_(0),
dataStore_(nullptr)
{
  ui_ = new Ui_DataTableComboBox;
  ui_->setupUi(this);

  connect(ui_->TableComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(tableSelected_(int)));
  observer_ = simData::DataTableManager::ManagerObserverPtr(new TableManagerObserver(this));
}

DataTableComboBox::~DataTableComboBox()
{
  if (dataStore_ != nullptr)
    dataStore_->dataTableManager().removeObserver(observer_);
  delete ui_;
}

simData::DataTable* DataTableComboBox::currentSelection() const
{
  if (ui_->TableComboBox->count() == 0)
    return nullptr;
  return (simData::DataTable*)(ui_->TableComboBox->itemData(ui_->TableComboBox->currentIndex()).toLongLong());
}

void DataTableComboBox::setEntity(simData::ObjectId entityId)
{
  entityId_ = entityId;
  ui_->TableComboBox->clear();
  if (dataStore_ == nullptr)
    return;
  // get the table list for all tables owned by this entity
  const simData::TableList* tableList = dataStore_->dataTableManager().tablesForOwner(entityId_);
  if (tableList == nullptr)
    return;
  // visit the table list to populate the combo box with data tables
  TableListVisitor listVisitor(ui_->TableComboBox);
  tableList->accept(listVisitor);
}

void DataTableComboBox::setProviders(simData::DataStore* dataStore)
{
  // remove observer if replacing data store
  if (dataStore_ != nullptr)
    dataStore_->dataTableManager().removeObserver(observer_);
  dataStore_ = dataStore;
  if (dataStore_ == nullptr)
    return;
  // now register new observer
  dataStore_->dataTableManager().addObserver(observer_);
  // might need to update table list
  setEntity(entityId_);
}

void DataTableComboBox::tableSelected_(int index)
{
  simData::DataTable* table = (simData::DataTable*)(ui_->TableComboBox->itemData(index).toLongLong());
  Q_EMIT(dataTableSelected(table));
}

void DataTableComboBox::addTable_(simData::DataTable* table)
{
  // add new table at the end of the list
  if (table->ownerId() == entityId_)
    ui_->TableComboBox->addItem(table->tableName().c_str(), (unsigned long long)table);
}

void DataTableComboBox::removeTable_(simData::DataTable* table)
{
  // search combo box to find table and remove it
  int numItems = ui_->TableComboBox->count();
  for (int index = 0; index < numItems; index++)
  {
    simData::DataTable* item = (simData::DataTable*)(ui_->TableComboBox->itemData(index).toLongLong());
    if (table == item)
    {
      ui_->TableComboBox->removeItem(index);
      break;
    }
  }
  if (ui_->TableComboBox->count() == 0)
    Q_EMIT(dataTableSelected(nullptr));
}

}
