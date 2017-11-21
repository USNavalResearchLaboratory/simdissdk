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
#ifndef SIMUTIL_SCREENCOORDINATECALCULATOR_H
#define SIMUTIL_SCREENCOORDINATECALCULATOR_H

#include "osg/observer_ptr"
#include "osg/Matrix"
#include "osg/Vec2"
#include "osg/Vec3"
#include "simCore/Common/Export.h"

namespace osg { class Viewport; }
namespace simVis {
  class View;
  class EntityNode;
}

namespace simUtil
{

/**
 * Represents a coordinate in screen space.
 * A coordinate in screen space has an X and a Y that is in pixels.  The Z coordinate is
 * inversely relative to the near clipping plane.  A coordinate "behind" the viewer has
 * a Z value greater than 1, and a coordinate in front of the viewer has a Z value less than 1.
 * A coordinate may also be known to be off-screen, i.e. outside the extents of the viewport.
 * Origin is (0,0) in lower-left corner.  X increases positively to the right, Y positive up.
 *
 * ScreenCoordinate is typically instantiated as a return value from ScreenCoordinateCalculator.
 */
class SDKUTIL_EXPORT ScreenCoordinate
{
public:
  /**
   * Constructs a new screen coordinate.  Includes flag to indicates whether pixel is inside or outside view.
   * @param position Screen coordinate vector, with origin at lower-left corner of screen, X increasing right
   *   and Y increasing up.  Z is a unitless coordinate with 1 at near clipping plane, values greater than 1
   *   behind the near plane (behind viewer), and values less than 1 in front of the viewer.
   * @param outOfViewport Indicates that the coordinate is outside the View used to generate the coordinate.
   *   It is entirely possible for a coordinate to be in front of the camera, but outside the view's viewport.
   */
  ScreenCoordinate(const osg::Vec3& position, bool outOfViewport);

  /** X and Y position in pixels of the coordinate */
  osg::Vec2 position() const;
  /** X and Y position in pixels of coordinate, and Z value (unitless) representing relative depth */
  osg::Vec3 positionV3() const;
  /** Returns true if the position is behind the viewer / camera */
  bool isBehindCamera() const;
  /**
   * Returns true if the position is outside the bounds of the viewport.  Note that coordinate
   * can be on-screen but behind viewer.  In other words, it is possible that isOffScreen()
   * is false, but isBehindCamera() is true.
   */
  bool isOffScreen() const;

private:
  osg::Vec3 position_;
  bool isOffScreen_;
};

/**
 * Responsible for calculating screen coordinates from a given view.
 * Provides a cache for simVis::View to-screen matrix to optimize queries on multiple
 * platforms within the same view under the same frame.
 * Example usage:
 *
 * <code>
 * simUtil::ScreenCoordinateCalculator calc;
 * calc.updateMatrix(view);
 * simUtil::ScreenCoordinate coord = calc.calculate(entity);
 * </code>
 */
class SDKUTIL_EXPORT ScreenCoordinateCalculator
{
public:
  ScreenCoordinateCalculator();
  virtual ~ScreenCoordinateCalculator();

  /** Update the internal projection matrix based on the view.  Call whenever view, projection, or window matrix changes */
  void updateMatrix(const simVis::View& view);

  /** Retrieves a coordinate for a given entity, using the matrix from the most recent call to updateMatrix() */
  ScreenCoordinate calculate(const simVis::EntityNode& entity);
  /** Retrieves a screen coordinate for a given lla coordinate */
  ScreenCoordinate calculate(const simCore::Vec3& lla);

private:
  /** recalculates the VPW matrix if needed (if dirty); returns 0 on success */
  int recalculateVPW_();
  /* Convert an ecef coordinate to a screen coordinate */
  ScreenCoordinate matrixCalculate_(const osg::Vec3d& ecefCoordinate) const;

  /// Combined View matrix * projection matrix * window matrix
  osg::Matrix viewProjectionWindow_;
  /// Flags true when the VPW is dirty.  Used to avoid unnecessary recalculations
  bool dirtyMatrix_;
  /// Pointer to the viewport for the current view
  osg::observer_ptr<const simVis::View> view_;
};

}

#endif /* SIMUTIL_SCREENCOORDINATECALCULATOR_H */
