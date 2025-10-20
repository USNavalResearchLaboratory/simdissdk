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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simVis/Beam.h"
#include "simVis/ElevationQueryProxy.h"
#include "simVis/Entity.h"
#include "simVis/Platform.h"
#include "simVis/RFProp/RFPropagationFacade.h"
#include "simVis/SimdisRangeToolState.h"
#include "simVis/Utils.h"
#include "simVis/SimdisMeasurement.h"

namespace simVis
{

std::string HorizonFormatter::stringValue(double value, int precision) const
{
  if (value == 0.0)
    return "Below";

  return "Above";
}

//------------------------------------------------------------------------

RfMeasurement::RfMeasurement(const std::string& name, const std::string& abbr, const simCore::Units& units)
  : RelOriMeasurement(name, abbr, units)
{

}

void RfMeasurement::getRfParameters_(RangeToolState& state, double *azAbs, double *elAbs, double *hgtMeters, double* xmtGaindB, double* rcvGaindB, double* rcs, bool useDb,
  double* freqMHz, double* powerWatts) const
{
  if (azAbs != nullptr || elAbs != nullptr)
  {
    double azAbsLocal;
    double elAbsLocal;
    calculateTrueAngles_(state, &azAbsLocal, &elAbsLocal, nullptr);
    if (azAbs != nullptr)
      *azAbs = azAbsLocal;

    if (elAbs != nullptr)
      *elAbs = elAbsLocal;
  }

  // it is believed that this assertion is true;
  assert(state.beginEntity_ && state.endEntity_);
  const auto simdisBeginState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  const auto simdisEndState = dynamic_cast<SimdisEntityState*>(state.endEntity_);
  // simdisBeginState and simdisEndState can be null, if beginEntity_ or endEntity_ are not SimdisEntityState (possibly base class simVis::EntityState instead)

  if (hgtMeters != nullptr)
  {
    *hgtMeters = 0.0;

    if (simdisBeginState != nullptr)
    {
      const simRF::RFPropagationFacade* rf = simdisBeginState->rfPropagation_;
      if (rf != nullptr)
        *hgtMeters = rf->antennaHeight();
    }
  }

  // Do NOT set RF parameter values from RFPropagationFacade, in order to match the behavior of SIMDIS 9

  if (xmtGaindB != nullptr || rcvGaindB != nullptr)
  {
    double xmtGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    double rcvGaindBLocal = simCore::DEFAULT_ANTENNA_GAIN;
    if (simdisBeginState != nullptr)
    {
      const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(simdisBeginState->node_.get());
      if (beam)
      {
        double azRelLocal;
        double elRelLocal;
        RelOriMeasurement::getAngles(&azRelLocal, &elRelLocal, nullptr, state);
        xmtGaindBLocal = beam->gain(azRelLocal, elRelLocal);
        rcvGaindBLocal = xmtGaindBLocal;
      }
    }

    if (xmtGaindB != nullptr)
      *xmtGaindB = xmtGaindBLocal;

    if (rcvGaindB != nullptr)
      *rcvGaindB = rcvGaindBLocal;
  }

  if (rcs != nullptr)
  {
    double rcsLocal = useDb ? simCore::SMALL_DB_VAL : simCore::SMALL_RCS_SM;
    // To match SIMDIS 9, the end entity must be a platform.
    if (state.endEntity_->type_ == simData::PLATFORM)
    {
      if (simdisEndState != nullptr)
      {
        simCore::RadarCrossSectionPtr rcsPtr = simdisEndState->platformHostNode_->getRcs();
        if (rcsPtr != nullptr)
        {
          // need the angles from the target to the beam source to get the correct rcs values
          double azTarget;
          double elTarget;
          double frequency = simCore::DEFAULT_FREQUENCY;
          simCore::PolarityType type = simCore::POLARITY_UNKNOWN;
          // if begin state node is a beam, use the beam polarity/frequency
          const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(simdisBeginState->node_.get());
          if (beam)
          {
            const auto& prefs = beam->getPrefs();
            type = static_cast<simCore::PolarityType>(prefs.polarity());
            frequency = prefs.frequency();
          }
          else // otherwise, use the end platform's polarity/frequency
          {
            const simVis::PlatformNode* plat = dynamic_cast<const simVis::PlatformNode*>(simdisEndState->node_.get());
            if (plat)
            {
              type = static_cast<simCore::PolarityType>(plat->getPrefs().rcspolarity());
              frequency = plat->getPrefs().rcsfrequency();
            }
            else
              assert(0); // node class should match the type, which was shown to be PLATFORM above

          }
          simCore::calculateRelAzEl(state.endEntity_->lla_, state.endEntity_->ypr_, state.beginEntity_->lla_, &azTarget, &elTarget, nullptr, state.earthModel_, &state.coordConv_);
          if (useDb)
            rcsLocal = rcsPtr->RCSdB(frequency, azTarget, elTarget, type);
          else
            rcsLocal = rcsPtr->RCSsm(frequency, azTarget, elTarget, type);
        }
      }
    }
    *rcs = rcsLocal;
  }

  if (freqMHz != nullptr || powerWatts != nullptr)
  {
    double freqMHzBLocal = 0.0;
    double powerWattsLocal = 0.0;
    const simVis::BeamNode* beam = (simdisBeginState == nullptr) ? nullptr : dynamic_cast<const simVis::BeamNode*>(simdisBeginState->node_.get());
    if (beam)
    {
      const auto& prefs = beam->getPrefs();
      freqMHzBLocal = prefs.frequency();
      powerWattsLocal = prefs.power();
    }

    if (freqMHz != nullptr)
      *freqMHz = freqMHzBLocal;

    if (powerWatts != nullptr)
      *powerWatts = powerWattsLocal;
  }
}

//----------------------------------------------------------------------------

RFGainMeasurement::RFGainMeasurement()
  : RelOriMeasurement("Gain", "Gain", LOG10)
{ }

double RFGainMeasurement::value(RangeToolState& state) const
{
  const auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState)
  {
    const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(simdisState->node_.get());
    if (beam)
    {
      double azRelLocal;
      double elRelLocal;
      getAngles(&azRelLocal, &elRelLocal, nullptr, state);
      return beam->gain(azRelLocal, elRelLocal);
    }
  }
  return 0.0;
}

