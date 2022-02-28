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
#include "simVis/Constants.h"
#include "simVis/RangeToolState.h"

namespace simVis
{

RangeToolState::RangeToolState(EntityState* beginEntity, EntityState* endEntity)
  : beginEntity_(beginEntity),
  endEntity_(endEntity)
{
  // Must pass in entities
  assert(beginEntity_ != nullptr);
  assert(endEntity_ != nullptr);
}

RangeToolState::~RangeToolState()
{
  delete beginEntity_;
  delete endEntity_;
}

void RangeToolState::line(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset, osg::Vec3Array* verts)
{
  // Use Sodano method to calculate azimuth and distance
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(lla0.lat(), lla0.lon(), lla0.alt(), lla1.lat(), lla1.lon(), &azimuth);

  // purely vertical line will be drawn as a single segment
  if (simCore::areEqual(distance, 0.0))
  {
    verts->push_back(lla2local(lla0.x(), lla0.y(), lla0.z() + altOffset));
    verts->push_back(lla2local(lla1.x(), lla1.y(), lla1.z() + altOffset));
    return;
  }

  // if total distance of the line is less than the max segment length, use that
  double segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH);
  // When lines are at/close to surface, we might need to tessellate more closely
  if (fabs(lla0.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD && fabs(lla1.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD)
  {
    // if the total distance of the line is less than the max segment length, use that
    segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH_GROUNDED);
  }

  // make sure there's enough room. Don't bother shrinking.
  const unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(distance / segmentLength)));
  verts->reserve(numSegs + 1);
  verts->clear();

  // Add points to the vertex list, from back to front, for consistent stippling.  Order
  // matters because it affects the line direction during stippling.
  for (unsigned int k = 0; k <= numSegs; ++k)
  {
    const float percentOfFull = static_cast<float>(k) / static_cast<float>(numSegs); // From 0 to 1

    // Calculate the LLA value of the point, and replace the altitude
    double lat = 0.0;
    double lon = 0.0;
    simCore::sodanoDirect(lla0.lat(), lla0.lon(), lla0.alt(), distance * percentOfFull, azimuth, &lat, &lon);
    verts->push_back(lla2local(lat, lon, lla0.z() + altOffset));
  }
}

void RangeToolState::intermediatePoints(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double rangeDelta, std::vector<simCore::Vec3>& llaPointsOut) const
{
  llaPointsOut.clear();

  // Use Sodano method to calculate azimuth and distance
  double azimuth = 0.0;
  double distance = simCore::sodanoInverse(lla0.lat(), lla0.lon(), lla0.alt(), lla1.lat(), lla1.lon(), &azimuth);

  if (simCore::areEqual(distance, 0))
  {
    return;
  }

  rangeDelta = simCore::sdkMin(distance, rangeDelta);
  const unsigned int numPoints = static_cast<unsigned int>(distance / rangeDelta) + 1;
  for (unsigned int i = 1; i < numPoints; i++)
  {
    const float portionOfFull = static_cast<float>(i) / static_cast<float>(numPoints); // From 0 to 1

    // Calculate the LLA value of the point, and replace the altitude
    double lat = 0.0;
    double lon = 0.0;
    simCore::sodanoDirect(lla0.lat(), lla0.lon(), lla0.alt(), distance * portionOfFull, azimuth, &lat, &lon);
    llaPointsOut.push_back(simCore::Vec3(lat, lon, 0));
  }
}

simCore::Vec3 RangeToolState::midPoint(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset)
{
  // Use Sodano method to calculate azimuth and distance
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(lla0.lat(), lla0.lon(), lla0.alt(), lla1.lat(), lla1.lon(), &azimuth);

  // purely vertical line will be drawn as a single segment
  if (simCore::areEqual(distance, 0.0))
    return lla0;

  // Calculate the LLA value of the point, and replace the altitude
  double lat = 0.0;
  double lon = 0.0;
  simCore::sodanoDirect(lla0.lat(), lla0.lon(), lla0.alt(), distance * 0.5, azimuth, &lat, &lon);
  return simCore::Vec3(lat, lon, (lla0.alt() + lla1.alt()) / 2.0 + altOffset);
}

osg::Vec3d RangeToolState::rotateEndVec(double az)
{
  // Use Sodano method to calculate azimuth and distance from beginEntity_ to endEntity_
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(beginEntity_->lla_.lat(), beginEntity_->lla_.lon(), beginEntity_->lla_.alt(), endEntity_->lla_.lat(), endEntity_->lla_.lon(), &azimuth);

  // purely vertical line returns the original end entity pos, in local coords
  if (simCore::areEqual(distance, 0.0))
    return coord(COORD_OBJ_1);

  // Calculate the LLA value of the point, and replace the altitude
  double lat = 0.0;
  double lon = 0.0;
  simCore::sodanoDirect(beginEntity_->lla_.lat(), beginEntity_->lla_.lon(), beginEntity_->lla_.alt(), distance, (azimuth - az), &lat, &lon);
  return lla2local(lat, lon, endEntity_->lla_.alt());
}

