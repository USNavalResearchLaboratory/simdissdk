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
#ifndef SIMVIS_RFPROP_PROFILE_CONTEXT_H
#define SIMVIS_RFPROP_PROFILE_CONTEXT_H

#include <memory>
#include "osg/Vec3f"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/RFProp/Profile.h"

namespace simCore { class DatumConvert; }
namespace simRF
{
/** Display context that all profiles share */
struct ProfileContext
{
public:
  explicit ProfileContext(std::shared_ptr<simCore::DatumConvert> datumConvert);

  void setRefLla(const simCore::Vec3& refLla);

  /// returns wgs84 height of point at specified xEast offset from the refLla
  double adjustHeight(const simCore::Vec3& xEast) const;

  simCore::Vec3 refLla;      ///< Reference coordinate used for coordinate conversion used in the visualization, in radians and meters
  double heightM;           ///< display height, in meters
  double elevAngleR;        ///< elevation angle used in the current display, in radians
  unsigned int displayThickness;  ///< display thickness for 3D displays, in # height steps
  Profile::DrawMode mode;  ///< Type of display, e.g. 2D, 3D
  ProfileDataProvider::ThresholdType type;  ///< threshold type selected for display, e.g., POD, SNR, CNR
  bool agl;                ///< whether height values for the 2D Horizontal display are referenced to height above ground level (AGL) or to mean sea level (MSL).
  bool sphericalEarth;     ///< whether the profile data are specified for spherical or WGS84 earth

private:
  std::shared_ptr<simCore::DatumConvert> datumConvert_;    ///< provides MSL data for height correction
  simCore::CoordinateConverter coordConvert_;   ///< converts datapoint enu values to LLA for datum
  simCore::Vec3 tpSphereXYZ_; ///< refLLA converted to spherical earth
};
}
#endif
