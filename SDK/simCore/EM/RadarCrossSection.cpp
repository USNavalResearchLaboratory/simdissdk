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
#include <algorithm>
#include <string>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cfloat>
#include <limits>

#include "simCore/String/Tokenizer.h"
#include "simCore/String/Format.h"
#include "simCore/String/UtfUtils.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Interpolation.h"
#include "simCore/Calc/Angle.h"
#include "simNotify/Notify.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/RadarCrossSection.h"

namespace
{

  bool getTokenLine(std::istream &inFile, std::string &str, size_t *lineNumber)
  {
    // track line number for error reporting purposes
    (*lineNumber)++;
    return simCore::getStrippedLine(inFile, str);
  }

  bool getFirstToken(std::istream &inFile, std::string &token, size_t *lineNumber)
  {
    std::string str;
    if (getTokenLine(inFile, str, lineNumber))
    {
      std::vector<std::string> vec;
      simCore::stringTokenizer(vec, str);
      if (!vec.empty())
      {
        token = vec[0];
        return true;
      }
    }
    return false;
  }

  /** Retrieve the RCS type from stream by inspecting contents for markers */
  simCore::RCSType getRCSType(std::istream& is)
  {
    simCore::RCSType rv = simCore::NO_RCS;
    if (!is) // early exit
      return rv;
    const std::istream::pos_type readPosition = is.tellg();

    // Read the first line of the file
    std::string str;
    if (simCore::getStrippedLine(is, str))
    {
      // Trim, and find the first non-whitespace token
      str.erase(str.find_last_not_of(" \n\r\t") + 1);
      const size_t strStart = str.find_first_not_of(" \n\r\t");
      const std::string typeStr = str.substr(strStart, str.find_first_of(" \n\r\t", strStart) - strStart);

      // Compare token to determine file type
      if (typeStr == "0")
        rv = simCore::RCS_LUT;
      else if (typeStr == "1")
        rv = simCore::RCS_BLOOM;
      else if (typeStr.find("%") != std::string::npos || typeStr.find("&") != std::string::npos)
        rv = simCore::RCS_SADM;
      else if (typeStr.find("#") != std::string::npos || typeStr.find("f(GHz)") != std::string::npos)
        rv = simCore::RCS_XPATCH;
    }

    // go back to the beginning of file
    is.seekg(readPosition);
    return rv;
  }

}

