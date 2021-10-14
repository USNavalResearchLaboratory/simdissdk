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
#include "simCore/Calc/Math.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Propagation.h"
#include "simVis/RFProp/PPFDataProvider.h"

namespace simRF
{
PPFDataProvider::PPFDataProvider(const ProfileDataProvider* templateProvider, const RadarParametersPtr radarParameters)
  : FunctionalProfileDataProvider(templateProvider),
  radarParameters_(radarParameters)
{
  setType_(ProfileDataProvider::THRESHOLDTYPE_FACTOR);
}

double PPFDataProvider::getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const
{
  const double lossdB = FunctionalProfileDataProvider::templateGetValueByIndex_(heightIndex, rangeIndex);
  return getPPF_(lossdB, FunctionalProfileDataProvider::getHeight_(heightIndex), FunctionalProfileDataProvider::getRange_(rangeIndex));
}

double PPFDataProvider::interpolateValue(double height, double range) const
{
  const double lossdB = FunctionalProfileDataProvider::templateInterpolateValue_(height, range);
  return getPPF_(lossdB, height, range);
}

double PPFDataProvider::getPPF_(double lossdB, double height, double range) const
{
  const double slantRangeM = sqrt(simCore::square(range) + simCore::square(height));
  const double ppf_dB = simCore::lossToPpf(slantRangeM, radarParameters_->freqMHz, lossdB);
  return ((ppf_dB != simCore::SMALL_DB_VAL) ? ppf_dB : simRF::INVALID_VALUE);
}

}
