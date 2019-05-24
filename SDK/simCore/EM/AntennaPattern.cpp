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
#include <cstring>
#include <string>
#include "simNotify/Notify.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Interpolation.h"
#include "simCore/LUT/InterpTable.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/Constants.h"
#include "simCore/EM/AntennaPattern.h"

namespace simCore {
// Returns the string representation of the antenna pattern type
std::string antennaPatternTypeString(AntennaPatternType antPatType)
{
  switch (antPatType)
  {
  case ANTENNA_PATTERN_PEDESTAL:  return ANTENNA_STRING_ALGORITHM_PEDESTAL; break;
  case ANTENNA_PATTERN_GAUSS:  return ANTENNA_STRING_ALGORITHM_GAUSS; break;
  case ANTENNA_PATTERN_CSCSQ:  return ANTENNA_STRING_ALGORITHM_CSCSQ; break;
  case ANTENNA_PATTERN_SINXX:  return ANTENNA_STRING_ALGORITHM_SINXX; break;
  case ANTENNA_PATTERN_OMNI:  return ANTENNA_STRING_ALGORITHM_OMNI; break;
  case ANTENNA_PATTERN_TABLE:  return ANTENNA_STRING_FORMAT_TABLE; break;
  case ANTENNA_PATTERN_MONOPULSE:  return ANTENNA_STRING_FORMAT_MONOPULSE; break;
  case ANTENNA_PATTERN_CRUISE:  return ANTENNA_STRING_FORMAT_CRUISE; break;
  case ANTENNA_PATTERN_RELATIVE:  return ANTENNA_STRING_FORMAT_RELATIVE; break;
  case ANTENNA_PATTERN_BILINEAR:  return ANTENNA_STRING_FORMAT_BILINEAR; break;
  case ANTENNA_PATTERN_NSMA:  return ANTENNA_STRING_FORMAT_NSMA; break;
  case ANTENNA_PATTERN_EZNEC:  return ANTENNA_STRING_FORMAT_EZNEC; break;
  case ANTENNA_PATTERN_XFDTD:  return ANTENNA_STRING_FORMAT_XFDTD; break;
  default: return "UNKNOWN"; break;
  }
}

// Returns the antenna pattern type for the given string
AntennaPatternType antennaPatternType(const std::string& antPatStr)
{
  simCore::AntennaPatternType type = simCore::NO_ANTENNA_PATTERN;
  if (!antPatStr.empty())
  {
    if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_ALGORITHM_SINXX) == 0)
      type = simCore::ANTENNA_PATTERN_SINXX;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_ALGORITHM_PEDESTAL) == 0)
      type = simCore::ANTENNA_PATTERN_PEDESTAL;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_ALGORITHM_GAUSS) == 0)
      type = simCore::ANTENNA_PATTERN_GAUSS;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_ALGORITHM_OMNI) == 0)
      type = simCore::ANTENNA_PATTERN_OMNI;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_ALGORITHM_CSCSQ) == 0)
      type = simCore::ANTENNA_PATTERN_CSCSQ;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_TABLE) == 0)
      type = simCore::ANTENNA_PATTERN_TABLE;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_RELATIVE) == 0)
      type = simCore::ANTENNA_PATTERN_RELATIVE;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_MONOPULSE) == 0)
      type = simCore::ANTENNA_PATTERN_MONOPULSE;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_BILINEAR) == 0)
      type = simCore::ANTENNA_PATTERN_BILINEAR;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_CRUISE) == 0)
      type = simCore::ANTENNA_PATTERN_CRUISE;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_NSMA) == 0)
      type = simCore::ANTENNA_PATTERN_NSMA;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_EZNEC) == 0)
      type = simCore::ANTENNA_PATTERN_EZNEC;
    else if (simCore::caseCompare(antPatStr, simCore::ANTENNA_STRING_FORMAT_XFDTD) == 0)
      type = simCore::ANTENNA_PATTERN_XFDTD;
    else
    {
      // check value as a filename for the pattern extension
      std::string extension = simCore::getExtension(antPatStr);
      if (extension == simCore::ANTENNA_STRING_EXTENSION_TABLE)
        type = simCore::ANTENNA_PATTERN_TABLE;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_RELATIVE)
        type = simCore::ANTENNA_PATTERN_RELATIVE;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_BILINEAR)
        type = simCore::ANTENNA_PATTERN_BILINEAR;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_CRUISE)
        type = simCore::ANTENNA_PATTERN_CRUISE;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_MONOPULSE)
        type = simCore::ANTENNA_PATTERN_MONOPULSE;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_NSMA)
        type = simCore::ANTENNA_PATTERN_NSMA;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_EZNEC)
        type = simCore::ANTENNA_PATTERN_EZNEC;
      else if (extension == simCore::ANTENNA_STRING_EXTENSION_XFDTD)
        type = simCore::ANTENNA_PATTERN_XFDTD;
    }
  }
  return type;
}

/* ************************************************************************** */
/* SymmetricAntennaPattern                                                    */
/* ************************************************************************** */

