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
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/GeoFence.h"

namespace simCore {

GeoFence::GeoFence()
  : valid_(false)
{
}

GeoFence::GeoFence(const GeoFence& rhs)
  : points_(rhs.points_),
  tope_(rhs.tope_),
  valid_(rhs.valid_)
{
}

GeoFence::GeoFence(const Vec3String& points, const CoordinateSystem& cs)
{
  set(points, cs);
}

void GeoFence::set(const Vec3String& points, const CoordinateSystem& cs)
{
  // must have at least three vertices
  if (points.size() <= 2)
    return;

  // We want ECEF. Convert the input to ECEF if necessary.
  if (cs == COORD_SYS_ECEF)
    points_ = points;
  else
  {
    CoordinateConverter conv;
    Coordinate output;

    points_.clear();

    for (Vec3String::const_iterator i = points.begin(); i != points.end(); ++i)
    {
      Coordinate input(cs, *i);
      conv.convert(input, output, COORD_SYS_ECEF);
      points_.push_back(output.position());
    }
  }

  // Now rebuild the polytope from the ECEF point set.
  tope_.clear();

  Vec3 origin(0, 0, 0);
  Vec3String::const_iterator last = points_.end()-1;

  for (Vec3String::const_iterator i = points_.begin(); i != last; ++i)
  {
    tope_.addPlane(Plane(*i, *(i+1), origin));
  }

  // validate.
  valid_ = verifyConvexity_(points_);
}

bool GeoFence::contains(const Vec3& ecef) const
{
  return tope_.contains(ecef);
}

bool GeoFence::contains(const Coordinate& input) const
{
  if (input.coordinateSystem() == COORD_SYS_ECEF)
    return contains(input.position());

  // convert to ECEF and try again.
  CoordinateConverter conv;
  Coordinate output;
  conv.convert(input, output, COORD_SYS_ECEF);
  return contains(output);
}

bool GeoFence::verifyConvexity_(const Vec3String& v) const
{
  for (unsigned int i = 0; i < v.size(); ++i)
  {
    if (!contains(v[i]))
      return false;
  }
  return true;
}

}
