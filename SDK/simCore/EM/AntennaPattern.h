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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_EM_ANTENNA_PATTERN_H
#define SIMCORE_EM_ANTENNA_PATTERN_H

#include <map>
#include <string>
#include <fstream>
#include <complex>
#include <cfloat>

#include "simCore/Common/Common.h"
#include "simCore/LUT/InterpTable.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Constants.h"

namespace simCore
{
/* ************************************************************************** */
/* SymmetricAntennaPattern                                                    */
/* ************************************************************************** */

/** Bilinear look up table for complex antenna patterns */
typedef InterpTable< std::complex<double> > SymmetricAntennaPattern;
/** Bilinear look up table exception handler for complex antenna patterns */
typedef InterpTableLimitException< std::complex<double> > SymmetricAntennaPatternLimitException;

/**
* Reads and parses a SymmetricAntennaPattern from an input stream
* @param[out] sap SymmetricAntennaPattern class to fill
* @param[in ] in Input stream to read
* @param[in ] name Name of antenna pattern
* @param[in ] frequency Frequency of antenna pattern (Hz)
* @param[in ] frequencythreshold Frequency threshold between different frequencies in pattern (Hz)
* @return boolean, true:success, false on failure
* @pre sap valid param
*/
SDKCORE_EXPORT bool readPattern(SymmetricAntennaPattern *sap, std::istream &in, const std::string &name, double frequency, double frequencythreshold = 0.5e+9);

/**
* Reads and parses a SymmetricAntennaPattern from an input file
* @param[out] sap SymmetricAntennaPattern class to fill
* @param[in ] filename Input file name to read
* @param[in ] name Name of antenna pattern
* @param[in ] frequency Frequency of antenna pattern (Hz)
* @param[in ] frequencythreshold Frequency threshold between different frequencies in pattern (Hz)
* @return boolean, true:success, false on failure
* @pre sap valid param
*/
SDKCORE_EXPORT bool readPattern(SymmetricAntennaPattern *sap, const std::string &filename, const std::string &name, double frequency, double frequencythreshold = 0.5e+9);

/** Bilinear look up table for gain only antenna patterns */
typedef InterpTable< double > SymmetricGainAntPattern;
/** Bilinear look up table exception handler for gain only antenna patterns */
typedef InterpTableLimitException< double > SymmetricGainAntPatternLimitException;

/**
* Reads and parses a SymmetricGainAntPattern from an input stream
* @param[out] sap SymmetricGainAntPattern class to fill
* @param[in ] in Input stream to read
* @param[in ] frequency Frequency of antenna pattern (Hz)
* @param[in ] frequencythreshold Frequency threshold between different frequencies in pattern (Hz)
* @return boolean, true:success, false on failure
* @pre sap valid param
*/
SDKCORE_EXPORT bool readPattern(SymmetricGainAntPattern *sap, std::istream &in, double frequency, double frequencythreshold = 0.5e+9);

/**
* Reads and parses a SymmetricGainAntPattern from an input file
* @param[out] sap SymmetricGainAntPattern class to fill
* @param[in ] filename Input file name to read
* @param[in ] frequency Frequency of antenna pattern (Hz)
* @param[in ] frequencythreshold Frequency threshold between different frequencies in pattern (Hz)
* @return boolean, true:success, false on failure
* @pre sap valid param
*/
SDKCORE_EXPORT bool readPattern(SymmetricGainAntPattern *sap, const std::string &filename, double frequency, double frequencythreshold = 0.5e+9);

/* ************************************************************************** */
/* Bilinear look up table for Gain Data                                       */
/* ************************************************************************** */

/** Look up table for floating point gain data */
typedef InterpTable< float > GainData;
/** Look up table exception handler for floating point gain data */
typedef InterpTableLimitException< float > GainDataLimitException;

/* ************************************************************************** */

/**
* Returns the string representation of the antenna pattern type
* @param[in ] antPatType AntennaPatternType
* @return string representation of the antenna pattern type
*/
SDKCORE_EXPORT std::string antennaPatternTypeString(AntennaPatternType antPatType);

/* ************************************************************************** */

/**
* Returns the antenna pattern type for the given string
* @param[in ] antPatStr string representation of the antenna pattern type or antenna pattern file extension
* @return AntennaPatternType, NO_ANTENNA_PATTERN for no match
*/
SDKCORE_EXPORT AntennaPatternType antennaPatternType(const std::string& antPatStr);

/* ************************************************************************** */

/**
* @brief This function returns the gain for a antenna pattern look up table
* @param[in ] azimData Azimuth gain data
* @param[in ] elevData Elevation gain data
* @param[out ] lastLobe AntennaLobeType of lobe last seen, set based on normalized beam width (phi)
* @param[in ] azim Azimuth relative to antenna (rad)
* @param[in ] elev Elevation relative to antenna (rad)
* @param[in ] hbw Horizontal beam width of radar (rad), must be non-zero
* @param[in ] vbw Vertical beam width of radar (rad), must be non-zero
* @param[in ] maxGain Maximum (normalized) antenna gain (dB)
* @param[in ] applyWeight Boolean toggle to apply weighting (true) to the antenna gain
* @return Antenna pattern gain (dB).
*/
SDKCORE_EXPORT float calculateGain(const std::map<float, float> *azimData,
  const std::map<float, float> *elevData,
  AntennaLobeType &lastLobe,
  float azim,
  float elev,
  float hbw,
  float vbw,
  float maxGain,
  bool applyWeight);


/// Container class that contains antenna parameters for gain calculations
class SDKCORE_EXPORT AntennaGainParameters
{
public:

