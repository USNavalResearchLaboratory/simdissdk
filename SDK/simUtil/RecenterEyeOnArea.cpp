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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Math.h"
#include "simVis/View.h"
#include "simUtil/RecenterEyeOnArea.h"

namespace simUtil
{

/// Minimum distance for eye range when centering; distance from surface
static const double CLOSE_EYE_DISTANCE = 10.0; // meters
/// Maximum distance for eye range when centering; distance from surface
static const double FAR_EYE_DISTANCE = 7.0e6;  // meters

RecenterEyeOnArea::RecenterEyeOnArea(simVis::View* view)
  : view_(view),
    minEyeDistance_(CLOSE_EYE_DISTANCE),
    maxEyeDistance_(FAR_EYE_DISTANCE)
{
}

RecenterEyeOnArea::RecenterEyeOnArea(const simVis::FocusManager& focusManager)
  : view_(focusManager.getFocusedView()),
    minEyeDistance_(CLOSE_EYE_DISTANCE),
    maxEyeDistance_(FAR_EYE_DISTANCE)
{
}

RecenterEyeOnArea::~RecenterEyeOnArea()
{
}

void RecenterEyeOnArea::setView(simVis::View* view)
{
  view_ = view;
}

void RecenterEyeOnArea::setView(const simVis::FocusManager& focusManager)
{
  view_ = focusManager.getFocusedView();
}

void RecenterEyeOnArea::setRangeClamp(double clampAbove, double clampBelow)
{
  minEyeDistance_ = clampAbove;
  maxEyeDistance_ = clampBelow;
}

int RecenterEyeOnArea::centerOn(double lowerLat, double upperLat, double leftLon, double rightLon, double transitionSec, double distanceFactor)
{
  // Put the observer_ptr into a ref_ptr so the memory doesn't go away
  osg::ref_ptr<simVis::View> view;
  if (!view_.lock(view) || !view.valid())
    return 1;

  // Get the center point of the positions
  simCore::Vec3 centerLla;
  simCore::calculateGeodeticMidPoint(simCore::Vec3(lowerLat, leftLon, 0.0), simCore::Vec3(upperLat, rightLon, 0.0), false, centerLla);
  const double distance = distance_(lowerLat, upperLat, leftLon, rightLon) * distanceFactor;

  // update the eye position's focal point
  simVis::Viewpoint eyePos = view->getViewpoint();
  eyePos.setNode(nullptr);
  eyePos.focalPoint() = osgEarth::GeoPoint(
    osgEarth::SpatialReference::create("wgs84"),
    osg::Vec3d(centerLla.lon() * simCore::RAD2DEG, centerLla.lat() * simCore::RAD2DEG, 0.0));

  // Always look down on the area�
  eyePos.heading()->set(0.0, osgEarth::Units::DEGREES);
  eyePos.pitch()->set(-90.0, osgEarth::Units::DEGREES);
  // Clamp the distance between the close and far eye distances, so that we don't
  // hit our eyeball on the surface, or zoom out to a pinpoint of an earth
  osgEarth::Distance range(simCore::sdkMin(maxEyeDistance_, simCore::sdkMax(minEyeDistance_, distance)), osgEarth::Units::METERS);
  eyePos.setRange(range);
  view->setViewpoint(eyePos, transitionSec);
  return 0;
}

int RecenterEyeOnArea::centerOn(const osgEarth::DataExtentList& extents, double transitionSec)
{
  if (!view_.valid() || extents.empty())
    return 1;
  osgEarth::GeoExtent geoExtent;
  if (makeGeoExtent_(extents, geoExtent) != 0)
    return 1;
  return centerOn(geoExtent, transitionSec);
}

int RecenterEyeOnArea::centerOn(const osgEarth::GeoExtent& extent, double transitionSec)
{
  if (!view_.valid() || extent.isInvalid())
    return 1;

  return centerOn(simCore::DEG2RAD * extent.south(), simCore::DEG2RAD * extent.north(),
    simCore::DEG2RAD * extent.west(), simCore::DEG2RAD * extent.east(), transitionSec);
}

int RecenterEyeOnArea::makeGeoExtent_(const osgEarth::DataExtentList& extents, osgEarth::GeoExtent& geoExtent) const
{
  const osgEarth::SpatialReference* wgs84 = osgEarth::SpatialReference::create("wgs84");
  bool foundValid = false;

  // Loop through each extent in the list
  for (osgEarth::DataExtentList::const_iterator i = extents.begin(); i != extents.end(); ++i)
  {
    // Transform to WGS84, then expand our geoExtent to include the area
    osgEarth::GeoExtent tmpExtent;
    if ((*i).transform(wgs84, tmpExtent))
    {
      if (!foundValid)
        geoExtent = tmpExtent;
      else
        geoExtent.expandToInclude(tmpExtent);
      foundValid = true;
    }
  }
  return foundValid ? 0 : 1;
}

double RecenterEyeOnArea::distance_(double minLat, double maxLat, double minLon, double maxLon) const
{
  return simCore::sodanoInverse(minLat, minLon, 0.0, maxLat, maxLon);
}

}
