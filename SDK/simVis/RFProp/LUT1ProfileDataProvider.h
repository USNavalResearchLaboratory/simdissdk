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
#ifndef SIMVIS_RFPROP_LUT1_PROFILE_DATA_PROVIDER_H
#define SIMVIS_RFPROP_LUT1_PROFILE_DATA_PROVIDER_H

#include "simCore/Common/Common.h"
#include "simCore/LUT/LUT1.h"
#include "simVis/RFProp/ProfileDataProvider.h"

namespace simRF
{

/** LUT1ProfileDataProvider provides Profile data using a 1D look-up table (LUT1) */
class SDKVIS_EXPORT LUT1ProfileDataProvider: public ProfileDataProvider
{
public:
  /**
   * Creates a new LUT1ProfileDataProvider, the provider takes ownership of the LUT1
   * @param lut The look-up table to use as a data source, short is sufficient precision for loss values in dB
   * @param scalar Scalar value to store normalized loss values converted from dB, values typically stored as cB
   */
  LUT1ProfileDataProvider(simCore::LUT::LUT1<short> *lut, double scalar = 0.1);

  /**
   * Creates a new LUT1ProfileDataProvider, the provider takes ownership of the LUT1
   * @param lut The look-up table to use as a data source, short is sufficient precision for loss values in dB
   * @param type ProfileDataProvider type
   * @param scalar Scalar value to store normalized loss values converted from dB, values typically stored as cB
   */
  LUT1ProfileDataProvider(simCore::LUT::LUT1<short> *lut, ProfileDataProvider::ThresholdType type, double scalar = 0.1);

  /**@name data access
   * @{
   */
  virtual unsigned int getNumRanges() const;
  virtual double getRangeStep() const;
  virtual double getMinRange() const;
  virtual double getMaxRange() const;

  virtual unsigned int getNumHeights() const;
  virtual double getMinHeight() const;
  virtual double getMaxHeight() const;
  virtual double getHeightStep() const;
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const;
  ///@}

  /**
  * Interpolates the value on this Profile at the given height and range.
  * @param hgtMeters The height index of the desired sample, in meters
  * @param gndRngMeters The range of the desired sample, in meters
  * @return value at the specified height and range
  */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const;

protected:
  /// osg::Referenced-derived
  virtual ~LUT1ProfileDataProvider();

protected:
  simCore::LUT::LUT1<short> *lut_;  ///< 1D look-up table based on range, stored value in centibels (cB)
  double scalar_;                   ///< 1D table scalar value, doubles are scaled to a short for efficient memory use
};

}

#endif /* SIMVIS_RFPROP_LUT1_PROFILE_DATA_PROVIDER_H */
