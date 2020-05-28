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
#ifndef SIMCORE_CALC_GOGTOGEOFENCE_H
#define SIMCORE_CALC_GOGTOGEOFENCE_H

#include <iostream>
#include <memory>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Geometry.h"

namespace simCore
{
class GeoFence;

/// Converts GOG coordinates into GeoFences
class SDKCORE_EXPORT GogToGeoFence
{
public:
  /// Vector of simCore::GeoFence
  typedef std::vector<std::shared_ptr<simCore::GeoFence> > GeoFenceVec;

  /**
  * Converts a GOG to a GeoFence. Only works
  * with the "poly" GOG keyword.
  */
  GogToGeoFence();

  /// Destructor
  virtual ~GogToGeoFence();

  /**
  * Parses a serialized GOG std::istream, generates a matching
  * Vec3String lla coordinate in radians, and creates a matching
  * GeoFence before adding each to their respective vectors.
  * @param[in ] is std::istream containing a serialized GOG
  * Returns 0 on success, 1 otherwise.
  */
  int parse(std::istream& is);

  /**
  * Fill vec with vector of simCore::Vec3String coordinates generated
  * from the ll or lla coordinates given in the converted GOG file.
  * Each simCore::Vec3String has a matching simCore::GeoFence at the
  * same index in the vector accessible by calling getFences().
  * @param[in ] vec vector to be filled
  */
  void getCoordinatesVec(std::vector<simCore::Vec3String>& vec) const;

  /**
  * Fill fences with vector of simCore::GeoFence converted from GOG
  * coordinates. Each simCore::GeoFence has a matching simCore::Vec3String
  * at the same index in the vector accessible by calling getCoordinatesVec().
  * @param[in ] fences vector to be filled
  */
  void getFences(GeoFenceVec& fences) const;

  /** Clears out internal coordinates and fences */
  void clear();

private:

  /// Parses a "start" GOG keyword
  int parseStartKeyword_(int lineNumber, bool& start) const;
  /// Parses a "poly", "polygon", or "line" GOG keyword
  int parseObjKeyword_(int lineNumber, bool& start, bool& obj) const;
  /// Parses an "end" GOG keyword
  int parseEndKeyword_(int lineNumber, std::string, bool& start, bool& obj, bool& off, std::string& name, simCore::Vec3String& coordinates);
  /// Parses a shape, called after "start" and an object keyword are found
  int parseShape_(const std::vector<std::string>& tokens, int lineNumber, std::string& name, simCore::Vec3String& coordinates) const;
  /// Parses an "ll", "lla", or "latlon" GOG keyword
  int parseLatLonAlt_(const std::vector<std::string>& tokens, int lineNumber, simCore::Vec3String& coordinates) const;

  /** Vector of all coordinate sets, one per GOG poly */
  std::vector<simCore::Vec3String> coordinatesVec_;
  /** Vector of all generated simCore::GeoFence */
  GeoFenceVec fences_;
};

}

#endif /* SIMCORE_CALC_GOGTOGEOFENCE_H */