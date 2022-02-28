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
#include <algorithm>
#include "simData/TableCellTranslator.h"
#include "simData/DataTable.h"

// Define NullTableCell as nullptr when available
#ifdef WIN32
#if _MSC_VER < 1600
#define NullTableCell NULL
#else
#define NullTableCell nullptr
#endif
#else
// SIMDIS-2239 - C++0x in gcc 4.4 (RHEL 6) does not provide nullptr, but errors on implicit cast of NULL to a templated type *
// C++0x in gcc 4.8/RHEL 7 does provide nullptr, so this would not be needed, same in recent Ubuntu
// unless we can detect that we have nullptr, provide the explicit correct cast of NULL
#define NullTableCell (static_cast<TableCell*>(NULL))
#endif

namespace simData
{

/**
 * Table rows are divided into zero or more table cells.  Each cell contains a single
 * value that can be set or retrieved.  This is the pure interface for a cell.  Specific
 * cells based on data type (uint16_t, string, etc.) are instantiated using the
 * template TableCellT<> class.
 */
class TableCell
{
public:
  virtual ~TableCell() {}

  /**
   * Creates a copy of the cell data.
   */
  virtual TableCell* clone() const = 0;

  /**
   * Returns true if the two cells contain the exact same type and value.  Two cells with
   * different types (e.g. int16_t and uint32_t) will return false, even if they contain
   * the same underlying value (e.g. "7").  The only time this returns true is if the underlying
   * data type is the same, AND the value is the same.
   */
  virtual bool equals(const TableCell& b) const = 0;

  ///@{
  /**
   * obtains a cell value in the given data type
   * @param val Variable into which to place the cell value.  Cell value converted to type as necessary.
   * @return Success or failure flag in TableStatus.
   */
  virtual TableStatus value(uint8_t& val) const = 0;
  virtual TableStatus value(int8_t& val) const = 0;
  virtual TableStatus value(uint16_t& val) const = 0;
  virtual TableStatus value(int16_t& val) const = 0;
  virtual TableStatus value(uint32_t& val) const = 0;
  virtual TableStatus value(int32_t& val) const = 0;
  virtual TableStatus value(uint64_t& val) const = 0;
  virtual TableStatus value(int64_t& val) const = 0;
  virtual TableStatus value(float& val) const = 0;
  virtual TableStatus value(double& val) const = 0;
  virtual TableStatus value(std::string& val) const = 0;
  ///@}

  ///@{
  /**
   * Sets a cell value in the given data type
   * @param value Variable into which to place the cell value.  Cell value converted to type as necessary.
   * @return Success or failure flag in TableStatus.
   */
  virtual TableStatus setValue(uint8_t value) = 0;
  virtual TableStatus setValue(int8_t value) = 0;
  virtual TableStatus setValue(uint16_t value) = 0;
  virtual TableStatus setValue(int16_t value) = 0;
  virtual TableStatus setValue(uint32_t value) = 0;
  virtual TableStatus setValue(int32_t value) = 0;
  virtual TableStatus setValue(uint64_t value) = 0;
  virtual TableStatus setValue(int64_t value) = 0;
  virtual TableStatus setValue(float value) = 0;
  virtual TableStatus setValue(double value) = 0;
  virtual TableStatus setValue(const std::string& value) = 0;
  ///@}

  /**
   * Helper to cleanly visit this cell from a Table Row.
   */
  virtual void accept(TableColumnId asColumnId, TableRow::CellVisitor& toVisitor) const = 0;
};

/////////////////////////////////////////////////////////////////////////////////

/**
 * Typed version of the TableCell interface stores the value with exact precision.
 */
template <typename ValueType>
class TableCellT : public TableCell
{
public:
  /** Instantiates the cell with a given value */
  explicit TableCellT(const ValueType& givenValue)
    : value_(givenValue)
  {
  }
  /** Destructor, has no dynamic memory */
  virtual ~TableCellT()
  {
  }

  // Inherits parent's documentation
  virtual TableCell* clone() const
  {
    return new TableCellT<ValueType>(value_);
  }

  // Inherits parent's documentation
  virtual bool equals(const TableCell& b) const
  {
    const TableCellT<ValueType>* cellThisType = dynamic_cast<const TableCellT<ValueType>*>(&b);
    if (cellThisType == nullptr)
      return false;
    ValueType tmpValue;
    cellThisType->value(tmpValue);
    return (tmpValue == value_);
  }

