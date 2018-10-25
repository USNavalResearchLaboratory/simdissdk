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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"
#include "simVis/Beam.h"
#include "simVis/ElevationQueryProxy.h"
#include "simVis/Entity.h"
#include "simVis/RFProp/RFPropagationFacade.h"
#include "simVis/RangeToolState.h"
#include "simVis/Utils.h"
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

std::string HorizonFormatter::stringValue(double value, int precision) const
{
  if (value == 0.0)
    return "Below";

  return "Above";
}

//------------------------------------------------------------------------

Measurement::Measurement(const std::string& typeName, const std::string& typeAbbr, const osgEarth::Units& units)
  : formatter_(new ValueFormatter),
  typeName_(typeName),
  typeAbbr_(typeAbbr),
  units_(units)
{
  //nop
}

double Measurement::value(const osgEarth::Units& outputUnits, RangeToolState& state) const
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
  bool raeBeginEntity = isRaeObject_(state.beginEntity_.node_->type());
  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());

  if ((raeBeginEntity && raeEndEntity && (state.beginEntity_.hostId_ == state.endEntity_.hostId_)) ||
    (raeEndEntity && (state.beginEntity_.hostId_ == state.endEntity_.hostId_)))
  {
    // handle cases where calculations are between RAE based objects on the same host platform or
    // between a host platform (begin) and one of its own RAE based objects (end)
    if (az)
      *az = state.endEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0, 0, state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else if (raeBeginEntity && (state.beginEntity_.hostId_ == state.endEntity_.hostId_))
  {
    // between a host platform (end) and one of its own RAE based objects (begin)
    if (az)
      *az = state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(0, 0, state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateAbsAzEl(state.beginEntity_.lla_, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}


//----------------------------------------------------------------------------

GroundDistanceMeasurement::GroundDistanceMeasurement()
  : Measurement("Ground Rng", "Dist", osgEarth::Units::METERS)
{ }

double GroundDistanceMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool GroundDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

SlantDistanceMeasurement::SlantDistanceMeasurement()
  : Measurement("Slant Rng", "Rng", osgEarth::Units::METERS)
{ }

double SlantDistanceMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool SlantDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

AltitudeDeltaMeasurement::AltitudeDeltaMeasurement()
  : Measurement("Altitude", "Alt", osgEarth::Units::METERS)
{ }

double AltitudeDeltaMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateAltitude(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
}

bool AltitudeDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

BeamGroundDistanceMeasurement::BeamGroundDistanceMeasurement()
  : Measurement("Beam Ground Rng", "Dist(B)", osgEarth::Units::METERS)
{ }

double BeamGroundDistanceMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateGroundDist(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamGroundDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

BeamSlantDistanceMeasurement::BeamSlantDistanceMeasurement()
  : Measurement("Beam Slant Rng", "Rng(B)", osgEarth::Units::METERS)
{ }

double BeamSlantDistanceMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateSlant(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamSlantDistanceMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

BeamAltitudeDeltaMeasurement::BeamAltitudeDeltaMeasurement()
  : Measurement("Beam Altitude", "Alt(B)", osgEarth::Units::METERS)
{ }

double BeamAltitudeDeltaMeasurement::value(RangeToolState& state) const
{
  simCore::Vec3 from = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_0));
  simCore::Vec3 to = state.osg2simCore(state.coord(RangeToolState::COORD_BEAM_LLA_1));
  return simCore::calculateAltitude(from, to, state.earthModel_, &state.coordConv_);
}

bool BeamAltitudeDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToNonBeamAssociation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

DownRangeMeasurement::DownRangeMeasurement()
  : Measurement("Downrange", "DR", osgEarth::Units::METERS)
{ }

double DownRangeMeasurement::value(RangeToolState& state) const
{
  double dr;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, &dr, NULL, NULL);
  return dr;
}

bool DownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

CrossRangeMeasurement::CrossRangeMeasurement()
  : Measurement("Crossrange", "CR", osgEarth::Units::METERS)
{ }

double CrossRangeMeasurement::value(RangeToolState& state) const
{
  double cr;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, &cr, NULL);
  return cr;
}

bool CrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

DownRangeCrossRangeDownValueMeasurement::DownRangeCrossRangeDownValueMeasurement()
  : Measurement("Down Value", "DV", osgEarth::Units::METERS)
{ }

double DownRangeCrossRangeDownValueMeasurement::value(RangeToolState& state) const
{
  double dv;
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, NULL, &dv);
  return dv;
}

bool DownRangeCrossRangeDownValueMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

GeoDownRangeMeasurement::GeoDownRangeMeasurement()
  : Measurement("Geo Downrange", "DR(g)", osgEarth::Units::METERS)
{ }

double GeoDownRangeMeasurement::value(RangeToolState& state) const
{
  double dr;
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, &dr, NULL);
  return dr;
}

bool GeoDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

GeoCrossRangeMeasurement::GeoCrossRangeMeasurement()
  : Measurement("Geo Crossrange", "CR(g)", osgEarth::Units::METERS)
{ }

double GeoCrossRangeMeasurement::value(RangeToolState& state) const
{
  double cr;
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, state.beginEntity_.ypr_.x(), state.endEntity_.lla_, NULL, &cr);
  return cr;
}

bool GeoCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

TrueAzimuthMeasurement::TrueAzimuthMeasurement()
  : Measurement("True Azim", "Az(T)", osgEarth::Units::RADIANS)
{ }

double TrueAzimuthMeasurement::value(RangeToolState& state) const
{
  double az;
  calculateTrueAngles_(state, &az, NULL, NULL);
  return az;
}

bool TrueAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

TrueElevationMeasurement::TrueElevationMeasurement()
  : Measurement("True Elev", "El", osgEarth::Units::RADIANS)
{ }

double TrueElevationMeasurement::value(RangeToolState& state) const
{
  double el;
  calculateTrueAngles_(state, NULL, &el, NULL);
  return el;
}

bool TrueElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

TrueCompositeAngleMeasurement::TrueCompositeAngleMeasurement()
  : Measurement("True Composite", "Cmp(T)", osgEarth::Units::RADIANS)
{ }

double TrueCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double cmp;
  calculateTrueAngles_(state, NULL, NULL, &cmp);
  return cmp;
}

bool TrueCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

MagneticAzimuthMeasurement::MagneticAzimuthMeasurement(std::shared_ptr<simCore::DatumConvert> datumConvert)
  : Measurement("Mag Azim", "Az(M)", osgEarth::Units::RADIANS),
  datumConvert_(datumConvert)
{
}

double MagneticAzimuthMeasurement::value(RangeToolState& state) const
{
  double az;
  calculateTrueAngles_(state, &az, NULL, NULL);
  az = datumConvert_->convertMagneticDatum(state.beginEntity_.lla_, state.timeStamp_, az, simCore::COORD_SYS_LLA, simCore::MAGVAR_TRUE, simCore::MAGVAR_WMM, 0.0);
  return az;
}

bool MagneticAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

