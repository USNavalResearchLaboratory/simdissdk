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
#include <cassert>
#include "simCore/EM/Decibel.h"
#include "simVis/RFProp/SNRDataProvider.h"

namespace simRF
{
SNRDataProvider::SNRDataProvider(const TwoWayPowerDataProvider* twoWayPowerProvider, const RadarParametersPtr radarParameters)
  : FunctionalProfileDataProvider(twoWayPowerProvider),
  radarParameters_(radarParameters)
{
  assert(twoWayPowerProvider);
  twoWayPowerProvider_ = twoWayPowerProvider;
  setType_(ProfileDataProvider::THRESHOLDTYPE_SNR);
}

SNRDataProvider::~SNRDataProvider()
{
  twoWayPowerProvider_ = NULL;
}

double SNRDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  double rcvPowerdB = FunctionalProfileDataProvider::templateGetValueByIndex_(heightIndex, rangeIndex);
  return (rcvPowerdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL : (rcvPowerdB - radarParameters_->noisePowerdB);
}

double SNRDataProvider::interpolateValue(double height, double range) const
{
  double rcvPowerdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return (rcvPowerdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL : (rcvPowerdB - radarParameters_->noisePowerdB);
}

double SNRDataProvider::getSNR(double height, double range, double slantRangeM, double xmtGaindB, double rcvGaindB, double rcsSqm) const
{
  double rcvPowerdB = twoWayPowerProvider_->getTwoWayPower(height, range, slantRangeM, xmtGaindB, rcvGaindB, rcsSqm);
  return (rcvPowerdB <= simCore::SMALL_DB_VAL) ? simCore::SMALL_DB_VAL : (rcvPowerdB - radarParameters_->noisePowerdB);
}

}