namespace simCore {

/* ************************************************************************ */
/* RCSTableUD Methods                                                       */
/* ************************************************************************ */

RCSTable::RCSTable()
  : freq_(0.f),
  elev_(0.f),
  polarity_(POLARITY_UNKNOWN)
{}

/* ************************************************************************ */

float RCSTable::RCS(double azim) const
{
  if (azMap_.empty())
    return static_cast<float>(SMALL_RCS_SM);

  AZIM_RCS_MAP::const_iterator iter;
  if (azMap_.size() == 1)
  {
    iter = azMap_.begin();
    return iter->second;
  }

  iter = azMap_.find(static_cast<float>(azim));
  if (iter == azMap_.end())
  {
    iter = azMap_.lower_bound(static_cast<float>(azim));

    if (iter != azMap_.end())
    {
      if (iter == azMap_.begin())
      {
        return iter->second;
      }
      else
      {
        float rcshi = iter->second;
        float azimhi = iter->first;
        --iter;
        float rcslo = iter->second;
        float azimlo = iter->first;

        // make sure we are not interpolating unnecessarily
        assert(azim > azimlo);
        assert(azim < azimhi);
        return linearInterpolate(rcslo, rcshi, azimlo, azim, azimhi);
      }
    }
    else
    {
      // item not found, use rbegin
      AZIM_RCS_MAP::const_reverse_iterator riter = azMap_.rbegin();
      return riter->second;
    }
  }

  return iter->second;
}

/* ************************************************************************ */

void RCSTable::setRCS(float azim, float rcs)
{
  azMap_[azim] = rcs;
}

/* ************************************************************************ */
/* ELEVMAP Methods                                                          */
/* ************************************************************************ */
ELEVMAP::~ELEVMAP()
{
  ELEV_RCSTABLE_MAP::iterator it = eMap.begin();
  while (it != eMap.end())
  {
    delete it->second;
    ++it;
  }
}

/* ************************************************************************ */
/* FREQMAP Methods                                                       */
/* ************************************************************************ */
FREQMAP::~FREQMAP()
{
  FREQ_ELEV_MAP::iterator it = freqMap.begin();
  while (it != freqMap.end())
  {
    delete it->second;
    ++it;
  }
}

/* ************************************************************************ */
/* RCSLUT Methods                                                       */
/* ************************************************************************ */

RCSLUT::RCSLUT()
  : RadarCrossSection(),
  description_(""),
  tableType_(RCS_LUT_TYPE),
  functionType_(RCS_MEAN_FUNC),
  modulation_(1.f),
  mean_(0.),
  median_(SMALL_DB_VAL),
  min_(std::numeric_limits<float>::max()),
  max_(-std::numeric_limits<float>::max()),
  lastFreq_(-std::numeric_limits<float>::max()),
  lastPolarity_(POLARITY_UNKNOWN)
{
  for (unsigned int i = 0; i < 2; ++i)
  {
    lastElev_[i] = std::numeric_limits<float>::max();
    loTable_[i] = nullptr;
    hiTable_[i] = nullptr;
  }
}

RCSLUT::~RCSLUT()
{
  POLARITY_FREQ_ELEV_MAP::iterator it = rcsMap_.begin();
  while (it != rcsMap_.end())
  {
    delete it->second;
    ++it;
  }
}

RCSTable *RCSLUT::getTable_(float freq, float elev, PolarityType pol, bool create)
{
  POLARITY_FREQ_ELEV_MAP::iterator pfeiter;
  FREQ_ELEV_MAP::iterator feiter;
  ELEV_RCSTABLE_MAP::iterator eiter;
  RCSTable *rcsTable = nullptr;
  FREQMAP* fm = nullptr;
  ELEVMAP* em = nullptr;

  // look for specified polarization
  pfeiter = rcsMap_.find(pol);
  if (pfeiter == rcsMap_.end())
  {
    if (create)
    {
      fm = new FREQMAP();
      rcsMap_[pol] = fm;
      em = new ELEVMAP();
      fm->freqMap[freq] = em;
      rcsTable = new RCSTable;
      rcsTable->setFreq(freq);
      rcsTable->setElev(elev);
      rcsTable->setPolarity(pol);
      em->eMap[elev] = rcsTable;
    }
    return rcsTable;
  }

  // look for specified frequency
  fm = pfeiter->second;
  feiter = fm->freqMap.find(freq);
  if (feiter == fm->freqMap.end())
  {
    if (create)
    {
      em = new ELEVMAP();
      fm->freqMap[freq] = em;
      rcsTable = new RCSTable;
      rcsTable->setFreq(freq);
      rcsTable->setElev(elev);
      rcsTable->setPolarity(pol);
      em->eMap[elev] = rcsTable;
    }
    return rcsTable;
  }

  // look for specified elevation
  em = feiter->second;
  eiter = em->eMap.find(elev);
  if (eiter == em->eMap.end())
  {
    if (create)
    {
      rcsTable = new RCSTable;
      rcsTable->setFreq(freq);
      rcsTable->setElev(elev);
      rcsTable->setPolarity(pol);
      em->eMap[elev] = rcsTable;
    }
    return rcsTable;
  }

  rcsTable = eiter->second;
  return rcsTable;
}

float RCSLUT::calcTableRCS_(float freq, double azim, double elev, PolarityType pol)
{
  float final_rcs = SMALL_RCS_SM;

  if (rcsMap_.empty())
  {
    return final_rcs;
  }

  RCSTable *rcstabLo = nullptr;
  RCSTable *rcstabHi = nullptr;
  bool foundInCache = false;
  if (simCore::areEqual(lastFreq_, freq, 0.1) && pol == lastPolarity_)
  {
    if (lastElev_[0] != std::numeric_limits<float>::max() && simCore::areAnglesEqual(elev, lastElev_[0], 1.0e-11))
    {
      rcstabLo = loTable_[0];
      rcstabHi = hiTable_[0];
      foundInCache = true;
    }
    else if (lastElev_[1] != std::numeric_limits<float>::max() && simCore::areAnglesEqual(elev, lastElev_[1], 1.0e-11))
    {
      // note, not promoting this to primary cache
      rcstabLo = loTable_[1];
      rcstabHi = hiTable_[1];
      foundInCache = true;
    }
  }

  if (!foundInCache)
  {
    // find tables in map
    POLARITY_FREQ_ELEV_MAP::const_iterator pfeiter;
    if (pol == POLARITY_UNKNOWN)
    {
      // unknown polarity, grab first one
      pfeiter = rcsMap_.begin();
    }
    else
    {
      pfeiter = rcsMap_.find(pol);
      if (pfeiter == rcsMap_.end())
      {
        return final_rcs;
      }
    }

    // look for selected frequency
    FREQMAP* fm = pfeiter->second;
    FREQ_ELEV_MAP::const_iterator feiter = fm->freqMap.find(freq);

    // choose closest
    if (feiter == fm->freqMap.end())
    {
      feiter = fm->freqMap.lower_bound(freq);
      if (feiter == fm->freqMap.end())
      {
        // grab the last one
        --feiter;
      }
      else if (feiter != fm->freqMap.begin())
      {
        float maxFreq = feiter->first;
        --feiter;
        float minFreq = feiter->first;
        if (fabs(freq-minFreq) > fabs(maxFreq-freq))
        {
          // max freq is closer to requested freq
          ++feiter;
        }
      }
    }

    // look for selected elev
    ELEVMAP* emap = feiter->second;
    ELEV_RCSTABLE_MAP::const_iterator eiter;
    if (emap->eMap.size() == 1)
    {
      eiter = emap->eMap.begin();
      rcstabLo = eiter->second;
      rcstabHi = rcstabLo;
    }
    else
    {
      // using find here assumes that requests will not be made for elev values close to but not equal to key values, e.g. elev=30.00001
      // such values will be interpolated; a comparison that uses simCore::areAnglesEqual could be better in such cases.
      eiter = emap->eMap.find(static_cast<float>(elev));
      if (eiter != emap->eMap.end())
      {
        // exact match found
        rcstabLo = eiter->second;
        rcstabHi = rcstabLo;
      }
      else
      {
        eiter = emap->eMap.lower_bound(static_cast<float>(elev));
        if (eiter == emap->eMap.end())
        {
          // after last table, use rbegin
          ELEV_RCSTABLE_MAP::const_reverse_iterator riter = emap->eMap.rbegin();
          rcstabLo = riter->second;
          rcstabHi = rcstabLo;
        }
        else if (eiter == emap->eMap.begin())
        {
          // before first table
          rcstabLo = eiter->second;
          rcstabHi = rcstabLo;
        }
        else
        {
          // in between two tables, need to interpolate
          rcstabHi = eiter->second;
          --eiter;
          rcstabLo = eiter->second;
        }
      }
    }

    // cache two elev/table combinations, to support calling pattern used by 3D rcs calculation
    if (simCore::areEqual(lastFreq_, freq, 0.1) && pol == lastPolarity_)
    {
      // cache backup elev/table cache
      loTable_[1] = loTable_[0];
      hiTable_[1] = hiTable_[0];
      lastElev_[1] = lastElev_[0];
    }
    else
    {
      // freq or polarization changed, cached elev/table values are no longer valid
      loTable_[1] = nullptr;
      hiTable_[1] = nullptr;
      lastElev_[1] = std::numeric_limits<float>::max();
    }

    // update cached primary freq/polarity/elev/table combination
    loTable_[0] = rcstabLo;
    hiTable_[0] = rcstabHi;
    lastFreq_ = freq;
    lastElev_[0] = static_cast<float>(elev);
    lastPolarity_ = pol;
  }

  // either both tables are NULL, or both are non-NULL
  assert((rcstabLo == nullptr && rcstabHi == nullptr) || (rcstabLo && rcstabHi));

  if (rcstabLo == nullptr && rcstabHi == nullptr)
  {
    // we didn't find any matching tables
    final_rcs = dB2Linear(mean_);
  }
  else if (rcstabLo == rcstabHi)
  {
    // only found one table
    final_rcs = rcstabLo->RCS(azim);
  }
  else if (rcstabLo && rcstabHi)
  {
    // we need to interpolate
    // we are pretty sure that the two tables cannot have different freqs
    assert(rcstabLo->freq() == rcstabHi->freq());
    // we are pretty sure that the elev does not match table values
    assert(elev > rcstabLo->elev());
    assert(elev < rcstabHi->elev());
    final_rcs = linearInterpolate(rcstabLo->RCS(azim), rcstabHi->RCS(azim), rcstabLo->elev(), elev, rcstabHi->elev());
  }
  // these cases should not occur, assert is above
  else if (rcstabLo)
    final_rcs = rcstabLo->RCS(azim);
  else if (rcstabHi)
    final_rcs = rcstabHi->RCS(azim);

  return final_rcs;
}

float RCSLUT::RCSdB(float freq, double azim, double elev, PolarityType pol)
{
  return linear2dB(RCSsm(freq, azim, elev, pol));
}

float RCSLUT::RCSsm(float freq, double azim, double elev, PolarityType pol)
{
  // convert incoming azimuth & elevation to correct units & limits
  azim = angFix2PI(azim);
  elev = angFixPI(elev);

  switch (tableType_)
  {
  case RCS_LUT_TYPE:
    // strictly a lookup table, return mean value
    return calcTableRCS_(freq, azim, elev, pol);
  
  case RCS_SYM_LUT_TYPE:
    // symmetrical lookup table, return mean value
    return calcTableRCS_(freq, fabs(angFixPI(azim)), elev, pol);
  
  case RCS_DISTRIBUTION_FUNC_TYPE:
  default: // UTILS::eRCS_DISTRIBUTION_FUNC_TYPE
    {
      const double rcs = calcTableRCS_(freq, azim, elev, pol);
      // apply distribution to mean rcs value
      switch (functionType_)
      {
      case RCS_MEAN_FUNC:
      default: // RCS_MEAN_FUNC
        // apply scintillation to mean value
        return static_cast<float>(rcs + modulation_);
      case RCS_GAUSSIAN_FUNC:
        // apply Gaussian distribution to rcs value
        return static_cast<float>(rcs + (modulation_ * gaussian_()));
      case RCS_RAYLEIGH_FUNC:
        {
          // apply Rayleigh distribution to rcs value
          // sqrt (sum of the squares of two gaussians)
          double x = gaussian_();
          double y = gaussian_();
          return static_cast<float>(rcs + (modulation_ * (sqrt(square(x) + square(y)))));
        }
      case RCS_LOG_NORMAL_FUNC:
        {
          // apply log normal distribution to rcs value
          // (log of Rayleigh)
          double x = gaussian_();
          double y = gaussian_();
          return static_cast<float>(rcs + (modulation_ * (log10(sqrt(square(x) + square(y))))));
        }
      }
    }
  }
  return SMALL_RCS_SM;
}

int RCSLUT::loadXPATCHRCSFile_(std::istream &inFile)
{
  int rv = 1;
  RCSTable *rcsTable = nullptr;
  std::vector<float> medianVec;
  float freq = 0;
  float elev = 0;
  float azim = 0;
  float rcsVV = 0;
  float rcsHV = 0;
  float rcsVH = 0;
  float rcsHH = 0;
  double rcsVal;
  std::vector<std::string> vec;

  while (getTokens(inFile, vec))
  {
    size_t vecLen = vec.size();
    // FR(GHz)  inc-EL  inc-AZ  VV  HV  VH  HH
    if (vec[0] != "#" && vecLen == 7 && vec[3].find("VV") == std::string::npos)
    {
      // found valid data, mark return value
      if (rv == 1)
      {
        // reset previous look up table data
        reset_();
        description_ = "XPATCH RCS";
      }
      rv = 0;

      if (!isValidNumber(vec[0], freq))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH frequency" << std::endl;
        return 1;
      }
      // convert GHz to MHz
      freq *= 1000.f;
      if (!isValidNumber(vec[1], elev))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH elevation" << std::endl;
        return 1;
      }
      elev = static_cast<float>(DEG2RAD*elev);
      if (!isValidNumber(vec[2], azim))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH azimuth" << std::endl;
        return 1;
      }
      azim = static_cast<float>(DEG2RAD*azim);
      if (!isValidNumber(vec[3], rcsVal))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH VV value" << std::endl;
        return 1;
      }
      // use double for dB2Linear to avoid precision truncation
      rcsVV = static_cast<float>(dB2Linear(rcsVal));
      if (!isValidNumber(vec[4], rcsVal))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH HV value" << std::endl;
        return 1;
      }
      // use double for dB2Linear to avoid precision truncation
      rcsHV = static_cast<float>(dB2Linear(rcsVal));
      if (!isValidNumber(vec[5], rcsVal))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH VH value" << std::endl;
        return 1;
      }
      // use double for dB2Linear to avoid precision truncation
      rcsVH = static_cast<float>(dB2Linear(rcsVal));
      if (!isValidNumber(vec[6], rcsVal))
      {
        SIM_ERROR << "Encountered invalid number for XPATCH HH value" << std::endl;
        return 1;
      }
      // use double for dB2Linear to avoid precision truncation
      rcsHH = static_cast<float>(dB2Linear(rcsVal));

      // calculate median, min, max and mean rcs values
      medianVec.push_back(rcsVV);
      min_ = sdkMin(min_, rcsVV);
      max_ = sdkMax(max_, rcsVV);
      mean_ += rcsVV;

      // add data to LUT
      rcsTable = getTable_(freq, elev, POLARITY_HORIZONTAL, true);
      rcsTable->setRCS(azim, rcsHH);
      rcsTable = getTable_(freq, elev, POLARITY_VERTICAL, true);
      rcsTable->setRCS(azim, rcsVV);
      rcsTable = getTable_(freq, elev, POLARITY_HORZVERT, true);
      rcsTable->setRCS(azim, rcsHV);
      rcsTable = getTable_(freq, elev, POLARITY_VERTHORZ, true);
      rcsTable->setRCS(azim, rcsVH);
    } // end of if vec
  } // end of while

  if (rv)
  {
    return rv;
  }

  computeStatistics_(&medianVec);

  // So far, so good...
  return 0;
}

