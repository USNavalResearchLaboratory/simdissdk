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
#ifndef SIMCORE_CALC_GOGTOGEOFENCE_H
#define SIMCORE_CALC_GOGTOGEOFENCE_H

#include <iosfwd>
#include <memory>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/GeoFence.h"

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
  * with the "line" and "poly" GOG keywords.
  */
  GogToGeoFence();

  /// Destructor
  virtual ~GogToGeoFence();

  /**
  * Parses a serialized GOG std::istream, creates
  * GeoFences from the GOGs and adds them to the fences vectors.
  * @param is  std::istream containing a serialized GOG
  * @param gogFileName  identifies the source GOG file or shape group
  * Returns 0 on success, 1 otherwise.
  */
  int parse(std::istream& is, const std::string& gogFileName = "");

  /**
  * Fills fences with vector of simCore::GeoFence converted from GOG coordinates.
  * @param fences vector to be filled
  */
  void getFences(GeoFenceVec& fences) const;

  /** Clears out internal coordinates and fences */
  void clear();

private:
  /**
  * Convert gog coordinates into a geoFence, test for validity, and on success add it to the GeoFenceVec.
  * @param name  name of the original GOG shape, used for error reporting
  * @param coordinates  vector of coordinates to be converted to a geofence
  * Returns 0 on success, 1 otherwise.
  */
  int generateGeoFence_(const std::string& name, const Vec3String& coordinates);

  /** Vector of all generated simCore::GeoFence */
  GeoFenceVec fences_;
};

}

#endif /* SIMCORE_CALC_GOGTOGEOFENCE_H */
