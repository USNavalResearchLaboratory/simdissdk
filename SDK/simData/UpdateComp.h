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
#ifndef SIMDATA_UPDATECOMP_H
#define SIMDATA_UPDATECOMP_H

namespace simData
{

/// Comparison operator for data entry updates (Platform, Beam, Gate, etc), compared by time
template<typename T>
struct UpdateComp
{
  /// Less-than operator for time, two structures
  bool operator()(const T *lhs, const T *rhs) const
  {
    return lhs->time() < rhs->time();
  }

  /// Less-than operator for time, LHS structure
  bool operator()(const T *lhs, double rhs) const
  {
    return lhs->time() < rhs;
  }

  /// Less-than operator for time, RHS structure
  bool operator()(double lhs, const T *rhs) const
  {
    return lhs < rhs->time();
  }
};

} // End of namespace simData

#endif // SIMDATA_UPDATECOMP_H
