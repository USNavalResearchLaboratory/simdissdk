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

#ifndef SIMCORE_CALC_MGRS_H
#define SIMCORE_CALC_MGRS_H

#include <string>
#include "simCore/Common/Export.h"

namespace simCore {

/**
 * @brief Methods for conversion from MGRS/UTM/UPS to geodetic coordinates.
 *
 * Note: Several functions have been repurposed from software provided by the
 *       White Sands Missile Range (WSMR), GEOTRANS library and GeographicLib.
 *       Source code for GEOTRANS can be found here: http://earth-info.nga.mil/GandG/geotrans/
 *       GEOTRANS license can be found here: http://earth-info.nga.mil/GandG/geotrans/docs/MSP_GeoTrans_Terms_of_Use.pdf
 *       Source code for GeographicLib can be found here: https://sourceforge.net/projects/geographiclib/
 *       GeographicLib license can be found here: https://geographiclib.sourceforge.io/html/LICENSE.txt
 */
class SDKCORE_EXPORT Mgrs
{
public:

  /**
  * Converts an MGRS coordinate to geodetic coordinates.
  * Note that the function is currently defined only for values that convert to latitudes of less than 80 degrees
  * south or 84 degrees north, as it uses converts first to UTM (see convertUtmToGeodetic).
  * @param[in ] mgrs MGRS coordinate string
  * @param[out] lat Latitude of resulting conversion in radians
  * @param[out] lon Longitude of resulting conversion in radians
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int convertMgrsToGeodetic(const std::string& mgrs, double& lat, double& lon, std::string* err = nullptr);

  /**
  * Breaks an MGRS coordinate string into its components.
  *
  * @param[in ] mgrs MGRS coordinate string
  * @param[out] zone UTM zone, should be in the range of 1-60. If 0, then the coordinate is in UPS format.
  * @param[out] gzdLetters GZD of the UPS coordinate, minus the zone (will always be 3 characters)
  * @param[out] easting Easting portion of position within grid, output resolution of 1 meter or finer
  * @param[out] northing Northing portion of position within grid, output resolution of 1 meter or finer
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int breakMgrsString(const std::string& mgrs, int& zone, std::string& gzdLetters, double& easting,
    double& northing, std::string* err = nullptr);

  /**
  * Converts an MGRS coordinate to geodetic coordinates.
  * This is currently used for grid coordinates that would convert to latitudes of less than 80 degrees
  * south or 84 degrees north.
  *
  * @param[in ] zone UTM zone, should be in the range of 1-60
  * @param[in ] gzdLetters GZD of the UPS coordinate, minus the zone (should always be 3 characters)
  * @param[in ] mgrsEasting Easting portion of MGRS coordinate position within grid
  * @param[in ] mgrsNorthing Northing portion of MGRS coordinate position within grid
  * @param[out] northPole Pole which is the center of UPS projection (true means north, false means south)
  * @param[out] utmEasting Easting portion of resulting UTM coordinate
  * @param[out] utmNorthing Northing portion of resulting UTM coordinate
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int convertMgrsToUtm(int zone, const std::string& gzdLetters, double mgrsEasting, double mgrsNorthing,
    bool& northPole, double& utmEasting, double& utmNorthing, std::string* err = nullptr);

  /**
  * Converts a UTM coordinate to geodetic coordinates.
  * Note that the function is defined only for values that convert to latitudes of less than 80 degrees
  * south or 84 degrees north, as per the UTM standard. Basic range checking is done, but the user is
  * expected to provide valid easting and northing values.
  *
  * @param[in ] zone UTM zone, should be in the range of 1-60
  * @param[in ] northPole Pole which is the center of UPS projection (true means north, false means south)
  * @param[in ] easting Easting portion of position within grid
  * @param[in ] northing Northing portion of position within grid
  * @param[out] lat Latitude of resulting conversion in radians
  * @param[out] lon Longitude of resulting conversion in radians
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int convertUtmToGeodetic(int zone, bool northPole, double easting, double northing, double& lat, double& lon, std::string* err = nullptr);

  /**
  * Converts an MGRS coordinate to UPS coordinates.
  * This is used for grid coordinates that would convert to latitudes greater than 80 degrees south or 84
  * degrees north. UTM zone should always be 0 and is thus not passed in as a parameter to the function.
  *
  * @param[in ] gzdLetters GZD of the MGRS coordinate
  * @param[in ] mgrsEasting Easting portion of position within MGRS grid
  * @param[in ] mgrsNorthing Northing portion of position within MGRS grid
  * @param[out] northPole Pole which is the center of UPS projection (true means north, false means south)
  * @param[out] upsEasting Easting portion of position within UPS grid
  * @param[out] upsNorthing Northing portion of position within UPS grid
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int convertMgrsToUps(const std::string& gzdLetters, double mgrsEasting, double mgrsNorthing,
    bool& northPole, double& upsEasting, double& upsNorthing, std::string* err = nullptr);

  /**
  * Converts a UPS coordinate to geodetic coordinates.
  * Note that this method should only be used for grid coordinates that would convert to latitudes
  * greater than 80 degrees south or 84 degrees north, as per the UTM standard.
  *
  * @param[in ] northPole Pole which is the center of UPS projection (true means north, false means south)
  * @param[in ] easting False easting portion of position within grid
  * @param[in ] northing False northing portion of position within grid
  * @param[out] lat Latitude of resulting conversion in radians
  * @param[out] lon Longitude of resulting conversion in radians
  * @param[out] err Optional pointer to error string
  * @return 0 if conversion is successful, non-zero otherwise
  */
  static int convertUpsToGeodetic(bool northPole, double easting, double northing, double& lat, double& lon, std::string* err = nullptr);

private:

  struct Latitude_Band
  {
    /// minimum northing for latitude band
    double minNorthing;
    /// latitude band northing offset
    double northingOffset;
  };

  struct UPS_Constants
  {
    /// Grid column letter range - low value
    char gridColumnLowValue;
    /// Grid column letter range - high value
    char gridColumnHighValue;
    /// Grid row letter range - high value
    char gridRowHighValue;
    /// False easting based on grid column letter
    double falseEasting;
    /// False northing based on grid row letter
    double falseNorthing;
  };

  /*
  * Receives a latitude band letter and returns the minimum northing and northing offset for that
  * latitude band letter.
  *
  * @param[in ] bandLetter Latitude band letter
  * @param[out] minNorthing Minimum northing for the given band letter
  * @param[out] northingOffset Latitude band northing offset
  * @return 0 on valid band letter input, non-zero otherwise
  */
  static int getLatitudeBandMinNorthing_(char bandLetter, double& minNorthing, double& northingOffset);

  /*
  * Sets the letter range used for the grid zone column letter in the MGRS coordinate string, based on the
  * UTM zone number. It also sets the pattern offset using the UTM zone's pattern.
  *
  * @param[in ] zone UTM Zone number
  * @param[out] columnLetterLowValue Lower bound for the grid column letter
  * @param[out] columnLetterHighValue Upper bound for the grid column letter
  * @param[out] patternOffset Offset to the grid northing value based on the pattern of the UTM zone number
  */
  static void getGridValues_(int zone, char& columnLetterLowValue, char& columnLetterHighValue, double& patternOffset);

  /// Computes the hyperbolic arctangent of the given input.
  static double atanh_(double x);

  /// The hypotenuse function avoiding underflow and overflow.
  static double hypot_(double x, double y);

  /// Computes tan chi in terms of tan phi.
  static double taupf_(double tau);

  /// Computes tan phi in terms of tan chi.
  static double tauf_(double taup);
};

} // Namespace simCore

#endif
