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
#include "simData/DataTypes.h"

namespace simData
{
  // Max value is used as no value.
  PlatformUpdate::PlatformUpdate()
    : time_(std::numeric_limits<double>::max()),
      x_(std::numeric_limits<double>::max()),
      y_(std::numeric_limits<double>::max()),
      z_(std::numeric_limits<double>::max()),
      psi_(std::numeric_limits<float>::max()),
      theta_(std::numeric_limits<float>::max()),
      phi_(std::numeric_limits<float>::max()),
      vx_(std::numeric_limits<float>::max()),
      vy_(std::numeric_limits<float>::max()),
      vz_(std::numeric_limits<float>::max())
  {
  }

  void PlatformUpdate::CopyFrom(const PlatformUpdate& from)
  {
    if (&from == this)
      return;

    time_ = from.time_;
    x_ = from.x_;
    y_ = from.y_;
    z_ = from.z_;
    psi_= from.psi_;
    theta_ = from.theta_;
    phi_ = from.phi_;
    vx_ = from.vx_;
    vy_ = from.vy_;
    vz_ = from.vz_;
  }

  bool operator!=(const Position &left, const Position &right)
  {
    return left.x() != right.x() ||
           left.y() != right.y() ||
           left.z() != right.z();
  }

  bool operator==(const Position &left, const Position &right)
  {
    return !(left != right);
  }
}