void RelOriMeasurement::getAngles(double* az, double* el, double* cmp, RangeToolState& state) const
{
  bool raeBgnEntity = isRaeObject_(state.beginEntity_.node_->type());
  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());
  if (raeBgnEntity && raeEndEntity && (state.beginEntity_.hostId_ == state.endEntity_.hostId_))
  {
    // handle cases where calculations are between RAE based objects with the same host platform
    if (az)
      *az = state.endEntity_.ypr_.yaw() - state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else if ((raeBgnEntity && (state.endEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.hostId_ == state.endEntity_.hostId_)) ||
    (raeEndEntity && (state.beginEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.hostId_ == state.endEntity_.hostId_)))
  {
    // handle cases where calculations are between RAE based objects their own host platform
    if (az)
      *az = state.endEntity_.ypr_.yaw() - state.beginEntity_.ypr_.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - state.beginEntity_.ypr_.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(state.beginEntity_.ypr_.yaw(), state.beginEntity_.ypr_.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_.lla_, state.beginEntity_.ypr_, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RelOriAzimuthMeasurement::RelOriAzimuthMeasurement()
  : RelOriMeasurement("Rel Azim", "Az(r)", osgEarth::Units::RADIANS)
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
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

RelOriElevationMeasurement::RelOriElevationMeasurement()
  : RelOriMeasurement("Rel Elev", "El(r)", osgEarth::Units::RADIANS)
{ }

double RelOriElevationMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RelOriElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

RelOriCompositeAngleMeasurement::RelOriCompositeAngleMeasurement()
  : RelOriMeasurement("Rel Composite", "Cmp(r)", osgEarth::Units::RADIANS)
{ }

double RelOriCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RelOriCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

void RelVelMeasurement::getAngles(double* az, double* el, double* cmp, RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
  {
    *az = 0.;
    *el = 0.;
    *cmp = 0.;
    return;
  }
  simCore::Vec3 fpaVec;
  simCore::calculateFlightPathAngles(vel, fpaVec);

  bool raeEndEntity = isRaeObject_(state.endEntity_.node_->type());
  if (raeEndEntity && (state.beginEntity_.node_->type() == simData::PLATFORM) && (state.beginEntity_.hostId_ == state.endEntity_.hostId_))
  {
    // handle case where calculation is between host platform and its RAE based objects
    if (az)
      *az = state.endEntity_.ypr_.yaw() - fpaVec.yaw();
    if (el)
      *el = state.endEntity_.ypr_.pitch() - fpaVec.pitch();
    if (cmp)
      *cmp = getCompositeAngle_(fpaVec.yaw(), fpaVec.pitch(), state.endEntity_.ypr_.yaw(), state.endEntity_.ypr_.pitch());
  }
  else
  {
    simCore::calculateRelAzEl(state.beginEntity_.lla_, fpaVec, state.endEntity_.lla_, az, el, cmp, state.earthModel_, &state.coordConv_);
  }
}

//----------------------------------------------------------------------------

RelVelAzimuthMeasurement::RelVelAzimuthMeasurement()
  : RelVelMeasurement("Rel Vel Azim", "Az(v)", osgEarth::Units::RADIANS)
{ }

double RelVelAzimuthMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return az;
}

bool RelVelAzimuthMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

RelVelElevationMeasurement::RelVelElevationMeasurement()
  : RelVelMeasurement("Rel Vel Elev", "El(v)", osgEarth::Units::RADIANS)
{ }

double RelVelElevationMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return el;
}

bool RelVelElevationMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}

//----------------------------------------------------------------------------

RelVelCompositeAngleMeasurement::RelVelCompositeAngleMeasurement()
  : RelVelMeasurement("Rel Vel Composite", "Cmp(v)", osgEarth::Units::RADIANS)
{ }

double RelVelCompositeAngleMeasurement::value(RangeToolState& state) const
{
  double az, el, cmp;
  getAngles(&az, &el, &cmp, state);
  return cmp;
}

bool RelVelCompositeAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isVelocityAngle_(state.beginEntity_.node_->type(), state.beginEntity_.hostId_, state.endEntity_.node_->type(), state.endEntity_.hostId_);
}
//----------------------------------------------------------------------------

ClosingVelocityMeasurement::ClosingVelocityMeasurement()
  : Measurement("Closing Vel", "V(c)", osgEarth::Units::METERS_PER_SECOND)
{ }

double ClosingVelocityMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateClosingVelocity(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool ClosingVelocityMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

SeparationVelocityMeasurement::SeparationVelocityMeasurement()
  : Measurement("Separation Vel", "V(s)", osgEarth::Units::METERS_PER_SECOND)
{ }

double SeparationVelocityMeasurement::value(RangeToolState& state) const
{
  return -simCore::calculateClosingVelocity(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool SeparationVelocityMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

VelocityDeltaMeasurement::VelocityDeltaMeasurement()
  : Measurement("Vel Delta", "V(d)", osgEarth::Units::METERS_PER_SECOND)
{ }

double VelocityDeltaMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateVelocityDelta(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_, state.beginEntity_.vel_, state.endEntity_.vel_);
}

bool VelocityDeltaMeasurement::willAccept(const RangeToolState& state) const
{
  return isPlatformToPlatform_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

VelAzimDownRangeMeasurement::VelAzimDownRangeMeasurement()
  : Measurement("Vel Azim Down Range", "DR(v)", osgEarth::Units::METERS)
{ }

double VelAzimDownRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double downRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, state.earthModel_, &state.coordConv_, &downRng, NULL, NULL);
  return downRng;
}

bool VelAzimDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimCrossRangeMeasurement::VelAzimCrossRangeMeasurement()
  : Measurement("Vel Azim Cross Range", "CR(v)", osgEarth::Units::METERS)
{ }

double VelAzimCrossRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double crossRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateDRCRDownValue(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, state.earthModel_, &state.coordConv_, NULL, &crossRng, NULL);
  return crossRng;
}

bool VelAzimCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimGeoDownRangeMeasurement::VelAzimGeoDownRangeMeasurement()
  : Measurement("Vel Azim Geo Down Range", "DR(gv)", osgEarth::Units::METERS)
{ }

double VelAzimGeoDownRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double downRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, &downRng, NULL);
  return downRng;
}

bool VelAzimGeoDownRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

VelAzimGeoCrossRangeMeasurement::VelAzimGeoCrossRangeMeasurement()
  : Measurement("Vel Azim Geo Cross Range", "CR(gv)", osgEarth::Units::METERS)
{ }

double VelAzimGeoCrossRangeMeasurement::value(RangeToolState& state) const
{
  const simCore::Vec3& vel = state.beginEntity_.vel_;
  if (simCore::v3AreEqual(vel, simCore::Vec3()))
    return 0.0;

  double crossRng = 0;
  simCore::Vec3 fpa;
  simCore::calculateFlightPathAngles(vel, fpa);
  simCore::calculateGeodesicDRCR(state.beginEntity_.lla_, fpa[0], state.endEntity_.lla_, NULL, &crossRng);
  return crossRng;
}

bool VelAzimGeoCrossRangeMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.beginEntity_.node_->type() == simData::PLATFORM);
}

//----------------------------------------------------------------------------

AspectAngleMeasurement::AspectAngleMeasurement()
  : Measurement("Aspect Angle", "Asp(r)", osgEarth::Units::RADIANS)
{ }

double AspectAngleMeasurement::value(RangeToolState& state) const
{
  return simCore::calculateAspectAngle(state.beginEntity_.lla_, state.endEntity_.lla_, state.endEntity_.ypr_);
}