bool readPattern(SymmetricAntennaPattern *sap, std::istream &in, const std::string &name, double frequency, double frequencythreshold)
{
  std::string st;
  double minfreq=0;
  double stepfreq=1;
  double minel=0;
  double maxel=0;
  double minaz=0;
  double maxaz=0;
  size_t numfreq=0;
  size_t numel=0;
  size_t numaz=0;
  size_t i=0, j=0, k=0;
  std::complex<double> magphase;
  double magnitude=0, phase=0;
  std::vector<std::string> vec;
  bool found = false;
  bool freqFound = false;

  // check for valid pattern
  while (getStrippedLine(in, st) && !found)
  {
    stringTokenizer(vec, st);
    if (!vec.empty() && vec[0] == name)
    {
      found = true;
    }
  }

  if (!found)
  {
    SIM_ERROR << "SymmetricAntennaPattern could not find pattern " << name;
    return false;
  }

  // parse freq data
  stringTokenizer(vec, st);
  if (vec.size() > 2)
  {
    if (!isValidNumber(vec[0], minfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern minimum frequency" << std::endl;
      return 1;
    }
    double maxfreq = 0;
    if (!isValidNumber(vec[1], maxfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern maximum frequency" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[2], stepfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern step frequency" << std::endl;
      return 1;
    }

    if (stepfreq == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricAntennaPattern can not use step frequency of 0" << std::endl;
      return 1;
    }
    if (minfreq == maxfreq && minfreq == 0)
    {
      SIM_ERROR << "SymmetricAntennaPattern could not determine frequency limits";
    }
    numfreq = size_t(floor((maxfreq - minfreq) / stepfreq)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricAntennaPattern expected 3 values for frequency limits";
    return false;
  }

  // parse azimuth data
  if (getTokens(in, vec, 3))
  {
    if (!isValidNumber(vec[0], minaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern minimum azimuth" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[1], maxaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern maximum azimuth" << std::endl;
      return 1;
    }
    double stepaz = 0.;
    if (!isValidNumber(vec[2], stepaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern step azimuth" << std::endl;
      return 1;
    }
    if (stepaz == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricAntennaPattern can not use step azimuth of 0" << std::endl;
      return 1;
    }
    numaz = size_t(floor((maxaz - minaz) / stepaz)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricAntennaPattern expected 3 values for azimuth limits";
    return false;
  }

  // parse elevation data
  if (getTokens(in, vec, 3))
  {
    if (!isValidNumber(vec[0], minel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern minimum elevation" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[1], maxel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern maximum elevation" << std::endl;
      return 1;
    }
    double stepel = 0;
    if (!isValidNumber(vec[2], stepel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern step elevation" << std::endl;
      return 1;
    }
    if (stepel == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricAntennaPattern can not use step elevation of 0" << std::endl;
      return 1;
    }
    numel = size_t(floor((maxel - minel) / stepel)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricAntennaPattern expected 3 values for elevation limits";
    return false;
  }

  sap->initialize(minaz, maxaz, numaz, minel, maxel, numel);

  // parse mag phase pairs
  for (i = 0; i < numfreq; ++i)
  {
    double currentFreq = minfreq + i * stepfreq;
    if (frequency < currentFreq + frequencythreshold && frequency > currentFreq - frequencythreshold)
    {
      freqFound = true;
      for (j = 0; j < numaz; ++j)
      {
        for (k = 0; k < numel; ++k)
        {
          st.clear();
          getStrippedLine(in, st);
          stringTokenizer(vec, st);
          if (vec.size() > 1)
          {
            if (!isValidNumber(vec[0], magnitude))
            {
              SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern magnitude" << std::endl;
              return 1;
            }
            if (!isValidNumber(vec[1], phase))
            {
              SIM_ERROR << "Encountered invalid number for SymmetricAntennaPattern phase" << std::endl;
              return 1;
            }
            magphase = std::polar(simCore::dB2Linear(magnitude), simCore::DEG2RAD*(phase));
            (*sap)(j, k) = magphase;
          }
          else
          {
            SIM_ERROR << "SymmetricAntennaPattern expected magnitude and phase";
            return false;
          }
          if (!in)
          {
            SIM_ERROR << "SymmetricAntennaPattern ran out of data for frequency " << frequency;
            return false;
          }
        }
      }
    }
    else
    {
      // skip over data until the next frequency
      for (j = 0; j < numaz*numel; ++j)
      {
        getStrippedLine(in, st);
      }
    }
  }

  if (!freqFound)
  {
    SIM_ERROR << "SymmetricAntennaPattern could not find pattern " << name << " with frequency " << frequency << " within threshold " << frequencythreshold;
    return false;
  }

  return true;
}

bool readPattern(SymmetricAntennaPattern *sap, const std::string &filename, const std::string &name, double frequency, double frequencythreshold)
{
  std::fstream in(filename.c_str(), std::ios::in);
  return readPattern(sap, in, name, frequency, frequencythreshold);
}

/* ************************************************************************** */
/* Bilinear look up table for gain only antenna patterns                      */
/* ************************************************************************** */

bool readPattern(SymmetricGainAntPattern *sap, std::istream &in, double frequency, double frequencyThreshold)
{
  std::string st;
  double minfreq=0;
  double stepfreq=1;
  double minel=0;
  double maxel=0;
  double minaz=0;
  double maxaz=0;
  size_t numfreq=0;
  size_t numel=0;
  size_t numaz=0;
  size_t i=0, j=0, k=0;
  double magnitude=0;
  std::vector<std::string> vec;
  bool found = false;
  bool freqFound = false;

  // check for valid pattern
  while (getStrippedLine(in, st) && !found)
  {
    stringTokenizer(vec, st);
    if (!vec.empty() && vec[0] == "bilinear")
    {
      found = true;
    }
  }

  if (!found)
  {
    SIM_ERROR << "SymmetricGainAntennaPattern could not find bilinear pattern with frequency " << frequency << " within threshold " << frequencyThreshold;
    return false;
  }

  // parse freq data
  stringTokenizer(vec, st);
  if (vec.size() > 2)
  {
    if (!isValidNumber(vec[0], minfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern minimum frequency" << std::endl;
      return 1;
    }
    double maxfreq = 0;
    if (!isValidNumber(vec[1], maxfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern maximum frequency" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[2], stepfreq))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern step frequency" << std::endl;
      return 1;
    }
    if (stepfreq == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricGainAntPattern can not use step frequency of 0" << std::endl;
      return 1;
    }
    if (minfreq == maxfreq && minfreq == 0)
    {
      SIM_ERROR << "SymmetricGainAntPattern could not determine frequency limits";
    }
    numfreq = size_t(floor((maxfreq - minfreq) / stepfreq)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricGainAntPattern expected 3 values for frequency limits";
    return false;
  }

  // parse azimuth data
  if (getTokens(in, vec, 3))
  {
    if (!isValidNumber(vec[0], minaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern minimum azimuth" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[1], maxaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern maximum azimuth" << std::endl;
      return 1;
    }
    double stepaz = 0;
    if (!isValidNumber(vec[2], stepaz))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern step azimuth" << std::endl;
      return 1;
    }
    if (stepaz == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricGainAntPattern can not use step azimuth of 0" << std::endl;
      return 1;
    }
    numaz = size_t(floor((maxaz - minaz) / stepaz)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricGainAntPattern expecting 3 values for azimuth limits";
    return false;
  }

  // parse elevation data
  if (getTokens(in, vec, 3))
  {
    if (!isValidNumber(vec[0], minel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern minimum elevation" << std::endl;
      return 1;
    }
    if (!isValidNumber(vec[1], maxel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern maximum elevation" << std::endl;
      return 1;
    }
    double stepel = 0;
    if (!isValidNumber(vec[2], stepel))
    {
      SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern step elevation" << std::endl;
      return 1;
    }
    if (stepel == 0.) // Protect against divide by zero below
    {
      SIM_ERROR << "SymmetricGainAntPattern can not use step elevation of 0" << std::endl;
      return 1;
    }
    numel = size_t((floor(maxel - minel) / stepel)) + 1;
  }
  else
  {
    SIM_ERROR << "SymmetricGainAntPattern expected 3 values for elevation limits";
    return false;
  }

  sap->initialize(minaz, maxaz, numaz, minel, maxel, numel);

  // parse mag phase pairs
  for (i = 0; i < numfreq; ++i)
  {
    double currentFreq = minfreq + i * stepfreq;
    if (frequency < currentFreq + frequencyThreshold && frequency > currentFreq - frequencyThreshold)
    {
      freqFound = true;
      for (j = 0; j < numaz; ++j)
      {
        for (k = 0; k < numel; ++k)
        {
          st.clear();
          getStrippedLine(in, st);
          stringTokenizer(vec, st);
          if (!vec.empty())
          {
            if (!isValidNumber(vec[0], magnitude))
            {
              SIM_ERROR << "Encountered invalid number for SymmetricGainAntPattern magnitude" << std::endl;
              return 1;
            }
            (*sap)(j, k) = magnitude;
          }
          else
          {
            SIM_ERROR << "SymmetricGainAntPattern expected magnitude and phase";
            return false;
          }
          if (!in)
          {
            SIM_ERROR << "SymmetricGainAntPattern ran out of data for frequency " << frequency;
            return false;
          }
        }
      }
    }
    else
    {
      // skip over data until the next frequency
      for (j = 0; j < numaz*numel; ++j)
      {
        getStrippedLine(in, st);
      }
    }
  }

  if (!freqFound)
  {
    SIM_ERROR << "SymmetricGainAntPattern could not find bilinear pattern with frequency " << frequency << " within threshold " << frequencyThreshold;
    return false;
  }

  return true;
}

bool readPattern(SymmetricGainAntPattern *sap, const std::string &filename, double frequency, double frequencythreshold)
{
  std::fstream in(filename.c_str(), std::ios::in);
  return readPattern(sap, in, frequency, frequencythreshold);
}


  /* ************************************************************************** */

  float calculateGain(const std::map<float, float> *azimData,
    const std::map<float, float> *elevData,
    AntennaLobeType &lastLobe,
    float azim,
    float elev,
    float hbw,
    float vbw,
    float maxGain,
    bool applyWeight)
  {
    if (!azimData || azimData->size() == 0 || !elevData || elevData->size() == 0)
      return SMALL_DB_VAL;

    if (hbw == 0.f || vbw == 0.f)
    {
      assert(0); // hbw and vbw must be non-zero to avoid divide-by-zero errors
      return SMALL_DB_VAL;
    }

    std::map<float, float>::const_iterator iter;
    if (applyWeight == false)
    {
      iter = azimData->begin();
      std::map<float, float>::const_reverse_iterator riter = azimData->rbegin();
      if (azim < iter->first || azim > riter->first)
        return SMALL_DB_VAL;

      iter = elevData->begin();
      riter = elevData->rbegin();
      if (elev < iter->first || elev > riter->first)
        return SMALL_DB_VAL;
    }

    // Compute angular distance in normalized beam widths
    double azim_bw = azim / hbw;
    double elev_bw = elev / vbw;
    double phi = sqrt(square(azim_bw) + square(elev_bw));

    // Determine lobe
    if (phi < 1.29)
      lastLobe = ANTENNA_LOBE_MAIN;
    else if (phi < 4.0)
      lastLobe = ANTENNA_LOBE_SIDE;
    else if (phi < 5.0)
      lastLobe = ANTENNA_LOBE_SIDE;
    else
      lastLobe = ANTENNA_LOBE_BACK;

    double azim_ang = applyWeight ? sdkMin(phi * hbw, M_PI) : azim;
    double elev_ang = applyWeight ? sdkMin(phi * vbw, M_PI_2) : elev;
    double az_gain = SMALL_DB_VAL;
    double el_gain = SMALL_DB_VAL;

    // Azimuth data
    // gets the first element in map with an angle >= dazim
    iter = azimData->lower_bound(static_cast<float>(azim_ang));

    // checks if an element with a an angle >= azim_ang was found
    if (iter != azimData->end())
    {
      // checks if the obtained element's angle is equal to the given angle,
      // or if the obtained element is the first element in map
      if ((iter->first == azim_ang) || (iter == azimData->begin()))
      {
        az_gain = iter->second;
      }
      else
      {
        // the obtained element's angle is > the given angle and it is not
        // the first element in the map, so an interpolated angle between
        // the current element and the previous element will be used
        double hiGain = iter->second;
        double hiAng = iter->first;
        --iter;
        double loGain = iter->second;
        double loAng = iter->first;

        az_gain = linearInterpolate(loGain, hiGain, loAng, azim_ang, hiAng);
      }
    }
    else
    {
      // check for rounding errors due to casting
      iter = azimData->begin();
      std::map<float, float>::const_reverse_iterator riter = azimData->rbegin();
      if (areEqual(azim_ang, iter->first))
        az_gain = iter->second;
      else if (areEqual(azim_ang, riter->first))
        az_gain = riter->second;
      else
        az_gain = SMALL_DB_VAL;
    }

    // Elevation data
    // gets the first element in map with an angle >= elev_ang
    iter = elevData->lower_bound(static_cast<float>(elev_ang));

    // checks if an element with a an angle >= elev_ang was found
    if (iter != elevData->end())
    {
      // checks if the obtained element's angle is equal to the given angle,
      // or if the obtained element is the first element in map
      if ((iter->first == elev_ang) || (iter == elevData->begin()))
      {
        el_gain = iter->second;
      }
      else
      {
        // the obtained element's angle is > the given angle and it is not
        // the first element in the map, so an interpolated angle between
        // the current element and the previous element will be used
        double hiGain = iter->second;
        double hiAng = iter->first;
        --iter;
        double loGain = iter->second;
        double loAng = iter->first;

        el_gain = linearInterpolate(loGain, hiGain, loAng, elev_ang, hiAng);
      }
    }
    else
    {
      // check for rounding errors due to casting
      iter = elevData->begin();
      std::map<float, float>::const_reverse_iterator riter = elevData->rbegin();
      if (areEqual(elev_ang, iter->first))
        el_gain = iter->second;
      else if (areEqual(elev_ang, riter->first))
        el_gain = riter->second;
      else
        el_gain = SMALL_DB_VAL;
    }

    // Determine angles (alpha & beta) associated with normalized
    // azim / elev components.  They will be used to obtain a
    // 'weighted average' antenna loss value

    double gain;
    if (applyWeight)
    {
      double alpha, beta;
      if ((azim_bw == 0.0 && elev_bw == 0.0) || vbw == hbw)
      {
        gain = maxGain + (az_gain + el_gain) / 2.0;
      }
      else if (azim_bw <= elev_bw)
      {
        // since atan2 returns values between -pi and pi,
        // alpha and beta should be in rad instead of deg
        alpha = fabs(atan2(azim_bw, elev_bw));
        if (alpha > M_PI_2)
          alpha = M_PI - alpha;
        beta = M_PI_2 - alpha;
        gain = maxGain + (alpha * az_gain + beta * el_gain) / M_PI_2;
      }
      else
      {
        // since atan2 returns values between -pi and pi,
        // alpha and beta should be in rad instead of deg
        beta = fabs(atan2(elev_bw, azim_bw));
        if (beta > M_PI_2)
          beta = M_PI - beta;
        alpha = M_PI_2 - beta;
        gain = maxGain + (alpha * az_gain + beta * el_gain) / M_PI_2;
      }
    }
    else
    {
      gain = maxGain + (az_gain + el_gain) / 2.0;
    }

    return static_cast<float>(gain);
  }

/* ************************************************************************** */
/* AntennaPattern                                                             */
/* ************************************************************************** */

AntennaPattern* loadPatternFile(const std::string &filename, float freq)
{
  AntennaPattern* pattern = NULL;
  if (filename.empty())
    return pattern;

  if (simCore::caseCompare(filename, ANTENNA_STRING_ALGORITHM_SINXX) == 0)
  {
    pattern = new AntennaPatternSinXX;
  }
  else if (simCore::caseCompare(filename, ANTENNA_STRING_ALGORITHM_PEDESTAL) == 0)
  {
    pattern = new AntennaPatternPedestal;
  }
  else if (simCore::caseCompare(filename, ANTENNA_STRING_ALGORITHM_GAUSS) == 0)
  {
    pattern = new AntennaPatternGauss;
  }
  else if (simCore::caseCompare(filename, ANTENNA_STRING_ALGORITHM_OMNI) == 0)
  {
    pattern = new AntennaPatternOmni;
  }
  else if (simCore::caseCompare(filename, ANTENNA_STRING_ALGORITHM_CSCSQ) == 0)
  {
    pattern = new AntennaPatternCscSq;
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_TABLE))
  {
    AntennaPatternTable *antTable = new AntennaPatternTable;
    if (antTable->readPat(filename) != 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_RELATIVE))
  {
    AntennaPatternRelativeTable *antTable = new AntennaPatternRelativeTable;
    if (antTable->readPat(filename) != 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_BILINEAR))
  {
    AntennaPatternBiLinear *bilinear = new AntennaPatternBiLinear;
    if (bilinear->readPat(filename, (freq*1e6)) != 0)
    {
      delete bilinear;
    }
    else
    {
      pattern = bilinear;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_CRUISE))
  {
    AntennaPatternCRUISE *antTable = new AntennaPatternCRUISE;
    if (antTable->readPat(filename)!= 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_MONOPULSE))
  {
    AntennaPatternMonopulse *monopulse = new AntennaPatternMonopulse;
    if (monopulse->readPat(filename, (freq*1e6))!= 0)
    {
      delete monopulse;
    }
    else
    {
      pattern = monopulse;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_NSMA))
  {
    AntennaPatternNSMA *antTable = new AntennaPatternNSMA;
    if (antTable->readPat(filename)!= 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_EZNEC))
  {
    AntennaPatternEZNEC *antTable = new AntennaPatternEZNEC;
    if (antTable->readPat(filename) != 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }
  else if (hasExtension(filename, ANTENNA_STRING_EXTENSION_XFDTD))
  {
    AntennaPatternXFDTD *antTable = new AntennaPatternXFDTD;
    if (antTable->readPat(filename) != 0)
    {
      delete antTable;
    }
    else
    {
      pattern = antTable;
    }
  }

  return pattern;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

float AntennaPatternGauss::gain(const AntennaGainParameters &params)
{
  double var = sin(0.5*params.vbw_); // Avoid divide by zero below
  // 0.69314718055994530942 = ln(2)
  double antfac = -0.5 * (0.69314718055994530942 / square(var == 0.0 ? 1.0 : var));
  double patfac = exp(antfac * square(sin(angFixPI(params.elev_))));
  // Ereps clips below 0.03
  return static_cast<float>(params.refGain_ + 20. * log10((patfac < 0.03) ? 0.03 : patfac));
}

// ----------------------------------------------------------------------------

void AntennaPatternGauss::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_)
  {
    *min = minGain_ + params.refGain_;
    *max = maxGain_ + params.refGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  // determine min & max values
  AntennaGainParameters agp(params);
  agp.refGain_ = 0.;
  for (int jj = -90; jj <= 90; ++jj)
  {
    agp.elev_ = static_cast<float>(DEG2RAD*(jj));
    float radius = gain(agp);
    if (radius > SMALL_DB_COMPARE)
    {
      minGain_ = sdkMin(minGain_, radius);
    }
    maxGain_ = sdkMax(maxGain_, radius);
  } // end for jj

  *min = minGain_ + params.refGain_;
  *max = maxGain_ + params.refGain_;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

float AntennaPatternCscSq::gain(const AntennaGainParameters &params)
{
  double delev = angFixPI(params.elev_);
  double elevFactor;
  if (delev <= params.vbw_)
  {
    double onePlus = 1.0 + delev;
    if (params.vbw_ != 0.f)
      onePlus = 1.0 + delev/params.vbw_; // protect against divide by zero with params.vbw_
    else
      assert(0); //params.vbw_ should not be zero, would result in a divide by zero
    elevFactor = sdkMin(1.0, sdkMax(0.03, onePlus));
  }
  else
  {
    double denom = sin(fabs(delev));
    if (denom == 0.0)
      denom = 1.0; // protect against divide by zero below
    elevFactor = sin(params.vbw_ / denom);
  }

  if (elevFactor == 0.0)
    elevFactor = 0.03; // Set to minimum possible result from if block above to avoid log10(0) below
  return static_cast<float>(params.refGain_ + 20. * log10(elevFactor));
}

// ----------------------------------------------------------------------------

void AntennaPatternCscSq::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_)
  {
    *min = minGain_ + params.refGain_;
    *max = maxGain_ + params.refGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  // determine min & max values
  AntennaGainParameters agp(params);
  agp.refGain_ = 0.;
  for (int jj = -90; jj <= 90; ++jj)
  {
    agp.elev_ = static_cast<float>(DEG2RAD*(jj));
    float radius = gain(agp);
    if (radius > SMALL_DB_COMPARE)
    {
      minGain_ = sdkMin(minGain_, radius);
    }
    maxGain_ = sdkMax(maxGain_, radius);
  } // end for jj

  *min = minGain_ + params.refGain_;
  *max = maxGain_ + params.refGain_;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

float AntennaPatternSinXX::gain(const AntennaGainParameters &params)
{
  double delev = angFixPI(params.elev_);
  double dazim = angFixPI(params.azim_);

  // Avoid divide by zero below
  if (params.hbw_ == 0.f || params.vbw_ == 0.f)
    return params.refGain_;

  // Compute angular distance in normalized beam widths
  double phi = sqrt(square(dazim/params.hbw_) + square(delev/params.vbw_));

  // Compute antenna gain
  if (phi == 0.0)
    return params.refGain_;

  double gain = square(sin(2.783*phi) / (2.783*phi));
  gain = params.refGain_ + 10.0 * log10(gain);

  // Add sin x/x side lobe gain
  if (phi > M_2_SQRTPI)
    gain += params.firstLobe_ + 13.2;

  return static_cast<float>(gain);
}

// ----------------------------------------------------------------------------

void AntennaPatternSinXX::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_ && params.hbw_ == lastHbw_)
  {
    *min = minGain_ + params.refGain_;
    *max = maxGain_ + params.refGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  lastHbw_ = params.hbw_;
  // determine min & max values
  float radius;
  AntennaGainParameters agp(params);
  agp.refGain_ = 0.;
  for (int ii = -180; ii <= 180; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = -90; jj <= 90; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        minGain_ = sdkMin(minGain_, radius);
      }
      maxGain_ = sdkMax(maxGain_, radius);
    } // end for jj
  } // end for ii

  *min = minGain_ + params.refGain_;
  *max = maxGain_ + params.refGain_;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

float AntennaPatternOmni::gain(const AntennaGainParameters &params)
{
  return params.refGain_;
}

// ----------------------------------------------------------------------------

void AntennaPatternOmni::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  *min = params.refGain_;
  *max = params.refGain_;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

float AntennaPatternPedestal::gain(const AntennaGainParameters &params)
{
  double gain = 0.;

  double delev = angFixPI(params.elev_);
  double dazim = angFixPI(params.azim_);

  // Avoid divide by zero below
  if (params.hbw_ == 0.f || params.vbw_ == 0.f)
    return params.refGain_;

  // Compute angular distance in normalized beam widths
  double phi = (sqrt(square(dazim/params.hbw_) + square(delev/params.vbw_)));

  // Determine lobe and compute antenna gain
  if (phi < 1.29)
  {
    gain = params.refGain_ - 12.0 * square(phi);
  }
  else if (phi < 4.00)
  {
    gain = params.refGain_ - 20.0;
  }
  else if (phi < 5.00)
  {
    gain = 5.0 * params.refGain_ - phi * (params.refGain_ - 10.0) - 60.0;
  }
  else
  {
    gain = -10.0;
  }

  if (gain < -10.0)
    gain = -10.0;

  return static_cast<float>(gain);
}

// ----------------------------------------------------------------------------

void AntennaPatternPedestal::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_ && params.hbw_ == lastHbw_ && params.refGain_ == lastGain_)
  {
    *min = minGain_;
    *max = maxGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  lastHbw_ = params.hbw_;
  lastGain_ = params.refGain_;
  // determine min & max values
  float radius;
  AntennaGainParameters agp(params);
  for (int ii = -180; ii <= 180; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = -90; jj <= 90; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        minGain_ = sdkMin(minGain_, radius);
      }
      maxGain_ = sdkMax(maxGain_, radius);
    } // end for jj
  } // end for ii

  *min = minGain_;
  *max = maxGain_;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternTable methods

AntennaPatternTable::AntennaPatternTable(bool type)
  : AntennaPattern(),
  beamWidthType_(type),
  lastVbw_(-FLT_MAX),
  lastHbw_(-FLT_MAX),
  lastGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

float AntennaPatternTable::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;
  AntennaLobeType lastLobe;
  return calculateGain(&azimData_,
    &elevData_,
    lastLobe,
    static_cast<float>(angFixPI(params.azim_)),
    static_cast<float>(angFixPI2(params.elev_)),
    params.hbw_,
    params.vbw_,
    params.refGain_,
    params.weighting_);
}

/// --------------------------------------------------------------------------
int AntennaPatternTable::readPat(std::istream& fp)
{
  int i, j;
  short symmetry;
  short tableSize[4];
  float *value[4];
  float *gain[4];
  float azim, elev;
  valid_ = false;
  std::string st;
  std::vector<std::string> tmpvec;

  // check for comments preceding data
  do
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "Antenna Table EOF reached while reading antenna pattern table data" << std::endl;
      return 1;
    }
    stringTokenizer(tmpvec, st);
  } while (st.empty() || tmpvec[0] == "//" || tmpvec[0] == "#");

  // Read in type and symmetry
  if (tmpvec.size() < 2)
  {
    SIM_ERROR << "Invalid number of tokens for antenna pattern table type and symmetry" << std::endl;
    return 1;
  }
  short type = 0;
  if (!isValidNumber(tmpvec[0], type))
  {
    SIM_ERROR << "Encountered invalid number for antenna pattern table type" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], symmetry))
  {
    SIM_ERROR << "Encountered invalid number for antenna pattern table symmetry" << std::endl;
    return 1;
  }

  if (!(type == 1 || type == 0))
  {
    SIM_ERROR << "Antenna Table Type must be 0 or 1 : " << type << std::endl;
    return 1;
  }
  beamWidthType_ = (type != 0);

  if (!(symmetry == 1 || symmetry == 2 || symmetry == 4))
  {
    SIM_ERROR << "Antenna Table Symmetry must be 1, 2 or 4 : " << symmetry << std::endl;
    return 1;
  }

  // Read in pattern tables
  double dblVal=0;
  for (i = 0; i < symmetry; i++)
  {
    //  Read table size
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "Antenna Table EOF reached while reading table size" << std::endl;
      return 1;
    }
    stringTokenizer(tmpvec, st);

    if (tmpvec.empty())
    {
      SIM_ERROR << "Invalid number of tokens for antenna pattern table size" << std::endl;
      return 1;
    }
    if (!isValidNumber(tmpvec[0], tableSize[i]))
    {
      SIM_ERROR << "Encountered invalid number for antenna table size" << std::endl;
      return 1;
    }
    value[i] = new float[tableSize[i]];
    gain[i] = new float[tableSize[i]];
    for (j = 0; j < tableSize[i]; j++)
    {
      dblVal=0;
      if (!getStrippedLine(fp, st))
      {
        SIM_ERROR << "Antenna Table EOF reached while reading data" << std::endl;
        return 1;
      }
      stringTokenizer(tmpvec, st);

      if (tmpvec.size() < 2)
      {
        SIM_ERROR << "Invalid number of tokens for antenna pattern table angle and gain" << std::endl;
        return 1;
      }
      if (!isValidNumber(tmpvec[0], dblVal))
      {
        SIM_ERROR << "Encountered invalid number for antenna table angle" << std::endl;
        return 1;
      }
      if (!isValidNumber(tmpvec[1], gain[i][j]))
      {
        SIM_ERROR << "Encountered invalid number for antenna table gain" << std::endl;
        return 1;
      }

      // convert degrees to radians
      if (beamWidthType_ == false)
      {
        value[i][j] = static_cast<float>(DEG2RAD*(dblVal));
      }
    }
  }

  if (beamWidthType_)
    beamWidthType_ = false;

  // The antenna symmetry value indicates the number of tables the user is going to provide.
  switch (symmetry)
  {
    // If the symmetry is 1, then the user will provide the [0, 180] azimuth table.
    // This table will be reused for the other three tables.
  case 1:
    {
      for (i = 0; i < tableSize[0]; i++)
      {
        azim = static_cast<float>(angFixPI(value[0][i]));
        elev = static_cast<float>(angFixPI2(value[0][i]));
        azimData_[azim] = gain[0][i];
        elevData_[elev] = gain[0][i];
        // mirror missing data
        azimData_[-azim] = gain[0][i];
        elevData_[-elev] = gain[0][i];
      }
    }
    break;

    // If the symmetry is 2, then the user will provide the [0, 180] azimuth table
    // and the [0, 90] elevation table. The tables will be reused for the missing
    // azimuth, and elevation tables.
  case 2:
    {
      for (i = 0; i < tableSize[0]; i++)
      {
        azim = static_cast<float>(angFixPI(value[0][i]));
        azimData_[azim] = gain[0][i];
        // mirror missing data
        azimData_[-azim] = gain[0][i];
      }
      for (i = 0; i < tableSize[1]; i++)
      {
        elev = static_cast<float>(angFixPI2(value[1][i]));
        elevData_[elev] = gain[1][i];
        // mirror missing data
        elevData_[-elev] = gain[1][i];
      }
    }
    break;

    // If the symmetry is 4, the user will provide all four azimuth and elevation tables.
  case 4:
    {
      for (i = 0; i < tableSize[0]; i++)
      {
        azim = static_cast<float>(angFixPI(value[0][i]));
        azimData_[azim] = gain[0][i];
      }
      for (i = 0; i < tableSize[1]; i++)
      {
        azim = static_cast<float>(angFixPI(value[1][i]));
        azimData_[azim] = gain[1][i];
      }
      for (i = 0; i < tableSize[2]; i++)
      {
        elev = static_cast<float>(angFixPI2(value[2][i]));
        elevData_[elev] = gain[2][i];
      }
      for (i = 0; i < tableSize[3]; i++)
      {
        elev = static_cast<float>(angFixPI2(value[3][i]));
        elevData_[elev] = gain[3][i];
      }
    }
    break;
  }

  for (i = 0; i < symmetry; i++)
  {
    delete [] value[i];
    delete [] gain[i];
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

void AntennaPatternTable::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_ && params.hbw_ == lastHbw_ && params.refGain_ == lastGain_ && minGain_ != -SMALL_DB_VAL)
  {
    *min = minGain_;
    *max = maxGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  lastHbw_ = params.hbw_;
  lastGain_ = params.refGain_;
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
  // determine min & max values
  float radius;
  AntennaGainParameters agp(params);
  agp.weighting_ = false;
  for (int ii = -180; ii <= 180; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = -90; jj <= 90; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        minGain_ = sdkMin(minGain_, radius);
      }
      maxGain_ = sdkMax(maxGain_, radius);
    } // end for jj
  } // end for ii
  *min = minGain_;
  *max = maxGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternTable::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
      }
    }
  }
  return st;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternRelativeTable methods

AntennaPatternRelativeTable::AntennaPatternRelativeTable()
  : AntennaPattern(),
  lastVbw_(-FLT_MAX),
  lastHbw_(-FLT_MAX),
  lastGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

float AntennaPatternRelativeTable::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;
  AntennaLobeType lastLobe;
  return calculateGain(&azimData_,
    &elevData_,
    lastLobe,
    static_cast<float>(angFixPI(params.azim_)),
    static_cast<float>(angFixPI2(params.elev_)),
    params.hbw_,
    params.vbw_,
    params.refGain_,
    params.weighting_);
}

// ----------------------------------------------------------------------------

int AntennaPatternRelativeTable::readPat_(std::istream& fp)
{
  assert(fp);

  int i;
  std::string st;
  std::string delimiter = " \t\n\r";
  std::vector<std::string> tmpvec;
  valid_ = false;

  // Antenna pattern file based on 2-D azimuth and 2-D elevation data
  // angles are in degrees
  // Gains are in relative dB, max gain in table should be 0.0, rest
  // of data is referenced to the main beam.)
  // Tue Dec 21 18:49:51 EST 1999

  // Read in Azimuth & Elevation limits
  do
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "Relative Table EOF reached" << std::endl;
      return 1;
    }
    stringTokenizer(tmpvec, st, delimiter);
  } while (st.empty() || tmpvec[0] == "//" || tmpvec[0] == "#");

  int azimLen = 0;
  int elevLen = 0;
  if (tmpvec.size() >= 2)
  {
    if (!isValidNumber(tmpvec[0], azimLen))
    {
      SIM_ERROR << "Encountered invalid number for relative Table azimuth length" << std::endl;
      return 1;
    }
    if (!isValidNumber(tmpvec[1], elevLen))
    {
      SIM_ERROR << "Encountered invalid number for relative Table elevation length" << std::endl;
      return 1;
    }
  }
  else
  {
    SIM_ERROR << "Relative Table azim and elev length not found" << std::endl;
    return 1;
  }

  if (azimLen < 2)
  {
    SIM_ERROR << "Relative Table azim length < 2 : " << azimLen << std::endl;
    return 1;
  }

  if (elevLen < 2)
  {
    SIM_ERROR << "Relative Table elev length < 2 : " << elevLen << std::endl;
    return 1;
  }

  float tmp=0;
  // Read in azimuth pattern data
  for (i = 0; i < azimLen; i++)
  {
    do
    {
      if (!getStrippedLine(fp, st))
      {
        SIM_ERROR << "Relative Table EOF reached while reading azim data" << std::endl;
        return 1;
      }
      stringTokenizer(tmpvec, st, delimiter);
    } while (st.empty() || tmpvec[0] == "//" || tmpvec[0] == "#");
    if (tmpvec.size() >= 2)
    {
      double azAng = 0;
      if (!isValidNumber(tmpvec[0], azAng))
      {
        SIM_ERROR << "Encountered invalid number for Relative Table azimuth angle" << std::endl;
        return 1;
      }
      if (!isValidNumber(tmpvec[1], tmp))
      {
        SIM_ERROR << "Encountered invalid number for Relative Table azimuth data" << std::endl;
        return 1;
      }
      // map key stored as radians
      azimData_[static_cast<float>(angFixPI(DEG2RAD*azAng))] = tmp;
    }
    else
    {
      SIM_ERROR << "Relative Table corresponding azim angle and gain value not found" << std::endl;
      return 1;
    }
  }

  // determine if -180 or 180 is missing
  float mpi = static_cast<float>(M_PI);
  std::map<float, float>::iterator iter180 = azimData_.find(mpi);
  std::map<float, float>::iterator iterMinus180 = azimData_.find(-mpi);
  if (iter180 != azimData_.end() && iterMinus180 == azimData_.end())
    azimData_[-mpi] = iter180->second;
  else if (iter180 == azimData_.end() && iterMinus180 != azimData_.end())
    azimData_[mpi] = iterMinus180->second;

  // Read in elevation pattern data
  for (i = 0; i < elevLen; i++)
  {
    do
    {
      if (!getStrippedLine(fp, st))
      {
        SIM_ERROR << "Relative Table EOF reached while reading elev data" << std::endl;
        return 1;
      }
      stringTokenizer(tmpvec, st, delimiter);
    } while (st.empty() || tmpvec[0] == "//" || tmpvec[0] == "#");
    if (tmpvec.size() >= 2)
    {
      double elAng = 0;
      if (!isValidNumber(tmpvec[0], elAng))
      {
        SIM_ERROR << "Encountered invalid number for Relative Table elevation angle" << std::endl;
        return 1;
      }
      if (!isValidNumber(tmpvec[1], tmp))
      {
        SIM_ERROR << "Encountered invalid number for Relative Table elevation data" << std::endl;
        return 1;
      }
      // map key stored as radians
      elevData_[static_cast<float>(angFixPI(DEG2RAD*elAng))] = tmp;
    }
    else
    {
      SIM_ERROR << "Relative Table: corresponding elev angle and gain value not found" << std::endl;
      return 1;
    }
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

void AntennaPatternRelativeTable::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.vbw_ == lastVbw_ && params.hbw_ == lastHbw_ && params.refGain_ == lastGain_ && minGain_ != -SMALL_DB_VAL)
  {
    *min = minGain_;
    *max = maxGain_;
    return;
  }

  lastVbw_ = params.vbw_;
  lastHbw_ = params.hbw_;
  lastGain_ = params.refGain_;
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
  // determine min & max values
  float radius;
  AntennaGainParameters agp(params);
  agp.weighting_ = false;
  for (int ii = -180; ii <= 180; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = -90; jj <= 90; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        minGain_ = sdkMin(minGain_, radius);
      }
      maxGain_ = sdkMax(maxGain_, radius);
    } // end for jj
  } // end for ii
  *min = minGain_;
  *max = maxGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternRelativeTable::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat_(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
      }
    }
  }
  return st;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternCRUISE methods

