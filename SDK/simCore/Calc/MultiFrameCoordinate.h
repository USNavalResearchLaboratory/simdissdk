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
#ifndef SIMCORE_CALC_MULTIFRAMECOORDINATE_H
#define SIMCORE_CALC_MULTIFRAMECOORDINATE_H

#include "simCore/Common/Export.h"
#include "simCore/Calc/Coordinate.h"

namespace simCore
{

class CoordinateConverter;

/**
 * Responsible for caching a single coordinate in multiple coordinate frames.  For
 * example, an LLA coordinate could be set, and the ECEF coordinate can be generated
 * on the fly when required, and cached, with both the LLA and ECEF coordinate
 * representing the same posit.  The intent is to cache both to prevent multiple
 * calculations.
 *
 * This class uses simCore::Coordinate extensively and is meant to replace/augment
 * simCore::Coordinate when it is known that both ECEF and LLA frames will be needed.
 */
class SDKCORE_EXPORT MultiFrameCoordinate
{
public:
  /** Constructs an empty coordinate */
  MultiFrameCoordinate();
  /** Construct from an LLA or ECEF coordinate */
  explicit MultiFrameCoordinate(const simCore::Coordinate& coordinate);

  /** Virtual destructor cleans up memory */
  virtual ~MultiFrameCoordinate();

  /**
   * Sets to a coordinate in LLA or ECEF space.  ECI and tangent plane coordinates are not permitted.
   * The call to coordinate() will return this coordinate value.
   * @param coordinate Coordinate in LLA or ECEF space
   * @return 0 on successful set; non-zero on error (i.e. not LLA or ECEF)
   */
  int setCoordinate(const simCore::Coordinate& coordinate);
  /**
   * Sets to a given coordinate, using the provided Coordinate Converter for ECI and tangent transforms.
   * The call to coordinate() will return this coordinate value only if coordinate is in a LLA or ECEF
   * frame.  Otherwise, an LLA or ECEF position is calculated and that is returned by coordinate().
   * @param coordinate Any coordinate in any valid coordinate system
   * @param converter Initialized coordinate converter to make an LLA or ECEF value from coordinate
   * @return 0 on successful set; non-zero on error (i.e. NONE system or invalid converter)
   */
  int setCoordinate(const simCore::Coordinate& coordinate, const simCore::CoordinateConverter& converter);

  /** Clears the coordinate. (Cleared coordinates are !isValid()) */
  void clear();
  /** Returns true if the coordinate is valid. */
  bool isValid() const;

  /**
   * Retrieves the loaded coordinate in LLA frame.  If !isValid(), return an empty Coordinate with
   * its coordinate system set to NONE.  May need to calculate LLA and update the cache.
   */
  const simCore::Coordinate& llaCoordinate() const;

  /**
   * Retrieves the loaded coordinate in ECEF frame.  If !isValid(), return an empty Coordinate with
   * its coordinate system set to NONE.  May need to calculate ECEF and update the cache.
   */
  const simCore::Coordinate& ecefCoordinate() const;

private:

  // Note that the following variables are mutable because the cache functions
  // llaCoordinate() and ecefCoordinate() might need to update them (using lazy update)

  mutable simCore::Coordinate llaCoordinate_;
  mutable bool llaValid_;
  mutable simCore::Coordinate ecefCoordinate_;
  mutable bool ecefValid_;

  /** Enumerates the state of lla and ECEF values */
  enum UserValue
  {
    USERSET_NONE,
    USERSET_LLA,
    USERSET_ECEF
  };
};

}

#endif /* SIMCORE_CALC_MULTIFRAMECOORDINATE_H */
