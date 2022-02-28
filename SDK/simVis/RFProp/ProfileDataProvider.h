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
#ifndef SIMVIS_RFPROP_PROFILE_DATA_PROVIDER_H
#define SIMVIS_RFPROP_PROFILE_DATA_PROVIDER_H

#include "osg/Referenced"
#include "simCore/Common/Common.h"

namespace simRF
{

/** scale factor used to convert from shorts in LUT to actual value/float */
static const double SCALE_FACTOR = 10.0;
/** Initialization value for AREPS to use for loss values when AREPS reports an erroneous initialization value */
static const short INIT_VALUE = -32768;
/** Height returned by AREPS for values along the ground, where propagation is more complicated. */
static const short GROUND_VALUE = -32766;
/** Erroneous initialization sentinel value that AREPS returns when cells are not initialized. */
static const short ERRONEOUS_INIT_VALUE = -32678;
/** Value to represent invalid data */
static const short INVALID_VALUE = -32767;

/**
 * ProfileDataProvider provides data along a height vs range profile sample.  The height samples and range samples are
 * expected to be at consistent intervals.
 */
class ProfileDataProvider: public osg::Referenced
{
public:
  /// enumeration that describes different data types
  enum ThresholdType
  {
    THRESHOLDTYPE_POD = 0,          ///< Thresholds based on Probability of Detection (POD) levels
    THRESHOLDTYPE_LOSS = 1,         ///< Thresholds based on propagation loss levels
    THRESHOLDTYPE_FACTOR = 2,       ///< Thresholds based on Pattern Propagation Factor (PPF) levels
    THRESHOLDTYPE_SNR = 3,          ///< Thresholds based on Signal to Noise Ratio (SNR) levels
    THRESHOLDTYPE_CNR = 4,          ///< Thresholds based on Clutter to Noise Ratio (CNR) levels
    THRESHOLDTYPE_ONEWAYPOWER = 5,  ///< Thresholds based on one way power levels
    THRESHOLDTYPE_RECEIVEDPOWER = 6,  ///< Thresholds based on two way power levels
    THRESHOLDTYPE_NONE = 7          ///< initial value
  };


public:
  /** Gets the number of range values */
  virtual unsigned int getNumRanges() const = 0;

  /** Gets the spacing between range samples */
  virtual double getRangeStep() const = 0;

  /** Gets the min range */
  virtual double getMinRange() const = 0;

  /** Gets the max range */
  virtual double getMaxRange() const = 0;

  /** Gets the number of height values */
  virtual unsigned int getNumHeights() const = 0;

  /** Gets the min height */
  virtual double getMinHeight() const = 0;

  /** Gets the max height */
  virtual double getMaxHeight() const = 0;

  /** Gets the spacing between height samples */
  virtual double getHeightStep() const = 0;

  /**
   * Gets the value on this Profile
   * @param heightIndex The height index of the desired sample
   * @param rangeIndex The range index of the desired sample
   * @return Scaled LUT value at the specified height and range
   */
  virtual double getValueByIndex(unsigned int heightIndex, unsigned int rangeIndex) const = 0;

  /**
   * Interpolates the value on this Profile at the given height and range.
   * @param hgtMeters The height index of the desired sample, in meters
   * @param gndRngMeters The range of the desired sample, in meters
   * @return value at the specified height and range
   */
  virtual double interpolateValue(double hgtMeters, double gndRngMeters) const = 0;

  /** Retrieves the threshold type value */
  virtual ThresholdType getType() const { return type_; }

protected:
  /// osg::Referenced-derived
  virtual ~ProfileDataProvider() {}

  /** Sets the threshold type value */
  virtual void setType_(ThresholdType type) { type_ = type; }

private:
  ThresholdType type_;
};
}

#endif /* SIMVIS_RFPROP_PROFILE_DATA_PROVIDER_H */
