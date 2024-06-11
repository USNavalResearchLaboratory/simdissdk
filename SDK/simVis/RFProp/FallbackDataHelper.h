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
#ifndef SIMVIS_RFPROP_FALLBACK_DATA_HELPER_H
#define SIMVIS_RFPROP_FALLBACK_DATA_HELPER_H
#include "simCore/EM/Decibel.h"

namespace simRF
{
/**
 * FallbackDataHelper provides an interface to define classes that
 * the RFPropagationFacade can fall back upon when there is no
 * adequate ProfileDataProvider loaded for a given data request.
 * E.g., a LossDataHelper that can be configured to fetch a calculation
 * from an external API.
 */
class FallbackDataHelper
{
public:
  virtual ~FallbackDataHelper() {}

  /**
   * Indicates whether the DataHelper can provide a result for the given beam.
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @return -300 on success, valid value otherwise
   */
  virtual double value(double azimRad, double gndRngMeters, double hgtMeters) = 0;
};

class NullDataHelper : public FallbackDataHelper
{
public:
  NullDataHelper() {}

  /**
   * Indicates whether the DataHelper can provide a result for the given beam.
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @return -300 on success, valid value otherwise
   */
  virtual double value(double azimRad, double gndRngMeters, double hgtMeters) override
  {
    return simCore::SMALL_DB_VAL;
  }
};

}

#endif /* SIMVIS_RFPROP_FALLBACK_DATA_HELPER_H */
