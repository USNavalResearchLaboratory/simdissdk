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
#include <limits>
#include "simVis/RadialLOS.h"
#include "simVis/Utils.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "osgEarth/GeoData"
#include "osgEarth/Terrain"
#include <cassert>

#define LC "[LOS] "

using namespace simVis;

//----------------------------------------------------------------------------

RadialLOS::Sample::Sample(double range_m, const osgEarth::GeoPoint& point)
  : valid_(false),
    range_m_(range_m),
    point_(point)
{
  //nop
}

RadialLOS::Sample::Sample(double range_m, const osgEarth::GeoPoint& point, double hamsl_m, double hae_m, double dot, bool visible)
  : valid_(true),
    range_m_(range_m),
    hamsl_m_(hamsl_m),
    hae_m_(hae_m),
    elev_rad_(dot),
    visible_(visible),
    point_(point)
{
  //nop
}

RadialLOS::Sample::Sample(const Sample& rhs)
  : valid_(rhs.valid_),
    range_m_(rhs.range_m_),
    hamsl_m_(rhs.hamsl_m_),
    hae_m_(rhs.hae_m_),
    elev_rad_(rhs.elev_rad_),
    visible_(rhs.visible_),
    point_(rhs.point_)
{
  //nop
}

//----------------------------------------------------------------------------

RadialLOS::RadialLOS()
  : dirty_(true),
    range_max_(Distance(10.0, Units::KILOMETERS)),
    range_resolution_(Distance(1.0, Units::KILOMETERS)),
    azim_center_(Angle(0.0, Units::DEGREES)),
    fov_(Angle(360.0, Units::DEGREES)),
    azim_resolution_(Angle(15.0, Units::DEGREES))
{
  //nop
}

RadialLOS::RadialLOS(const RadialLOS& rhs)
  : dirty_(rhs.dirty_),
    radials_(rhs.radials_),
    originMap_(rhs.originMap_),
    range_max_(rhs.range_max_),
    range_resolution_(rhs.range_resolution_),
    azim_center_(rhs.azim_center_),
    fov_(rhs.fov_),
    azim_resolution_(rhs.azim_resolution_)
{
  //nop
}

void RadialLOS::setMaxRange(const Distance& value)
{
  if (range_max_ != value)
  {
    range_max_ = value;
    dirty_  = true;
  }
}

void RadialLOS::setCentralAzimuth(const Angle& value)
{
  if (azim_center_ != value)
  {
    azim_center_ = value;
    dirty_ = true;
  }
}

void RadialLOS::setFieldOfView(const Angle& value)
{
  if (fov_ != value)
  {
    fov_ = value;
    dirty_ = true;
  }
}

void RadialLOS::setRangeResolution(const Distance& value)
{
  if (range_resolution_ != value)
  {
    range_resolution_ = value;
    dirty_  = true;
  }
}

void RadialLOS::setAzimuthalResolution(const Angle& value)
{
  if (azim_resolution_ != value)
  {
    azim_resolution_ = value;
    dirty_  = true;
  }
}

