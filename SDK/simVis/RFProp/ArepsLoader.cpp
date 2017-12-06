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
#include <iostream>
#include "osgDB/FileUtils"
#include "simCore/LUT/LUT2.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Format.h"
#include "simCore/String/Constants.h"
#include "simCore/String/Utils.h"
#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/LocatorNode.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/LUTProfileDataProvider.h"
#include "simVis/RFProp/LUT1ProfileDataProvider.h"
#include "simVis/RFProp/PODProfileDataProvider.h"
#include "simVis/RFProp/OneWayPowerDataProvider.h"
#include "simVis/RFProp/TwoWayPowerDataProvider.h"
#include "simVis/RFProp/SNRDataProvider.h"
#include "simVis/RFProp/ArepsLoader.h"

namespace simRF {

ArepsLoader::ArepsLoader(RFPropagationFacade* beamHandler)
  : maxHeight_(0.0),
  minHeight_(0.0),
  numRanges_(0),
  numHeights_(0),
  maxRange_(0.0),
  minRange_(0.0),
  antennaHgt_(0.0),
  beamHandler_(beamHandler)
{
}

ArepsLoader::~ArepsLoader()
{
}

double ArepsLoader::getAntennaHeight() const
{
  return antennaHgt_;
}

int ArepsLoader::loadFile(const std::string& arepsFile, simRF::Profile& profile, bool firstFile)
{
  // create stream to input file
  std::ifstream inFile(arepsFile.c_str());
  if (!inFile)
  {
    SIM_ERROR << "Could not open AREPS file: " << simCore::toNativeSeparators(arepsFile) << " for reading" << std::endl;
    return 1;
  }
  else
  {
    SIM_INFO << "Loading AREPS file: " << simCore::toNativeSeparators(arepsFile) << std::endl;
  }

  // Older versions of AREPS file had bearing embedding in filename
  double bearingAngleRad = getBearingAngle_(arepsFile);
  RadarParameters radarParameters;
  std::string st;
  while (simCore::getStrippedLine(inFile, st))
  {
    // clears vec every time through loop, otherwise, accumulate
    // tokens in vec from previous lines
    std::vector<std::string> tmpvec;
    // tokenize while removing quotes - some files may have various values quoted
    simCore::stringTokenizer(tmpvec, simCore::StringUtils::substitute(st, "\"", ""));

    size_t vecLen = tmpvec.size();
    if (vecLen > 0 && tmpvec[0] != "#")
    {
      // some values are only read/processed for the first file (of a multi-file set)
      if (firstFile)
      {
        if ((tmpvec[0] == "AntGain") && vecLen >= 3)
        {
          //# Antenna gain in dB
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.antennaGaindB))
          {
            SIM_ERROR << "Could not determine antenna gain for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "AntHt") && vecLen >= 3)
        {
          //# Antenna ht(m) above ground
          if (!simCore::isValidNumber(tmpvec[2], antennaHgt_))
          {
            SIM_ERROR << "Could not determine antenna height for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Freq") && vecLen >= 3)
        {
          //# Frequency(MHz)
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.freqMHz))
          {
            SIM_ERROR << "Could not determine freq for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Noise") && vecLen >= 3)
        {
          //# Noise figure
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.noiseFiguredB))
          {
            SIM_ERROR << "Could not determine noiseFigure for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "PulseWidth") && vecLen >= 3)
        {
          //#  Pulse width or length in usec
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.pulseWidth_uSec))
          {
            SIM_ERROR << "Could not determine pulseWidth for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "SysLoss") && vecLen >= 3)
        {
          //# System losses in dB
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.systemLossdB))
          {
            SIM_ERROR << "Could not determine system loss for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "TransPower") && vecLen >= 3)
        {
          //# Transmitter power in KW
          if (!simCore::isValidNumber(tmpvec[2], radarParameters.xmtPowerKW))
          {
            SIM_ERROR << "Could not determine xmtPower for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Hmax") && vecLen >= 3)
        {
          //# Maximum height Meters
          if (!simCore::isValidNumber(tmpvec[2], maxHeight_))
          {
            SIM_ERROR << "Could not determine max height for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Hmin") && vecLen >= 3)
        {
          //# Minimum height Meters
          if (!simCore::isValidNumber(tmpvec[2], minHeight_))
          {
            SIM_ERROR << "Could not determine min height for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Nrout") && vecLen >= 3)
        {
          //# Number of range steps to output
          if (!simCore::isValidNumber(tmpvec[2], numRanges_))
          {
            SIM_ERROR << "Could not determine number of ranges for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if ((tmpvec[0] == "Nzout") && vecLen >= 3)
        {
          //# Number of height points to output
          if (!simCore::isValidNumber(tmpvec[2], numHeights_))
          {
            SIM_ERROR << "Could not determine number of heights for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
          // add 1 due to incorrect value specified by AREPS
          ++numHeights_;
        }
        else if ((tmpvec[0] == "Rmax") && vecLen >= 3)
        {
          //# Maximum range in meters
          if (!simCore::isValidNumber(tmpvec[2], maxRange_))
          {
            SIM_ERROR << "Could not determine max range for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
        else if (beamHandler_ && (st == "[Probability of detection]"))
        {
          // skip comment
          simCore::getStrippedLine(inFile, st);
          // Thresholds in DB for a probability of detection from 1% to 100%, 10 lines of 10 values
          // expected to be positive, in decreasing order;
          // they are sign-inverted by setPODLossThreshold, producing a vector of negative thresholds, in increasing order.
          std::vector<float> podVector;
          podVector.reserve(PODProfileDataProvider::POD_VECTOR_SIZE);
          for (size_t i = 0; i < 10; i++)
          {
            simCore::getStrippedLine(inFile, st);

            std::vector<std::string> pdVec;
            //remove quotes
            simCore::stringTokenizer(pdVec, simCore::StringUtils::substitute(st, "\"", ""));
            if (pdVec.size() != 10)
            {
              SIM_ERROR << "Bad formatting of POD data for AREPS file: " << arepsFile << std::endl;
              return 1;
            }

            for (size_t j = 0; j < 10; j++)
            {
              float pdVal = 0.0;
              if (!simCore::isValidNumber(pdVec[j], pdVal) || pdVal < 0)
              {
                // if assert fails, the AREPS file contains negative POD thresholds.
                assert(pdVal >= 0);
                SIM_ERROR << "Invalid data in POD data for AREPS file: " << arepsFile << std::endl;
                return 1;
              }
              podVector.push_back(pdVal);
            }
          }
          if (podVector.size() != PODProfileDataProvider::POD_VECTOR_SIZE)
          {
            SIM_ERROR << "Invalid POD data for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
          if (0 != beamHandler_->setPODLossThreshold(podVector))
          {
            SIM_ERROR << "Error saving POD data for AREPS file: " << arepsFile << std::endl;
            return 1;
          }
        }
      }

      // the following entries are processed for every file in a fileset

      if ((tmpvec[0] == "Bearing") && vecLen >= 3)
      {
        // Newer versions of AREPS file have bearing in the file itself
        std::vector<std::string> bearVec;
        double bearingAngleDeg;
        // tokenize on degree symbol
        simCore::stringTokenizer(bearVec, tmpvec[3], simCore::STR_DEGREE_SYMBOL_ASCII);
        if (simCore::isValidNumber(bearVec[0], bearingAngleDeg))
        {
          // convert degrees to radians
          bearingAngleRad = simCore::angFix2PI(bearingAngleDeg * simCore::DEG2RAD);
        }
        else
        {
          SIM_ERROR << "Could not determine bearing for AREPS file: " << arepsFile << std::endl;
          return 1;
        }
      }
      else if ((tmpvec[0] == "HorBw" || tmpvec[0] == "HorzBwidth") && vecLen >= 3)
      {
        //# Horizontal beam width in deg
        if (!simCore::isValidNumber(tmpvec[2], radarParameters.hbwD))
        {
          SIM_ERROR << "Could not determine beam width for AREPS file: " << arepsFile << std::endl;
          return 1;
        }
      }
      else if (st == "[Clutter to noise ratio]")
      {
        // skip comment
        simCore::getStrippedLine(inFile, st);

        simCore::LUT::LUT1<short>* cnr = new simCore::LUT::LUT1<short>();

        // minRange and rangeStep are the same
        minRange_ = (numRanges_ == 0) ? 0 : (maxRange_ / numRanges_);
        cnr->initialize(minRange_, maxRange_, numRanges_);

        size_t rngCnt = 0;
        std::vector<std::string> tmpvec;
        do
        {
          if (simCore::getStrippedLine(inFile, st))
          {
            // tokenize based on white space
            simCore::stringTokenizer(tmpvec, st);
          }
          else
          {
            tmpvec.clear();
          }
          vecLen = tmpvec.size();
          for (size_t i = 0; i < vecLen; ++i)
          {
            // AREPS CNR data stored as decibels, convert to centibels
            float cnr_dB;
            if (rngCnt == numRanges_ || !simCore::isValidNumber(tmpvec[i], cnr_dB))
            {
              SIM_ERROR << "Invalid CNR data for AREPS file: " << arepsFile << std::endl;
              delete cnr;
              return 1;
            }
            short cnr_cB = static_cast<short>(simCore::rint(cnr_dB * AREPS_SCALE_FACTOR));
            (*cnr)(rngCnt) = cnr_cB;
            rngCnt++;
          }
        } while (rngCnt < numRanges_);

        // data must be populated in the provider prior to assigning to profile, provider takes ownership of the cnr LUT
        profile.addProvider(
          new simRF::LUT1ProfileDataProvider(cnr, ProfileDataProvider::THRESHOLDTYPE_CNR, 1.0/AREPS_SCALE_FACTOR));
      }
      else if (st == "[Apm Loss Data]" || st == "[Apm Factor Data]")
      {
        ProfileDataProvider::ThresholdType type;
        if (st == "[Apm Loss Data]")
          type = ProfileDataProvider::THRESHOLDTYPE_LOSS;
        else
        {
          // if assert fails, check that handling of [Apm Loss Data] and [Apm Factor Data] cases here and just above is not mis-implemented
          assert(st == "[Apm Factor Data]");
          type = ProfileDataProvider::THRESHOLDTYPE_FACTOR;
        }

        minRange_ = (numRanges_ == 0) ? 0 : (maxRange_ / numRanges_);
        simCore::LUT::LUT2<short>* loss = new simCore::LUT::LUT2<short>();
        loss->initialize(minHeight_, maxHeight_, numHeights_, minRange_, maxRange_, numRanges_);

        // parse APM data in AREPS file
        std::vector<std::string> vec;

        // skip InitValue, InvalidValue and GroundValue lines
        // skip comment lines, and then read height based values
        do
        {
          simCore::getStrippedLine(inFile, st);
        } while (st.find("Height(") == std::string::npos);

        for (size_t i = 0; i < static_cast<size_t>(numHeights_); i++)
        {
          // read first data line
          simCore::getStrippedLine(inFile, st);
          size_t k = 0;
          do
          {
            simCore::stringTokenizer(vec, st);
            for (size_t j = 0; j < vec.size(); j++)
            {
              // read in centibel data, then store in LUT
              short lossVal = 0;
              if (k == numRanges_ || !simCore::isValidNumber(vec[j], lossVal))
              {
                if (type == ProfileDataProvider::THRESHOLDTYPE_LOSS)
                {
                  SIM_ERROR << "Invalid Loss data for AREPS file: " << arepsFile << std::endl;
                }
                else
                {
                  SIM_ERROR << "Invalid PPF data for AREPS file: " << arepsFile << std::endl;
                }
                delete loss;
                return 1;
              }

              // fix incorrect initialization value
              if (lossVal == AREPS_ERRONEOUS_INIT_VALUE)
                lossVal = AREPS_INIT_VALUE;
              (*loss)(i, k) = lossVal;
              k++;
            }
            simCore::getStrippedLine(inFile, st);
          } while (k < static_cast<size_t>(numRanges_));
        } // end of for numHeights

        // loss/ppf data provided must be populated prior to assigning to profile, provider takes ownership of the LUT
        profile.addProvider(
          new simRF::LUTProfileDataProvider(loss, type, 1.0/AREPS_SCALE_FACTOR));
      }
    }
  } // end of while (simCore::getStrippedLine ...

  if (profile.getDataProvider()->getNumProviders() == 0)
  {
    SIM_ERROR << "File: " << arepsFile << " did not contain valid AREPS data" << std::endl;
    return 1;
  }

  // set our radar parameters for all subsequent files
  if (firstFile && beamHandler_)
  {
    beamHandler_->setRadarParams(radarParameters);
  }

  // string caches for missing data/calcs notifications
  std::string missingData;
  std::string missingCalcs;

  // PODProfileDataProvider depends on the loss provider
  const simRF::ProfileDataProvider* lossProvider = profile.getDataProvider()->getProvider(ProfileDataProvider::THRESHOLDTYPE_LOSS);
  if (beamHandler_ != NULL && lossProvider)
  {
    osg::ref_ptr<PODProfileDataProvider> podProvider = new PODProfileDataProvider(lossProvider, beamHandler_->getPODLossThreshold());
    profile.addProvider(podProvider.get());
  }
  else if (!lossProvider)
  {
    missingData = "loss";
    missingCalcs = "loss, POD";
  }

  // create providers that depend on the PPF provider
  const simRF::ProfileDataProvider* ppfProvider = profile.getDataProvider()->getProvider(ProfileDataProvider::THRESHOLDTYPE_FACTOR);
  if (beamHandler_ != NULL && ppfProvider)
  {
    profile.addProvider(new OneWayPowerDataProvider(ppfProvider, beamHandler_->radarParams()));

    osg::ref_ptr<TwoWayPowerDataProvider> twoWayPowerDataProvider = new TwoWayPowerDataProvider(ppfProvider, beamHandler_->radarParams());
    profile.addProvider(twoWayPowerDataProvider.get());

    // SNRDataProvider depends on TwoWayPowerDataProvider
    profile.addProvider(new SNRDataProvider(twoWayPowerDataProvider.get(), beamHandler_->radarParams()));
  }
  else if (!ppfProvider)
  {
    missingData += missingData.empty() ? "PPF" : ", PPF";
    missingCalcs += missingCalcs.empty() ? "PPF, one-way power, two-way power, SNR" : ", PPF, one-way power, two-way power, SNR";
  }

  // determine if CNR data is available
  if (profile.getDataProvider()->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_CNR) == NULL)
  {
    missingData += missingData.empty() ? "CNR" : ", CNR";
    missingCalcs += missingCalcs.empty() ? "CNR" : ", CNR";
  }

  if (!missingData.empty())
  {
    SIM_WARN << "File: " << arepsFile << " is missing AREPS data types: " << missingData << std::endl;
    SIM_WARN << "The following RF calcs will be unavailable: " << missingCalcs << std::endl;
  }

  profile.setBearing(bearingAngleRad);
  profile.setHalfBeamWidth(radarParameters.hbwD * simCore::DEG2RAD / 2.0);
  profile.setDisplayThickness(maxHeight_);
  return 0;
}

double ArepsLoader::getBearingAngle_(const std::string& infilename) const
{
  // According to SPAWAR, the bearing angle is used in making the
  // file name, hence it is not found in the AREPS ASCII file.
  double bearing = -1;
  if (infilename.empty())
    return bearing;

  // tokenize based on "_", which are used to delineate bearing angle
  // example filename that this processes:  SCORE1_APM_0_15_30.txt
  std::vector<std::string> tmpFileVec;
  simCore::stringTokenizer(tmpFileVec, simCore::StringUtils::beforeLast(infilename, ".txt"), "_");

  // at a minimum, two tokens should be expected (so return invalid bearing)
  if (tmpFileVec.size() < 2)
    return bearing;

  // process tokens, transfer all tokens to the right of "APM"
  std::vector<std::string>::const_reverse_iterator riter;
  std::vector<std::string> bearingVec;
  for (riter = tmpFileVec.rbegin(); riter != tmpFileVec.rend(); ++riter)
  {
    if (simCore::stringCaseFind(*riter, "APM") != std::string::npos)
    {
      break;
    }
    else
    {
      bearingVec.push_back(*riter);
    }
  }

  // reverse order of bearing calc due to vector "push_back"
  switch (bearingVec.size())
  {
  case 1: // degrees
    bearing = atof(bearingVec[0].c_str());
    break;
  case 2: // degrees minutes
    bearing = atof(bearingVec[1].c_str()) + (atof(bearingVec[0].c_str()) / 60.);
    break;
  case 3: // degrees minutes seconds
    bearing = atof(bearingVec[2].c_str()) + (atof(bearingVec[1].c_str()) / 60.) + (atof(bearingVec[0].c_str()) / 3600.);
    break;
  }
  // convert degrees to radians
  return simCore::angFix2PI(bearing * simCore::DEG2RAD);
}

}
