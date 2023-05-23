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
#ifndef SIMQT_DATATABLE_MODEL_H
#define SIMQT_DATATABLE_MODEL_H

#include <memory>
#include <QList>
#include <QAbstractItemModel>
#include "simData/DataTable.h"

namespace simCore { class Units; }
namespace simUtil { class UnitTypeConverter; }

namespace simQt {

  /** A data table model based on QAbstractItemModel */
  class SDKQT_EXPORT DataTableModel : public QAbstractItemModel
  {
    Q_OBJECT
  public:

    /** Invalid time value */
    static const double INVALID_TIME;

    /// data role for obtaining raw values rather than string
    static const int SortRole = Qt::UserRole + 1;

    /**
    * Constructor, takes the DataTable the model represents
    * @param parent
    * @param dataTable data for the model
    */
    DataTableModel(QObject *parent = nullptr, simData::DataTable* dataTable = nullptr);

    /** Destructor */
    virtual ~DataTableModel();

    // from QAbstractItemModel
    /** @return number of columns in the data table */
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    /** @return data for given item */
    virtual QVariant data(const QModelIndex &index, int role) const;
    /** @return the header data for given section */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    /** @return the index for the given row and column */
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    /** @return the index of the parent of the item given by index */
    virtual QModelIndex parent(const QModelIndex &index) const;
    /** @return number of rows currently loaded in the model */
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
    * Get time associated with this index, uses row value to find time.  Returns INVALID_TIME if row index not valid
    * @param index
    * @return double time associated with index row, INVALID_TIME if row index not valid
    */
    double getTime(const QModelIndex& index) const;

    /**
    * Set the data table this model represents.  Will clear out the old data and repopulate with the
    * new data table values.  Unregisters all listeners, registers listeners with new data table.
    * @param dataTable  data for the model
    */
    void setDataTable(simData::DataTable* dataTable);

    /** Returns the current data table; can be nullptr */
    simData::DataTable* dataTable() const;

    /** Set the UnitTypeConverter to show units in the column header */
    void setUnitTypeConverter(std::shared_ptr<simUtil::UnitTypeConverter> converter);

  public Q_SLOTS:
    /** Set the number of digits after the decimal for floats and doubles */
    void setGenericPrecision(unsigned int digitsAfterDecimal);

  protected Q_SLOTS:
    /** Emits all data has changed. Use this for example if your units change. */
    void emitAllDataChanged_();
    /** Emits header display data changed. Use this if your unitsName_() returns a new value. */
    void emitAllHeaderDataChanged_();

  protected:
    /** Returns the units name for a given column, applied to QHeaderView's data(). Emit dataChanged() if this changes. */
    virtual QString unitsName_(const simData::TableColumn& col) const;
    /** Convert the DataTable cell value to a QVariant; converting float and double into strings with the correct precision */
    virtual QVariant cellDisplayValue_(const simData::TableColumn& column, simData::TableColumn::Iterator& cellIter) const;
    /** Convert the DataTable cell value to a QVariant */
    virtual QVariant cellSortValue_(const simData::TableColumn& column, simData::TableColumn::Iterator& cellIter) const;

    simData::DataTable* dataTable_ = nullptr; ///< reference to the data table this model represents
    QList<const simData::TableColumn*> columns_; ///< index in list corresponds to model column index
    QList<double> rows_; ///< index in list corresponds to model row index
    unsigned int genericPrecision_ = 3;  ///< number of digits after the decimal for floats and doubles
    std::shared_ptr<simUtil::UnitTypeConverter> unitTypeConverter_;
  };

}
#endif
