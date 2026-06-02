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
  Properties *mutable_properties() override { return &properties_; }

  /// Retrieve the data entry's properties object defining the reference frame for its state
  const Properties *properties() const override { return &properties_; }

  /// Retrieve the data entry's preferences object describing its appearance
  Preferences *mutable_preferences() override { return &preferences_; }

  /// Retrieve the data entry's preferences object describing its appearance
  const Preferences *preferences() const override { return &preferences_; }

  /// Retrieve the data entry's DataSlice containing state updates
  Updates *updates() override { return &updates_; }

  /// Retrieve the data entry's DataSlice containing state modifying commands
  Commands *commands() override { return &commands_; }

  /// Retrieve the data entry's DataSlice containing category data describing its type
  CategoryDataSlice *categoryData() override { return &categoryData_; }

  /// Retrieve the data entry's DataSlice containing user-defined state data (generic data)
  GenericDataSlice *genericData() override { return &genericData_; }

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
