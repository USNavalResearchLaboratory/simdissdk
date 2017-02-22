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
#ifndef SIMDATA_MEMORYDATAENTRY_H
#define SIMDATA_MEMORYDATAENTRY_H

#include "simData/DataEntry.h"
#include "simData/MemoryDataSlice.h"
#include "simData/MemoryGenericDataSlice.h"
#include "simData/CategoryData/MemoryCategoryDataSlice.h"

/// container for classes relating to data storage
namespace simData
{

/// Aggregate of platform attributes to be used internally by DataStore implementations
template<typename Properties, typename Preferences, typename Updates, typename Commands>
class MemoryDataEntry : public DataEntry<Properties, Preferences, Updates, Commands>
{
public:
  /// Retrieve the data entry's properties object defining the reference frame for its state
  virtual Properties *mutable_properties() { return &properties_; }

  /// Retrieve the data entry's properties object defining the reference frame for its state
  virtual const Properties *properties() const { return &properties_; }

  /// Retrieve the data entry's preferences object describing its appearance
  virtual Preferences *mutable_preferences() { return &preferences_; }

  /// Retrieve the data entry's preferences object describing its appearance
  virtual const Preferences *preferences() const { return &preferences_; }

  /// Retrieve the data entry's DataSlice containing state updates
  virtual Updates *updates() { return &updates_; }

  /// Retrieve the data entry's DataSlice containing state modifying commands
  virtual Commands *commands() { return &commands_; }

  /// Retrieve the data entry's DataSlice containing category data describing its type
  virtual CategoryDataSlice *categoryData() { return &categoryData_; }

  /// Retrieve the data entry's DataSlice containing user-defined state data (generic data)
  virtual GenericDataSlice *genericData() { return &genericData_; }

private:
  Properties              properties_;
  Preferences             preferences_;
  Updates                 updates_;
  Commands                commands_;
  MemoryCategoryDataSlice categoryData_;
  MemoryGenericDataSlice  genericData_;
}; // End of class MemoryDataEntry<>

} // namespace simData

#endif
