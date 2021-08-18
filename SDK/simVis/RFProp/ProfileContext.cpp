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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Time/TimeClass.h"
#include "simVis/RFProp/ProfileContext.h"

namespace simRF {

ProfileContext::ProfileContext(std::shared_ptr<simCore::DatumConvert> datumConvert)
  : heightM(0.),
  elevAngleR(0.),
  displayThickness(1),
  mode(Profile::DRAWMODE_2D_HORIZONTAL),
  type(simRF::ProfileDataProvider::THRESHOLDTYPE_NONE),
  agl(false),
  sphericalEarth(true),
  datumConvert_(datumConvert)
{
}

void ProfileContext::setRefLla(const simCore::Vec3& lla)
{
  refLla = lla;
  coordConvert_.setReferenceOrigin(refLla);
  simCore::geodeticToSpherical(refLla.lat(), refLla.lon(), refLla.alt(), tpSphereXYZ_);
}

double ProfileContext::adjustHeight(const simCore::Vec3& xEast) const
{
  if (sphericalEarth)
  {
    // heights are offsets in meters to a spherical tangent plane at the reflla
    simCore::Vec3 sphereXYZ;
    simCore::tangentPlane2Sphere(refLla, xEast, sphereXYZ, &tpSphereXYZ_);
    const double altAboveSphere = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;
    return xEast.z() + refLla.alt() - (altAboveSphere - xEast.z());
  }

  // heights in data are MSL or AGL and need to be converted to HAE height for the scenegraph
  // AGL not implemented
  if (datumConvert_.get())
  {
    // determine lla at x,y offset from reflla
    simCore::Coordinate out;
    simCore::Coordinate in(simCore::COORD_SYS_XEAST, xEast);
    coordConvert_.convert(in, out, simCore::COORD_SYS_LLA);

    // determine conversion from MSL to HAE, force use of EGM96
    const double mslToHaeM = datumConvert_->convertVerticalDatum(simCore::Vec3(out.lat(), out.lon(), 0.), simCore::TimeStamp(1996, 0.),
      simCore::COORD_SYS_LLA, simCore::VERTDATUM_MSL, simCore::VERTDATUM_WGS84, 0.);
    return xEast.z() + mslToHaeM;
  }
  return xEast.z();
}

}
