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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/DatumConvert.h"
#include "simVis/RangeToolState.h"
#include "simVis/Measurement.h"

namespace simVis
{

std::string ValueFormatter::stringValue(double value, int precision) const
{
  std::stringstream buf;
  buf << std::fixed << std::setprecision(precision) << value;
  return buf.str();
}

//------------------------------------------------------------------------

Measurement::Measurement(const std::string& typeName, const std::string& typeAbbr, const simCore::Units& units)
  : formatter_(new ValueFormatter),
  typeName_(typeName),
  typeAbbr_(typeAbbr),
  units_(units)
{
  //nop
}

double Measurement::value(const simCore::Units& outputUnits, RangeToolState& state) const
{
  return units_.convertTo(outputUnits, value(state));
}

bool Measurement::isEntityToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if ((fromType == simData::NONE) || (fromType == simData::PROJECTOR))
    return false;

  if ((toType == simData::NONE) || (toType == simData::PROJECTOR))
    return false;

  return true;
}

bool Measurement::isPlatformToPlatform_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if ((fromType != simData::PLATFORM) || (toType != simData::PLATFORM))
    return false;

  return true;
}

bool Measurement::isLocationToLocation_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  return (((fromType == simData::PLATFORM) || (fromType == simData::CUSTOM_RENDERING)) &&
    ((toType == simData::PLATFORM) || (toType == simData::CUSTOM_RENDERING)));
}

bool Measurement::isBeamToNonBeamAssociation_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if (((fromType == simData::PLATFORM) ||
    (fromType == simData::GATE) ||
    (fromType == simData::LOB_GROUP) ||
    (fromType == simData::LASER) ||
    (fromType == simData::CUSTOM_RENDERING)) &&
    (toType == simData::BEAM))
    return true;

  return (((toType == simData::PLATFORM) ||
    (toType == simData::GATE) ||
    (toType == simData::LOB_GROUP) ||
    (toType == simData::LASER) ||
    (toType == simData::CUSTOM_RENDERING)) &&
    (fromType == simData::BEAM));
}

bool Measurement::isBeamToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const
{
  if (fromType != simData::BEAM)
    return false;

  return ((toType == simData::PLATFORM) ||
    (toType == simData::BEAM) ||
    (toType == simData::GATE) ||
    (toType == simData::LOB_GROUP) ||
    (toType == simData::LASER) ||
    (toType == simData::CUSTOM_RENDERING));
}

bool Measurement::isRaeObject_(simData::ObjectType type) const
{
  return ((type == simData::GATE) ||
    (type == simData::LOB_GROUP) ||
    (type == simData::LASER) ||
    (type == simData::BEAM));
}

bool Measurement::isAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId,
  simData::ObjectType toType, simData::ObjectId toHostId) const
{
  if (isRaeObject_(toType) && isRaeObject_(fromType) && (fromHostId != toHostId))
  {
    // not valid when RAE based objects are not on the same host platform
    return false;
  }
  else if ((fromType == simData::PLATFORM) && isRaeObject_(toType) && (fromHostId != toHostId))
  {
    // not valid when RAE based end entity is compared to a platform other than its host
    return false;
  }

  return true;
}

bool Measurement::isVelocityAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId,
  simData::ObjectType toType, simData::ObjectId toHostId) const
{
  if (fromType != simData::PLATFORM)
    return false;

  if (isRaeObject_(toType) && (fromHostId != toHostId))
    return false;

  return true;
}

double Measurement::getCompositeAngle_(double bgnAz, double bgnEl, double endAz, double endEl) const
{
  // assumes both bgn and end are wrt the same point/host platform
  simCore::Vec3 bgnVec, endVec;
  simCore::v3SphtoRec(simCore::Vec3(1, bgnAz, bgnEl), bgnVec);
  simCore::v3SphtoRec(simCore::Vec3(1, endAz, endEl), endVec);
  return simCore::v3Angle(bgnVec, endVec);
}