bool RadialLOS::compute(osgEarth::MapNode* mapNode, const simCore::Coordinate& originCoord)
{
  assert(mapNode != NULL);

  // clear out existing data
  radials_.clear();

  // set up the localizer transforms:
  if (!convertCoordToGeoPoint(originCoord, originMap_, mapNode->getMapSRS()))
    return false;

  osg::Matrix local2world;
  originMap_.createLocalToWorld(local2world);

  // convert everything to the proper units:
  double azim_center  = azim_center_.as(Units::RADIANS);
  double fov          = fov_.as(Units::RADIANS);
  double azim_res_rad = azim_resolution_.as(Units::RADIANS);
  double range_max_m  = range_max_.as(Units::METERS);
  double range_res_m  = range_resolution_.as(Units::METERS);

  // collect the azimuth list:
  std::vector<double> azimuths;
  double   azim_min_rad = azim_center - 0.5*fov;
  double   azim_max_rad = azim_center + 0.5*fov;
  double   halfSpan       = 0.5 * (azim_max_rad - azim_min_rad);
  double   halfCount      = halfSpan/azim_res_rad;
  double   halfRemainder  = fmod(halfSpan, azim_res_rad);
  unsigned int halfCountInt = static_cast<unsigned int>(floor(halfCount));
  double remainder = halfRemainder;

  // Precision issues can sometimes cause a remainder to exist when azim_res_rad divides halfSpan evenly
  if (osg::equivalent(fov, azim_res_rad * 2 * halfCountInt))
    remainder = 0.0;

  double azim_iter = azim_min_rad;
  if (!osg::equivalent(remainder, 0.0))
  {
    azimuths.push_back(azim_iter);
    azim_iter += remainder;
  }
  for (unsigned int i = 0; i <= 2 * halfCountInt; ++i)
  {
    azimuths.push_back(azim_iter);
    azim_iter += azim_res_rad;
  }
  if (!osg::equivalent(remainder, 0.0))
  {
    azimuths.push_back(azim_max_rad);
  }

  simCore::CoordinateConverter cc;
  simCore::Coordinate originLlaCoord;
  cc.convert(originCoord, originLlaCoord, simCore::COORD_SYS_LLA);
  cc.setReferenceOrigin(originLlaCoord.position());

  bool validLos = false;
  // step through the azimuthal range:
  for (std::vector<double>::iterator i = azimuths.begin(); i != azimuths.end(); ++i)
  {
    double azim_rad = *i;
    double x = sin(azim_rad);
    double y = cos(azim_rad);

    radials_.push_back(Radial(azim_rad));
    Radial& radial = radials_.back();

    // Track the highest elevation along this azimuth to check for visibility
    double maxElev = -2 * M_PI;
    // step through the distance range:
    bool rangeDone = false;
    bool lastSampleValid = false;
    for (double range_m = range_res_m; !rangeDone; range_m += range_res_m)
    {
      if (range_m >= range_max_m)
      {
        range_m = range_max_m;
        rangeDone = true;
      }

      // calculate the world point:
      osg::Vec3d sampleWorld = osg::Vec3d(x*range_m, y*range_m, 0.0) * local2world;

      // convert to a map point
      osgEarth::GeoPoint mapPoint;
      mapPoint.fromWorld(mapNode->getMapSRS(), sampleWorld);

      // sample the terrain at that point
      double hamsl = 0.0, hae = 0.0;

      bool ok = mapNode->getTerrain()->getHeight(mapPoint.getSRS(), mapPoint.x(), mapPoint.y(), &hamsl, &hae);

      if (ok)
      {
        // see if the point is unobstructed.
        mapPoint.z() = hae;

        simCore::Coordinate destCoord;
        convertGeoPointToCoord(mapPoint, destCoord, mapNode);

        double elev;
        simCore::calculateAbsAzEl(originLlaCoord.position(), destCoord.position(), NULL, &elev, NULL, simCore::FLAT_EARTH, &cc);

        bool visible = false;
        if (elev >= maxElev)
        {
          maxElev = elev;
          visible = true;
        }

        radial.samples_.push_back(Sample(range_m, mapPoint, hamsl, hae, elev, visible));
        if (!validLos)
        {
          // To be valid there needs to be at least two consecutive points on the same azimuth
          if (lastSampleValid)
            validLos = true;
          lastSampleValid = true;
        }
      }
      else
      {
        // record an "invalid" sample
        radial.samples_.push_back(Sample(range_m, mapPoint));
        lastSampleValid = false;
      }
    }
  }

  srs_ = mapNode->getMapSRS();

  dirty_ = false;
  return validLos;
}

