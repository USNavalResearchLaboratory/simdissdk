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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"
#include "simVis/RFProp/TwoWayPowerDataProvider.h"

namespace simRF
{
TwoWayPowerDataProvider::TwoWayPowerDataProvider(const ProfileDataProvider* templateProvider, const RadarParametersPtr radarParameters)
  : FunctionalProfileDataProvider(templateProvider),
  radarParameters_(radarParameters)
{
  setType_(ProfileDataProvider::THRESHOLDTYPE_RECEIVEDPOWER);
}

double TwoWayPowerDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  double ppfdB = FunctionalProfileDataProvider::templateGetValueByIndex_(heightIndex, rangeIndex);
  return (ppfdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL :
    getTwoWayPower_(ppfdB, FunctionalProfileDataProvider::getRange_(rangeIndex), radarParameters_->antennaGaindBi, radarParameters_->antennaGaindBi);
}

double TwoWayPowerDataProvider::interpolateValue(double height, double range) const
{
  double ppfdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return (ppfdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL :
    getTwoWayPower_(ppfdB, range, radarParameters_->antennaGaindBi, radarParameters_->antennaGaindBi);
}

double TwoWayPowerDataProvider::getTwoWayPower(double height, double range, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm) const
{
  double ppfdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return (ppfdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL :
    getTwoWayPower_(ppfdB, slantRangeM, xmtGaindB, rcvGaindB, rcsSqm);
}

double TwoWayPowerDataProvider::getTwoWayPower_(double ppfdB, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm) const
{
  return simCore::getRcvdPowerBlake(
    slantRangeM,
    radarParameters_->freqMHz,
    radarParameters_->xmtPowerW,
    xmtGaindB,
    rcvGaindB,
    rcsSqm,
    ppfdB,
    radarParameters_->systemLossdB,
    false);
}

}