  float azim_;                  ///< Relative azimuth angle, referenced to host antenna (rad)
  float elev_;                  ///< Relative elevation angle, referenced to host antenna (rad)
  PolarityType polarity_;       ///< Antenna polarity
  float hbw_;                   ///< Antenna horizontal beam width (rad)
  float vbw_;                   ///< Antenna vertical beam width (rad)
  float refGain_;               ///< Reference gain of pattern (dB)
  float firstLobe_;             ///< Value of first side lobe (dB)
  float backLobe_;              ///< Value of back lobe (dB)
  double freq_;                 ///< Frequency of pattern (Hz)
  bool weighting_;              ///< Boolean flag to indicated use of a weighted average for gain (true: weighted)
  bool delta_;                  ///< Boolean flag to indicated use sum or delta channel for monopulse antenna (true: delta channel)

  /**
  * AntennaGainParameters constructor
  * @param[in ] az Relative azimuth angle, referenced to host antenna (rad)
  * @param[in ] el Relative elevation angle, referenced to host antenna (rad)
  * @param[in ] pol Antenna polarity
  * @param[in ] hbw Antenna horizontal beam width (rad)
  * @param[in ] vbw Antenna vertical beam width (rad)
  * @param[in ] gain Reference gain of pattern (dB)
  * @param[in ] firstLobe Value of first side lobe (dB)
  * @param[in ] backLobe Value of back lobe (dB)
  * @param[in ] freq Frequency of pattern (Hz)
  * @param[in ] weight Boolean flag to indicated use of a weighted average for gain (true: weighted)
  * @param[in ] delta Boolean flag to indicated use sum or delta channel for monopulse antenna (true: delta channel)
  */
  AntennaGainParameters(float az=0.f, float el=0.f, PolarityType pol=POLARITY_UNKNOWN,
    float hbw=0.1f, float vbw=0.1f, float gain=0.f, float firstLobe=-23.2f,
    float backLobe=-20.0f, double freq=2e6, bool weight=false, bool delta=false)
    : azim_(az), elev_(el), polarity_(pol), hbw_(hbw), vbw_(vbw), refGain_(gain),
    firstLobe_(firstLobe), backLobe_(backLobe), freq_(freq),
    weighting_(weight), delta_(delta) {}
  ~AntennaGainParameters() {}
};

// ----------------------------------------------------------------------------

/// Abstract class that all antenna patterns are derived from
class SDKCORE_EXPORT AntennaPattern
{
public:
  /** AntennaPattern constructor */
  AntennaPattern() : valid_(false),
    minGain_(-SMALL_DB_VAL),
    maxGain_(SMALL_DB_VAL),
    polarity_(POLARITY_UNKNOWN),
    filename_("") {};