bool RadialLOS::update(osgEarth::MapNode* mapNode, const osgEarth::GeoExtent& extent, osg::Node* patch)
{
  osg::Vec3d originWorld;
  originMap_.toWorld(originWorld);

  // NOTE:
  // if any point in the radial falls within the extent, we will have to
  // recalculate the visibility of the entire radial.
  for (unsigned int radialIndex = 0; radialIndex < radials_.size(); ++radialIndex)
  {
    Radial& radial = radials_[radialIndex];
    unsigned int firstNewSampleIndex = ~0;

    for (unsigned int sampleIndex = 0; sampleIndex < radial.samples_.size(); ++sampleIndex)
    {
      Sample& sample = radial.samples_[sampleIndex];

      if (!sample.point_.isValid() || extent.contains(sample.point_))
      {
        bool ok = mapNode->getTerrain()->getHeight(patch, sample.point_.getSRS(), sample.point_.x(), sample.point_.y(), &sample.hamsl_m_, &sample.hae_m_);

        if (ok && firstNewSampleIndex == ~(0u))
        {
          firstNewSampleIndex = sampleIndex;
        }
      }
    }

    // if we re-sampled anything, start with the first new sample and recalculate
    // visibility from there on out.
    if (firstNewSampleIndex != ~(0u))
    {
      osg::Matrix local2world;
      originMap_.createLocalToWorld(local2world);

      osg::Vec3d originUpWorld = osg::Vec3d(0, 0, 1) * local2world;
      originUpWorld.normalize();

      double maxElev = -2 * M_PI;
      for (unsigned int sampleIndex = 0; sampleIndex < radial.samples_.size(); ++sampleIndex)
      {
        Sample& sample = radial.samples_[sampleIndex];

        // recalculate the elevation for all the new samples only.
        if (sampleIndex >= firstNewSampleIndex)
        {
          // see if the point is unobstructed.
          sample.point_.z() = sample.hae_m_;

          simCore::Coordinate destCoord;
          convertGeoPointToCoord(sample.point_, destCoord, mapNode);
          simCore::Coordinate originCoord;
          convertGeoPointToCoord(originMap_, originCoord, mapNode);

          simCore::CoordinateConverter cc;
          simCore::Coordinate originLlaCoord;
          cc.convert(originCoord, originLlaCoord, simCore::COORD_SYS_LLA);
          cc.setReferenceOrigin(originLlaCoord.lat(), originLlaCoord.lon(), originLlaCoord.alt());

          double elev;
          simCore::calculateAbsAzEl(originLlaCoord.position(), destCoord.position(), NULL, &elev, NULL, simCore::FLAT_EARTH, &cc);
          sample.elev_rad_ = elev;
        }

        if (sample.elev_rad_ >= maxElev)
        {
          maxElev = sample.elev_rad_;
          sample.visible_ = true;
        }
        else
        {
          sample.visible_ = false;
        }
      }
    }
  }
  return true;
}

bool RadialLOS::getMinMaxHeight(const Angle& azimuth, Distance& out_minHeight, Distance& out_maxHeight) const
{
  // ensure the test is within the computed range
  //if ( azimuth < azim_min_ || azimuth > azim_max_ )
  //  return false;

  if (azimuth < (azim_center_-(fov_*0.5)) || azimuth > (azim_center_+(fov_*0.5)))
    return false;

  // find the radials bounding the specified azimuth:
  Radial interp(azimuth.as(Units::RADIANS));
  if (!makeRadial_(interp))
    return false;

  // Find the minimum and maximum height.
  double h_min = std::numeric_limits<double>::max(), h_max = std::numeric_limits<double>::min();
  for (unsigned int i = 0; i < interp.samples_.size(); ++i)
  {
    double h = interp.samples_[i].hamsl_m_;
    if (h < h_min)
      h_min = h;
    if (h > h_max)
      h_max = h;
  }

  // dump to output.
  out_minHeight = Distance(h_min, Units::METERS);
  out_maxHeight = Distance(h_max, Units::METERS);

  return true;
}