AntennaPatternCRUISE::AntennaPatternCRUISE()
  : AntennaPattern(),
  azimLen_(0),
  elevLen_(0),
  freqLen_(0),
  azimMin_(0),
  elevMin_(0),
  azimStep_(0),
  elevStep_(0),
  freqData_(NULL),
  azimData_(NULL),
  elevData_(NULL)
{}

// ----------------------------------------------------------------------------

void AntennaPatternCRUISE::reset_()
{
  int i;
  for (i = 0; i < freqLen_; i++)
  {
    if (azimData_ && azimData_[i])
      delete [] azimData_[i];
    if (elevData_ && elevData_[i])
      delete [] elevData_[i];
  }

  delete [] freqData_;
  delete [] azimData_;
  delete [] elevData_;

  valid_ = false;
  filename_.clear();
  azimLen_= 0;
  elevLen_= 0;
  freqLen_= 0;
  azimMin_= 0;
  elevMin_= 0;
  azimStep_= 0;
  elevStep_= 0;
  freqData_ = NULL;
  azimData_ = NULL;
  elevData_ = NULL;
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
}

// ----------------------------------------------------------------------------

float AntennaPatternCRUISE::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;

  int alowindex=0;
  int elowindex=0;
  int flowindex=0;
  double adelta=0;
  double edelta=0;
  double fdelta=0;
  double temp=0;

  double dazim = RAD2DEG*(angFixPI(params.azim_));
  double delev = RAD2DEG*(angFixPI(params.elev_));

  // need at least two points to interpolate
  assert(azimLen_ >= 2 && freqLen_ >= 2);

  // Interpolate azimuth
  if (dazim <= azimMin_)
  {
    alowindex = 0;
    adelta    = 0.0;
  }
  else if (dazim >= azimMin_ + azimStep_ * (azimLen_-1))
  {
    alowindex = azimLen_-2;
    adelta    = 1.0;
  }
  else
  {
    temp      = (dazim-azimMin_);
    if (azimStep_ != 0.0)
      temp    = temp/azimStep_;
    alowindex = static_cast<int>(floor(temp));
    adelta    = temp - alowindex;
  }

  // Interpolate elevation
  if (delev <= elevMin_)
  {
    elowindex = 0;
    edelta    = 0.0;
  }
  else if (delev >= elevMin_ + elevStep_ * (elevLen_-1))
  {
    elowindex = elevLen_-2;
    edelta    = 1.0;
  }
  else
  {
    temp      = (delev-elevMin_);
    if (elevStep_ != 0.0)
      temp    = temp/elevStep_;
    elowindex = static_cast<int>(floor(temp));
    edelta    = temp - elowindex;
  }

  // Interpolate frequency
  if (params.freq_ <= freqData_[0])
  {
    flowindex  = 0;
    fdelta     = 0.0;
  }
  else if (params.freq_ >= freqData_[freqLen_-1])
  {
    flowindex  = freqLen_-2;
    fdelta     = 1.0;
  }
  else
  {
    for (int i = 1; i < freqLen_; i++)
    {
      if (params.freq_ < freqData_[i])
      {
        flowindex = i - 1;
        fdelta = (params.freq_ - freqData_[flowindex]);
        double denom = (freqData_[flowindex + 1] - freqData_[flowindex]);
        if (denom != 0.0)
          fdelta = fdelta / denom;
        break;
      }
    }
  }

  double azGain = (azimData_[flowindex  ][alowindex  ]*(1.0-fdelta)*(1.0-adelta) +
    azimData_[flowindex  ][alowindex+1]*(1.0-fdelta)*     adelta  +
    azimData_[flowindex+1][alowindex  ]*     fdelta *(1.0-adelta) +
    azimData_[flowindex+1][alowindex+1]*     fdelta *     adelta);

  double elGain = (elevData_[flowindex  ][elowindex  ]*(1.0-fdelta)*(1.0-edelta) +
    elevData_[flowindex  ][elowindex+1]*(1.0-fdelta)*     edelta  +
    elevData_[flowindex+1][elowindex  ]*     fdelta *(1.0-edelta) +
    elevData_[flowindex+1][elowindex+1]*     fdelta *     edelta);

  // CRUISE Antenna Table data are saved as voltage gains instead of power gains
  // We expect all gains to be power gains, hence the square.
  return static_cast<float>(square(azGain * elGain));
}