void Measurement::calculateTrueAngles_(const RangeToolState& state, double* az, double* el, double* cmp) const
{
  bool raeBeginEntity = isRaeObject_(state.beginEntity_->type_);
  bool raeEndEntity = isRaeObject_(state.endEntity_->type_);

  if ((raeBeginEntity && raeEndEntity && (state.beginEntity_->hostId_ == state.endEntity_->hostId_)) ||
    (raeEndEntity && (state.beginEntity_->hostId_ == state.endEntity_->hostId_)))
  {
    // handle cases where calculations are between RAE based objects on the same host platform or
    // between a host platform (begin) and one of its own RAE based objects (end)
    if (az)
      *az = state.endEntity_->ypr_.yaw();
    if (el)
      *el = state.endEntity_->ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0, 0, state.endEntity_->ypr_.yaw(), state.endEntity_->ypr_.pitch());
  }
  else if (raeBeginEntity && (state.beginEntity_->hostId_ == state.endEntity_->hostId_))
  {
    // between a host platform (end) and one of its own RAE based objects (begin)
    if (az)
      *az = state.beginEntity_->ypr_.yaw();
    if (el)
      *el = state.beginEntity_->ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0, 0, state.beginEntity_->ypr_.yaw(), state.beginEntity_->ypr_.pitch());
  }
  else
  {
    simCore::calculateAbsAzEl(state.beginEntity_->lla_, state.endEntity_->lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}


//----------------------------------------------------------------------------

GroundDistanceMeasurement::GroundDistanceMeasurement()
  : Measurement("Ground Rng", "Dist", simCore::Units::METERS)
{ }

double GroundDistanceMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
}

bool GroundDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

SlantDistanceMeasurement::SlantDistanceMeasurement()
  : Measurement("Slant Rng", "Rng", simCore::Units::METERS)
{ }

double SlantDistanceMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateSlant(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
}

bool SlantDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

AltitudeDeltaMeasurement::AltitudeDeltaMeasurement()
  : Measurement("Altitude", "Alt", simCore::Units::METERS)
{ }

double AltitudeDeltaMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateAltitude(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
}

bool AltitudeDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

BeamGroundDistanceMeasurement::BeamGroundDistanceMeasurement()
  : Measurement("Beam Ground Rng", "Dist(B)", simCore::Units::METERS)
{ }

double BeamGroundDistanceMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateGroundDist(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamGroundDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

BeamSlantDistanceMeasurement::BeamSlantDistanceMeasurement()
  : Measurement("Beam Slant Rng", "Rng(B)", simCore::Units::METERS)
{ }

double BeamSlantDistanceMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateSlant(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamSlantDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

BeamAltitudeDeltaMeasurement::BeamAltitudeDeltaMeasurement()
  : Measurement("Beam Altitude", "Alt(B)", simCore::Units::METERS)
{ }

double BeamAltitudeDeltaMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateAltitude(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamAltitudeDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

DownRangeMeasurement::DownRangeMeasurement()
  : Measurement("Downrange", "DR", simCore::Units::METERS)
{ }

double DownRangeMeasurement::value(RangeToolState& state) const
{
  double dr;
  simCore::calculateDRCRDownValue(state.beginEntity_->lla_, state.beginEntity_->ypr_.x(), state.endEntity_->lla_, state.earthModel_, &state.coordConv_, &dr, nullptr, nullptr);
  return dr;
}

bool DownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

CrossRangeMeasurement::CrossRangeMeasurement()
  : Measurement("Crossrange", "CR", simCore::Units::METERS)
{ }

double CrossRangeMeasurement::value(RangeToolState& state) const
{
  double cr;
  simCore::calculateDRCRDownValue(state.beginEntity_->lla_, state.beginEntity_->ypr_.x(), state.endEntity_->lla_, state.earthModel_, &state.coordConv_, nullptr, &cr, nullptr);
  return cr;
}

bool CrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

DownRangeCrossRangeDownValueMeasurement::DownRangeCrossRangeDownValueMeasurement()
  : Measurement("Down Value", "DV", simCore::Units::METERS)
{ }

double DownRangeCrossRangeDownValueMeasurement::value(RangeToolState& state) const
{
  double dv;
  simCore::calculateDRCRDownValue(state.beginEntity_->lla_, state.beginEntity_->ypr_.x(), state.endEntity_->lla_, state.earthModel_, &state.coordConv_, nullptr, nullptr, &dv);
  return dv;
}

bool DownRangeCrossRangeDownValueMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

GeoDownRangeMeasurement::GeoDownRangeMeasurement()
  : Measurement("Geo Downrange", "DR(g)", simCore::Units::METERS)
{ }

double GeoDownRangeMeasurement::value(RangeToolState& state) const
{
  double dr;
  simCore::calculateGeodesicDRCR(state.beginEntity_->lla_, state.beginEntity_->ypr_.x(), state.endEntity_->lla_, &dr, nullptr);
  return dr;
}

bool GeoDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

GeoCrossRangeMeasurement::GeoCrossRangeMeasurement()
  : Measurement("Geo Crossrange", "CR(g)", simCore::Units::METERS)
{ }

double GeoCrossRangeMeasurement::value(RangeToolState& state) const
{
  double cr;
  simCore::calculateGeodesicDRCR(state.beginEntity_->lla_, state.beginEntity_->ypr_.x(), state.endEntity_->lla_, nullptr, &cr);
  return cr;
}

bool GeoCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

TrueAzimuthMeasurement::TrueAzimuthMeasurement()
  : Measurement("True Azim", "Az(T)", simCore::Units::RADIANS)
{ }

double TrueAzimuthMeasurement::value(RangeToolState& state) const
{
  double az;
  calculateTrueAngles_(state, &az, nullptr, nullptr);
  return az;
}

bool TrueAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

TrueElevationMeasurement::TrueElevationMeasurement()
  : Measurement("True Elev", "El", simCore::Units::RADIANS)
{ }

double TrueElevationMeasurement::value(RangeToolState& state) const
{
  double el;
  calculateTrueAngles_(state, nullptr, &el, nullptr);
  return el;
}

bool TrueElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

TrueCompositeAngleMeasurement::TrueCompositeAngleMeasurement()
  : Measurement("True Composite", "Cmp(T)", simCore::Units::RADIANS)
{ }

double TrueCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double cmp;
  calculateTrueAngles_(state, nullptr, nullptr, &cmp);
  return cmp;
}

bool TrueCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

MagneticAzimuthMeasurement::MagneticAzimuthMeasurement(std::shared_ptr<simCore::DatumConvert> datumConvert)
  : Measurement("Mag Azim", "Az(M)", simCore::Units::RADIANS),
  datumConvert_(datumConvert)
{
}

double MagneticAzimuthMeasurement::value(RangeToolState& state) const
{
  double az;
  calculateTrueAngles_(state, &az, nullptr, nullptr);
  az = datumConvert_->convertMagneticDatum(state.beginEntity_->lla_, state.timeStamp_, az, simCore::COORD_SYS_LLA, simCore::MAGVAR_TRUE, simCore::MAGVAR_WMM, 0.0);
  return az;
}

bool MagneticAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

void RelOriMeasurement::getAngles(double* az, double* el, double* cmp, RangeToolState& state) const
{
  bool raeBgnEntity = isRaeObject_(state.beginEntity_->type_);
  bool raeEndEntity = isRaeObject_(state.endEntity_->type_);
  if (raeBgnEntity && raeEndEntity && (state.beginEntity_->hostId_ == state.endEntity_->hostId_))
  {
    // handle cases where calculations are between RAE based objects with the same host platform
    if (az)
      *az = state.endEntity_->ypr_.yaw() - state.beginEntity_->ypr_.yaw();
    if (el)
      *el = state.endEntity_->ypr_.pitch() - state.beginEntity_->ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_->ypr_.yaw(), state.beginEntity_->ypr_.pitch(), state.endEntity_->ypr_.yaw(), state.endEntity_->ypr_.pitch());
  }
  else if ((raeBgnEntity && (state.endEntity_->type_ == simData::PLATFORM) && (state.beginEntity_->hostId_ == state.endEntity_->hostId_)) ||
    (raeEndEntity && (state.beginEntity_->type_ == simData::PLATFORM) && (state.beginEntity_->hostId_ == state.endEntity_->hostId_)))
  {
    // handle cases where calculations are between RAE based objects their own host platform
    if (az)
      *az = state.endEntity_->ypr_.yaw() - state.beginEntity_->ypr_.yaw();
    if (el)
      *el = state.endEntity_->ypr_.pitch() - state.beginEntity_->ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_->ypr_.yaw(), state.beginEntity_->ypr_.pitch(), state.endEntity_->ypr_.yaw(), state.endEntity_->ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_->lla_, state.beginEntity_->ypr_, state.endEntity_->lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RelOriAzimuthMeasurement::RelOriAzimuthMeasurement()
  : RelOriMeasurement("Rel Azim", "Az(r)", simCore::Units::RADIANS)
{ }

double RelOriAzimuthMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  az = simCore::angFixPI(az);
  return az;
}

bool RelOriAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

RelOriElevationMeasurement::RelOriElevationMeasurement()
  : RelOriMeasurement("Rel Elev", "El(r)", simCore::Units::RADIANS)
{ }

double RelOriElevationMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RelOriElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

RelOriCompositeAngleMeasurement::RelOriCompositeAngleMeasurement()
  : RelOriMeasurement("Rel Composite", "Cmp(r)", simCore::Units::RADIANS)
{ }

double RelOriCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RelOriCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

void RelVelMeasurement::getAngles(double* az, double* el, double* cmp, RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
  {
    *az = 0.;
    *el = 0.;
    *cmp = 0.;
    return;
  }
  simCore::Vec3 fpaVec;
  simCore::calculateFlightPathAngles(vel, fpaVec);

  bool raeEndEntity = isRaeObject_(state.endEntity_->type_);
  if (raeEndEntity && (state.beginEntity_->type_ == simData::PLATFORM) && (state.beginEntity_->hostId_ == state.endEntity_->hostId_))
  {
    // handle case where calculation is between host platform and its RAE based objects
    if (az)
      *az = state.endEntity_->ypr_.yaw() - fpaVec.yaw();
    if (el)
      *el = state.endEntity_->ypr_.pitch() - fpaVec.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(fpaVec.yaw(), fpaVec.pitch(), state.endEntity_->ypr_.yaw(), state.endEntity_->ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_->lla_, fpaVec, state.endEntity_->lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RelVelAzimuthMeasurement::RelVelAzimuthMeasurement()
  : RelVelMeasurement("Rel Vel Azim", "Az(v)", simCore::Units::RADIANS)
{ }

double RelVelAzimuthMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return az;
}

bool RelVelAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

RelVelElevationMeasurement::RelVelElevationMeasurement()
  : RelVelMeasurement("Rel Vel Elev", "El(v)", simCore::Units::RADIANS)
{ }

double RelVelElevationMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RelVelElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}

//----------------------------------------------------------------------------

RelVelCompositeAngleMeasurement::RelVelCompositeAngleMeasurement()
  : RelVelMeasurement("Rel Vel Composite", "Cmp(v)", simCore::Units::RADIANS)
{ }

double RelVelCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RelVelCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_->type_, state.beginEntity_->hostId_, state.endEntity_->type_, state.endEntity_->hostId_);
}
//----------------------------------------------------------------------------

ClosingVelocityMeasurement::ClosingVelocityMeasurement()
  : Measurement("Closing Vel", "V(c)", simCore::Units::METERS_PER_SECOND)
{ }

double ClosingVelocityMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateClosingVelocity(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_, state.beginEntity_->vel_, state.endEntity_->vel_);
}