  /** AntennaPattern destructor */
  virtual ~AntennaPattern() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return NO_ANTENNA_PATTERN; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params) = 0;

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params) = 0;

  /**
  * This method returns the file name of the antenna pattern
  * @return file name.
  */
  std::string filename() const { return filename_; }

  /**
  * This method sets the polarity of the antenna pattern
  * @param[in ] pol Antenna polarity.
  */
  void polarity(PolarityType pol) { polarity_ = pol; }

  /**
  * This method returns the polarity of the antenna pattern
  * @return polarity.
  */
  PolarityType polarity() const { return polarity_; }

  /**
  * This method returns the validity status of the antenna pattern
  * @return validity.
  */
  bool valid() const { return valid_; }

protected:
  bool valid_;                  ///< Indicates status of data, true: good, false: bad
  float minGain_;               ///< Minimum gain value (dB)
  float maxGain_;               ///< Maximum gain value (dB)
  PolarityType polarity_;       ///< Antenna pattern polarity
  std::string filename_;        ///< Filename containing antenna pattern data
};

// ----------------------------------------------------------------------------

/// Gaussian antenna pattern class
class SDKCORE_EXPORT AntennaPatternGauss : public AntennaPattern
{
public:
  AntennaPatternGauss() : AntennaPattern(), lastVbw_(-FLT_MAX) {valid_ = true; filename_ = ANTENNA_STRING_ALGORITHM_GAUSS;}
  virtual ~AntennaPatternGauss() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_GAUSS; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

protected:
  float lastVbw_;             ///< Last vertical beam width used to calculate min & max gains
};


/// Cosecant squared antenna pattern class
class SDKCORE_EXPORT AntennaPatternCscSq : public AntennaPattern
{
public:
  AntennaPatternCscSq() : AntennaPattern(), lastVbw_(-FLT_MAX) {valid_ = true; filename_ = ANTENNA_STRING_ALGORITHM_CSCSQ;}
  virtual ~AntennaPatternCscSq() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_CSCSQ; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

protected:
  float lastVbw_;             ///< Last vertical beam width used to calculate min & max gains
};


/// Sine X/X (sinc) antenna pattern class
class SDKCORE_EXPORT AntennaPatternSinXX : public AntennaPattern
{
public:
  AntennaPatternSinXX() : AntennaPattern(), lastVbw_(-FLT_MAX), lastHbw_(-FLT_MAX) {valid_ = true; filename_ = ANTENNA_STRING_ALGORITHM_SINXX;}
  virtual ~AntennaPatternSinXX() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_SINXX; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

protected:
  float lastVbw_;             ///< Last vertical beam width used to calculate min & max gains
  float lastHbw_;             ///< Last horizontal beam width used to calculate min & max gains
};


/// Pedestal antenna pattern class
class SDKCORE_EXPORT AntennaPatternPedestal : public AntennaPattern
{
public:
  AntennaPatternPedestal() : AntennaPattern(), lastVbw_(-FLT_MAX), lastHbw_(-FLT_MAX), lastGain_(SMALL_DB_VAL) {valid_ = true; filename_ = ANTENNA_STRING_ALGORITHM_PEDESTAL;}
  virtual ~AntennaPatternPedestal() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_PEDESTAL; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

protected:
  float lastVbw_;             ///< Last vertical beam width used to calculate min & max gains
  float lastHbw_;             ///< Last horizontal beam width used to calculate min & max gains
  float lastGain_;            ///< Last gain value used to calculate min & max gains
};


