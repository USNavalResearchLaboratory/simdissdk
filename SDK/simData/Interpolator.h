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
#ifndef SIMDATA_INTERPOLATOR_H
#define SIMDATA_INTERPOLATOR_H

#include "simCore/Common/Export.h"
#include "simData/DataTypes.h"

namespace simData
{

/// Interface for finding a value for a time between two given values at given times
class SDKDATA_EXPORT Interpolator
{
public:
  virtual ~Interpolator() {}

  /**
   * Computes an interpolated Platform update for the specified time.
   * The value is interpolated from the updates prev and next for the specified time between the updates specified by prev and next, and stores the result
   *
   * If the times for either prev or next match the specified time, the value with the matching time should be
   * copied into the result value, with no interpolation performed.
   *
   * @return true if interpolated, false if copied.
   */
  virtual bool interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result) = 0;

 /**
   * Computes an interpolated Beam update for the specified time.
   * The value is interpolated from the updates prev and next for the specified time between the updates specified by prev and next, and stores the result
   *
   * If the times for either prev or next match the specified time, the value with the matching time should be
   * copied into the result value, with no interpolation performed.
   *
   * @return true if interpolated, false if copied.
   */
  virtual bool interpolate(double time, const BeamUpdate &prev, const BeamUpdate &next, BeamUpdate *result) = 0;

  /**
   * Computes an interpolated Gate update for the specified time.
   * The value is interpolated from the updates prev and next for the specified time between the updates specified by prev and next, and stores the result
   *
   * If the times for either prev or next match the specified time, the value with the matching time should be
   * copied into the result value, with no interpolation performed.
   *
   * @return true if interpolated, false if copied.
   */
  virtual bool interpolate(double time, const GateUpdate &prev, const GateUpdate &next, GateUpdate *result) = 0;

  /**
   * Computes an interpolated Laser update for the specified time.
   * The value is interpolated from the updates prev and next for the specified time between the updates specified by prev and next, and stores the result
   *
   * If the times for either prev or next match the specified time, the value with the matching time should be
   * copied into the result value, with no interpolation performed.
   *
   * @return true if interpolated, false if copied.
   */
  virtual bool interpolate(double time, const LaserUpdate &prev, const LaserUpdate &next, simData::LaserUpdate *result) = 0;

  /**
  * Computes an interpolated Projector update for the specified time.
  * The value is interpolated from the updates prev and next for the specified time between the updates specified by prev and next, and stores the result
  *
  * If the times for either prev or next match the specified time, the value with the matching time should be
  * copied into the result value, with no interpolation performed.
  *
  * @return true if interpolated, false if copied.
  */
  virtual bool interpolate(double time, const ProjectorUpdate &prev, const ProjectorUpdate &next, simData::ProjectorUpdate *result) = 0;
}; // End Interpolator

} // End namespace simData

#endif

