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
#ifndef SIMCORE_EM_ELECTRO_MAG_RANGE_H
#define SIMCORE_EM_ELECTRO_MAG_RANGE_H

#include <complex>
#include <cmath>
#include "simCore/Calc/MathConstants.h"
#include "simCore/EM/Constants.h"

namespace simCore
{

  /**
  * Computes path length phase
  * @param[in ] range a double in m
  * @param[in ] wavelength a double in m
  * @return phase a double in radians
  */
  inline double range2Phase(double range, double wavelength)
  { return fmod(M_TWOPI * range / wavelength, M_TWOPI); }

  /**
  * Computes wavelength for a particular frequency
  * @param[in ] frequency a double in Hz
  * @return wavelength a double in m
  */
  inline double frequency2Wavelength(double frequency)
  { return LIGHT_SPEED_AIR / frequency; }

  /**
  * Computes frequency for a particular wavelength
  * @param[in ] wavelength a double in m
  * @return frequency a double in Hz
  */
  inline double wavelength2Frequency(double wavelength)
  { return LIGHT_SPEED_AIR / wavelength; }

  /**
  * Calculates voltage for a particular input power
  * @param[in ] power a double in watts
  * @param[in ] resistance a double in ohms
  * @return voltage a double in V
  */
  inline double power2Volts(double power, double resistance=1.)
  { return sqrt(power *resistance); }

  /**
  * Calculates voltage for a particular input power
  * @param[in ] power a std::complex template value
  * @param[in ] resistance a double in ohms
  * @return voltage as a template value in V
  */
  template <class Type>
  inline std::complex<Type> power2Volts(const std::complex<Type> &power, double resistance=1.)
  { return std::polar(sqrt(std::abs(power) * resistance), std::arg(power)); }

  /**
  * Computes free space two-way range for an EM signal in a vacuum given the elapsed time
  * @param[in ] time travel time in seconds
  * @return two-way range in meters
  */
  inline double timeToRange(double time)
  { return time * LIGHT_SPEED_VACUUM * .5; }

  /**
  * Computes free space time for an EM signal in a vacuum to travel the given two-way range
  * @param[in ] range two-way range in meters
  * @return travel time in seconds
  */
  inline double rangeToTime(double range)
  { return range / (LIGHT_SPEED_VACUUM * .5); }

  /**
  * Computes free space two-way range for an EM signal in air given the elapsed time
  * @param[in ] time travel time in seconds
  * @return two-way range in meters
  */
  inline double timeToRangeAir(double time)
  { return time * LIGHT_SPEED_AIR * .5; }

  /**
  * Computes free space time for an EM signal in air to travel the given two-way range
  * @param[in ] range two-way range in meters
  * @return travel time in seconds
  */
  inline double rangeToTimeAir(double range)
  { return range / (LIGHT_SPEED_AIR * .5); }

}

#endif /* SIMCORE_EM_ELECTRO_MAG_RANGE_H */