/// Omni directional antenna pattern class
class SDKCORE_EXPORT AntennaPatternOmni : public AntennaPattern
{
public:

  AntennaPatternOmni() : AntennaPattern() {valid_ = true; filename_ = ANTENNA_STRING_ALGORITHM_OMNI;}
  virtual ~AntennaPatternOmni() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_OMNI; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);
};


/// Table based antenna pattern class
class SDKCORE_EXPORT AntennaPatternTable : public AntennaPattern
{
public:
  AntennaPatternTable(bool type = false);
  virtual ~AntennaPatternTable() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_TABLE; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat(std::istream& fp);

  /**
  * This method sets the validity of the antenna pattern, accessed by SimLogic binary FCT loader
  * @param[in ] val Boolean, validity of antenna pattern
  */
  void setValid(bool val) {valid_ = val;}

  /**
  * This method sets the type of units for the azimuth and elevation data, accessed by SimLogic binary FCT loader
  * @param[in ] val false: angles in radians, true: angles in beamwidth (m)
  */
  void setType(bool val) {beamWidthType_ = val;}

  /**
  * This method sets the file name of the antenna pattern, accessed by SimLogic binary FCT loader
  * @param[in ] str File name of antenna pattern
  */
  void setFilename(const std::string& str) {filename_ = str;}

  /**
  * This method sets the gain value for the specified azimuth, accessed by SimLogic binary FCT loader
  * @param[in ] ang Azimuth position of antenna pattern, units based on type_
  * @param[in ] gain Gain of antenna pattern at specified azimuth (dB)
  */
  void setAzimData(float ang, float gain) {azimData_[ang] = gain;}

  /**
  * This method sets the gain value for the specified elevation, accessed by SimLogic binary FCT loader
  * @param[in ] ang Elevation position of antenna pattern, units based on type_
  * @param[in ] gain Gain of antenna pattern at specified elevation (dB)
  */
  void setElevData(float ang, float gain) {elevData_[ang] = gain;}

protected:
  bool beamWidthType_;              ///< false: angles in radians, true: angles in beamwidth (m)
  float lastVbw_;                   ///< Last vertical beam width used to calculate min & max gains
  float lastHbw_;                   ///< Last horizontal beam width used to calculate min & max gains
  float lastGain_;                  ///< Last gain value used to calculate min & max gains
  std::map<float, float> azimData_; ///< Azimuth gain data
  std::map<float, float> elevData_; ///< Elevation gain data
};


/// CRUISE model antenna pattern class
class SDKCORE_EXPORT AntennaPatternCRUISE : public AntennaPattern
{
public:

  AntennaPatternCRUISE();
  virtual ~AntennaPatternCRUISE() {reset_();}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_CRUISE; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

protected:

  int azimLen_;               ///< Size of azimuth array
  int elevLen_;               ///< Size of elevation array
  int freqLen_;               ///< Size of frequency array
  double azimMin_;            ///< Minimum azimuth value
  double elevMin_;            ///< Minimum elevation value
  double azimStep_;           ///< Azimuth step value
  double elevStep_;           ///< Elevation step value
  double *freqData_;          ///< Frequency data
  double **azimData_;         ///< Azimuth gain data
  double **elevData_;         ///< Elevation gain data

  /**
  * This method resets the pattern
  */
  void reset_();

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);
};


/// Relative table antenna pattern class
class SDKCORE_EXPORT AntennaPatternRelativeTable : public AntennaPattern
{
public:
  AntennaPatternRelativeTable();
  virtual ~AntennaPatternRelativeTable() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_RELATIVE; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

protected:
  float lastVbw_;                   ///< Last vertical beam width used to calculate min & max gains
  float lastHbw_;                   ///< Last horizontal beam width used to calculate min & max gains
  float lastGain_;                  ///< Last gain value used to calculate min & max gains
  std::map<float, float> azimData_; ///< Azimuth gain data (dB)
  std::map<float, float> elevData_; ///< Elevation gain data (dB)

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);
};