// ----------------------------------------------------------------------------

void AntennaPatternCRUISE::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  *min = minGain_;
  *max = maxGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternCRUISE::readPat_(std::istream& fp)
{
  assert(fp);

  int i, j;
  int tmpFreq=0;
  double tmpData=0;
  valid_ = false;

  // ##### //

  // original CRUISE antenna patterns consisted of two files,
  // they have been merged into a single file, azimuth info
  // first, followed by elevation data.
  // Fri Oct  1 15:57:46 EDT 1999
  reset_();

  // Read in Azimuth #angles and #freq
  fp >> azimLen_ >> freqLen_;

  // Read in Azimuth #angle limits
  fp >> azimMin_ >> azimStep_;

  // Allocate tables
  freqData_ = new double[freqLen_];
  elevData_ = new double*[freqLen_];
  azimData_ = new double*[freqLen_];

  // Read in freq pattern table
  for (i = 0; i < freqLen_; i++)
  {
    fp >> freqData_[i];
    // convert GHz to Hz
    freqData_[i] *= 1e09;
  }

  // Read in azim pattern tables
  for (i = 0; i < freqLen_; i++)
  {
    azimData_[i] = new double[azimLen_];
    for (j = 0; j < azimLen_; j++)
    {
      fp >> azimData_[i][j];
    }
  }

  //
  // now handle elevation data ...
  //

  // Read in Elevation #angles and #freq
  fp >> elevLen_ >> tmpFreq;

  if ((azimLen_ != elevLen_ && tmpFreq != freqLen_))
  {
    delete [] elevData_;
    elevData_ = 0;
    SIM_ERROR << "CRUISE azimLen_(" << azimLen_ << ") != elevLen_(" << elevLen_ << ") or freqLen_s (" << tmpFreq << ", "<< freqLen_ << ") do not match!" << std::endl;
    return 1;
  }

  // Read in Elevation #angle limits
  fp >> elevMin_ >> elevStep_;

  // Read in freq pattern table
  // This info should be the exact same data found in the azimuth portion.
  for (i = 0; i < freqLen_; i++)
  {
    fp >> tmpData;
#ifndef NDEBUG
    // convert GHz to Hz
    assert(floor(0.5 + freqData_[i]) == floor(0.5 + (tmpData*1e09)));
#endif
  }

  // Read in elev pattern tables
  for (i = 0; i < freqLen_; i++)
  {
    elevData_[i] = new double[elevLen_];
    for (j = 0; j < elevLen_; j++)
    {
      fp >> elevData_[i][j];
    }
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

int AntennaPatternCRUISE::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat_(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
        // determine min & max values
        float radius;
        AntennaGainParameters agp;
        agp.freq_ = freqData_[0];
        for (int ii = -180; ii <= 180; ++ii)
        {
          agp.azim_ = static_cast<float>(DEG2RAD*(ii));
          for (int jj = -90; jj <= 90; ++jj)
          {
            agp.elev_ = static_cast<float>(DEG2RAD*(jj));
            radius = gain(agp);
            if (radius > SMALL_DB_COMPARE)
            {
              minGain_ = sdkMin(minGain_, radius);
            }
            maxGain_ = sdkMax(maxGain_, radius);
          } // end for jj
        } // end for ii
      } // end if st == 0
    } // end if inFile.is_open
  }
  return st;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternMonopulse methods

