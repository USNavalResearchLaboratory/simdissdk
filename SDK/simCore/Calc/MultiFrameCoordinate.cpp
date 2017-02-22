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
#include <cassert>
#include "simCore/Calc/MultiFrameCoordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

namespace simCore
{

MultiFrameCoordinate::MultiFrameCoordinate()
  : llaValid_(false),
    ecefValid_(false)
{
}

MultiFrameCoordinate::MultiFrameCoordinate(const simCore::Coordinate& coordinate)
  : llaValid_(false),
    ecefValid_(false)
{
  setCoordinate(coordinate);
}

MultiFrameCoordinate::~MultiFrameCoordinate()
{
}

int MultiFrameCoordinate::setCoordinate(const simCore::Coordinate& coordinate)
{
  if (coordinate.coordinateSystem() == simCore::COORD_SYS_LLA)
  {
    llaValid_ = true;
    ecefValid_ = false;
    llaCoordinate_ = coordinate;
    ecefCoordinate_.clear();
    return 0;
  }
  else if (coordinate.coordinateSystem() == simCore::COORD_SYS_ECEF)
  {
    llaValid_ = false;
    ecefValid_ = true;
    llaCoordinate_.clear();
    ecefCoordinate_ = coordinate;
    return 0;
  }

  // Error, not a valid system
  clear();
  return 1;
}

int MultiFrameCoordinate::setCoordinate(const simCore::Coordinate& coordinate, const simCore::CoordinateConverter& converter)
{
  // Check for case where coord system passed in is unnecessary
  if (coordinate.coordinateSystem() == simCore::COORD_SYS_ECEF || coordinate.coordinateSystem() == simCore::COORD_SYS_LLA)
    return setCoordinate(coordinate);

  // Check for invalid coordinate and invalid CC
  if (coordinate.coordinateSystem() == simCore::COORD_SYS_NONE || !converter.hasReferenceOrigin())
  {
    clear();
    // Error, invalid coordinate or converter
    return 1;
  }

  // Convert the coordinate and pass in using setCoordinate
  simCore::Coordinate llaValues;
  converter.convert(coordinate, llaValues, simCore::COORD_SYS_LLA);
  return setCoordinate(llaValues);
}

void MultiFrameCoordinate::clear()
{
  llaValid_ = false;
  ecefValid_ = false;
  llaCoordinate_.clear();
  ecefCoordinate_.clear();
}

bool MultiFrameCoordinate::isValid() const
{
  return llaValid_ || ecefValid_;
}

const simCore::Coordinate& MultiFrameCoordinate::llaCoordinate() const
{
  // Need to convert from ECEF into LLA, if ECEF is valid but LLA is not
  if (ecefValid_ && !llaValid_)
  {
    simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoordinate_, llaCoordinate_);
    llaValid_ = true;
  }
  // Return the LLA value, either valid or not
  return llaCoordinate_;
}

const simCore::Coordinate& MultiFrameCoordinate::ecefCoordinate() const
{
  // Need to convert from LLA into ECEF, if LLA is valid but ECEF is not
  if (llaValid_ && !ecefValid_)
  {
    simCore::CoordinateConverter::convertGeodeticToEcef(llaCoordinate_, ecefCoordinate_);
    ecefValid_ = true;
  }
  // Return the ECEF value, either valid or not
  return ecefCoordinate_;
}

}