bool AspectAngleMeasurement::willAccept(const RangeToolState& state) const
{
  return isLocationToLocation_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RfMeasurement::RfMeasurement(const std::string& name, const std::string& abbr, const osgEarth::Units& units)
  : RelOriMeasurement(name, abbr, units)
{

}

void RfMeasurement::getRfParameters_(RangeToolState& state, double *azAbs, double *elAbs, double *hgtMeters, double* xmtGaindB, double* rcvGaindB, double* rcs, bool useDb,
  double* freqMHz, double* powerWatts) const
{
  if (azAbs != NULL || elAbs != NULL)
  {
    double azAbsLocal;
    double elAbsLocal;
    calculateTrueAngles_(state, &azAbsLocal, &elAbsLocal, NULL);
    if (azAbs != NULL)
      *azAbs = azAbsLocal;

    if (elAbs != NULL)
      *elAbs = elAbsLocal;
  }

  if (hgtMeters != NULL)
  {
    const simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
    if (rf != NULL)
      *hgtMeters = rf->antennaHeight();
    else
      *hgtMeters = 0.0;
  }

  // Do NOT set RF parameter values from RFPropagationFacade, in order to match the behavior of SIMDIS 9

  if (xmtGaindB != NULL || rcvGaindB != NULL)
  {
    double xmtGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    double rcvGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
    if (beam)
    {
      double azRelLocal;
      double elRelLocal;
      RelOriMeasurement::getAngles(&azRelLocal, &elRelLocal, NULL, state);
      xmtGaindBLocal = beam->gain(azRelLocal, elRelLocal);
      rcvGaindBLocal = xmtGaindBLocal;
    }
    if (xmtGaindB != NULL)
      *xmtGaindB = xmtGaindBLocal;

    if (rcvGaindB != NULL)
      *rcvGaindB = rcvGaindBLocal;
  }

  if (rcs != NULL)
  {
    double rcsLocal = useDb ? simCore::SMALL_DB_VAL : simCore::SMALL_RCS_SM;
    // To match SIMDIS 9, the end entity must be a platform.
    if (state.endEntity_.node_->type() == simData::PLATFORM)
    {
      simCore::RadarCrossSectionPtr rcsPtr = state.endEntity_.platformHostNode_->getRcs();
      if (rcsPtr != NULL)
      {
        // need the angles from the target to the beam source to get the correct rcs values
        double azTarget;
        double elTarget;
        const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
        simCore::PolarityType type = (beam != NULL) ? beam->polarity() : simCore::POLARITY_UNKNOWN;
        const double frequency = simCore::DEFAULT_FREQUENCY;
        simCore::calculateRelAzEl(state.endEntity_.lla_, state.endEntity_.ypr_, state.beginEntity_.lla_, &azTarget, &elTarget, NULL, state.earthModel_, &state.coordConv_);
        if (useDb)
          rcsLocal = rcsPtr->RCSdB(frequency, azTarget, elTarget, type);
        else
          rcsLocal = rcsPtr->RCSsm(frequency, azTarget, elTarget, type);
      }
    }
    *rcs = rcsLocal;
  }

  if ((freqMHz != NULL) || (powerWatts != NULL))
  {
    double freqMHzBLocal = 0.0;
    double powerWattsLocal = 0.0;
    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
    if (beam)
    {
      auto prefs = beam->getPrefs();
      freqMHzBLocal = prefs.frequency();
      powerWattsLocal = prefs.power();
    }

    if (freqMHz != NULL)
      *freqMHz = freqMHzBLocal;

    if (powerWatts != NULL)
      *powerWatts = powerWattsLocal;
  }
}

//----------------------------------------------------------------------------

RFGainMeasurement::RFGainMeasurement()
  : RfMeasurement("Gain", "Gain", LOG10)
{ }

double RFGainMeasurement::value(RangeToolState& state) const
{
  const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(state.beginEntity_.node_.get());
  if (beam)
  {
    double azRelLocal;
    double elRelLocal;
    getAngles(&azRelLocal, &elRelLocal, NULL, state);
    return beam->gain(azRelLocal, elRelLocal);
  }
  return 0.0;
}

bool RFGainMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}


//----------------------------------------------------------------------------

RFPowerMeasurement::RFPowerMeasurement()
  : RfMeasurement("Power", "Pwr", RF_POWER)
{ }

double RFPowerMeasurement::value(RangeToolState& state) const
{
  double az;
  double hgtMeters;
  double xmtGaindB;
  double rcvGaindB;
  double rcsSqm;
  double freqMHz;
  double xmtPowerWatts;

  getRfParameters_(state, &az, NULL, &hgtMeters, &xmtGaindB, &rcvGaindB, &rcsSqm, false, &freqMHz, &xmtPowerWatts);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }

  double power = simCore::SMALL_DB_VAL;

  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
  if (rf != NULL)
    power = rf->getReceivedPower(az, slantRngMeters, hgtMeters, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);

  // if simRF::RFPropagationFacade did not return a value, use free space calculation if values available
  if ((power == simCore::SMALL_DB_VAL) && (freqMHz != 0.0) && (xmtPowerWatts != 0.0))
    power = simCore::getRcvdPowerFreeSpace(slantRngMeters, freqMHz, xmtPowerWatts,
    xmtGaindB, rcvGaindB, rcsSqm, 0.0, false);

  return power;
}