AntennaPatternMonopulse::AntennaPatternMonopulse()
  : AntennaPattern(),
  freq_(0),
  minDelGain_(-SMALL_DB_VAL),
  maxDelGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

AntennaPatternMonopulse::~AntennaPatternMonopulse()
{
  reset_();
}

// ----------------------------------------------------------------------------

void AntennaPatternMonopulse::reset_()
{
  valid_ = false;
  freq_ = 0.0;
  filename_.clear();
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
  minDelGain_ = -SMALL_DB_VAL;
  maxDelGain_ = SMALL_DB_VAL;
}

// ----------------------------------------------------------------------------

float AntennaPatternMonopulse::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;

  std::complex<double> magph;

  if (params.delta_)
  {
    try
    {
      magph = BilinearLookup(delPat_, RAD2DEG*(params.azim_), RAD2DEG*(params.elev_));
    }
    catch (const SymmetricAntennaPatternLimitException)
    {
      return SMALL_DB_VAL;
    }
  }
  else
  {
    try
    {
      magph = BilinearLookup(sumPat_, RAD2DEG*(params.azim_), RAD2DEG*(params.elev_));
    }
    catch (const SymmetricAntennaPatternLimitException)
    {
      return SMALL_DB_VAL;
    }
  }

  return static_cast<float>(params.refGain_ + linear2dB(std::abs(magph)));
}

// ----------------------------------------------------------------------------

void AntennaPatternMonopulse::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  if (params.delta_ && minDelGain_ == -SMALL_DB_VAL)
  {
    setMinMaxGain_(&minDelGain_, &maxDelGain_, params.refGain_, params.delta_);
  }
  else if (!params.delta_ && minGain_ == -SMALL_DB_VAL)
  {
    setMinMaxGain_(&minGain_, &maxGain_, params.refGain_, params.delta_);
  }

  *min = (params.delta_) ? minDelGain_ : minGain_;
  *max = (params.delta_) ? maxDelGain_ : maxGain_;
}

// ----------------------------------------------------------------------------

void AntennaPatternMonopulse::setMinMaxGain_(float *min, float *max, float maxGain, bool delta)
{
  assert(min && max);
  if (!min || !max)
    return;

  // determine min & max values
  double radius;
  double dmin = HUGE_VAL;
  double dmax = -HUGE_VAL;
  int maxAz = static_cast<int>((delta) ? delPat_.lut().maxX() : sumPat_.lut().maxX());
  int minAz = static_cast<int>((delta) ? delPat_.lut().minX() : sumPat_.lut().minX());
  int maxEl = static_cast<int>((delta) ? delPat_.lut().maxY() : sumPat_.lut().maxY());
  int minEl = static_cast<int>((delta) ? delPat_.lut().minY() : sumPat_.lut().minY());
  AntennaGainParameters agp;
  agp.refGain_ = maxGain;
  agp.delta_ = delta;
  for (int ii = minAz; ii <= maxAz; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = minEl; jj <= maxEl; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        dmin = sdkMin(dmin, radius);
      }
      dmax = sdkMax(dmax, radius);
    } // end for jj
  } // end for ii

  // convert linear magnitude to dB
  *min = static_cast<float>(dmin);
  *max = static_cast<float>(dmax);
}

// ----------------------------------------------------------------------------

int AntennaPatternMonopulse::readPat(const std::string& inFileName, double freq)
{
  reset_();
  if (inFileName.empty())
    return 1;

  freq_ = freq;

  if (!readPattern(&sumPat_, inFileName, "sum", freq_))
  {
    SIM_ERROR << inFileName << " sum channel failed to load" << std::endl;
    return 2;
  }

  if (!readPattern(&delPat_, inFileName, "diff", freq_))
  {
    SIM_ERROR << inFileName << " diff channel failed to load" << std::endl;
    return 2;
  }

  filename_ = inFileName;
  valid_ = true;

  return 0;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternBiLinear methods

AntennaPatternBiLinear::AntennaPatternBiLinear()
  : AntennaPattern(),
  freq_(0)
{}

// ----------------------------------------------------------------------------

AntennaPatternBiLinear::~AntennaPatternBiLinear()
{
  reset_();
}

// ----------------------------------------------------------------------------

void AntennaPatternBiLinear::reset_()
{
  valid_ = false;
  freq_ = 0.0;
  filename_.clear();
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
}

// ----------------------------------------------------------------------------

float AntennaPatternBiLinear::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;

  float gain = 0.f;
  try
  {
    gain = static_cast<float>(BilinearLookup(antPat_, RAD2DEG*(params.azim_), RAD2DEG*(params.elev_)));
  }
  catch (const SymmetricGainAntPatternLimitException)
  {
    // error, could not find requested angles
    return SMALL_DB_VAL;
  }
  // units are stored as dB, therefore add
  return params.refGain_ + gain;
}

// ----------------------------------------------------------------------------