bool RadialLOS::getLineOfSight(const simCore::Coordinate& target, bool& out_isVisible) const
{
  assert(srs_.valid());

  // Convert the target to local tangent frame:
  simCore::CoordinateConverter conv;
  simCore::Coordinate targetECEF;
  conv.convert(target, targetECEF, simCore::COORD_SYS_ECEF);

  // compute the local frame transforms.
  // TODO: we can probably just cache these on first use
  osg::Matrix local2world, world2local;
  originMap_.createLocalToWorld(local2world);
  world2local.invert(local2world);

  // convert the target coords to the local frame, record the target's range,
  // and normalize the vector to the target so we can use it for dot-product
  // visibility detection:
  osg::Vec3d targetLocal = osg::Vec3d(targetECEF.x(), targetECEF.y(), targetECEF.z()) * world2local;
  osg::Vec2d targetLocal2D(targetLocal.x(), targetLocal.y());
  double targetRange = targetLocal2D.length();
  targetLocal.normalize();

  // calculate the azimuth of the target vector:
  double azim_rad = atan2(targetLocal.x(), targetLocal.y());

  // compute the interpolated radial at that azimuth:
  Radial r(azim_rad);
  if (!makeRadial_(r))
    return false; // error.

  // iterate over the radial samples until we reach the target range.
  unsigned int num = r.samples_.size();
  if (num == 0)
    return false; // error.

  unsigned int i;
  for (i = 0; i < num && targetRange > r.samples_[i].range_m_; ++i);

  bool ok = true;

  // before the first sample?
  if (i == 0)
  {
    out_isVisible = r.samples_[i].visible_;
  }
  else if (i == num)
  {
    // past the last sample?
    ok = false; // error.
  }
  else if (r.samples_[i].visible_ == r.samples_[i-1].visible_)
  {
    // between two samples but they are the same?
    out_isVisible = r.samples_[i].visible_;
  }
  else
  {
    // between two differing samples? take the closest one.
    double t = (targetRange - r.samples_[i-1].range_m_) / (r.samples_[i].range_m_ - r.samples_[i-1].range_m_);
    out_isVisible = t < 0.5 ? r.samples_[i-1].visible_ : r.samples_[i].visible_;
  }

  return ok;
}

bool RadialLOS::getBoundingRadials_(double azim_rad, const Radial*& out_r0, const Radial*& out_r1, double& out_mix) const
{
  // make sure data is legit
  if (
    dirty_ ||
    azim_rad          <  (azim_center_-(fov_*0.5)).as(Units::RADIANS) || //azim_min_.as(Units::RADIANS) ||
    azim_rad          >  (azim_center_+(fov_*0.5)).as(Units::RADIANS) || //azim_max_.as(Units::RADIANS) ||
    azim_resolution_  <= Angle(0.0, Units::RADIANS) ||
    range_max_        <= Distance(0.0, Units::METERS) ||
    range_resolution_ <= Distance(0.0, Units::METERS))
  {
    return false;
  }

  // find the index of the lower-bounding radial:
  double azim_min_rad = (azim_center_-(fov_*0.5)).as(Units::RADIANS);
  unsigned int index = static_cast<unsigned int>(floor((azim_rad - azim_min_rad) / azim_resolution_.as(Units::RADIANS)));

  out_r0 = &radials_[index];

  // record the upper-bounding radial and the interpolation factor between the two.
  if (index == radials_.size()-1)
  {
    out_r1 = out_r0;
    out_mix = 0.0;
  }
  else
  {
    out_r1 = &radials_[index+1];
    out_mix = (azim_rad - out_r0->azim_rad_) / (out_r1->azim_rad_ - out_r0->azim_rad_);
  }

  return true;
}

unsigned int RadialLOS::getNumSamplesPerRadial() const
{
  return radials_.size() > 0 ? radials_.front().samples_.size() : 0u;
}

bool RadialLOS::makeRadial_(Radial& out_radial) const
{
  const Radial* r0;
  const Radial* r1;
  double        mix;
  if (!getBoundingRadials_(out_radial.azim_rad_, r0, r1, mix))
    return false;

  osg::Vec3d localUp(0, 0, 1);
  double maxDotProduct = -1.1;

  unsigned int num = r0->samples_.size();
  for (unsigned int i = 0; i < num; ++i)
  {
    double hamsl0 = r0->samples_[i].hamsl_m_;
    double hae0   = r0->samples_[i].hae_m_;
    double hamsl1 = r1->samples_[i].hamsl_m_;
    double hae1   = r1->samples_[i].hae_m_;
    double range  = r0->samples_[i].range_m_;

    double hamsl = hamsl0 + mix*(hamsl1-hamsl0);
    double hae   = hae0   + mix*(hae1-hae0);
    bool   visible = false;

    osg::Vec3d look(range, 0.0, hae-originMap_.z());
    look.normalize();
    double dot = localUp * look;

    if (dot > maxDotProduct)
    {
      maxDotProduct = dot;
      visible = true;
    }

    out_radial.samples_.push_back(Sample(range, osgEarth::GeoPoint::INVALID, hamsl, hae, dot, visible));
  }

  return true;
}
