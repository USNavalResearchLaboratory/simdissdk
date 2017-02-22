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
#ifndef SIMVIS_SPHERICAL_VOLUME_H
#define SIMVIS_SPHERICAL_VOLUME_H

#include "simCore/Common/Common.h"
#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "osgEarth/Revisioning"
#include "osgEarthSymbology/Symbol"

namespace simVis
{

/// Configuration data for creating volumetric geometry for beams and gates
struct SVData
{
  /// Shape of the base, which extends out to the point
  enum Shape
  {
    SHAPE_PYRAMID, ///< four sided base
    SHAPE_CONE     ///<   circular base
  };

  /// how will the volume be drawn
  enum DrawMode
  {
    DRAW_MODE_NONE    = 0,      ///< invisible
    DRAW_MODE_SOLID   = 1 << 0, ///< solid (probably translucent)
    DRAW_MODE_STIPPLE = 1 << 1, ///< stippled (many small dots)
    DRAW_MODE_WIRE    = 1 << 2, ///< wireframe
    DRAW_MODE_OUTLINE = 1 << 3, ///< outlines only
    DRAW_MODE_POINTS  = 1 << 4  ///< points at the key locations
  };

  /** Draw as a pyramid or cone */
  Shape      shape_;
  /** Mask of DrawMode enumeration values */
  int        drawMode_;
  /** Resolution of the cap */
  unsigned int        capRes_;
  /** Resolution of the cone */
  unsigned int        coneRes_;
  /** Resolution of the wall */
  unsigned int        wallRes_;
  /** OSG (RGBA) color of the shape */
  osg::Vec4f color_;
  /** Width of the outline around the volume */
  float      outlineWidth_;
  /** True if lighting is enabled */
  bool       lightingEnabled_;
  /** True if blending is enabled */
  bool       blendingEnabled_;
  /** Shape XYZ offset from origin in meters */
  osg::Vec3  xyzOffset_m_;
  /** Heading and pitch offset for the shape in degrees */
  osg::Vec2  hpOffset_deg_;
  /** Scale to apply to the volume */
  float      scale_;
  /** True if cone is drawn */
  bool       drawCone_;
  /** True if should be drawn as a spherical segment */
  bool       drawAsSphereSegment_;

  /** True to draw */
  bool  visible_;
  /** Horizontal field of view in degrees */
  float hfov_deg_;
  /** Vertical field of view in degrees */
  float vfov_deg_;
  /** Azimuth offset in degrees */
  float azimOffset_deg_;
  /** Elevation offset in degrees */
  float elevOffset_deg_;
  /** Near plane for the volume in meters */
  float nearRange_;
  /** Far plane for the volume in meters */
  float farRange_;
  /** Center range for the centroid shape */
  float centerRange_;

  /** Default constructor */
  SVData()
    : shape_(SHAPE_CONE),
    drawMode_(DRAW_MODE_SOLID),
    capRes_(10),
    coneRes_(20),
    wallRes_(10),
    color_(osg::Vec4f(1, 1, 0, 0.5)),
    outlineWidth_(1.0f),
    lightingEnabled_(false),
    blendingEnabled_(true),
    scale_(1.0f),
    drawCone_(true),
    drawAsSphereSegment_(false),

    visible_(true),
    hfov_deg_(15.0f),
    vfov_deg_(10.0f),
    azimOffset_deg_(0.0f),
    elevOffset_deg_(0.0f),
    nearRange_(0.0f),
    farRange_(10000.0f),
    centerRange_(5000.0f)
  {
  }
};

/// structure for recording information about an existing geometry so we can update it
struct SVState
{
  /** Index offset for the near face */
  unsigned int nearFaceOffset_;
  /** Index offset for the far face */
  unsigned int farFaceOffset_;
};

/// Utility class to create volumetric geometry for beams and gates (internal)
class SVFactory
{
public:
  /// create a node visualizing the spherical volume given in 'data'
  static osg::MatrixTransform* createNode(const SVData &data, const osg::Vec3& dir = osg::Vec3(0, 1, 0));

  /// set lighting
  static void updateLighting(osg::MatrixTransform* xform, bool lighting);
  /// set blending
  static void updateBlending(osg::MatrixTransform* xform, bool blending);
  /// set the color
  static void updateColor(osg::MatrixTransform* xform, const osg::Vec4f& color);
  /// set the stipple mode
  static void updateStippling(osg::MatrixTransform* xform, bool stippling);
  /// move the verts comprising the far range
  static void updateNearRange(osg::MatrixTransform* xform, float range);
  /// move the verts comprising the near range
  static void updateFarRange(osg::MatrixTransform* xform, float range);
  /// tweak the verts to update the horizontal angle
  static void updateHorizAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle);
  /// tweak the verts to update the vertical angle
  static void updateVertAngle(osg::MatrixTransform* xform, float oldAngle, float newAngle);

private:
  static void createPyramid_(osg::Geode& geode, const SVData &data, const osg::Vec3& dir);
  static osg::Geometry* createCone_(const SVData &data, const osg::Vec3& dir);

  /** Retrieves the 'solid' geometry */
  static osg::Geometry* solidGeometry_(osg::MatrixTransform* xform);
  /** Retrieves the 'outline' geometry */
  static osg::Geometry* outlineGeometry_(osg::MatrixTransform* xform);
};

}

#endif // SIMVIS_SPHERICAL_VOLUME_H