bool ClosingVelocityMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

SeparationVelocityMeasurement::SeparationVelocityMeasurement()
  : Measurement("Separation Vel", "V(s)", simCore::Units::METERS_PER_SECOND)
{ }

double SeparationVelocityMeasurement::value(RangeToolState& state) const
{
  return -simCore::calculateClosingVelocity(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_, state.beginEntity_->vel_, state.endEntity_->vel_);
}

bool SeparationVelocityMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

VelocityDeltaMeasurement::VelocityDeltaMeasurement()
  : Measurement("Vel Delta", "V(d)", simCore::Units::METERS_PER_SECOND)
{ }

double VelocityDeltaMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateVelocityDelta(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_, state.beginEntity_->vel_, state.endEntity_->vel_);
}

bool VelocityDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

VelAzimDownRangeMeasurement::VelAzimDownRangeMeasurement()
  : Measurement("Vel Azim Down Range", "DR(v)", simCore::Units::METERS)
{ }

double VelAzimDownRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double downRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_->lla_, fpa[0], state.endEntity_->lla_, state.earthModel_, &state.coordConv_, &downRng, nullptr, nullptr);
  return downRng;
}

bool VelAzimDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_->type_ == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimCrossRangeMeasurement::VelAzimCrossRangeMeasurement()
  : Measurement("Vel Azim Cross Range", "CR(v)", simCore::Units::METERS)
{ }

double VelAzimCrossRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double crossRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_->lla_, fpa[0], state.endEntity_->lla_, state.earthModel_, &state.coordConv_, nullptr, &crossRng, nullptr);
  return crossRng;
}

bool VelAzimCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_->type_ == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimGeoDownRangeMeasurement::VelAzimGeoDownRangeMeasurement()
  : Measurement("Vel Azim Geo Down Range", "DR(gv)", simCore::Units::METERS)
{ }

double VelAzimGeoDownRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double downRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_->lla_, fpa[0], state.endEntity_->lla_, &downRng, nullptr);
  return downRng;
}

bool VelAzimGeoDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_->type_ == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimGeoCrossRangeMeasurement::VelAzimGeoCrossRangeMeasurement()
  : Measurement("Vel Azim Geo Cross Range", "CR(gv)", simCore::Units::METERS)
{ }

double VelAzimGeoCrossRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_->vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double crossRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_->lla_, fpa[0], state.endEntity_->lla_, nullptr, &crossRng);
  return crossRng;
}

bool VelAzimGeoCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_->type_ == simData::PLATFORM);
}

//----------------------------------------------------------------------------

AspectAngleMeasurement::AspectAngleMeasurement()
  : Measurement("Aspect Angle", "Asp(r)", simCore::Units::RADIANS)
{ }

double AspectAngleMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateAspectAngle(state.beginEntity_->lla_, state.endEntity_->lla_, state.endEntity_->ypr_);
}

bool AspectAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isLocationToLocation_(state.beginEntity_->type_, state.endEntity_->type_);
}

}