osg::Vec3 RangeToolState::lla2local(double lat, double lon, double alt) const
{
  simCore::Vec3 ecefPos;
  simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lat, lon, alt), ecefPos);
  return simCore2osg(ecefPos) * world2local_;
}

simCore::Vec3 RangeToolState::local2lla(const osg::Vec3d& local)
{
  const osg::Vec3d world = local * local2world_;
  simCore::Vec3 llaPos;
  simCore::CoordinateConverter::convertEcefToGeodeticPos(osg2simCore(world), llaPos);
  return llaPos;
}

osg::Vec3d RangeToolState::coord(RangeToolState::Coord which)
{
  if (coord_[which].isSet())
    return *coord_[which];

  switch (which)
  {
  case COORD_OBJ_0:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(beginEntity_->lla_, ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_OBJ_1:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(endEntity_->lla_, ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_OBJ_0_0HAE:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(beginEntity_->lla_.x(), beginEntity_->lla_.y(), 0.0), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_OBJ_1_0HAE:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(endEntity_->lla_.x(), endEntity_->lla_.y(), 0.0), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_OBJ_1_AT_OBJ_0_ALT:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(endEntity_->lla_.x(), endEntity_->lla_.y(), beginEntity_->lla_.z()), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_OBJ_0_AT_OBJ_1_ALT:
  {
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(beginEntity_->lla_.x(), beginEntity_->lla_.y(), endEntity_->lla_.z()), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_DR:
  {
    double dr, cr, dv;
    simCore::calculateDRCRDownValue(beginEntity_->lla_, beginEntity_->ypr_.x(), endEntity_->lla_, earthModel_, &coordConv_, &dr, &cr, &dv);

    // down/cross range point in TP coords:
    // TODO: not sure this is correct, should it be calculated in TP space?
    coord_[which] = osg::Vec3d(dr*sin(beginEntity_->ypr_.x()), dr*cos(beginEntity_->ypr_.x()), 0.0);
  }
  break;
  case COORD_VEL_AZIM_DR:
  {
    // measurement is not meaningful when vel is zero
    if (simCore::v3AreEqual(beginEntity_->vel_, simCore::Vec3()))
    {
      coord_[which] = osg::Vec3d();
      break;
    }
    double downRng = 0.0;
    simCore::Vec3 fpa;
    simCore::calculateFlightPathAngles(beginEntity_->vel_, fpa);
    simCore::calculateDRCRDownValue(beginEntity_->lla_, fpa[0],
      endEntity_->lla_,
      earthModel_,
      &coordConv_,
      &downRng,
      nullptr,
      nullptr);
    coord_[which] = osg::Vec3d(downRng*sin(fpa[0]), downRng*cos(fpa[0]), 0.0);
  }
  break;
  case COORD_BEAM_LLA_0:
  case COORD_BEAM_LLA_1:
    // Needs to be handled at a higher level
    assert(0);
    break;

  case COORD_BEAM_0:
  {
    simCore::Vec3 ecefPos;
    const osg::Vec3d& point = coord(COORD_BEAM_LLA_0);
    simCore::CoordinateConverter::convertGeodeticPosToEcef(osg2simCore(point), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_BEAM_1:
  {
    simCore::Vec3 ecefPos;
    const osg::Vec3d& point = coord(COORD_BEAM_LLA_1);
    simCore::CoordinateConverter::convertGeodeticPosToEcef(osg2simCore(point), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_BEAM_0_0HAE:
  {
    simCore::Vec3 ecefPos;
    const osg::Vec3d& point = coord(COORD_BEAM_LLA_0);
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(point.x(), point.y(), 0.0), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_BEAM_1_0HAE:
  {
    simCore::Vec3 ecefPos;
    const osg::Vec3d& point = coord(COORD_BEAM_LLA_1);
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(point.x(), point.y(), 0.0), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_BEAM_1_AT_BEAM_0_ALT:
  {
    simCore::Vec3 ecefPos;
    const simCore::Vec3& from = osg2simCore(coord(RangeToolState::COORD_BEAM_LLA_0));
    const simCore::Vec3& to = osg2simCore(coord(RangeToolState::COORD_BEAM_LLA_1));
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(to.x(), to.y(), from.z()), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;

  case COORD_BEAM_0_AT_BEAM_1_ALT:
  {
    simCore::Vec3 ecefPos;
    const simCore::Vec3& from = osg2simCore(coord(RangeToolState::COORD_BEAM_LLA_0));
    const simCore::Vec3& to = osg2simCore(coord(RangeToolState::COORD_BEAM_LLA_1));
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(from.x(), from.y(), to.z()), ecefPos);
    coord_[which] = simCore2osg(ecefPos) * world2local_;
  }
  break;
  }
  return *coord_[which];
}

void RangeToolState::resetCoordCache()
{
  for (size_t i = 0; i < COORD_CACHE_SIZE; i++)
    coord_[i].clear();
}

simCore::Vec3 RangeToolState::osg2simCore(const osg::Vec3d& point) const
{
  return simCore::Vec3(point.x(), point.y(), point.z());
}

osg::Vec3d RangeToolState::simCore2osg(const simCore::Vec3& point) const
{
  return osg::Vec3d(point.x(), point.y(), point.z());
}

}
