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
#ifndef SIMDATA_NEARESTNEIGHBOR_INTERPOLATOR_H
#define SIMDATA_NEARESTNEIGHBOR_INTERPOLATOR_H

#include "simCore/Common/Export.h"
#include "simData/Interpolator.h"

namespace simData
{

/// An interpolation object used to compute nearest-neighbor interpolated data points for Platforms, Beams, and Gates
class SDKDATA_EXPORT NearestNeighborInterpolator : public Interpolator
{
public:
  /** @see Interpolator::interpolate() */
  virtual bool interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result);

  /** @see Interpolator::interpolate() */
  virtual bool interpolate(double time, const BeamUpdate &prev, const BeamUpdate &next, BeamUpdate *result);

  /** @see Interpolator::interpolate() */
  virtual bool interpolate(double time, const GateUpdate &prev, const GateUpdate &next, GateUpdate *result);

  /** @see Interpolator::interpolate() */
  virtual bool interpolate(double time, const LaserUpdate &prev, const LaserUpdate &next, LaserUpdate *result);

  /** @see Interpolator::interpolate() */
  virtual bool interpolate(double time, const ProjectorUpdate &prev, const ProjectorUpdate &next, ProjectorUpdate *result);
}; // End NearestNeighborInterpolator

} // End namespace simData

#endif
