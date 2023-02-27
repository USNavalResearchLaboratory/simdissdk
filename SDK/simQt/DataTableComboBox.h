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
#ifndef SIMQT_DATATABLE_COMBOBOX_H
#define SIMQT_DATATABLE_COMBOBOX_H

#include <QWidget>
#include "simData/DataTable.h"

class Ui_DataTableComboBox;
namespace simData {
  class DataStore;
  class DataTable;
}

namespace simQt {

/**
* Combo Box for selecting a data table
*/
class SDKQT_EXPORT DataTableComboBox : public QWidget
{
Q_OBJECT;
public:
  /** Constructor */
  explicit DataTableComboBox(QWidget* parent = nullptr);

  /** Destructor */
  virtual ~DataTableComboBox();

  /**
  * Get the currently selected data table
  * @return simData::DataTable
  */
  simData::DataTable* currentSelection() const;

  /**
  * Set the owner of the data tables to display
  * @param entityId
  */
  void setEntity(simData::ObjectId entityId);

  /**
  * Set the reference to the data store, which will update the combo box based on changes to the data store
  * @param dataStore
  */
  void setProviders(simData::DataStore* dataStore);

Q_SIGNALS:
  /**
  * Emitted when a new table is selected.  Will pass nullptr if combo box becomes empty
  * @param table  current selected table
  */
  void dataTableSelected(simData::DataTable* table);

private Q_SLOTS:
  /** Called when the combo box selection has changed */
  void tableSelected_(int index);

private:
  /** Observer for data table add/remove events */
  class TableManagerObserver;

  /** Add a table to the combo box list */
  void addTable_(simData::DataTable* table);
  /** Remove a table from the combo box list */
  void removeTable_(simData::DataTable* table);

  simData::ObjectId entityId_; ///< id of the data table owner
  simData::DataStore* dataStore_; ///< reference to the data store
  Ui_DataTableComboBox* ui_; ///< display
  simData::DataTableManager::ManagerObserverPtr observer_; ///< table manager observer

};

}

#endif
