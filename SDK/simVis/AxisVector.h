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
#ifndef SIMVIS_AXIS_VECTOR_H
#define SIMVIS_AXIS_VECTOR_H

#include "osg/LineWidth"
#include "osg/MatrixTransform"
#include "osg/Vec3"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/Types.h"

namespace osg { class Geometry; }

namespace simVis
{

/**
 * X, Y, and Z Axis display.  Unit length, though can be scaled.
 */
class SDKVIS_EXPORT AxisVector : public osg::MatrixTransform
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, AxisVector);
  /** Constructor */
  AxisVector();
  /** OSG Copy constructor */
  AxisVector(const AxisVector &rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Helper method to configure a scale matrix; will optimize away if not set, unless force is true */
  void setAxisLengths(osg::Vec3f axisLengths, bool force=false);
  /** Helper method to configure a scale matrix; will optimize away if not set, unless force is true */
  void setAxisLengths(float xLength, float yLength, float zLength, bool force=false);
  /** Last line length set from setAxisLength */
  osg::Vec3f axisLengths() const;

  /** Set width of the lines for the axes */
  void setLineWidth(float lineWidth);
  /** Retrieve width of th elines for the axes */
  float lineWidth() const;

  /** Change the axis colors */
  void setColors(const simVis::Color& x, const simVis::Color& y, const simVis::Color& z);
  /** Retrieve the X axis color */
  simVis::Color xColor() const;
  /** Retrieve the Y axis color */
  simVis::Color yColor() const;
  /** Retrieve the Z axis color */
  simVis::Color zColor() const;

protected:

  /// osg::Referenced-derived
  virtual ~AxisVector();

private:
  /// create the geometry
  void init_();

  /// create the axis vector lines
  void createAxisVectors_(osg::Geode* geode) const;

  /// Draws a straight line between two points, subdividing it an arbitrary number of times
  void addLineStrip_(osg::Geometry& geom, osg::Vec3Array& vertices, int& primitiveSetStart,
    const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine) const;

  /// width of axis vector lines
  osg::ref_ptr<osg::LineWidth> lineWidth_;
  /// array of X, Y, and Z colors for binding
  osg::ref_ptr<osg::Vec4Array> colors_;
  /// most recent value for axis size
  osg::Vec3f axisLengths_;
};

} // namespace simVis

#endif // SIMVIS_AXIS_VECTOR_H
