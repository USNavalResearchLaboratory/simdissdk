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
#ifndef SIMUTIL_VIEWPOINTPOSITIONS_H
#define SIMUTIL_VIEWPOINTPOSITIONS_H

#include "osg/Matrix"
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"

namespace osg { class Node; }
namespace osgEarth {
  class SpatialReference;
  class Viewpoint;
}
namespace simVis { class View; }

namespace simUtil {

/**
 * Utility class that can be used to extract absolute geodetic (latitude, longitude, altitude)
 * positions out of a simVis::Viewpoint (i.e. osgEarth::Viewpoint).  This is particularly useful
 * because simVis::Viewpoint does not necessarily store the center or eye position in all cases.
 * When the Viewpoint is tethered to an entity node, the focal point is ignored.
 */
class SDKUTIL_EXPORT ViewpointPositions
{
public:
  /**
   * Extracts the Center LLA position out of a viewpoint. Returns Vec3 of Latitude (rad), Longitude
   * (rad), Altitude (meters).
   * @param vp Viewpoint to extract position from
   * @return LLA position of the center position (radians, radians, meters)
   */
  static simCore::Vec3 centerLla(const osgEarth::Viewpoint& vp);

  /**
   * Extracts the Eye's own LLA position out of a view.  Calculates the position based on the
   * centerLla() and the offsets (range, heading, pitch).   Returns Vec3 of Latitude (rad), Longitude
   * (rad), Altitude (meters).  Note that this returns the eye position, not the tether location.
   * This method accounts correctly for position offsets.
   * @param view View from which to extract position of the eye
   * @return LLA position of the eye position (radians, radians, meters)
   */
  static simCore::Vec3 eyeLla(const simVis::View& view);

private:
  /** Lazy initialization on static WGS-84 SRS, returning a valid SRS for WGS-84 */
  static const osgEarth::SpatialReference* wgs84_();

  /** Privately holds an SRS for WGS84 to avoid repeated calls to the SRS factory. */
  static const osgEarth::SpatialReference* wgs84Srs_;
};

}

#endif /* SIMUTIL_VIEWPOINTPOSITIONS_H */
