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
#ifndef SIMVIS_CYLINDER_H
#define SIMVIS_CYLINDER_H

#include "osg/ref_ptr"
#include "osg/Referenced"
#include "osg/Vec4"
#include "simCore/Calc/Vec3.h"
#include "simCore/Common/Common.h"

namespace osg {
  class MatrixTransform;
  class Geode;
}

namespace simVis
{
class PlatformNode;

/** Node holding a visual representation of a cylinder. */
class SDKVIS_EXPORT CylinderGeode : public osg::Referenced
{
public:
  /** Describes the user-defined shape of the cylinder */
  struct SDKVIS_EXPORT ShapeData
  {
    /** Radius (meters) near the host */
    double radiusNear;
    /** Radius (meters) away from the host */
    double radiusFar;
    /** Distance (meters) from near face to far face */
    double length;

    /**  Color (RGBA) to use */
    osg::Vec4 colorNear;
    /**  Color (RGBA) to use */
    osg::Vec4 colorFar;

    /**  Default constructor gives reasonable values */
    ShapeData();

    /**  Comparison operator */
    bool operator==(const ShapeData& other) const;
  };

  /**
  * Construct a new cylinder.  Adds to the scene.
  * @param hostPlatform platform the cylinder is connected to.
  */
  CylinderGeode(PlatformNode &hostPlatform);

  /**  Update the shape of the cylinder */
  void update(const ShapeData &newShapeData);

  /**  Changes the pointing angles (radians) and offset position (meters XYZ relative to platform) for the cylinder */
  void setPositionOrientation(const simCore::Vec3& newPosition, const simCore::Vec3& yprRadians);

protected:
  /**  Osg::Referenced-derived */
  virtual ~CylinderGeode();

private:
  /**  Removes the cylinder node from the scene */
  void removeFromScene_();
  void rebuild_();

  /** Rotation and Translation matrix */
  osg::ref_ptr<osg::MatrixTransform> transform_;
  /**  Holds the drawables for the Cylinder */
  osg::ref_ptr<osg::Geode> geode_;
  /**  Describes the current cylinder */
  ShapeData currentShape_;
};

}

#endif