bool RFPowerMeasurement::willAccept(const RangeToolState& state) const
{
  // rfPropagation_ is not required, can fall back to free space calculation
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

RFOneWayPowerMeasurement::RFOneWayPowerMeasurement()
  : RfMeasurement("One Way Power", "Pwr(1)", RF_POWER)
{ }

double RFOneWayPowerMeasurement::value(RangeToolState& state) const
{
  double az;
  double hgtMeters;
  double xmtGaindB;
  double rcvGaindB;
  double freqMHz;
  double xmtPowerWatts;

  getRfParameters_(state, &az, NULL, &hgtMeters, &xmtGaindB, &rcvGaindB, NULL, false, &freqMHz, &xmtPowerWatts);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  double power = simCore::SMALL_DB_VAL;

  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;
  if (rf != NULL)
    power = rf->getOneWayPower(az, slantRngMeters, hgtMeters, xmtGaindB, gndRngMeters, rcvGaindB);

  // if simRF::RFPropagationFacade did not return a value, use free space calculation if values available
  if ((power == simCore::SMALL_DB_VAL) && (freqMHz != 0.0) && (xmtPowerWatts != 0.0))
    power = simCore::getRcvdPowerFreeSpace(slantRngMeters, freqMHz, xmtPowerWatts,
    xmtGaindB, 0.0, 1.0, 0.0, true);

  return power;
}

bool RFOneWayPowerMeasurement::willAccept(const RangeToolState& state) const
{
  // rfPropagation_ is not required, can fall back to free space calculation
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

//----------------------------------------------------------------------------

HorizonMeasurement::HorizonMeasurement(const std::string &typeName, const std::string &typeAbbr, const osgEarth::Units &units)
  : Measurement(typeName, typeAbbr, units),
  opticalEffectiveRadius_(DEFAULT_OPTICAL_RADIUS),
  rfEffectiveRadius_(DEFAULT_RF_RADIUS)
{
  // Override the default formatter
  formatter_ = new HorizonFormatter;
}

bool HorizonMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type());
}

void HorizonMeasurement::setEffectiveRadius(double opticalRadius, double rfRadius)
{
  opticalEffectiveRadius_ = opticalRadius;
  rfEffectiveRadius_ = rfRadius;
}

// TODO this needs to be recalculated if an elevation map layer is added or removed
double HorizonMeasurement::calcAboveHorizon_(RangeToolState& state, simCore::HorizonCalculations horizon) const
{
  // Check that they're within range of each other
  double maxRng = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double losRng = simCore::calculateHorizonDist(state.beginEntity_.lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_) +
    simCore::calculateHorizonDist(state.endEntity_.lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_);
  if (maxRng > losRng)
    return 0;

  // Check if obstructed by terrain
  if (state.mapNode_.valid() && state.mapNode_->getMap() && state.mapNode_->getMap()->getElevationPool())
  {
    // If any elevation from beginEntity_ to the terrain at an intermediate point is higher than this, endEntity_ is obstructed by terrain
    double targetElev;
    simCore::calculateAbsAzEl(state.beginEntity_.lla_, state.endEntity_.lla_, NULL, &targetElev, NULL, state.earthModel_, &state.coordConv_);

    simVis::ElevationQueryProxy query(state.mapNode_->getMap(), NULL);

    // Use the los range resolution of the begin entity as the rangeDelta for getting intermediate points
    double rangeDelta = state.beginEntity_.platformHostNode_->getPrefs().losrangeresolution();

    std::vector<simCore::Vec3> points;
    state.intermediatePoints(state.beginEntity_.lla_, state.endEntity_.lla_, rangeDelta, points);
    for (auto iter = points.begin(); iter != points.end(); iter++)
    {
      osgEarth::GeoPoint currGeoPoint;
      // A geopoint is necessary to get elevation.  If conversion fails, disregard this point
      if (!convertCoordToGeoPoint(simCore::Coordinate(simCore::COORD_SYS_LLA, *iter), currGeoPoint, state.mapNode_->getMapSRS()))
        continue;

      double elevation = 0;
      if (query.getElevation(currGeoPoint, elevation))
      {
        currGeoPoint.z() = elevation;
        simCore::Coordinate currLlaPoint;
        convertGeoPointToCoord(currGeoPoint, currLlaPoint, state.mapNode_.get());
        double elev;
        simCore::calculateAbsAzEl(state.beginEntity_.lla_, currLlaPoint.position(), NULL, &elev, NULL, state.earthModel_, &state.coordConv_);
        if (elev > targetElev)
          return 0;
      }
    }
  }

  // Within range and not blocked by terrain
  return 1;
}

