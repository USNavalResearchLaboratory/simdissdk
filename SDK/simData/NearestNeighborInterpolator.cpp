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
#include <cassert>
#include "simData/NearestNeighborInterpolator.h"

using namespace simData;

namespace
{
  template <typename T>
  bool compute(double time, const T &prev, const T &next, T *result)
  {
    assert(result != NULL && prev.time() <= time && time <= next.time());

    double midPoint = (next.time() + prev.time()) / 2.0;
    if (time < midPoint)
    {
      result->CopyFrom(prev);
    }
    else
    {
      result->CopyFrom(next);
    }

    result->set_time(time);
    return false;
  }
}

bool NearestNeighborInterpolator::interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result)
{
  return compute(time, prev, next, result);
}

bool NearestNeighborInterpolator::interpolate(double time, const BeamUpdate &prev, const BeamUpdate &next, BeamUpdate *result)
{
  return compute(time, prev, next, result);
}

bool NearestNeighborInterpolator::interpolate(double time, const GateUpdate &prev, const GateUpdate &next, GateUpdate *result)
{
  return compute(time, prev, next, result);
}

bool NearestNeighborInterpolator::interpolate(double time, const LaserUpdate &prev, const LaserUpdate &next, LaserUpdate *result)
{
  return compute(time, prev, next, result);
}

bool NearestNeighborInterpolator::interpolate(double time, const ProjectorUpdate &prev, const ProjectorUpdate &next, ProjectorUpdate *result)
{
  return compute(time, prev, next, result);
}