  // obtains a cell value
  virtual TableStatus value(uint8_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(int8_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(uint16_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(int16_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(uint32_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(int32_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(uint64_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(int64_t& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(float& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(double& val) const { getValue_(val); return TableStatus::Success(); }
  virtual TableStatus value(std::string& val) const { getValue_(val); return TableStatus::Success(); }

  // sets a cell value
  virtual TableStatus setValue(uint8_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(int8_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(uint16_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(int16_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(uint32_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(int32_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(uint64_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(int64_t value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(float value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(double value) { setValue_(value); return TableStatus::Success(); }
  virtual TableStatus setValue(const std::string& value) { setValue_(value); return TableStatus::Success(); }

  // Inherits parent's documentation
  virtual void accept(TableColumnId asColumnId, TableRow::CellVisitor& toVisitor) const
  {
    toVisitor.visit(asColumnId, value_);
  }

private:
  ValueType value_;

  /// Template version of getValue_() that uses TableCellTranslator to perform casting
  template <typename GivenType>
  void getValue_(GivenType& val) const
  {
    TableCellTranslator::cast(value_, val);
  }

  /// Template version of setValue_() that uses TableCellTranslator to perform casting
  template <typename GivenType>
  void setValue_(const GivenType& val)
  {
    TableCellTranslator::cast(val, value_);
  }
};

/////////////////////////////////////////////////////////////////////////////////

// Anonymous namespace for helper functions that cannot be put into classes
namespace
{
  // Private typedef to reduce templates
  typedef std::pair<TableColumnId, TableCell*> ColumnCellPair;

  /// Required for lower_bound to work
  bool operator<(const ColumnCellPair& p1, const ColumnCellPair& p2)
  {
    return p1.first < p2.first;
  }

  template <typename T>
  TableStatus getCellValue(const std::vector<ColumnCellPair>& vec, TableColumnId columnId, T& value)
  {
    std::vector<ColumnCellPair>::const_iterator i = std::lower_bound(vec.begin(), vec.end(), ColumnCellPair(columnId, NullTableCell));
    if (i == vec.end() || i->second == nullptr || i->first != columnId)
      return TableStatus::Error("Cell not found.");
    return i->second->value(value);
  }

  template <typename T>
  void setCellValue(std::vector<ColumnCellPair>& vec, TableColumnId columnId, const T& value)
  {
    // Check common case of empty or past-end
    if (vec.empty() || (columnId > vec.back().first))
    {
      vec.push_back(ColumnCellPair(columnId, new TableCellT<T>(value)));
      return;
    }

    // Search for the cell
    std::vector<ColumnCellPair>::iterator iter1 = std::lower_bound(vec.begin(), vec.end(), ColumnCellPair(columnId, NullTableCell));
    if (iter1 != vec.end() && iter1->first == columnId)
    {
      // Found an entry, cell already exists
      TableCellT<T>* cellTyped = dynamic_cast<TableCellT<T>*>(iter1->second);
      if (cellTyped != nullptr)
      {
        // existing cell is same type as cellValue, is okay to just set it
        cellTyped->setValue(value);
        return;
      }
    }

    // Create and insert the new cell
    TableCell* newCell = new TableCellT<T>(value);
    // inserts cell
    std::vector<ColumnCellPair>::iterator iter2 = vec.insert(iter1, ColumnCellPair(columnId, newCell));
    ++iter2;
    if ((iter2 != vec.end()) && (iter2->first == columnId))
    {
      delete iter2->second;
      vec.erase(iter2);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////

TableRow::TableRow()
  : time_(0.0)
{
}

TableRow::TableRow(const TableRow& copyConstr)
{
  operator=(copyConstr);
}

TableRow& TableRow::operator=(const TableRow& copy)
{
  if (this != &copy)
  {
    clear();
    time_ = copy.time_;
    for (std::vector<ColumnCellPair>::const_iterator i = copy.cells_.begin(); i != copy.cells_.end(); ++i)
    {
      cells_.push_back(std::make_pair(i->first, i->second->clone()));
    }
  }
  return *this;
}

TableRow::~TableRow()
{
  clear();
}

void TableRow::clear()
{
  for (std::vector<ColumnCellPair>::const_iterator i = cells_.begin(); i != cells_.end(); ++i)
    delete i->second;
  cells_.clear();
  time_ = 0.0;
}

void TableRow::reserve(size_t number)
{
  cells_.reserve(number);
}

double TableRow::time() const
{
  return time_;
}

void TableRow::setTime(double t)
{
  time_ = t;
}

bool TableRow::containsCell(TableColumnId columnId) const
{
  std::vector<ColumnCellPair>::const_iterator i = std::lower_bound(cells_.begin(), cells_.end(), ColumnCellPair(columnId, NullTableCell));
  return (i != cells_.end() && i->first == columnId);
}

size_t TableRow::cellCount() const
{
  return cells_.size();
}

bool TableRow::empty() const
{
  return cells_.empty();
}

TableStatus TableRow::value(TableColumnId columnId, uint8_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, int8_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, uint16_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, int16_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, uint32_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, int32_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, uint64_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, int64_t& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, float& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, double& v) const { return getCellValue(cells_, columnId, v); }
TableStatus TableRow::value(TableColumnId columnId, std::string& v) const { return getCellValue(cells_, columnId, v); }

void TableRow::setValue(TableColumnId columnId, uint8_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, int8_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, uint16_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, int16_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, uint32_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, int32_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, uint64_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, int64_t value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, float value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, double value) { setCellValue(cells_, columnId, value); }
void TableRow::setValue(TableColumnId columnId, const std::string& value) { setCellValue(cells_, columnId, value); }

void TableRow::accept(TableRow::CellVisitor& visitor) const
{
  for (std::vector<ColumnCellPair>::const_iterator i = cells_.begin(); i != cells_.end(); ++i)
  {
    i->second->accept(i->first, visitor);
  }
}

////////////////////////////////////////////////////////////////////////

}
