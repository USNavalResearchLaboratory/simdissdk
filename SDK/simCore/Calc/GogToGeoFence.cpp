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
#include <algorithm>
#include <iostream>
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Units.h"
#include "simCore/GOG/Parser.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/GogToGeoFence.h"

namespace simCore
{

GogToGeoFence::GogToGeoFence()
{
}

GogToGeoFence::~GogToGeoFence()
{
  // no-op
}

int GogToGeoFence::parse(std::istream& is, const std::string& gogFileName)
{
  std::vector<simCore::GOG::GogShapePtr> shapes;
  simCore::GOG::Parser parser;
  parser.parse(is, gogFileName, shapes);
  for (const auto& shapePtr : shapes)
  {
    // Don't create a fence if the off keyword was found in this shape
    bool draw = true;
    shapePtr->getIsDrawn(draw);
    if (!draw)
      continue;

    if (shapePtr->isRelative() ||
      (shapePtr->shapeType() != simCore::GOG::ShapeType::LINE &&
      shapePtr->shapeType() != simCore::GOG::ShapeType::POLYGON))
    {
      SIM_ERROR << "Only absolute line and polygon shapes are accepted.\n";
      continue;
    }

    // parser seems to assign default names if no name provided so "no name" is unlikely
    std::string name = "no name";
    shapePtr->getName(name);


    int rv = 0;

    if (shapePtr->shapeType() == simCore::GOG::ShapeType::LINE)
    {
      const simCore::GOG::Line* line = dynamic_cast<const simCore::GOG::Line*>(shapePtr.get());
      if (!line)
      {
        // LINE shapetype should guarantee the cast to shape
        assert(0);
        continue;
      }
      const Vec3String& coordinates = line->points();
      // Geo-Fence doesn't require a closed shape, but we only consider closed lines for geo-fences
      // so that we do not mistake an actual line for a fence. Must have at least 4 points: first
      // and last point match, with a total of 3 unique points.
      if (coordinates[0] != coordinates[coordinates.size() - 1] || coordinates.size() < 4)
      {
        SIM_ERROR << "Fence \"" << name  << "\" is not closed. The first and last coordinates must be the same. This line shape will not act as an exclusion zone.\n";
        continue;
      }
      rv = generateGeoFence_(name, coordinates);
    }

    else if (shapePtr->shapeType() == simCore::GOG::ShapeType::POLYGON)
    {
      const simCore::GOG::Polygon* poly = dynamic_cast<const simCore::GOG::Polygon*>(shapePtr.get());
      if (!poly)
      {
        // POLYGON shapetype should guarantee the cast to shape
        assert(0);
        continue;
      }
      // Coordinates do not need to be closed
      const Vec3String& coordinates = poly->points();
      rv = generateGeoFence_(name, coordinates);
    }

    if (rv == 0)
    {
      // warn about altitude mode
      simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
      shapePtr->getAltitudeMode(mode);
      if (mode == simCore::GOG::AltitudeMode::NONE)
      {
        SIM_DEBUG << "Added GOG \"" << name << "\" as geoFence.\n";
      }
      else
      {
        SIM_WARN << "GOG \"" << name << "\" added as a geoFence. The geofence will ignore the altitude mode.\n";
      }
    }
  }

  // If fences_ is empty, then all GOG shapes parsed were invalid
  if (fences_.empty())
  {
    SIM_ERROR << "No valid geofences could be created from the shape(s) in the GOG file.\n";
    return 1;
  }

  return 0;
}

int GogToGeoFence::generateGeoFence_(const std::string& name, const Vec3String& coordinates)
{
  // Generate a simCore::GeoFence for this shape and save it
  std::shared_ptr<simCore::GeoFence> fence = std::make_shared<simCore::GeoFence>(coordinates, simCore::COORD_SYS_LLA);

  // Check that newly created GeoFence is convex
  if (!fence->valid())
  {
    // Invalid, maybe the points are drawn in clockwise order?
    // Reverse the points, and re-set the GeoFence
    Vec3String coordinates_copy = coordinates;
    std::reverse(coordinates_copy.begin(), coordinates_copy.end());
    fence->set(coordinates_copy, simCore::COORD_SYS_LLA);
    if (!fence->valid())
    {
      SIM_ERROR << "Fence \"" << name  << "\" is concave. This shape will be drawn but will not act as an exclusion zone.\n";
      return 1;
    }
  }

  fences_.push_back(fence);
  return 0;
}

void GogToGeoFence::getFences(simCore::GogToGeoFence::GeoFenceVec& fences) const
{
  fences = fences_;
}

void GogToGeoFence::clear()
{
  fences_.clear();
}

}
