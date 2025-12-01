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
#ifndef SIMCORE_EM_DECIBEL_H
#define SIMCORE_EM_DECIBEL_H

#include <cassert>
#include <complex>
#include "simCore/Calc/MathConstants.h"

namespace simCore
{

  /// Converts an dB value to linear notation
  /**
  * Converts an dB value to linear notation
  * @param[in ] in a value in dB
  * @return a linear value, can be 0 if in is -std::numeric_limits<Type>::infinity()
  */
  template <class Type>
  inline Type dB2Linear(Type in) { return static_cast<Type>(pow(10., in * 0.1)); }

  /**
  * Converts an dBm value to linear notation
  * @param[in ] in a value in dBm
  * @return a linear value, can be 0 if in is -std::numeric_limits<Type>::infinity()
  */
  template <class Type>
  inline Type dBm2Linear(Type in) { return pow(10., (in - 30.) * 0.1); }

  /**
  * Converts an linear value to dB
  * @param[in ] in a linear value
  * @return a dB value
  */
  template <class Type>
  inline Type linear2dB(Type in)
  {
    assert(in >= 0);
    constexpr Type smalldB = -300.;
    return (in > 0) ? (Type)(10. * log10(in)) : smalldB;
  }

  /**
  * Converts an linear value to dBm
  * @param[in ] in a linear value
  * @return a dBm value
  */
  template <class Type>
  inline Type linear2dBm(Type in)
  {
    assert(in >= 0);
    constexpr Type smalldB = -300;
    return (in > 0) ? (30 + 10 * log10(in)) : smalldB;
  }

  /**
  * Converts a voltage value to dBm (milliwatts)
  * @param[in ] value a voltage value in volts
  * @param[in ] impedance an impedance value in ohms
  * @return a value in dB milliwatts
  */
  template <class Type>
  inline Type voltage2dBm(Type value, Type impedance)
  {
    return linear2dBm((value * value)/impedance);
  }

  /**
  * Converts a voltage value to dB (Watts)
  * @param[in ] value a voltage value in volts
  * @param[in ] impedance an impedance value in ohms
  * @return a value in dB Watts
  */
  template <class Type>
  inline Type voltage2dB(Type value, Type impedance)
  {
    return linear2dB((value * value)/impedance);
  }

  /**
  * Converts a dB Watts value to voltage (V)
  * @param[in ] value assumes power (W), then voltage = sqrt(power)
  * @return a voltage in V
  */
  template <class Type>
  inline Type dB2Voltage(Type value)
  {
    return (sqrt(dB2Linear(value)));
  }

  /**
  * Converts a complex number into a dB value
  * @param[in ] real real component in the E-field notation
  * @param[in ] img imaginary component in the E-field notation
  * @return a value in dB
  */
  template <class Type>
  inline Type complex2dB(Type real, Type img)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters, then to dB
    return (linear2dB(4.0 * M_PI * ((real*real) + (img*img))));
  }

  /**
  * Converts a complex number into a dB value
  * @param[in ] real real component in the E-field notation
  * @param[in ] img imaginary component in the E-field notation
  * @return a value in dBm
  */
  template <class Type>
  inline Type complex2dBm(Type real, Type img)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters, then to dB
    return (linear2dBm(4.0 * M_PI * ((real*real) + (img*img))));
  }

  /**
  * Converts a complex number represented as a magnitude and phase into a dB value
  * @param[in ] mag magnitude
  * @param[in ] phase phase value in radians
  * @return a value in dB
  */
  template <class Type>
  inline Type magPhase2dB(Type mag, Type phase)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters, then to dB
    return (linear2dB(4.0 * M_PI *
      ((mag * cos(phase))*(mag * cos(phase))) +
      ((mag * sin(phase))*(mag * sin(phase)))));
  }

  /**
  * Converts a complex number represented as a magnitude and phase into a dBm value
  * @param[in ] mag magnitude
  * @param[in ] phase phase value in radians
  * @return a value in dBm
  */
  template <class Type>
  inline Type magPhase2dBm(Type mag, Type phase)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters, then to dB
    return (linear2dBm(4.0 * M_PI *
      ((mag * cos(phase))*(mag * cos(phase))) +
      ((mag * sin(phase))*(mag * sin(phase)))));
  }

  /**
  * Converts a complex number into a square meters value
  * @param[in ] real real component in the E-field notation
  * @param[in ] img imaginary component in the E-field notation
  * @return a value in square meters
  */
  template <class Type>
  inline Type complex2Sqm(Type real, Type img)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters
    return (4.0 * M_PI * ((real*real) + (img*img)));
  }

  /**
  * Converts a complex number represented as a magnitude and phase into a square meters value
  * @param[in ] mag magnitude
  * @param[in ] phase phase value in radians
  * @return a value in square meters
  */
  template <class Type>
  inline Type magPhase2Sqm(Type mag, Type phase)
  {
    // assumes complex number is represented in the E-field notation
    // hence the 4.0 * M_PI to convert to square meters
    return (4.0 * M_PI *
      ((mag * cos(phase))*(mag * cos(phase))) +
      ((mag * sin(phase))*(mag * sin(phase))));
  }

  /// small dB/Sq meter values that are used for values near zero
  inline constexpr float SMALL_DB_VAL = -300.f;
  /// Small radar cross section dB/Sq meter value for near-zero
  inline constexpr float SMALL_RCS_SM = static_cast<float>(1.E-30);
  /// comparison value used to account for conversion errors from double to float
  inline constexpr float SMALL_DB_COMPARE = SMALL_DB_VAL + 0.01f;
}

#endif /* SIMCORE_EM_DECIBEL_H */