bool RFGainMeasurement::willAccept(const RangeToolState& state) const
{
  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
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

  getRfParameters_(state, &az, nullptr, &hgtMeters, &xmtGaindB, &rcvGaindB, &rcsSqm, false, &freqMHz, &xmtPowerWatts);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }

  double power = simCore::SMALL_DB_VAL;

  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState)
  {
    simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
    if (rf != nullptr)
      power = rf->getReceivedPower(az, slantRngMeters, hgtMeters, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
  }

  // if simRF::RFPropagationFacade did not return a value, use free space calculation if values available
  if ((power == simCore::SMALL_DB_VAL) && (freqMHz != 0.0) && (xmtPowerWatts != 0.0))
    power = simCore::getRcvdPowerFreeSpace(slantRngMeters, freqMHz, xmtPowerWatts,
    xmtGaindB, rcvGaindB, rcsSqm, 0.0, false);

  return power;
}

bool RFPowerMeasurement::willAccept(const RangeToolState& state) const
{
  // rfPropagation_ is not required, can fall back to free space calculation
  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
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

  getRfParameters_(state, &az, nullptr, &hgtMeters, &xmtGaindB, &rcvGaindB, nullptr, false, &freqMHz, &xmtPowerWatts);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);

  double power = simCore::SMALL_DB_VAL;

  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState)
  {
    simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
    if (rf != nullptr)
      power = rf->getOneWayPower(az, slantRngMeters, hgtMeters, xmtGaindB, gndRngMeters, rcvGaindB);
  }

  // if simRF::RFPropagationFacade did not return a value, use free space calculation if values available
  if ((power == simCore::SMALL_DB_VAL) && (freqMHz != 0.0) && (xmtPowerWatts != 0.0))
    power = simCore::getRcvdPowerFreeSpace(slantRngMeters, freqMHz, xmtPowerWatts,
    xmtGaindB, 0.0, 1.0, 0.0, true);

  return power;
}

