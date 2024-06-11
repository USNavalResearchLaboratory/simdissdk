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
#include "osg/Transform"
#include "osgEarth/Viewpoint"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simUtil/ViewpointPositions.h"

namespace simUtil {

/// Initialize the SRS to nullptr (lazy initialization later)
const osgEarth::SpatialReference* ViewpointPositions::wgs84Srs_ = nullptr;

/// Extracts the Center LLA position out of a viewpoint. Returns Vec3 of Latitude (rad), Longitude (rad), Altitude (meters)
simCore::Vec3 ViewpointPositions::centerLla(const osgEarth::Viewpoint& vp)
{
  // Check the Tethered case first
  if (vp.nodeIsSet())
  {
    osg::ref_ptr<osg::Node> node = vp.getNode();
    if (node.valid())
      return simVis::computeNodeGeodeticPosition(node.get());
  }
  // Not tethered, or invalid tether
  simCore::Vec3 llaWorld;
  if (vp.focalPoint().isSet())
  {
    osgEarth::GeoPoint lonLatAlt;
    vp.focalPoint()->transform(wgs84_(), lonLatAlt);
    return simCore::Vec3(lonLatAlt.y() * simCore::DEG2RAD, lonLatAlt.x() * simCore::DEG2RAD, lonLatAlt.z());
  }

  // We're not tethered, but also do not have a focal point.  There is no valid center position that
  // can be obtained from the Viewpoint.  This can happen when adding a simVis::View that does not have
  // the expected scene data.  This might be operator error, but it also might occur if using a
  // simVis::View as a debug view for an RTT picker.

  // Else: return 0,0,0
  return simCore::Vec3();
}

/// Extracts the eye's own LLA, given a viewpoint.  Calculates position based on centerLla() plus offsets. Returns Vec3 of Latitude (rad), Longitude (rad), Altitude (meters)
simCore::Vec3 ViewpointPositions::eyeLla(const simVis::View& view)
{
  // Grab the viewpoint directly from the manipulator, instead of the View.  This prevents watch
  // mode offsets from tainting the azimuth/elevation calculations
  const osgEarth::Util::EarthManipulator* manip = dynamic_cast<const osgEarth::Util::EarthManipulator*>(view.getCameraManipulator());
  if (manip == nullptr)
  {
    // Failure means we don't have an Earth Manipulator.  This might happen if there is
    // a simVis::View that is used as an inset that displays something besides the scene.
    // This could happen with debug RTT textures.
    return simCore::Vec3();
  }
  const simVis::Viewpoint viewVp = view.getViewpoint();
  const simVis::Viewpoint manipVp = manip->getViewpoint();
  simCore::Vec3 llaOrigin = ViewpointPositions::centerLla(viewVp);

  // Move the origin based on the position offsets provided in offsetXYZ
  if (manipVp.positionOffset().isSet() && *manipVp.positionOffset() != osg::Vec3d(0,0,0))
  {
    // Create a coordinate converter centered on the view entity
    simCore::CoordinateConverter cc;
    cc.setReferenceOrigin(llaOrigin);
    const simCore::Vec3 posOffset(manipVp.positionOffset()->x(), manipVp.positionOffset()->y(), manipVp.positionOffset()->z());
    const simCore::Coordinate offsetCoord(simCore::COORD_SYS_ENU, posOffset);
    // Convert to LLA and replace the llaOrigin value
    simCore::Coordinate outLla;
    cc.convert(offsetCoord, outLla, simCore::COORD_SYS_LLA);
    llaOrigin = outLla.position();
  }

  // Pull the Azimuth and Elevation from the manipulator to get absolute values (simVis::View::getViewpoint might be relative in Watch mode)
  double azToEye;
  double elToEye;
  manip->getCompositeEulerAngles(&azToEye, &elToEye);
  // Adjust the values to reverse the direction, suitable for calculateGeodeticEndPoint()
  elToEye = -elToEye;
  azToEye -= M_PI;

  // Calculate the endpoint
  simCore::Vec3 llaEye;
  simCore::calculateGeodeticEndPoint(llaOrigin, azToEye, elToEye, manipVp.range()->as(osgEarth::Units::METERS), llaEye);
  return llaEye;
}

/// Lazy initialization on static WGS-84 SRS */
const osgEarth::SpatialReference* ViewpointPositions::wgs84_()
{
  if (ViewpointPositions::wgs84Srs_ == nullptr)
    ViewpointPositions::wgs84Srs_ = osgEarth::SpatialReference::create("wgs84");
  return ViewpointPositions::wgs84Srs_;
}

}
