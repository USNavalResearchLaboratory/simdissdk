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
#ifndef SIMCORE_EM_CONSTANTS_H
#define SIMCORE_EM_CONSTANTS_H 1

#include <algorithm>
#include <string>

namespace simCore
{
  ///Various speed of light constants
  ///http://physics.nist.gov/cgi-bin/cuu/Value?c|search_for=speed+of+light+vacuum
  inline constexpr double LIGHT_SPEED_VACUUM = 2.99792458e8;  /**< (m/s) in vacuum */
  ///c/n  via index of refraction of air n=1.0003:  CRC Handbook of Chemistry and Physics
  inline constexpr double LIGHT_SPEED_AIR = 299702547.23582925122463261021693;      /**< (m/s) in air */
  ///Radar range equation constant: (4 * M_PI) ^ 3
  inline constexpr double RRE_CONSTANT = 1984.40170753918820;

  /// Default peak power setting for RCS and Antenna patterns (W)
  inline constexpr float DEFAULT_PEAK_POWER = 2000;   // W
  /// Default frequency setting for RCS and Antenna patterns (MHz)
  inline constexpr float DEFAULT_FREQUENCY = 7000;    // MHz
  /// Default antenna gain setting for RCS and Antenna patterns (dB)
  inline constexpr float DEFAULT_ANTENNA_GAIN = 20;   // dB

  ///Enumeration of possible polarization types
  enum PolarityType
  {
    POLARITY_UNKNOWN=0,
    POLARITY_HORIZONTAL,
    POLARITY_VERTICAL,
    POLARITY_CIRCULAR,
    POLARITY_HORZVERT,
    POLARITY_VERTHORZ,
    POLARITY_LEFTCIRC,
    POLARITY_RIGHTCIRC,
    POLARITY_LINEAR
  };

  /** Polarity string for unknown polarity */
  static const std::string POLARITY_STRING_UNKNOWN = "UNKNOWN";
  /** Polarity string for horizontal polarity */
  static const std::string POLARITY_STRING_HORIZONTAL = "HORIZONTAL";
  /** Polarity string for vertical polarity */
  static const std::string POLARITY_STRING_VERTICAL = "VERTICAL";
  /** Polarity string for circular polarity */
  static const std::string POLARITY_STRING_CIRCULAR = "CIRCULAR";
  /** Polarity string for horizontal/vertical polarity */
  static const std::string POLARITY_STRING_HORZVERT = "HORZVERT";
  /** Polarity string for vertical/horizontal polarity */
  static const std::string POLARITY_STRING_VERTHORZ = "VERTHORZ";
  /** Polarity string for left circular polarity */
  static const std::string POLARITY_STRING_LEFTCIRC = "LEFTCIRC";
  /** Polarity string for right circular polarity */
  static const std::string POLARITY_STRING_RIGHTCIRC = "RIGHTCIRC";
  /** Polarity string for linear polarity */
  static const std::string POLARITY_STRING_LINEAR = "LINEAR";

  /**
  * Returns the string representation of the polarity type
  * @param[in ] polarity PolarityType
  * @return string representation of the polarity type
  */
  inline const std::string polarityString(PolarityType polarity)
  {
    switch (polarity)
    {
      case POLARITY_HORIZONTAL:
        return POLARITY_STRING_HORIZONTAL;
      case POLARITY_VERTICAL:
        return POLARITY_STRING_VERTICAL;
      case POLARITY_CIRCULAR:
        return POLARITY_STRING_CIRCULAR;
      case POLARITY_HORZVERT:
        return POLARITY_STRING_HORZVERT;
      case POLARITY_VERTHORZ:
        return POLARITY_STRING_VERTHORZ;
      case POLARITY_LEFTCIRC:
        return POLARITY_STRING_LEFTCIRC;
      case POLARITY_RIGHTCIRC:
        return POLARITY_STRING_RIGHTCIRC;
      case POLARITY_LINEAR:
        return POLARITY_STRING_LINEAR;
      default:
        return POLARITY_STRING_UNKNOWN;
    }
  }

  /**
  * Determines the polarity type from the incoming string
  * @param[in ] type String representation of polarity
  * @return PolarityType, POLARITY_UNKNOWN returned if no match
  */
  inline PolarityType polarityType(const std::string &type)
  {
    if (type.empty()) return POLARITY_UNKNOWN;
    std::string upperStrType;
    upperStrType.resize(type.size());
    std::transform(type.begin(), type.end(), upperStrType.begin(), ::toupper);
    if (upperStrType == POLARITY_STRING_HORIZONTAL)
      return POLARITY_HORIZONTAL;
    else if (upperStrType == POLARITY_STRING_VERTICAL)
      return POLARITY_VERTICAL;
    else if (upperStrType == POLARITY_STRING_CIRCULAR)
      return POLARITY_CIRCULAR;
    else if (upperStrType == POLARITY_STRING_HORZVERT)
      return POLARITY_HORZVERT;
    else if (upperStrType == POLARITY_STRING_VERTHORZ)
      return POLARITY_VERTHORZ;
    else if (upperStrType == POLARITY_STRING_LEFTCIRC)
      return POLARITY_LEFTCIRC;
    else if (upperStrType == POLARITY_STRING_RIGHTCIRC)
      return POLARITY_RIGHTCIRC;
    else if (upperStrType == POLARITY_STRING_LINEAR)
      return POLARITY_LINEAR;
    else return POLARITY_UNKNOWN;
  }

