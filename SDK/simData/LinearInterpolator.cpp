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
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Interpolation.h"
#include "simData/LinearInterpolator.h"

namespace simData {

bool LinearInterpolator::interpolate(double time, const PlatformUpdate &prev, const PlatformUpdate &next, PlatformUpdate *result)
{
  // Test for same input/output -- this function cannot handle case of prev == result, or next == result
  if (!result || &prev == result || &next == result)
  {
    assert(0);
    return false;
  }
  // time must be within bounds for interpolation to work
  assert(prev.time() <= time && time <= next.time());

  // compute time ratio
  double factor = simCore::getFactor(prev.time(), time, next.time());

  simCore::Coordinate prevEcef(simCore::COORD_SYS_ECEF, simCore::Vec3(prev.x(), prev.y(), prev.z()));
  if (prev.has_orientation() && next.has_orientation())
    prevEcef.setOrientation(simCore::Vec3(prev.psi(), prev.theta(), prev.phi()));
  if (prev.has_velocity() && next.has_velocity())
    prevEcef.setVelocity(simCore::Vec3(prev.vx(), prev.vy(), prev.vz()));

  const std::optional<simCore::Coordinate> prevLlaOpt = simCore::CoordinateConverter::convertEcefToGeodetic(prevEcef);
  if (!prevLlaOpt)
  {
    assert(false); // Cannot fail, prevEcef guaranteed to be ecef
    return false;
  }

  simCore::Coordinate nextEcef(simCore::COORD_SYS_ECEF, simCore::Vec3(next.x(), next.y(), next.z()));
  if (prev.has_orientation() && next.has_orientation())
    nextEcef.setOrientation(simCore::Vec3(next.psi(), next.theta(), next.phi()));
  if (prev.has_velocity() && next.has_velocity())
    nextEcef.setVelocity(simCore::Vec3(next.vx(), next.vy(), next.vz()));

  const std::optional<simCore::Coordinate> nextLlaOpt = simCore::CoordinateConverter::convertEcefToGeodetic(nextEcef);
  if (!nextLlaOpt)
  {
    assert(false); // Cannot fail, nextEcef guaranteed to be ecef
    return false;
  }

  // do the interpolation in geocentric, this way the
  // interpolation is correct at N/S and E/W transitions
  simCore::Vec3 xyz(simCore::linearInterpolate(prev.x(), next.x(), factor),
                    simCore::linearInterpolate(prev.y(), next.y(), factor),
                    simCore::linearInterpolate(prev.z(), next.z(), factor));

  std::optional<simCore::Vec3> llaPosOpt = simCore::CoordinateConverter::convertEcefToGeodeticPos(xyz);
  if (!llaPosOpt)
  {
    assert(false); // guaranteed to be safe by wrapper
    return false;
  }

  // Use interpolated geodetic altitude to prevent short cuts through the earth
  simCore::Coordinate resultsLla;
  resultsLla.setCoordinateSystem(simCore::COORD_SYS_LLA);
  resultsLla.setPositionLLA(llaPosOpt->lat(), llaPosOpt->lon(), simCore::linearInterpolate(prevLlaOpt->z(), nextLlaOpt->z(), factor));

  if (prev.has_orientation() && next.has_orientation())
  {
    double l_yaw = (simCore::angFix2PI(prevLlaOpt->yaw()));
    double l_pitch = (simCore::angFix2PI(prevLlaOpt->pitch()));
    double l_roll = (simCore::angFix2PI(prevLlaOpt->roll()));
    double h_yaw = (simCore::angFix2PI(nextLlaOpt->yaw()));
    double h_pitch = (simCore::angFix2PI(nextLlaOpt->pitch()));
    double h_roll = (simCore::angFix2PI(nextLlaOpt->roll()));

    // orientations assumed to be between 0 and 360
    double delta_yaw = (h_yaw - l_yaw);
    double delta_pitch = (h_pitch - l_pitch);
    double delta_roll = (h_roll - l_roll);

    double yaw;
    double pitch;
    double roll;

    if (delta_yaw == 0.)
      yaw = l_yaw;
    else if (std::abs(delta_yaw) < M_PI)
      yaw = (l_yaw + factor * delta_yaw);
    else
    {
      if (delta_yaw > 0)
        yaw = (l_yaw - factor * (M_TWOPI - delta_yaw));
      else
        yaw = (l_yaw + factor * (M_TWOPI + delta_yaw));
    }

    if (delta_pitch == 0.)
      pitch = l_pitch;
    else if (std::abs(delta_pitch) < M_PI)
      pitch = (l_pitch + factor * delta_pitch);
    else
    {
      if (delta_pitch > 0)
        pitch = (l_pitch - factor * (M_TWOPI - delta_pitch));
      else
        pitch = (l_pitch + factor * (M_TWOPI + delta_pitch));
    }

    if (delta_roll == 0.)
      roll = l_roll;
    else if (std::abs(delta_roll) < M_PI)
      roll = (l_roll + factor * delta_roll);
    else
    {
      if (delta_roll > 0)
        roll = (l_roll - factor * (M_TWOPI - delta_roll));
      else
        roll = (l_roll + factor * (M_TWOPI + delta_roll));
    }

    resultsLla.setOrientation(yaw, pitch, roll);
  }

  if (prev.has_velocity() && next.has_velocity())
  {
    resultsLla.setVelocity(simCore::linearInterpolate(prevLlaOpt->vx(), nextLlaOpt->vx(), factor),
                           simCore::linearInterpolate(prevLlaOpt->vy(), nextLlaOpt->vy(), factor),
                           simCore::linearInterpolate(prevLlaOpt->vz(), nextLlaOpt->vz(), factor));
  }

  std::optional<simCore::Coordinate> resultsEcefOpt = simCore::CoordinateConverter::convertGeodeticToEcef(resultsLla);
  if (!resultsEcefOpt)
  {
    assert(false); // must succeed, resultsLla known to be in LLA
    return false;
  }

  result->set_time(time);

  result->set_x(resultsEcefOpt->x());
  result->set_y(resultsEcefOpt->y());
  result->set_z(resultsEcefOpt->z());

  if (resultsEcefOpt->hasVelocity())
  {
    result->set_vx(resultsEcefOpt->vx());
    result->set_vy(resultsEcefOpt->vy());
    result->set_vz(resultsEcefOpt->vz());
  }

  if (resultsEcefOpt->hasOrientation())
  {
    result->set_psi(resultsEcefOpt->psi());
    result->set_theta(resultsEcefOpt->theta());
    result->set_phi(resultsEcefOpt->phi());
  }

  return true;
}

bool LinearInterpolator::interpolate(double time, const BeamUpdate &prev, const BeamUpdate &next, BeamUpdate *result)
{
  // Test for same input/output -- this function cannot handle case of prev == result, or next == result
  if (!result || &prev == result || &next == result)
  {
    assert(0);
    return false;
  }
  // time must be within bounds for interpolation to work
  assert(prev.time() <= time && time <= next.time());

  result->set_time(time);

  // compute time ratio
  double factor = simCore::getFactor(prev.time(), time, next.time());

  result->set_azimuth(simCore::linearInterpolateAngle(prev.azimuth(), next.azimuth(), factor));
  result->set_elevation(simCore::angFixPI(simCore::linearInterpolateAngle(prev.elevation(), next.elevation(), factor)));
  result->set_range(simCore::linearInterpolate(prev.range(), next.range(), factor));

  return true;
}

bool LinearInterpolator::interpolate(double time, const GateUpdate &prev, const GateUpdate &next, GateUpdate *result)
{
  // Test for same input/output -- this function cannot handle case of prev == result, or next == result
  if (!result || &prev == result || &next == result)
  {
    assert(0);
    return false;
  }
  // time must be within bounds for interpolation to work
  assert(prev.time() <= time && time <= next.time());

  result->set_time(time);

  // compute time ratio
  double factor = simCore::getFactor(prev.time(), time, next.time());

  result->set_azimuth(simCore::linearInterpolateAngle(prev.azimuth(), next.azimuth(), factor));
  result->set_elevation(simCore::angFixPI(simCore::linearInterpolateAngle(prev.elevation(), next.elevation(), factor)));

  result->set_centroid(simCore::linearInterpolate(prev.centroid(), next.centroid(), factor));
  result->set_minrange(simCore::linearInterpolate(prev.minrange(), next.minrange(), factor));
  result->set_maxrange(simCore::linearInterpolate(prev.maxrange(), next.maxrange(), factor));

  //  if width <= 0, gate uses beam width, not interpolated here
  if (prev.width() <= 0 || next.width() <= 0)
    result->set_width(prev.width());
  else
    result->set_width(simCore::linearInterpolate(prev.width(), next.width(), factor));

  //  if height <= 0, gate uses beam height, not interpolated here
  if (prev.height() <= 0 || next.height() <= 0)
    result->set_height(prev.height());
  else
    result->set_height(simCore::linearInterpolate(prev.height(), next.height(), factor));

  return true;
}

bool LinearInterpolator::interpolate(double time, const LaserUpdate &prev, const LaserUpdate &next, simData::LaserUpdate *result)
{
  // Test for same input/output -- this function cannot handle case of prev == result, or next == result
  if (!result || &prev == result || &next == result)
  {
    assert(0);
    return false;
  }
  // time must be within bounds for interpolation to work
  assert(prev.time() <= time && time <= next.time());

  result->set_time(time);

  // compute time ratio
  const double factor = simCore::getFactor(prev.time(), time, next.time());

  // apply Euler angle interpolation
  const double yaw = simCore::angFix2PI(simCore::linearInterpolateAngle(prev.yaw(), next.yaw(), factor));
  const double pitch = simCore::angFixPI(simCore::linearInterpolateAngle(prev.pitch(), next.pitch(), factor));
  const double roll = simCore::angFixPI(simCore::linearInterpolateAngle(prev.roll(), next.roll(), factor));

  result->set_yaw(yaw);
  result->set_pitch(pitch);
  result->set_roll(roll);

  return true;
}

bool LinearInterpolator::interpolate(double time, const ProjectorUpdate &prev, const ProjectorUpdate &next, simData::ProjectorUpdate *result)
{
  // Test for same input/output -- this function cannot handle case of prev == result, or next == result
  if (!result || &prev == result || &next == result)
  {
    assert(0);
    return false;
  }
  // time must be within bounds for interpolation to work
  assert(prev.time() <= time && time <= next.time());

  result->set_time(time);

  // compute time ratio
  double factor = simCore::getFactor(prev.time(), time, next.time());

  result->set_fov(simCore::linearInterpolate(prev.fov(), next.fov(), factor));
  // <=0 is a special value indicating to use the image's aspect ratio. Don't interpolate between that and a manual hfov
  if (prev.hfov() > 0 && next.hfov() > 0)
    result->set_hfov(simCore::linearInterpolate(prev.hfov(), next.hfov(), factor));
  else
    result->set_hfov(prev.hfov());
  return true;
}

}
