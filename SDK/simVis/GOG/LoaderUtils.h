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
#ifndef SIMVIS_GOG_LOADERUTILS_H
#define SIMVIS_GOG_LOADERUTILS_H

#include "osgEarth/GeoData"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Utils.h"
#include "simVis/GOG/LoaderUtils.h"

namespace osgEarth {
  class LocalGeometryNode;
  class Geometry;
}
namespace simCore { namespace GOG { class GogShape; } }

namespace simVis { namespace GOG
{

/** Utility class for converting the simCore::GOG::GogShape classes into GogNodeInterface objects */
class SDKVIS_EXPORT LoaderUtils
{
public:
  /// Return true if the shape has 3D geometry
  static bool isShape3D(const simCore::GOG::GogShape& shape);
  /// Return true if the shape geometry will result in its being coincident with the terrain, a possible z - fighting condition.
  static bool geometryRequiresClipping(const simCore::GOG::GogShape& shape);

  /**
  * Set the shape's center position and apply any position and orientation offsets. For attached shapes, simply sets the center point as the position, since
  * the host platform handles all geo-positioning. For absolute shapes, simply uses centerPoint as position. For relative un-attached shapes, finds reference
  * point, either provided in the shape or the passed in refPoint, and applies centerPoint offsets.
  * Applies the shape's orientation offsets to the node's local rotation.
  * @param node the LocalGeometryNode that needs position and orientation offsets set
  * @param shape the parsed shape data to use for applying offsets
  * @param centerPoint treated as lla radians or xyz meters, depening on if shape is relative
  * @param refPoint default reference point for use with relative un-attached shapes, lla radians
  * @param attached true if shape is attached to a platform
  * @param ignoreOffset ignore the offset values provided for relative un-attached shapes, uses the shape's reference point or the provided refPoint
  */
  static void setShapePositionOffsets(osgEarth::LocalGeometryNode& node, const simCore::GOG::GogShape& shape, const simCore::Vec3& centerPoint, const simCore::Vec3& refPoint, bool attached, bool ignoreOffset);

  /**
  * Return the geographical position of the shape. If it's an absolute shape, simply uses the passed in centerPoint. For relative shapes, uses the shape's
  * reference position and apply the centerPoint xyz offsets, falling back to the specified refPoint if shape has no reference position.
  * @param shape the parsed shape data to use for applying offsets
  * @param centerPoint treated as lla radians or xyz meters, depening on if shape is relative
  * @param refPoint default reference point for use with relative un-attached shapes, lla radians
  * @param ignoreOffset ignore the offset values provided for relative un-attached shapes, uses the shape's reference point or the provided refPoint
  */
  static osgEarth::GeoPoint getShapeGeoPosition(const simCore::GOG::GogShape& shape, const simCore::Vec3& centerPoint, const simCore::Vec3& refPoint, bool ignoreOffset);

  /// Set the scale value from the shape on the top level osgEarth::AnnotationNode found in the specified node
  static void setScale(const simCore::GOG::GogShape& shape, osg::Node* node);
  /// Set the specified points in the specified geometry, treated as xyz meters if relative is true, lla radians otherwise
  static void setPoints(const std::vector<simCore::Vec3>& points, bool relative, osgEarth::Geometry& geom);
  /// Return a spatial reference based on the provided vertical datum string. Defaults to wgs84 if not a valid string value.
  static osgEarth::SpatialReference* getSrs(const std::string vdatum);

  // convert from simCore to simVis
  static osg::Vec4f convertToOsgColor(const simCore::GOG::Color& color);
  static AltitudeMode convertToVisAltitudeMode(simCore::GOG::AltitudeMode mode);
  static Utils::LineStyle convertToVisLineStyle(simCore::GOG::LineStyle lineStyle);
  static GogShape convertToVisShapeType(simCore::GOG::ShapeType type);
  static TessellationStyle convertToVisTessellation(simCore::GOG::TessellationStyle style);
  static simData::TextOutline convertToVisOutlineThickness(simCore::GOG::OutlineThickness thickness);

  // convert from simVis to simCore
  static simCore::GOG::AltitudeMode convertToCoreAltitudeMode(AltitudeMode mode);
  static simCore::GOG::Color convertToCoreColor(const osg::Vec4f& color);
  static simCore::GOG::LineStyle convertToCoreLineStyle(Utils::LineStyle style);
  static simCore::GOG::TessellationStyle convertToCoreTessellation(TessellationStyle style);
  static simCore::GOG::OutlineThickness convertToCoreOutlineThickness(simData::TextOutline thickness);
};

} } // namespace simVis::GOG

#endif