  ///Enumeration of supported Radar Cross Section (RCS) types
  enum RCSType
  {
    NO_RCS = 0,
    RCS_LUT,
    RCS_BLOOM,
    RCS_SADM,
    RCS_XPATCH,
    RCS_RTS
  };

  ///Enumeration of supported Radar Cross Section (RCS) table types
  enum RCSTableType
  {
    RCS_DISTRIBUTION_FUNC_TYPE = 0,
    RCS_LUT_TYPE,
    RCS_SYM_LUT_TYPE
  };

  ///Enumeration of supported Radar Cross Section (RCS) distribution function types
  enum RCSFuncType
  {
    RCS_MEAN_FUNC = 0,
    RCS_GAUSSIAN_FUNC,
    RCS_RAYLEIGH_FUNC,
    RCS_LOG_NORMAL_FUNC
  };

  ///Enumeration of possible antenna lobes
  enum AntennaLobeType
  {
    ANTENNA_LOBE_NONE=0,
    ANTENNA_LOBE_MAIN,
    ANTENNA_LOBE_SIDE,
    ANTENNA_LOBE_BACK
  };

  ///Enumeration of possible antenna pattern algorithms
  enum AntennaAlgorithmType
  {
    ANTENNA_ALGORITHM_UNKNOWN=0,
    ANTENNA_ALGORITHM_PEDESTAL,
    ANTENNA_ALGORITHM_GAUSS,
    ANTENNA_ALGORITHM_CSCSQ,
    ANTENNA_ALGORITHM_SINXX,
    ANTENNA_ALGORITHM_OMNI,
  };

  ///Enumeration of possible antenna pattern file formats
  enum AntennaFormatType
  {
    ANTENNA_FORMAT_UNKNOWN=5,
    ANTENNA_FORMAT_TABLE,
    ANTENNA_FORMAT_MONOPULSE,
    ANTENNA_FORMAT_CRUISE,
    ANTENNA_FORMAT_RELATIVE,
    ANTENNA_FORMAT_BILINEAR,
    ANTENNA_FORMAT_NSMA,
    ANTENNA_FORMAT_EZNEC,
    ANTENNA_FORMAT_XFDTD
  };

  ///Enumeration of supported antenna pattern types
  enum AntennaPatternType
  {
    NO_ANTENNA_PATTERN=0,
    ANTENNA_PATTERN_PEDESTAL=1,
    ANTENNA_PATTERN_GAUSS,
    ANTENNA_PATTERN_CSCSQ,
    ANTENNA_PATTERN_SINXX,
    ANTENNA_PATTERN_OMNI,
    ANTENNA_PATTERN_TABLE,
    ANTENNA_PATTERN_MONOPULSE,
    ANTENNA_PATTERN_CRUISE,
    ANTENNA_PATTERN_RELATIVE,
    ANTENNA_PATTERN_BILINEAR,
    ANTENNA_PATTERN_NSMA,
    ANTENNA_PATTERN_EZNEC,
    ANTENNA_PATTERN_XFDTD
  };

  /// String for GAUSS antenna algorithm
  static const std::string ANTENNA_STRING_ALGORITHM_GAUSS = "GAUSS";
  /// String for CSCSQ antenna algorithm
  static const std::string ANTENNA_STRING_ALGORITHM_CSCSQ = "CSCSQ";
  /// String for SINXX antenna algorithm
  static const std::string ANTENNA_STRING_ALGORITHM_SINXX = "SINXX";
  /// String for PEDESTAL antenna algorithm
  static const std::string ANTENNA_STRING_ALGORITHM_PEDESTAL = "PEDESTAL";
  /// String for OMNI antenna algorithm
  static const std::string ANTENNA_STRING_ALGORITHM_OMNI = "OMNI";

  /// String for TABLE antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_TABLE = "TABLE";
  /// String for RELATIVE antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_RELATIVE = "RELATIVE";
  /// String for MONOPULSE antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_MONOPULSE = "MONOPULSE";
  /// String for BILINEAR antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_BILINEAR = "BILINEAR";
  /// String for CRUISE antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_CRUISE = "CRUISE";
  /// String for NSMA antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_NSMA = "NSMA";
  /// String for EZNEC antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_EZNEC = "EZNEC";
  /// String for XFDTD antenna pattern file format
  static const std::string ANTENNA_STRING_FORMAT_XFDTD = "XFDTD";

  /// File extension to use for TABLE antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_TABLE = ".aptf";
  /// File extension to use for RELATIVE antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_RELATIVE = ".aprf";
  /// File extension to use for BILINEAR antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_BILINEAR = ".apbf";
  /// File extension to use for CRUISE antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_CRUISE = ".apcf";
  /// File extension to use for MONOPULSE antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_MONOPULSE = ".apmf";
  /// File extension to use for NSMA antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_NSMA = ".nsm";
  /// File extension to use for EZNEC antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_EZNEC = ".txt";
  /// File extension to use for XFDTD antenna pattern files
  static const std::string ANTENNA_STRING_EXTENSION_XFDTD = ".uan";

} // namespace simCore

#endif /* SIMCORE_EM_CONSTANTS_H */