bool RFOneWayPowerMeasurement::willAccept(const RangeToolState& state) const
{
  // rfPropagation_ is not required, can fall back to free space calculation
  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

HorizonMeasurement::HorizonMeasurement(const std::string &typeName, const std::string &typeAbbr, const simCore::Units &units)
  : Measurement(typeName, typeAbbr, units),
  opticalEffectiveRadius_(DEFAULT_OPTICAL_RADIUS),
  rfEffectiveRadius_(DEFAULT_RF_RADIUS)
{
  // Override the default formatter
  formatter_ = new HorizonFormatter;
}

bool HorizonMeasurement::willAccept(const RangeToolState& state) const
{
  return isEntityToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
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
  double maxRng = simCore::calculateSlant(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  double losRng = simCore::calculateHorizonDist(state.beginEntity_->lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_) +
    simCore::calculateHorizonDist(state.endEntity_->lla_, horizon, opticalEffectiveRadius_, rfEffectiveRadius_);
  if (maxRng > losRng)
    return 0;

  SimdisEntityState* simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return 0;

  // Check if obstructed by terrain
  if (state.mapNode_.valid() && state.mapNode_->getMap() && state.mapNode_->getMap()->getElevationPool())
  {
    // If any elevation from beginEntity_ to the terrain at an intermediate point is higher than this, endEntity_ is obstructed by terrain
    double targetElev;
    simCore::calculateAbsAzEl(state.beginEntity_->lla_, state.endEntity_->lla_, nullptr, &targetElev, nullptr, state.earthModel_, &state.coordConv_);

    // find the elevation pool and establish a local working set
    // since we'll be doing multiple spatially-similar queries
    osg::ref_ptr<osgEarth::ElevationPool> pool = state.mapNode_->getMap()->getElevationPool();
    osgEarth::ElevationPool::WorkingSet workingSet;

    // Use the los range resolution of the begin entity as the rangeDelta for getting intermediate points
    double rangeDelta = simdisState->platformHostNode_->getPrefs().losrangeresolution();
    std::vector<simCore::Vec3> points;
    state.intermediatePoints(state.beginEntity_->lla_, state.endEntity_->lla_, rangeDelta, points);

    osgEarth::GeoPoint currGeoPoint(state.mapNode_->getMapSRS()->getGeographicSRS(), 0, 0, 0, osgEarth::ALTMODE_ABSOLUTE);

    // Iterate over the points, sampling the elevation at each until the target becomes invisible:
    for (std::vector<simCore::Vec3>::const_iterator iter = points.begin(); iter != points.end(); ++iter)
    {
      currGeoPoint.x() = iter->lon() * simCore::RAD2DEG;
      currGeoPoint.y() = iter->lat() * simCore::RAD2DEG;

      osgEarth::ElevationSample sample = pool->getSample(currGeoPoint, osgEarth::Distance(1.0, osgEarth::Units::METERS), &workingSet);
      if (sample.hasData())
      {
        currGeoPoint.z() = sample.elevation().as(osgEarth::Units::METERS);
        simCore::Vec3 currLLA(currGeoPoint.y() * simCore::DEG2RAD, currGeoPoint.x() * simCore::DEG2RAD, currGeoPoint.z());

        double relativeElev;
        simCore::calculateAbsAzEl(state.beginEntity_->lla_, currLLA, nullptr, &relativeElev, nullptr, state.earthModel_, &state.coordConv_);
        if (relativeElev > targetElev)
          return 0;
      }
    }
  }

  // Within range and not blocked by terrain
  return 1;
}

//----------------------------------------------------------------------------

RadioHorizonMeasurement::RadioHorizonMeasurement()
  : HorizonMeasurement("Radio Horizon", "Hor(r)", simCore::Units::UNITLESS)
{ }

double RadioHorizonMeasurement::value(RangeToolState& state) const
{
  return calcAboveHorizon_(state, simCore::RADAR_HORIZON);
}

//----------------------------------------------------------------------------

OpticalHorizonMeasurement::OpticalHorizonMeasurement()
  : HorizonMeasurement("Optical Horizon", "Hor(o)", simCore::Units::UNITLESS)
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
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return 0.0;

  simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
  if (rf == nullptr)
    return 0.0;

  double az;
  getRfParameters_(state, &az, nullptr, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);

  return rf->getPOD(az, gndRngMeters, state.endEntity_->lla_.alt());
}

bool PodMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return false;

  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

LossMeasurement::LossMeasurement()
  : RfMeasurement("Loss", "Loss", LOG10)
{ }

double LossMeasurement::value(RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
  if (rf == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, nullptr, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);

  return rf->getLoss(az, gndRngMeters, state.endEntity_->lla_.alt());
}

bool LossMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return false;

  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

PpfMeasurement::PpfMeasurement()
  : RfMeasurement("PPF", "PPF", LOG10)
{ }

double PpfMeasurement::value(RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
  if (rf == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, nullptr, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);

  return rf->getPPF(az, gndRngMeters, state.endEntity_->lla_.alt());
}

bool PpfMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return false;

  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

SnrMeasurement::SnrMeasurement()
  : RfMeasurement("SNR", "SNR", LOG10)
{ }

double SnrMeasurement::value(RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
  if (rf == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  double xmtGaindB;
  double rcvGaindB;
  double rcsSqm;

  getRfParameters_(state, &az, nullptr, nullptr, &xmtGaindB, &rcvGaindB, &rcsSqm, false, nullptr, nullptr);
  double slantRngMeters = simCore::calculateSlant(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);
  double altitude = state.endEntity_->lla_.alt();
  if (rcsSqm == simCore::SMALL_RCS_SM)
  {
    // no valid rcs data found; use default 1.0 sqm as documented in SIMDIS User Manual
    rcsSqm = 1.0;
  }
  return rf->getSNR(az, slantRngMeters, altitude, xmtGaindB, rcvGaindB, rcsSqm, gndRngMeters);
}

bool SnrMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return false;

  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

CnrMeasurement::CnrMeasurement()
  : RfMeasurement("CNR", "CNR", LOG10)
{ }

double CnrMeasurement::value(RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  simRF::RFPropagationFacade* rf = simdisState->rfPropagation_;
  if (rf == nullptr)
    return static_cast<double>(simCore::SMALL_DB_VAL);

  double az;
  getRfParameters_(state, &az, nullptr, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
  //unlike other RF - related calculations, CNR doesn't have a height component
  double gndRngMeters = simCore::calculateGroundDist(state.beginEntity_->lla_, state.endEntity_->lla_, state.earthModel_, &state.coordConv_);

  return rf->getCNR(az, gndRngMeters);
}

bool CnrMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.beginEntity_);
  if (simdisState == nullptr)
    return false;

  return isBeamToEntity_(state.beginEntity_->type_, state.endEntity_->type_);
}

//----------------------------------------------------------------------------

RcsMeasurement::RcsMeasurement()
  : RfMeasurement("RCS", "RCS", RF_POWER_SM)
{ }

double RcsMeasurement::value(RangeToolState& state) const
{
  //RCS is a measure of the electrical or reflective area of a target, it is usually expressed in square meters or dBsm.
  double rcsDb;
  getRfParameters_(state, nullptr, nullptr, nullptr, nullptr, nullptr, &rcsDb, true, nullptr, nullptr);

  return rcsDb;
}

bool RcsMeasurement::willAccept(const RangeToolState& state) const
{
  auto simdisState = dynamic_cast<SimdisEntityState*>(state.endEntity_);
  if (simdisState == nullptr)
    return false;

  return (simdisState->type_ == simData::PLATFORM) &&
    (simdisState->node_->getId() == simdisState->platformHostNode_->getId()) &&
    (simdisState->platformHostNode_->getRcs() != nullptr);
}


}
