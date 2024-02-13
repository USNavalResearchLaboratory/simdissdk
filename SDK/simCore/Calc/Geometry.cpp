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

#undef  LC
#define LC "[simCore::Plane] "

namespace simCore {

Plane::Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
  Vec3 a = p2 - p1;
  Vec3 b = p3 - p2;
  Vec3 w = a.cross(b);
  Vec3 normal = w.normalize();

  v_[0] = normal[0];
  v_[1] = normal[1];
  v_[2] = normal[2];
  v_[3] = -p1.dot(normal);
}

Plane::Plane(const Plane& rhs)
{
  for (int i = 0; i < 4; ++i)
    v_[i] = rhs.v_[i];
}

double Plane::distance(const Vec3& p) const
{
  return v_[0]*p.x() + v_[1]*p.y() + v_[2]*p.z() + v_[3];
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[simCore::Polytope] "

Polytope::Polytope()
{
}

Polytope::Polytope(const Polytope& rhs) :
planes_(rhs.planes_)
{
}

void Polytope::addPlane(const Plane& plane)
{
  planes_.push_back(plane);
}

bool Polytope::contains(const Vec3& p) const
{
  const double epsilon = 1e-5;

  for (std::vector<Plane>::const_iterator i = planes_.begin(); i != planes_.end(); ++i)
  {
    const Plane& plane = *i;
    double dist = plane.distance(p);
    if (dist + epsilon < 0.0)
      return false;
  }
  return true;
}

void Polytope::clear()
{
  planes_.clear();
}

//------------------------------------------------------------------------

#undef  LC
#define LC "[simCore::GeoFence] "

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
