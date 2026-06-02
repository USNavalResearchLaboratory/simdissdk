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
#ifndef SIMVIS_SIMDIS_MEASUREMENT_H
#define SIMVIS_SIMDIS_MEASUREMENT_H

#include <string>
#include <vector>
#include "simCore/Calc/CoordinateSystem.h"
#include "simVis/Measurement.h"

namespace simVis
{
/**
* Class for formatting Above/Below into a string.
* Intended for use with RadioHorizonMeasurement and OpticalHorizonMeasurement
*/
class SDKVIS_EXPORT HorizonFormatter : public ValueFormatter
{
public:
  virtual ~HorizonFormatter() = default;
  /**
  * Formats the value into a string
  * @param value The value that needs to be converted into a string.
  * @param precision Ignored
  * @return The value as a string
  */
  std::string stringValue(double value, int precision) const override;
};


/// Base class for RF calculations
class SDKVIS_EXPORT RfMeasurement : public RelOriMeasurement
{
public:
  /**
  * Constructor.
  * @param name Name of the type.
  * @param abbr The type abbr.
  * @param units The units.
  */
  RfMeasurement(const std::string& name, const std::string& abbr, const simCore::Units& units);

protected:
  /// osg::Referenced-derived
  virtual ~RfMeasurement() = default;
  /**
  * Calculates RF parameters from the given state
  * @param state State information for both the begin and end entities
  * @param azAbs The absolute true azimuth, in radians, between the begin entity and the end entity
  * @param elAbs The absolute elevation, in radians, between the begin entity and the end entity
  * @param hgtMeters The height, in meters, of the antenna
  * @param xmtGaindB The gain, in dB, of the transmit antenna
  * @param rcvGaindB The gain, in dB, of the receive antenna
  * @param rcsSqm RCS db if useDb is true or RCS dBsm if useDb is false;
  * @param useDb Flag for selecting type of rcs value to return
  * @param freqMHz The frequency, in MHz, of the RF signal
  * @param powerWatts The power, in watts, of the RF signal
  */
  void getRfParameters_(RangeToolState& state, double* azAbs, double* elAbs, double *hgtMeters, double* xmtGaindB, double* rcvGaindB, double* rcsSqm, bool useDb,
    double* freqMHz, double* powerWatts) const;
};

/// Antenna Gain
class SDKVIS_EXPORT RFGainMeasurement : public RelOriMeasurement
{
public:
  RFGainMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~RFGainMeasurement() = default;
};

/// Received Power
class SDKVIS_EXPORT RFPowerMeasurement : public RfMeasurement
{
public:
  RFPowerMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~RFPowerMeasurement() = default;
};

/// One-Way Power
class SDKVIS_EXPORT RFOneWayPowerMeasurement : public RfMeasurement
{
public:
  RFOneWayPowerMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~RFOneWayPowerMeasurement() = default;
};

/// Base class for Horizon calculations
class SDKVIS_EXPORT HorizonMeasurement : public Measurement
{
public:

  /**
  * Constructor.
  * @param typeName Name of the type.
  * @param typeAbbr The type abbr.
  * @param units The units.
  */
  HorizonMeasurement(const std::string &typeName, const std::string &typeAbbr, const simCore::Units &units);
  bool willAccept(const RangeToolState& state) const override;

  /**
  * Set effective Earth radius scalars for optical and rf horizon measurement.
  * @param opticalRadius new Earth radius scalar for optical horizon calculations
  * @param rfRadius new Earth radius scalar for rf horizon calculations
  */
  void setEffectiveRadius(double opticalRadius, double rfRadius);

protected:
  /// osg::Referenced-derived
  virtual ~HorizonMeasurement() = default;

  /**
  * Calculates if the end entity is above or below the horizon
  * @param state Information on both the begin entity and end entity
  * @param horizon Type of calculation
  * @return 0 = below horizon and 1 = above horizon
  */
  virtual double calcAboveHorizon_(RangeToolState& state, simCore::HorizonCalculations horizon) const;

private:
  double opticalEffectiveRadius_;
  double rfEffectiveRadius_;
};

/// Radio Horizon
class SDKVIS_EXPORT RadioHorizonMeasurement : public HorizonMeasurement
{
public:
  RadioHorizonMeasurement();
  double value(RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~RadioHorizonMeasurement() = default;
};

/// Optical Horizon
class SDKVIS_EXPORT OpticalHorizonMeasurement : public HorizonMeasurement
{
public:
  OpticalHorizonMeasurement();
  double value(RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~OpticalHorizonMeasurement() = default;
};

/// Probability of Detection (PoD)
class SDKVIS_EXPORT PodMeasurement : public RfMeasurement
{
public:
  PodMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~PodMeasurement() = default;
};

/// Propagation Loss
class SDKVIS_EXPORT LossMeasurement : public RfMeasurement
{
public:
  LossMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~LossMeasurement() = default;
};

/// Pattern Propagation Factor (PPF)
class SDKVIS_EXPORT PpfMeasurement : public RfMeasurement
{
public:
  PpfMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~PpfMeasurement() = default;
};

/// Signal to Noise (SNR)
class SDKVIS_EXPORT SnrMeasurement : public RfMeasurement
{
public:
  SnrMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~SnrMeasurement() = default;
};

/// Clutter to Noise (CNR)
class SDKVIS_EXPORT CnrMeasurement : public RfMeasurement
{
public:
  CnrMeasurement();
  //unlike other RF - related calculations, CNR doesn't have a height component
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~CnrMeasurement() = default;
};

/// Radar Cross Section (RCS)
class SDKVIS_EXPORT RcsMeasurement : public RfMeasurement
{
public:
  RcsMeasurement();
  double value(RangeToolState& state) const override;
  bool willAccept(const RangeToolState& state) const override;

protected:
  /// osg::Referenced-derived
  virtual ~RcsMeasurement() = default;
};

}

#endif
