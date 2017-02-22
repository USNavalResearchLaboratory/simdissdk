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
#ifndef SIMUTIL_RECENTEREYEONAREA_H
#define SIMUTIL_RECENTEREYEONAREA_H

#include "osg/observer_ptr"
#include "osgEarth/GeoData"
#include "simCore/Common/Export.h"

namespace simVis {
  class View;
  class FocusManager;
}

namespace simUtil
{

/**
 * Helper class that can quickly recenter the current focus view on an
 * area defined by min/max lat/lon boundaries.
 */
class SDKUTIL_EXPORT RecenterEyeOnArea
{
public:
  /**
   * Initialize, optionally with a view to use when recentering.
   * @param view Inset view to use when updating viewpoints.  If NULL, then setView() must be called before recentering.
   */
  RecenterEyeOnArea(simVis::View* view=NULL);
  /**
   * Initialize using the current focus of a FocusManager for recentering.
   * @param focusManager Focus manager whose currently focused inset view will be used for updating viewpoints.
   */
  RecenterEyeOnArea(const simVis::FocusManager& focusManager);
  virtual ~RecenterEyeOnArea();

  /**
   * Changes the inset view to be used when updating viewpoints.  An inset view must be set
   * before calling methods like centerOn().
   * @param view Inset to update when calling centerOn().
   */
  void setView(simVis::View* view);

  /**
   * Changes the inset view to be used when updating viewpoints by using the current focus of a FocusManager.
   * An inset view must be set before calling methods like centerOn().
   * @param focusManager Focus manager whose currently focused inset view will be used for updating viewpoints.
   */
  void setView(const simVis::FocusManager& focusManager);

  /**
   * Changes the clamped minimum/maximum distances permitted for the eye.  The eye distance in centerOn()
   * is set based on the distance between the area corners, then clamped between these values.
   * @param clampAbove Minimum value for eye range, in meters
   * @param clampBelow Maximum value for eye range, in meters
   */
  void setRangeClamp(double clampAbove, double clampBelow);

  /**
   * Changes eye position to center on the area described, returns 0 on success; values in radians.  The
   * view specified by an earlier call to setView() will be updated so that a reasonable portion of the
   * area is visible in the view.  Lat/lon value variant of centerOn().
   * @param lowerLat Minimum latitude (south) of the area of interest, in radians
   * @param upperLat Maximum latitude (north) of the area of interest, in radians
   * @param leftLon Minimum longitude (west) of the area of interest, in radians.  Is the west/left
   *   longitude.  If set greater than rightLon, then calculation crosses antimeridian (dateline).
   * @param rightLon Maximum longitude (east) of the area of interest, in radians
   * @param transitionSec Duration of the viewpoint transition in seconds
   * @return 0 on success, non-zero on error (including no view set)
   */
  int centerOn(double lowerLat, double upperLat, double leftLon, double rightLon, double transitionSec=0.0);

  /**
   * Changes eye position to center on the area described, returns 0 on success.  The view specified
   * by an earlier call to setView() will be updated so that a reasonable portion of the are is visible
   * in the view.  DataExtentList variant of centerOn().
   * @param extents Vector of data extents.  Typically these values are retrieved from a Tile Source or
   *    Model Source associated with an Image, Elevation, or Model layer.  This should be a non-empty
   *    vector of data extents that can be converted to WGS-84.
   * @param transitionSec Duration of the viewpoint transition in seconds
   * @return 0 on success, non-zero on error (including no view set, and no valid extents)
   */
  int centerOn(const osgEarth::DataExtentList& extents, double transitionSec=0.0);

  /**
   * Changes eye position to center on the area described, returns 0 on success.  The view specified
   * by an earlier call to setView() will be updated so that a reasonable portion of the are is visible
   * in the view.  GeoExtent variant of centerOn().
   * @param extent Geographic boundary in WGS-84 to center the eye on.  Expected to be in WGS-84 with
   *    the various west/east/north/south values in degrees, as is osgEarth style.
   * @param transitionSec Duration of the viewpoint transition in seconds
   * @return 0 on success, non-zero on error (including no view set, and invalid extent)
   */
  int centerOn(const osgEarth::GeoExtent& extent, double transitionSec=0.0);

private:
  /// Returns 0 on successful conversion to WGS84-based GeoExtents
  int makeGeoExtent_(const osgEarth::DataExtentList& extents, osgEarth::GeoExtent& geoExtent) const;
  /// Calculates the distance for camera RAE given the extents (radians)
  double distance_(double minLat, double maxLat, double minLon, double maxLon) const;

  /// Current view that will be manipulated with calls to centerOn()
  osg::observer_ptr<simVis::View> view_;
  /// Minimum permitted distance on eye from earth, in meters; clamped above this value
  double minEyeDistance_;
  /// Maximum permitted distance on eye from earth, in meters; clamped below this value
  double maxEyeDistance_;
};

}

#endif /* SIMUTIL_RECENTEREYEONAREA_H */
