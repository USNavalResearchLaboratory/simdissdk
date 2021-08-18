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
#ifndef SIMDATA_MEMORYTABLE_DATACONTAINER_H
#define SIMDATA_MEMORYTABLE_DATACONTAINER_H

#include "simData/DataTable.h"

namespace simData { namespace MemoryTable {

/** Base class for a container that allows insert, replace, and gets based on index */
class DataContainer
{
public:
  virtual ~DataContainer() {}

  /**@name Data Container insert() methods
   * @{
   */
  /// Insert item into the data container at the given position
  virtual void insert(size_t position, uint8_t value) = 0;
  virtual void insert(size_t position, int8_t value) = 0;
  virtual void insert(size_t position, uint16_t value) = 0;
  virtual void insert(size_t position, int16_t value) = 0;
  virtual void insert(size_t position, uint32_t value) = 0;
  virtual void insert(size_t position, int32_t value) = 0;
  virtual void insert(size_t position, uint64_t value) = 0;
  virtual void insert(size_t position, int64_t value) = 0;
  virtual void insert(size_t position, float value) = 0;
  virtual void insert(size_t position, double value) = 0;
  virtual void insert(size_t position, const std::string& value) = 0;
  ///@}

  /**@name Data Container replace() methods
   * @{
   */
  /// Replace item in the data container at the given position
  virtual TableStatus replace(size_t position, uint8_t value) = 0;
  virtual TableStatus replace(size_t position, int8_t value) = 0;
  virtual TableStatus replace(size_t position, uint16_t value) = 0;
  virtual TableStatus replace(size_t position, int16_t value) = 0;
  virtual TableStatus replace(size_t position, uint32_t value) = 0;
  virtual TableStatus replace(size_t position, int32_t value) = 0;
  virtual TableStatus replace(size_t position, uint64_t value) = 0;
  virtual TableStatus replace(size_t position, int64_t value) = 0;
  virtual TableStatus replace(size_t position, float value) = 0;
  virtual TableStatus replace(size_t position, double value) = 0;
  virtual TableStatus replace(size_t position, const std::string& value) = 0;
  ///@}

  /**@name Data Container getValue() methods
   * @{
   */
  /// Retrieve the item in the data container at the given position
  virtual TableStatus getValue(size_t position, uint8_t& value) const = 0;
  virtual TableStatus getValue(size_t position, int8_t& value) const = 0;
  virtual TableStatus getValue(size_t position, uint16_t& value) const = 0;
  virtual TableStatus getValue(size_t position, int16_t& value) const = 0;
  virtual TableStatus getValue(size_t position, uint32_t& value) const = 0;
  virtual TableStatus getValue(size_t position, int32_t& value) const = 0;
  virtual TableStatus getValue(size_t position, uint64_t& value) const = 0;
  virtual TableStatus getValue(size_t position, int64_t& value) const = 0;
  virtual TableStatus getValue(size_t position, float& value) const = 0;
  virtual TableStatus getValue(size_t position, double& value) const = 0;
  virtual TableStatus getValue(size_t position, std::string& value) const = 0;
  ///@}

  /** Copies the contents of a given position into a row at cell position whichCell */
  virtual TableStatus copyToRowCell(TableRow& row, simData::TableColumnId whichCell, size_t position) const = 0;

  /** Removes the elements starting at the given index */
  virtual void erase(size_t position, size_t number = 1) = 0;
  /** Number of items inside the data container */
  virtual size_t size() const = 0;
  /** True if the number of items is 0 */
  virtual bool empty() const = 0;
  /** Removes all items in the data container */
  virtual void clear() = 0;
};

}}

#endif /* SIMDATA_MEMORYTABLE_DATACONTAINER_H */
