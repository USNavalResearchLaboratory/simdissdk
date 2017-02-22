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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <limits>
#include "simCore/Calc/Math.h"
#include "DataTableModel.h"

namespace simQt {

const double DataTableModel::INVALID_TIME = std::numeric_limits<double>::max();
const QVariant EMPTY_CELL = QVariant("NULL");

// Number of rows to load into the model in increments
static const int ROWLOADINCREMENT = 50;

// Increment for time values
static const double EPSILON = 1e-7;

/// Visits all columns of a table and populates a QList with column ptrs
class ColumnTimeValueAccumulator : public simData::DataTable::ColumnVisitor
{
public:
  /** Constructor */
  ColumnTimeValueAccumulator()
  {
  }

  virtual void visit(simData::TableColumn* column)
  {
    // just add columns onto our vector, they will be in order
    columns_.push_back(column);
  }

  /** Returns the columns in the data table */
  const QList<const simData::TableColumn*>& columns() const { return columns_; }

private:
  QList<const simData::TableColumn*> columns_; ///< all the TableColumn ptrs
};

/**
* Visits all rows in a table, populates the QList with the row time values, starting at first row visited until
* it has visited the number of rows specified by numRows
*/
class RowValueAccumulator : public simData::DataTable::RowVisitor
{
public:
  /** Constructor */
  explicit RowValueAccumulator(QList<double>& rows)
    : rows_(rows)
  {
  }

  virtual VisitReturn visit(const simData::TableRow& row)
  {
    // add rows in the order they exist in the table, will be time ordered
    rows_.push_back(row.time());
    return simData::DataTable::RowVisitor::VISIT_CONTINUE;
  }

private:
  QList<double>& rows_; ///< all the row time values
};

//----------------------------------------------------------------------------
DataTableModel::DataTableModel(QObject *parent, simData::DataTable* dataTable)
:QAbstractItemModel(parent),
dataTable_(NULL)
{
  setDataTable(dataTable);
}

DataTableModel::~DataTableModel()
{
}

QVariant DataTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || dataTable_ == NULL)
    return QVariant();
  if (!(columns_.size() > index.column()) || !(rows_.size() > index.row()))
    return QVariant();

  // what time are we looking for
  const double time = rows_.at(index.row());

  if (role == Qt::DisplayRole)
  {
    // col 0 is a special case, we just return the time value.
    if (index.column() == 0)
    {
      // TODO: format time string here
      QString timeString = QString("%1").arg(time, 0, 'f', 3);
      return QVariant(timeString);
    }

    const simData::TableColumn* col = columns_[index.column()];
    simData::TableColumn::Iterator cell = col->findAtOrBeforeTime(time);
    if (!cell.hasNext())
      return EMPTY_CELL;

    // return NULL if we found no data at this time
    if (cell.peekNext()->time() != time)
      return EMPTY_CELL;

    return cellDisplayValue_(col->variableType(), cell);
  }

  if (role == SortRole)
  {
    // col 0 is a special case, we just return the time value.
    if (index.column() == 0)
    {
      return QVariant(time);
    }

    const simData::TableColumn* col = columns_[index.column()];
    simData::TableColumn::Iterator cell = col->findAtOrBeforeTime(time);
    if (!cell.hasNext())
      return EMPTY_CELL;

    // return NULL if we found no data at this time
    if (cell.peekNext()->time() != time)
      return EMPTY_CELL;

    return cellSortValue_(col->variableType(), cell);
  }

  if (role == Qt::TextAlignmentRole)
  {
    // column 0 is time string, left align
    if (index.column() == 0)
      return Qt::AlignLeft;
    const simData::TableColumn* col = columns_[index.column()];
    simData::TableColumn::Iterator cell = col->findAtOrBeforeTime(time);
    // this is a NULL block, left align
    if (cell.next()->time() != time)
      return Qt::AlignLeft;

    // everything else is right aligned
    return Qt::AlignRight;
  }
  // TODO: will we do tool tips eventually?

  return QVariant();
}

QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (section >= 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    // column 0 is special case, time column
    if (section == 0)
      return "Time";

    if (columns_.size() <= section)
      return EMPTY_CELL;

    const simData::TableColumn* col = columns_[section];
    return QVariant(col->name().c_str());
  }

  // Isn't the bar across the top -- fall back to whatever QAIM does
  return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex DataTableModel::index(int row, int column, const QModelIndex &parent) const
{
  // return invalid index if we don't have this row/column
  if (parent != QModelIndex() || row < 0 || column < 0 ||
      row >= static_cast<int>(rows_.size()) || column >= static_cast<int>(columns_.size()))
    return QModelIndex();

  // no hierarchy in the model, just return an index with the specified row/column
  return createIndex(row, column);
}

