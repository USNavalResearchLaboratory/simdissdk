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
#ifndef SIMVIS_RFPROPAGATIONMANAGER_H
#define SIMVIS_RFPROPAGATIONMANAGER_H

#include <memory>
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
    * Returns an existing RFPropagationFacade object for the specified beam entity
    * @param beamId ID of a beam to retrieve RF Propagation data
    * @return RFPropagationData object pointer, NULL if specified beamId does not have a RFPropagationFacade Object
    */
  virtual RFPropagationFacade* getRFPropagation(simData::ObjectId beamId) const = 0;

  /**
   * Returns existing or newly created RFPropagationFacade object for the specified beam entity, new objects are owned by this manager
   * @param beamId ID of a beam to retrieve RF Propagation data
   * @return RFPropagationData object pointer, NULL if specified beamId is not a beam entity ID
   */
  virtual RFPropagationFacade* getOrCreateRFPropagation(simData::ObjectId beamId) = 0;
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
    return NULL;
  }
  virtual simRF::RFPropagationFacade* getOrCreateRFPropagation(simData::ObjectId id)
  {
    return NULL;
  }
};

}

#endif /* SIMVIS_RFPROPAGATIONMANAGER_H */
