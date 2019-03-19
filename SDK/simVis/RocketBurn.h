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
#ifndef SIMVIS_ROCKET_BURN_H
#define SIMVIS_ROCKET_BURN_H

#include "osg/ref_ptr"
#include "osg/Texture2D"
#include "osg/Billboard"
#include "simCore/Calc/Vec3.h"

namespace osg { class MatrixTransform; }

namespace simVis
{
class PlatformNode;

/**
 * Node holding a visual representation of a rocket burn.
 */
class SDKVIS_EXPORT RocketBurn : public osg::Referenced
{
public:
  /** Describes the user-defined shape of the rocket burn */
  struct SDKVIS_EXPORT ShapeData
  {
    float radiusNear; ///< radius (meters) near the host
    float radiusFar; ///< radius (meters) away from the host
    float length; ///< distance (meters) from far to near

    osg::Vec4 color; ///< color (RGBA) to use
    bool scaleAlpha; ///< should alpha value be scaled along the length

    /// default constructor gives reasonable values
    ShapeData();

    /// Comparison operator
    bool operator==(const ShapeData& other) const;
  };

  /**
   * Construct a new rocket burn.  Adds to the scene.
   * @param hostPlatform platform the burn is connected to.
   * @param texture the texture that will be used for the rcoket burn visualization.
   */
  RocketBurn(PlatformNode &hostPlatform, osg::Texture2D& texture);

  /// update the shape of the burn
  void update(const ShapeData &newShapeData);

  /// Changes the pointing angles (radians) and offset position (meters XYZ relative to platform) for the rocket burn
  void setPositionOrientation(const osg::Vec3f& newPosition, const osg::Vec3f& yprRadians);

protected:
  /// osg::Referenced-derived
  virtual ~RocketBurn();

private:
  /// Removes the rocket burn node from the scene
  void removeFromScene_();
  /// Lazy initialize on the group_, build or update the puff geometry
  void rebuild_();

  /// Holds onto the billboard texture
  osg::ref_ptr<osg::Texture2D> texture_;
  /// Rotate and position matrix
  osg::ref_ptr<osg::MatrixTransform> transform_;
  /// Holds the drawables for the rocketburn
  osg::ref_ptr<osg::Group> group_;
  /// Rocket burn geometry
  osg::ref_ptr<osg::Geometry> geometry_;
  /// Describes the current burn
  ShapeData currentShape_;
  /// Flags whether we've had the shader generator run or not (shader generation can be expensive)
  bool shaderGeneratorRun_;

  /// Shared stateset for rocket burn programs
  static osg::observer_ptr<osg::StateSet> s_stateSet_;
};

}

#endif

