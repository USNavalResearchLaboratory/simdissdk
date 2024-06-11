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
#ifndef SIMVIS_RFPROP_AREPS_LOADER_H
#define SIMVIS_RFPROP_AREPS_LOADER_H

#include "simVis/RFProp/RFPropagationFacade.h"
#include "simCore/Common/Common.h"

namespace simCore { class TimeStamp; }

namespace simRF
{
/**
 * ArepsLoader is a file loader AREPS .txt files
 */
class SDKVIS_EXPORT ArepsLoader
{
public:
  /**
  * Construct an ArepsLoader class instance
  * some Areps provider types require resources provided by an RFPropagationFacade instance
  * if no RFPropagationFacade instance is provided, only PPF and Loss providers are supported
  * @param beamHandler ptr to a RFPropagationFacade instance
  */
  ArepsLoader(RFPropagationFacade* beamHandler = nullptr);
  ~ArepsLoader();

  /**
   * Loads one AREPS file into the specified profile
   * @param arepsFile filename to load
   * @param profile profile to load with information from file
   * @param firstFile indicator that this is the first file in a set of related files
   * @return 0 on success, !0 on error loading the file
   */
  int loadFile(const std::string& arepsFile, simRF::Profile& profile, bool firstFile = true);

  /**
   * Retrieves the antenna height used by files
   * @return antennaHeight used by loaded files; in meters; not valid before load()
   */
  double getAntennaHeight() const;

private:
  /**
   * getBearingAngle_() obtains the bearing angle for the file, from the filename;
   * this is to support older versions of AREPS files which specified the bearing for a file only in the filename
   * @param infilename filename to process
   * @return the bearing (converted to radians) extracted from the filename
   */
  double getBearingAngle_(const std::string& infilename) const;

private:
  double maxHeight_;
  double minHeight_;
  size_t numRanges_;
  size_t numHeights_;
  double maxRange_;
  double minRange_;
  double antennaHgt_;
  RFPropagationFacade* beamHandler_;
};
}

#endif /* SIMVIS_RFPROP_AREPS_LOADER_H */