void AntennaPatternBiLinear::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  *min = minGain_ + params.refGain_;
  *max = maxGain_ + params.refGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternBiLinear::readPat(const std::string& inFileName, double freq)
{
  reset_();
  if (inFileName.empty())
    return 1;

  freq_ = freq;

  if (!readPattern(&antPat_, inFileName, freq_))
  {
    SIM_ERROR << inFileName << " Bilinear pattern failed to load" << std::endl;
    return 2;
  }

  filename_ = inFileName;
  valid_ = true;

  // determine min & max values
  float radius;
  int maxAz = static_cast<int>(antPat_.lut().maxX());
  int minAz = static_cast<int>(antPat_.lut().minX());
  int maxEl = static_cast<int>(antPat_.lut().maxY());
  int minEl = static_cast<int>(antPat_.lut().minY());
  AntennaGainParameters agp;
  for (int ii = minAz; ii <= maxAz; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = minEl; jj <= maxEl; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        minGain_ = sdkMin(minGain_, radius);
      }
      maxGain_ = sdkMax(maxGain_, radius);
    } // end for jj
  } // end for ii

  return 0;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternNSMA methods

AntennaPatternNSMA::AntennaPatternNSMA()
  : AntennaPattern(),
  midBandGain_(0),
  halfPowerBeamWidth_(0),
  minFreq_(0),
  maxFreq_(0),
  minHVGain_(-SMALL_DB_VAL),
  maxHVGain_(SMALL_DB_VAL),
  minVHGain_(-SMALL_DB_VAL),
  maxVHGain_(SMALL_DB_VAL),
  minVVGain_(-SMALL_DB_VAL),
  maxVVGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

float AntennaPatternNSMA::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;
  AntennaLobeType lastLobe;
  switch (params.polarity_)
  {
  case POLARITY_VERTICAL:
    return calculateGain(&VVDataMap_,
      &ELVVDataMap_,
      lastLobe,
      params.azim_,
      params.elev_,
      halfPowerBeamWidth_,
      halfPowerBeamWidth_,
      midBandGain_ + params.refGain_,
      false);
    break;

  case POLARITY_HORZVERT:
  case POLARITY_RIGHTCIRC:
    return calculateGain(&HVDataMap_,
      &ELHVDataMap_,
      lastLobe,
      params.azim_,
      params.elev_,
      halfPowerBeamWidth_,
      halfPowerBeamWidth_,
      midBandGain_ + params.refGain_,
      false);
    break;

  case POLARITY_VERTHORZ:
  case POLARITY_LEFTCIRC:
    return calculateGain(&VHDataMap_,
      &ELVHDataMap_,
      lastLobe,
      params.azim_,
      params.elev_,
      halfPowerBeamWidth_,
      halfPowerBeamWidth_,
      midBandGain_ + params.refGain_,
      false);
    break;

  default:
    return calculateGain(&HHDataMap_,
      &ELHHDataMap_,
      lastLobe,
      params.azim_,
      params.elev_,
      halfPowerBeamWidth_,
      halfPowerBeamWidth_,
      midBandGain_ + params.refGain_,
      false);
    break;
  }
}

// ----------------------------------------------------------------------------

void AntennaPatternNSMA::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  switch (params.polarity_)
  {
  case POLARITY_VERTICAL:
    {
      if (minVVGain_ == -SMALL_DB_VAL)
      {
        setMinMax_(&minVVGain_, &maxVVGain_, params.refGain_, params.polarity_);
      }
      *min = minVVGain_;
      *max = maxVVGain_;
    }
    break;

  case POLARITY_HORZVERT:
  case POLARITY_RIGHTCIRC:
    {
      if (minHVGain_ == -SMALL_DB_VAL)
      {
        setMinMax_(&minHVGain_, &maxHVGain_, params.refGain_, params.polarity_);
      }
      *min = minHVGain_;
      *max = maxHVGain_;
    }
    break;

  case POLARITY_VERTHORZ:
  case POLARITY_LEFTCIRC:
    {
      if (minVHGain_ == -SMALL_DB_VAL)
      {
        setMinMax_(&minVHGain_, &maxVHGain_, params.refGain_, params.polarity_);
      }
      *min = minVHGain_;
      *max = maxVHGain_;
    }
    break;

  default:
    {
      if (minGain_ == -SMALL_DB_VAL)
      {
        setMinMax_(&minGain_, &maxGain_, params.refGain_, params.polarity_);
      }
      *min = minGain_;
      *max = maxGain_;
    }
    break;
  }
}

// ----------------------------------------------------------------------------

void AntennaPatternNSMA::setMinMax_(float *min, float *max, float maxGain, PolarityType polarity)
{
  assert(min && max);
  if (!min || !max)
    return;

  // determine min & max values
  float radius;
  AntennaGainParameters agp;
  agp.refGain_ = maxGain;
  agp.polarity_ = polarity;
  float fmin = -SMALL_DB_VAL;
  float fmax = SMALL_DB_VAL;
  for (int ii = -180; ii <= 180; ++ii)
  {
    agp.azim_ = static_cast<float>(DEG2RAD*(ii));
    for (int jj = -90; jj <= 90; ++jj)
    {
      agp.elev_ = static_cast<float>(DEG2RAD*(jj));
      radius = gain(agp);
      if (radius > SMALL_DB_COMPARE)
      {
        fmin = sdkMin(fmin, radius);
      }
      fmax = sdkMax(fmax, radius);
    } // end for jj
  } // end for ii
  *min = fmin;
  *max = fmax;
}

// ----------------------------------------------------------------------------

namespace
{
  /**
  * This functions verifies the incoming angle and antenna data for a NSMA pattern
  * @param[in ] tmpvec Temporary string vector holding parsed values for angle and data
  * @param[in ] patternType Type of antenna pattern being processed
  * @param[out] dataContainer Data contain for antenna pattern data
  * @param[out] errMsg Error message, empty if no error detected
  * @return 0 on success, !0 on error
  */
  int readNmsaData(const std::vector<std::string>& tmpvec, const std::string& patternType, std::map<float, float>& dataContainer, std::string& errMsg)
  {
    assert(tmpvec.size() > 1);
    double angle;
    float data;
    errMsg.clear();
    if (!isValidNumber(tmpvec[0], angle))
    {
      errMsg = "Encountered invalid number for NSMA " + patternType + " angle";
      return 1;
    }
    if (!isValidNumber(tmpvec[1], data))
    {
      errMsg = "Encountered invalid number for NSMA " + patternType + " data";
      return 1;
    }
    dataContainer[static_cast<float>(angFixPI(DEG2RAD*angle))] = data;
    return 0;
  }
}

