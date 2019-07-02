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

#ifndef SIMCORE_CALC_GARS_H
#define SIMCORE_CALC_GARS_H

#include <string>
#include "simCore/Common/Export.h"

namespace simCore {

/**
 * Methods for conversion from Global Area Reference System (GARS) coordinates
 * to geodetic coordinates, and vice versa.
 */
class SDKCORE_EXPORT Gars
{
public:

  /** Level of detail used when converting to a GARS coordinate. */
  enum Level {
    GARS_30, // 30 minute level, 5 character GARS coordinate
    GARS_15, // 15 minute level, 6 character GARS coordinate
    GARS_5 // 5 minute level, 7 character GARS coordinate
  };

  /**
   * Converts a GARS coordinate to geodetic coordinates. The resulting latitude/longitude
   * will be the southwest corner of the specified GARS coordinate.
   * @param[in ] gars GARS coordinate string
   * @param[out] lat Latitude of resulting conversion in radians
   * @param[out] lon Longitude of resulting conversion in radians
   * @param[out] err Optional pointer to error string
   * @return 0 if conversion is successful, non-zero otherwise
   */
  static int convertGarsToGeodetic(const std::string& gars, double& latRad, double& lonRad, std::string* err = NULL);

  /**
   * Converts geodetic goordinates to a GARS coordinate.
   * @param[in ] lat Latitude to convert in radians
   * @param[in ] lon Longitude to convert in radians
   * @param[out] gars Resulting GARS coordinate string
   * @param[in ] level Optional level of detail used when converting
   * @param[out] err Optional pointer to error string
   * @return 0 if conversion is successful, non-zero otherwise
   */
  static int convertGeodeticToGars(double latRad, double lonRad, std::string& gars, Level level = GARS_5, std::string* err = NULL);
};

}

#endif
