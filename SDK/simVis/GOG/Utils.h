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
#ifndef SIMVIS_GOG_UTILS_H
#define SIMVIS_GOG_UTILS_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/Units.h"
#include "simVis/GOG/GOGNode.h"
#include "osgEarth/GeoData"
#include "osgEarth/Units"
#include "osgEarth/Geometry"
#include "osgEarth/AnnotationData"
#include "osgEarth/AnnotationNode"

namespace osgEarth {
  class LocalGeometryNode;
}

/**
 * Utilities used internally by the GOG parsers.
 */
namespace simVis { namespace GOG
{

  /** Utility class for the shapes */
  class SDKVIS_EXPORT Utils
  {
    public:
      /// enum describing Overlay line style
      enum LineStyle
      {
        LINE_SOLID,
        LINE_DASHED,
        LINE_DOTTED
      };

      /**
      * Determines if the specified shape's geometry can be serialized directly into Overlay format. This is dependent on how the shapes
      * are constructed from osg::Nodes. Things like lines and polygons have few points representing the vertices, which matches. However circles
      * and ellipses are made up of multiple points around the circumference, and these points will not easily translate into Overlay format
      */
      static bool canSerializeGeometry_(simVis::GOG::GogShape shape);

      /**
      * Get a vector of all the points in the Geometry. Handle the case where the geometry may be a MultiGeometry, for shapes like
      * line segs. Fills up the points param with all the point values, in standard osgEarth format, lon/lat/alt, units are deg/deg/meters
      */
      static void getGeometryPoints(const osgEarth::Geometry* geometry, std::vector<osg::Vec3d>& points);

      /**
      * Returns the LineStyle based on the stipple value
      * @param stipple
      * @return the equivalent line style
      */
      static LineStyle getLineStyleFromStipple(unsigned short stipple);

      /**
      * Retrieve the osgEarth stipple value from the LineStyle
      * @param lineStyle
      * @return osgEarth stipple value
      */
      static unsigned short getStippleFromLineStyle(LineStyle lineStyle);

      /**
      * Decrypt the geometry object to determine if it is a MultiGeometry, then serialize the position information
      * from the osgEarth::Geometry into a string in the standard GOG format
      * @param geometry to serialize
      * @param relativeShape  true if these are relative positions, false for absolute
      * @param gogOutputStream  ostream that holds the serialized position information
      */
      static void serializeShapeGeometry(const osgEarth::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream);

      /**
      * Serialize the position information from the osgEarth::Geometry into a string in the standard GOG format.
      * Applies the keyword 'xyz' if relative, 'lla otherwise
      * @param geometry to serialize
      * @param relativeShape  true if these are relative positions, false for absolute
      * @param gogOutputStream  ostream that holds the serialized position information
      */
      static void serializeGeometry(const osgEarth::Geometry* geometry, bool relativeShape, std::ostream& gogOutputStream);

      /**
      * Serialize the osg color into a AGBR hex string
      * @param colorVec  osg color vector (RGBA)
      * @return string representation of hex AGBR color
      */
      static std::string serializeOsgColor(const osg::Vec4f& colorVec);

      /**
      * Serialize the line style, converts the enum into a string
      * @param lineStyle   enum
      * @return string representation of the linestyle enum
      */
      static std::string serializeLineStyle(LineStyle lineStyle);

      /**
       * True if the points in the geometry all have a zero Z value OR the
       * style calls for terrain-clamping. This is useful to know since "2D"
       * geometry will likely encounter Z-buffer issues, and therefore we
       * need to account for that.
       */
      static bool isGeometry2D(const osgEarth::Geometry* geom);

      /**
       * If the geometry in this parser is "2D" (as determined by isGeometry2D)
       * update its rendering style to prevent it from z-fighting with the terrain.
       * You should call this after all the normal style and geometry parsing
       * has completed.
       */
      static void configureStyleForClipping(osgEarth::Style& style);

      /** Attempts to read an image, given a filename or URL; attempts to resolve file:// references */
      static osg::ref_ptr<osg::Image> readRefImage(const std::string& addr);

      /**
      * Converts the GogShape enum to the GOG file keyword
      * @param shape GogShape enum to get the keyword for
      * @return GOG file shape keyword, empty for unknown
      */
      static std::string getKeywordFromShape(GogShape shape);
  };

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_H