int AntennaPatternNSMA::readPat_(std::istream& fp)
{
  assert(fp);

  int i;
  std::string st;
  std::vector<std::string> tmpvec;
  valid_ = false;

  // skip first 7 lines of NSMA file:
  // [Antenna Manufacturer] + CRLF
  // [Antenna Model number] + CRLF
  // [Comment] + CRLF
  // [FCC ID number] + CRLF
  // [reverse pattern ID number] + CRLF
  // [date of data] + CRLF
  // [Manufacturer ID Number (see file naming convention)] + CRLF
  for (i = 0; i < 7; i++)
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "EOF reached while processing NSMA data" << std::endl;
      return 1;
    }
  }

  // [frequency range] + CRLF
  if (getTokens(fp, tmpvec, 2, "-") == false)
  {
    SIM_ERROR << "Error processing NSMA frequency range" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[0], minFreq_))
  {
    SIM_ERROR << "Encountered invalid number for NSMA minimum frequency" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], maxFreq_))
  {
    SIM_ERROR << "Encountered invalid number for NSMA maximum frequency" << std::endl;
    return 1;
  }
  // convert MHz to Hz
  minFreq_ *= 1e6;
  maxFreq_ *= 1e6;

  // [mid-band gain] + CRLF
  if (getTokens(fp, tmpvec, 1) == false)
  {
    SIM_ERROR << "Error processing NSMA mid band gain" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[0], midBandGain_))
  {
    SIM_ERROR << "Encountered invalid number for NSMA midban gain" << std::endl;
    return 1;
  }

  // [Half-power beam width] + CRLF
  if (getTokens(fp, tmpvec, 1) == false)
  {
    SIM_ERROR << "Error processing NSMA half power beam width" << std::endl;
    return 1;
  }
  double halfPwrBW = 0;
  if (!isValidNumber(tmpvec[0], halfPwrBW))
  {
    SIM_ERROR << "Encountered invalid number for NSMA half power beam width" << std::endl;
    return 1;
  }
  halfPowerBeamWidth_ = static_cast<float>(DEG2RAD*halfPwrBW);

  // [polarization (char 7) + chr$(32) + datacount (char 7)  + chr$(32) + CRLF]
  // [angle(1) (char 7) + chr$(32) + relative gain in dB(char 7) + chr$(32) + CRLF]
  // Polarization must be in the set [HH|HV|VV|VH|ELHH|ELHV|ELVV|ELVH]

  // -180 deg < Angle(x) < 180 deg for [HH|HV|VV|VH]
  // -90 deg < Angle(x) < 90 deg for [ELHH|ELHV|ELVV|ELVH]
  // Angle(1) < Angle(2) < ... < Angle(datacount)
  // Relative Gain in dB < ~0 including sign

  std::string dataErrMsg;

  // Handle HH case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "HH")
  {
    SIM_ERROR << "NSMA HH pattern not found" << std::endl;
    return 1;
  }
  int dataCount = 0;
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA HH data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA HH data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "HH", HHDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // determine if -180 or 180 is missing
  float mpi = static_cast<float>(M_PI);
  std::map<float, float>::iterator iter180 = HHDataMap_.find(mpi);
  std::map<float, float>::iterator iterMinus180 = HHDataMap_.find(-mpi);
  if (iter180 != HHDataMap_.end() && iterMinus180 == HHDataMap_.end())
    HHDataMap_[-mpi] = iter180->second;
  else if (iter180 == HHDataMap_.end() && iterMinus180 != HHDataMap_.end())
    HHDataMap_[mpi] = iterMinus180->second;

  // Handle HV case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "HV")
  {
    SIM_ERROR << "NSMA HV pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA HV data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA HV data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "HV", HVDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // determine if -180 or 180 is missing
  iter180 = HVDataMap_.find(mpi);
  iterMinus180 = HVDataMap_.find(-mpi);
  if (iter180 != HVDataMap_.end() && iterMinus180 == HVDataMap_.end())
    HVDataMap_[-mpi] = iter180->second;
  else if (iter180 == HVDataMap_.end() && iterMinus180 != HVDataMap_.end())
    HVDataMap_[mpi] = iterMinus180->second;

  // Handle VV case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "VV")
  {
    SIM_ERROR << "NSMA VV pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA VV data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA VV data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "VV", VVDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // determine if -180 or 180 is missing
  iter180 = VVDataMap_.find(mpi);
  iterMinus180 = VVDataMap_.find(-mpi);
  if (iter180 != VVDataMap_.end() && iterMinus180 == VVDataMap_.end())
    VVDataMap_[-mpi] = iter180->second;
  else if (iter180 == VVDataMap_.end() && iterMinus180 != VVDataMap_.end())
    VVDataMap_[mpi] = iterMinus180->second;

  // Handle VH case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "VH")
  {
    SIM_ERROR << "NSMA VH pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA VH data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA VH data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "VH", VHDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // determine if -180 or 180 is missing
  iter180 = VHDataMap_.find(mpi);
  iterMinus180 = VHDataMap_.find(-mpi);
  if (iter180 != VHDataMap_.end() && iterMinus180 == VHDataMap_.end())
    VHDataMap_[-mpi] = iter180->second;
  else if (iter180 == VHDataMap_.end() && iterMinus180 != VHDataMap_.end())
    VHDataMap_[mpi] = iterMinus180->second;

  // Handle ELHH case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "ELHH")
  {
    SIM_ERROR << "NSMA ELHH pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA ELHH data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA ELHH data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "ELHH", ELHHDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // Handle ELHV case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "ELHV")
  {
    SIM_ERROR << "NSMA ELHV pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA ELHV data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA ELHV data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "ELHV", ELHVDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // Handle ELVV case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "ELVV")
  {
    SIM_ERROR << "NSMA ELVV pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA ELVV data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA ELVV data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "ELVV", ELVVDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  // Handle ELVH case
  if (getTokens(fp, tmpvec, 2) == false || tmpvec[0] != "ELVH")
  {
    SIM_ERROR << "NSMA ELVH pattern not found" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], dataCount))
  {
    SIM_ERROR << "Encountered invalid number for NSMA ELVH data count" << std::endl;
    return 1;
  }
  for (i = 0; i < dataCount; i++)
  {
    if (getTokens(fp, tmpvec, 2) == false)
    {
      SIM_ERROR << "Error processing NSMA ELVH data, expected two tokens" << std::endl;
      return 1;
    }
    if (readNmsaData(tmpvec, "ELVH", ELVHDataMap_, dataErrMsg) != 0)
    {
      SIM_ERROR << dataErrMsg << std::endl;
      return 1;
    }
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

int AntennaPatternNSMA::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat_(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
      }
    }
  }
  return st;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternEZNEC methods

AntennaPatternEZNEC::AntennaPatternEZNEC()
  : AntennaPattern(),
  frequency_(0),
  reference_(0),
  angleConvCCW_(true),
  minVertGain_(-SMALL_DB_VAL),
  maxVertGain_(SMALL_DB_VAL),
  minHorzGain_(-SMALL_DB_VAL),
  maxHorzGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

float AntennaPatternEZNEC::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;

  // adjust requested azim based on pattern's angle convention
  float azim = (angleConvCCW_) ? -params.azim_ : static_cast<float>((M_PI_2 + params.azim_));
  azim = static_cast<float>(RAD2DEG*(angFix2PI(azim)));
  float elev = static_cast<float>(RAD2DEG*(angFixPI2(params.elev_)));
  float gain = params.refGain_;
  try
  {
    switch (params.polarity_)
    {
    case POLARITY_VERTICAL:
      gain += BilinearLookup(vertData_, azim, elev);
      break;

    case POLARITY_HORIZONTAL:
      gain += BilinearLookup(horzData_, azim, elev);
      break;

    default:
      gain += BilinearLookup(totalData_, azim, elev);
      break;
    }
  }
  catch (const GainDataLimitException&)
  {
    return SMALL_DB_VAL;
  }
  return gain;
}

// ----------------------------------------------------------------------------

void AntennaPatternEZNEC::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  switch (params.polarity_)
  {
  case POLARITY_VERTICAL:
    *min = minVertGain_;
    *max = maxVertGain_;
    break;

  case POLARITY_HORIZONTAL:
    *min = minHorzGain_;
    *max = maxHorzGain_;
    break;

  default:
    *min = minGain_;
    *max = maxGain_;
    break;
  }
  *min += params.refGain_;
  *max += params.refGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternEZNEC::readPat_(std::istream& fp)
{
  assert(fp);
  std::string st;
  std::string delimiter = " \t\n\r";
  bool csv = false;
  std::vector<std::string> tmpvec;
  valid_ = false;

  // EZNEC version
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "EZNEC EOF reached while searching for EZNEC" << std::endl;
    return 1;
  }

  if (st.find("EZNEC") == std::string::npos)
  {
    SIM_ERROR << "EZNEC file identifier not found" << std::endl;
    return 1;
  }

  // Find Frequency
  do
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "EZNEC EOF reached while searching for Frequency" << std::endl;
      return 1;
    }
  } while (st.find("Frequency") == std::string::npos);

  // detect if file is delimited using spaces or commas
  if (st.find(",") != std::string::npos)
  {
    delimiter = ",";
    csv = true;
  }

  // set antenna pattern frequency
  stringTokenizer(tmpvec, st, delimiter);
  if (csv && tmpvec.size() > 1)
  {
    if (!isValidNumber(tmpvec[1], frequency_))
    {
      SIM_ERROR << "Encountered invalid number for EZNEC frequency" << std::endl;
      return 1;
    }
  }
  else if (!csv && tmpvec.size() > 2)
  {
    if (!isValidNumber(tmpvec[2], frequency_))
    {
      SIM_ERROR << "Encountered invalid number for EZNEC frequency" << std::endl;
      return 1;
    }
  }
  else
  {
    SIM_ERROR << "EZNEC Frequency line has incorrect # of tokens" << std::endl;
    return 1;
  }

  // Find Reference
  do
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "EZNEC EOF reached while searching for Reference" << std::endl;
      return 1;
    }
  } while (st.find("Reference") == std::string::npos);

  // verify incoming gain units are referenced to dBi
  if (stringCaseFind(st, "dBi") == std::string::npos)
  {
    SIM_ERROR << "EZNEC antenna pattern gain values must be in dB." << std::endl;
    return 1;
  }

  // set antenna pattern reference value
  stringTokenizer(tmpvec, st, delimiter);
  if (csv && tmpvec.size() > 1)
  {
    if (!isValidNumber(tmpvec[1], reference_))
    {
      SIM_ERROR << "Encountered invalid number for EZNEC reference" << std::endl;
      return 1;
    }
  }
  else if (!csv && tmpvec.size() > 2)
  {
    if (!isValidNumber(tmpvec[2], reference_))
    {
      SIM_ERROR << "Encountered invalid number for EZNEC reference" << std::endl;
      return 1;
    }
  }
  else
  {
    SIM_ERROR << "EZNEC Reference line has incorrect # of tokens" << std::endl;
    return 1;
  }

  // Find Azimuth Pattern
  do
  {
    if (!getStrippedLine(fp, st))
    {
      SIM_ERROR << "EZNEC EOF reached while searching for Azimuth Pattern" << std::endl;
      SIM_ERROR << "Elevation patterns are not supported." << std::endl;
      return 1;
    }
  } while (st.find("Azimuth Pattern") == std::string::npos);

  float minElev = 90;
  float maxElev = -90;
  size_t elevCnt = 0;
  float minAzim = 360;
  float maxAzim = 0;
  size_t azimCnt = 0;
  float value = 0;
  minVertGain_ = -SMALL_DB_VAL;
  maxVertGain_ = SMALL_DB_VAL;
  minHorzGain_ = -SMALL_DB_VAL;
  maxHorzGain_ = SMALL_DB_VAL;
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
  stringTokenizer(tmpvec, st, delimiter);
  if (!isValidNumber((csv ? tmpvec[1] : tmpvec[5]), value))
  {
    SIM_ERROR << "Encountered invalid number for EZNEC elevation" << std::endl;
    return 1;
  }
  minElev = sdkMin(value, minElev);
  maxElev = sdkMax(value, maxElev);
  elevCnt++;

  // process row header to determine EZNEC angle convention and polarizations
  //
  // Angle Convention – EZNEC allows you to represent azimuth angles in either of two ways.
  //
  // Compass Bearing – Zero is in the direction of the +y axis (at the top of the 2D azimuth plot display).
  // Angles increase as you go clockwise from zero.
  // Files saved in this format will have the following unit line:
  // "Bear","V dB","H dB","Tot dB"
  //
  // CCW From X Axis – Zero is in the direction of the +x axis (to the right of the 2D azimuth plot display).
  // Angles increase as you go counterclockwise from zero. This is the convention commonly used in mathematics and physics.
  // Files saved in this format will have the following unit line:
  // "Deg","V dB","H dB","Tot dB"
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "EZNEC EOF reached while searching for data row header" << std::endl;
    return 1;
  }

  // Prevent the loading of other possible EZNEC antenna pattern permutations
  // We only support Vert, Horz and Total gain patterns referenced to dB
  if (stringCaseFind(st, "V dB") == std::string::npos ||
    stringCaseFind(st, "H dB") == std::string::npos ||
    stringCaseFind(st, "Tot dB") == std::string::npos)
  {
    SIM_ERROR << "Vert, Horz and Total gain is the only EZNEC far field format supported" << std::endl;
    return 1;
  }

  // set angle convention of far field data
  angleConvCCW_ = (stringCaseFind(st, "Bear") == std::string::npos);

  std::vector<float> vVPol;
  std::vector<float> vHPol;
  std::vector<float> vTPol;
  size_t i = 0;
  float gainVal;

  // Read in remaining data to figure it out
  while (getStrippedLine(fp, st))
  {
    stringTokenizer(tmpvec, st, delimiter);
    if (stringCaseFind(st, "Azimuth Pattern") != std::string::npos)
    {
      // set current elevation value
      if (csv && tmpvec.size() > 1)
      {
        if (!isValidNumber(tmpvec[1], value))
        {
          SIM_ERROR << "Encountered invalid number for EZNEC elevation" << std::endl;
          return 1;
        }
      }
      else if (!csv && tmpvec.size() > 5)
      {
        if (!isValidNumber(tmpvec[5], value))
        {
          SIM_ERROR << "Encountered invalid number for EZNEC elevation" << std::endl;
          return 1;
        }
      }
      else
      {
        SIM_ERROR << "EZNEC Azimuth Pattern line has incorrect # of tokens" << std::endl;
        return 1;
      }
      minElev = sdkMin(value, minElev);
      maxElev = sdkMax(value, maxElev);
      elevCnt++;
    }
    else if (stringCaseFind(st, "Tot dB") != std::string::npos)
    {
      // skip row header and reset azimuth counter
      azimCnt = 0;
    }
    else if (tmpvec.size() >= 4 && stringIsRealNumber(tmpvec[0]))
    {
      // process vert, horiz and total gain patterns
      // EZNEC Pro also saves out circular and linear too
      if (!isValidNumber(tmpvec[0], value))
      {
        SIM_ERROR << "Encountered invalid number for EZNEC azimuth" << std::endl;
        return 1;
      }
      minAzim = sdkMin(value, minAzim);
      maxAzim = sdkMax(value, maxAzim);
      azimCnt++;

      if (!isValidNumber(tmpvec[1], gainVal))
      {
        SIM_ERROR << "Encountered invalid number for EZNEC V gain" << std::endl;
        return 1;
      }
      vVPol.push_back(gainVal);
      minVertGain_ = sdkMin(minVertGain_, gainVal);
      maxVertGain_ = sdkMax(maxVertGain_, gainVal);

      if (!isValidNumber(tmpvec[2], gainVal))
      {
        SIM_ERROR << "Encountered invalid number for EZNEC H gain" << std::endl;
        return 1;
      }
      vHPol.push_back(gainVal);
      minHorzGain_ = sdkMin(minHorzGain_, gainVal);
      maxHorzGain_ = sdkMax(maxHorzGain_, gainVal);

      if (!isValidNumber(tmpvec[3], gainVal))
      {
        SIM_ERROR << "Encountered invalid number for EZNEC T gain" << std::endl;
        return 1;
      }
      vTPol.push_back(gainVal);
      minGain_ = sdkMin(minGain_, gainVal);
      maxGain_ = sdkMax(maxGain_, gainVal);
    }
  }

  // verify data was processed
  if (vVPol.empty() || vHPol.empty() || vTPol.empty())
  {
    SIM_ERROR << "EZNEC antenna pattern data was not processed." << std::endl;
    return 1;
  }

  // initialize Bilinear LUTs
  vertData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);
  horzData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);
  totalData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);

  // copy data into LUTs and normalize pattern to 0 dBi
  size_t j = 0, k = 0;
  for (i = 0; i < vVPol.size(); i++)
  {
    if (j == azimCnt)
    {
      j = 0;
      k++;
    }
    vertData_(j, k) = vVPol[i] - reference_;
    horzData_(j, k) = vHPol[i] - reference_;
    totalData_(j, k) = vTPol[i] - reference_;
    j++;
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

int AntennaPatternEZNEC::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    minVertGain_ = -SMALL_DB_VAL;
    maxVertGain_ = SMALL_DB_VAL;
    minHorzGain_ = -SMALL_DB_VAL;
    maxHorzGain_ = SMALL_DB_VAL;
    minGain_ = -SMALL_DB_VAL;
    maxGain_ = SMALL_DB_VAL;
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat_(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
      }
    }
  }
  return st;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/// AntennaPatternXFDTD methods

