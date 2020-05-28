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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMDATA_DATAENTRY_H
#define SIMDATA_DATAENTRY_H

#include "simData/DataSlice.h"

/// container for classes relating to data storage
namespace simData
{
class CategoryDataSlice;
class GenericDataSlice;

/// Aggregate of platform attributes to be used internally by DataStore implementations
template<typename Properties, typename Preferences, typename Updates, typename Commands>
class DataEntry
{
public:
  virtual ~DataEntry() { }

  /// Retrieve the data entry's properties object defining the reference frame for its state
  virtual Properties *mutable_properties() = 0;

  /// Retrieve the data entry's properties object defining the reference frame for its state
  virtual const Properties *properties() const = 0;

  /// Retrieve the data entry's preferences object describing its appearance
  virtual Preferences *mutable_preferences() = 0;

  /// Retrieve the data entry's preferences object describing its appearance
  virtual const Preferences *preferences() const = 0;

  /// Retrieve the data entry's DataSlice containing state updates
  virtual Updates *updates() = 0;

  /// Retrieve the data entry's DataSlice containing state modifying commands
  virtual Commands *commands() = 0;

  /// Retrieve the data entry's DataSlice containing category data describing its type
  virtual CategoryDataSlice *categoryData() = 0;

  /// Retrieve the data entry's DataSlice containing user-defined state data (generic data)
  virtual GenericDataSlice *genericData() = 0;
}; // End of class DataEntry<>

} // namespace simData

#endif
