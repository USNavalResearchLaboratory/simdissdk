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

#ifndef SIMVIS_MEMORYDATASTORE_PLATFORMFILTER_H
#define SIMVIS_MEMORYDATASTORE_PLATFORMFILTER_H

#include "simData/DataTypes.h"
#include "simData/ObjectId.h"

namespace simVis {

/// Filters that are called after the platform's TSPI information has been updated to allow for modification
class PlatformTspiFilter;


/**
 * The manager take a platform update and passes it through a series of filters.  A filter may do nothing to the update or alter the update
 * or reject (drop) the update.  Processing goes through all filters unless a filter rejects an update.  if a filter rejects an update
 * processing stops and POINT_DROPPED is returned.  A filter sees the modifications to update of any previous filters.
 *
 * Filters are used to implement features like Altitude Clamping.  See AltitudeMinMaxClamping as an example.
 */
class PlatformTspiFilterManager
{
public:
  /** Defines various responses to the filter() virtual method */
  enum FilterResponse
  {
    POINT_UNCHANGED,
    POINT_CHANGED,
    POINT_DROPPED
  };

  /// Default constructor
  PlatformTspiFilterManager();
  virtual ~PlatformTspiFilterManager();

  /// Adds a filter; takes ownership of the memory
  void addFilter(PlatformTspiFilter* filter);

  /// Removes a filter; the caller takes ownership of the memory
  void removeFilter(PlatformTspiFilter* filter);

  /// Filters the given platform state
  virtual FilterResponse filter(simData::PlatformUpdate& update, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props);

private:
  /// Returns simCore::Coordinate based off of update
  simCore::Coordinate toCoordinate_(const simData::PlatformUpdate& update) const;

  /// Updates the position, orientation and velocity form coord to update
  void toPlatformUpdate_(const simCore::Coordinate& coord, simData::PlatformUpdate& update) const;

  /// Filters that are allowed to modified the platform state
  std::vector<PlatformTspiFilter*> platformFilters_;
};


/// Filters that are called after the platform's TSPI information has been updated to allow for modification
class PlatformTspiFilter
{
public:
  virtual ~PlatformTspiFilter() {}

  /// Returns true if the filter might modify the TSPI data; used to help avoid an ECEF/LLA conversion
  virtual bool isApplicable(const simData::PlatformPrefs& prefs) const = 0;

  /// Filters the given platform state
  virtual PlatformTspiFilterManager::FilterResponse filter(simCore::Coordinate& llaCoord, const simData::PlatformPrefs& prefs, const simData::PlatformProperties& props) = 0;
};

}

#endif