//----------------------------------------------------------------------------

RadioHorizonMeasurement::RadioHorizonMeasurement()
  : HorizonMeasurement("Radio Horizon", "Hor(r)", UNITLESS)
{ }

double RadioHorizonMeasurement::value(RangeToolState& state) const
{
  return calcAboveHorizon_(state, simCore::RADAR_HORIZON);
}

//----------------------------------------------------------------------------

OpticalHorizonMeasurement::OpticalHorizonMeasurement()
  : HorizonMeasurement("Optical Horizon", "Hor(o)", UNITLESS)
{ }

double OpticalHorizonMeasurement::value(RangeToolState& state) const
{
  return calcAboveHorizon_(state, simCore::OPTICAL_HORIZON);
}

//----------------------------------------------------------------------------

PodMeasurement::PodMeasurement()
  : RfMeasurement("POD", "POD", PERCENTAGE)
{ }

double PodMeasurement::value(RangeToolState& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return 0.0;

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false, NULL, NULL);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getPOD(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool PodMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
    (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

LossMeasurement::LossMeasurement()
  : RfMeasurement("Loss", "Loss", LOG10)
{ }

double LossMeasurement::value(RangeToolState& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false, NULL, NULL);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getLoss(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool LossMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
    (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

PpfMeasurement::PpfMeasurement()
  : RfMeasurement("PPF", "PPF", LOG10)
{ }

double PpfMeasurement::value(RangeToolState& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false, NULL, NULL);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getPPF(az, gndRngMeters, state.endEntity_.lla_.alt());
}

bool PpfMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
    (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

SnrMeasurement::SnrMeasurement()
  : RfMeasurement("SNR", "SNR", LOG10)
{ }

double SnrMeasurement::value(RangeToolState& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  double xmtGaindB;
  double rcvGaindB;
  double rcsSqm;

  getRfParameters_(state, &az, NULL, NULL, &xmtGaindB, &rcvGaindB, &rcsSqm, false, NULL, NULL);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);
  double altitude = state.endEntity_.lla_.alt();
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }
  return rf->getSNR(az, slantRngMeters, altitude, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
}

bool SnrMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
    (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

CnrMeasurement::CnrMeasurement()
  : RfMeasurement("CNR", "CNR", LOG10)
{ }

double CnrMeasurement::value(RangeToolState& state) const
{
  simRF::RFPropagationFacade* rf = state.beginEntity_.rfPropagation_;

  if (rf == NULL)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, NULL, NULL, NULL, NULL, NULL, false, NULL, NULL);
  //unlike other RF - related calculations, CNR doesn't have a height component
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_.lla_, state.endEntity_.lla_, state.earthModel_, &state.coordConv_);

  return rf->getCNR(az, gndRngMeters);
}

bool CnrMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_.node_->type(), state.endEntity_.node_->type()) &&
    (state.beginEntity_.rfPropagation_ != NULL);
}

//----------------------------------------------------------------------------

RcsMeasurement::RcsMeasurement()
  : RfMeasurement("RCS", "RCS", RF_POWER_SM)
{ }

double RcsMeasurement::value(RangeToolState& state) const
{
  //RCS is a measure of the electrical or reflective area of a target, it is usually expressed in square meters or dBsm.
  double rcsDb;
  getRfParameters_(state, NULL, NULL, NULL, NULL, NULL, &rcsDb, true, NULL, NULL);

  return rcsDb;
}

bool RcsMeasurement::willAccept(const RangeToolState& state) const
{
  return (state.endEntity_.node_->type() == simData::PLATFORM) &&
    (state.endEntity_.node_->getId() == state.endEntity_.platformHostNode_->getId()) &&
    (state.endEntity_.platformHostNode_->getRcs() != NULL);
}


}