AntennaPatternXFDTD::AntennaPatternXFDTD()
  : AntennaPattern(),
  reference_(0),
  minVertGain_(-SMALL_DB_VAL),
  maxVertGain_(SMALL_DB_VAL),
  minHorzGain_(-SMALL_DB_VAL),
  maxHorzGain_(SMALL_DB_VAL)
{}

// ----------------------------------------------------------------------------

float AntennaPatternXFDTD::gain(const AntennaGainParameters &params)
{
  if (!valid_) return SMALL_DB_VAL;

  // XFDTD pattern is offset  by 90
  float azim = static_cast<float>(RAD2DEG*(angFix2PI(params.azim_+M_PI_2)));
  float elev = static_cast<float>(RAD2DEG*(angFixPI2(params.elev_)));
  float gain = params.refGain_;
  try
  {
    switch (params.polarity_)
    {
    case POLARITY_VERTICAL:
      gain += BilinearLookup(vertData_, azim, elev);
      break;

    case POLARITY_HORIZONTAL:
      gain += BilinearLookup(horzData_, azim, elev);
      break;

    default:
      gain += BilinearLookup(totalData_, azim, elev);
      break;
    }
  }
  catch (const GainDataLimitException&)
  {
    return SMALL_DB_VAL;
  }
  return gain;
}

// ----------------------------------------------------------------------------

void AntennaPatternXFDTD::minMaxGain(float *min, float *max, const AntennaGainParameters &params)
{
  assert(min && max);
  if (!min || !max)
    return;

  switch (params.polarity_)
  {
  case POLARITY_VERTICAL:
    *min = minVertGain_;
    *max = maxVertGain_;
    break;

  case POLARITY_HORIZONTAL:
    *min = minHorzGain_;
    *max = maxHorzGain_;
    break;

  default:
    *min = minGain_;
    *max = maxGain_;
    break;
  }
  *min += params.refGain_;
  *max += params.refGain_;
}

// ----------------------------------------------------------------------------

int AntennaPatternXFDTD::readPat_(std::istream& fp)
{
  assert(fp);
  valid_ = false;

  // Email from Chad Pendley [pendley@remcom.com]:
  // Three-Dimensional Far-Zone Files
  // The default filetype for 3D Far-Zone files is a file format called UAN.
  // This file format is shared between Remcom's XFDTD and Wireless
  // Insite software packages. The file format consists of two parts, a
  // delimited parameters section, and a section containing all of the angle
  // data.

  // The parameters section will look something like the following:
  // begin_<parameters>
  // format free
  // phi_min 0
  // phi_max 360
  // phi_inc 5
  // theta_min 0
  // theta_max 180
  // theta_inc 5
  // complex
  // mag_phase
  // pattern gain
  // magnitude dB
  // maximum_gain 0
  // phase degrees
  // direction degrees
  // polarization theta_phi
  // end_<parameters>

  // The parameters should be self-explanatory based on the parameters of the
  // 3D Far-Zone requested. For example, the value after phi_min will
  // represent the minimum phi slice to save along with the file.

  // After the parameters section will follow a data section. This section
  // is not delimited by anything other than the end of file marker. There
  // will be up to 6 columns of data in this section, in the following order:

  // Theta-angle Phi-angle Theta-gain(db) Phi-gain(db) Theta-phase(degrees)
  // Phi-phase(degrees)

  // This order may be slightly different if you are not looking for any
  // slices in phi or theta. Also, note that some of the parameters will
  // directly affect the data values. For instance, if you change the
  // magnitude parameter to linear, the gain will be read as a linear
  // value into XFDTD. Similarly, the phase parameter can be changed to
  // radians, to read in the phase in radian units.

  float value;
  bool magLinear = false;
  std::string st;
  std::vector<std::string> tmpvec;

  // skip begin
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for begin parameters" << std::endl;
    return 1;
  }

  // skip format
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for format" << std::endl;
    return 1;
  }

  // phi_min
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing phi_min" << std::endl;
    return 1;
  }
  float minAzim = 0;
  if (!isValidNumber(tmpvec[1], minAzim))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD minimum phi" << std::endl;
    return 1;
  }

  // phi_max
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing phi_max" << std::endl;
    return 1;
  }
  float maxAzim = 0;
  if (!isValidNumber(tmpvec[1], maxAzim))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD maximum phi" << std::endl;
    return 1;
  }

  // phi_inc
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing phi_inc" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], value))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD phi increment" << std::endl;
    return 1;
  }
  if (value == 0.f) // Avoid divide by zero below
  {
    SIM_ERROR << "Cannot use XFDTD phi increment of 0" << std::endl;
    return 1;
  }
  size_t azimCnt = static_cast<size_t>(rint((maxAzim - minAzim)/value));

  // theta_min
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing theta_min" << std::endl;
    return 1;
  }
  float minElev = 0.f;
  if (!isValidNumber(tmpvec[1], minElev))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD minimum theta" << std::endl;
    return 1;
  }
  minElev -= 90.f;

  // theta_max
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing theta_max" << std::endl;
    return 1;
  }
  float maxElev = 0.f;
  if (!isValidNumber(tmpvec[1], maxElev))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD maximum theta" << std::endl;
    return 1;
  }
  maxElev -= 90.f;

  // theta_inc
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing theta_inc" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], value))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD theta increment" << std::endl;
    return 1;
  }
  if (value == 0.f) // Avoid divide by zero below
  {
    SIM_ERROR << "Cannot use XFDTD theta increment of 0" << std::endl;
    return 1;
  }
  size_t elevCnt = static_cast<size_t>(rint((maxElev - minElev)/value));
  elevCnt++;

  // skip complex
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for complex" << std::endl;
    return 1;
  }

  // skip mag_phase
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for mag_phase" << std::endl;
    return 1;
  }

  // skip pattern
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for pattern" << std::endl;
    return 1;
  }

  // magnitude
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for magnitude" << std::endl;
    return 1;
  }
  if (stringCaseFind(st, "dB") == std::string::npos)
    magLinear = true;

  // maximum_gain
  if (getTokens(fp, tmpvec, 2) == false)
  {
    SIM_ERROR << "XFDTD processing maximum_gain" << std::endl;
    return 1;
  }
  if (!isValidNumber(tmpvec[1], reference_))
  {
    SIM_ERROR << "Encountered invalid number for XFDTD maximum gain" << std::endl;
    return 1;
  }

  // phase
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for phase" << std::endl;
    return 1;
  }

  // skip direction
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for direction" << std::endl;
    return 1;
  }

  // skip polarization
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for polarization" << std::endl;
    return 1;
  }

  // skip end marker
  if (!getStrippedLine(fp, st))
  {
    SIM_ERROR << "XFDTD EOF reached while searching for end parameters" << std::endl;
    return 1;
  }

  // initialize Bilinear LUTs
  vertData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);
  horzData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);
  totalData_.initialize(minAzim, maxAzim, azimCnt, minElev, maxElev, elevCnt);

  // Read in remaining data and normalize pattern to 0 dBi
  size_t j = 0;
  size_t k = 0;
  float vVal;
  float hVal;
  float tVal;
  minVertGain_ = -SMALL_DB_VAL;
  maxVertGain_ = SMALL_DB_VAL;
  minHorzGain_ = -SMALL_DB_VAL;
  maxHorzGain_ = SMALL_DB_VAL;
  minGain_ = -SMALL_DB_VAL;
  maxGain_ = SMALL_DB_VAL;
  while (getStrippedLine(fp, st))
  {
    stringTokenizer(tmpvec, st);
    if (tmpvec.size() > 5)
    {
      if (j == azimCnt)
      {
        j = 0;
        k++;
      }
      if (!isValidNumber(tmpvec[2], value))
      {
        SIM_ERROR << "Encountered invalid number for XFDTD vertical gain" << std::endl;
        return 1;
      }
      vVal = static_cast<float>((magLinear ? linear2dB(value) : value) - reference_);
      vertData_(j, k) = vVal;
      minVertGain_ = sdkMin(minVertGain_, vVal);
      maxVertGain_ = sdkMax(maxVertGain_, vVal);

      if (!isValidNumber(tmpvec[3], value))
      {
        SIM_ERROR << "Encountered invalid number for XFDTD horizontal gain" << std::endl;
        return 1;
      }
      hVal = static_cast<float>((magLinear ? linear2dB(value) : value) - reference_);
      horzData_(j, k) = hVal;
      minHorzGain_ = sdkMin(minHorzGain_, hVal);
      maxHorzGain_ = sdkMax(maxHorzGain_, hVal);

      tVal = static_cast<float>(linear2dB(dB2Linear(vVal) + dB2Linear(hVal)));
      totalData_(j, k) = tVal;
      minGain_ = sdkMin(minGain_, tVal);
      maxGain_ = sdkMax(maxGain_, tVal);

      j++;
    }
  }

  valid_ = true;
  return 0;
}

// ----------------------------------------------------------------------------

int AntennaPatternXFDTD::readPat(const std::string& inFileName)
{
  int st=1;
  if (!inFileName.empty())
  {
    filename_.clear();
    std::fstream inFile;
    inFile.open(inFileName.c_str(), std::ios::in);
    if (inFile.is_open())
    {
      st = readPat_(inFile);
      inFile.close();
      if (st == 0)
      {
        filename_ = inFileName;
      }
    }
  }
  return st;
}

}