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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMDATA_LINEAR_INTERPOLATOR_H
#define SIMDATA_LINEAR_INTERPOLATOR_H

#include "simCore/Common/Export.h"
#include "simData/Interpolator.h"

namespace simData
{

  /// An interpolation object used to compute linearly interpolated data points for objects
  /**
  * @brief Performs interpolation between a set of bounded data values
  * @param[in ] time Time of requested update
  * @param[in ] prev Previous (low bound) data store update
  * @param[in ] next Next (high bound) data store update
  * @param[out] result Interpolated data store update
  * @pre result valid, result cannot be the same data store structure as either prev or next
  * @return true if interpolation was a success
  */
  class SDKDATA_EXPORT LinearInterpolator : public Interpolator
  {
  public:

    virtual bool interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result);

    virtual bool interpolate(double time, const BeamUpdate &prev, const BeamUpdate &next, BeamUpdate *result);

    virtual bool interpolate(double time, const GateUpdate &prev, const GateUpdate &next, GateUpdate *result);

    virtual bool interpolate(double time, const LaserUpdate &prev, const LaserUpdate &next, simData::LaserUpdate *result);

    virtual bool interpolate(double time, const ProjectorUpdate &prev, const ProjectorUpdate &next, simData::ProjectorUpdate *result);
  };

}

#endif