/// Monopulse antenna pattern class
class SDKCORE_EXPORT AntennaPatternMonopulse : public AntennaPattern
{
public:
  AntennaPatternMonopulse();
  virtual ~AntennaPatternMonopulse();

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_MONOPULSE; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @param[in ] freq Frequency of antenna pattern to load (Hz)
  * @return 0 on success.
  */
  int readPat(const std::string& file, double freq);

protected:
  double freq_; ///< Current freq associated with computed gain
  float minDelGain_; ///< Minimum delta gain value (dB)
  float maxDelGain_; ///< Maximum delta gain value (dB)

  SymmetricAntennaPattern sumPat_;  ///< Monopulse sum pattern (linear)
  SymmetricAntennaPattern delPat_;  ///< Monopulse delta pattern (linear)

  /**
  * This method resets the pattern
  */
  void reset_();

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);

  /**
  * This method sets the minimum and maximum gains for the requested pattern type
  * @param[out] min Minimum gain value to set (dB)
  * @param[out] max Maximum gain value to set (dB)
  * @param[in ] maxGain Maximum gain to be applied to computed gain value (dB)
  * @param[in ] delta Boolean, true: use delta pattern, false: use sum pattern
  * @pre min and max valid params
  */
  void setMinMaxGain_(float *min, float *max, float maxGain, bool delta);

};


/// Bilinear interpolation antenna pattern class
class SDKCORE_EXPORT AntennaPatternBiLinear : public AntennaPattern
{
public:
  AntennaPatternBiLinear();
  virtual ~AntennaPatternBiLinear();

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_BILINEAR; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @param[in ] freq Frequency of antenna pattern to load (Hz)
  * @return 0 on success.
  */
  int readPat(const std::string& file, double freq);

protected:
  double freq_;                     ///< Current freq associated with computed gain
  SymmetricGainAntPattern antPat_;  ///< Antenna gain data (dB)

  /**
  * This method resets the pattern
  */
  void reset_();

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);
};


/// National Spectrum Management Association (NSMA) antenna pattern class
class SDKCORE_EXPORT AntennaPatternNSMA : public AntennaPattern
{
public:

  AntennaPatternNSMA();
  virtual ~AntennaPatternNSMA() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_NSMA; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

protected:
  float midBandGain_;                   ///< Relative gain of antenna pattern
  float halfPowerBeamWidth_;            ///< Half power (3 dB) beam width
  double minFreq_;                      ///< Minimum frequency of antenna, Hz
  double maxFreq_;                      ///< Maximum frequency of antenna, Hz

  std::map<float, float> HHDataMap_;    ///< Azimuth HH polarization gain data (dB)
  std::map<float, float> ELHHDataMap_;  ///< Elevation HH polarization gain data (dB)

  std::map<float, float> HVDataMap_;    ///< Azimuth HV polarization gain data (dB)
  std::map<float, float> ELHVDataMap_;  ///< Elevation HV polarization gain data (dB)
  float minHVGain_;                     ///< Minimum HV gain value (dB)
  float maxHVGain_;                     ///< Maximum HV gain value (dB)

  std::map<float, float> VHDataMap_;    ///< Azimuth VH polarization gain data (dB)
  std::map<float, float> ELVHDataMap_;  ///< Elevation VH polarization gain data (dB)
  float minVHGain_;                     ///< Minimum VH gain value (dB)
  float maxVHGain_;                     ///< Maximum VH gain value (dB)

