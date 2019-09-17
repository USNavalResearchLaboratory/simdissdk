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
   * Validates a given GARS coordinate string. Optionally extracts useful pieces of the GARS coordinate.
   * @param[in ] gars GARS coordinate string to validate
   * @param[out] err Optional pointer to error string
   * @param[out] lonBand Optional int pointer set to longitude band of the GARS coordinate on success
   * @param[out] latPimaryIdx Optional int pointer set to index [0, 14] of the primary latitudinal band letter 
   * @param[out] latSecondaryIdx Optional int pointer set to index [0, 24] of the secondary latitudinal band letter
   * @param[out] quad15 Optional int pointer set to 15 minute quadrant specified in the GARS coordinate, if available
   * @param[out] key5 Optional int pointer set to 5 minute key specified in the GARS coordinate, if available
   * @return true if valid GARS coordinate string, false otherwise
   */
  static bool isValidGars(const std::string& gars, std::string* err = NULL, int* lonBand = NULL, int* latPimaryIdx = NULL, int* latSecondaryIdx = NULL, int* quad15 = NULL, int* key5 = NULL);

  /**
   * Converts a GARS coordinate to geodetic coordinates. The resulting latitude/longitude
   * will be the southwest corner of the specified GARS coordinate.
   * @param[in ] gars GARS coordinate string
   * @param[out] latRad Latitude of resulting conversion in radians
   * @param[out] lonRad Longitude of resulting conversion in radians
   * @param[out] err Optional pointer to error string
   * @return 0 if conversion is successful, non-zero otherwise
   */
  static int convertGarsToGeodetic(const std::string& gars, double& latRad, double& lonRad, std::string* err = NULL);

  /**
   * Converts geodetic goordinates to a GARS coordinate.
   * @param[in ] latRad Latitude to convert in radians
   * @param[in ] lonRad Longitude to convert in radians
   * @param[out] gars Resulting GARS coordinate string
   * @param[in ] level Optional level of detail used when converting
   * @param[out] err Optional pointer to error string
   * @return 0 if conversion is successful, non-zero otherwise
   */
  static int convertGeodeticToGars(double latRad, double lonRad, std::string& gars, Level level = GARS_5, std::string* err = NULL);
};

}

#endif