QModelIndex DataTableModel::parent(const QModelIndex &index) const
{
  // no hierarchy in the model, just return a default model index
  return QModelIndex();
}

int DataTableModel::columnCount(const QModelIndex & parent) const
{
  return parent == QModelIndex() ? columns_.size() : 0;
}

int DataTableModel::rowCount(const QModelIndex & parent) const
{
  return parent == QModelIndex() ? rows_.size() : 0;
}

double DataTableModel::getTime(const QModelIndex& index) const
{
  if (rows_.size() > index.row())
    return rows_.at(index.row());
  return INVALID_TIME;
}

void DataTableModel::setDataTable(simData::DataTable* dataTable)
{
  // clear out our local references to the DataTable
  // TODO: See SIMSDK-402: This function needs some TLC ASAP
  beginResetModel();
  columns_.clear();
  rows_.clear();

  dataTable_ = dataTable;

  // no table, update layout and return
  if (dataTable_ == NULL)
  {
    endResetModel();
    return;
  }

  // update rows/columns

  // fill in columns vector
  // first column is time, no TableColumn ptr
  ColumnTimeValueAccumulator cv;
  dataTable_->accept(cv);
  // empty table, nothing more to do
  if (cv.columns().empty())
    return;

  // use size() instead of size() - 1 because of the time column
  const int lastColIndex = cv.columns().size();
  beginInsertColumns(QModelIndex(), 0, lastColIndex);
  columns_.push_back(NULL); // time column
  columns_ += cv.columns();
  endInsertColumns();

  // Add rows
  RowValueAccumulator rvc(rows_);
  dataTable_->accept(0, std::numeric_limits<double>::max(), rvc);

  // force an update now
  endResetModel();
}

simData::DataTable* DataTableModel::dataTable() const
{
  return dataTable_;
}

QVariant DataTableModel::cellDisplayValue_(simData::VariableType type, simData::TableColumn::Iterator& cell) const
{
  if (!cell.hasNext())
    return QVariant();

  switch (type)
  {
  case simData::VT_UINT8:
    {
      uint8_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_UINT16:
    {
      uint16_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_UINT32:
    {
      uint32_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_UINT64:
    {
      uint64_t val;
      cell.next()->getValue(val);
      return QVariant(static_cast<unsigned long long>(val));
    }
  case simData::VT_INT8:
    {
      int8_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_INT16:
    {
      int16_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_INT32:
    {
      int32_t val;
      cell.next()->getValue(val);
      return QVariant(val);
    }
  case simData::VT_INT64:
    {
      int64_t val;
      cell.next()->getValue(val);
      return QVariant(static_cast<long long>(val));
    }
  case simData::VT_FLOAT:
    {
      float val;
      cell.next()->getValue(val);

      if (std::isnan(val))
        return QVariant(QString::fromStdString("NaN"));

      if (std::isinf(val))
      {
        if (val > 0)
          return QVariant(QString::fromStdString("Infinity"));
        return QVariant(QString::fromStdString("-Infinity"));
      }

      return QVariant(QString::number(val, 'f', 3));
    }
  case simData::VT_DOUBLE:
    {
      double val;
      cell.next()->getValue(val);

      if (std::isnan(val))
        return QVariant(QString::fromStdString("NaN"));

      if (std::isinf(val))
      {
        if (val > 0)
          return QVariant(QString::fromStdString("Infinity"));
        return QVariant(QString::fromStdString("-Infinity"));
      }

      return QVariant(QString::number(val, 'f', 3));
    }
  case simData::VT_STRING:
    {
      std::string val;
      cell.next()->getValue(val);
      return QVariant(val.c_str());
    }
  default:
    assert(0);
    return QVariant();
  }
  return QVariant();
}

QVariant DataTableModel::cellSortValue_(simData::VariableType type, simData::TableColumn::Iterator& cell) const
{
  if (!cell.hasNext())
    return QVariant();

  if (type == simData::VT_FLOAT)
  {
    float val;
    cell.next()->getValue(val);
    return QVariant(val);
  }
  if (type == simData::VT_DOUBLE)
  {
    double val;
    cell.next()->getValue(val);
    return QVariant(val);
  }

  return cellDisplayValue_(type, cell);
}
}