int RCSLUT::loadRcsLutFile_(std::istream &inFile)
{
  int numTables = 0;
  int tableType;
  size_t lineNumber = 0;

  // Table Type
  std::string st;
  if (!getFirstToken(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing table type for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  if (!isValidNumber(st, tableType))
  {
    SIM_ERROR << "Encountered invalid number for RCS LUT table type on line: " << lineNumber << std::endl;
    return 1;
  }

  if (tableType != 0)
  {
    SIM_ERROR << "Unsupported RCS File Format\n" << std::endl;
    return 1;
  }

  // reset previous look up table data
  reset_();

  // rcs pattern title
  if (!getTokenLine(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing pattern title for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  description_ = st;

  // table type
  lineNumber++;
  if (!getFirstToken(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing table type for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  int intTabType = 0;
  if (!isValidNumber(st, intTabType))
  {
    SIM_ERROR << "Encountered invalid number for RCS LUT table type on line: " << lineNumber << std::endl;
    return 1;
  }
  if (intTabType < 0 || intTabType > 2)
  {
    SIM_ERROR << "Incorrect table type found for RCS LUT on line: " << lineNumber << " <" << st << ">" << std::endl;
    return 1;
  }
  tableType_ = static_cast<RCSTableType>(intTabType);

  // distribution function type
  if (!getFirstToken(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing distribution function for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  int intFuncType = 0;
  if (!isValidNumber(st, intFuncType))
  {
    SIM_ERROR << "Encountered invalid number for RCS LUT distribution function on line: " << lineNumber << std::endl;
    return 1;
  }
  if (intFuncType < 0 || intFuncType > 3)
  {
    SIM_ERROR << "Incorrect distribution function found for RCS LUT on line: " << lineNumber << " <" << st << ">" << std::endl;
    return 1;
  }
  functionType_ = static_cast<RCSFuncType>(intFuncType);

  // scintillation modulation
  if (!getFirstToken(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing scintillation modulation for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  if (!isValidNumber(st, modulation_))
  {
    SIM_ERROR << "Encountered invalid number for RCS LUT modulation on line: " << lineNumber << std::endl;
    return 1;
  }
  if (modulation_ < 0)
  {
    SIM_ERROR << "Modulation value found for RCS LUT on line: " << lineNumber << " <" << st << ">, must be >= 0" << std::endl;
    return 1;
  }

  // # tables
  if (!getFirstToken(inFile, st, &lineNumber))
  {
    SIM_ERROR << "Error processing # tables for RCS LUT on line: " << lineNumber << std::endl;
    return 1;
  }
  if (!isValidNumber(st, numTables))
  {
    SIM_ERROR << "Encountered invalid number for RCS LUT number of tables on line: " << lineNumber << std::endl;
    return 1;
  }

  PolarityType pol=POLARITY_UNKNOWN;
  float azim = 0;
  float rcs = 0;
  double val = 0;
  std::vector<float> medianVec;

  // read in remainder of data
  for (int i = 0; i < numTables; i++)
  {
    // Frequency of rcs
    if (!getFirstToken(inFile, st, &lineNumber))
    {
      SIM_ERROR << "Error processing RCS freq for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
      return 1;
    }
    float freq = 0.f;
    if (!isValidNumber(st, freq))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT frequency on line: " << lineNumber << std::endl;
      return 1;
    }
    if (freq <= 0)
    {
      SIM_ERROR << "Incorrect frequency value found for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }

    // Elevation of rcs
    if (!getFirstToken(inFile, st, &lineNumber))
    {
      SIM_ERROR << "Error processing elev for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
      return 1;
    }
    float elev = 0.f;
    if (!isValidNumber(st, elev))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT elevation on line: " << lineNumber << std::endl;
      return 1;
    }
    elev = static_cast<float>(DEG2RAD*elev);

    // polarization of pattern
    if (!getFirstToken(inFile, st, &lineNumber))
    {
      SIM_ERROR << "Error processing polarity for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
      return 1;
    }
    int intpol = 0;
    if (!isValidNumber(st, intpol))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT polarity on line: " << lineNumber << std::endl;
      return 1;
    }
    if (intpol < 0 || intpol > 8)
    {
      SIM_ERROR << "Incorrect polarity found for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }
    pol = static_cast<PolarityType>(intpol);

    // # of (azim,rcs) pairs in table
    if (!getFirstToken(inFile, st, &lineNumber))
    {
      SIM_ERROR << "Error processing (azim, rcs) table size for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
      return 1;
    }
    int tableSize = 0;
    if (!isValidNumber(st, tableSize))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT table size on line: " << lineNumber << std::endl;
      return 1;
    }
    if (tableSize <= 0)
    {
      SIM_ERROR << "Incorrect table size found for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }

    // units for angle & rcs
    if (!getTokenLine(inFile, st, &lineNumber))
    {
      SIM_ERROR << "Error processing angle and RCS units for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
      return 1;
    }
    std::vector<std::string> unitVec;
    stringTokenizer(unitVec, st);
    if (unitVec.size() < 2)
    {
      SIM_ERROR << "Incorrect # tokens (>=2) " << unitVec.size() << " found with angle and RCS units for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }
    short angleUnits = 0;
    if (!isValidNumber(unitVec[0], angleUnits))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT angle unit on line: " << lineNumber << std::endl;
      return 1;
    }
    if (angleUnits < 0 || angleUnits > 1)
    {
      SIM_ERROR << "Incorrect angle unit found for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }
    short rcsUnits = 0;
    if (!isValidNumber(unitVec[1], rcsUnits))
    {
      SIM_ERROR << "Encountered invalid number for RCS LUT rcs unit on line: " << lineNumber << std::endl;
      return 1;
    }
    if (rcsUnits < 0 || rcsUnits > 1)
    {
      SIM_ERROR << "Incorrect RCS unit found for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
      return 1;
    }

    RCSTable *rcsTable = getTable_(freq, elev, pol, true);

    // read in remainder of data
    for (int ii = 0; ii < tableSize; ii++)
    {
      if (!getTokenLine(inFile, st, &lineNumber))
      {
        SIM_ERROR << "Error processing (azim, rcs) pair " << ii+1 << " for RCS LUT # " << i+1 << " on line: " << lineNumber << std::endl;
        return 1;
      }
      std::vector<std::string> dataVec;
      stringTokenizer(dataVec, st);
      if (dataVec.size() < 2)
      {
        SIM_ERROR << "Incorrect # tokens (>=2) " << dataVec.size() << " found with (azim, rcs) pair " << ii+1 << " for RCS LUT # " << i+1 << " on line: " << lineNumber << " <" << st << ">" << std::endl;
        return 1;
      }
      if (!isValidNumber(dataVec[0], azim))
      {
        SIM_ERROR << "Encountered invalid number for RCS LUT azimuth on line: " << lineNumber << std::endl;
        return 1;
      }
      if (!isValidNumber(dataVec[1], val))
      {
        SIM_ERROR << "Encountered invalid number for RCS LUT value on line: " << lineNumber << std::endl;
        return 1;
      }

      // verify angle units, force to rad
      // we force all rcs units to be in m^2
      // use double for dB2Linear to avoid precision truncation
      rcs = static_cast<float>((rcsUnits == 1) ?  dB2Linear(val) : val);
      rcsTable->setRCS((angleUnits == 0) ? static_cast<float>(DEG2RAD*(azim)) : azim, rcs);

      // calc statistics on data
      min_ = sdkMin(min_, rcs);
      max_ = sdkMax(max_, rcs);
      mean_ += rcs;
      medianVec.push_back(rcs);
    }
  } // end of for numTables

  computeStatistics_(&medianVec);

  // So far, so good...
  return 0;
}

int RCSLUT::loadSADMRCSFile_(std::istream &inFile)
{
  std::string str;
  // remove SADM comments
  do
  {
    getStrippedLine(inFile, str);
  } while (str.find("%") != std::string::npos);

  // Verify correct SADM file
  if (str.find("&RCS") == std::string::npos)
  {
    SIM_ERROR << "Error processing SADM RCS file, did not find \"&RCS\" after comment section." << std::endl;
    return 1;
  }

  float freq = 0.f;
  PolarityType pol=POLARITY_UNKNOWN;
  std::vector<float> medianVec;

  // The SADM documentation explicitly states that:
  //
  // % There are THREE forms of this file:
  // %  1.  Version 1 contains total RCS vs azimuth and elevation
  // %      at each frequency and polarization.  This file version
  // %      applies if the flag RCS_IQ is false or missing.
  // %  2.  Version 2 contains RCS at high sampling resolution at
  // %      each azimuth for a specified frequency and polarization.
  // %      This file version applies if the flag RCS_IQ_FLAG is true.
  // %  3.  Version 3 contains multiple RCS scatterers plus their
  // %      locations on the ship.  This file version applies if the
  // %      flag RCS_MULTI_SCATTERERS is true.
  // %
  // %**********************************************************

  std::vector<std::string> vec;
  if (!getTokens(inFile, vec))
  {
    SIM_ERROR << "Error processing SADM RCS file" << std::endl;
    return 1;
  }
  // Verify correct version of SADM file
  // We only process version 1 of the file
  if (vec.size() > 2 && stringCaseFind(vec[0], "RCS_IQ") != std::string::npos)
  {
    if (upperCase(vec[2]) == "T")
    {
      SIM_ERROR << "SADM version 2 RCS file is not supported" << std::endl;
      return 1;
    }
  }

  if (vec.size() > 2 && stringCaseFind(vec[0], "RCS_MULTI_SCATTERERS") != std::string::npos)
  {
    if (upperCase(vec[2]) == "T")
    {
      SIM_ERROR << "SADM version 3 RCS file is not supported" << std::endl;
      return 1;
    }
  }

  if (!vec.empty() && simCore::caseCompare(vec[0], "RCS_FREQ_INTERPOLATE") != 0)
  {
    if (!getTokens(inFile, vec))
    {
      SIM_ERROR << "Could not find RCS_FREQ_INTERPOLATE" << std::endl;
      return 1;
    }
  }

  // check for existing RCS, then reset
  reset_();
  description_ = "SADM RCS";

  // verify next token is RCS_FREQ_INTERPOLATE
  if (checkTokens_(vec.size(), 3, "RCS_FREQ_INTERPOLATE")) return 1;

  // % The initial parameters that go into Version 1 of the file are:
  // %
  // % RCS_IQ = F
  // % RCS_FREQ_INTERPOLATE = T/F
  // % RCS_N_AZ =
  // % RCS_N_EL =
  // % RCS_N_FREQ =
  // % RCS_EL =  el1  el2  ...
  // %
  // % They are followed by the following values, repeated for
  // % each frequency / polarization combination.
  // %
  // % RCS_FREQ = freq1
  // % RCS_POL = H
  // % RCS_ON_AXIS = on_axis_rcs @ freq1 & H pol
  // % RCS_TABLE =
  // %   ...
  // %   ...
  // % RCS_FREQ = freq1
  // % RCS_POL = V
  // % RCS_ON_AXIS = on_axis_rcs @ freq1 & V pol
  // % RCS_TABLE =
  // %   ...
  // %   ...
  // % RCS_FREQ = freq2
  // % RCS_POL = H
  // % RCS_ON_AXIS = on_axis_rcs @ freq2 & H pol
  // % RCS_TABLE =
  // %   ...
  // % RCS_FREQ = freq2
  // % RCS_POL = V
  // % RCS_ON_AXIS = on_axis_rcs @ freq2 & V pol
  // % RCS_TABLE =
  // %   ...
  // %   ...
  // % etc
  // % /            (terminate with a slash)
  // %
  // %
  // % RCS files can have up to 361 az values, and up to 16 el
  // % values, up to 16 frequencies, and must have both
  // % horizontal and vertical polarizations. The frequency
  // % values must be monotonically increasing, but the
  // % polarization values can occur in either order.

  // Unfortunately, that is not the case, depending on type of object,
  // the next value could be either RCS_N_AZ or RCS_N_FREQ

  int numFreq = 0;
  std::vector<std::string> elevVec;
  RCSTable *rcsTable = 0;
  int i, j;
  if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_N_AZ or RCS_N_FREQ")) return 1;

  if (simCore::caseCompare(vec[0], "RCS_N_FREQ") == 0)
  {
    // Must be a "special case" chaff/decoy RCS file
    // read RCS_N_FREQ
    if (checkTokens_(vec.size(), 3, "RCS_N_FREQ")) return 1;
    if (!isValidNumber(vec[2], numFreq))
    {
      SIM_ERROR << "Encountered invalid number for RCS_N_FREQ" << std::endl;
      return 1;
    }

    for (i = 0; i < numFreq; i++)
    {
      // read RCS_FREQ
      if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_N_AZ or RCS_FREQ")) return 1;
      if (!isValidNumber(vec[2], freq))
      {
        SIM_ERROR << "Encountered invalid number for RCS_FREQ" << std::endl;
        return 1;
      }
      // convert freq from GHz to MHz
      freq *= 1000.f;

      for (j = 0; j<2; j++)
      {
        // read RCS_POL
        if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_N_AZ or RCS_POL")) return 1;
        if (simCore::caseCompare(vec[2], "H") == 0)
          pol = POLARITY_HORIZONTAL;
        else if (simCore::caseCompare(vec[2], "V") == 0)
          pol = POLARITY_VERTICAL;
        else if (simCore::caseCompare(vec[2], "C") == 0)
          pol = POLARITY_CIRCULAR;
        else
          pol = POLARITY_UNKNOWN;

        // read RCS_ON_AXIS
        if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_ON_AXIS")) return 1;
        double onAxisVal = 0;
        if (!isValidNumber(vec[2], onAxisVal))
        {
          SIM_ERROR << "Encountered invalid number for RCS_ON_AXIS" << std::endl;
          return 1;
        }
        // convert dB to sqm
        // use double for dB2Linear to avoid precision truncation
        float onAxisRCS = static_cast<float>(dB2Linear(onAxisVal));

        rcsTable = getTable_(freq, 0, pol, true);
        // Azimuth, For chaff clouds use the on-axis RCS values only
        rcsTable->setRCS(0, onAxisRCS);

        // calculate median, min, max and mean rcs values
        medianVec.push_back(onAxisRCS);
        min_ = sdkMin(min_, onAxisRCS);
        max_ = sdkMax(max_, onAxisRCS);
        mean_ += onAxisRCS;
      }
    }
  }
  else if (simCore::caseCompare(vec[0], "RCS_N_AZ") == 0)
  {
    if (checkTokens_(vec.size(), 3, "RCS_N_AZ")) return 1;
    // read RCS_N_AZ
    int numAz = 0;
    if (!isValidNumber(vec[2], numAz))
    {
      SIM_ERROR << "Encountered invalid number for RCS_N_AZ" << std::endl;
      return 1;
    }

    // read RCS_N_EL
    if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_N_EL")) return 1;
    int numEl = 0;
    if (!isValidNumber(vec[2], numEl))
    {
      SIM_ERROR << "Encountered invalid number for RCS_N_EL" << std::endl;
      return 1;
    }

    // read RCS_N_FREQ
    if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_N_FREQ")) return 1;
    if (!isValidNumber(vec[2], numFreq))
    {
      SIM_ERROR << "Encountered invalid number for RCS_N_FREQ" << std::endl;
      return 1;
    }

    // read RCS_EL
    if (!getTokens(inFile, elevVec, 1))
    {
      SIM_ERROR << "Could not process RCS_EL line" << std::endl;
      reset_();
      return 1;
    }

    for (i = 0; i < 2 * numFreq; i++)
    {
      std::vector< std::vector<std::string> > azList;
      std::vector< std::vector<std::string> >::iterator azListIter;

      // read RCS_FREQ
      if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_FREQ or missing polarization")) return 1;
      if (!isValidNumber(vec[2], freq))
      {
        SIM_ERROR << "Encountered invalid number for RCS_FREQ" << std::endl;
        return 1;
      }
      // convert freq from GHz to MHz
      freq *= 1000.f;

      // read RCS_POL
      if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_POL")) return 1;
      if (simCore::caseCompare(vec[2], "H") == 0)
        pol = POLARITY_HORIZONTAL;
      else if (simCore::caseCompare(vec[2], "V") == 0)
        pol = POLARITY_VERTICAL;
      else if (simCore::caseCompare(vec[2], "C") == 0)
        pol = POLARITY_CIRCULAR;
      else
        pol = POLARITY_UNKNOWN;

      // read RCS_ON_AXIS
      if (!getTokens(inFile, vec, 3) || checkTokens_(vec.size(), 3, "RCS_ON_AXIS")) return 1;

      // read RCS_TABLE line
      if (!getTokens(inFile, vec, 2) || checkTokens_(vec.size(), 2, "RCS_TABLE")) return 1;

      // read entire table in, then process
      for (j = 0; j<numAz; j++)
      {
        std::vector<std::string> rcsVec;
        if (!getTokens(inFile, rcsVec, numEl+1) || checkTokens_(rcsVec.size(), numEl+1, "RCS Data")) return 1;
        azList.push_back(rcsVec);
      }

      for (j = 0; j<numEl; j++)
      {
        // Elevation of rcs
        // convert to radians
        float elev = 0.f;
        if (!isValidNumber(elevVec[j+2], elev))
        {
          SIM_ERROR << "Encountered invalid number for RCS elevation" << std::endl;
          return 1;
        }
        rcsTable = getTable_(freq, static_cast<float>(DEG2RAD*elev), pol, true);

        // read in remainder of data
        for (azListIter=azList.begin(); azListIter!=azList.end(); ++azListIter)
        {
          // verify angle units, force to rad
          // we force all rcs units to be in m^2
          float azim = 0.f;
          double rcsVal = 0;
          if (!isValidNumber((*azListIter)[0], azim))
          {
            SIM_ERROR << "Encountered invalid number for RCS azimuth" << std::endl;
            return 1;
          }
          if (!isValidNumber((*azListIter)[j+1], rcsVal))
          {
            SIM_ERROR << "Encountered invalid number for RCS value" << std::endl;
            return 1;
          }
          // use double for dB2Linear to avoid precision truncation
          float rcs = static_cast<float>(dB2Linear(rcsVal));
          rcsTable->setRCS(static_cast<float>(DEG2RAD*azim), rcs);

          // calculate median, min, max and mean rcs values
          medianVec.push_back(rcs);
          min_ = sdkMin(min_, rcs);
          max_ = sdkMax(max_, rcs);
          mean_ += rcs;
        }
      }
    }
  }
  else if (str.empty())
  {
    SIM_ERROR << "Error loading RCS file: empty string detected" << std::endl;
    reset_();
    return 1;
  }

  computeStatistics_(&medianVec);

  return 0;
}

void RCSLUT::computeStatistics_(std::vector<float>* medianVec)
{
  if (!medianVec)
  {
    // the argument cannot be nullptr
    assert(0);
    return;
  }
  if (medianVec->empty())
    return; // Avoid divide by zero below

  // convert sq m to dBsm
  min_ = static_cast<float>(linear2dB(min_));
  max_ = static_cast<float>(linear2dB(max_));
  mean_ = static_cast<float>(linear2dB(mean_ / medianVec->size()));

  std::sort(medianVec->begin(), medianVec->end());
  float median = 0.f;
  if (medianVec->size() == 1)
  {
    median = (*medianVec)[0];
  }
  else if (odd(static_cast<int>(medianVec->size())))
  {
    median = (*medianVec)[medianVec->size()/2];
  }
  else
  {
    const size_t midpoint = medianVec->size()/2;
    median = static_cast<float>(((*medianVec)[midpoint] + (*medianVec)[midpoint-1]) * .5);
  }
  median_ = static_cast<float>(linear2dB(median));
}

int RCSLUT::checkTokens_(size_t val, size_t min, const std::string& param)
{
  if (val < min)
  {
    SIM_ERROR << "Error loading RCS file:  check " << param << std::endl;
    SIM_ERROR << "  Expected " << min << ", detected " << val << " tokens" << std::endl;
    reset_();
    return 1;
  }
  return 0;
}

void RCSLUT::reset_()
{
  POLARITY_FREQ_ELEV_MAP::const_iterator it = rcsMap_.begin();
  while (it != rcsMap_.end())
  {
    delete it->second;
    ++it;
  }
  rcsMap_.clear();
  tableType_ = RCS_LUT_TYPE;
  functionType_ = RCS_MEAN_FUNC;
  modulation_ = 1.f;
  lastFreq_ = -std::numeric_limits<float>::max();
  lastPolarity_ = POLARITY_UNKNOWN;
  for (unsigned int i = 0; i < 2; ++i)
  {
    lastElev_[i] = std::numeric_limits<float>::max();
    loTable_[i] = nullptr;
    hiTable_[i] = nullptr;
  }
  mean_ = 0.;
  median_ = SMALL_DB_VAL;
  min_ = std::numeric_limits<float>::max();
  max_ = -std::numeric_limits<float>::max();
}

int RCSLUT::loadRCSFile(const std::string& fname)
{
  int st = 1;

  if (fname.empty())
  {
    SIM_ERROR << "Invalid filename" << std::endl;
    return st;
  }

  // find file
  std::fstream inFile;
  inFile.open(simCore::streamFixUtf8(fname), std::ios::in);
  if (inFile.is_open())
  {
    setFilename(fname);
    SIM_INFO << "Loading RCS File: " << simCore::toNativeSeparators(fname) << std::endl;
    st = loadRCSFile(inFile);
    inFile.close();
  }
  else
  {
    SIM_ERROR << "Could not open RCS file: " << simCore::toNativeSeparators(fname) << std::endl;
    return st;
  }

  return st;
}

int RCSLUT::loadRCSFile(std::istream& istream)
{
  RCSType rcsType = getRCSType(istream);
  switch (rcsType)
  {
  case RCS_LUT:
    return loadRcsLutFile_(istream);
  case RCS_XPATCH:
    return loadXPATCHRCSFile_(istream);
  case RCS_SADM:
    return loadSADMRCSFile_(istream);
  case NO_RCS:
  case RCS_BLOOM:
  case RCS_RTS:
    // Not handled
    break;
  }
  return 1;
}


/* ************************************************************************ */
/* RcsFileParser Methods                                                    */
/* ************************************************************************ */

RadarCrossSection* RcsFileParser::loadRCSFile(const std::string& fname)
{
  if (fname.empty())
    return nullptr;

  RCSLUT* rcsdata = new RCSLUT();
  if (rcsdata->loadRCSFile(fname) != 0)
  {
    delete rcsdata;
    rcsdata = nullptr;
  }

  return rcsdata;
}

}
