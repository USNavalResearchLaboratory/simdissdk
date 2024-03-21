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
#ifndef SIMUTIL_UNITTYPECONVERTER_H
#define SIMUTIL_UNITTYPECONVERTER_H

#include <memory>
#include "osgEarth/Version"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"

#if OSGEARTH_SOVERSION >= 151
namespace osgEarth { class UnitsType; }
#else
namespace osgEarth {
  class Units;
  using UnitsType = Units;
}
#endif

namespace simCore { class Units; }

namespace simUtil {

/** Provides a conversion between osgEarth, simCore, and simData unit types */
class SDKUTIL_EXPORT UnitTypeConverter
{
public:
  UnitTypeConverter();
  virtual ~UnitTypeConverter();

  /** Returns an osgEarth::Units mapped from the simCore::Units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarth(const simCore::Units& core) const;
  /** Returns an osgEarth::Units mapped from the simData units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarth(simData::ElapsedTimeFormat data) const;
  /** Returns an osgEarth::Units mapped from the simData units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarth(simData::AngleUnits data) const;
  /** Returns an osgEarth::Units mapped from the simData units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarth(simData::DistanceUnits data) const;
  /** Returns an osgEarth::Units mapped from the simData units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarth(simData::SpeedUnits data) const;
  /** Returns an osgEarth::Units mapped from the simData units provided; empty string name on error. */
  const osgEarth::UnitsType& toOsgEarthFromData(int data) const;

  /** Returns a simCore::Units mapped from the osgEarth::Units provided; !isValid() on error. */
  const simCore::Units& toCore(const osgEarth::UnitsType& osg) const;
  /** Returns a simCore::Units mapped from the simData::ElapsedTimeFormat provided; !isValid() on error, but should not error. */
  const simCore::Units& toCore(simData::ElapsedTimeFormat data) const;
  /** Returns a simCore::Units mapped from the simData::AngleUnits provided; !isValid() on error, but should not error. */
  const simCore::Units& toCore(simData::AngleUnits data) const;
  /** Returns a simCore::Units mapped from the simData::DistanceUnits provided; !isValid() on error, but should not error. */
  const simCore::Units& toCore(simData::DistanceUnits data) const;
  /** Returns a simCore::Units mapped from the simData::SpeedUnits provided; !isValid() on error, but should not error. */
  const simCore::Units& toCore(simData::SpeedUnits data) const;
  /** Returns a simCore::Units mapped from the simData units provided; !isValid() on error, but should not error. */
  const simCore::Units& toCoreFromData(int data) const;

  /** Returns an simData units value mapped from the osgEarth::Units provided; 0 (CU_UNKNOWN) on error. */
  int toData(const osgEarth::UnitsType& osg) const;
  /** Returns an simData units value mapped from the simCore::Units provided; 0 (CU_UNKNOWN) on error. */
  int toData(const simCore::Units& core) const;

  /** Convenience to add a new mapping */
  void addMapping(const osgEarth::UnitsType& osg, const simCore::Units& core, int data);
  /** Convenience to add a new mapping without valid data enum */
  void addMapping(const osgEarth::UnitsType& osg, const simCore::Units& core);
  /** Convenience to add a new mapping without valid osgEarth enum */
  void addMapping(const simCore::Units& core, int data);
  /** Convenience to add a new mapping without valid simCore enum */
  void addMapping(const osgEarth::UnitsType& osg, int data);

  /** Helper method to determine if a given unit is registered with this system.  Useful for unit tests. */
  bool isRegistered(const simCore::Units& units) const;

private:
  class LookupHelper;
  std::unique_ptr<LookupHelper> helper_;
};

}

#endif /* SIMUTIL_UNITTYPECONVERTER_H */