  std::map<float, float> VVDataMap_;    ///< Azimuth VV polarization gain data (dB)
  std::map<float, float> ELVVDataMap_;  ///< Elevation VV polarization gain data (dB)
  float minVVGain_;                     ///< Minimum VV gain value (dB)
  float maxVVGain_;                     ///< Maximum VV gain value (dB)

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);

  /**
  * This method computes and stores the minimum and maximum gains
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] maxGain Maximum gain to be applied to computed gain value (dB)
  * @param[in ] polarity Antenna polarity
  * @pre min and max valid params
  */
  void setMinMax_(float *min, float *max, float maxGain, PolarityType polarity);
};


/// Easy Numerical Electromagnetic Code (EZNEC) antenna pattern class
class SDKCORE_EXPORT AntennaPatternEZNEC : public AntennaPattern
{
public:
  AntennaPatternEZNEC();
  virtual ~AntennaPatternEZNEC() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_EZNEC; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

protected:
  double frequency_;          ///< Antenna pattern frequency
  float reference_;           ///< Reference gain value (dB)
  bool angleConvCCW_;         ///< Pattern angle convention, CCW:true, Compass:false
  GainData vertData_;         ///< Vertical component gain values (dB)
  float minVertGain_;         ///< Minimum horizontal gain value (dB)
  float maxVertGain_;         ///< Maximum horizontal gain value (dB)
  GainData horzData_;         ///< Horizontal component gain values (dB)
  float minHorzGain_;         ///< Minimum horizontal gain value (dB)
  float maxHorzGain_;         ///< Maximum horizontal gain value (dB)
  GainData totalData_;        ///< Total gain values (dB)

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);
};


/// REMCOM Finite Difference Time Domain (XFDTD) antenna pattern class
class SDKCORE_EXPORT AntennaPatternXFDTD : public AntennaPattern
{
public:
  AntennaPatternXFDTD();
  virtual ~AntennaPatternXFDTD() {}

  /**
  * This method returns the type of antenna pattern
  * @return antenna pattern type
  */
  virtual AntennaPatternType type() const { return ANTENNA_PATTERN_XFDTD; }

  /**
  * This method computes the antenna pattern gain for the requested parameters
  * @param[in ] params Collection of antenna parameters used to compute the requested gain value
  * @return antenna pattern gain (dB)
  */
  virtual float gain(const AntennaGainParameters &params);

  /**
  * This method returns the minimum and maximum gains for the pattern
  * @param[out] min Minimum gain value to retrieve (dB)
  * @param[out] max Maximum gain value to retrieve (dB)
  * @param[in ] params Collection of antenna parameters used to compute the requested gain bounds
  * @pre min and max valid params
  */
  virtual void minMaxGain(float *min, float *max, const AntennaGainParameters &params);

  /**
  * This method checks the incoming antenna pattern data filename, opens a file stream and calls readPat_
  * @param[in ] file Input file name
  * @return 0 on success.
  */
  int readPat(const std::string& file);

protected:
  float reference_;           ///< Reference gain value (dB)
  GainData vertData_;         ///< Vertical component gain values (dB)
  float minVertGain_;         ///< Minimum horizontal gain value (dB)
  float maxVertGain_;         ///< Maximum horizontal gain value (dB)
  GainData horzData_;         ///< Horizontal component gain values (dB)
  float minHorzGain_;         ///< Minimum horizontal gain value (dB)
  float maxHorzGain_;         ///< Maximum horizontal gain value (dB)
  GainData totalData_;        ///< Total gain values (dB)

  /**
  * This method parses and stores the incoming antenna pattern data
  * @param[in ] fp Input file stream handle
  * @return 0 on success.
  */
  int readPat_(std::istream& fp);
};


/** Factory function to load a pattern file with the given gain and frequency, based
  * on the extension of the filename.
  * @param filename Name of the file to load (extension matters)
  * @param freq Frequency value to pass to loader
  * @return Pointer to antenna pattern instance
  */
SDKCORE_EXPORT AntennaPattern* loadPatternFile(const std::string &filename, float freq);

} // namespace simCore

#endif /* SIMCORE_EM_ANTENNA_PATTERN_H */
