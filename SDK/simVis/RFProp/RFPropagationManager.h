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
#ifndef SIMVIS_RFPROPAGATIONMANAGER_H
#define SIMVIS_RFPROPAGATIONMANAGER_H

#include <memory>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"

namespace simRF
{
class RFPropagationFacade;

/** Factory to manage a set of RFPropagationFacade classes. */
class RFPropagationManager
{
public:
  virtual ~RFPropagationManager() {}

  /**
    * Returns an existing RFPropagationFacade object for the specified platform or beam
    * @param hostId ID of a platform or beam to retrieve RF Propagation data
    * @return RFPropagationData object pointer, nullptr if specified hostId does not have a RFPropagationFacade Object
    */
  virtual RFPropagationFacade* getRFPropagation(simData::ObjectId hostId) const = 0;

  /**
   * Returns existing or newly created RFPropagationFacade object for the specified platform or beam, new objects are owned by this manager
   * @param hostId ID of a platform or beam to retrieve RF Propagation data
   * @return RFPropagationData object pointer, nullptr if specified hostId is not a platform or beam ID
   */
  virtual RFPropagationFacade* getOrCreateRFPropagation(simData::ObjectId hostId) = 0;

  /**
  * Loads the specified files for the specified platform or beam, which must already exist
  * @param hostId ID of a platform or beam to receive RF Propagation data
  * @param files vector of filenames to load
  * @return 0 on success, !0 on error loading the files
  */
  virtual int loadFiles(simData::ObjectId beamId, const std::vector<std::string>& files) = 0;
};

typedef std::shared_ptr<simRF::RFPropagationManager> RFPropagationManagerPtr;

/** Null object implementation for RFPropManager */
class NullRFPropagationManager : public RFPropagationManager
{
public:
  NullRFPropagationManager()
    : RFPropagationManager()
  {
  }
  virtual simRF::RFPropagationFacade* getRFPropagation(simData::ObjectId id) const
  {
    return nullptr;
  }
  virtual simRF::RFPropagationFacade* getOrCreateRFPropagation(simData::ObjectId id)
  {
    return nullptr;
  }
  virtual int loadFiles(simData::ObjectId beamId, const std::vector<std::string>& files)
  {
    return 1;
  }
};

}

#endif /* SIMVIS_RFPROPAGATIONMANAGER_H */
