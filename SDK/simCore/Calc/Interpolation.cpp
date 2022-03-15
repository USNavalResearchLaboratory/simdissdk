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
#include "simCore/Calc/Interpolation.h"
#include "simCore/Calc/Angle.h"

namespace simCore
{
  template<>
  Vec3 linearInterpolate<Vec3>(const Vec3 &prev, const Vec3 &next, double mixFactor)
  {
    return Vec3(
      simCore::linearInterpolate(prev.x(), next.x(), mixFactor),
      simCore::linearInterpolate(prev.y(), next.y(), mixFactor),
      simCore::linearInterpolate(prev.z(), next.z(), mixFactor));
  }

  double linearInterpolateAngle(double a, double b, double factor)
  {
    double angle;

    a = angFix2PI(a);
    b = angFix2PI(b);
    // Check for 360 to 0 degree azimuth crossing for interpolation
    if (fabs(b - a) > M_PI)
    {
      if (b > a)
      {
        a += M_TWOPI;
      }
      else
      {
        b += M_TWOPI;
      }
      // Interpolate Vector Azim and correct angle
      angle = angFix2PI(linearInterpolate(a, b, factor));
    }
    else
    {
      // Interpolate Vector Azim
      angle = linearInterpolate(a, b, factor);
    }

    return angle;
  }

  double linearInterpolateAngle(double a, double b, double ta, double t, double tb)
  {
    // This assertion isn't strictly required, but if it occurs, we're extrapolating instead of interpolating.
    assert(ta <= t && t <= tb);
    if (ta == tb)
    {
      return b;
    }
    return linearInterpolateAngle(a, b, (t - ta) / (tb - ta));
  }

} // end of namespace simCore
