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
#include <cassert>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Dcm.h"

namespace simCore
{

Dcm::Dcm()
{
}

Dcm::~Dcm()
{
}

Vec3 Dcm::toEuler() const
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 29, Eqn 1.3-24

  // The Direction Cosine Matrix to Euler angles function converts a
  // 3x3 direction cosine matrix (DCM) into three Euler rotation angles.
  // The DCM matrix performs the coordinate transformation of a vector in
  // inertial axes into a vector in body axes. The order of the axis
  // rotations required to bring the body axis into coincidence with the
  // inertial axis is first, a rotation about the body x through the roll
  // angle phi, second, a rotation about the body y through the pitch angle
  // theta, and finally a rotation about the body z through the yaw angle psi.

  // prevent division by zero and inverse trig function arguments of
  // magnitude greater than unity:
  //   atan2 returns in the range -pi to pi
  //   asin returns in the range -pi/2 to pi/2
  Vec3 ea;
  if (areEqual(get(0, 2), 1.0))
  {
    ea.setV0(0.0);
    ea.setV1(-M_PI_2);
    ea.setV2(atan2(-get(1, 0), -get(2, 0)));
  }
  else if (areEqual(get(0, 2), -1.0))
  {
    ea.setV1(M_PI_2);
    ea.setV2(atan2(get(1, 0), get(2, 0)));
    ea.setV0(0.0);
  }
  else
  {
    // no gimbal lock
    // we want psi (yaw) between 0 to 360
    ea.setV0(angFix2PI(atan2(get(0, 1), get(0, 0))));
    ea.setV1(simCore::inverseSine(-get(0, 2)));
    ea.setV2(atan2(get(1, 2), get(2, 2)));
  }
  return ea;
}

void Dcm::fromEuler(const Vec3& ea)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 26, Eqn 1.3-20

  // The Euler angles to Direction Cosine Matrix function converts three
  // Euler rotation angles into a 3x3 direction cosine matrix (DCM).
  // The DCM matrix performs the coordinate transformation of a yaw, pitch, roll
  // (YPR) vector in inertial axes into a vector in a NED body axes. The order
  // of the axis rotations required to bring the body axis into coincidence with
  // the inertial axis is first, a rotation about the body x through the roll
  // angle phi, second, a rotation about the body y through the pitch angle
  // theta, and finally a rotation about the body z through the yaw angle psi.

  // psi/yaw components
  const double spsi = sin(ea[0]);
  const double cpsi = cos(ea[0]);
  // theta/pitch components
  const double stheta = sin(ea[1]);
  const double ctheta = cos(ea[1]);
  // phi/roll components
  const double sphi = sin(ea[2]);
  const double cphi = cos(ea[2]);

  // complete transformation from a reference frame to the body frame
  // The sequence of rotations to describe the instantaneous attitude
  // (orientation) with respect to a reference frame is as follows:
  //
  // 1. Rotate about the z-axis, nose right (positive yaw)
  // 2. Rotate about the new y-axis, nose up (positive pitch)
  // 3. Rotate about the new x-axis, right wing down (positive roll)
  //
  // Coordinate transformation from YPR vector to NED frame

  set(0, 0, cpsi * ctheta);
  set(0, 1, spsi * ctheta);
  set(0, 2, -stheta);

  set(1, 0, cpsi * stheta * sphi - spsi * cphi);
  set(1, 1, spsi * stheta * sphi + cpsi * cphi);
  set(1, 2, ctheta * sphi);

  set(2, 0, cpsi * stheta * cphi + spsi * sphi);
  set(2, 1, spsi * stheta * cphi - cpsi * sphi);
  set(2, 2, ctheta * cphi);
}

int Dcm::fromDQ(const double q[4])
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // 
  // Function expects a normalized quaternion in the form:  q0s + q1i + q2j + q3k
  // note that the conversion from quaternion to DCM is straightforward, but the reverse is not.

  if (q == nullptr)
  {
    assert(0);
    return 1;
  }
  return fromQ(std::to_array<double>({ q[0], q[1], q[2], q[3] }));
}

int Dcm::fromQ(const std::array<double, 4>& q)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // 
  // Function expects a normalized quaternion in the form:  q0s + q1i + q2j + q3k
  // note that the conversion from quaternion to DCM is straightforward, but the reverse is not.

  if (q.empty())
  {
    assert(0);
    return 1;
  }

  const double sq0 = q[0] * q[0];
  const double sq1 = q[1] * q[1];
  const double sq2 = q[2] * q[2];
  const double sq3 = q[3] * q[3];

  // this is the normalization criterion:
  if (!areEqual((sq0 + sq1 + sq2 + sq3), 1.0, 1.E-10))
    return 1;

  const double q0q1 = q[0] * q[1];
  const double q0q2 = q[0] * q[2];
  const double q0q3 = q[0] * q[3];

  const double q1q2 = q[1] * q[2];
  const double q1q3 = q[1] * q[3];
  const double q2q3 = q[2] * q[3];

  set(0, 0, sq0 + sq1 - sq2 - sq3);
  set(0, 1, 2. * (q1q2 + q0q3));
  set(0, 2, 2. * (q1q3 - q0q2));

  set(1, 0, 2. * (q1q2 - q0q3));
  set(1, 1, sq0 - sq1 + sq2 - sq3);
  set(1, 2, 2. * (q2q3 + q0q1));

  set(2, 0, 2. * (q1q3 + q0q2));
  set(2, 1, 2. * (q2q3 - q0q1));
  set(2, 2, sq0 - sq1 - sq2 + sq3);
  return 0;
}

}
